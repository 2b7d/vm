void test_sti()
{
    printf("test_sti\n");
    reset_vm();

    regfile[R10] = 0xabcd;

    sti(R10, 100);
    halt();

    pc = 0;
    vm_start();

    assert(read_word(100) == regfile[R10]);
}
