struct cmpb_test_case {
    char *title;
    uint16_t a;
    uint16_t b;
    enum vm_flag flag;
    int expect;
};

struct cmpb_test_case cmpb_cases[] = {
    {
        .title = "zero flag is set",
        .a = 1,
        .b = 1,
        .flag = ZF,
        .expect = 1
    },
    {
        .title = "zero flag is not set",
        .a = 2,
        .b = 1,
        .flag = ZF,
        .expect = 0
    },
    {
        .title = "sign flag is set",
        .a = 0,
        .b = 1,
        .flag = SF,
        .expect = 1
    },
    {
        .title = "sign flag is not set",
        .a = 1,
        .b = 1,
        .flag = SF,
        .expect = 0
    },
    {
        .title = "carry flag is set",
        .a = 5,
        .b = 3,
        .flag = CF,
        .expect = 1
    },
    {
        .title = "carry flag is not set",
        .a = 1,
        .b = 2,
        .flag = CF,
        .expect = 0
    },
    {
        .title = "overflow (positive) flag is set",
        .a = 0,
        .b = 0xab80,
        .flag = OF,
        .expect = 1
    },
    {
        .title = "overflow (positive) flag is not set",
        .a = 0,
        .b = 1,
        .flag = OF,
        .expect = 0
    },
    {
        .title = "overflow (negative) flag is set",
        .a = 0xab80,
        .b = 1,
        .flag = OF,
        .expect = 1
    },
    {
        .title = "overflow (negative) flag is not set",
        .a = 0xab81,
        .b = 1,
        .flag = OF,
        .expect = 0
    }
};
void test_cmpb()
{
    printf("test_cmpb\n");

    for (int i = 0; i < arrlen(cmpb_cases); ++i) {
        struct cmpb_test_case tcase = cmpb_cases[i];

        printf("    %s\n", tcase.title);
        reset_vm();

        regfile[R10] = tcase.a;
        regfile[R11] = tcase.b;
        cmpb(R11, R10);
        halt();

        pc = 0;
        vm_start();

        assert(flags[tcase.flag] == tcase.expect);
    }
}
