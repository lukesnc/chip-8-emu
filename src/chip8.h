#pragma once

class Chip8 {
private:
    unsigned short opcode; // Current op code

    unsigned char memory[4096]; // 4k memory
    unsigned char V[16];        // CPU registers

    unsigned short I;  // Index register
    unsigned short pc; // Program counter

    unsigned char delay_timer; // Delay timer
    unsigned char sound_timer; // Sound timer

    unsigned short stack[16]; // CPU stack
    unsigned short sp;        // Stack pointer

    void init();

public:
    unsigned char gfx[64 * 32]; // Display
    unsigned char key[16];      // Keypad
    bool draw_flag;             // Update the screen?

    void exec();                      // One emulation cycle
    bool load(const char* file_path); // Load rom
};