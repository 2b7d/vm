void test_stbi()
{
    printf("test_stbi\n");
    reset_vm();

    regfile[R10] = 0x69;

    stbi(R10, 69);
    halt();

    pc = 0;
    vm_start();

    assert(read_byte(69) == regfile[R10]);
}
