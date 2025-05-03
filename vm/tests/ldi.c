void test_ldi()
{
    printf("test_ldi\n");
    reset_vm();

    write_word(0xabcd, 100);

    ldi(100, R10);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[R10] == 0xabcd);
}

