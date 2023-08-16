// #include "scanner.h"
// #include "vm.h"

struct parser {
    struct scanner s;
    struct token tok;
};

struct inst {
    enum vm_opcode opcode;
    int operand;
    int operand_size;
};

int parse_instruction(struct parser *p, struct inst *inst);
void make_parser(struct parser *p, char *filepath);
