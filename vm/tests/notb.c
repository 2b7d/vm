void test_notb()
{
    printf("test_notb\n");
    reset_vm();

    regfile[R10] = 0xabf0;
    notb(R10);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[R10] == 0xab0f);
}
