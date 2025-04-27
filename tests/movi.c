void test_movi()
{
    printf("test_movi\n");
    reset_vm();

    movi(0xaabb, R10);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[R10] == 0xaabb);
}
