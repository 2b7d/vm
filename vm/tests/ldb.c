void test_ldb()
{
    printf("test_ldb\n");
    reset_vm();

    write_byte(0x80, 100);
    regfile[R10] = 100;
    regfile[R11] = 0xabcd;

    ldb(R10, R11);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[R11] == 0xab80);
}
