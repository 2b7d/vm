struct addb_test_case {
    char *title;
    uint16_t a;
    uint16_t b;
    uint16_t expect;
};

struct addb_flag_test_case {
    char *title;
    uint16_t a;
    uint16_t b;
    enum vm_flag flag;
    int expect;
};

struct addb_test_case addb_cases[] = {
    {
        .title = "add zero",
        .a = 0xab01,
        .b = 0xab00,
        .expect = 0xab01
    },
    {
        .title = "add positive",
        .a = 0xab01,
        .b = 0xab01,
        .expect = 0xab02
    },
    {
        .title = "add negative",
        .a = 0xab01,
        .b = 0xabff,
        .expect = 0xab00
    }
};

struct addb_flag_test_case addb_flag_cases[] = {
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
        .a = 0xabff,
        .b = 1,
        .flag = CF,
        .expect = 1
    },
    {
        .title = "carry flag is not set",
        .a = 0xabfe,
        .b = 1,
        .flag = CF,
        .expect = 0
    },
    {
        .title = "overflow (positive) flag is set",
        .a = 0xab7f,
        .b = 1,
        .flag = OF,
        .expect = 1
    },
    {
        .title = "overflow (positive) falg is not set",
        .a = 0xab7e,
        .b = 1,
        .flag = OF,
        .expect = 0
    },
    {
        .title = "overflow (negative) flag is set",
        .a = 0xab80,
        .b = 0xabff,
        .flag = OF,
        .expect = 1
    },
    {
        .title = "overflow (negative) flag is not set",
        .a = 0xab81,
        .b = 0xabff,
        .flag = OF,
        .expect = 0
    }
};

void test_addb()
{
    printf("test_addb\n");

    for (int i = 0; i < arrlen(addb_cases); ++i) {
        struct addb_test_case tcase = addb_cases[i];

        printf("    %s\n", tcase.title);
        reset_vm();

        regfile[R10] = tcase.a;
        regfile[R11] = tcase.b;
        addb(R11, R10);
        halt();

        pc = 0;
        vm_start();

        assert(regfile[R10] == tcase.expect);
    }

    for (int i = 0; i < arrlen(addb_flag_cases); ++i) {
        struct addb_flag_test_case tcase = addb_flag_cases[i];

        printf("    %s\n", tcase.title);
        reset_vm();

        regfile[R10] = tcase.a;
        regfile[R11] = tcase.b;
        addb(R11, R10);
        halt();

        pc = 0;
        vm_start();

        assert(flags[tcase.flag] == tcase.expect);
    }
}
