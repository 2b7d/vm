void test_orbi()
{
    printf("test_orbi\n");
    reset_vm();

    regfile[R10] = 0xabf0;
    orbi(0x0f, R10);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[R10] == 0xabff);
}
