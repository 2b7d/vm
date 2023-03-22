enum vm_opcode {
    OP_ST,       // n1 addr ->
    OP_LD,       // addr -> n1

    OP_ADD,      // n1 n2 -> n3
    OP_SUB,      // n1 n2 -> n3
    OP_MUL,      // n1 n2 -> n3
    OP_DIV,      // n1 n2 -> n3
    OP_MOD,      // n1 n2 -> n3
    OP_INC,      // n1 -> n2
    OP_DEC,      // n1 -> n2

    OP_PUSH,     // -> n1
    OP_DUP,      // n1 -> n1 n1
    OP_OVER,     // n1 n2 -> n1 n2 n1
    OP_SWAP,     // n1 n2 -> n2 n1
    OP_DROP,     // n1 ->
    OP_RSPUSH,   // n1 ->
    OP_RSPOP,    // -> n1
    OP_RSCOPY,   // -> n1
    OP_RSDROP,   // ->
    OP_RSP,      // -> n1
    OP_RSPSET,   // n1 ->

    OP_EQ,       // n1 n2 -> n3
    OP_GT,       // n1 n2 -> n3
    OP_LT,       // n1 n2 -> n3
    OP_OR,       // n1 n2 -> n3
    OP_AND,      // n1 n2 -> n3
    OP_NOT,      // n1 -> n2

    OP_JMP,      // ->
    OP_JMPIF,    // n1 ->

    OP_HALT,     // ->

    OP_SYSCALL,  // vm_syscall
    OP_CALL,     // ->
    OP_RET,      // ->

    OP_COUNT
};

enum vm_syscall {
    SYS_READ,
    SYS_WRITE   // ptr len ->
};
