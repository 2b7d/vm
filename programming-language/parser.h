// #include "scanner.h"

typedef struct {
    Scanner s;
    Tokens toks;
    Token *tok;
    int cur;
} Parser;

typedef enum {
    EXPR_LIT = 0,
    EXPR_BINARY,
} Expr_Kind;

typedef struct {
    Expr_Kind kind;
    void *body;
} Expr;

typedef struct {
    Token *value;
} Expr_Lit;

typedef struct {
    Expr x;
    Expr y;
    Token *op;
} Expr_Binary;

typedef enum {
    STMT_BLOCK = 0,
    STMT_LET,
    STMT_ASSIGN,
    STMT_PROC,
    STMT_RET
} Stmt_Kind;

typedef struct {
    Stmt_Kind kind;
    void *body;
} Stmt;

typedef struct {
    int len;
    int cap;
    int data_size;
    Stmt *buf;
} Stmts;

typedef struct {
    Stmts stmts;
} Stmt_Block;

typedef struct {
    Token *ident;
} Stmt_Let;

typedef struct {
    Token *ident;
    Expr value;
} Stmt_Assign;

typedef struct {
    Token *ident;
    Stmt body;
} Stmt_Proc;

typedef struct {
    Token *tok;
} Stmt_Ret;

void make_parser(Parser *p, char *filepath);
void parse(Parser *p, Stmts *stmts);
