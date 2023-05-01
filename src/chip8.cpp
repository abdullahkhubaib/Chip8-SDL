#include <string>
#include "../include/chip8.h"

chip8::chip8(const std::string& fName) : reg(), stack(), prev_frame(), frame_buffer(), mem() {
    index = 0;
    pc = 0x200;
    key = 0;
    sp = -1;
    dt = 0;
    st = 0;
    running = false;
    std::ifstream file(fName, std::ios::binary);
    // Get file size
    file.seekg(0, std::ios::end);
    auto size = file.tellg();
    file.seekg(0, std::ios::beg);
    if(size > 0xDFF) {
        std::cerr << "File too large" << std::endl;
        exit(1);
    }
    // Load file into memory starting at offset 0x200
    char c;
    for(int i = 0; file.get(c); i++)
        mem[0x200 + i] = c;

    SDL_Init(SDL_INIT_EVERYTHING);
    window = SDL_CreateWindow(("Chip 8 - " + fName).c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              64 * SCALE_RATIO, 32 * SCALE_RATIO, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_RenderSetScale(renderer, SCALE_RATIO, SCALE_RATIO);
    running = true;

    // Load character sprites into memory.
    for(int i = 0; i < 80; i++)
        mem[0x50 + i] = characters[i / 5][i % 5];

    // Initialize the RNG.
    rng = std::mt19937(std::random_device()());
    rand = std::uniform_int_distribution<std::mt19937::result_type>(0, 255);

}

chip8::~chip8() {
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
}

// 16x5 byte array containing the sprites for the characters 0 - F.
const uint8_t chip8::characters[16][5] = {{0xF0, 0x90, 0x90, 0x90, 0xF0}, {0x20, 0x60, 0x20, 0x20, 0x70},  // 0, 1
                                          {0xF0, 0x10, 0xF0, 0x80, 0xF0}, {0xF0, 0x10, 0xF0, 0x10, 0xF0},  // 2, 3
                                          {0x90, 0x90, 0xF0, 0x10, 0x10}, {0xF0, 0x80, 0xF0, 0x10, 0xF0},  // 4, 5
                                          {0xF0, 0x80, 0xF0, 0x90, 0xF0}, {0xF0, 0x10, 0x20, 0x40, 0x40},  // 6, 7
                                          {0xF0, 0x90, 0xF0, 0x90, 0xF0}, {0xF0, 0x90, 0xF0, 0x10, 0xF0},  // 8, 9
                                          {0xF0, 0x90, 0xF0, 0x90, 0x90}, {0xE0, 0x90, 0xE0, 0x90, 0xE0},  // A, B
                                          {0xF0, 0x80, 0x80, 0x80, 0xF0}, {0xE0, 0x90, 0x90, 0x90, 0xE0},  // C, D
                                          {0xF0, 0x80, 0xF0, 0x80, 0xF0}, {0xF0, 0x80, 0xF0, 0x80, 0x80}}; // E, F

void chip8::handleEvents() {
    SDL_Event event;
    SDL_PollEvent(&event);

    switch (event.type) {
        case SDL_QUIT:
            running = false;
            break;
        case SDL_KEYDOWN:
            if(event.key.keysym.sym >= SDLK_0 && event.key.keysym.sym <= SDLK_9)
                key |= 1 << (event.key.keysym.sym - SDLK_0);
            else if(event.key.keysym.sym >= SDLK_a && event.key.keysym.sym <= SDLK_f)
                key |= 1 << (event.key.keysym.sym - SDLK_a + 10);
            break;
        case SDL_KEYUP:
            if(event.key.keysym.sym >= SDLK_0 && event.key.keysym.sym <= SDLK_9)
                key &= ~(1 << (event.key.keysym.sym - SDLK_0));
            else if(event.key.keysym.sym >= SDLK_a && event.key.keysym.sym <= SDLK_f)
                key &= ~(1 << (event.key.keysym.sym - SDLK_a + 10));
            break;
        default:
            break;
    }


}

void chip8::update() {
    uint16_t opcode = (mem[pc] << 8) | mem[pc + 1];
    pc += 2;
    switch(opcode & 0xF000) {
        case 0x0000:
            if(opcode == 0x00E0) {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderClear(renderer);
            } else if(opcode == 0x00EE) {
                if(sp < 0) {
                    std::cout << "Illegal return at PC: " << pc << std::endl;
                    exit(1);
                }
                pc = stack[sp--];
            } else
                invalid_opcode(opcode);
            break;
        case 0x1000:
            pc = opcode & 0x0FFF;
            break;
        case 0x2000:
            if(sp > 0xF) {
                std::cout << "Stack Overflow at PC: " << pc << std::endl;
                exit(1);
            }
            stack[++sp] = pc;
            pc = opcode & 0x0FFF;
            break;
        case 0x3000:
            if(reg[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
                pc += 2;
            break;
        case 0x4000:
            if(reg[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
                pc += 2;
            break;
        case 0x5000:
            if(opcode & 0x000F)
                invalid_opcode(opcode);
            if(reg[(opcode & 0x0F00) >> 8] == reg[(opcode & 0x00F0) >> 4])
                pc += 2;
            break;
        case 0x6000:
            reg[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
            break;
        case 0x7000:
            reg[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
            break;
        case 0x8000:
            switch(opcode & 0x000F) {
                case 0x0000:
                    reg[(opcode & 0x0F00) >> 8] = reg[(opcode & 0x00F0) >> 4];
                    break;
                case 0x0001:
                    reg[(opcode & 0x0F00) >> 8] = reg[(opcode & 0x0F00) >> 8] | reg[(opcode & 0x00F0) >> 4];
                    break;
                case 0x0002:
                    reg[(opcode & 0x0F00) >> 8] = reg[(opcode & 0x0F00) >> 8] & reg[(opcode & 0x00F0) >> 4];
                    break;
                case 0x0003:
                    reg[(opcode & 0x0F00) >> 8] = reg[(opcode & 0x0F00) >> 8] ^ reg[(opcode & 0x00F0) >> 4];
                    break;
                case 0x0004:
                    if(reg[(opcode & 0x0F00) >> 8] + reg[(opcode & 0x00F0) >> 4] > 255)
                        reg[0xF] = 1;
                    reg[(opcode & 0x0F00) >> 8] = reg[(opcode & 0x0F00) >> 8] + reg[(opcode & 0x00F0) >> 4];
                    break;
                case 0x0005:
                    reg[0xF] = (reg[(opcode & 0x0F00) >> 8] > reg[(opcode & 0x00F0) >> 4]) ? 1 : 0;
                    reg[(opcode & 0x0F00) >> 8] = reg[(opcode & 0x0F00) >> 8] - reg[(opcode & 0x00F0) >> 4];
                    break;
                case 0x0006:
                    reg[0xF] = reg[(opcode & 0x0F00) >> 8] & 0b00000001;
                    reg[(opcode & 0x0F00) >> 8] >>= 1;
                    break;
                case 0x0007:
                    reg[0xF] = (reg[(opcode & 0x00F0) >> 4] > reg[(opcode & 0x0F00) >> 8]) ? 1 : 0;
                    reg[(opcode & 0x0F00) >> 8] = reg[(opcode & 0x00F0) >> 4] - reg[(opcode & 0x0F00) >> 8];
                    break;
                case 0x000E:
                    reg[0xF] = reg[(opcode & 0x0F00) >> 8] & 0b10000000;
                    reg[(opcode & 0x0F00) >> 8] <<= 1;
                    break;
                default:
                    invalid_opcode(opcode);
            }
            break;
        case 0x9000:
            if(opcode & 0x000F)
                invalid_opcode(opcode);
            if(reg[(opcode & 0x0F00) >> 8] != reg[(opcode & 0x0F00) >> 8])
                pc += 2;
            break;
        case 0xA000:
            index = opcode & 0x0FFF;
            break;
        case 0xB000:
            pc = reg[0] + (opcode & 0x0FFF);
            break;
        case 0xC000:
            reg[(opcode & 0x0F00) >> 8] = rand(rng) & (opcode & 0x00FF);
            break;
        case 0xD000:
            break;
        case 0xE000:
            if((reg[(opcode & 0x0F00) >> 8] > 0x000F) || ((opcode & 0x00FF) != 0x009E && (opcode & 0x00FF) != 0x00A1))
                invalid_opcode(opcode);
            if((opcode & 0x00FF) == 0x009E && (key & (1 << reg[(opcode & 0x0F00) >> 8])))
                pc += 2;
            else if(!(key & (1 << reg[(opcode & 0x0F00) >> 8])))
                pc += 2;
            break;
        case 0xF000:
            switch (opcode & 0x00FF) {
                case 0x0007:
                    reg[(opcode & 0x0F00) >> 8] = dt;
                    break;
                case 0x0015:
                    dt = reg[(opcode & 0x0F00) >> 8];
                    break;
                case 0x0018:
                    st = reg[(opcode & 0x0F00) >> 8];
                    break;
                case 0x001E:
                    if(index + reg[(opcode & 0x0F00) >> 8] > 0x0FFF)
                        reg[0xF] = 1;
                    index += reg[(opcode & 0x0F00) >> 8];
                    break;
                case 0x000A:
                    if(!(key & (1 << reg[(opcode & 0x0F00) >> 8])))
                        pc -= 2;
                    break;
                case 0x0029:
                    index = 0x050 + (5 * (reg[(opcode & 0x0F00) >> 8] & 0x000F));
                    break;
                case 0x0033: {
                    uint8_t num = reg[(opcode & 0x0F00) >> 8];
                    mem[index + 2] = num % 10;
                    num /= 10;
                    mem[index + 1] = num % 10;
                    num /= 10;
                    mem[index] = num;
                    break;
                }
                case 0x0055:
                    for(int i = 0, k = (opcode & 0x0F00) >> 8; i <= k; i++)
                        mem[index + i] = reg[i];
                    break;
                case 0x0065:
                    for(int i = 0, k = (opcode & 0x0F00) >> 8; i <= k; i++)
                        reg[i] = mem[index + i];
                    break;
                default:
                    invalid_opcode(opcode);
            }
            break;
        default:
            invalid_opcode(opcode);
    }

}

void chip8::render() {
    if(prev_frame == frame_buffer)
        return;
    prev_frame = frame_buffer;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for(int y = 0; y < 32; y++) {
        for(int x = 0; x < 64; x++) {
            SDL_Rect rect = {x, y, 1, 1};
            if(frame_buffer[y] & (1ULL << (63ULL - x)))
                SDL_RenderFillRect(renderer, &rect);
        }
    }
    SDL_RenderPresent(renderer);
}

void chip8::clear_buffer() {
    for(int i = 0; i < 32; i++)
        frame_buffer[i] = 0;
}

void chip8::set_pixel(int x, int y) {
    frame_buffer[y] |= 1ULL << (63ULL - x);
}

void chip8::invalid_opcode(uint16_t opcode) {
    std::cerr << "Invalid opcode " << opcode << std::endl;
    exit(1);
}


