#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../lib/mem.h"
#include "../lib/sstring.h"

#include "vmc.h"

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
        fprintf(stderr, "%s:%d: expected %s but got %s\n", p->tok->pos.file, p->tok->pos.line, token_str(kind), token_str(p->tok->kind));
        exit(1);
    }

    advance(p);
}

static void expression(Parser *p, Expr *e);

static void primary(Parser *p, Expr *e)
{
    switch (p->tok->kind) {
    case TOK_NUM:
    case TOK_TRUE:
    case TOK_FALSE:
        e->kind = EXPR_LIT;
        e->as.lit.value = p->tok;
        advance(p);
        break;
    case TOK_LPAREN:
        advance(p);
        expression(p, e);
        consume(p, TOK_RPAREN);
        break;
    default:
        fprintf(stderr, "%s:%d: expected expression\n", p->tok->pos.file, p->tok->pos.line);
        exit(1);
    }
}

static void unary(Parser *p, Expr *e)
{
    if (p->tok->kind == TOK_MINUS || p->tok->kind == TOK_BANG) {
        e->kind = EXPR_UNARY;
        e->as.unary.op = p->tok;

        advance(p);

        unary(p, &e->as.unary.x);
        return;
    }

    primary(p, e);
}

static void term(Parser *p, Expr *e)
{
    unary(p, e);

    while (p->tok->kind == TOK_SLASH || p->tok->kind == TOK_STAR) {
        e->kind = EXPR_BINARY;
        e->as.binary.op = p->tok;

        advance(p);

        memcpy(&e->as.binary.x, e, sizeof(Expr));
        unary(p, &e->as.binary.y);
    }
}

static void factor(Parser *p, Expr *e)
{
    term(p, e);

    while (p->tok->kind == TOK_SLASH || p->tok->kind == TOK_STAR) {
        e->kind = EXPR_BINARY;
        e->as.binary.op = p->tok;

        advance(p);

        memcpy(&e->as.binary.x, e, sizeof(Expr));
        term(p, &e->as.binary.y);
    }
}

static void comparison(Parser *p, Expr *e)
{
    factor(p, e);

    while (p->tok->kind == TOK_LT || p->tok->kind == TOK_GT) {
        e->kind = EXPR_BINARY;
        e->as.binary.op = p->tok;

        advance(p);

        memcpy(&e->as.binary.x, e, sizeof(Expr));
        factor(p, &e->as.binary.y);
    }
}

static void equality(Parser *p, Expr *e)
{
    comparison(p, e);

    while (p->tok->kind == TOK_EQEQ) {
        e->kind = EXPR_BINARY;
        e->as.binary.op = p->tok;

        advance(p);

        memcpy(&e->as.binary.x, e, sizeof(Expr));
        comparison(p, &e->as.binary.y);
    }
}

static void expression(Parser *p, Expr *e)
{
    equality(p, e);
}

static void stmt_expr(Parser *p, Stmt *s)
{
    s->kind = STMT_EXPR;
    expression(p, &s->as.stmt_expr.expr);

    consume(p, TOK_SEMICOLON);
}

static void statement(Parser *p, Stmt *s)
{
    stmt_expr(p, s);
}

void parser_parse(Parser *p, Stmts *stmts)
{
    while (p->tok->kind != TOK_EOF) {
        Stmt *s;

        mem_grow(stmts);
        s = mem_next(stmts);

        statement(p, s);
    }
}

void parser_make(Parser *p, Scanner *s)
{
    mem_make(&p->toks, 256);

    scanner_scan(s, &p->toks);

    p->tok = p->toks.buf;
    p->cur = 0;
}
