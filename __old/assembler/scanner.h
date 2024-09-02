// #include "lib/sstring.h"

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

    TOK_NUM,
    TOK_SYM,
    TOK_STR,

    TOK_DOT,
    TOK_COMMA,
    TOK_COLON,

    TOK_mnemonic_start,
    TOK_HALT,
    TOK_PUSH,
    TOK_PUSHB,
    TOK_DROP,
    TOK_DROPB,
    TOK_LD,
    TOK_LDB,
    TOK_ST,
    TOK_STB,
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
    TOK_CALL,
    TOK_RET,
    TOK_SYSCALL,
    TOK_mnemonic_end,

    TOK_BYTE,
    TOK_WORD,
    TOK_EXTERN,
    TOK_GLOBAL,

    TOK_EOF
};

struct token {
    enum token_kind kind;
    string lex;
    int line;
};

struct tokens {
    int len;
    int cap;
    int data_size;
    struct token *buf;
};

void make_scanner(struct scanner *s, char *filepath);
void scan_tokens(struct scanner *s, struct tokens *toks);
char *tok_to_str(enum token_kind kind);
int is_mnemonic(enum token_kind kind);
