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

    TOK_VAR,
    TOK_PROC,
    TOK_RET,

    TOK_storage_begin,
    TOK_EXTERN,
    TOK_GLOBAL,
    TOK_storage_end,

    TOK_type_begin,
    TOK_VOID,
    TOK_U16,
    TOK_U8,
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

char *token_str(Token_Kind kind);
int token_is_storage(Token_Kind kind);
int token_is_type(Token_Kind kind);
