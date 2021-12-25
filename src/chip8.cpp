#include <fstream>
#include <iostream>

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
    for (int i = 0; i < 16; i++)
    {
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
}

bool Chip8::load(const char *file_path)
{
    using namespace std;

    cout << "Loading file: " << file_path << endl;

    // Load ROM
    ifstream file(file_path, ios::binary | ios::ate);
    if (!file.is_open())
    {
        cerr << "error: Unable to load ROM" << endl;
        return false;
    }

    // Get file size
    streampos size = file.tellg();
    if (size > (4096 - 0x200))
    {
        cerr << "error: ROM file is too large" << endl;
        return false;
    }

    // Write to memory
    char *buffer;
    for (int i = 0; i < size; i++)
    {
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
    switch (opcode & 0xF000)
    {
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
        switch (opcode & 0x000F)
        {
        case 0x0000: // Vx = Vy
            V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;

        case 0x0001: // Vx = Vx OR Vy
            V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;

        case 0x0002:
            break;

        case 0x0003:
            break;

        case 0x0004:
            break;

        case 0x0005:
            break;

        case 0x0006:
            break;

        case 0x0007:
            break;

        case 0x000E:
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

    if (sound_timer > 0)
    {
        if (sound_timer == 1)
            std::cout << "BEEP!" << std::endl;
        sound_timer--;
    }
}