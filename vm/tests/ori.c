void test_ori()
{
    printf("test_ori\n");
    reset_vm();

    regfile[R10] = 0xff00;
    ori(0x00ff, R10);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[R10] == 0xffff);
}
