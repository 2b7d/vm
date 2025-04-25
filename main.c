#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define RAM_CAP 1<<16

enum vm_opcode {
    HALT,

    MOV,   // src dst
    MOVI,  // imm16 dst
    MOVB,  // src dst
    MOVBE, // src dst

    ADD,   // src dst
    ADDI,  // imm16 dst
    ADDB,  // src dst
    ADDBI, // imm8 dst
    INC,   // dst
    INCB,  // dst

    SUB,   // src dst
    SUBI,  // imm16 dst
    SUBB,  // src dst
    SUBBI, // imm8 dst
    DEC,   // dst
    DECB   // dst
};

enum vm_register {
    R0,
    R1,
    R2,

    RCOUNT
};

struct register_flags {
    int zero;
    int negative;
    int sign_overflow;
    int unsign_overflow;
};

uint8_t ram[RAM_CAP];
uint16_t regfile[RCOUNT];
struct register_flags regflags;
int pc;

void write_byte(uint8_t val)
{
    ram[pc++] = val;
}

void write_word(uint16_t val)
{
    ram[pc++] = val;
    ram[pc++] = val >> 8;
}

uint8_t read_byte()
{
    return ram[pc++];
}

uint16_t read_word()
{
    uint8_t lsb, msb;

    lsb = ram[pc++];
    msb = ram[pc++];

    return (msb << 8) | lsb;
}

void set_register_flags(int a, int b, int t)
{
    regflags.zero = t == 0;
    regflags.negative = t < 0;
    regflags.sign_overflow = (a < 0 && b < 0 && t >= 0) || (a >= 0 && b >= 0 && t < 0);
    regflags.unsign_overflow = (unsigned) t < (unsigned) a;
}

// TODO(art), 25.04.25: return status code or something
void vm_start()
{
    for (;;) {
        uint8_t opcode;
        int saved_pc;

        saved_pc = pc;
        opcode = read_byte();

        switch (opcode) {
        case HALT:
            return;

        case MOV: {
            enum vm_register src, dst;

            src = read_byte();
            dst = read_byte();

            regfile[dst] = regfile[src];
        } break;

        case MOVI: {
            uint16_t imm;
            enum vm_register dst;

            imm = read_word();
            dst = read_byte();

            regfile[dst] = imm;
        } break;

        case MOVB: {
            enum vm_register src, dst;

            src = read_byte();
            dst = read_byte();

            regfile[dst] = (uint8_t) regfile[src];
        } break;

        case MOVBE: {
            enum vm_register src, dst;

            src = read_byte();
            dst = read_byte();

            regfile[dst] = (int8_t) regfile[src];
        } break;

        case ADD: {
            enum vm_register src, dst;
            int16_t a, b, t;

            src = read_byte();
            dst = read_byte();

            a = regfile[dst];
            b = regfile[src];
            t = a + b;

            regfile[dst] = t;
            set_register_flags(a, b, t);
        } break;

        case ADDI: {
            enum vm_register dst;
            int16_t a, b, t;

            b = read_word();
            dst = read_byte();

            a = regfile[dst];
            t = a + b;

            regfile[dst] = t;
            set_register_flags(a, b, t);
        } break;

        case ADDB: {
            enum vm_register src, dst;
            int8_t a, b, t;

            src = read_byte();
            dst = read_byte();

            a = regfile[dst];
            b = regfile[src];
            t = a + b;

            regfile[dst] = (uint8_t) t;
            set_register_flags(a, b, t);
        } break;

        case ADDBI: {
            enum vm_register dst;
            int8_t a, b, t;

            b = read_byte();
            dst = read_byte();

            a = regfile[dst];
            t = a + b;

            regfile[dst] = (uint8_t) t;
            set_register_flags(a, b, t);
        } break;

        case INC: {
            enum vm_register dst;
            int16_t a, b, t;

            dst = read_byte();

            a = regfile[dst];
            b = 1;
            t = a + b;

            regfile[dst] = t;
            set_register_flags(a, b, t);
        } break;

        case INCB: {
            enum vm_register dst;
            int8_t a, b, t;

            dst = read_byte();

            a = regfile[dst];
            b = 1;
            t = a + b;

            regfile[dst] = (uint8_t) t;
            set_register_flags(a, b, t);
        } break;

        case SUB: {
            enum vm_register src, dst;
            int16_t a, b, t;

            src = read_byte();
            dst = read_byte();

            a = regfile[dst];
            b = -regfile[src];
            t = a + b;

            regfile[dst] = t;
            set_register_flags(a, b, t);
        } break;

        case SUBI: {
            enum vm_register dst;
            int16_t a, b, t;

            b = read_word();
            dst = read_byte();

            a = regfile[dst];
            b = -b;
            t = a + b;

            regfile[dst] = t;
            set_register_flags(a, b, t);
        } break;

        case SUBB: {
            enum vm_register src, dst;
            int8_t a, b, t;

            src = read_byte();
            dst = read_byte();

            a = regfile[dst];
            b = -regfile[src];
            t = a + b;

            regfile[dst] = (uint8_t) t;
            set_register_flags(a, b, t);
        } break;

        case SUBBI: {
            enum vm_register dst;
            int8_t a, b, t;

            b = read_byte();
            dst = read_byte();

            a = regfile[dst];
            b = -b;
            t = a + b;

            regfile[dst] = (uint8_t) t;
            set_register_flags(a, b, t);
        } break;

        case DEC: {
            enum vm_register dst;
            int16_t a, b, t;

            dst = read_byte();

            a = regfile[dst];
            b = -1;
            t = a + b;

            regfile[dst] = t;
            set_register_flags(a, b, t);
        } break;

        case DECB: {
            enum vm_register dst;
            int8_t a, b, t;

            dst = read_byte();

            a = regfile[dst];
            b = -1;
            t = a + b;

            regfile[dst] = (uint8_t) t;
            set_register_flags(a, b, t);
        } break;

        default:
            fprintf(stderr, "unknown opcode `%02x` at ram[%d]\n", opcode, saved_pc);
            return;
        }
    }

}

