#include <stdio.h>
#include <stdint.h>

void printBinary(uint16_t value, int bits){
    for(int i = bits - 1; i >= 0; i--){
        printf("%d", (value >> i) & 1);
        if(i % 4 == 0) printf(" ");
    }
    printf("\n");
}

int main(void){
    // ── Opcode decoding ─────────────────────────────────────────────────────
    // In CHIP-8, opcodes are 2 bytes. They are fetched like this:
    //   byte1 = memory[pc]
    //   byte2 = memory[pc + 1]
    //   opcode = (byte1 << 8) | byte2

    uint8_t byte1 = 0x8A;
    uint8_t byte2 = 0xB4;
    uint16_t opcode = (byte1 << 8) | byte2;

    printf("opcode:        0x%04X\n", opcode);
    printBinary(opcode, 16);

    // ── Nibble extraction ───────────────────────────────────────────────────
    // CHIP-8 opcodes are broken into 4-bit nibbles: [type][x][y][n]
    // For opcode 0x8AB4:
    //   type = 8  (top nibble: what kind of instruction)
    //   x    = A  (second nibble: usually register index Vx)
    //   y    = B  (third nibble: usually register index Vy)
    //   n    = 4  (bottom nibble: small constant)

    uint8_t  type = (opcode & 0xF000) >> 12;
    uint8_t  x    = (opcode & 0x0F00) >> 8;
    uint8_t  y    = (opcode & 0x00F0) >> 4;
    uint8_t  n    = (opcode & 0x000F);
    uint8_t  kk   = (opcode & 0x00FF);       // bottom byte (8-bit constant)
    uint16_t nnn  = (opcode & 0x0FFF);       // bottom 12 bits (memory address)

    printf("\nNibble breakdown of 0x%04X:\n", opcode);
    printf("  type (top nibble):   0x%X\n", type);
    printf("  x    (2nd nibble):   0x%X  (register V%X)\n", x, x);
    printf("  y    (3rd nibble):   0x%X  (register V%X)\n", y, y);
    printf("  n    (4th nibble):   0x%X\n", n);
    printf("  kk   (low byte):     0x%02X\n", kk);
    printf("  nnn  (low 12 bits):  0x%03X\n", nnn);

    // ── Bitwise AND ─────────────────────────────────────────────────────────
    // Use & to mask (isolate) specific bits
    uint8_t a = 0xCA;
    uint8_t mask = 15u;
    printf("\nAND  %02X & %02X = %02X  (isolate low nibble)\n", a, mask, a & mask);

    // ── Bitwise OR ──────────────────────────────────────────────────────────
    // Use | to combine two bytes into a 16-bit opcode
    uint8_t high = 0xD0;
    uint8_t low  = 0x5F;
    uint16_t combined = (high << 8) | low;
    printf("OR   %02X | %02X  = %04X  (combine bytes)\n", high, low, combined);

    // ── Bitwise XOR ─────────────────────────────────────────────────────────
    // Used in CHIP-8 opcode 8xy3 (Vx XOR Vy)
    // Also used in the draw opcode to flip pixels on screen
    uint8_t vx = 0xAA;
    uint8_t vy = 0xCC;
    printf("XOR  %02X ^ %02X = %02X\n", vx, vy, vx ^ vy);

    // ── Bit shifting ────────────────────────────────────────────────────────
    // >> shifts bits right (divide by 2)
    // << shifts bits left  (multiply by 2)
    // Used in opcodes 8xy6 (shift right) and 8xyE (shift left)
    uint8_t val = 0xB4;
    printf("\nSHL  %02X << 1 = %02X\n", val, (uint8_t)(val << 1));
    printf("SHR  %02X >> 1 = %02X\n", val, val >> 1);

    // When shifting, CHIP-8 cares about the bit that falls off the edge.
    // That bit goes into VF (the carry/flag register).
    uint8_t vf_shr = val & 0x01;        // bit that falls off the right
    uint8_t vf_shl = (val >> 7) & 0x01; // bit that falls off the left
    printf("VF after SHR: %d\n", vf_shr);
    printf("VF after SHL: %d\n", vf_shl);

    // ── Addition with carry ──────────────────────────────────────────────────
    // Used in opcode 8xy4 (Vx += Vy). Result is stored as uint8 but
    // computed as uint16 so we can detect overflow into VF.
    uint8_t add_x = 0xFF;
    uint8_t add_y = 0x02;
    uint16_t result = add_x + add_y;
    uint8_t carry = result > 0xFF ? 1 : 0;
    printf("\nADD  %02X + %02X = result:%04X  carry(VF):%d  stored:%02X\n",
           add_x, add_y, result, carry, (uint8_t)result);

    return 0;
}