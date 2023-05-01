#include <iostream>
#include <string>
#include "SDL.h"
#include "include/chip8.h"



int main(int argc, char* args[]) {
    chip8 c8(args[1]);
    while(c8.running) {
        c8.handleEvents();
        c8.update();
        c8.render();
        SDL_Delay(10);
    }

    return 0;
}
