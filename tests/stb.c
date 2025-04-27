void test_stb()
{
    printf("test_stb\n");
    reset_vm();

    regfile[R10] = 0x69;
    regfile[R11] = 69;

    stb(R10, R11);
    halt();

    pc = 0;
    vm_start();

    assert(read_byte(regfile[R11]) == regfile[R10]);
}
