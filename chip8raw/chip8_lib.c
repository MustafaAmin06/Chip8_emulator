#include "chip8.h"
#include <string.h>
#include <stdlib.h>

static chip8 cpu;
static char current_rom[256];

void lib_init(const char *rompath){
    strncpy(current_rom, rompath, 255);
    powerOn(&cpu);
    loadROM(&cpu, rompath);
}

void lib_reset(){
    powerOn(&cpu);
    loadROM(&cpu, current_rom);
}

void lib_getmem(uint8_t *out){
    for (int i = 0; i < 4096; i++){
        out[i] = cpu.memory[i];
    }
}

void lib_get_registers(uint8_t *out) {
    for (int i = 0; i < 16; i++) {
        out[i] = cpu.V[i];
    }
}

void lib_get_bcd_addresses(uint16_t *out) {
    out[0] = cpu.bcd_address[0];
    out[1] = cpu.bcd_address[1];
}

void lib_step(int cycles) {
    for (int i = 0; i < cycles; i++) {
        cycle(&cpu);
        if (i % 8 == 0) {
            if (cpu.delaytimer > 0) cpu.delaytimer--;
            if (cpu.soundtimer > 0) cpu.soundtimer--;
        }
    }
}

void lib_set_key(int key, int state) {
    if (key >= 0 && key < 16) {
        cpu.keypad[key] = (uint8_t)state;
    }
}

// Copies the 2048-pixel display into a buffer Python provides
void lib_get_display(uint8_t *out) {
    for (int i = 0; i < 2048; i++) {
        out[i] = (uint8_t)(cpu.display[i] & 0x1);  //We only need to know if a pixel is on or off, thats why we output 8 bit.
    }
}
