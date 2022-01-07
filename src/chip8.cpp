#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <time.h>

#include "chip8.h"

void Chip8::init()
{
    pc = 0x200; // Start program counter at 0x200
    opcode = 0; // Reset op code
    I = 0;      // Reset I
    sp = 0;     // Reset stack pointer

    // Clear display
    for (int i = 0; i < 2048; i++)
        gfx[i] = 0;

    // Clear stack, keypad, V registers
    for (int i = 0; i < 16; i++) {
        stack[i] = 0;
        V[i] = 0;
        key[i] = 0;
    }

    // Clear memory
    for (int i = 0; i < 4096; i++)
        memory[i] = 0;

    // Load fontset
    for (int i = 0; i < 80; i++)
        memory[i] = fontset[i];

    // Reset timers
    sound_timer = 0;
    delay_timer = 0;

    srand(time(NULL)); // Seed random
}

bool Chip8::load(const char* file_path)
{
    using namespace std;

    cout << "Loading file: " << file_path << endl;

    // Load ROM
    ifstream file(file_path, ios::binary | ios::ate);
    if (!file.is_open()) {
        cerr << "error: Unable to load ROM" << endl;
        return false;
    }

    // Get file size
    streampos size = file.tellg();
    if (size > (4096 - 0x200)) {
        cerr << "error: ROM file is too large" << endl;
        return false;
    }

    // Write to memory
    char* buffer;
    for (int i = 0; i < size; i++) {
        buffer = new char[1];
        file.seekg(i, ios::beg);
        file.read(buffer, 1);
        memory[i + 0x200] = *buffer;
    }

    // Cleanup
    file.close();
    delete[] buffer;

    return true;
}

void Chip8::emulate_cycle()
{
    // Get opcode
    opcode = memory[pc] << 8 | memory[pc + 1];

    // Decode opcode
    switch (opcode & 0xF000) {
    case 0x00E0: // CLS
        for (int i = 0; i < 2048; i++)
            gfx[i] = 0;
        pc += 2;
        break;

    case 0x00EE: // RET
        pc = stack[sp];
        sp--;
        pc += 2;
        break;

    case 0x1000: // JP addr
        pc = opcode & 0x0FFF;
        break;

    case 0x2000: // CALL addr
        stack[sp] = pc;
        sp++;
        pc = opcode & 0x0FFF;
        break;

    case 0x3000: // Skip if Vx == kk
        if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
            pc += 2;
        pc += 2;
        break;

    case 0x4000: // Skip if Vx != kk
        if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
            pc += 2;
        pc += 2;
        break;

    case 0x5000: // Skip if Vx == Vy
        if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
            pc += 2;
        pc += 2;
        break;

    case 6000: // Set Vx = kk
        V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
        pc += 2;
        break;

    case 7000: // ADD Vx += kk
        V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
        pc += 2;
        break;

    case 8000: // 8xy_
        switch (opcode & 0x000F) {
        case 0x0000: // Vx = Vy
            V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;

        case 0x0001: // Vx = Vx OR Vy
            V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;

        case 0x0002: // Vx = Vx AND Vy
            V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;

        case 0x0003: // Vx = Vx XOR Vy
            V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;

        case 0x0004: // Vx = Vx + Vy, set VF = carry
            if (V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8]))
                V[0xF] = 1;
            else
                V[0xF] = 0;
            V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;

        case 0x0005: // Set Vx = Vx - Vy, set VF = NOT borrow
            if (V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4])
                V[0xF] = 1;
            else
                V[0xF] = 0;
            V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;

        case 0x0006: // Set Vx = Vx SHR 1
            V[0xF] = V[(opcode & 0x0F00) >> 8] & 1;
            V[(opcode & 0x0F00) >> 8] >>= 1; // same as /= 2
            pc += 2;
            break;

        case 0x0007: // Set Vx = Vy - Vx, set VF = NOT borrow
            if (V[(opcode & 0x00F0) >> 4] > (V[(opcode & 0x0F00) >> 8]))
                V[0xF] = 1;
            else
                V[0xF] = 0;
            V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
            pc += 2;
            break;

        case 0x000E: // Vx = Vx SHL 1
            V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
            V[(opcode & 0x0F00) >> 8] <<= 1;
            pc += 2;
            break;
        }
        break;

    case 0x9000: // Skip next instruction if Vx != Vy
        if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
            pc += 2;
        pc += 2;
        break;

    case 0xA000: // Set I = nnn
        I = opcode & 0x0FFF;
        pc += 2;
        break;

    case 0xB000: // Jump to location nnn + V0
        pc = (opcode & 0x0FFF) + V[0];
        break;

    case 0xC000: // Set Vx = random byte AND kk
        V[(opcode & 0x0F00) >> 8] = (rand() % 256) & (opcode & 0x00FF);
        pc += 2;
        break;

    case 0xD000: { // Draw n at (Vx, Vy)

    } break;

    case 0xE000: // Ex__
        switch (opcode & 0x00FF) {
        case 0x009E: // SKP if keypad[Vx] is pressed
            if (key[V[(opcode & 0x0F00) >> 8]] != 0)
                pc += 2;
            pc += 2;
            break;
        case 0x00A1: // SKP if keypad[Vx] is not pressed
            if (key[V[(opcode & 0x0F00) >> 8]] == 0)
                pc += 2;
            pc += 2;
            break;
        }
        break;

    case 0xF000: // Fx__
        switch (opcode & 0x00FF) {
        case 0x0007: // Set Vx = delay timer
            V[(opcode & 0x0F00) >> 8] = delay_timer;
            pc += 2;
            break;
        case 0x000A: // Wait for key press, store key in Vx
            for (int i = 0; i < 16; i++) {
                if (key[i] != 0) {
                    V[(opcode & 0x0F00) >> 8] = key[i];
                    pc += 2;
                    break;
                }
            }
            break;
        case 0x0033: // LD B, Vx
            memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
            memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
            memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
            pc += 2;
            break;
        }
        break;

    default:
        std::cout << "Unimplemented opcode: 0x" << opcode << std::endl;
        exit(1);
    }

    // Update timers
    if (delay_timer > 0)
        delay_timer--;

    if (sound_timer > 0) {
        if (sound_timer == 1)
            std::cout << "BEEP!" << std::endl;
        sound_timer--;
    }
}