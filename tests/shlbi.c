void test_shlbi()
{
    printf("test_shlbi\n");
    reset_vm();

    regfile[R10] = 0xabff;
    shlbi(4, R10);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[R10] == 0xabf0);
}
