#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "vm.h"

enum {
    RAM_CAP = 1 << 16
};

enum vm_flag {
    ZF,
    SF,
    CF,
    OF,

    VM_FLAG_COUNT
};

uint8_t ram[RAM_CAP];
uint16_t regfile[VM_REGISTER_COUNT];
int flags[VM_FLAG_COUNT];
int pc;

void write_byte(uint8_t val, uint16_t addr)
{
    ram[addr] = val;
}

void write_word(uint16_t val, uint16_t addr)
{
    ram[addr] = val;
    ram[addr + 1] = val >> 8;
}

uint8_t read_byte(uint16_t addr)
{
    return ram[addr];
}

uint16_t read_word(uint16_t addr)
{
    uint8_t lsb, msb;

    lsb = ram[addr];
    msb = ram[addr + 1];

    return (msb << 8) | lsb;
}

void stack_push(uint16_t val)
{
    regfile[RSP] -= 2;
    write_word(val, regfile[RSP]);
}

uint16_t stack_pop(void)
{
    uint16_t val;

    val = read_word(regfile[RSP]);
    regfile[RSP] += 2;

    return val;
}

void decode_registers(uint8_t byte, enum vm_register *r1, enum vm_register *r2)
{
    *r1 = byte >> 4;
    *r2 = byte & 0x0f;
}

void register_write_byte(enum vm_register r, uint8_t val)
{
    regfile[r] = (regfile[r] & 0xff00) | val;
}

void set_flags(int16_t a, int16_t b, int16_t t)
{
    flags[ZF] = t == 0;
    flags[SF] = t < 0;
    flags[OF] = (a < 0 && b >= 0 && t >= 0) || (a >= 0 && b < 0 && t < 0);
    flags[CF] = (uint16_t) t < (uint16_t) a;
}

