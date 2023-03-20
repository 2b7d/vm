#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "vm.h"

enum {
    RAM_CAP = 1 << 15,
    STACK_CAP = 1 << 7
};

uint16_t ram[RAM_CAP];
uint16_t stack[STACK_CAP];
uint16_t retstack[STACK_CAP];

uint16_t pc;
uint16_t sp;
uint16_t rsp;

void print_memory(uint16_t *mem, uint16_t size, char *label)
{
    uint16_t i;

    printf("%-8s {", label);
    for (i = 0; i < size ; ++i) {
        printf("%u", mem[i]);
        if (i != size - 1) {
            printf(", ");
        }
    }
    printf("}\n");
}

void push(uint16_t val)
{
    // TODO: proper error handling
    if (sp >= STACK_CAP) {
        printf("ERROR: stack overflow\n");
        exit(1);
    }

    stack[sp++] = val;
}

uint16_t pop()
{
    // TODO: proper error handling
    if (sp < 1) {
        printf("ERROR: stack underflow\n");
        exit(1);
    }

    return stack[--sp];
}

uint16_t ramload(uint16_t addr)
{
    if (addr >= RAM_CAP) {
        printf("ERROR: address(0x%04x) is out of bound\n", addr);
        exit(1);
    }

    return ram[addr];
}

void ramstore(uint16_t addr, uint16_t val)
{
    if (addr >= RAM_CAP) {
        printf("ERROR: address(0x%04x) is out of bound\n", addr);
        exit(1);
    }

    ram[addr] = val;
}

void load_program(char *pathname)
{
    FILE *f;
    size_t i = 0;

    f = fopen(pathname, "r");
    if (f == NULL) {
        printf("ERROR: failed to open binary file\n");
        exit(1);
    }

    fread(&pc, sizeof(uint16_t), 1, f);

    for (;;) {
        if (i >= RAM_CAP) {
            printf("ERROR: not enough memory to load program\n");
            exit(1);
        }

        fread(ram + i, sizeof(uint16_t), 1, f);
        i++;

        if (ferror(f) != 0) {
            printf("ERROR: failed to read binary file\n");
            exit(1);
        }

        if (feof(f) != 0) {
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
        printf("binary file is required\n");
        return 1;
    }

    pc = 0;
    sp = 0;
    rsp = 0;

    load_program(*argv);

    print_memory(ram, 20, "RAM");
    print_memory(stack, sp, "stack");

    while (halt == 0) {
        uint16_t a, b;
        uint8_t op = ramload(pc++) & 0xff;

        switch (op) {
        case OP_ST:
            a = pop();
            ramstore(a, pop());
            break;

        case OP_LD:
            push(ramload(pop()));
            break;

        case OP_LIT:
            push(ramload(pc++));
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
            a = pop();
            push(a + 1);
            break;

        case OP_DEC:
            a = pop();
            push(a - 1);
            break;

        case OP_DUP:
            push(stack[sp - 1]);
            break;

        case OP_OVER:
            push(stack[sp - 2]);
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

        case OP_IFJMP:
            a = ramload(pc++);
            if (pop() == 1) {
                pc = a;
            }
            break;

        case OP_HALT:
            halt = 1;
            break;

        default:
            printf("ERROR: unknown vm_opcode(%u)\n", op);
            exit(1);
        }

        print_memory(stack, sp, "stack");
    }

    print_memory(ram, 20, "RAM");
    print_memory(stack, sp, "stack");

    return 0;
}
