void test_stb()
{
    printf("test_stb\n");
    reset_vm();

    regfile[R10] = 0xabcd;
    regfile[R11] = 100;

    stb(R10, R11);
    halt();

    pc = 0;
    vm_start();

    assert(read_byte(regfile[R11]) == 0xcd);
}
