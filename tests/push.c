void test_push()
{
    printf("test_push\n");
    reset_vm();

    movi(69, R10);
    push(R10);
    movi(420, R10);
    push(R10);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[RSP] == RAM_CAP - (2 * 2));
    assert(read_word(RAM_CAP - (2 * 1)) == 69);
    assert(read_word(RAM_CAP - (2 * 2)) == 420);
}
