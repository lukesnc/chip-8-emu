# chip-8-emu
A CHIP-8 emulator in C++

## Requirements
Debian/Ubuntu
```bash
sudo apt install cmake libsdl2-dev
```
Arch
```bash
sudo pacman -S cmake sdl2
```

## Build & Run
Compile
```bash
mkdir build
cd build
cmake ..
make
```
Run
```bash
./chip8 [ROM]
```

## Resources
- https://multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/
- http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
