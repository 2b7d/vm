void test_sti()
{
    printf("test_sti\n");
    reset_vm();

    regfile[R10] = 0x6969;

    sti(R10, 69);
    halt();

    pc = 0;
    vm_start();

    assert(read_word(69) == regfile[R10]);
}
