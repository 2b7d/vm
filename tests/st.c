void test_st()
{
    printf("test_st\n");
    reset_vm();

    regfile[R10] = 0x6969;
    regfile[R11] = 69;

    st(R10, R11);
    halt();

    pc = 0;
    vm_start();

    assert(read_word(regfile[R11]) == regfile[R10]);
}
