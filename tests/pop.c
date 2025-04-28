void test_pop()
{
    printf("test_pop\n");
    reset_vm();

    pushi(69);
    pushi(420);
    pop(R10);
    pop(R11);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[RSP] == 0);
    assert(regfile[R10] == 420);
    assert(regfile[R11] == 69);
}
