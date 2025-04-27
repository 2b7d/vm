void test_ld()
{
    printf("test_ld\n");
    reset_vm();

    write_word(0x6969, 69);
    regfile[R10] = 69;

    ld(R10, R11);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[R11] == read_word(69));
}
