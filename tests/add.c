struct add_test_case {
    char *title;
    uint16_t a;
    uint16_t b;
    uint16_t expect;
};

struct add_flag_test_case {
    char *title;
    uint16_t a;
    uint16_t b;
    enum vm_flag flag;
    int expect;
};

struct add_test_case add_cases[] = {
    {
        .title = "add zero",
        .a = 1,
        .b = 0,
        .expect = 1
    },
    {
        .title = "add positive",
        .a = 1,
        .b = 1,
        .expect = 2
    },
    {
        .title = "add negative",
        .a = 1,
        .b = -1,
        .expect = 0
    }
};

struct add_flag_test_case add_flag_cases[] = {
    {
        .title = "zero flag is set",
        .a = 0,
        .b = 0,
        .flag = ZF,
        .expect = 1
    },
    {
        .title = "zero flag is not set",
        .a = 1,
        .b = 0,
        .flag = ZF,
        .expect = 0
    },
    {
        .title = "sign flag is set",
        .a = 0,
        .b = -1,
        .flag = SF,
        .expect = 1
    },
    {
        .title = "sign flag is not set",
        .a = 1,
        .b = -1,
        .flag = SF,
        .expect = 0
    },
    {
        .title = "carry flag is set",
        .a = 0xffff,
        .b = 1,
        .flag = CF,
        .expect = 1
    },
    {
        .title = "carry flag is not set",
        .a = 0xfffe,
        .b = 1,
        .flag = CF,
        .expect = 0
    },
    {
        .title = "overflow (positive) flag is set",
        .a = 0x7fff,
        .b = 1,
        .flag = OF,
        .expect = 1
    },
    {
        .title = "overflow (positive) falg is not set",
        .a = 0x7ffe,
        .b = 1,
        .flag = OF,
        .expect = 0
    },
    {
        .title = "overflow (negative) flag is set",
        .a = 0x8000,
        .b = 0xffff,
        .flag = OF,
        .expect = 1
    },
    {
        .title = "overflow (negative) flag is not set",
        .a = 0x8001,
        .b = 0xffff,
        .flag = OF,
        .expect = 0
    }
};

void test_add()
{
    printf("test_add\n");

    for (int i = 0; i < arrlen(add_cases); ++i) {
        struct add_test_case tcase = add_cases[i];

        printf("    %s\n", tcase.title);
        reset_vm();

        regfile[R10] = tcase.a;
        regfile[R11] = tcase.b;
        add(R11, R10);
        halt();

        pc = 0;
        vm_start();

        assert(regfile[R10] == tcase.expect);
    }

    for (int i = 0; i < arrlen(add_flag_cases); ++i) {
        struct add_flag_test_case tcase = add_flag_cases[i];

        printf("    %s\n", tcase.title);
        reset_vm();

        regfile[R10] = tcase.a;
        regfile[R11] = tcase.b;
        add(R11, R10);
        halt();

        pc = 0;
        vm_start();

        assert(flags[tcase.flag] == tcase.expect);
    }
}
