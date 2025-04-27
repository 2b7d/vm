void test_mov()
{
    printf("test_mov\n");
    reset_vm();

    regfile[R10] = 0xaabb;
    mov(R10, R11);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[R11] == 0xaabb);
}
