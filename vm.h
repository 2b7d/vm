enum vm_opcode {
    OP_ST,
    OP_LD,

    OP_LIT,

    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,
    OP_INC,
    OP_DEC,

    OP_DUP,
    OP_OVER,

    OP_EQ,
    OP_GT,
    OP_LT,
    OP_OR,
    OP_AND,
    OP_NOT,

    OP_JMP,
    OP_IFJMP,

    OP_HALT
};
