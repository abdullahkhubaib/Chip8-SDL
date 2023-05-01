
#ifndef CHIP8_CHIP8_H
#define CHIP8_CHIP8_H
#include <cstdint>
#include <array>
#include <fstream>
#include <iostream>
#include "SDL.h"
#define SCALE_RATIO 16

class chip8 {
private:
    std::array<uint8_t, 16> reg;
    uint16_t index;
    uint16_t pc;
    std::array<uint8_t, 64> stack;
    uint8_t sp;
    uint8_t dt;
    uint8_t st;
    std::array<uint64_t, 32> frame_buffer;
    std::array<uint8_t, 4096> mem;
    static const uint8_t characters[16][5];
    bool running;
    SDL_Window* window;
    SDL_Renderer* renderer;
public:
    chip8();
    ~chip8();
    void init(const std::string& fName);
};


#endif //CHIP8_CHIP8_H
