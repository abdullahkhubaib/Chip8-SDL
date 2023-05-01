
#ifndef CHIP8_CHIP8_H
#define CHIP8_CHIP8_H
#include <cstdint>
#include <array>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <bitset>
#include "SDL.h"
#define SCALE_RATIO 16

class chip8 {
private:
    std::array<uint8_t, 16> V;
    std::array<uint16_t, 16> stack;
    std::array<uint64_t, 32> prev_frame;
    std::array<uint64_t, 32> frame_buffer;
    std::array<uint8_t, 4096> mem;
    static const uint8_t characters[16][5];
    std::bitset<16> key;
    uint16_t index;
    uint16_t pc;
    uint8_t sp;
    uint8_t dt;
    uint8_t st;
    SDL_Window* window;
    SDL_Renderer* renderer;
    void clear_buffer();
    void set_pixel(int x, int y, bool b);
    void toggle_pixel(int x, int y);
    void invalid_opcode(uint16_t opcode);

    std::mt19937 rng;
    std::uniform_int_distribution<std::mt19937::result_type> rand;

public:
    bool running;
    chip8(const std::string& fName);
    ~chip8();
    void handleEvents();
    void update();
    void render();
};


#endif //CHIP8_CHIP8_H
