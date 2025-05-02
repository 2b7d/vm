void test_xori()
{
    printf("test_xori\n");
    reset_vm();

    regfile[R10] = 0xffff;
    xori(0x0f0f, R10);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[R10] == 0xf0f0);
}
