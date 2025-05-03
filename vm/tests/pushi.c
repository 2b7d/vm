void test_pushi()
{
    printf("test_pushi\n");
    reset_vm();

    pushi(69);
    pushi(420);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[RSP] == RAM_CAP - (2 * 2));
    assert(read_word(RAM_CAP - (2 * 1)) == 69);
    assert(read_word(RAM_CAP - (2 * 2)) == 420);
}
