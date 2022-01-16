#include <SDL2/SDL.h>
#include <chrono>
#include <iostream>
#include <thread>

#include "chip8.h"

#define CYCLE 16666 // 60hz
#define WIDTH 1024
#define HEIGHT 512

int main(int argc, char** argv)
{
    if (argc != 2) {
        std::cout << "Usage: ./chip8 [ROM]" << std::endl;
        return 1;
    }

    Chip8 myChip8 = Chip8();

    // Create window
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("CHIP-8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_RenderSetLogicalSize(renderer, WIDTH, HEIGHT);

    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);

    unsigned int frame_buffer[64 * 32];

    // Load rom
    if (!myChip8.load(argv[1]))
        return 1;

    // Emulation cycle
    for (;;) {
        myChip8.exec();

        // Check keyboard
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                std::cout << "Exiting..." << std::endl;
                SDL_DestroyTexture(texture);
                SDL_DestroyRenderer(renderer);
                SDL_DestroyWindow(window);
                SDL_Quit();
                return 0;
            }
        }

        // Update screen
        if (myChip8.draw_flag) {
            myChip8.draw_flag = false;

            // Copy screen into frame buffer
            for (int i = 0; i < 2048; i++)
                frame_buffer[i] = (0x00FFFFFF * myChip8.gfx[i]) | 0xFF000000;

            SDL_UpdateTexture(texture, NULL, frame_buffer, 64 * sizeof(unsigned int));

            // Render
            SDL_RenderClear(renderer); // Clear screen
            SDL_RenderCopy(renderer, texture, NULL, NULL);
            SDL_RenderPresent(renderer);
        }

        std::this_thread::sleep_for(std::chrono::microseconds(CYCLE));
    }

    return 0;
}