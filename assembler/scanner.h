struct scanner {
    char *src;
    char *filepath;

    int src_len;
    int cur;
    int pos;
    int line;

    char ch;
};

enum token_kind {
    TOK_ERR = 0,

    TOK_IDENT,
    TOK_NUM,

    TOK_HALT,
    TOK_PUSH,
    TOK_PUSHB,
    TOK_CTW,
    TOK_CTB,
    TOK_ADD,
    TOK_ADDB,
    TOK_SUB,
    TOK_SUBB,
    TOK_NEG,
    TOK_NEGB,
    TOK_EQ,
    TOK_EQB,
    TOK_LT,
    TOK_LTB,
    TOK_GT,
    TOK_GTB,
    TOK_JMP,
    TOK_CJMP,

    TOK_EOF
};

struct token {
    enum token_kind kind;
    char *lex;
};

void make_scanner(struct scanner *s, char *filepath);
void scan_token(struct scanner *s, struct token *tok);
