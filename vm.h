enum vm_opcode {
    OP_HALT = 0,

    OP_PUSH,
    OP_PUSHB,

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
    OP_CJMP
};

