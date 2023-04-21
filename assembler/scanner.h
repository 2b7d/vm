enum token_kind {
    TOK_BYTES,
    TOK_EXTERN,
    TOK_GLOBAL,

    TOK_SYMBOL,
    TOK_OPCODE,
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
    int is_byte;
};

struct token_array {
    int len;
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
