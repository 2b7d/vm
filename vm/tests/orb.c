void test_orb()
{
    printf("test_orb\n");
    reset_vm();

    regfile[R10] = 0xabf0;
    regfile[R11] = 0xab0f;
    orb(R11, R10);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[R10] == 0xabff);
}
