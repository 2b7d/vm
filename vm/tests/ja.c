void test_ja()
{
    struct test_case {
        char *title;
        uint16_t a;
        uint16_t b;
        uint16_t expect;
    };

    struct test_case cases[] = {
        {
            .title = "jumps to address",
            .a = 5,
            .b = 4,
            .expect = 1
        },
        {
            .title = "does not jump to address",
            .a = 4,
            .b = 5,
            .expect = 0
        },
        {
            .title = "does not jump to address 2",
            .a = 5,
            .b = 5,
            .expect = 0
        }
    };

    printf("test_ja\n");

    for (int i = 0; i < arrlen(cases); ++i) {
        struct test_case tcase = cases[i];

        printf("    %s\n", tcase.title);
        reset_vm();

        regfile[R10] = tcase.a;
        cmpi(tcase.b, R10);
        ja(69);
        movi(0, R10);
        halt();

        pc = 69;
        movi(1, R10);
        halt();

        pc = 0;
        vm_start();

        assert(regfile[R10] == tcase.expect);
    }
}
