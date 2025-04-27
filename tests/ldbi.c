void test_ldbi()
{
    printf("test_ldbi\n");
    reset_vm();

    write_byte(0x69, 69);

    ldbi(69, R10);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[R10] == read_byte(69));
}
