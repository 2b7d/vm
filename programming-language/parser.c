#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../lib/mem.h"
#include "../lib/sstring.h"

#include "scanner.h"
#include "parser.h"

static void statement(Parser *p, Stmt *stmt);
static void expression(Parser *p, Expr *e);

static void advance(Parser *p)
{
    if (p->tok->kind == TOK_EOF) {
        return;
    }

    p->cur++;
    p->tok = p->toks.buf + p->cur;
}

static void consume(Parser *p, Token_Kind kind)
{
    if (p->tok->kind != kind) {
        fprintf(stderr, "%s:%d: expected %s but got %s\n", p->s.filepath, p->tok->line, tokstr(kind), tokstr(p->tok->kind));
        exit(1);
    }

    advance(p);
}

static void primary(Parser *p, Expr *expr)
{
    Expr_Lit *lit;

    if (p->tok->kind == TOK_LPAREN) {
        advance(p);
        expression(p, expr);
        consume(p, TOK_RPAREN);
        return;
    }

    if (p->tok->kind != TOK_IDENT && p->tok->kind != TOK_NUM) {
        fprintf(stderr, "%s:%d: expected number or identifier but got %s\n", p->s.filepath, p->tok->line, tokstr(p->tok->kind));
        exit(1);
    }

    lit = malloc(sizeof(Expr_Lit));
    if (lit == NULL) {
        perror("primary");
        exit(1);
    }

    lit->value = p->tok;
    advance(p);

    expr->kind = EXPR_LIT;
    expr->body = lit;
}

static void term(Parser *p, Expr *expr)
{
    primary(p, expr);

    while (p->tok->kind == TOK_PLUS || p->tok->kind == TOK_MINUS) {
        Expr_Binary *binary = malloc(sizeof(Expr_Binary));
        if (binary == NULL) {
            perror("term");
            exit(1);
        }

        memcpy(&binary->x, expr, sizeof(Expr));
        binary->op = p->tok;
        advance(p);

        primary(p, &binary->y);

        expr->kind = EXPR_BINARY;
        expr->body = binary;
    }
}

static void expression(Parser *p, Expr *expr)
{
    term(p, expr);
}

static void constant_expression(Parser *p, Expr *expr)
{
    Expr_Lit *lit;

    if (p->tok->kind != TOK_NUM) {
        fprintf(stderr, "%s:%d: expected constant expression\n", p->s.filepath, p->tok->line);
        exit(1);
    }

    lit = malloc(sizeof(Expr_Lit));
    if (lit == NULL) {
        perror("constant_expression");
        exit(1);
    }

    lit->value = p->tok;
    advance(p);

    expr->kind = EXPR_CONST;
    expr->body = lit;
}

static void assign(Parser *p, Stmt *stmt)
{
    Stmt_Assign *assign = malloc(sizeof(Stmt_Assign));
    if (assign == NULL) {
        perror("assign");
        exit(1);
    }

    assign->ident = p->tok;

    consume(p, TOK_IDENT);
    consume(p, TOK_EQ);

    expression(p, &assign->value);

    consume(p, TOK_SEMICOLON);

    stmt->kind = STMT_ASSIGN;
    stmt->body = assign;
}

static void _return(Parser *p, Stmt *stmt)
{
    Stmt_Ret *ret = malloc(sizeof(Stmt_Ret));
    if (ret == NULL) {
        perror("parse_ret");
        exit(1);
    }

    ret->tok = p->tok;

    consume(p, TOK_RET);
    consume(p, TOK_SEMICOLON);

    stmt->kind = STMT_RET;
    stmt->body = ret;
}

static void block_variable(Parser *p, Decl *decl)
{
    Decl_Var *var = malloc(sizeof(Decl_Var));
    if (var == NULL) {
        perror("block_variable");
        exit(1);
    }

    consume(p, TOK_LET);

    var->ident = p->tok;

    consume(p, TOK_IDENT);
    consume(p, TOK_EQ);

    expression(p, &var->value);

    consume(p, TOK_SEMICOLON);

    decl->kind = DECL_BLOCK_VAR;
    decl->body = var;
}

static void block(Parser *p, Stmt *stmt)
{
    Stmt_Block *block = malloc(sizeof(Stmt_Block));
    if (block == NULL) {
        perror("parse_block");
        exit(1);
    }

    consume(p, TOK_LCURLY);

    meminit(&block->stmts, sizeof(Stmt), 64);
    meminit(&block->decls, sizeof(Decl), 64);

    while (p->tok->kind == TOK_LET) {
        Decl *tmp = memnext(&block->decls);
        block_variable(p, tmp);
    }

    while (p->tok->kind != TOK_RCULRY && p->tok->kind != TOK_EOF) {
        Stmt *tmp = memnext(&block->stmts);
        statement(p, tmp);
    }

    consume(p, TOK_RCULRY);

    stmt->kind = STMT_BLOCK;
    stmt->body = block;
}


static void statement(Parser *p, Stmt *stmt)
{
    switch (p->tok->kind) {
    case TOK_IDENT:
        assign(p, stmt);
        break;
    case TOK_RET:
        _return(p, stmt);
        break;
    case TOK_LCURLY:
        block(p, stmt);
        break;
    default:
        fprintf(stderr, "%s:%d: expected statement\n", p->s.filepath, p->tok->line);
        exit(1);
    }
}

static void procedure(Parser *p, Decl *decl)
{
    Decl_Proc *proc = malloc(sizeof(Decl_Proc));
    if (proc == NULL) {
        perror("procedure");
        exit(1);
    }

    consume(p, TOK_PROC);

    proc->ident = p->tok;

    consume(p, TOK_IDENT);
    consume(p, TOK_LPAREN);
    consume(p, TOK_RPAREN);

    block(p, &proc->body);

    decl->kind = DECL_PROC;
    decl->body = proc;
}

static void file_variable(Parser *p, Decl *decl)
{
    Decl_Var *var = malloc(sizeof(Decl_Var));
    if (var == NULL) {
        perror("file_variable");
        exit(1);
    }

    consume(p, TOK_LET);

    var->ident = p->tok;

    consume(p, TOK_IDENT);
    consume(p, TOK_EQ);

    constant_expression(p, &var->value);

    consume(p, TOK_SEMICOLON);

    decl->kind = DECL_FILE_VAR;
    decl->body = var;
}

static void declaration(Parser *p, Decl *decl)
{
    switch (p->tok->kind) {
    case TOK_LET:
        file_variable(p, decl);
        break;
    case TOK_PROC:
        procedure(p, decl);
        break;
    default:
        fprintf(stderr, "%s:%d: expected declaration\n", p->s.filepath, p->tok->line);
        exit(1);
    }
}

void parse(Parser *p, Decls *decls)
{
    while (p->tok->kind != TOK_EOF) {
        Decl *d = memnext(decls);

        declaration(p, d);
    }
}

void make_parser(Parser *p, char *filepath)
{
    meminit(&p->toks, sizeof(Token), 128);

    make_scanner(&p->s, filepath);
    scan_tokens(&p->s, &p->toks);

    p->cur = 0;
    p->tok = p->toks.buf;
}