int run_tests();

int main(void)
{
    memset(ram, 0, sizeof(ram));
    memset(regfile, 0, sizeof(regfile));
    memset(&regflags, 0, sizeof(regflags));
    pc = 0;

#ifdef TEST
    return run_tests();
#endif

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
    for (int i = 0; i < RCOUNT; ++i) {
        printf("%04x", regfile[i]);
        if (i < RCOUNT - 1) {
            printf(", ");
        }
    }
    printf("}\n");

    return 0;
}

#ifdef TEST
#include <assert.h>

#define halt() write_byte(HALT)

#define mov(s, d) write_byte(MOV), write_byte((s)), write_byte((d))
#define movi(v, r) write_byte(MOVI), write_word((v)), write_byte((r))
#define movb(s, d) write_byte(MOVB), write_byte((s)), write_byte((d))
#define movbe(s, d) write_byte(MOVBE), write_byte((s)), write_byte((d))

#define add(s, d) write_byte(ADD), write_byte((s)), write_byte((d))
#define addi(v, d) write_byte(ADDI), write_word((v)), write_byte((d))
#define addb(s, d) write_byte(ADDB), write_byte((s)), write_byte((d))
#define addbi(v, d) write_byte(ADDBI), write_byte((v)), write_byte((d))
#define inc(d) write_byte(INC), write_byte((d))
#define incb(d) write_byte(INCB), write_byte((d))

#define sub(s, d) write_byte(SUB), write_byte((s)), write_byte((d))
#define subi(v, d) write_byte(SUBI), write_word((v)), write_byte((d))
#define subb(s, d) write_byte(SUBB), write_byte((s)), write_byte((d))
#define subbi(v, d) write_byte(SUBBI), write_byte((v)), write_byte((d))
#define dec(d) write_byte(DEC), write_byte((d))
#define decb(d) write_byte(DECB), write_byte((d))

#define arrlen(arr) sizeof((arr)) / sizeof(*(arr))

