void test_cmpbi()
{
    struct test_case {
        char *title;
        uint8_t a;
        uint8_t b;
        int *expect_flag;
    };

    struct test_case cases[] = {
        {
            .title = "sets zero flag",
            .a = 1,
            .b = 1,
            .expect_flag = &regflags.zero
        },
        {
            .title = "sets negative flag",
            .a = 3,
            .b = 5,
            .expect_flag = &regflags.negative
        },
        {
            .title = "sets unsigned overflow flag",
            .a = 5,
            .b = 3,
            .expect_flag = &regflags.unsign_overflow
        },
        {
            .title = "sets signed overflow flag (positive)",
            .a = 0x7f,
            .b = -1,
            .expect_flag = &regflags.sign_overflow
        },
        {
            .title = "sets signed overflow flag (negative)",
            .a = 0x80,
            .b = 1,
            .expect_flag = &regflags.sign_overflow
        }
    };

    printf("test_cmpbi\n");

    for (int i = 0; i < arrlen(cases); ++i) {
        struct test_case tcase = cases[i];

        printf("    %s\n", tcase.title);
        reset_vm();

        regfile[R10] = tcase.a;
        cmpbi(tcase.b, R10);
        halt();

        pc = 0;
        vm_start();

        assert(*tcase.expect_flag == 1);
    }
}
