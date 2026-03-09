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

void cycle(chip8 *cpu){
    if(cpu->pc > 4094){
        fprintf(stderr, "Program counter out of bounds: 0x%03X\n", cpu->pc);
        return;
    }

    uint16_t opcode = ((uint16_t)cpu->memory[cpu->pc] << 8) | cpu->memory[cpu->pc + 1];
    uint16_t nnn = opcode & 0x0FFF;
    uint8_t kk = opcode & 0x00FF;
    uint8_t n = opcode & 0x000F;
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;

    cpu->pc += 2;

    switch(opcode & 0xF000){
        case 0x0000:
            switch(opcode){
                case 0x00E0:
                    break;
                case 0x00EE:
                    if(cpu->sp == 0){
                        fprintf(stderr, "Stack underflow on RET\n");
                        return;
                    }
                    cpu->sp--;
                    cpu->pc = cpu->stack[cpu->sp];
                    break;
                default:
                    fprintf(stderr, "Unsupported opcode: 0x%04X\n", opcode);
                    break;
            }
            break;
        case 0x1000:
            cpu->pc = nnn;
            break;
        case 0x2000:
            if(cpu->sp >= 16){
                fprintf(stderr, "Stack overflow on CALL\n");
                return;
            }
            cpu->stack[cpu->sp] = cpu->pc;
            cpu->sp++;
            cpu->pc = nnn;
            break;
        case 0x3000:
            if(cpu->V[x] == kk){
                cpu->pc += 2;
            }
            break;
        case 0x4000:
            if(cpu->V[x] != kk){
                cpu->pc += 2;
            }
            break;
        case 0x5000:
            if(n == 0 && cpu->V[x] == cpu->V[y]){
                cpu->pc += 2;
            }
            break;
        case 0x6000:
            cpu->V[x] = kk;
            break;
        case 0x7000:
            cpu->V[x] += kk;
            break;
        case 0x8000:
            switch(n){
                case 0x0:
                    cpu->V[x] = cpu->V[y];
                    break;
                case 0x1:
                    cpu->V[x] |= cpu->V[y];
                    break;
                case 0x2:
                    cpu->V[x] &= cpu->V[y];
                    break;
                case 0x3:
                    cpu->V[x] ^= cpu->V[y];
                    break;
                case 0x4: {
                    uint16_t sum = cpu->V[x] + cpu->V[y];
                    cpu->V[0xF] = sum > 0xFF;
                    cpu->V[x] = (uint8_t)sum;
                    break;
                }
                case 0x5:
                    cpu->V[0xF] = cpu->V[x] >= cpu->V[y];
                    cpu->V[x] -= cpu->V[y];
                    break;
                default:
                    fprintf(stderr, "Unsupported opcode: 0x%04X\n", opcode);
                    break;
            }
            break;
        case 0x9000:
            if(n == 0 && cpu->V[x] != cpu->V[y]){
                cpu->pc += 2;
            }
            break;
        case 0xA000:
            cpu->I = nnn;
            break;
        case 0xB000:
            cpu->pc = nnn + cpu->V[0];
            break;
        default:
            fprintf(stderr, "Unsupported opcode: 0x%04X\n", opcode);
            break;
    }

    if(cpu->delaytimer > 0){
        cpu->delaytimer--;
    }
    if(cpu->soundtimer > 0){
        cpu->soundtimer--;
    }
}


int main(){
    chip8 my_cpu;
    powerOn(&my_cpu);
    loadROM(&my_cpu, "Pong (alt).ch8");
    cycle(&my_cpu);
    return 0;
}