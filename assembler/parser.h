// #include "scanner.h"
// #include "vm.h"

struct parser {
    struct scanner s;
    struct tokens toks;
    struct token *tok;
    int cur;
};

struct symbol {
    char *label;
    int label_len;
    int addr;
    int is_resolved;
};

struct symtable {
    int len;
    int cap;
    int data_size;
    struct symbol *buf;
};

struct push_operand {
    enum { PUSH_NUM, PUSH_SYM } kind;
    int size;
    union {
        int num;
        struct symbol *sym;
    } as;
};

struct mnemonic_push {
    enum vm_opcode opcode;
    struct push_operand operand;
};

struct mnemonic {
    enum vm_opcode opcode;
};

struct data_label {
    int value_size;
    struct {
        int len;
        int cap;
        int data_size;
        int *buf;
    } values;
};

struct parsed_value {
    enum {
        PARSVAL_MNEMONIC,
        PARSVAL_MNEMONIC_PUSH,
        PARSVAL_DATA_LABEL
    } kind;
    void *value;
};

struct parsed_values {
    int len;
    int cap;
    int data_size;
    struct parsed_value *buf;
};

void parse(struct parser *p, struct symtable *st, struct parsed_values *values);
void make_parser(struct parser *p, char *filepath);
