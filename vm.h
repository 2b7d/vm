enum vm_opcode {
    OP_HALT = 0,

    OP_PUSH,
    OP_PUSHB,

    OP_LD,
    OP_LDB,
    OP_ST,
    OP_STB,

    OP_CTW,
    OP_CTB,

    OP_ADD,
    OP_ADDB,
    OP_SUB,
    OP_SUBB,
    OP_NEG,
    OP_NEGB,

    OP_EQ,
    OP_EQB,
    OP_LT,
    OP_LTB,
    OP_GT,
    OP_GTB,

    OP_JMP,
    OP_CJMP,

    OP_CALL,
    OP_RET,

    OP_SYSCALL
};

enum vm_syscall {
    SYS_READ = 0,
    SYS_WRITE
};

enum vm_section {
    SECTION_DATA = 0,
    SECTION_TEXT
};
