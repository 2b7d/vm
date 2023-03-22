enum vm_opcode {
    OP_ST,       // a ptr ->
    OP_LD,       // ptr -> a

    OP_ADD,      // a b -> c
    OP_SUB,      // a b -> c
    OP_MUL,      // a b -> c
    OP_DIV,      // a b -> c
    OP_MOD,      // a b -> c
    OP_INC,      // a -> b
    OP_DEC,      // a -> b

    OP_PUSH,     // -> a
    OP_DUP,      // a -> a a
    OP_OVER,     // a b -> a b a
    OP_SWAP,     // a b -> b a
    OP_DROP,     // a ->
    OP_RSPUSH,   // a ->
    OP_RSPOP,    // -> a
    OP_RSCOPY,   // -> a
    OP_RSDROP,   // ->
    OP_RSP,      // -> ptr
    OP_RSPSET,   // ptr ->
                 //
    OP_BRK,      // -> ptr
    OP_BRKSET,   // ptr -> ptr

    OP_EQ,       // a b -> c
    OP_GT,       // a b -> c
    OP_LT,       // a b -> c
    OP_OR,       // a b -> c
    OP_AND,      // a b -> c
    OP_NOT,      // a -> b

    OP_JMP,      // ->
    OP_JMPIF,    // a ->

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
