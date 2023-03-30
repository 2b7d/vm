// #include <stddef.h>
// #include <stdint.h>

enum token_kind {
    // keywords must be zero aligned to be used as array indices
    TOK_DB,
    TOK_EXTERN,
    TOK_GLOBAL,
    TOK_SECTION,
    TOK_TEXT,
    TOK_DATA,
    KWD_COUNT,

    TOK_NUM,
    TOK_STR,
    TOK_SYMBOL,
    TOK_OPCODE,

    TOK_COMMA,
    TOK_COLON,
    TOK_EOF
};

struct token {
    enum token_kind kind;
    char *start;
    size_t len;
    uint8_t opcode;
    int is_byteop;
};

struct scanner {
    char *cur;
    char *start;
    char *src;
};

void scanner_init(struct scanner *s, char *src);
void scan_token(struct scanner *s, struct token *t);
