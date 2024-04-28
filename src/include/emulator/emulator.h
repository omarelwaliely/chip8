#include <stdint.h>
#include <stdio.h>
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

} Emulator;

void startProgram(Emulator *em, const char *ROM); // load program into memory and perform intilizations
// these are the three stages there probably wont just be a function for each one but ill keep it like this for now
void fetch(Emulator *em);
void decode(Emulator *em);
void execute(Emulator *em);

#endif /* EMULATOR_H */