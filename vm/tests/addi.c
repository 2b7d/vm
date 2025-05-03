void test_addi()
{
    printf("test_addi\n");

    for (int i = 0; i < arrlen(add_cases); ++i) {
        struct add_test_case tcase = add_cases[i];

        printf("    %s\n", tcase.title);
        reset_vm();

        regfile[R10] = tcase.a;
        addi(tcase.b, R10);
        halt();

        pc = 0;
        vm_start();

        assert(regfile[R10] == tcase.expect);
    }
}

