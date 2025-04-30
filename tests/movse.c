void test_movse()
{
    struct test_case {
        char *title;
        uint16_t a;
        uint16_t expect;
    };

    struct test_case cases[] = {
        {
            .title = "extends sign if sign bit is set",
            .a = 0xab80,
            .expect = 0xff80
        },
        {
            .title = "does not extend sign if sign bit is not set",
            .a = 0xab7f,
            .expect = 0x007f
        }
    };

    printf("test_movse\n");

    for (int i = 0; i < arrlen(cases); ++i) {
        struct test_case tcase = cases[i];

        printf("    %s\n", tcase.title);
        reset_vm();

        regfile[R10] = tcase.a;
        movse(R10, R10);
        halt();

        pc = 0;
        vm_start();

        assert(regfile[R10] == tcase.expect);
    }
}
