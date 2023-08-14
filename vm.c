#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

typedef uint8_t byte;
typedef uint16_t word;

enum {
    RAM_CAP = 1 << 16
};

enum vm_segment {
    SEG_CONST = 0,
    SEG_LOCAL
};

enum vm_opcode {
    OP_HALT = 0,

    OP_PUSH,
    OP_POP,

    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,

    OP_EQ,
    OP_LT,
    OP_GT,

    OP_JMP,
    OP_CJMP
};

byte ram[RAM_CAP];
word ip;
word sp;
word lcp;

int stack_start;
int local_start;

byte ram_readb(word addr)
{
    return ram[addr];
}

void ram_writeb(word addr, byte value)
{
    ram[addr] = value;
}

word ram_read(word addr)
{
    byte lsb, msb;

    lsb = ram_readb(addr);
    msb = ram_readb(addr + 1);

    return (msb << 8) | lsb;
}

void ram_write(word addr, word value)
{
    byte lsb, msb;

    lsb = value;
    msb = value >> 8;

    ram_writeb(addr, lsb);
    ram_writeb(addr + 1, msb);
}

void push(word value)
{
    // TODO(art): check overflow
    ram_write(sp, value);
    sp += 2;
}

word pop()
{
    // TODO(art): check underflow
    sp -= 2;
    return ram_read(sp);
}

void dump_mem(char *label, int start, int size)
{
    printf("%s: ", label);
    for (int i = start; i < start + size; ++i) {
        printf("%d ", ram[i]);
    }
    printf("\n");
}

int main(void)
{
    int i, halted;

    halted = 0;
    i = 0;

    ram_writeb(i, OP_PUSH); i++;
    ram_writeb(i, SEG_CONST); i++;
    ram_write(i, 5); i+=2;

    ram_writeb(i, OP_PUSH); i++;
    ram_writeb(i, SEG_CONST); i++;
    ram_write(i, 9); i+=2;

    ram_writeb(i, OP_ADD); i++;

    ram_writeb(i, OP_HALT); i++;

    ip = 0;
    sp = i;
    lcp = sp + 4096; // TODO(art): make it constant

    stack_start = sp;
    local_start = lcp;

    dump_mem("Stack", stack_start, sp - stack_start);
    dump_mem("Local", local_start, 10);
    printf("\n");

    while (!halted) {
        enum vm_opcode op;
        enum vm_segment seg;
        word w1, w2;

        op = ram_readb(ip++);

        switch (op) {
        case OP_PUSH:
            seg = ram_readb(ip++);
            w1 = ram_read(ip);
            ip += 2;

            switch (seg) {
            case SEG_CONST:
                push(w1);
                break;
            case SEG_LOCAL:
                w2 = ram_read(lcp + w1 * 2);
                push(w2);
                break;
            default:
                fprintf(stderr, "unknown segment %d\n", seg);
                exit(1);
            }
            break;
        case OP_POP:
            seg = ram_readb(ip++);
            w1 = ram_read(ip);
            ip += 2;

            switch (seg) {
            case SEG_CONST:
                fprintf(stderr, "pop from constant segment\n");
                exit(1);
            case SEG_LOCAL:
                w2 = pop();
                ram_write(lcp + w1 * 2, w2);
                break;
            default:
                fprintf(stderr, "unknown segment %d\n", seg);
                exit(1);
            }
            break;
        case OP_ADD:
            w2 = pop();
            w1 = pop();
            push(w1 + w2);
            break;
        case OP_SUB:
            w2 = pop();
            w1 = pop();
            push(w1 - w2);
            break;
        case OP_MUL:
            w2 = pop();
            w1 = pop();
            push(w1 * w2);
            break;
        case OP_DIV:
            w2 = pop();
            w1 = pop();
            push(w1 / w2);
            break;
        case OP_MOD:
            w2 = pop();
            w1 = pop();
            push(w1 % w2);
            break;
        case OP_EQ:
            w2 = pop();
            w1 = pop();
            push(w1 == w2);
            break;
        case OP_LT:
            w2 = pop();
            w1 = pop();
            push(w1 < w2);
            break;
        case OP_GT:
            w2 = pop();
            w1 = pop();
            push(w1 > w2);
            break;
        case OP_JMP:
            ip = pop();
            break;
        case OP_CJMP:
            w1 = pop();
            w2 = pop();
            if (w2 == 1) {
                ip = w1;
            }
            break;
        case OP_HALT:
            halted = 1;
            break;
        default:
            fprintf(stderr, "unknown opcode: %d\n", op);
            return 1;
        }

        dump_mem("Stack", stack_start, sp - stack_start);
        dump_mem("Local", local_start, 10);
        printf("\n");
    }

    return 0;
}
