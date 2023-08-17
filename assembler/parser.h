// #include "scanner.h"
// #include "vm.h"

struct parser {
    struct scanner s;
    struct token_array ta;
    struct token *tok;
    int cur;
    int offset;
};

struct inst {
    enum vm_opcode opcode;

    union {
        int as_int;
        struct token *as_tok;
    } operand;
    int is_resolved;
    int operand_size;
};

struct inst_array {
    int len;
    int cap;
    int data_size;
    struct inst *buf;
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

void parse_instructions(struct parser *p, struct symtable *st, struct inst_array *ia);
void resolve_labels(struct parser *p, struct symtable *st, struct inst_array *ia);
void make_parser(struct parser *p, char *filepath);
