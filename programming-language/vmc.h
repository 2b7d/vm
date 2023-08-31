//#include "lib/sstring.h"

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

typedef struct {
    Tokens toks;
    Token *tok;
    int cur;
} Parser;

typedef enum {
    EXPR_LIT = 0,
    EXPR_BINARY,
    EXPR_VAR,
    EXPR_CALL
} Expr_Kind;

typedef struct {
    Expr_Kind kind;
    void *body;
} Expr;

typedef struct {
    int len;
    int cap;
    Expr *buf;
} Exprs;

typedef struct {
    Token *value;
} Expr_Lit;

typedef struct {
    Expr x;
    Expr y;
    Token *op;
} Expr_Binary;

typedef struct {
    Token *ident;
} Expr_Var;

typedef struct {
    Expr callee;
    Exprs args;
} Expr_Call;

typedef enum {
    STMT_VAR = 0,
    STMT_PROC,
    STMT_PROC_VAR,
    STMT_ASSIGN,
    STMT_RET,
    STMT_EXPR
} Stmt_Kind;

typedef struct {
    Stmt_Kind kind;
    void *body;
} Stmt;

typedef struct {
    int len;
    int cap;
    Stmt *buf;
} Stmts;

typedef struct {
    Token *ident;
    Expr value;
    Token_Kind storage;
    Token_Kind type;
} Stmt_Var;

typedef struct {
    Token *ident;
    Expr value;
    Token_Kind type;
} Stmt_Proc_Var;

typedef struct {
    Token *ident;
    Token_Kind type;
} Proc_Param;

typedef struct {
    Token *ident;
    Stmts vars;
    Stmts stmts;

    struct {
        int len;
        int cap;
        Proc_Param *buf;
    } params;

    Token_Kind storage;
    Token_Kind ret_type;
} Stmt_Proc;

typedef struct {
    Token *ident;
    Expr value;
} Stmt_Assign;

typedef struct {
    Token *tok;
    Expr value;
} Stmt_Ret;

typedef struct {
    Expr expr;
} Stmt_Expr;

char *token_str(Token_Kind kind);
int token_is_storage(Token_Kind kind);
int token_is_type(Token_Kind kind);
int token_is_numtype(Token_Kind kind);

void scanner_make(Scanner *s, char *filepath);
void scanner_scan(Scanner *s, Tokens *toks);

void parser_make(Parser *p, Scanner *s);
void parser_parse(Parser *p, Stmts *stmts);
