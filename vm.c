#include <stdio.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;

enum {
    RAM_CAP = 1 << 16
};

enum vm_opcode {
    OP_HALT = 0,

    OP_IRMOV, // value(2)dest4(1)
    OP_RRMOV, // src4,dest4(1)
    OP_MRMOV, // src4,dest4(1)
    OP_RMMOV, // src4,dest4(1)
    OP_ADD,   // src4,dest4(1)
};

enum vm_reg {
    R1 = 0,
    R2,
    R3,
    R4,

    RSP,
    RBP,
    RIP,

    // TODO(art): flags register?
    VM_REG_COUNT
};

u8 ram[RAM_CAP];
u16 regs[VM_REG_COUNT];

u8 read8(u16 addr)
{
    // TODO(art): validate addr
    return ram[addr];
}

u16 read16(u8 addr)
{
    u8 lsb, msb;

    lsb = read8(addr);
    msb = read8(addr + 1);

    return (msb << 8) | lsb;
}

int main(void)
{
    u16 i = 0;
    int halted = 0;

    // irmov $4 r1
    ram[i++] = 0x01;
    ram[i++] = 0x04;
    ram[i++] = 0x00;
    ram[i++] = 0x00;

    // irmov $6 r2
    ram[i++] = 0x01;
    ram[i++] = 0x06;
    ram[i++] = 0x00;
    ram[i++] = 0x01;

    // add r2, r1
    ram[i++] = 0x05;
    ram[i++] = (0x01 << 4) | 0x00;

    // halt
    ram[i++] = 0x00;

    regs[RIP] = 0;

    while (!halted) {
        enum vm_reg src, dest;
        u8 byte;
        u16 word;

        enum vm_opcode op = read8(regs[RIP]++);

        switch (op) {
        case OP_IRMOV:
            word = read16(regs[RIP]);
            regs[RIP] += 2;
            dest = read8(regs[RIP]++);
            regs[dest] = word;
            break;
        case OP_RRMOV:
            printf("OP_RRMOV: not implementd\n");
            break;
        case OP_MRMOV:
            printf("OP_MRMOV: not implementd\n");
            break;
        case OP_RMMOV:
            printf("OP_RMMOV: not implementd\n");
            break;
        case OP_ADD:
            byte = read8(regs[RIP]++);
            src = byte >> 4;
            dest = byte & 0xf;
            regs[dest] = regs[dest] + regs[src];
            break;
        case OP_HALT:
            halted = 1;
            break;
        default:
            fprintf(stderr, "unknown opcode: %d\n", op);
            return 1;
        }
    }

    return 0;
}
