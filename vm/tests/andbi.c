void test_andbi()
{
    printf("test_andbi\n");
    reset_vm();

    regfile[R10] = 0xabcd;
    andbi(0x0f, R10);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[R10] == 0xab0d);
}
