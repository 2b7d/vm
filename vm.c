#include <stdio.h>
#include <stdint.h>

typedef uint8_t byte;
typedef uint16_t word;

enum {
    RAM_CAP = 1 << 16
};

enum vm_opcode {
    OP_HALT = 0,

    OP_PUSH,
    OP_PUSHB,

    OP_CTW,
    OP_CTB,

    OP_ADD,
    OP_ADDB,
    OP_SUB,
    OP_SUBB,
    OP_NEG,
    OP_NEGB,

    OP_EQ,
    OP_EQB,
    OP_LT,
    OP_LTB,
    OP_GT,
    OP_GTB,

    OP_JMP,
    OP_CJMP
};

byte ram[RAM_CAP];
word ip;
word sp;

byte ram_readb(word addr)
{
    return ram[addr];
}

void ram_writeb(word addr, byte val)
{
    ram[addr] = val;
}

word ram_read(word addr)
{
    byte lsb, msb;

    lsb = ram_readb(addr);
    msb = ram_readb(addr + 1);

    return (msb << 8) | lsb;
}

void ram_write(word addr, word val)
{
    byte lsb, msb;

    lsb = val;
    msb = val >> 8;

    ram[addr] = lsb;
    ram[addr + 1] = msb;
}

void pushb(byte val)
{
    ram_writeb(sp, val);
    sp++;
}

byte popb()
{
    sp--;
    return ram_readb(sp);
}

void push(word val)
{
    ram_write(sp, val);
    sp += 2;
}

word pop()
{
    sp -= 2;
    return ram_read(sp);
}

void dump_mem(int start, int size)
{
    printf("Stack: ");
    for (int i = start; i < start + size; ++i) {
        printf("%d ", ram[i]);
    }
    printf("\n");
}

int main(void)
{
    int halt, i, stack_start;

    halt = 0;
    i = 0;

    ram_writeb(i, OP_PUSHB); i++;
    ram_writeb(i, 10); i++;

    ram_writeb(i, OP_PUSHB); i++;
    ram_writeb(i, 5); i++;

    ram_writeb(i, OP_ADDB); i++;

    ram_writeb(i, OP_CTW); i++;

    ram_writeb(i, OP_PUSH); i++;
    ram_write(i, 80); i += 2;

    ram_writeb(i, OP_ADD); i++;

    ram_writeb(i, OP_CTB); i++;

    ram_writeb(i, OP_HALT); i++;

    sp = i;
    stack_start = sp;

    dump_mem(stack_start, sp - stack_start);

    while (halt == 0) {
        enum vm_opcode op;
        byte b1, b2;
        word w1, w2;

        op = ram_readb(ip++);

        switch (op) {
        case OP_HALT:
            halt = 1;
            break;

        case OP_PUSH:
            w1 = ram_read(ip);
            ip += 2;
            push(w1);
            break;
        case OP_PUSHB:
            b1 = ram_readb(ip++);
            pushb(b1);
            break;

        case OP_CTW:
            w1 = popb();
            push(w1);
            break;
        case OP_CTB:
            b1 = pop();
            pushb(b1);
            break;

        case OP_ADD:
            w2 = pop();
            w1 = pop();
            push(w1 + w2);
            break;
        case OP_ADDB:
            b2 = popb();
            b1 = popb();
            pushb(b1 + b2);
            break;
        case OP_SUB:
            w2 = pop();
            w1 = pop();
            push(w1 - w2);
            break;
        case OP_SUBB:
            b2 = popb();
            b1 = popb();
            pushb(b1 - b2);
            break;
        case OP_NEG:
            w1 = pop();
            push(-w1);
            break;
        case OP_NEGB:
            b1 = popb();
            pushb(-b1);
            break;

        case OP_EQ:
            w2 = pop();
            w1 = pop();
            pushb(w1 == w2);
            break;
        case OP_EQB:
            b2 = popb();
            b1 = popb();
            pushb(b1 == b2);
            break;
        case OP_LT:
            w2 = pop();
            w1 = pop();
            pushb(w1 < w2);
            break;
        case OP_LTB:
            b2 = popb();
            b1 = popb();
            pushb(b1 < b2);
            break;
        case OP_GT:
            w2 = pop();
            w1 = pop();
            pushb(w1 > w2);
            break;
        case OP_GTB:
            b2 = popb();
            b1 = popb();
            pushb(b1 > b2);
            break;

        case OP_JMP:
            ip = pop();
            break;
        case OP_CJMP:
            w1 = pop();
            b1 = popb();
            if (b1 == 1) {
                ip = w1;
            }
            break;

        default:
            fprintf(stderr, "unknown opcode %d\n", op);
            return 1;
        }

        dump_mem(stack_start, sp - stack_start);
    }

    return 0;

}
