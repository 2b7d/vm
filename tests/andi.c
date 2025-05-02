void test_andi()
{
    printf("test_andi\n");
    reset_vm();

    regfile[R10] = 0xabcd;
    andi(0x00ff, R10);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[R10] == 0x00cd);
}
