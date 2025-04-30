#include <stdio.h>
#include <stdint.h>
#include <string.h>

enum {
    RAM_CAP = 1 << 16
};

enum vm_opcode {
    HALT,

    MOV,   // reg4 reg4
    MOVI,  // imm16 reg8
    MOVB,  // reg4 reg4
    MOVBI, // imm8 reg8
    MOVZE, // reg4 reg4
    MOVSE, // reg4 reg4

    ST,   // reg4 reg4
    STI,  // reg8 imm16
    STB,  // reg4 reg4
    STBI, // reg8 imm16

    LD,   // reg4 reg4
    LDI,  // imm16 reg8
    LDB,  // reg4 reg4
    LDBI, // imm16 reg8

    ADD,   // reg4 reg4
    ADDI,  // imm16 reg8
    ADDB,  // reg4 reg4
    ADDBI, // imm8 reg8

    SUB,   // reg4 reg4
    SUBI,  // imm16 reg8
    SUBB,  // reg4 reg4
    SUBBI, // imm8 reg8

    /*NOT,*/
    /*AND,*/
    /*OR,*/
    /*XOR,*/
    /*SHL,*/
    /*SHR,*/
    /*SHRA,*/

    CMP,   // reg4 reg4
    CMPI,  // imm16 reg8
    CMPB,  // reg4 reg4
    CMPBI, // imm8 reg8

    JABS, // imm16
    JE,   // imm16
    JNE,  // imm16
    JG,   // imm16
    JGE,  // imm16
    JL,   // imm16
    JLE,  // imm16
    JA,   // imm16
    JAE,  // imm16
    JB,   // imm16
    JBE,  // imm16

    PUSH,  // reg8
    PUSHI, // imm16
    POP,   // reg8

    CALL,  // imm16
    CALLR, // reg8
    RET,

    /*SYSCALL*/
};

enum vm_register {
    R0,
    R1,
    R2,
    R3,
    R4,
    R5,
    R6,
    R7,
    R8,
    R9,
    R10,
    R11,
    R12,
    R13,

    RSP,
    RBP,

    VM_REGISTER_COUNT
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

void set_flags_add(int16_t a, int16_t b, int16_t t)
{
    flags[ZF] = t == 0;
    flags[SF] = t < 0;
    flags[OF] = (a < 0 && b < 0 && t >= 0) || (a >= 0 && b >= 0 && t < 0);
    flags[CF] = (uint16_t) t < (uint16_t) a;
}

void set_flags_sub(int16_t a, int16_t b, int16_t t)
{
    flags[ZF] = t == 0;
    flags[SF] = t < 0;
    flags[OF] = (a < 0 && b >= 0 && t >= 0) || (a >= 0 && b < 0 && t < 0);
    flags[CF] = (uint16_t) t < (uint16_t) a;
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
            int16_t a, b, t;

            decode_registers(read_byte(pc++), &r1, &r2);

            a = regfile[r2];
            b = regfile[r1];
            t = a + b;

            regfile[r2] = t;
            set_flags_add(a, b, t);
        } break;

        case ADDI: {
            enum vm_register r1;
            int16_t a, b, t;

            b = read_word(pc++); pc++;
            r1 = read_byte(pc++);

            a = regfile[r1];
            t = a + b;

            regfile[r1] = t;
            set_flags_add(a, b, t);
        } break;

        case ADDB: {
            enum vm_register r1, r2;
            int8_t a, b, t;

            decode_registers(read_byte(pc++), &r1, &r2);

            a = regfile[r2];
            b = regfile[r1];
            t = a + b;

            register_write_byte(r2, t);
            set_flags_add(a, b, t);
        } break;

        case ADDBI: {
            enum vm_register r1;
            int8_t a, b, t;

            b = read_byte(pc++);
            r1 = read_byte(pc++);

            a = regfile[r1];
            t = a + b;

            register_write_byte(r1, t);
            set_flags_add(a, b, t);
        } break;

        case SUB: {
            enum vm_register r1, r2;
            int16_t a, b, t;

            decode_registers(read_byte(pc++), &r1, &r2);

            a = regfile[r2];
            b = regfile[r1];
            t = a - b;

            regfile[r2] = t;
            set_flags_sub(a, b, t);
        } break;

        case SUBI: {
            enum vm_register r1;
            int16_t a, b, t;

            b = read_word(pc++); pc++;
            r1 = read_byte(pc++);

            a = regfile[r1];
            t = a - b;

            regfile[r1] = t;
            set_flags_sub(a, b, t);
        } break;

        case SUBB: {
            enum vm_register r1, r2;
            int8_t a, b, t;

            decode_registers(read_byte(pc++), &r1, &r2);

            a = regfile[r2];
            b = regfile[r1];
            t = a - b;

            register_write_byte(r2, t);
            set_flags_sub(a, b, t);
        } break;

        case SUBBI: {
            enum vm_register r1;
            int8_t a, b, t;

            b = read_byte(pc++);
            r1 = read_byte(pc++);

            a = regfile[r1];
            t = a - b;

            register_write_byte(r1, t);
            set_flags_sub(a, b, t);
        } break;

        case CMP: {
            enum vm_register r1, r2;
            int16_t a, b, t;

            decode_registers(read_byte(pc++), &r1, &r2);

            a = regfile[r2];
            b = regfile[r1];
            t = a - b;

            set_flags_sub(a, b, t);
        } break;

        case CMPI: {
            enum vm_register r1;
            int16_t a, b, t;

            b = read_word(pc++); pc++;
            r1 = read_byte(pc++);

            a = regfile[r1];
            t = a - b;

            set_flags_sub(a, b, t);
        } break;

        case CMPB: {
            enum vm_register r1, r2;
            int8_t a, b, t;

            decode_registers(read_byte(pc++), &r1, &r2);

            a = regfile[r2];
            b = regfile[r1];
            t = a - b;

            set_flags_sub(a, b, t);
        } break;

        case CMPBI: {
            enum vm_register r1;
            int8_t a, b, t;

            b = read_byte(pc++);
            r1 = read_byte(pc++);

            a = regfile[r1];
            t = a - b;

            set_flags_sub(a, b, t);
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
