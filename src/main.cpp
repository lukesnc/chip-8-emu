#include "chip8.h"

int main() {
    Chip8 myChip8;

    myChip8.init();
    bool status = myChip8.load("../roms/test_opcode.ch8");

    return 0;
}