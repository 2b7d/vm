#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

enum {
    MEMORY_CAP = 1 << 15,
    STACK_CAP = 1 << 7
};

enum op_code {
    OP_ADD,
    OP_SUB,
    OP_LIT,
    OP_ST,
    OP_LD,
    OP_JMP,
    OP_HALT
};

uint16_t memory[MEMORY_CAP];
uint16_t data_stack[STACK_CAP];
uint16_t ret_stack[STACK_CAP];

uint16_t *dp;
uint16_t *rp;
uint16_t *pc;

uint16_t ptrdiff(uint16_t *begin, uint16_t *end)
{
    return end - begin;
}

void print_stack(uint16_t *stack, uint16_t *sp, char *label)
{
    uint16_t i;

    printf("%-10s: {", label);
    for (i = 0; i < sp - stack ; ++i) {
        printf("0x%x", stack[i]);
        if (i != ptrdiff(stack, sp) - 1) {
            printf(", ");
        }
    }
    printf("}\n");
}

void push(uint16_t val)
{
    // TODO: proper error handling
    if (ptrdiff(data_stack, dp) >= STACK_CAP) {
        printf("ERROR: stack overflow\n");
        exit(1);
    }

    *dp++ = val;
}

uint16_t pop()
{
    // TODO: proper error handling
    if (ptrdiff(data_stack, dp) < 1) {
        printf("ERROR: stack underflow\n");
        exit(1);
    }

    return *--dp;
}

int main(void)
{
    size_t i = 0;
    int halt = 0;

    dp = data_stack;
    rp = ret_stack;
    pc = memory + i;

    memory[i++] = OP_LIT;
    memory[i++] = 3;
    memory[i++] = OP_LIT;
    memory[i++] = 2;

    memory[i++] = OP_LIT;
    memory[i++] = 1;
    memory[i++] = OP_JMP;
    memory[i++] = 9;

    memory[i++] = OP_ADD;

    memory[i++] = OP_HALT;

    print_stack(memory, memory + i, "memory");
    print_stack(data_stack, dp, "data stack");
    printf("\n");
    while (halt == 0) {
        uint16_t a, b;
        uint8_t op = *pc++ & 0xff;

        switch (op) {
        case OP_ADD:
            push(pop() + pop());
            break;

        case OP_SUB:
            a = pop();
            push(pop() - a);
            break;

        case OP_LIT:
            push(*pc++);
            break;

        case OP_ST:
            // TODO: validate address
            a = pop();
            memory[a] = pop();
            break;

        case OP_LD:
            // TODO: validate address
            a = pop();
            push(memory[a]);
            break;

        case OP_JMP:
            a = pop();
            b = *pc++;
            if (a == 1) {
                pc = memory + b;
            }
            break;

        case OP_HALT:
            halt = 1;
            break;

        default:
            printf("ERROR: unknown opcode %d\n", op);
            exit(1);
        }

        print_stack(memory, memory + i, "memory");
        print_stack(data_stack, dp, "data stack");
        printf("\n");
    }


    return 0;
}
