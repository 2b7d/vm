void test_subbi()
{
    printf("test_subbi\n");

    for (int i = 0; i < arrlen(subb_cases); ++i) {
        struct subb_test_case tcase = subb_cases[i];

        printf("    %s\n", tcase.title);
        reset_vm();

        regfile[R10] = tcase.a;
        subbi(tcase.b, R10);
        halt();

        pc = 0;
        vm_start();

        assert(regfile[R10] == tcase.expect_result);
        assert(flags[tcase.expect_flag] == 1);
    }
}