void reset_vm()
{
    memset(ram, 0, sizeof(ram));
    memset(regfile, 0, sizeof(regfile));
    memset(&regflags, 0, sizeof(regflags));
    pc = 0;
}

void test_mov()
{
    printf("test_mov\n");
    reset_vm();

    movi(0xaabb, R0);
    mov(R0, R1);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[R1] == 0xaabb);
}

void test_movi()
{
    printf("test_movi\n");
    reset_vm();

    movi(0xaabb, R0);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[R0] == 0xaabb);
}

void test_movb()
{
    printf("test_movb\n");
    reset_vm();

    movi(0x80, R0);
    movb(R0, R1);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[R1] == 0x0080);
}

void test_movbe()
{
    struct test_case {
        char *title;
        uint8_t a;
        uint16_t expect;
    };

    struct test_case cases[] = {
        {
            .title = "extend sign",
            .a = 0x80,
            .expect = 0xff80
        },
        {
            .title = "do not extend sign",
            .a = 0x7f,
            .expect = 0x007f
        },
    };

    printf("test_movbe\n");

    for (int i = 0; i < arrlen(cases); ++i) {
        struct test_case tcase;

        tcase = cases[i];

        printf("    %s\n", tcase.title);
        reset_vm();

        movi(tcase.a, R0);
        movbe(R0, R1);
        halt();

        pc = 0;
        vm_start();

        assert(regfile[R1] == tcase.expect);
    }
}

void test_add_addi()
{
    struct test_case {
        char *title;
        uint16_t a;
        uint16_t b;
        uint16_t expect_result;
        int *expect_flag;
    };

    struct test_case cases[] = {
        {
            .title = "normal",
            .a = 3,
            .b = 5,
            .expect_result = 8,
            .expect_flag = NULL
        },
        {
            .title = "zero flag set",
            .a = 0,
            .b = 0,
            .expect_result = 0,
            .expect_flag = &regflags.zero
        },
        {
            .title = "negative flag set",
            .a = -5,
            .b = 3,
            .expect_result = -2,
            .expect_flag = &regflags.negative
        },
        {
            .title = "unsigned overflow flag set",
            .a = 0xffff,
            .b = 1,
            .expect_result = 0,
            .expect_flag = &regflags.unsign_overflow
        },
        {
            .title = "signed overflow flag set (positive)",
            .a = 0x7fff,
            .b = 1,
            .expect_result = 0x8000,
            .expect_flag = &regflags.sign_overflow
        },
        {
            .title = "signed overflow flag set (negative)",
            .a = 0x8000,
            .b = 0xffff,
            .expect_result = 0x7fff,
            .expect_flag = &regflags.sign_overflow
        }
    };

    printf("test_add\n");

    for (int i = 0; i < arrlen(cases); ++i) {
        struct test_case tcase;

        tcase = cases[i];

        printf("    %s\n", tcase.title);
        reset_vm();

        movi(tcase.a, R0);
        movi(tcase.b, R1);
        add(R1, R0);
        halt();

        pc = 0;
        vm_start();

        assert(regfile[R0] == tcase.expect_result);
        if (tcase.expect_flag != NULL) {
            assert(*tcase.expect_flag == 1);
        }
    }

    printf("test_addi\n");

    for (int i = 0; i < arrlen(cases); ++i) {
        struct test_case tcase;

        tcase = cases[i];

        printf("    %s\n", tcase.title);
        reset_vm();

        movi(tcase.a, R0);
        addi(tcase.b, R0);
        halt();

        pc = 0;
        vm_start();

        assert(regfile[R0] == tcase.expect_result);
        if (tcase.expect_flag != NULL) {
            assert(*tcase.expect_flag == 1);
        }
    }
}

