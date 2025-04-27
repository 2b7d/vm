void test_ldi()
{
    printf("test_ldi\n");
    reset_vm();

    write_word(0x6969, 69);

    ldi(69, R10);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[R10] == read_word(69));
}
