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
    case 0x2000:
        stack[sp] = pc;
        sp++;
        pc = opcode & 0x0FFF;
        break;
    default:
        std::cout << "Unknown opcode: 0x" << opcode << std::endl;
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