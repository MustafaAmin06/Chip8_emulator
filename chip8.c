#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

typedef struct chip8{
    uint8_t memory[4096]; // main memory, 0x000 -> 0x1FF stores fonts, 0x200 -> 2xFFF stores ROM
    uint16_t stack[16]; // Essentially a record book of all the subroutines (functions) the cpu does
    uint16_t I; // This stores mem addresses. 16bits because 8bits is too small for 12 bits of mem we have
    uint16_t pc; // Holds the address of the next instruction to be executed
    uint8_t V[16]; // General purpose registers. Do math here
    uint8_t sp; // An index for the stack
    uint8_t delaytimer;  // Both of these are simple clocks
    uint8_t soundtimer;
} chip8;

uint8_t fontset[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void powerOn(chip8 *cpu){
    for(int i = 0; i < 4096; i++){
        cpu->memory[i] = 0;
    }
    for(int i = 0; i < 16; i++){
        cpu->V[i] = 0;
        cpu->stack[i] = 0;
    }
    cpu->I = 0;
    cpu->pc = 0x200;
    cpu->sp = 0;
    cpu->delaytimer = 0;
    cpu->soundtimer = 0;

    for(int i = 0; i < 80; i++){
        cpu->memory[0x50 + i] = fontset[i];
    }
}

void loadROM(chip8 *cpu, char *filename){
    FILE *file;
    file = fopen(filename, "rb");
    if (file == NULL){
        perror("Error opening file");
        return;
    }
    fread((void *)&cpu->memory[0x200], 1, 3584, file);

    for(int i = 0; i < 10; i++) {
    printf("Byte at 0x%03X: %02X\n", 0x200 + i, cpu->memory[0x200 + i]);
    }
    fclose(file);  
}


int main(){
    chip8 my_cpu;
    powerOn(&my_cpu);
    loadROM(&my_cpu, "Pong (alt).ch8");
    return 0;
}