void test_addb_addbi()
{
    struct test_case {
        char *title;
        uint8_t a;
        uint8_t b;
        uint8_t expect_result;
        int *expect_flag;
    };

    struct test_case cases[] = {
        {
            .title = "normal",
            .a = 3,
            .b = 5,
            .expect_result = 8,
            .expect_flag = NULL
        },
        {
            .title = "zero flag set",
            .a = 0,
            .b = 0,
            .expect_result = 0,
            .expect_flag = &regflags.zero
        },
        {
            .title = "negative flag set",
            .a = -5,
            .b = 3,
            .expect_result = -2,
            .expect_flag = &regflags.negative
        },
        {
            .title = "unsigned overflow flag set",
            .a = 0xff,
            .b = 1,
            .expect_result = 0,
            .expect_flag = &regflags.unsign_overflow
        },
        {
            .title = "signed overflow flag set (positive)",
            .a = 0x7f,
            .b = 1,
            .expect_result = 0x80,
            .expect_flag = &regflags.sign_overflow
        },
        {
            .title = "signed overflow flag set (negative)",
            .a = 0x80,
            .b = 0xff,
            .expect_result = 0x7f,
            .expect_flag = &regflags.sign_overflow
        }
    };

    printf("test_addb\n");

    for (int i = 0; i < arrlen(cases); ++i) {
        struct test_case tcase;

        tcase = cases[i];

        printf("    %s\n", tcase.title);
        reset_vm();

        movi(tcase.a, R0);
        movi(tcase.b, R1);
        addb(R1, R0);
        halt();

        pc = 0;
        vm_start();

        assert(regfile[R0] == tcase.expect_result);
        if (tcase.expect_flag != NULL) {
            assert(*tcase.expect_flag == 1);
        }
    }

    printf("test_addbi\n");

    for (int i = 0; i < arrlen(cases); ++i) {
        struct test_case tcase;

        tcase = cases[i];

        printf("    %s\n", tcase.title);
        reset_vm();

        movi(tcase.a, R0);
        addbi(tcase.b, R0);
        halt();

        pc = 0;
        vm_start();

        assert(regfile[R0] == tcase.expect_result);
        if (tcase.expect_flag != NULL) {
            assert(*tcase.expect_flag == 1);
        }
    }
}

void test_inc()
{
    struct test_case {
        char *title;
        uint16_t a;
        uint16_t expect_result;
        int *expect_flag;
    };

    struct test_case cases[] = {
        {
            .title = "normal",
            .a = 5,
            .expect_result = 6,
            .expect_flag = NULL
        },
        {
            .title = "zero flag set",
            .a = -1,
            .expect_result = 0,
            .expect_flag = &regflags.zero
        },
        {
            .title = "negative flag set",
            .a = -2,
            .expect_result = -1,
            .expect_flag = &regflags.negative
        },
        {
            .title = "unsigned overflow flag set",
            .a = 0xffff,
            .expect_result = 0,
            .expect_flag = &regflags.unsign_overflow
        },
        {
            .title = "signed overflow flag set (positive)",
            .a = 0x7fff,
            .expect_result = 0x8000,
            .expect_flag = &regflags.sign_overflow
        }
    };

    printf("test_inc\n");

    for (int i = 0; i < arrlen(cases); ++i) {
        struct test_case tcase;

        tcase = cases[i];

        printf("    %s\n", tcase.title);
        reset_vm();

        movi(tcase.a, R0);
        inc(R0);
        halt();

        pc = 0;
        vm_start();

        assert(regfile[R0] == tcase.expect_result);
        if (tcase.expect_flag != NULL) {
            assert(*tcase.expect_flag == 1);
        }
    }
}

void test_incb()
{
    struct test_case {
        char *title;
        uint8_t a;
        uint8_t expect_result;
        int *expect_flag;
    };

    struct test_case cases[] = {
        {
            .title = "normal",
            .a = 5,
            .expect_result = 6,
            .expect_flag = NULL
        },
        {
            .title = "zero flag set",
            .a = -1,
            .expect_result = 0,
            .expect_flag = &regflags.zero
        },
        {
            .title = "negative flag set",
            .a = -2,
            .expect_result = -1,
            .expect_flag = &regflags.negative
        },
        {
            .title = "unsigned overflow flag set",
            .a = 0xff,
            .expect_result = 0,
            .expect_flag = &regflags.unsign_overflow
        },
        {
            .title = "signed overflow flag set (positive)",
            .a = 0x7f,
            .expect_result = 0x80,
            .expect_flag = &regflags.sign_overflow
        }
    };

    printf("test_incb\n");

    for (int i = 0; i < arrlen(cases); ++i) {
        struct test_case tcase;

        tcase = cases[i];

        printf("    %s\n", tcase.title);
        reset_vm();

        movi(tcase.a, R0);
        incb(R0);
        halt();

        pc = 0;
        vm_start();

        assert(regfile[R0] == tcase.expect_result);
        if (tcase.expect_flag != NULL) {
            assert(*tcase.expect_flag == 1);
        }
    }
}

