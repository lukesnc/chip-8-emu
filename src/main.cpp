#include <iostream>
#include <chrono>
#include <thread>

#include "chip8.h"

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cout << "Usage: chip8 [ROM]" << std::endl;
        return 1;
    }

    Chip8 myChip8;
    myChip8.init();

    if (!myChip8.load(argv[1]))
        return 1;

    for (;;)
    {
        myChip8.emulate_cycle();

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    return 0;
}