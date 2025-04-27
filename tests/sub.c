void test_sub()
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
            .title = "sets zero flag",
            .a = 1,
            .b = 1,
            .expect_result = 0,
            .expect_flag = &regflags.zero
        },
        {
            .title = "sets negative flag",
            .a = 3,
            .b = 5,
            .expect_result = -2,
            .expect_flag = &regflags.negative
        },
        {
            .title = "sets unsigned overflow flag",
            .a = 5,
            .b = 3,
            .expect_result = 2,
            .expect_flag = &regflags.unsign_overflow
        },
        {
            .title = "sets signed overflow flag (positive)",
            .a = 0x7fff,
            .b = -1,
            .expect_result = 0x8000,
            .expect_flag = &regflags.sign_overflow
        },
        {
            .title = "sets signed overflow flag (negative)",
            .a = 0x8000,
            .b = 1,
            .expect_result = 0x7fff,
            .expect_flag = &regflags.sign_overflow
        }
    };

    printf("test_sub\n");

    for (int i = 0; i < arrlen(cases); ++i) {
        struct test_case tcase = cases[i];

        printf("    %s\n", tcase.title);
        reset_vm();

        regfile[R10] = tcase.a;
        regfile[R11] = tcase.b;
        sub(R11, R10);
        halt();

        pc = 0;
        vm_start();

        assert(regfile[R10] == tcase.expect_result);
        assert(*tcase.expect_flag == 1);
    }
}
