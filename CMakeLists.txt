cmake_minimum_required(VERSION 3.10)

# set the project name
project(CHIP-8)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# add the executable
add_executable(chip8 src/main.cpp src/chip8.cpp)
target_link_libraries(chip8 ${SDL2_LIBRARIES})
