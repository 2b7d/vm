void test_cmpi()
{
    printf("test_cmpi\n");

    for (int i = 0; i < arrlen(sub_cases); ++i) {
        struct sub_test_case tcase = sub_cases[i];

        printf("    %s\n", tcase.title);
        reset_vm();

        regfile[R10] = tcase.a;
        cmpi(tcase.b, R10);
        halt();

        pc = 0;
        vm_start();

        assert(flags[tcase.flag] == tcase.expect_flag);
    }
}

