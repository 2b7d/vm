//#include "lib/sstring.h"

typedef enum {
    TOK_IDENT = 0,
    TOK_NUM,

    TOK_PLUS,
    TOK_MINUS,
    TOK_SLASH,
    TOK_STAR,

    TOK_EQ,
    TOK_EQEQ,
    TOK_LT,
    TOK_GT,
    TOK_BANG,

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
    TOK_TRUE,
    TOK_FALSE,

    TOK_storage_begin,
    TOK_EXTERN,
    TOK_GLOBAL,
    TOK_storage_end,

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
        int num;
    } as;

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

typedef struct {
    Tokens toks;
    Token *tok;
    int cur;
} Parser;

typedef enum {
    EXPR_LIT = 0,
    EXPR_UNARY,
    EXPR_BINARY
} Expr_Kind;

typedef struct expr {
    Expr_Kind kind;
    union {
        struct {
            Token *value;
        } lit;

        struct {
            struct expr x;
            Token *op;
        } unary;

        struct {
            struct expr x;
            struct expr y;
            Token *op;
        } binary;
    } as;
} Expr;

typedef struct {
    int len;
    int cap;
    Expr *buf;
} Exprs;

typedef enum {
    STMT_EXPR = 0
} Stmt_Kind;

typedef struct {
    struct expr expr;
} Stmt_Expr;

typedef struct {
    Stmt_Kind kind;
    union {
        Stmt_Expr stmt_expr;
    } as;
} Stmt;

typedef struct {
    int len;
    int cap;
    Stmt *buf;
} Stmts;

char *token_str(Token_Kind kind);
int token_is_storage(Token_Kind kind);
int token_is_type(Token_Kind kind);
int token_is_numtype(Token_Kind kind);

void scanner_make(Scanner *s, char *filepath);
void scanner_scan(Scanner *s, Tokens *toks);

void parser_make(Parser *p, Scanner *s);
void parser_parse(Parser *p, Stmts *stmts);
