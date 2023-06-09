#include <iostream>
#include <string>
#include "SDL.h"
#include "include/chip8.h"

#define INSTRUCTIONS_PER_SECOND 700

int main(int argc, char* args[]) {
    if(argc != 2)
        chip8::error("Usage: Drag a file onto the executable or \n run chip8 <filename>");
    const int frameDelay = 1000 / INSTRUCTIONS_PER_SECOND;
    uint32_t frameStart = 0;
    uint32_t frameTime = 0;
    chip8 c8(args[1]);
    while(c8.running) {
        frameStart = SDL_GetTicks();
        c8.handleEvents();
        c8.update();
        c8.render();
        frameTime = SDL_GetTicks() - frameStart;
        if(frameDelay > frameTime)
            SDL_Delay(frameDelay - frameTime);
    }

    return 0;
}
