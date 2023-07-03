#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

enum {
    RAM_CAP = 1 << 16
};

enum vm_opcode {
    OP_RRMOV, // 0 0 000 000 free1 size1 src3 dst3
    OP_RIMOV, // 0000 0 000 00000000 00000000 free4 size1 dst3 imm8-16
    OP_RMMOV, // 0000 0 000 00000000 00000000 free4 size1 dst3 mem16
    OP_MRMOV, // 0000 0 000 00000000 00000000 free4 size1 src3 mem16
    OP_MIMOV, // 0000000 0 00000000 00000000 00000000 00000000 free7 size1 mem16 imm8-16

    OP_HALT
};

enum vm_reg {
    R0 = 0,
    R1,
    R2,
    R3,
    R4,
    R5,
    R6,
    R7,
    PC,

    REG_COUNT
};

uint8_t ram[RAM_CAP];
uint16_t regs[REG_COUNT];

void ramstore8(uint16_t addr, uint8_t val)
{
    ram[addr] = val;
}

uint8_t ramload8(uint16_t addr)
{
    return ram[addr];
}

void ramstore(uint16_t addr, uint16_t val)
{
    uint8_t lsb, msb;

    lsb = val;
    msb = val >> 8;

    ramstore8(addr, lsb);
    ramstore8(addr + 1, msb);
}

uint16_t ramload(uint16_t addr)
{
    uint8_t lsb, msb;

    lsb = ramload8(addr);
    msb = ramload8(addr + 1);

    return (msb << 8) | lsb;
}

void vm_dump()
{
    printf("regs { ");
    for (int i = 0; i < REG_COUNT; ++i) {
        printf("%d ", regs[i]);
    }
    printf("}\n");

    printf("ram { ");
    for (int i = 0; i < 30; ++i) {
        printf("%d ", ram[i]);
    }
    printf("}\n\n");
}

int main()
{
    int i = 0;
    int vm_exit = 0;

    ramstore8(i++, OP_RIMOV);
    ramstore8(i++, 0x0 | R3);
    ramstore(i, 3125); i += 2;

    ramstore8(i++, OP_RRMOV);
    ramstore8(i++, ((0x0 | R3) << 3) | R0);

    ramstore(28, 4511);
    ramstore8(i++, OP_RMMOV);
    ramstore8(i++, 0x0 | R1);
    ramstore(i, 28); i += 2;

    ramstore8(i++, OP_MRMOV);
    ramstore8(i++, 0x0 | R0);
    ramstore(i, 28); i += 2;

    ramstore8(i++, OP_MIMOV);
    ramstore8(i++, 0x0);
    ramstore(i, 28); i += 2;
    ramstore(i, 420); i += 2;

    ramstore8(i++, OP_HALT);

    regs[PC] = 0;

    //vm_dump();

    while (vm_exit == 0) {
        uint8_t op, byte, size, dst, src;
        uint16_t mem;

        op = ramload8(regs[PC]++);

        vm_dump();

        switch (op) {
        case OP_RRMOV:
            byte = ramload8(regs[PC]++);
            dst = byte & 0x7;
            src = (byte >> 3) & 0x7;
            size = (byte >> 6) & 0x1;

            if (size == 1) {
                regs[dst] = (uint8_t) regs[src];
            } else {
                regs[dst] = regs[src];
            }

            break;

        case OP_RIMOV:
            byte = ramload8(regs[PC]++);
            dst = byte & 0x7;
            size = (byte >> 3) & 0x1;

            if (size == 1) {
                regs[dst] = ramload8(regs[PC]++);
            } else {
                regs[dst] = ramload(regs[PC]);
                regs[PC] += 2;
            }

            break;

        case OP_RMMOV:
            byte = ramload8(regs[PC]++);
            dst = byte & 0x7;
            size = (byte >> 3) & 0x1;
            mem = ramload(regs[PC]);
            regs[PC] += 2;

            if (size == 1) {
                regs[dst] = ramload8(mem);
            } else {
                regs[dst] = ramload(mem);
            }

            break;

        case OP_MRMOV:
            byte = ramload8(regs[PC]++);
            src = byte & 0x7;
            size = (byte >> 3) & 0x1;
            mem = ramload(regs[PC]);
            regs[PC] += 2;

            if (size == 1) {
                ramstore8(mem, regs[src]);
            } else {
                ramstore(mem, regs[src]);
            }

            break;

        case OP_MIMOV:
            byte = ramload8(regs[PC]++);
            size = byte & 0x1;
            mem = ramload(regs[PC]);
            regs[PC] += 2;

            if (size == 1) {
                ramstore8(mem, ramload8(regs[PC]++));
            } else {
                ramstore(mem, ramload(regs[PC]));
                regs[PC] += 2;
            }

            break;

        case OP_HALT:
            vm_exit = 1;
            break;

        default:
            fprintf(stderr, "unknown opcode %d\n", op);
        }
    }

    //vm_dump();

    return 0;
}
