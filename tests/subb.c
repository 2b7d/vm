struct subb_test_case {
    char *title;
    uint16_t a;
    uint16_t b;
    uint16_t expect_result;
    enum vm_flag expect_flag;
};

struct subb_test_case subb_cases[] = {
    {
        .title = "sets zero flag",
        .a = 0xab01,
        .b = 0xab01,
        .expect_result = 0xab00,
        .expect_flag = ZF,
    },
    {
        .title = "sets negative flag",
        .a = 0xab00,
        .b = 0xab01,
        .expect_result = 0xabff,
        .expect_flag = SF,
    },
    {
        .title = "sets unsigned overflow flag",
        .a = 0xab05,
        .b = 0xab03,
        .expect_result = 0xab02,
        .expect_flag = CF,
    },
    {
        .title = "sets signed overflow flag (positive)",
        .a = 0xab7f,
        .b = 0xabff,
        .expect_result = 0xab80,
        .expect_flag = OF,
    },
    {
        .title = "sets signed overflow flag (negative)",
        .a = 0xab80,
        .b = 0xab01,
        .expect_result = 0xab7f,
        .expect_flag = OF,
    }
};

void test_subb()
{
    printf("test_subb\n");

    for (int i = 0; i < arrlen(subb_cases); ++i) {
        struct subb_test_case tcase = subb_cases[i];

        printf("    %s\n", tcase.title);
        reset_vm();

        regfile[R10] = tcase.a;
        regfile[R11] = tcase.b;
        subb(R11, R10);
        halt();

        pc = 0;
        vm_start();

        assert(regfile[R10] == tcase.expect_result);
        assert(flags[tcase.expect_flag] == 1);
    }
}
