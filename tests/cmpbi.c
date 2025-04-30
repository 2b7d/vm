void test_cmpbi()
{
    printf("test_cmpbi\n");

    for (int i = 0; i < arrlen(subb_cases); ++i) {
        struct subb_test_case tcase = subb_cases[i];

        printf("    %s\n", tcase.title);
        reset_vm();

        regfile[R10] = tcase.a;
        cmpbi(tcase.b, R10);
        halt();

        pc = 0;
        vm_start();

        assert(flags[tcase.expect_flag] == 1);
    }
}

