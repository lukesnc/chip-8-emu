#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <time.h>

#include "chip8.h"

unsigned char fontset[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, //0
    0x20, 0x60, 0x20, 0x20, 0x70, //1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
    0x90, 0x90, 0xF0, 0x10, 0x10, //4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
    0xF0, 0x10, 0x20, 0x40, 0x40, //7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
    0xF0, 0x90, 0xF0, 0x90, 0x90, //A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
    0xF0, 0x80, 0x80, 0x80, 0xF0, //C
    0xE0, 0x90, 0x90, 0x90, 0xE0, //D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
    0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};

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

void Chip8::exec()
{
    // Get opcode
    opcode = memory[pc] << 8 | memory[pc + 1];
    x = (opcode & 0x0F00) >> 8; // x register
    y = (opcode & 0x00F0) >> 4; // y register

    // Decode opcode
    switch (opcode & 0xF000) {
    case 0x0000:
        switch (opcode & 0x000F) {
        case 0x0000: // CLS
            for (int i = 0; i < 2048; i++)
                gfx[i] = 0;
            draw_flag = true;
            pc += 2;
            break;
        case 0x000E: // RET
            pc = stack[sp];
            sp--;
            pc += 2;
            break;
        default:
            std::cerr << "Unimplemented opcode: 0x" << std::hex << opcode << std::endl;
            exit(1);
        }
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
        if (V[x] == (opcode & 0x00FF))
            pc += 2;
        pc += 2;
        break;

    case 0x4000: // Skip if Vx != kk
        if (V[x] != (opcode & 0x00FF))
            pc += 2;
        pc += 2;
        break;

    case 0x5000: // Skip if Vx == Vy
        if (V[x] == V[y])
            pc += 2;
        pc += 2;
        break;

    case 6000: // Set Vx = kk
        V[x] = opcode & 0x00FF;
        pc += 2;
        break;

    case 7000: // ADD Vx += kk
        V[x] += opcode & 0x00FF;
        pc += 2;
        break;

    case 8000: // 8xy_
        switch (opcode & 0x000F) {
        case 0x0000: // Vx = Vy
            V[x] = V[y];
            pc += 2;
            break;
        case 0x0001: // Vx = Vx OR Vy
            V[x] |= V[y];
            pc += 2;
            break;
        case 0x0002: // Vx = Vx AND Vy
            V[x] &= V[y];
            pc += 2;
            break;
        case 0x0003: // Vx = Vx XOR Vy
            V[x] ^= V[y];
            pc += 2;
            break;
        case 0x0004: // Vx = Vx + Vy, set VF = carry
            if (V[y] > (0xFF - V[x]))
                V[0xF] = 1;
            else
                V[0xF] = 0;
            V[x] += V[y];
            pc += 2;
            break;
        case 0x0005: // Set Vx = Vx - Vy, set VF = NOT borrow
            if (V[x] > V[y])
                V[0xF] = 1;
            else
                V[0xF] = 0;
            V[x] -= V[y];
            pc += 2;
            break;
        case 0x0006: // Set Vx = Vx SHR 1
            V[0xF] = V[x] & 1;
            V[x] >>= 1; // same as /= 2
            pc += 2;
            break;
        case 0x0007: // Set Vx = Vy - Vx, set VF = NOT borrow
            if (V[y] > (V[x]))
                V[0xF] = 1;
            else
                V[0xF] = 0;
            V[x] = V[y] - V[x];
            pc += 2;
            break;
        case 0x000E: // Vx = Vx SHL 1
            V[0xF] = V[x] >> 7;
            V[x] <<= 1;
            pc += 2;
            break;
        default:
            std::cerr << "Unimplemented opcode: 0x" << std::hex << opcode << std::endl;
            exit(1);
        }
        break;

    case 0x9000: // Skip next instruction if Vx != Vy
        if (V[x] != V[y])
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
        V[x] = (rand() % 256) & (opcode & 0x00FF);
        pc += 2;
        break;

    case 0xD000: { // Draw n at (Vx, Vy)
        unsigned short height = opcode & 0x000F;
        unsigned short pixel;

        V[0xF] = 0;
        for (int yline = 0; yline < height; yline++) {
            pixel = memory[I + yline];
            for (int xline = 0; xline < 8; xline++) {
                if ((pixel & (0x80 >> xline)) != 0) {
                    if (gfx[(V[x] + xline + ((V[y] + yline) * 64))] == 1)
                        V[0xF] = 1;
                    gfx[V[x] + xline + ((V[y] + yline) * 64)] ^= 1;
                }
            }
        }

        draw_flag = true;
        pc += 2;
    } break;

    case 0xE000: // Ex__
        switch (opcode & 0x00FF) {
        case 0x009E: // SKP if keypad[Vx] is pressed
            if (key[V[x]] != 0)
                pc += 2;
            pc += 2;
            break;
        case 0x00A1: // SKP if keypad[Vx] is not pressed
            if (key[V[x]] == 0)
                pc += 2;
            pc += 2;
            break;
        default:
            std::cerr << "Unimplemented opcode: 0x" << std::hex << opcode << std::endl;
            exit(1);
        }
        break;

    case 0xF000: // Fx__
        switch (opcode & 0x00FF) {
        case 0x0007: // Set Vx = delay timer
            V[x] = delay_timer;
            pc += 2;
            break;
        case 0x000A: // Wait for key press, store key in Vx
            for (int i = 0; i < 16; i++) {
                if (key[i] != 0) {
                    V[x] = key[i];
                    pc += 2;
                    break;
                }
            }
            break;
        case 0x0015: // Set delay timer = Vx
            delay_timer = V[x];
            pc += 2;
            break;
        case 0x0018: // Set sound timer = Vx
            sound_timer = V[x];
            pc += 2;
            break;
        case 0x001E: // ADD I, Vx
            I += V[x];
            pc += 2;
            break;
        case 0x0029: // Set I = location of sprite for digit Vx.
            for (int i = 0; i < 16; i++) {
                if (fontset[i] == V[x])
                    I = i;
            }
            pc += 2;
            break;
        case 0x0033: // LD B, Vx
            memory[I] = V[x] / 100;
            memory[I + 1] = (V[x] / 10) % 10;
            memory[I + 2] = (V[x] % 100) % 10;
            pc += 2;
            break;
        case 0x0055: // Store registers V0 through Vx in memory starting at location I.
            for (int i = 0; i <= (x); i++) {
                memory[I + i] = V[i];
            }
            pc += 2;
            break;
        case 0x0065: // Read registers V0 through Vx from memory starting at location I.
            for (int i = 0; i <= (x); i++) {
                V[i] = memory[I + i];
            }
            pc += 2;
            break;
        default:
            std::cerr << "Unimplemented opcode: 0x" << std::hex << opcode << std::endl;
            exit(1);
        }
        break;

    default:
        std::cerr << "Unimplemented opcode: 0x" << std::hex << opcode << std::endl;
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