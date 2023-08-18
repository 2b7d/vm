#include <stdio.h>
#include <stdint.h>

#include "vm.h"

typedef uint8_t byte;
typedef uint16_t word;

enum {
    ROM_CAP = 1 << 16,
    RAM_CAP = 1 << 16
};

byte rom[ROM_CAP];
byte ram[RAM_CAP];
word ip;
word sp;

byte mem_readb(byte *mem, word addr)
{
    return mem[addr];
}

void mem_writeb(byte *mem, word addr, byte val)
{
    mem[addr] = val;
}

word mem_read(byte *mem, word addr)
{
    byte lsb, msb;

    lsb = mem_readb(mem, addr);
    msb = mem_readb(mem, addr + 1);

    return (msb << 8) | lsb;
}

void mem_write(byte *mem, word addr, word val)
{
    byte lsb, msb;

    lsb = val;
    msb = val >> 8;

    mem[addr] = lsb;
    mem[addr + 1] = msb;
}

void pushb(byte val)
{
    mem_writeb(ram, sp, val);
    sp++;
}

byte popb()
{
    sp--;
    return mem_readb(ram, sp);
}

void push(word val)
{
    mem_write(ram, sp, val);
    sp += 2;
}

word pop()
{
    sp -= 2;
    return mem_read(ram, sp);
}

void dump_ram(int start, int size)
{
    printf("Stack: ");
    for (int i = start; i < start + size; ++i) {
        printf("%d ", ram[i]);
    }
    printf("\n");
}

int main(int argc, char **argv)
{
    int halt, stack_start, nsecs;
    FILE *in;

    argc--;
    argv++;

    if (argc < 1) {
        fprintf(stderr, "Provide file to execute\n");
        return 1;
    }

    in = fopen(*argv, "r");
    if (in == NULL) {
        perror(NULL);
        return 1;
    }

    fread(&ip, 2, 1, in);
    fread(&nsecs, 2, 1, in);

    while (nsecs > 0) {
        enum vm_segment kind;
        int size;

        fread(&kind, 1, 1, in);
        fread(&size, 2, 1, in);

        switch (kind) {
        case SEGMENT_DATA:
            fread(ram, 1, size, in);
            sp = size;
            break;
        case SEGMENT_TEXT:
            fread(rom, 1, size, in);
            break;
        default:
            fprintf(stderr, "unknown segment kind %d\n", kind);
            return 1;
        }

        nsecs--;
    }

    if (ferror(in) == 1) {
        perror("failed to read from file");
        return 1;
    }

    halt = 0;
    stack_start = sp;

    dump_ram(stack_start, sp - stack_start);

    while (halt == 0) {
        enum vm_opcode op;
        byte b1, b2;
        word w1, w2;

        op = mem_readb(rom, ip++);

        switch (op) {
        case OP_HALT:
            halt = 1;
            break;

        case OP_PUSH:
            w1 = mem_read(rom, ip);
            ip += 2;
            push(w1);
            break;
        case OP_PUSHB:
            b1 = mem_readb(rom, ip++);
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

        dump_ram(stack_start, sp - stack_start);
    }

    return 0;

}
