#include <iostream>
#include <string>
#include "SDL.h"
#include "include/chip8.h"



int main(int argc, char* args[]) {
    std::cout << "Hello, World!" << std::endl;
    std::string title = "Chip-8 ";

    chip8 c8;
    c8.init(args[1]);

    SDL_Delay(3000);
    return 0;
}
