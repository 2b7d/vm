void test_ld()
{
    printf("test_ld\n");
    reset_vm();

    write_word(0xabcd, 100);
    regfile[R10] = 100;

    ld(R10, R11);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[R11] == 0xabcd);
}
