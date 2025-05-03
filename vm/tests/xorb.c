void test_xorb()
{
    printf("test_xorb\n");
    reset_vm();

    regfile[R10] = 0xabff;
    regfile[R11] = 0xab0f;
    xorb(R11, R10);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[R10] == 0xabf0);
}
