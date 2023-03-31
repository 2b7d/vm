// #include <stddef.h>
// #include <stdint.h>

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
    char *start;
    size_t len;
    int opcode;
};

struct scanner {
    char *cur;
    char *start;
    char *src;
};

void scanner_init(struct scanner *s, char *src);
void scan_token(struct scanner *s, struct token *t);
