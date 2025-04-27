void test_ldb()
{
    printf("test_ldb\n");
    reset_vm();

    write_byte(0x69, 69);
    regfile[R10] = 69;

    ldb(R10, R11);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[R11] == read_byte(69));
}
