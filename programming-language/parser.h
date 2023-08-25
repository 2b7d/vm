// #include "lib/sstring.h"
// #include "scanner.h"

typedef struct {
    Scanner s;
    Tokens toks;
    Token *tok;
    int cur;
} Parser;

typedef enum {
    EXPR_LIT = 0,
    EXPR_CONST,
    EXPR_BINARY
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
    STMT_ASSIGN = 0,
    STMT_RET,
    STMT_BLOCK
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
    Token *ident;
    Expr value;
} Stmt_Assign;

typedef struct {
    Token *tok;
} Stmt_Ret;

typedef enum {
    DECL_FILE_VAR = 0,
    DECL_BLOCK_VAR,
    DECL_PROC
} Decl_Kind;

typedef struct {
    Decl_Kind kind;
    void *body;
} Decl;

typedef struct {
    int len;
    int cap;
    int data_size;
    Decl *buf;
} Decls;

typedef struct {
    Decls decls;
    Stmts stmts;
} Stmt_Block;

typedef struct {
    Token *ident;
    Expr value;
} Decl_Var;

typedef struct {
    Token *ident;
    Stmt body;
} Decl_Proc;

void make_parser(Parser *p, char *filepath);
void parse(Parser *p, Decls *decls);
