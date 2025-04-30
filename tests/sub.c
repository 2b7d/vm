struct sub_test_case {
    char *title;
    uint16_t a;
    uint16_t b;
    enum vm_flag flag;
    uint16_t expect_result;
    int expect_flag;
};

struct sub_test_case sub_cases[] = {
    {
        .title = "zf 1",
        .a = 1,
        .b = 1,
        .flag = ZF,
        .expect_result = 0,
        .expect_flag = 1
    },
    {
        .title = "zf 0",
        .a = 2,
        .b = 1,
        .flag = ZF,
        .expect_result = 1,
        .expect_flag = 0
    },
    {
        .title = "nf 1",
        .a = 0,
        .b = 1,
        .flag = SF,
        .expect_result = -1,
        .expect_flag = 1
    },
    {
        .title = "nf 0",
        .a = 1,
        .b = 1,
        .flag = SF,
        .expect_result = 0,
        .expect_flag = 0
    },
    {
        .title = "uof 1",
        .a = 5,
        .b = 3,
        .flag = CF,
        .expect_result = 2,
        .expect_flag = 1
    },
    {
        .title = "uof 0",
        .a = 1,
        .b = 2,
        .flag = CF,
        .expect_result = -1,
        .expect_flag = 0
    },
    {
        .title = "sof pos 1",
        .a = 0,
        .b = 0x8000,
        .flag = OF,
        .expect_result = 0x8000,
        .expect_flag = 1
    },
    {
        .title = "sof pos 0",
        .a = 0,
        .b = 1,
        .flag = OF,
        .expect_result = -1,
        .expect_flag = 0
    },
    {
        .title = "sof neg 1",
        .a = 0x8000,
        .b = 1,
        .flag = OF,
        .expect_result = 0x7fff,
        .expect_flag = 1
    },
    {
        .title = "sof neg 0",
        .a = 0x8001,
        .b = 1,
        .flag = OF,
        .expect_result = 0x8000,
        .expect_flag = 0
    }
};

void test_sub()
{
    printf("test_sub\n");

    for (int i = 0; i < arrlen(sub_cases); ++i) {
        struct sub_test_case tcase = sub_cases[i];

        printf("    %s\n", tcase.title);
        reset_vm();

        regfile[R10] = tcase.a;
        regfile[R11] = tcase.b;
        sub(R11, R10);
        halt();

        pc = 0;
        vm_start();

        assert(regfile[R10] == tcase.expect_result);
        assert(flags[tcase.flag] == tcase.expect_flag);
    }
}
