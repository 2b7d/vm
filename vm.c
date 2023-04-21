#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "vm.h"

enum {
    RAM_CAP = 1 << 16,
    STACK_CAP = 1 << 7,
};

uint8_t ram[RAM_CAP];
uint16_t stack[STACK_CAP];

uint16_t pc;
uint8_t sp;
uint16_t rsp;
uint16_t rbp;

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

void push(uint16_t val)
{
    stack[sp++] = val;
}

uint16_t pop()
{
    return stack[--sp];
}

void rspush(uint16_t val)
{
    rsp -= 2;
    ramstore(rsp, val);
}

uint16_t rspop()
{
    uint16_t val = ramload(rsp);
    rsp += 2;
    return val;
}

void load_program(char *pathname)
{
    int i;
    FILE *f;

    f = fopen(pathname, "r");
    if (f == NULL) {
        fprintf(stderr, "ERROR: failed to open binary file");
        exit(1);
    }

    fread(&pc, 2, 1, f);

    i = 0;
    for (;;) {
        if (i >= RAM_CAP) {
            fprintf(stderr, "ERROR: not enough memory to load program");
            exit(1);
        }

        fread(ram + i, 1, 1, f);
        i++;

        if (ferror(f) != 0) {
            fprintf(stderr, "ERROR: failed to read binary file");
            exit(1);
        }

        if (feof(f) != 0) {
            //ramstore(BRK, i - 1);
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
        fprintf(stderr, "binary file is required");
        return 1;
    }

    pc = 0;
    sp = 0;

    rsp = RAM_CAP - 1;
    rbp = rsp;

    load_program(*argv);

    while (halt == 0) {
        uint16_t a, b;
        enum vm_opcode opcode = ramload8(pc++);

        switch (opcode) {
        case OP_ST:
            a = pop();
            ramstore(a, pop());
            break;

        case OP_LD:
            push(ramload(pop()));
            break;

        case OP_STB:
            a = pop();
            ramstore8(a, pop());
            break;

        case OP_LDB:
            push(ramload8(pop()));
            break;

        case OP_ADD:
            push(pop() + pop());
            break;

        case OP_SUB:
            a = pop();
            push(pop() - a);
            break;

        case OP_MUL:
            push(pop() * pop());
            break;

        case OP_DIV:
            a = pop();
            push(pop() / a);
            break;

        case OP_MOD:
            a = pop();
            push(pop() % a);
            break;

        case OP_INC:
            push(pop() + 1);
            break;

        case OP_DEC:
            push(pop() - 1);
            break;

        case OP_PUSH:
            push(ramload(pc));
            pc += 2;
            break;

        case OP_PUSHB:
            push(ramload8(pc++));
            break;

        case OP_DUP:
            a = pop();
            push(a);
            push(a);
            break;

        case OP_OVER:
            a = pop();
            b = pop();
            push(b);
            push(a);
            push(b);
            break;

        case OP_SWAP:
            a = pop();
            b = pop();
            push(a);
            push(b);
            break;

        case OP_DROP:
            pop();
            break;

        case OP_RSPUSH:
            rspush(pop());
            break;

        case OP_RSPOP:
            push(rspop());
            break;

        case OP_RSCOPY:
            a = rspop();
            rspush(a);
            push(a);
            break;

        case OP_RSDROP:
            rspop();
            break;

        case OP_RSP:
            push(rsp);
            break;

        case OP_RSPSET:
            rsp = pop();
            break;

        case OP_RBP:
            push(rbp);
            break;

        case OP_RBPSET:
            rbp = pop();
            break;

//        case OP_BRK:
//            push(ramload(BRK));
//            break;
//
//        case OP_BRKSET:
//            a = ramload(BRK);
//            ramstore(BRK, pop());
//            push(a);
//            break;

        case OP_EQ:
            push(pop() == pop());
            break;

        case OP_GT:
            a = pop();
            push(pop() > a);
            break;

        case OP_LT:
            a = pop();
            push(pop() < a);
            break;

        case OP_OR:
            a = pop();
            b = pop();
            push(a || b);
            break;

        case OP_AND:
            a = pop();
            b = pop();
            push(a && b);
            break;

        case OP_NOT:
            push(!pop());
            break;

        case OP_JMP:
            pc = ramload(pc);
            break;

        case OP_JMPIF:
            a = ramload(pc);
            pc += 2;
            if (pop() == 1) {
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
                for (int i = 0; i < a; ++i) {
                    fputc(ramload8(b + i), stdout);
                }
                break;

            case SYS_READ:
                fprintf(stderr, "ERROR: SYS_READ not implemented\n");
                exit(1);
            default:
                fprintf(stderr, "ERROR: unknown vm_syscall(%u)\n", a);
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
            fprintf(stderr, "ERROR: unknown vm_opcode(%u)\n", opcode);
            exit(1);
        }
    }

    return 0;
}
