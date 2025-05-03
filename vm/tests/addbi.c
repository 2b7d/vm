void test_addbi()
{
    printf("test_addbi\n");

    for (int i = 0; i < arrlen(addb_cases); ++i) {
        struct addb_test_case tcase = addb_cases[i];

        printf("    %s\n", tcase.title);
        reset_vm();

        regfile[R10] = tcase.a;
        addbi(tcase.b, R10);
        halt();

        pc = 0;
        vm_start();

        assert(regfile[R10] == tcase.expect);
    }
}

