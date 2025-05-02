void test_xor()
{
    printf("test_xor\n");
    reset_vm();

    regfile[R10] = 0xffff;
    regfile[R11] = 0x0f0f;
    xor(R11, R10);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[R10] == 0xf0f0);
}
