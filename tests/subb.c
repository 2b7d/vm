struct subb_test_case {
    char *title;
    uint16_t a;
    uint16_t b;
    uint16_t expect;
};

struct subb_test_case subb_cases[] = {
    {
        .title = "sub zero",
        .a = 0xab00,
        .b = 0xab00,
        .expect = 0xab00
    },
    {
        .title = "sub positive",
        .a = 0xab00,
        .b = 0xab01,
        .expect = 0xabff
    },
    {
        .title = "sub negative",
        .a = 0xab00,
        .b = 0xabff,
        .expect = 0xab01
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

        assert(regfile[R10] == tcase.expect);
    }
}
