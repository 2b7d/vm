#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../lib/mem.h"
#include "../lib/sstring.h"

#include "token.h"
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

static int next(Parser *p, Token_Kind kind)
{
    if (p->tok->kind == TOK_EOF) {
        return 0;
    }

    return p->toks.buf[p->cur + 1].kind == kind;
}

static void consume(Parser *p, Token_Kind kind)
{
    if (p->tok->kind != kind) {
        fprintf(stderr, "%s:%d: expected %s but got %s\n", p->tok->pos.file, p->tok->pos.line, token_str(kind), token_str(p->tok->kind));
        exit(1);
    }

    advance(p);
}

static void consume_type(Parser *p)
{
    if (token_is_type(p->tok->kind) == 0) {
        fprintf(stderr, "%s:%d: expected type but got %s\n", p->tok->pos.file, p->tok->pos.line, token_str(p->tok->kind));
        exit(1);
    }

    advance(p);
}

static void primary(Parser *p, Expr *expr)
{
    Expr_Lit *lit;
    Expr_Var *var;

    switch (p->tok->kind) {
    case TOK_LPAREN:
        advance(p);
        expression(p, expr);
        consume(p, TOK_RPAREN);
        break;
    case TOK_IDENT:
        var = malloc(sizeof(Expr_Var));
        if (var == NULL) {
            perror("primary");
            exit(1);
        }

        var->ident = p->tok;
        advance(p);

        expr->kind = EXPR_VAR;
        expr->body = var;
        break;
    case TOK_NUM:
        lit = malloc(sizeof(Expr_Lit));
        if (lit == NULL) {
            perror("primary");
            exit(1);
        }

        lit->value = p->tok;
        advance(p);

        expr->kind = EXPR_LIT;
        expr->body = lit;
        break;
    default:
        fprintf(stderr, "%s:%d: expected expression but got %s\n", p->tok->pos.file, p->tok->pos.line, token_str(p->tok->kind));
        exit(1);

    }
}

static void call(Parser *p, Expr *expr)
{
    Expr_Call *call;

    primary(p, expr);

    if (p->tok->kind != TOK_LPAREN) {
        return;
    }

    advance(p);

    call = malloc(sizeof(Expr_Call));
    if (call == NULL) {
        perror("call");
        exit(1);
    }

    memcpy(&call->callee, expr, sizeof(Expr));

    if (p->tok->kind != TOK_RPAREN) {
        mem_make(&call->args, 256);

        for (;;) {
            Expr *arg;

            if (call->args.len > 255) {
                fprintf(stderr, "%s:%d: exceeded max number of arguments\n", p->tok->pos.file, p->tok->pos.line);
                exit(1);
            }

            mem_grow(&call->args);
            arg = mem_next(&call->args);
            expression(p, arg);

            if (p->tok->kind != TOK_COMMA) {
                break;
            }

            advance(p);
        }
    }

    consume(p, TOK_RPAREN);

    expr->kind = EXPR_CALL;
    expr->body = call;
}

static void term(Parser *p, Expr *expr)
{
    call(p, expr);

    while (p->tok->kind == TOK_PLUS || p->tok->kind == TOK_MINUS) {
        Expr_Binary *binary = malloc(sizeof(Expr_Binary));
        if (binary == NULL) {
            perror("term");
            exit(1);
        }

        memcpy(&binary->x, expr, sizeof(Expr));
        binary->op = p->tok;
        advance(p);

        call(p, &binary->y);

        expr->kind = EXPR_BINARY;
        expr->body = binary;
    }
}

static void expression(Parser *p, Expr *expr)
{
    term(p, expr);
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

    memset(ret, 0, sizeof(Stmt_Ret));

    ret->tok = p->tok;

    consume(p, TOK_RET);

    if (p->tok->kind != TOK_SEMICOLON) {
        expression(p, &ret->value);
    }

    consume(p, TOK_SEMICOLON);

    stmt->kind = STMT_RET;
    stmt->body = ret;
}

static void statement_expression(Parser *p, Stmt *stmt)
{
    Stmt_Expr *stmt_expr = malloc(sizeof(Stmt_Expr));
    if (stmt_expr == NULL) {
        perror("statement_expression");
        exit(1);
    }

    expression(p, &stmt_expr->expr);

    consume(p, TOK_SEMICOLON);

    stmt->kind = STMT_EXPR;
    stmt->body = stmt_expr;
}

static void statement(Parser *p, Stmt *stmt)
{
    switch (p->tok->kind) {
    case TOK_IDENT:
        if (next(p, TOK_EQ) == 1) {
            assign(p, stmt);
        } else {
            statement_expression(p, stmt);
        }
        break;
    case TOK_NUM:
    case TOK_LPAREN:
        statement_expression(p, stmt);
        break;
    case TOK_RET:
        _return(p, stmt);
        break;

    case TOK_VAR:
        fprintf(stderr, "%s:%d: variables are declared on top of procedure\n", p->tok->pos.file, p->tok->pos.line);
        exit(1);
    case TOK_PROC:
        fprintf(stderr, "%s:%d: can not declare procedure inside of procedure\n", p->tok->pos.file, p->tok->pos.line);
        exit(1);

    default:
        fprintf(stderr, "%s:%d: expected statement but got %s\n", p->tok->pos.file, p->tok->pos.line, token_str(p->tok->kind));
        exit(1);
    }
}

