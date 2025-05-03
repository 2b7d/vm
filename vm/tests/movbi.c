void test_movbi()
{
    printf("test_movbi\n");
    reset_vm();

    regfile[R10] = 0xab00;
    movbi(0x80, R10);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[R10] == 0xab80);
}

