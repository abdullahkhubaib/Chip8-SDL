#include <string>
#include "../include/chip8.h"

chip8::chip8() = default;

chip8::~chip8() = default;

void chip8::init(const std::string& fName) {
    SDL_Init(SDL_INIT_EVERYTHING);
    window = SDL_CreateWindow(("Chip 8 - " + fName).c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          64 * SCALE_RATIO, 32 * SCALE_RATIO, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
}


