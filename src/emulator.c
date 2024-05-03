#include <emulator/emulator.h>
// font that standard chip8 uses
uint8_t emulatorFont[80] = {0xF0, 0x90, 0x90, 0x90, 0xF0,  // 0
                            0x20, 0x60, 0x20, 0x20, 0x70,  // 1
                            0xF0, 0x10, 0xF0, 0x80, 0xF0,  // 2
                            0xF0, 0x10, 0xF0, 0x10, 0xF0,  // 3
                            0x90, 0x90, 0xF0, 0x10, 0x10,  // 4
                            0xF0, 0x80, 0xF0, 0x10, 0xF0,  // 5
                            0xF0, 0x80, 0xF0, 0x90, 0xF0,  // 6
                            0xF0, 0x10, 0x20, 0x40, 0x40,  // 7
                            0xF0, 0x90, 0xF0, 0x90, 0xF0,  // 8
                            0xF0, 0x90, 0xF0, 0x10, 0xF0,  // 9
                            0xF0, 0x90, 0xF0, 0x90, 0x90,  // A
                            0xE0, 0x90, 0xE0, 0x90, 0xE0,  // B
                            0xF0, 0x80, 0x80, 0x80, 0xF0,  // C
                            0xE0, 0x90, 0x90, 0x90, 0xE0,  // D
                            0xF0, 0x80, 0xF0, 0x80, 0xF0,  // E
                            0xF0, 0x80, 0xF0, 0x80, 0x80}; // F

