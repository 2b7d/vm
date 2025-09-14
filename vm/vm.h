enum vm_opcode {
    HALT,

    MOV,   // reg4 reg4
    MOVI,  // imm16 reg8
    MOVB,  // reg4 reg4
    MOVBI, // imm8 reg8
    MOVZE, // reg4 reg4
    MOVSE, // reg4 reg4

    ST,   // reg4 reg4
    STI,  // reg8 imm16
    STB,  // reg4 reg4
    STBI, // reg8 imm16
    LD,   // reg4 reg4
    LDI,  // imm16 reg8
    LDB,  // reg4 reg4
    LDBI, // imm16 reg8

    ADD,   // reg4 reg4
    ADDI,  // imm16 reg8
    ADDB,  // reg4 reg4
    ADDBI, // imm8 reg8
    SUB,   // reg4 reg4
    SUBI,  // imm16 reg8
    SUBB,  // reg4 reg4
    SUBBI, // imm8 reg8

    NOT,   // reg8
    NOTB,  // reg8
    AND,   // reg4 reg4
    ANDI,  // imm16 reg8
    ANDB,  // reg4 reg4
    ANDBI, // imm8 reg8
    OR,    // reg4 reg4
    ORI,   // imm16 reg8
    ORB,   // reg4 reg4
    ORBI,  // imm8 reg8
    XOR,   // reg4 reg4
    XORI,  // imm16 reg8
    XORB,  // reg4 reg4
    XORBI, // imm8 reg8

    SHL,    // reg4 reg4
    SHLI,   // imm8 reg8
    SHLB,   // reg4 reg4
    SHLBI,  // imm8 reg8
    SHR,    // reg4 reg4
    SHRI,   // imm8 reg8
    SHRB,   // reg4 reg4
    SHRBI,  // imm8 reg8
    SHRA,   // reg4 reg4
    SHRAI,  // imm8 reg8
    SHRAB,  // reg4 reg4
    SHRABI, // imm8 reg8

    CMP,   // reg4 reg4
    CMPI,  // imm16 reg8
    CMPB,  // reg4 reg4
    CMPBI, // imm8 reg8

    JABS, // imm16
    JE,   // imm16
    JNE,  // imm16
    JG,   // imm16
    JGE,  // imm16
    JL,   // imm16
    JLE,  // imm16
    JA,   // imm16
    JAE,  // imm16
    JB,   // imm16
    JBE,  // imm16

    PUSH,  // reg8
    PUSHI, // imm16
    POP,   // reg8
    CALL,  // imm16
    CALLR, // reg8
    RET,

    /*SYSCALL*/

    VM_OPCODE_COUNT
};

enum vm_register {
    R0,
    R1,
    R2,
    R3,
    R4,
    R5,
    R6,
    R7,
    R8,
    R9,
    R10,
    R11,
    R12,
    R13,

    RSP,
    RBP,

    VM_REGISTER_COUNT
};
