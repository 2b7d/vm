void test_andb()
{
    printf("test_andb\n");
    reset_vm();

    regfile[R10] = 0xabcd;
    regfile[R11] = 0xab0f;
    andb(R11, R10);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[R10] == 0xab0d);
}
