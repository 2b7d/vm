void test_movbe()
{
    struct test_case {
        char *title;
        uint8_t a;
        uint16_t expect;
    };

    struct test_case cases[] = {
        {
            .title = "extends sign",
            .a = 0x80,
            .expect = 0xff80
        },
        {
            .title = "does not extend sign",
            .a = 0x7f,
            .expect = 0x007f
        }
    };

    printf("test_movbe\n");

    for (int i = 0; i < arrlen(cases); ++i) {
        struct test_case tcase = cases[i];

        printf("    %s\n", tcase.title);
        reset_vm();

        regfile[R10] = tcase.a;
        movbe(R10, R11);
        halt();

        pc = 0;
        vm_start();

        assert(regfile[R11] == tcase.expect);
    }
}
