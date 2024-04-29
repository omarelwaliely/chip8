#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#ifndef EMULATOR_H
#define EMULATOR_H

typedef struct
{
    // might need "display" 64x32 attribute but not sure
    // keypad??
    uint8_t memory[4096];  // 4096 kilobyte memory
    uint16_t stack[32];    // 64 byte stack
    uint16_t pc;           // program counter
    uint16_t ir;           // index register (also referred to as I) used to point at location in memory
    uint8_t dtimer;        // delay timer (decremented at a rate of 60HZ until 0)
    uint8_t stimer;        // sound timer (same as delay timer but then beeps)
    uint8_t registers[16]; // 16 1 byte registers v0 -> vf (vf sometimes used as flag register)
    SDL_Window *window;    // the emulator window
    SDL_Event windowEvent; // checks if an event happens in emulator window
    SDL_Renderer *rend;    // renderer for pixels

} Emulator;

void startEmulator(Emulator *em);                 // perform initilizations
void startProgram(Emulator *em, const char *ROM); // load program into memory, I will most likely add support to close and open different chip8 programs
void printMemory(Emulator *em);                   // read memory (for debugging)

// The three stages (merged decode and execute into one function)
uint16_t fetch(Emulator *em); // returns the instruction pc is currently pointing at and increments pc by 2
void decode_execute(Emulator *em, uint16_t inst);

// instructions (i decided to make a function for each one just to keep it clean even if it was really small)
void clearScreen(Emulator *em);                     // clears screen
void jump(Emulator *em, uint16_t inst);             // changes pc to jump address
void setReg(Emulator *em, uint16_t inst);           // set register vx to NN
void addReg(Emulator *em, uint16_t inst);           // add value NN to register vx
void setIndexRegister(Emulator *em, uint16_t inst); // set register I to NNN
void draw(Emulator *em, uint16_t inst);             // draw an N tall sprite from I to x,y.

#endif /* EMULATOR_H */