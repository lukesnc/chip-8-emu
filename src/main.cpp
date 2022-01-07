#include <chrono>
#include <iostream>
#include <thread>

#include <SDL2/SDL.h>

#include "chip8.h"

#define CYCLE (1000 / 60) // 60 Hz in ms
#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 512

int main(int argc, char** argv)
{
    if (argc != 2) {
        std::cout << "Usage: ./chip8 [ROM]" << std::endl;
        return 1;
    }

    Chip8 myChip8;
    myChip8.init();

    // Create window
    SDL_Window* window = NULL;
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow("CHIP-8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    // Load rom
    if (!myChip8.load(argv[1]))
        return 1;

    // Emulation cycle
    for (;;) {
        myChip8.emulate_cycle();

        std::this_thread::sleep_for(std::chrono::milliseconds(CYCLE));
    }

    return 0;
}