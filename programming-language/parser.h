//#include "lib/sstring.h"
//#include "token.h"
//#include "scanner.h"

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

void parser_make(Parser *p, Scanner *s);
void parser_parse(Parser *p, Stmts *stmts);