void startEmulator(Emulator *em)
{
    for (int i = 0; i < 4096; i++)
    {
        em->memory[i] = 0;
    }
    for (int i = 0; i < 32; i++)
    {
        em->stack[i] = 0;
    }
    for (int i = 0; i < 80; i++)
    {
        em->memory[0x50 + i] = emulatorFont[i]; // storing fonts from 0x50–0x90 which is the most popular place to store it
    }
    SDL_Init(SDL_INIT_EVERYTHING);
    em->window = SDL_CreateWindow("Chip-8 Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 320, SDL_WINDOW_ALLOW_HIGHDPI);
    if (em->window == NULL)
    {
        printf("Could not create window %s \n", SDL_GetError());
    }
    em->rend = SDL_CreateRenderer(em->window, -1, SDL_RENDERER_ACCELERATED);
    SDL_RenderSetLogicalSize(em->rend, 64, 32);     // scaling by a factor of 10
    SDL_SetRenderDrawColor(em->rend, 0, 0, 0, 255); // Black background
    int8_t checkPlay = 0;
    int8_t quit = 0;

    while (!quit)
    {
        // Handle events on queue
        while (SDL_PollEvent(&em->windowEvent) != 0)
        {
            // User requests quit
            if (em->windowEvent.type == SDL_QUIT)
            {
                quit = 1;
            }
        }
        if (!checkPlay) // using this since later on ill allow user to select a file if nothing is loaded in
        {
            startProgram(em, "testwflag.ch8");
            printMemory(em);
            checkPlay = 1;
        }
        else
        {
            uint16_t inst = fetch(em);
            decode_execute(em, inst);
        }
    }
    SDL_DestroyWindow(em->window);
    SDL_Quit();
}

void printMemory(Emulator *em)
{
    for (int i = 0; i < 4096; i++)
    {
        printf("%02x ", em->memory[i]);
    }
}

// push on stack from left to right and pop from right to left
void pushStack(Emulator *em, uint16_t pc)
{
    if (em->stackIndex >= 1024)
    {
        printf("ERROR: Stack Overflow, exiting program.");
        exit(0);
    }
    em->stack[em->stackIndex++] = pc;
}

uint16_t popStack(Emulator *em)
{
    if (em->stackIndex == 0)
    {
        printf("ERROR: Nothing in stack, exiting program.");
        exit(0);
    }
    return em->stack[--em->stackIndex];
}

void startProgram(Emulator *em, const char *ROM)
{
    printf("Loading File: %s\n", ROM);
    FILE *f;
    f = fopen(ROM, "rb");
    fseek(f, 0, SEEK_END);                              // go to the end of the file
    long size = ftell(f);                               // get pointer index (size)
    rewind(f);                                          // go back to the beginning of the file
    uint8_t *buffer;                                    // buffer to hold memory (needs to be dynamic since we dont know size before hand)
    buffer = (uint8_t *)malloc(size * sizeof(uint8_t)); // allocate n bytes
    if (buffer == NULL)
    {
        printf("Memory not allocated \n");
        exit(0); // close the whole program might change this later
    }
    fread(buffer, 1, size, f); // read the file into the buffer
    for (int i = 0; i < size; i++)
    {
        em->memory[0x200 + i] = buffer[i]; // storing starting 0x200 which is where the first chip8 interperter stored ROM
    }
    free(buffer);
    em->pc = 0x200; // start in the first address in the ROM
}

uint16_t fetch(Emulator *em)
{
    uint16_t inst = (em->memory[em->pc] << 8) | (em->memory[em->pc + 1]); // storing as a single 2 byte instruction
    if (em->pc < 4096)                                                    // moving to next instruction
    {
        em->pc += 2;
    }
    else // made it to the end of ROM ill make it close the emulator in this case
    {
        exit(0);
    }
    return inst;
}

void decode_execute(Emulator *em, uint16_t inst)
{
    uint8_t firstNib = (inst & 0xF000) >> 12; // first 4 bits of instruction
    switch (firstNib)
    {
    case 0x0:
        if (inst == 0x00E0) // 00E0
        {
            clearScreen(em);
        }
        else if (inst == 0x00EE) // 00EE =modern programs dont implement the other 0x0 functions but ill keep it as else if just incase i do them anyway
        {
            returnCall(em);
        }
        break;
    case 0x1:
        jump(em, inst);
        break;
    case 0x2:
        callNNN(em, inst);
        break;
    case 0x3:
        skipNNEqual(em, inst);
        break;
    case 0x4:
        skipNNNotEqual(em, inst);
        break;
    case 0x5:
        skipXYEqual(em, inst);
        break;
    case 0x6:
        setXNN(em, inst);
        break;
    case 0x7:
        addXNN(em, inst);
        break;
    case 0x8:
        uint8_t leastSig = inst & (0x000F);
        switch (leastSig)
        {
        case 0x0:
            setXY(em, inst);
            break;
        case 0x1:
            setXorY(em, inst);
            break;
        case 0x2:
            setXandY(em, inst);
            break;
        case 0x3:
            setXxorY(em, inst);
            break;
        case 0x4:
            setXplusY(em, inst);
            break;
        case 0x5:
            setXminusY(em, inst);
            break;
        case 0x6:
            setXShiftRightOneY(em, inst, 1); // this is an ambgious inst it changes depending on chip8 version
            break;
        case 0x7:
            setXYminusX(em, inst);
            break;
        case 0xE:
            setXShiftLeftOneY(em, inst, 1); // this is an ambgious inst it changes depending on chip8 version
            break;
        }
        break;
    case 0x9:
        skipXYNotEqual(em, inst);
        break;
    case 0xA:
        setIndexRegisterNNN(em, inst);
        break;
    case 0xD:
        draw(em, inst);
        break;
    case 0xF:
        uint8_t leastHalf = inst & (0x00FF);
        switch (leastHalf)
        {
        case 0x33:
            storeBCD(em, inst);
            break;
        case 0x55:
            storeUntilX(em, inst, 1); // this is an ambgious inst it changes depending on chip8 version
            break;
        case 0x65:
            loadUntilX(em, inst, 1); // this is an ambgious inst it changes depending on chip8 version
            break;
        case 0x1E:
            addIndexRegisterX(em, inst);
            break;
        }
        break;
    default:
        printf("%x is unimplemented.\n", inst);
        break;
    }
}

// Instructions

void clearScreen(Emulator *em) // 00E0
{
    SDL_SetRenderDrawColor(em->rend, 0, 0, 0, 255); // setting color to opaque black
    SDL_RenderClear(em->rend);
    SDL_RenderPresent(em->rend);
}

void jump(Emulator *em, uint16_t inst) // 1NNN
{
    em->pc = inst & 0x0FFF;
}
void setXNN(Emulator *em, uint16_t inst) // 6XNN
{
    em->registers[(inst & 0x0F00) >> 8] = inst & 0x00FF; // store the last 8 bits in register of X which is bit 8->11
}
void addXNN(Emulator *em, uint16_t inst) // 7XNN
{
    em->registers[(inst & 0x0F00) >> 8] += (inst & 0x00FF); // add the last 8 bits to register of X which is bit 8->11 (no overflow for this instruction)
}
void setIndexRegisterNNN(Emulator *em, uint16_t inst) // ANNN
{
    em->ir = inst & 0x0FFF;
}
void draw(Emulator *em, uint16_t inst) // DXYN
{
    /*
    The starting position wraps. x coordinate of 5 is the same as an x of 68 (modulo 64 which is width of screen)
    Same logic for Y coordinate but 32 since height is 32 pixels
    */
    uint8_t xCoor = em->registers[(inst & 0x0F00) >> 8] % 64; // take X and get its register value modulo 64 due to width
    uint8_t yCoor = em->registers[(inst & 0x00F0) >> 4] % 32; // take Y and get its register value module 32 due to height
    uint8_t height = inst & 0x000F;
    em->registers[0xF] = 0;          // we will use vf as a flag so set it to 0 first
    for (int i = 0; i < height; i++) // get the Nth byte starting from top going to bottom
    {
        uint8_t spriteRByte = em->memory[em->ir + i]; // begin at ir then go downwards according to specification

        for (int j = 0; j < 8; j++) // there are 64/8 bytes since width is 64 bits
        {
            uint8_t screenX = (xCoor + j);
            uint8_t screenY = (yCoor + i);
            uint32_t pixel;
            SDL_RenderReadPixels(em->rend, &((SDL_Rect){screenX, screenY, 1, 1}), SDL_PIXELFORMAT_RGBA8888, &pixel, sizeof(pixel));
            uint8_t r, g, b; // since there is only white and black i only need one of these to figure out if its on or off
            SDL_GetRGB(pixel, SDL_GetWindowSurface(em->window)->format, &r, &g, &b);
            uint8_t currPixel = spriteRByte & (0x80 >> j); // start at 1000_0000 then shift right by col ie next 0100_0000 , so this is used to get pixel starting from left to right
            if (currPixel)
            {
                if (r == 1) // if both are one we set pixel to white and vf flag to 1
                {
                    SDL_SetRenderDrawColor(em->rend, 0, 0, 0, 255); // turn off
                    em->registers[0xF] = 1;
                }
                else // if sprite is one but row is black then we set pixel to black
                {
                    SDL_SetRenderDrawColor(em->rend, 255, 255, 255, 255); // turn on
                }
                SDL_RenderDrawPoint(em->rend, screenX, screenY);
            }
        }
        SDL_RenderPresent(em->rend);
    }
}

void skipNNEqual(Emulator *em, uint16_t inst) // 3XNN
{
    uint8_t vx = em->registers[(inst & 0x0F00) >> 8];
    uint8_t NN = (inst & 0x00FF);

    if (vx == NN)
    {
        em->pc = em->pc + 2; // in fetch stage we already did pc + 2 so we can just do it again to skip next inst
    }
}
void skipNNNotEqual(Emulator *em, uint16_t inst) // 4XNN
{
    uint8_t vx = em->registers[(inst & 0x0F00) >> 8];
    uint8_t NN = (inst & 0x00FF);

    if (vx != NN)
    {
        em->pc = em->pc + 2; // in fetch stage we already did pc + 2 so we can just do it again to skip next inst
    }
}
void skipXYEqual(Emulator *em, uint16_t inst) // 5XY0
{
    uint8_t vx = em->registers[(inst & 0x0F00) >> 8];
    uint8_t vy = em->registers[(inst & 0x00F0) >> 4];

    if (vx == vy)
    {
        em->pc = em->pc + 2; // in fetch stage we already did pc + 2 so we can just do it again to skip next inst
    }
}
void skipXYNotEqual(Emulator *em, uint16_t inst) // 9XY0
{
    uint8_t vx = em->registers[(inst & 0x0F00) >> 8];
    uint8_t vy = em->registers[(inst & 0x00F0) >> 4];

    if (vx != vy)
    {
        em->pc = em->pc + 2; // in fetch stage we already did pc + 2 so we can just do it again to skip next inst
    }
}

void callNNN(Emulator *em, uint16_t inst) // 2NNN
{
    pushStack(em, em->pc);    // first store the pc onto the stack
    em->pc = inst & (0x0FFF); // then set pc to NNN
}

void returnCall(Emulator *em) // 00EE
{
    em->pc = popStack(em); // take value from stack and set pc to it
}

void setXY(Emulator *em, uint16_t inst) // 8xy0
{
    em->registers[(inst & 0x0F00) >> 8] = em->registers[(inst & 0x00F0) >> 4];
}
void setXorY(Emulator *em, uint16_t inst) // 8xy1
{
    em->registers[(inst & 0x0F00) >> 8] |= em->registers[(inst & 0x00F0) >> 4];
}
void setXandY(Emulator *em, uint16_t inst) // 8xy2
{
    em->registers[(inst & 0x0F00) >> 8] &= em->registers[(inst & 0x00F0) >> 4];
}
void setXxorY(Emulator *em, uint16_t inst) // 8xy3
{
    em->registers[(inst & 0x0F00) >> 8] ^= em->registers[(inst & 0x00F0) >> 4];
}
void setXplusY(Emulator *em, uint16_t inst) // 8xy4
{
    uint8_t x = (inst & 0x0F00) >> 8;
    uint8_t vx = em->registers[x];
    uint8_t vy = em->registers[(inst & 0x00F0) >> 4];
    uint16_t sum = vx + vy;
    em->registers[x] = sum;
    if (sum > 255)
    {
        em->registers[0xF] = 1;
    }
    else
    {
        em->registers[0xF] = 0;
    }
}
void setXminusY(Emulator *em, uint16_t inst) // 8xy5
{
    uint8_t x = (inst & 0x0F00) >> 8;
    uint8_t vx = em->registers[x];
    uint8_t vy = em->registers[(inst & 0x00F0) >> 4];
    // according to specs if first larger than second flag to 1 otherwise flag to 0
    em->registers[x] = vx - vy;

    if (vx >= vy)
    {
        em->registers[0xF] = 1;
    }
    else
    {
        em->registers[0xF] = 0;
    }
}
void setXYminusX(Emulator *em, uint16_t inst) // 8xy7
{
    uint8_t x = (inst & 0x0F00) >> 8;
    uint8_t vx = em->registers[x];
    uint8_t vy = em->registers[(inst & 0x00F0) >> 4];
    // according to specs if first larger than second flag to 1 otherwise flag to 0
    em->registers[x] = vy - vx;

    if (vy >= vx)
    {
        em->registers[0xF] = 1;
    }
    else
    {
        em->registers[0xF] = 0;
    }
}

void setXShiftRightOneY(Emulator *em, uint16_t inst, uint8_t new) // 8xy6
{
    if (!new)
    {
        setXY(em, inst); // this line is removed for some chip8 implementations
    }
    uint8_t x = (inst & 0x0F00) >> 8;
    uint8_t vx = em->registers[x];
    uint8_t val = vx >> 1;
    uint8_t shifted = (vx) & 0x01;
    em->registers[x] = val;
    em->registers[0xF] = shifted; // setting flag to the bit that will be shifted out
}

void setXShiftLeftOneY(Emulator *em, uint16_t inst, uint8_t new) // 8xyE
{
    if (!new)
    {
        setXY(em, inst); // this line is removed for some chip8 implementations
    }
    uint8_t x = (inst & 0x0F00) >> 8;
    uint8_t vx = em->registers[x];
    uint8_t val = vx << 1;
    uint8_t shifted = (vx & 0x80) >> 7; // 1000 0000
    em->registers[x] = val;
    em->registers[0xF] = shifted; // setting flag to the bit that will be shifted out
}
void storeBCD(Emulator *em, uint16_t inst) // FX33
{
    uint8_t num = em->registers[(inst & 0x0F00) >> 8]; // original value in register[x]
    // not gonna while loop this since this avoids corner cases (the while loop would have to go backwards)
    uint8_t ones = num % 10;
    uint8_t tens = (num / 10) % 10;
    uint8_t hundreds = (num / 100) % 10;
    em->memory[em->ir] = hundreds;
    em->memory[em->ir + 1] = tens;
    em->memory[em->ir + 2] = ones;
}
void storeUntilX(Emulator *em, uint16_t inst, uint8_t new) // FX55
{
    uint8_t x = (inst & (0x0F00) >> 8);
    for (int i = 0; i <= x; i++)
        em->memory[i + em->ir] = em->registers[i];
    if (!new)
    {
        em->ir += x;
    }
}

void loadUntilX(Emulator *em, uint16_t inst, uint8_t new) // FX65
{
    uint8_t x = (inst & (0x0F00) >> 8);
    for (int i = 0; i <= x; i++)
    {
        em->registers[i] = em->memory[i + em->ir];
    }
    if (!new)
    {
        em->ir += x;
    }
}
void addIndexRegisterX(Emulator *em, uint16_t inst) // FX1E
{
    em->ir += em->registers[(inst & 0x0F00) >> 8];
}
