void test_cmpbi()
{
    printf("test_cmpbi\n");

    for (int i = 0; i < arrlen(cmpb_cases); ++i) {
        struct cmpb_test_case tcase = cmpb_cases[i];

        printf("    %s\n", tcase.title);
        reset_vm();

        regfile[R10] = tcase.a;
        cmpbi(tcase.b, R10);
        halt();

        pc = 0;
        vm_start();

        assert(flags[tcase.flag] == tcase.expect);
    }
}

