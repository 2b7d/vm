enum token_kind {
    TOK_BYTES,
    TOK_EXTERN,
    TOK_GLOBAL,

    TOK_SYMBOL,
    TOK_OPCODE,
    TOK_OPCODE_BYTE,
    TOK_NUM,
    TOK_STR,

    TOK_COMMA,
    TOK_COLON,
    TOK_EOF
};

struct token {
    enum token_kind kind;
    char *lex;
    int len;
    int value;
};

struct token_array {
    int size;
    int cap;
    int data_size;
    struct token *buf;
};

struct scanner {
    char *cur;
    char *start;
    char *src;
};

void scanner_init(struct scanner *s, char *src);
void scan_tokens(struct scanner *s, struct token_array *ta);
