#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>
#include <stdbool.h>

typedef struct chip8 {
    uint8_t  memory[4096];
    uint16_t stack[16];
    uint16_t I;
    uint16_t pc;
    uint8_t  V[16];
    uint8_t  sp;
    uint8_t  delaytimer;
    uint8_t  soundtimer;
    uint32_t display[64 * 32];
    uint8_t  keypad[16];
} chip8;

// Core functions
void powerOn(chip8 *cpu);
void loadROM(chip8 *cpu, const char *filename);
void cycle(chip8 *cpu);

#endif