void test_sub_subi()
{
    struct test_case {
        char *title;
        uint16_t a;
        uint16_t b;
        uint16_t expect_result;
        int *expect_flag;
    };

    struct test_case cases[] = {
        {
            .title = "normal",
            .a = 5,
            .b = 3,
            .expect_result = 2,
            .expect_flag = NULL
        },
        {
            .title = "zero flag set",
            .a = 1,
            .b = 1,
            .expect_result = 0,
            .expect_flag = &regflags.zero
        },
        {
            .title = "negative flag set",
            .a = 3,
            .b = 5,
            .expect_result = -2,
            .expect_flag = &regflags.negative
        },
        {
            .title = "unsigned overflow flag set",
            .a = 5,
            .b = 3,
            .expect_result = 2,
            .expect_flag = &regflags.unsign_overflow
        },
        {
            .title = "signed overflow flag set (positive)",
            .a = 0x7fff,
            .b = -1,
            .expect_result = 0x8000,
            .expect_flag = &regflags.sign_overflow
        },
        {
            .title = "signed overflow flag set (negative)",
            .a = 0x8000,
            .b = 1,
            .expect_result = 0x7fff,
            .expect_flag = &regflags.sign_overflow
        }
    };

    printf("test_sub\n");

    for (int i = 0; i < arrlen(cases); ++i) {
        struct test_case tcase;

        tcase = cases[i];

        printf("    %s\n", tcase.title);
        reset_vm();

        movi(tcase.a, R0);
        movi(tcase.b, R1);
        sub(R1, R0);
        halt();

        pc = 0;
        vm_start();

        assert(regfile[R0] == tcase.expect_result);
        if (tcase.expect_flag != NULL) {
            assert(*tcase.expect_flag == 1);
        }
    }

    printf("test_subi\n");

    for (int i = 0; i < arrlen(cases); ++i) {
        struct test_case tcase;

        tcase = cases[i];

        printf("    %s\n", tcase.title);
        reset_vm();

        movi(tcase.a, R0);
        subi(tcase.b, R0);
        halt();

        pc = 0;
        vm_start();

        assert(regfile[R0] == tcase.expect_result);
        if (tcase.expect_flag != NULL) {
            assert(*tcase.expect_flag == 1);
        }
    }
}

void test_subb_subbi()
{
    struct test_case {
        char *title;
        uint8_t a;
        uint8_t b;
        uint8_t expect_result;
        int *expect_flag;
    };

    struct test_case cases[] = {
        {
            .title = "normal",
            .a = 5,
            .b = 3,
            .expect_result = 2,
            .expect_flag = NULL
        },
        {
            .title = "zero flag set",
            .a = 1,
            .b = 1,
            .expect_result = 0,
            .expect_flag = &regflags.zero
        },
        {
            .title = "negative flag set",
            .a = 3,
            .b = 5,
            .expect_result = -2,
            .expect_flag = &regflags.negative
        },
        {
            .title = "unsigned overflow flag set",
            .a = 5,
            .b = 3,
            .expect_result = 2,
            .expect_flag = &regflags.unsign_overflow
        },
        {
            .title = "signed overflow flag set (positive)",
            .a = 0x7f,
            .b = -1,
            .expect_result = 0x80,
            .expect_flag = &regflags.sign_overflow
        },
        {
            .title = "signed overflow flag set (negative)",
            .a = 0x80,
            .b = 1,
            .expect_result = 0x7f,
            .expect_flag = &regflags.sign_overflow
        }
    };

    printf("test_subb\n");

    for (int i = 0; i < arrlen(cases); ++i) {
        struct test_case tcase;

        tcase = cases[i];

        printf("    %s\n", tcase.title);
        reset_vm();

        movi(tcase.a, R0);
        movi(tcase.b, R1);
        subb(R1, R0);
        halt();

        pc = 0;
        vm_start();

        assert(regfile[R0] == tcase.expect_result);
        if (tcase.expect_flag != NULL) {
            assert(*tcase.expect_flag == 1);
        }
    }

    printf("test_subbi\n");

    for (int i = 0; i < arrlen(cases); ++i) {
        struct test_case tcase;

        tcase = cases[i];

        printf("    %s\n", tcase.title);
        reset_vm();

        movi(tcase.a, R0);
        subbi(tcase.b, R0);
        halt();

        pc = 0;
        vm_start();

        assert(regfile[R0] == tcase.expect_result);
        if (tcase.expect_flag != NULL) {
            assert(*tcase.expect_flag == 1);
        }
    }
}

