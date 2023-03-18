#include <stdio.h>
#include <stdint.h>
#include <string.h>

enum {
    MEMORY_CAP = 1 << 15,
    STACK_CAP = 1 << 7
};

enum op_code {
    OP_ADD,
    OP_SUB,
    OP_LIT,
    OP_HALT,
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

    printf("%s: {", label);
    for (i = 0; i < sp - stack ; ++i) {
        printf("0x%04x", stack[i]);
        if (i != ptrdiff(stack, sp) - 1) {
            printf(", ");
        }
    }
    printf("}\n");
}

void push(uint16_t val)
{
    // TODO: check stack overflow
    *dp++ = val;
}

uint16_t pop()
{
    // TODO: check stack underflow
    return *--dp;
}

int main(void)
{
    size_t i = 0;
    int exit = 0;

    memory[i++] = OP_LIT;
    memory[i++] = 0x25;
    memory[i++] = OP_LIT;
    memory[i++] = 0x10;
    memory[i++] = OP_LIT;
    memory[i++] = 0x5;
    memory[i++] = OP_ADD;
    memory[i++] = OP_SUB;
    memory[i++] = OP_HALT;

    dp = data_stack;
    rp = ret_stack;
    pc = memory;

    print_stack(data_stack, dp, "data stack");
    for (;;) {
        uint16_t a;
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

        case OP_HALT:
            exit = 1;
            break;
        }

        if (exit == 1) {
            break;
        }

        print_stack(data_stack, dp, "data stack");
    }


    return 0;
}
