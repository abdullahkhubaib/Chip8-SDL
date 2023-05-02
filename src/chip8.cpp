
#include "../include/chip8.h"

chip8::chip8(const std::string& fName) : V(), stack(), prev_frame(), frame_buffer(), mem(), key(0) {
    index = 0;
    pc = 0x200;
    sp = 0;
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
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_RenderSetScale(renderer, SCALE_RATIO, SCALE_RATIO);
    running = true;

    // Load character sprites into memory.
    for(int i = 0; i < 80; i++)
        mem[0x50 + i] = characters[i / 5][i % 5];

    // Initialize the RNG.
    rng = std::mt19937(std::random_device()());
    rand = std::uniform_int_distribution<std::mt19937::result_type>(0, 255);

    t_thread = std::thread([this]() {
        while(true) {
            t_mutex.lock();
            if(dt > 0)
                dt--;
            if(st > 0) {
                st--;
                if(st == 0)
                    Beep(523, 100);
            }
            t_mutex.unlock();
            std::this_thread::sleep_for(std::chrono::nanoseconds(16666666)); // 60 Hz
        }
    });


}

chip8::~chip8() {
    t_thread.detach();
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
                key[event.key.keysym.sym - SDLK_0] = true;
            else if(event.key.keysym.sym >= SDLK_a && event.key.keysym.sym <= SDLK_f)
                key[event.key.keysym.sym - SDLK_a + 10] = true;
            break;
        case SDL_KEYUP:
            if(event.key.keysym.sym >= SDLK_0 && event.key.keysym.sym <= SDLK_9)
                key[event.key.keysym.sym - SDLK_0] = false;
            else if(event.key.keysym.sym >= SDLK_a && event.key.keysym.sym <= SDLK_f)
                key[event.key.keysym.sym - SDLK_a + 10] = false;
            break;
        default:
            break;
    }


}

void chip8::update() {
    const uint16_t opcode = (mem[pc] << 8) | mem[pc + 1];
    pc += 2;
    uint8_t& Vx = V[(opcode & 0x0F00) >> 8];
    uint8_t& Vy = V[(opcode & 0x00F0) >> 4];
    switch(opcode & 0xF000) {
        case 0x0000:
            if(opcode == 0x00E0) {
                for(uint64_t& i: frame_buffer)
                    i = 0;
            } else if(opcode == 0x00EE) {
                if(sp < 0) {
                    std::cout << "Illegal return at PC: " << pc << std::endl;
                    exit(1);
                }
                pc = stack[--sp];
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
            stack[sp++] = pc;
            pc = opcode & 0x0FFF;
            break;
        case 0x3000:
            if(Vx == (opcode & 0x00FF))
                pc += 2;
            break;
        case 0x4000:
            if(Vx != (opcode & 0x00FF))
                pc += 2;
            break;
        case 0x5000:
            if(opcode & 0x000F)
                invalid_opcode(opcode);
            if(Vx == Vy)
                pc += 2;
            break;
        case 0x6000:
            Vx = opcode & 0x00FF;
            break;
        case 0x7000:
            Vx += opcode & 0x00FF;
            break;
        case 0x8000:
            switch(opcode & 0x000F) {
                case 0x0000:
                    Vx = Vy;
                    break;
                case 0x0001:
                    Vx = Vx | Vy;
                    V[0xF] = 0;
                    break;
                case 0x0002:
                    Vx = Vx & Vy;
                    V[0xF] = 0;
                    break;
                case 0x0003:
                    Vx = Vx ^ Vy;
                    V[0xF] = 0;
                    break;
                case 0x0004:
                    if(Vx + Vy > 255)
                        V[0xF] = 1;
                    Vx = Vx + Vy;
                    break;
                case 0x0005:
                    V[0xF] = (Vx > Vy) ? 1 : 0;
                    Vx = Vx - Vy;
                    break;
                case 0x0006:
                    V[0xF] = Vx & 0b00000001;
                    Vx >>= 1;
                    break;
                case 0x0007:
                    V[0xF] = (Vy > Vx) ? 1 : 0;
                    Vx = Vy - Vx;
                    break;
                case 0x000E:
                    V[0xF] = Vx & 0b10000000;
                    Vx <<= 1;
                    break;
                default:
                    invalid_opcode(opcode);
            }
            break;
        case 0x9000:
            if(opcode & 0x000F)
                invalid_opcode(opcode);
            if(Vx != Vy)
                pc += 2;
            break;
        case 0xA000:
            index = opcode & 0x0FFF;
            break;
        case 0xB000:
            pc = V[0] + (opcode & 0x0FFF);
            break;
        case 0xC000:
            Vx = rand(rng) & (opcode & 0x00FF);
            break;
        case 0xD000: {
            const int x = Vx & 0b00111111;
            const int y = Vy & 0b00011111;
            const int n = opcode & 0x000F;
            V[0xF] = 0;
            for(int i = 0; i < n && (i + y) < 32; i++) {
                uint64_t mask;
                if(x < 56)
                    mask = ((uint64_t) mem[i + index]) << (56ULL - x);
                else
                    mask = ((uint64_t) mem[i + index]) >> (x - 56ULL);
                frame_buffer[y + i] ^= mask;
                V[0xF] = ((frame_buffer[y + i] & mask) == mask) ? 1 : 0;
            }

            break;
        }
        case 0xE000:
            if((Vx > 0x000F) || ((opcode & 0x00FF) != 0x009E && (opcode & 0x00FF) != 0x00A1))
                invalid_opcode(opcode);
            if(((opcode & 0x00FF) == 0x009E && key[Vx]) || ((opcode & 0x00FF) == 0x00A1 && !key[Vx]))
                pc += 2;
            break;
        case 0xF000:
            switch (opcode & 0x00FF) {
                case 0x0007:
                    t_mutex.lock();
                    Vx = dt;
                    t_mutex.unlock();
                    break;
                case 0x0015:
                    t_mutex.lock();
                    dt = Vx;
                    t_mutex.unlock();
                    break;
                case 0x0018:
                    t_mutex.lock();
                    st = Vx;
                    t_mutex.unlock();
                    break;
                case 0x001E:
                    if(index + Vx > 0x0FFF)
                        V[0xF] = 1;
                    index += Vx;
                    break;
                case 0x000A:
                    if(key.none())
                        pc -= 2;
                    else for(int i = 0; i < 16; i++) {
                        if(key[i]) {
                            Vx = i;
                            break;
                        }
                    }
                    break;
                case 0x0029:
                    index = 0x050 + (5 * (Vx & 0x000F));
                    break;
                case 0x0033: {
                    uint8_t num = Vx;
                    mem[index + 2] = num % 10;
                    num /= 10;
                    mem[index + 1] = num % 10;
                    num /= 10;
                    mem[index] = num;
                    break;
                }
                case 0x0055:
                    for(int i = 0, k = (opcode & 0x0F00) >> 8; i <= k; i++)
                        mem[index + i] = V[i];
                    break;
                case 0x0065:
                    for(int i = 0, k = (opcode & 0x0F00) >> 8; i <= k; i++)
                        V[i] = mem[index + i];
                    break;
                default:
                    invalid_opcode(opcode);
            }
            break;
        default:
            invalid_opcode(opcode);
    }
    asm("nop");
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

void chip8::invalid_opcode(uint16_t opcode) {
    std::cerr << "Invalid opcode " << opcode << std::endl;
    exit(1);
}


