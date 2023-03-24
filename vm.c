#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "vm.h"

enum {
    RAM_CAP = 1 << 16,
    STACK_CAP = 1 << 8,
    BRK = RAM_CAP - 2,
    RSP = RAM_CAP - 4
};

uint8_t ram[RAM_CAP];
uint8_t stack[STACK_CAP];

uint16_t pc;
uint8_t sp;
uint16_t rsp;

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

    ram[addr] = lsb;
    ram[addr + 1] = msb;
}

uint16_t ramload(uint16_t addr)
{
    uint8_t lsb, msb;

    lsb = ramload8(addr);
    msb = ramload8(addr + 1);

    return msb << 8 | lsb;
}

void push8(uint8_t val)
{
    stack[sp++] = val;
}

uint8_t pop8()
{
    return stack[--sp];
}

void push(uint16_t val)
{
    uint8_t lsb, msb;

    lsb = val;
    msb = val >> 8;

    push8(lsb);
    push8(msb);
}

uint16_t pop()
{
    uint8_t lsb, msb;

    msb = pop8();
    lsb = pop8();

    return msb << 8 | lsb;
}

void rspush8(uint8_t val)
{
    ramstore8(rsp--, val);
}

uint8_t rspop8()
{
    return ramload8(++rsp);
}

void rspush(uint16_t val)
{
    uint8_t lsb, msb;

    lsb = val;
    msb = val >> 8;

    rspush8(msb);
    rspush8(lsb);
}

uint16_t rspop()
{
    uint8_t lsb, msb;

    lsb = rspop8();
    msb = rspop8();

    return msb << 8 | lsb;
}

void load_program(char *pathname)
{
    FILE *f;
    size_t i = 0;

    f = fopen(pathname, "r");
    if (f == NULL) {
        puts("ERROR: failed to open binary file");
        exit(1);
    }

    fread(&pc, sizeof(uint16_t), 1, f);

    for (;;) {
        if (i >= RAM_CAP) {
            puts("ERROR: not enough memory to load program");
            exit(1);
        }

        fread(ram + i, sizeof(uint8_t), 1, f);
        i++;

        if (ferror(f) != 0) {
            puts("ERROR: failed to read binary file");
            exit(1);
        }

        if (feof(f) != 0) {
            ramstore(BRK, i - 1);
            break;
        }
    }

    fclose(f);
}

