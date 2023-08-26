// #include "lib/sstring.h"

typedef enum {
    TOK_ERR = 0,

    TOK_IDENT,
    TOK_NUM,

    TOK_EQ,

    TOK_PLUS,
    TOK_MINUS,

    TOK_SEMICOLON,
    TOK_COMMA,

    TOK_LPAREN,
    TOK_RPAREN,
    TOK_LCURLY,
    TOK_RCULRY,

    TOK_LET,
    TOK_PROC,
    TOK_RET,

    TOK_EOF
} Token_Kind;

typedef struct {
    Token_Kind kind;
    string lex;
    int line;
} Token;

typedef struct {
    int len;
    int cap;
    int data_size;
    Token *buf;
} Tokens;

typedef struct {
    char *filepath;
    char *src;

    int cur;
    int pos;
    int line;

    char ch;
} Scanner;

void make_scanner(Scanner *s, char *filepath);
void scan_tokens(Scanner *s, Tokens *toks);
char *tokstr(Token_Kind kind);
