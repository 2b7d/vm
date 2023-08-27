// #include "lib/sstring.h"
// #include "scanner.h"

typedef struct {
    Tokens toks;
    Token *tok;
    int cur;
} Parser;

typedef enum {
    EXPR_LIT = 0,
    EXPR_CONST,
    EXPR_BINARY,
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
    Expr callee;
    Exprs args;
} Expr_Call;

typedef struct {
    Expr x;
    Expr y;
    Token *op;
} Expr_Binary;

typedef enum {
    STMT_ASSIGN = 0,
    STMT_RET,
    STMT_PROC_BLOCK,
    STMT_BLOCK,
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
} Stmt_Assign;

typedef struct {
    Token *tok;
    Expr value;
    int has_value;
} Stmt_Ret;

typedef struct {
    Expr expr;
} Stmt_Expr;

typedef enum {
    DECL_VAR = 0,
    DECL_PROC
} Decl_Kind;

typedef struct {
    Decl_Kind kind;
    void *body;
} Decl;

typedef struct {
    int len;
    int cap;
    Decl *buf;
} Decls;

typedef struct {
    Stmts stmts;
} Stmt_Block;

typedef struct {
    Decls decls;
    Stmts stmts;
} Stmt_Proc_Block;

typedef struct {
    Token *ident;
    Token *type;
    Expr value;
} Decl_Var;

typedef struct {
    Token *ident;
    Token *type;
} Proc_Param;

typedef struct {
    Token *ident;
    Token *ret_type;
    struct {
        int len;
        int cap;
        Proc_Param *buf;
    } params;
    Stmt body;
} Decl_Proc;

void parser_make(Parser *p, Scanner *s);
void parser_parse(Parser *p, Decls *decls);
