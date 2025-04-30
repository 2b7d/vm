void test_ldbi()
{
    printf("test_ldbi\n");
    reset_vm();

    write_byte(0x80, 100);
    regfile[R10] = 0xabcd;

    ldbi(100, R10);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[R10] == 0xab80);
}

