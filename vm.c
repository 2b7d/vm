#include <stdio.h>
#include <stdint.h>
//#include <string.h>
//#include <stdlib.h>

enum {
    RAM_CAP = 1 << 16,
    STACK_CAP = 1 << 8,
//    BRK = RAM_CAP - 2,
    RSP = RAM_CAP - 4
};

uint8_t ram[RAM_CAP];
uint8_t stack[STACK_CAP];

uint16_t pc;
uint8_t sp;
uint16_t rsp;

uint8_t ramload8(uint16_t addr)
{
    return ram[addr];
}

void ramstore8(uint16_t addr, uint8_t val)
{
    ram[addr] = val;
}

uint16_t ramload(uint16_t addr)
{
    uint8_t lsb, msb;

    lsb = ramload8(addr);
    msb = ramload8(addr + 1);

    return msb << 8 | lsb;
}

void ramstore(uint16_t addr, uint16_t val)
{
    uint8_t lsb, msb;

    lsb = val;
    msb = val >> 8;

    ram[addr] = lsb;
    ram[addr + 1] = msb;
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

int main(void)
{
    int halt = 0;
    uint8_t program[] = {};

    pc = 0;
    sp = 0;
    rsp = RSP;

    ramstore(0, 69);
    ramstore(2, 420);

    printf("%u\n", ramload(0));
    printf("%u\n", ramload(2));

    push(69);
    push(420);

    printf("%u\n", pop());
    printf("%u\n", pop());

    while (halt == 0) {
        uint16_t a, b;
        uint8_t a8, b8, op;

        op = ramload8(pc++);

        switch (op) {
        }

        break;
    }

    return 0;
}

//void retpush(uint16_t val)
//{
//
//    // TODO: handle overflow
//    ramstore(rsp--, val);
//}
//
//uint16_t retpop()
//{
//    // TODO: handle underflow
//    return ramload(++rsp);
//}
//
//void load_program(char *pathname)
//{
//    FILE *f;
//    size_t i = 0;
//
//    f = fopen(pathname, "r");
//    if (f == NULL) {
//        printf("ERROR: failed to open binary file\n");
//        exit(1);
//    }
//
//    fread(&pc, sizeof(uint16_t), 1, f);
//
//    for (;;) {
//        if (i >= RAM_CAP) {
//            printf("ERROR: not enough memory to load program\n");
//            exit(1);
//        }
//
//        fread(ram + i, sizeof(uint16_t), 1, f);
//        i++;
//
//        if (ferror(f) != 0) {
//            printf("ERROR: failed to read binary file\n");
//            exit(1);
//        }
//
//        if (feof(f) != 0) {
//            ramstore(BRK, i - 1);
//            break;
//        }
//    }
//
//    fclose(f);
//}
//
//int main(int argc, char **argv)
//{
//    int halt = 0;
//
//    argc--;
//    argv++;
//
//    if (argc < 1) {
//        printf("binary file is required\n");
//        return 1;
//    }
//
//    pc = 0;
//    sp = 0;
//    rsp = RSP;
//
//    load_program(*argv);
//
//    while (halt == 0) {
//        uint16_t a, b;
//        uint8_t op = ramload(pc++) & 0xff;
//
//        switch (op) {
//        case OP_ST:
//            a = pop();
//            ramstore(a, pop());
//            break;
//
//        case OP_LD:
//            push(ramload(pop()));
//            break;
//
//        case OP_PUSH:
//            push(ramload(pc++));
//            break;
//
//        case OP_ADD:
//            push(pop() + pop());
//            break;
//
//        case OP_SUB:
//            a = pop();
//            push(pop() - a);
//            break;
//
//        case OP_MUL:
//            push(pop() * pop());
//            break;
//
//        case OP_DIV:
//            a = pop();
//            push(pop() / a);
//            break;
//
//        case OP_MOD:
//            a = pop();
//            push(pop() % a);
//            break;
//
//        case OP_INC:
//            a = pop();
//            push(a + 1);
//            break;
//
//        case OP_DEC:
//            a = pop();
//            push(a - 1);
//            break;
//
//        case OP_DUP:
//            push(stack[sp - 1]);
//            break;
//
//        case OP_OVER:
//            push(stack[sp - 2]);
//            break;
//
//        case OP_SWAP:
//            a = pop();
//            b = pop();
//            push(a);
//            push(b);
//            break;
//
//        case OP_DROP:
//            pop();
//            break;
//
//        case OP_RSPUSH:
//            retpush(pop());
//            break;
//
//        case OP_RSPOP:
//            push(retpop());
//            break;
//
//        case OP_RSCOPY:
//            push(ramload(rsp + 1));
//            break;
//
//        case OP_RSDROP:
//            retpop();
//            break;
//
//        case OP_RSP:
//            push(rsp);
//            break;
//
//        case OP_RSPSET:
//            rsp = pop();
//            break;
//
//        case OP_BRK:
//            push(ramload(BRK));
//            break;
//
//        case OP_BRKSET:
//            a = ramload(BRK);
//            ramstore(BRK, pop());
//            push(a);
//            break;
//
//        case OP_EQ:
//            push(pop() == pop());
//            break;
//
//        case OP_GT:
//            a = pop();
//            push(pop() > a);
//            break;
//
//        case OP_LT:
//            a = pop();
//            push(pop() < a);
//            break;
//
//        case OP_OR:
//            a = pop();
//            b = pop();
//            push(a || b);
//            break;
//
//        case OP_AND:
//            a = pop();
//            b = pop();
//            push(a && b);
//            break;
//
//        case OP_NOT:
//            push(!pop());
//            break;
//
//        case OP_JMP:
//            pc = ramload(pc);
//            break;
//
//        case OP_JMPIF:
//            a = ramload(pc++);
//            if (pop() == 1) {
//                pc = a;
//            }
//            break;
//
//        case OP_HALT:
//            halt = 1;
//            break;
//
//        case OP_SYSCALL:
//            a = ramload(pc++);
//            switch (a) {
//            case SYS_WRITE:
//                a = pop();
//                b = pop();
//                for (size_t i = 0; i < a; ++i) {
//                    fputc(ramload(b + i), stdout);
//                }
//                break;
//
//            case SYS_READ:
//            default:
//                printf("ERROR: unknown vm_syscall(%u)\n", a);
//                exit(1);
//            }
//            break;
//
//        case OP_CALL:
//            a = ramload(pc++);
//            retpush(pc);
//            pc = a;
//            break;
//
//        case OP_RET:
//            pc = retpop();
//            break;
//
//        default:
//            printf("ERROR: unknown vm_opcode(%u)\n", op);
//            exit(1);
//        }
//    }
//
//    return 0;
//}