// TODO(art), 25.04.25: return status code or something
void vm_start(void)
{
    for (;;) {
        uint8_t opcode;
        int saved_pc;

        saved_pc = pc;
        opcode = read_byte(pc++);

        switch (opcode) {
        case HALT:
            return;

        case MOV: {
            enum vm_register r1, r2;

            decode_registers(read_byte(pc++), &r1, &r2);
            regfile[r2] = regfile[r1];
        } break;

        case MOVI: {
            uint16_t imm;
            enum vm_register r1;

            imm = read_word(pc++); pc++;
            r1 = read_byte(pc++);

            regfile[r1] = imm;
        } break;

        case MOVB: {
            enum vm_register r1, r2;

            decode_registers(read_byte(pc++), &r1, &r2);
            register_write_byte(r2, regfile[r1]);
        } break;

        case MOVBI: {
            uint8_t imm;
            enum vm_register r1;

            imm = read_byte(pc++);
            r1 = read_byte(pc++);

            register_write_byte(r1, imm);
        } break;

        case MOVZE: {
            enum vm_register r1, r2;

            decode_registers(read_byte(pc++), &r1, &r2);
            regfile[r2] = (uint8_t) regfile[r1];
        } break;

        case MOVSE: {
            enum vm_register r1, r2;

            decode_registers(read_byte(pc++), &r1, &r2);
            regfile[r2] = (int8_t) regfile[r1];
        } break;

        case ST: {
            enum vm_register r1, r2;

            decode_registers(read_byte(pc++), &r1, &r2);
            write_word(regfile[r1], regfile[r2]);
        } break;

        case STI: {
            enum vm_register r1;
            uint16_t imm;

            r1 = read_byte(pc++);
            imm = read_word(pc++); pc++;
            write_word(regfile[r1], imm);
        } break;

        case STB: {
            enum vm_register r1, r2;

            decode_registers(read_byte(pc++), &r1, &r2);
            write_byte(regfile[r1], regfile[r2]);
        } break;

        case STBI: {
            enum vm_register r1;
            uint16_t imm;

            r1 = read_byte(pc++);
            imm = read_word(pc++); pc++;
            write_byte(regfile[r1], imm);
        } break;

        case LD: {
            enum vm_register r1, r2;

            decode_registers(read_byte(pc++), &r1, &r2);
            regfile[r2] = read_word(regfile[r1]);
        } break;

        case LDI: {
            enum vm_register r1;
            uint16_t imm;

            imm = read_word(pc++); pc++;
            r1 = read_byte(pc++);
            regfile[r1] = read_word(imm);
        } break;

        case LDB: {
            enum vm_register r1, r2;

            decode_registers(read_byte(pc++), &r1, &r2);
            register_write_byte(r2, read_byte(regfile[r1]));
        } break;

        case LDBI: {
            enum vm_register r1;
            uint16_t imm;

            imm = read_word(pc++); pc++;
            r1 = read_byte(pc++);
            register_write_byte(r1, read_byte(imm));
        } break;

        case ADD: {
            enum vm_register r1, r2;

            decode_registers(read_byte(pc++), &r1, &r2);
            regfile[r2] = regfile[r2] + regfile[r1];
        } break;

        case ADDI: {
            enum vm_register r1;
            uint16_t imm;

            imm = read_word(pc++); pc++;
            r1 = read_byte(pc++);

            regfile[r1] = regfile[r1] + imm;
        } break;

        case ADDB: {
            enum vm_register r1, r2;

            decode_registers(read_byte(pc++), &r1, &r2);
            register_write_byte(r2, regfile[r2] + regfile[r1]);
        } break;

        case ADDBI: {
            enum vm_register r1;
            uint8_t imm;

            imm = read_byte(pc++);
            r1 = read_byte(pc++);

            register_write_byte(r1, regfile[r1] + imm);
        } break;

        case SUB: {
            enum vm_register r1, r2;

            decode_registers(read_byte(pc++), &r1, &r2);
            regfile[r2] = regfile[r2] - regfile[r1];
        } break;

        case SUBI: {
            enum vm_register r1;
            uint16_t imm;

            imm = read_word(pc++); pc++;
            r1 = read_byte(pc++);

            regfile[r1] = regfile[r1] - imm;
        } break;

        case SUBB: {
            enum vm_register r1, r2;

            decode_registers(read_byte(pc++), &r1, &r2);
            register_write_byte(r2, regfile[r2] - regfile[r1]);
        } break;

        case SUBBI: {
            enum vm_register r1;
            uint8_t imm;

            imm = read_byte(pc++);
            r1 = read_byte(pc++);

            register_write_byte(r1, regfile[r1] - imm);
        } break;

        case NOT: {
            enum vm_register r1;

            r1 = read_byte(pc++);
            regfile[r1] = ~regfile[r1];
        } break;

        case NOTB: {
            enum vm_register r1;

            r1 = read_byte(pc++);
            register_write_byte(r1, ~regfile[r1]);
        } break;

        case AND: {
            enum vm_register r1, r2;

            decode_registers(read_byte(pc++), &r1, &r2);
            regfile[r2] = regfile[r2] & regfile[r1];
        } break;

        case ANDI: {
            enum vm_register r1;
            uint16_t imm;

            imm = read_word(pc++); pc++;
            r1 = read_byte(pc++);

            regfile[r1] = regfile[r1] & imm;
        } break;

        case ANDB: {
            enum vm_register r1, r2;

            decode_registers(read_byte(pc++), &r1, &r2);
            register_write_byte(r2, regfile[r2] & regfile[r1]);
        } break;

        case ANDBI: {
            enum vm_register r1;
            uint8_t imm;

            imm = read_byte(pc++);
            r1 = read_byte(pc++);

            register_write_byte(r1, regfile[r1] & imm);
        } break;

        case OR: {
            enum vm_register r1, r2;

            decode_registers(read_byte(pc++), &r1, &r2);
            regfile[r2] = regfile[r2] | regfile[r1];
        } break;

        case ORI: {
            enum vm_register r1;
            uint16_t imm;

            imm = read_word(pc++); pc++;
            r1 = read_byte(pc++);

            regfile[r1] = regfile[r1] | imm;
        } break;

        case ORB: {
            enum vm_register r1, r2;

            decode_registers(read_byte(pc++), &r1, &r2);
            register_write_byte(r2, regfile[r2] | regfile[r1]);
        } break;

        case ORBI: {
            enum vm_register r1;
            uint8_t imm;

            imm = read_byte(pc++);
            r1 = read_byte(pc++);

            register_write_byte(r1, regfile[r1] | imm);
        } break;

        case XOR: {
            enum vm_register r1, r2;

            decode_registers(read_byte(pc++), &r1, &r2);
            regfile[r2] = regfile[r2] ^ regfile[r1];
        } break;

        case XORI: {
            enum vm_register r1;
            uint16_t imm;

            imm = read_word(pc++); pc++;
            r1 = read_byte(pc++);

            regfile[r1] = regfile[r1] ^ imm;
        } break;

        case XORB: {
            enum vm_register r1, r2;

            decode_registers(read_byte(pc++), &r1, &r2);
            register_write_byte(r2, regfile[r2] ^ regfile[r1]);
        } break;

        case XORBI: {
            enum vm_register r1;
            uint8_t imm;

            imm = read_byte(pc++);
            r1 = read_byte(pc++);

            register_write_byte(r1, regfile[r1] ^ imm);
        } break;

        case SHL: {
            enum vm_register r1, r2;

            decode_registers(read_byte(pc++), &r1, &r2);
            regfile[r2] = regfile[r2] << regfile[r1];
        } break;

        case SHLI: {
            enum vm_register r1;
            uint8_t imm;

            imm = read_byte(pc++);
            r1 = read_byte(pc++);

            regfile[r1] = regfile[r1] << imm;
        } break;

        case SHLB: {
            enum vm_register r1, r2;
            uint8_t a;

            decode_registers(read_byte(pc++), &r1, &r2);
            a = regfile[r1];

            register_write_byte(r2, regfile[r2] << a);
        } break;

        case SHLBI: {
            enum vm_register r1;
            uint8_t imm;

            imm = read_byte(pc++);
            r1 = read_byte(pc++);

            register_write_byte(r1, regfile[r1] << imm);
        } break;

        case SHR: {
            enum vm_register r1, r2;

            decode_registers(read_byte(pc++), &r1, &r2);
            regfile[r2] = regfile[r2] >> regfile[r1];
        } break;

        case SHRI: {
            enum vm_register r1;
            uint8_t imm;

            imm = read_byte(pc++);
            r1 = read_byte(pc++);

            regfile[r1] = regfile[r1] >> imm;
        } break;

        case SHRB: {
            enum vm_register r1, r2;
            uint8_t a, b;

            decode_registers(read_byte(pc++), &r1, &r2);
            a = regfile[r2];
            b = regfile[r1];

            register_write_byte(r2, a >> b);
        } break;

        case SHRBI: {
            enum vm_register r1;
            uint8_t a, imm;

            imm = read_byte(pc++);
            r1 = read_byte(pc++);
            a = regfile[r1];

            register_write_byte(r1, a >> imm);
        } break;

        case SHRA: {
            enum vm_register r1, r2;
            int16_t a, b;

            decode_registers(read_byte(pc++), &r1, &r2);
            a = regfile[r2];
            b = regfile[r1];

            regfile[r2] = a >> b;
        } break;

        case SHRAI: {
            enum vm_register r1;
            int16_t a;
            int8_t imm;

            imm = read_byte(pc++);
            r1 = read_byte(pc++);
            a = regfile[r1];

            regfile[r1] = a >> imm;
        } break;

        case SHRAB: {
            enum vm_register r1, r2;
            int8_t a, b;

            decode_registers(read_byte(pc++), &r1, &r2);
            a = regfile[r2];
            b = regfile[r1];

            register_write_byte(r2, a >> b);
        } break;

        case SHRABI: {
            enum vm_register r1;
            int8_t a, imm;

            imm = read_byte(pc++);
            r1 = read_byte(pc++);
            a = regfile[r1];

            register_write_byte(r1, a >> imm);
        } break;

        case CMP: {
            enum vm_register r1, r2;
            int16_t a, b, t;

            decode_registers(read_byte(pc++), &r1, &r2);

            a = regfile[r2];
            b = regfile[r1];
            t = a - b;

            set_flags(a, b, t);
        } break;

        case CMPI: {
            enum vm_register r1;
            int16_t a, b, t;

            b = read_word(pc++); pc++;
            r1 = read_byte(pc++);

            a = regfile[r1];
            t = a - b;

            set_flags(a, b, t);
        } break;

        case CMPB: {
            enum vm_register r1, r2;
            int8_t a, b, t;

            decode_registers(read_byte(pc++), &r1, &r2);

            a = regfile[r2];
            b = regfile[r1];
            t = a - b;

            set_flags(a, b, t);
        } break;

        case CMPBI: {
            enum vm_register r1;
            int8_t a, b, t;

            b = read_byte(pc++);
            r1 = read_byte(pc++);

            a = regfile[r1];
            t = a - b;

            set_flags(a, b, t);
        } break;

        case JABS:
            pc = read_word(pc);
            break;

        case JE: {
            if (flags[ZF] == 1) {
                pc = read_word(pc);
            } else {
                pc += 2;
            }
        } break;

        case JNE: {
            if (flags[ZF] == 0) {
                pc = read_word(pc);
            } else {
                pc += 2;
            }
        } break;

        case JG: {
            if (!(flags[SF] ^ flags[OF]) && !flags[ZF]) {
                pc = read_word(pc);
            } else {
                pc += 2;
            }
        } break;

        case JGE: {
            if (!(flags[SF] ^ flags[OF])) {
                pc = read_word(pc);
            } else {
                pc += 2;
            }
        } break;

        case JL: {
            if (flags[SF] ^ flags[OF]) {
                pc = read_word(pc);
            } else {
                pc += 2;
            }
        } break;

        case JLE: {
            if ((flags[SF] ^ flags[OF]) || flags[ZF]) {
                pc = read_word(pc);
            } else {
                pc += 2;
            }
        } break;

        case JA: {
            if (flags[CF] && !flags[ZF]) {
                pc = read_word(pc);
            } else {
                pc += 2;
            }
        } break;

        case JAE: {
            if (flags[CF]) {
                pc = read_word(pc);
            } else {
                pc += 2;
            }
        } break;

        case JB: {
            if (!flags[CF]) {
                pc = read_word(pc);
            } else {
                pc += 2;
            }
        } break;

        case JBE: {
            if (!flags[CF] || flags[ZF]) {
                pc = read_word(pc);
            } else {
                pc += 2;
            }
        } break;

        case PUSH: {
            enum vm_register r1;

            r1 = read_byte(pc++);
            stack_push(regfile[r1]);
        } break;

        case PUSHI: {
            uint16_t imm;

            imm = read_word(pc++); pc++;
            stack_push(imm);
        } break;

        case POP: {
            enum vm_register r1;

            r1 = read_byte(pc++);
            regfile[r1] = stack_pop();
        } break;

        case CALL: {
            uint16_t imm;

            imm = read_word(pc++); pc++;
            stack_push(pc);
            pc = imm;
        } break;

        case CALLR: {
            enum vm_register r1;

            r1 = read_byte(pc++);
            stack_push(pc);
            pc = regfile[r1];
        } break;

        case RET:
            pc = stack_pop();
            break;

        default:
            fprintf(stderr, "unknown opcode `%02x` at ram[%d]\n", opcode, saved_pc);
            return;
        }
    }
}

#ifndef TEST

int main(void)
{
    memset(ram, 0, sizeof(ram));
    memset(regfile, 0, sizeof(regfile));
    memset(&flags, 0, sizeof(flags));
    pc = 0;

    printf("ram {");
    for (int i = 0; i < pc; ++i) {
        printf("%02x", ram[i]);
        if (i < pc - 1) {
            printf(", ");
        }
    }
    printf("}\n");

    vm_start();

    printf("regfile {");
    for (int i = 0; i < VM_REGISTER_COUNT; ++i) {
        printf("%04x", regfile[i]);
        if (i < VM_REGISTER_COUNT - 1) {
            printf(", ");
        }
    }
    printf("}\n");

    return 0;
}

#else
#include "tests/main.c"
#endif
