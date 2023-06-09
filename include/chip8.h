
#ifndef CHIP8_CHIP8_H
#define CHIP8_CHIP8_H
#include <cstdint>
#include <array>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <bitset>
#include <mutex>
#include <thread>
#include <chrono>
#include <future>
#include <windows.h>
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
    std::mutex t_mutex;
    std::thread t_thread;
    uint16_t index;
    uint16_t pc;
    uint8_t sp;
    uint8_t dt;
    uint8_t st;
    std::mt19937 rng;
    std::uniform_int_distribution<std::mt19937::result_type> rand;
    void invalid_opcode() const;
public:
    bool running;
    SDL_Window* window;
    SDL_Renderer* renderer;
    chip8(const std::string& fName);
    ~chip8();
    void handleEvents();
    void update();
    void render();
    static void error(const std::string& message);
};


#endif //CHIP8_CHIP8_H
