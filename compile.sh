#!/bin/bash
g++ -o chip8 src/main.cpp src/chip8.cpp $(pkg-config --cflags --libs sdl2)
