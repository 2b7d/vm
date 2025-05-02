struct sub_test_case {
    char *title;
    uint16_t a;
    uint16_t b;
    uint16_t expect;
};

struct sub_test_case sub_cases[] = {
    {
        .title = "sub zero",
        .a = 0,
        .b = 0,
        .expect = 0
    },
    {
        .title = "sub positive",
        .a = 0,
        .b = 1,
        .expect = -1
    },
    {
        .title = "sub negative",
        .a = 0,
        .b = -1,
        .expect = 1
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

        assert(regfile[R10] == tcase.expect);
    }
}
