// #include "lib/sstring.h"

typedef enum {
    TOK_IDENT = 0,
    TOK_NUM,

    TOK_EQ,

    TOK_PLUS,
    TOK_MINUS,

    TOK_COMMA,
    TOK_COLON,
    TOK_SEMICOLON,

    TOK_LPAREN,
    TOK_RPAREN,
    TOK_LCURLY,
    TOK_RCULRY,

    TOK_LET,
    TOK_PROC,
    TOK_RET,

    TOK_type_begin,
    TOK_VOID,

    TOK_numtype_begin,
    TOK_U16,
    TOK_U8,
    TOK_numtype_end,
    TOK_type_end,

    TOK_EOF
} Token_Kind;

typedef struct {
    Token_Kind kind;
    string lex;

    union {
        int as_num;
    } value;

    struct {
        char *file;
        int line;
    } pos;
} Token;

typedef struct {
    int len;
    int cap;
    Token *buf;
} Tokens;

typedef struct {
    char *file;
    char *src;

    int cur;
    int start;
    int line;

    char ch;
} Scanner;

void scanner_make(Scanner *s, char *filepath);
void scanner_scan(Scanner *s, Tokens *toks);
int scanner_is_type(Token_Kind kind);
int scanner_is_numtype(Token_Kind kind);
char *scanner_tokstr(Token_Kind kind);
