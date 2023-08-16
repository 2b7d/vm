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

struct symbol {
    char *label;
    int label_len;
    int value;
};

struct symtable {
    int len;
    int cap;
    int data_size;
    struct symbol *buf;
};

int parse_instruction(struct parser *p, struct symtable *st, struct inst *inst);
void populate_symbols(struct parser *p, struct symtable *st);
void make_parser(struct parser *p, char *filepath);
