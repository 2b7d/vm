void test_movze()
{
    struct test_case {
        char *title;
        uint16_t a;
        uint16_t expect;
    };

    struct test_case cases[] = {
        {
            .title = "extends with zero if sign bit is set",
            .a = 0xab80,
            .expect = 0x0080
        },
        {
            .title = "extends with zero if sign bit is not set",
            .a = 0xab7f,
            .expect = 0x007f
        }
    };

    printf("test_movze\n");

    for (int i = 0; i < arrlen(cases); ++i) {
        struct test_case tcase = cases[i];

        printf("    %s\n", tcase.title);
        reset_vm();

        regfile[R10] = tcase.a;
        movze(R10, R10);
        halt();

        pc = 0;
        vm_start();

        assert(regfile[R10] == tcase.expect);
    }
}
