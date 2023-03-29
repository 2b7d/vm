enum token_kind {
    TOK_DB,
    TOK_EXTERN,
    TOK_GLOBAL,
    TOK_SECTION,
    TOK_TEXT,
    TOK_DATA,
    TOK_ST,
    TOK_LD,
    TOK_ADD,
    TOK_SUB,
    TOK_MUL,
    TOK_DIV,
    TOK_MOD,
    TOK_INC,
    TOK_DEC,
    TOK_PUSH,
    TOK_DUP,
    TOK_OVER,
    TOK_SWAP,
    TOK_DROP,
    TOK_RSPUSH,
    TOK_RSPOP,
    TOK_RSCOPY,
    TOK_RSDROP,
    TOK_RSP,
    TOK_RSPSET,
    TOK_BRK,
    TOK_BRKSET,
    TOK_EQ,
    TOK_GT,
    TOK_LT,
    TOK_OR,
    TOK_AND,
    TOK_NOT,
    TOK_JMP,
    TOK_JMPIF,
    TOK_CALL,
    TOK_RET,
    TOK_HALT,
    TOK_SYSCALL,
    IDENT_COUNT,

    TOK_NUM,
    TOK_STR,
    TOK_SYMBOL,

    TOK_COMMA,
    TOK_COLON,
    TOK_EOF
};

struct token {
    enum token_kind kind;
    char *start;
    size_t len;
};

struct scanner {
    char *cur;
    char *start;
    char *src;
};

void scanner_init(struct scanner *s, char *src);
void scan_token(struct scanner *s, struct token *t);
