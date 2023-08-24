#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../lib/mem.h"
#include "../lib/sstring.h"

#include "scanner.h"
#include "parser.h"

static void parse_statement(Parser *p, Stmt *s);

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

static void parse_primary(Parser *p, Expr *e)
{
    Expr_Lit *lit;

    if (p->tok->kind != TOK_IDENT && p->tok->kind != TOK_NUM) {
        fprintf(stderr, "%s:%d: expected number or identifier but got %s\n", p->s.filepath, p->tok->line, tokstr(p->tok->kind));
        exit(1);
    }

    lit = malloc(sizeof(Expr_Lit));
    if (lit == NULL) {
        perror("parse_primary");
        exit(1);
    }

    lit->value = p->tok;
    advance(p);

    e->kind = EXPR_LIT;
    e->body = lit;
}

static void parse_term(Parser *p, Expr *e)
{
    parse_primary(p, e);

    if (p->tok->kind == TOK_PLUS) {
        Expr_Binary *binary;

        binary = malloc(sizeof(Expr_Binary));
        if (binary == NULL) {
            perror("parse_term");
            exit(1);
        }

        memcpy(&binary->x, e, sizeof(Expr));

        binary->op = p->tok;
        advance(p);

        parse_primary(p, &binary->y);

        e->kind = EXPR_BINARY;
        e->body = binary;
    }
}

static void parse_expression(Parser *p, Expr *e)
{
    parse_term(p, e);
}

static void parse_block(Parser *p, Stmt *s)
{
    Stmt_Block *block;

    consume(p, TOK_LCURLY);

    block = malloc(sizeof(Stmt_Block));
    if (block == NULL) {
        perror("parse_block");
        exit(1);
    }

    meminit(&block->stmts, sizeof(Stmt), 64);

    while (p->tok->kind != TOK_RCULRY && p->tok->kind != TOK_EOF) {
        Stmt *tmp;

        tmp = memnext(&block->stmts);
        parse_statement(p, tmp);
    }

    consume(p, TOK_RCULRY);

    s->kind = STMT_BLOCK;
    s->body = block;
}

static void parse_let(Parser *p, Stmt *s)
{
    Stmt_Let *let;

    consume(p, TOK_LET);

    let = malloc(sizeof(Stmt_Let));
    if (let == NULL) {
        perror("parse_let");
        exit(1);
    }

    meminit(&let->idents, sizeof(Token *), 16);

    for (;;) {
        memgrow(&let->idents);
        let->idents.buf[let->idents.len++] = p->tok;

        consume(p, TOK_IDENT);

        if (p->tok->kind != TOK_COMMA) {
            break;
        }

        advance(p);
    }

    consume(p, TOK_SEMICOLON);

    s->kind = STMT_LET;
    s->body = let;
}

static void parse_assign(Parser *p, Stmt *s)
{
    Stmt_Assign *assign;

    assign = malloc(sizeof(Stmt_Assign));
    if (assign == NULL) {
        perror("parse_assign");
        exit(1);
    }

    assign->ident = p->tok;

    consume(p, TOK_IDENT);
    consume(p, TOK_EQ);

    parse_expression(p, &assign->value);

    consume(p, TOK_SEMICOLON);

    s->kind = STMT_ASSIGN;
    s->body = assign;
}

static void parse_proc(Parser *p, Stmt *s)
{
    Stmt_Proc *proc;

    consume(p, TOK_PROC);

    proc = malloc(sizeof(Stmt_Proc));
    if (proc == NULL) {
        perror("parse_proc");
        exit(1);
    }

    proc->ident = p->tok;

    consume(p, TOK_IDENT);
    consume(p, TOK_LPAREN);
    consume(p, TOK_RPAREN);

    parse_block(p, &proc->body);

    s->kind = STMT_PROC;
    s->body = proc;
}

static void parse_ret(Parser *p, Stmt *s)
{
    Stmt_Ret *ret;

    ret = malloc(sizeof(Stmt_Ret));
    if (ret == NULL) {
        perror("parse_ret");
        exit(1);
    }

    ret->tok = p->tok;

    consume(p, TOK_RET);
    consume(p, TOK_SEMICOLON);

    s->kind = STMT_RET;
    s->body = ret;
}

static void parse_statement(Parser *p, Stmt *s)
{
    switch (p->tok->kind) {
    case TOK_LCURLY:
        parse_block(p, s);
        break;
    case TOK_LET:
        parse_let(p, s);
        break;
    case TOK_IDENT:
        parse_assign(p, s);
        break;
    case TOK_PROC:
        parse_proc(p, s);
        break;
    case TOK_RET:
        parse_ret(p, s);
        break;

    default:
        fprintf(stderr, "%s:%d: expected statement\n", p->s.filepath, p->tok->line);
        exit(1);
    }
}

void parse(Parser *p, Stmts *stmts)
{
    while (p->tok->kind != TOK_EOF) {
        Stmt *s;

        s = memnext(stmts);
        parse_statement(p, s);
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
