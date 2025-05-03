void test_stbi()
{
    printf("test_stbi\n");
    reset_vm();

    regfile[R10] = 0xabcd;

    stbi(R10, 100);
    halt();

    pc = 0;
    vm_start();

    assert(read_byte(100) == 0xcd);
}