int main(int argc, char **argv)
{
    int halt = 0;

    argc--;
    argv++;

    if (argc < 1) {
        puts("binary file is required");
        return 1;
    }

    pc = 0;
    sp = 0;
    rsp = RSP;

    load_program(*argv);

    while (halt == 0) {
        uint16_t a, b;
        uint8_t a8, b8, inst, opcode, mode;

        inst = ramload8(pc++);
        opcode = inst & 0x7f;
        mode = inst >> 7 & 0x1;

        switch (opcode) {
        case OP_ST:
            if (mode == 1) {
                a = pop();
                ramstore(a, pop());
            } else {
                a8 = pop8();
                ramstore8(a8, pop8());
            }
            break;

        case OP_LD:
            if (mode == 1) {
                push(ramload(pop()));
            } else {
                push8(ramload8(pop8()));
            }
            break;

        case OP_ADD:
            if (mode == 1) {
                push(pop() + pop());
            } else {
                push8(pop8() + pop8());
            }
            break;

        case OP_SUB:
            if (mode == 1) {
                a = pop();
                push(pop() - a);
            } else {
                a8 = pop8();
                push8(pop8() - a8);
            }
            break;

        case OP_MUL:
            if (mode == 1) {
                push(pop() * pop());
            } else {
                push8(pop8() * pop8());
            }
            break;

        case OP_DIV:
            if (mode == 1) {
                a = pop();
                push(pop() / a);
            } else {
                a8 = pop8();
                push8(pop8() / a8);
            }
            break;

        case OP_MOD:
            if (mode == 1) {
                a = pop();
                push(pop() % a);
            } else {
                a8 = pop8();
                push8(pop8() % a8);
            }
            break;

        case OP_INC:
            if (mode == 1) {
                a = pop();
                push(a + 1);
            } else {
                a8 = pop8();
                push8(a8 + 1);
            }
            break;

        case OP_DEC:
            if (mode == 1) {
                a = pop();
                push(a - 1);
            } else {
                a8 = pop8();
                push8(a8 - 1);
            }
            break;

        case OP_PUSH:
            if (mode == 1) {
                push(ramload(pc));
                pc += 2;
            } else {
                push8(ramload8(pc++));
            }
            break;

        case OP_DUP:
            if (mode == 1) {
                a = pop();
                push(a);
                push(a);
            } else {
                a8 = pop8();
                push8(a8);
                push8(a8);
            }
            break;

        case OP_OVER:
            if (mode == 1) {
                a = pop();
                b = pop();
                push(b);
                push(a);
                push(b);
            } else {
                a8 = pop8();
                b8 = pop8();
                push8(b8);
                push8(a8);
                push8(b8);
            }
            break;

        case OP_SWAP:
            if (mode == 1) {
                a = pop();
                b = pop();
                push(a);
                push(b);
            } else {
                a8 = pop8();
                b8 = pop8();
                push8(a8);
                push8(b8);
            }
            break;

        case OP_DROP:
            if (mode == 1) {
                pop();
            } else {
                pop8();
            }
            break;

        case OP_RSPUSH:
            if (mode == 1) {
                rspush(pop());
            } else {
                rspush8(pop8());
            }
            break;

        case OP_RSPOP:
            if (mode == 1) {
                push(rspop());
            } else {
                push8(rspop8());
            }
            break;

        case OP_RSCOPY:
            if (mode == 1) {
                a = rspop();
                rspush(a);
                push(a);
            } else {
                a8 = rspop8();
                rspush8(a8);
                push8(a8);
            }
            break;

        case OP_RSDROP:
            if (mode == 1) {
                rspop();
            } else {
                rspop8();
            }
            break;

        case OP_RSP:
            push(rsp);
            break;

        case OP_RSPSET:
            rsp = pop();
            break;

        case OP_BRK:
            push(ramload(BRK));
            break;

        case OP_BRKSET:
            a = ramload(BRK);
            ramstore(BRK, pop());
            push(a);
            break;

        case OP_EQ:
            if (mode == 1) {
                push8(pop() == pop());
            } else {
                push8(pop8() == pop8());
            }
            break;

        case OP_GT:
            if (mode == 1) {
                a = pop();
                push8(pop() > a);
            } else {
                a8 = pop8();
                push8(pop8() > a8);
            }
            break;

        case OP_LT:
            if (mode == 1) {
                a = pop();
                push8(pop() < a);
            } else {
                a8 = pop8();
                push8(pop8() < a8);
            }
            break;

        case OP_OR:
            a8 = pop8();
            b8 = pop8();
            push8(a8 || b8);
            break;

        case OP_AND:
            a8 = pop8();
            b8 = pop8();
            push8(a8 && b8);
            break;

        case OP_NOT:
            push8(!pop8());
            break;

        case OP_JMP:
            pc = ramload(pc);
            break;

        case OP_JMPIF:
            a = ramload(pc);
            pc += 2;
            if (pop8() == 1) {
                pc = a;
            }
            break;

        case OP_HALT:
            halt = 1;
            break;

        case OP_SYSCALL:
            a = ramload(pc);
            pc += 2;
            switch (a) {
            case SYS_WRITE:
                a = pop();
                b = pop();
                for (size_t i = 0; i < a; ++i) {
                    fputc(ramload8(b + i), stdout);
                }
                break;

            case SYS_READ:
            default:
                printf("ERROR: unknown vm_syscall(%u)\n", a);
                exit(1);
            }
            break;

        case OP_CALL:
            a = ramload(pc);
            pc += 2;
            rspush(pc);
            pc = a;
            break;

        case OP_RET:
            pc = rspop();
            break;

        default:
            printf("ERROR: unknown vm_opcode(%u)\n", opcode);
            exit(1);
        }
    }

    return 0;
}
