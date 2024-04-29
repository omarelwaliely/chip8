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
            startProgram(em, "IBMLogo.ch8");
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
            // implement return
        }
        break;
    case 0x1:
        jump(em, inst);
        break;
    case 0x6:
        setReg(em, inst);
        break;
    case 0x7:
        addReg(em, inst);
        break;
    case 0xA:
        setIndexRegister(em, inst);
        break;
    case 0xD:
        draw(em, inst);
        break;
    default:
        printf("%x is unimplemented.\n", firstNib);
    }
}

// Instructions

void clearScreen(Emulator *em)
{
    SDL_SetRenderDrawColor(em->rend, 0, 0, 0, 255); // setting color to opaque black
    SDL_RenderClear(em->rend);
    SDL_RenderPresent(em->rend);
}

void jump(Emulator *em, uint16_t inst) // 1NNN
{
    em->pc = inst & 0x0FFF;
}
void setReg(Emulator *em, uint16_t inst) // 6XNN
{
    em->registers[(inst & 0x0F00) >> 8] = inst & 0x00FF; // store the last 8 bits in register of X which is bit 8->11
}
void addReg(Emulator *em, uint16_t inst) // 7XNN
{
    em->registers[(inst & 0x0F00) >> 8] += (inst & 0x00FF); // add the last 8 bits to register of X which is bit 8->11 (no overflow for this instruction)
}
void setIndexRegister(Emulator *em, uint16_t inst) // ANNN
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
                if (r == 0) // if both are one we set pixel to white and vf flag to 1
                {
                    SDL_SetRenderDrawColor(em->rend, 255, 255, 255, 255);
                    em->registers[0xF] = 1;
                }
                else // if sprite is one but row is black then we set pixel to black
                {
                    SDL_SetRenderDrawColor(em->rend, 0, 0, 0, 255);
                }
                SDL_RenderDrawPoint(em->rend, screenX, screenY);
            }
        }
        SDL_RenderPresent(em->rend);
    }
}
