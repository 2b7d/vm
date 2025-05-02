struct addb_test_case {
    char *title;
    uint16_t a;
    uint16_t b;
    uint16_t expect;
};

struct addb_test_case addb_cases[] = {
    {
        .title = "add zero",
        .a = 0xab01,
        .b = 0xab00,
        .expect = 0xab01
    },
    {
        .title = "add positive",
        .a = 0xab01,
        .b = 0xab01,
        .expect = 0xab02
    },
    {
        .title = "add negative",
        .a = 0xab01,
        .b = 0xabff,
        .expect = 0xab00
    }
};

void test_addb()
{
    printf("test_addb\n");

    for (int i = 0; i < arrlen(addb_cases); ++i) {
        struct addb_test_case tcase = addb_cases[i];

        printf("    %s\n", tcase.title);
        reset_vm();

        regfile[R10] = tcase.a;
        regfile[R11] = tcase.b;
        addb(R11, R10);
        halt();

        pc = 0;
        vm_start();

        assert(regfile[R10] == tcase.expect);
    }
}