void test_dec()
{
    struct test_case {
        char *title;
        uint16_t a;
        uint16_t expect_result;
        int *expect_flag;
    };

    struct test_case cases[] = {
        {
            .title = "normal",
            .a = 5,
            .expect_result = 4,
            .expect_flag = NULL
        },
        {
            .title = "zero flag set",
            .a = 1,
            .expect_result = 0,
            .expect_flag = &regflags.zero
        },
        {
            .title = "negative flag set",
            .a = 0,
            .expect_result = -1,
            .expect_flag = &regflags.negative
        },
        {
            .title = "unsigned overflow flag set",
            .a = 1,
            .expect_result = 0,
            .expect_flag = &regflags.unsign_overflow
        },
        {
            .title = "signed overflow flag set (negative)",
            .a = 0x8000,
            .expect_result = 0x7fff,
            .expect_flag = &regflags.sign_overflow
        }
    };

    printf("test_dec\n");

    for (int i = 0; i < arrlen(cases); ++i) {
        struct test_case tcase;

        tcase = cases[i];

        printf("    %s\n", tcase.title);
        reset_vm();

        movi(tcase.a, R0);
        dec(R0);
        halt();

        pc = 0;
        vm_start();

        assert(regfile[R0] == tcase.expect_result);
        if (tcase.expect_flag != NULL) {
            assert(*tcase.expect_flag == 1);
        }
    }
}

void test_decb()
{
    struct test_case {
        char *title;
        uint8_t a;
        uint8_t expect_result;
        int *expect_flag;
    };

    struct test_case cases[] = {
        {
            .title = "normal",
            .a = 5,
            .expect_result = 4,
            .expect_flag = NULL
        },
        {
            .title = "zero flag set",
            .a = 1,
            .expect_result = 0,
            .expect_flag = &regflags.zero
        },
        {
            .title = "negative flag set",
            .a = 0,
            .expect_result = -1,
            .expect_flag = &regflags.negative
        },
        {
            .title = "unsigned overflow flag set",
            .a = 1,
            .expect_result = 0,
            .expect_flag = &regflags.unsign_overflow
        },
        {
            .title = "signed overflow flag set (negative)",
            .a = 0x80,
            .expect_result = 0x7f,
            .expect_flag = &regflags.sign_overflow
        }
    };

    printf("test_decb\n");

    for (int i = 0; i < arrlen(cases); ++i) {
        struct test_case tcase;

        tcase = cases[i];

        printf("    %s\n", tcase.title);
        reset_vm();

        movi(tcase.a, R0);
        decb(R0);
        halt();

        pc = 0;
        vm_start();

        assert(regfile[R0] == tcase.expect_result);
        if (tcase.expect_flag != NULL) {
            assert(*tcase.expect_flag == 1);
        }
    }
}

int run_tests()
{
    test_mov();
    test_movi();
    test_movb();
    test_movbe();

    test_add_addi();
    test_addb_addbi();
    test_inc();
    test_incb();

    test_sub_subi();
    test_subb_subbi();
    test_dec();
    test_decb();

    return 0;
}
#endif
