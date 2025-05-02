void test_cmpi()
{
    printf("test_cmpi\n");

    for (int i = 0; i < arrlen(cmp_cases); ++i) {
        struct cmp_test_case tcase = cmp_cases[i];

        printf("    %s\n", tcase.title);
        reset_vm();

        regfile[R10] = tcase.a;
        cmpi(tcase.b, R10);
        halt();

        pc = 0;
        vm_start();

        assert(flags[tcase.flag] == tcase.expect);
    }
}

