void test_shli()
{
    printf("test_shli\n");
    reset_vm();

    regfile[R10] = 0xffff;
    shli(4, R10);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[R10] == 0xfff0);
}
