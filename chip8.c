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
    uint32_t display[64 * 32]; // Display for bits of a 64 x 32 pixel disp
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
    // clears mem
    for(int i = 0; i < 4096; i++){
        cpu->memory[i] = 0;
    }
    // clears display
    for(int i = 0; i < 2048; i++) {
        cpu->display[i] = 0;
    }
    // clears both stacks
    for(int i = 0; i < 16; i++){
        cpu->V[i] = 0;
        cpu->stack[i] = 0;
    }
    cpu->I = 0;
    // mem and intructions usually start at 0x200, hence why we positioned it here
    cpu->pc = 0x200;
    cpu->sp = 0;
    cpu->delaytimer = 0;
    cpu->soundtimer = 0;
    // load fonts into mem starting from 0x50
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
    // Out of bounds
    if(cpu->pc > 4094){
        fprintf(stderr, "Program counter out of bounds: 0x%03X\n", cpu->pc);
        return;
    }
    // FETCH
    uint16_t opcode = (cpu->memory[cpu->pc] << 8) | cpu->memory[cpu->pc + 1];

    // DECODE
    uint8_t T = (opcode & 0xF000) >> 12;
    uint8_t X = (opcode & 0x0F00) >> 8;
    uint8_t Y = (opcode & 0x00F0) >> 4;
    uint8_t N = (opcode & 0x000F);
    uint8_t NN = (opcode & 0x00FF);
    uint16_t NNN = (opcode & 0x0FFF);

    // EXECUTE
    switch(T) {
        case 0x0:  // Intruction 0NNN: Reset display
            if (opcode == 0x00E0){
                for(int i = 0; i < 2048; i++){
                    cpu->display[i] = 0;
                }
                cpu->pc += 2;
            }
            else if (opcode == 0x00EE){ // Subroutine anihalator
                cpu->sp --;
                cpu->pc = cpu->stack[cpu->sp];
                cpu->pc += 2;
            }
            break;

        case 0x1:
            cpu->pc = NNN;
            break;

        case 0x2: // Instruction 2NNN: Subroutine call
            cpu->stack[cpu->sp] = cpu->pc;
            cpu->sp ++;
            cpu->pc = NNN;
            break;

        case 0x3:  // Intruction 3NNN: Skip if equal
            if (cpu->V[X] == NN){
                cpu->pc += 4;
            }   else {
                cpu->pc += 2;
            }
            break;

        case 0x4:  // Intruction 4NNN: Skip if not equal
            if (cpu->V[X] != NN){
                cpu->pc += 4;
            } else {
                cpu->pc += 2;
            }
            break;

        case 0x5:
            if (cpu->V[X] == cpu->V[Y]){
                cpu->pc += 4;
            } else {
                cpu->pc += 2;
            }
            break;

        case 0x6: // Instruction 6XNN: Set register VX to NN
            cpu->V[X] = NN;
            cpu->pc += 2;
            break;

        case 0x7:
            cpu->V[X] += NN;
            cpu->pc += 2;
            break;

        case 0x8:
            switch(N){
                case 0x0:
                    cpu->V[X] = cpu->V[Y];
                    cpu->pc += 2;
                    break;
                
                case 0x1:  // OR
                    cpu->V[X] = (cpu->V[X] | cpu->V[Y]);
                    cpu->pc += 2;
                    break;

                case 0x2:  // AND
                    cpu->V[X] &= cpu->V[Y];
                    cpu->pc += 2;
                    break;
                
                case 0x3: // XOR
                    cpu->V[X] ^= cpu->V[Y];
                    cpu->pc += 2;
                    break;

                case 0x4: { // Addition w carry
                    uint16_t sum = cpu->V[X] + cpu->V[Y];
                    cpu->V[0xF] = (sum > 0xFF) ? 1 : 0;  // Turn on carry flag if sum is greater than 8 bits
                    cpu->V[X] = sum & 0x00FF;
                    cpu->pc += 2;
                    break;
                }
                
                case 0x5: { // SUbtract x - y
                    uint8_t flag = (cpu->V[X] >= cpu->V[Y]) ? 1 : 0;  // If X is greater than Y, turn on borrow flag
                    cpu->V[X] -= cpu->V[Y];
                    cpu->V[0xF] = flag;
                    cpu->pc += 2;
                    break;
                }
                
                case 0x6: { // right shift (divid by 2)
                    uint8_t flag = cpu->V[X] & 0x1; // Extract the right most bit
                    cpu->V[X] >>= 1;
                    cpu->V[0xF] = flag;
                    cpu->pc += 2;
                    break;
                }

                case 0x7: { // Subtraction y - x
                    uint8_t flag = (cpu->V[X] >= cpu->V[Y]) ? 0 : 1;
                    cpu->V[X] = cpu->V[Y] - cpu->V[X];
                    cpu->V[0xF] = flag;
                    cpu->pc += 2;
                    break;
                }

                case 0x8: { // shift by left (mult by 2)
                    uint8_t flag = (cpu->V[X] & 0x0080) >> 7; // catches the left most bit (most signifigiant)
                    cpu->V[X] <<= 1;
                    cpu->V[0xF] = flag;
                    cpu->pc += 2;
                    break;
                }
                break;
            }

        case 0x9: // 9XY0: Skip if V[X] != V[Y]
            if (cpu->V[X] != cpu->V[Y]) {
                cpu->pc += 4;
            } else {
                cpu->pc += 2;
            }
            break;

        case 0xA: // Instruction ANNN: Set index register I to NNN
            cpu->I = NNN;
            cpu->pc += 2;
            break;
        
        case 0xB: // BNNN: Jump to location NNN + V[0]
            cpu->pc = NNN + cpu->V[0];
            break;
        
        case 0xC: // CXNN: Set V[X] to random byte AND NN
            // Requires <stdlib.h>, which you already included
            cpu->V[X] = (rand() % 256) & NN;
            cpu->pc += 2;
            break;

        case 0xF:
            switch(NN) {
                case 0x1E:  // Intruction FX
                    cpu->I += cpu->V[X];
                    cpu->pc += 2;
                    break;
            }
            break;

        default:
            printf("Opcode not implemented yet: %04X\n", opcode);
            cpu->pc += 2;
            break;
    }
}


int main(){
    chip8 my_cpu;
    powerOn(&my_cpu);
    loadROM(&my_cpu, "Pong (alt).ch8");
    cycle(&my_cpu);
    return 0;
}