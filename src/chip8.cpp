#include <string>
#include "../include/chip8.h"

chip8::chip8() : reg(), stack(), frame_buffer(), mem() {
    index = 0;
    pc = 0;
    sp = 0;
    dt = 0;
    st = 0;
    running = false;
}

chip8::~chip8() = default;

// 16x5 byte array containing the sprites for the characters 0 - F.
const uint8_t chip8::characters[16][5] = {{0xF0, 0x90, 0x90, 0x90, 0xF0}, {0x20, 0x60, 0x20, 0x20, 0x70},  // 0, 1
                                          {0xF0, 0x10, 0xF0, 0x80, 0xF0}, {0xF0, 0x10, 0xF0, 0x10, 0xF0},  // 2, 3
                                          {0x90, 0x90, 0xF0, 0x10, 0x10}, {0xF0, 0x80, 0xF0, 0x10, 0xF0},  // 4, 5
                                          {0xF0, 0x80, 0xF0, 0x90, 0xF0}, {0xF0, 0x10, 0x20, 0x40, 0x40},  // 6, 7
                                          {0xF0, 0x90, 0xF0, 0x90, 0xF0}, {0xF0, 0x90, 0xF0, 0x10, 0xF0},  // 8, 9
                                          {0xF0, 0x90, 0xF0, 0x90, 0x90}, {0xE0, 0x90, 0xE0, 0x90, 0xE0},  // A, B
                                          {0xF0, 0x80, 0x80, 0x80, 0xF0}, {0xE0, 0x90, 0x90, 0x90, 0xE0},  // C, D
                                          {0xF0, 0x80, 0xF0, 0x80, 0xF0}, {0xF0, 0x80, 0xF0, 0x80, 0x80}}; // E, F

void chip8::init(const std::string& fName) {
    std::ifstream file(fName, std::ios::binary);
    // Get file size
    file.seekg(0, std::ios::end);
    auto size = file.tellg();
    file.seekg(0, std::ios::beg);
    if(size > 0xDFF) {
        std::cerr << "File too large" << std::endl;
        exit(1);
    }
    // Load file into memory at 0x200
    int i = 0;
    char c;
    while(file.get(c))
        mem[0x200 + i++] = c;

    SDL_Init(SDL_INIT_EVERYTHING);
    window = SDL_CreateWindow(("Chip 8 - " + fName).c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          64 * SCALE_RATIO, 32 * SCALE_RATIO, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);


}