static void procedure(Parser *p, Stmt *stmt, Token *storage)
{
    Stmt_Proc *proc = malloc(sizeof(Stmt_Proc));
    if (proc == NULL) {
        perror("procedure");
        exit(1);
    }

    memset(proc, 0, sizeof(Stmt_Proc));

    if (storage != NULL) {
        proc->storage = storage->kind;
    }

    consume(p, TOK_PROC);

    proc->ident = p->tok;

    consume(p, TOK_IDENT);
    consume(p, TOK_LPAREN);

    if (p->tok->kind != TOK_RPAREN) {
        mem_make(&proc->params, 256);

        for (;;) {
            Proc_Param *param;

            if (proc->params.len > 255) {
                fprintf(stderr, "%s:%d: exceeded max number of parameters\n", p->tok->pos.file, p->tok->pos.line);
                exit(1);
            }

            mem_grow(&proc->params);
            param = mem_next(&proc->params);

            param->ident = p->tok;

            consume(p, TOK_IDENT);
            consume(p, TOK_COLON);

            param->type = p->tok->kind;

            consume_type(p);

            if (p->tok->kind != TOK_COMMA) {
                break;
            }

            advance(p);
        }
    }

    consume(p, TOK_RPAREN);
    consume(p, TOK_COLON);

    proc->ret_type = p->tok->kind;

    consume_type(p);

    stmt->kind = STMT_PROC;
    stmt->body = proc;

    if (p->tok->kind == TOK_SEMICOLON) {
        advance(p);
        return;
    }

    consume(p, TOK_LCURLY);

    mem_make(&proc->vars, 256);
    mem_make(&proc->stmts, 256);

    while (p->tok->kind == TOK_VAR) {
        Stmt *s;
        Stmt_Proc_Var *var;

        mem_grow(&proc->vars);
        s = mem_next(&proc->vars);

        var = malloc(sizeof(Stmt_Proc_Var));
        if (var == NULL) {
            perror("procedure");
            exit(1);
        }

        consume(p, TOK_VAR);

        var->ident = p->tok;

        consume(p, TOK_IDENT);
        consume(p, TOK_COLON);

        var->type = p->tok->kind;

        consume_type(p);

        if (p->tok->kind == TOK_EQ) {
            advance(p);
            expression(p, &var->value);
        }

        consume(p, TOK_SEMICOLON);

        s->kind = STMT_PROC_VAR;
        s->body = var;
    }

    while (p->tok->kind != TOK_RCULRY && p->tok->kind != TOK_EOF) {
        Stmt *s;

        mem_grow(&proc->stmts);
        s = mem_next(&proc->stmts);
        statement(p, s);
    }

    consume(p, TOK_RCULRY);
}

static void variable(Parser *p, Stmt *stmt, Token *storage)
{
    Stmt_Var *var = malloc(sizeof(Stmt_Var));
    if (var == NULL) {
        perror("variable");
        exit(1);
    }

    memset(var, 0, sizeof(Stmt_Var));

    if (storage != NULL) {
        var->storage = storage->kind;
    }

    consume(p, TOK_VAR);

    var->ident = p->tok;

    consume(p, TOK_IDENT);
    consume(p, TOK_COLON);

    var->type = p->tok->kind;

    consume_type(p);

    if (p->tok->kind == TOK_EQ) {
        Expr_Lit *lit;

        advance(p);
        expression(p, &var->value);

        if (var->value.kind != EXPR_LIT) {
            fprintf(stderr, "%s:%d: expected constant value\n", p->tok->pos.file, p->tok->pos.line);
            exit(1);
        }

        lit = var->value.body;
        if (lit->value->kind == TOK_IDENT) {
            fprintf(stderr, "%s:%d: expected constant value\n", p->tok->pos.file, p->tok->pos.line);
            exit(1);
        }
    }

    consume(p, TOK_SEMICOLON);

    stmt->kind = STMT_VAR;
    stmt->body = var;
}

static void declaration(Parser *p, Stmt *stmt)
{
    Token *storage = NULL;

    if (token_is_storage(p->tok->kind) == 1) {
        storage = p->tok;
        advance(p);
    }

    switch (p->tok->kind) {
    case TOK_VAR:
        variable(p, stmt, storage);
        break;
    case TOK_PROC:
        procedure(p, stmt, storage);
        break;
    default:
        fprintf(stderr, "%s:%d: expected declaration\n", p->tok->pos.file, p->tok->pos.line);
        exit(1);
    }
}

void parser_parse(Parser *p, Stmts *stmts)
{
    while (p->tok->kind != TOK_EOF) {
        Stmt *s;

        mem_grow(stmts);
        s = mem_next(stmts);

        declaration(p, s);
    }
}

void parser_make(Parser *p, Scanner *s)
{
    mem_make(&p->toks, 256);

    scanner_scan(s, &p->toks);

    p->tok = p->toks.buf;
    p->cur = 0;
}
