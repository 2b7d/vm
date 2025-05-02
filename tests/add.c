struct add_test_case {
    char *title;
    uint16_t a;
    uint16_t b;
    uint16_t expect;
};

struct add_test_case add_cases[] = {
    {
        .title = "add zero",
        .a = 1,
        .b = 0,
        .expect = 1
    },
    {
        .title = "add positive",
        .a = 1,
        .b = 1,
        .expect = 2
    },
    {
        .title = "add negative",
        .a = 1,
        .b = -1,
        .expect = 0
    }
};

void test_add()
{
    printf("test_add\n");

    for (int i = 0; i < arrlen(add_cases); ++i) {
        struct add_test_case tcase = add_cases[i];

        printf("    %s\n", tcase.title);
        reset_vm();

        regfile[R10] = tcase.a;
        regfile[R11] = tcase.b;
        add(R11, R10);
        halt();

        pc = 0;
        vm_start();

        assert(regfile[R10] == tcase.expect);
    }
}
