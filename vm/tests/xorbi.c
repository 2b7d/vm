void test_xorbi()
{
    printf("test_xorbi\n");
    reset_vm();

    regfile[R10] = 0xabff;
    xorbi(0x0f, R10);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[R10] == 0xabf0);
}
