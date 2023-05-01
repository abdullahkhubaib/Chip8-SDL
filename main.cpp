#include <iostream>
#include "SDL.h"
#include "include/chip8.h"

#define SCALE_RATIO 16

int main(int argc, char* args[]) {
    std::cout << "Hello, World!" << std::endl;

    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window* window = SDL_CreateWindow("Chip-8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          64 * SCALE_RATIO, 32 * SCALE_RATIO, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);


    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
    SDL_Delay(3000);

    return 0;
}
