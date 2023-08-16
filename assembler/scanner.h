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

    TOK_SYMBOL,
    TOK_NUM,

    TOK_EOF
};

struct token {
    enum token_kind kind;
    char *lex;
    int lex_len;
    int line;
};

char *tok_to_str(enum token_kind kind);
void make_scanner(struct scanner *s, char *filepath);
void scan_token(struct scanner *s, struct token *tok);
