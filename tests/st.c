void test_st()
{
    printf("test_st\n");
    reset_vm();

    regfile[R10] = 0xabcd;
    regfile[R11] = 100;

    st(R10, R11);
    halt();

    pc = 0;
    vm_start();

    assert(read_word(regfile[R11]) == regfile[R10]);
}
