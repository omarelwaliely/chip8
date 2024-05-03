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
    uint8_t memory[4096];  // 4096 byte memory
    uint16_t stack[1024];  // 2048 byte stack (note I made this big since i have no constraint to worry about but the offical one is 64 bytes
    uint8_t stackIndex;    // index for the implementation of stack
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

void pushStack(Emulator *em, uint16_t pc);
uint16_t popStack(Emulator *em);

// The three stages (merged decode and execute into one function)
uint16_t
fetch(Emulator *em); // returns the instruction pc is currently pointing at and increments pc by 2
void decode_execute(Emulator *em, uint16_t inst);

// instructions (i decided to make a function for each one just to keep it clean even if it was really small)
void clearScreen(Emulator *em);                                    // clears screen
void jump(Emulator *em, uint16_t inst);                            // changes pc to jump address
void setXNN(Emulator *em, uint16_t inst);                          // set register vx to NN
void addXNN(Emulator *em, uint16_t inst);                          // add value NN to register vx
void setIndexRegisterNNN(Emulator *em, uint16_t inst);             // set register I to NNN
void draw(Emulator *em, uint16_t inst);                            // draw an N tall sprite from I to vx,vy.
void skipNNEqual(Emulator *em, uint16_t inst);                     // if vx == NN skip next inst
void skipNNNotEqual(Emulator *em, uint16_t inst);                  // if vx != NN skip next inst
void skipXYEqual(Emulator *em, uint16_t inst);                     // if vx== vy skip next inst
void skipXYNotEqual(Emulator *em, uint16_t inst);                  // if vx!= vy skip next inst
void callNNN(Emulator *em, uint16_t inst);                         // push curr pc to stack then pc == NNN
void returnCall(Emulator *em);                                     // set pc to popped value of stack
void setXY(Emulator *em, uint16_t inst);                           // vx = vy
void setXorY(Emulator *em, uint16_t inst);                         // vx |= vy
void setXandY(Emulator *em, uint16_t inst);                        // vx &= vy
void setXxorY(Emulator *em, uint16_t inst);                        // vx ^= vy
void setXplusY(Emulator *em, uint16_t inst);                       // vx += vy carry flag affected
void setXminusY(Emulator *em, uint16_t inst);                      // vx -= vy
void setXYminusX(Emulator *em, uint16_t inst);                     // vx = vy-vx
void setXShiftRightOneY(Emulator *em, uint16_t inst, uint8_t new); // vx = vy>>1
void setXShiftLeftOneY(Emulator *em, uint16_t inst, uint8_t new);  // vx = vy<<1
void storeUntilX(Emulator *em, uint16_t inst, uint8_t new);        // store from v0 to vx in memory starting at address I
void loadUntilX(Emulator *em, uint16_t inst, uint8_t new);         // load into v0 to vx from memory starting at address I
void storeBCD(Emulator *em, uint16_t inst);                        // store the value in vx as a Decimal number with mem[I],mem[I+1],mem[I+2] containing hundreds,tens,ones
void addIndexRegisterX(Emulator *em, uint16_t inst);               // Add vx to I carry flag not affected

#endif /* EMULATOR_H */