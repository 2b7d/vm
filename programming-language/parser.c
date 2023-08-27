#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
        fprintf(stderr, "%s:%d: expected %s but got %s\n", p->tok->pos.file, p->tok->pos.line, scanner_tokstr(kind), scanner_tokstr(p->tok->kind));
        exit(1);
    }

    advance(p);
}

static void consume_type(Parser *p)
{
    if (scanner_is_type(p->tok->kind) == 0) {
        fprintf(stderr, "%s:%d: expected type but got %s\n", p->tok->pos.file, p->tok->pos.line, scanner_tokstr(p->tok->kind));
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
        fprintf(stderr, "%s:%d: expected number or identifier but got %s\n", p->tok->pos.file, p->tok->pos.line, scanner_tokstr(p->tok->kind));
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
    mem_make(&call->args, 256);

    if (p->tok->kind != TOK_RPAREN) {
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

static void constant_expression(Parser *p, Expr *expr)
{
    Expr_Lit *lit;

    if (p->tok->kind != TOK_NUM) {
        fprintf(stderr, "%s:%d: expected constant expression\n", p->tok->pos.file, p->tok->pos.line);
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
    ret->has_value = 0;

    consume(p, TOK_RET);

    if (p->tok->kind != TOK_SEMICOLON) {
        expression(p, &ret->value);
        ret->has_value = 1;
    }

    consume(p, TOK_SEMICOLON);

    stmt->kind = STMT_RET;
    stmt->body = ret;
}

static Decl_Var *variable(Parser *p)
{
    Decl_Var *var = malloc(sizeof(Decl_Var));
    if (var == NULL) {
        perror("variable");
        exit(1);
    }

    consume(p, TOK_LET);

    var->ident = p->tok;

    consume(p, TOK_IDENT);
    consume(p, TOK_COLON);

    var->type = p->tok;

    consume_type(p);
    consume(p, TOK_EQ);

    return var;
}

static void block_variable(Parser *p, Decl *decl)
{

    Decl_Var *var = variable(p);

    expression(p, &var->value);
    consume(p, TOK_SEMICOLON);

    decl->kind = DECL_VAR;
    decl->body = var;
}

static void proc_block(Parser *p, Stmt *stmt)
{
    Stmt_Proc_Block *block = malloc(sizeof(Stmt_Proc_Block));
    if (block == NULL) {
        perror("proc_block");
        exit(1);
    }

    consume(p, TOK_LCURLY);

    mem_make(&block->stmts, 64);
    mem_make(&block->decls, 64);

    while (p->tok->kind == TOK_LET) {
        Decl *tmp;

        mem_grow(&block->decls);
        tmp = mem_next(&block->decls);
        block_variable(p, tmp);
    }

    while (p->tok->kind != TOK_RCULRY && p->tok->kind != TOK_EOF) {
        Stmt *tmp;

        mem_grow(&block->stmts);
        tmp = mem_next(&block->stmts);
        statement(p, tmp);
    }

    consume(p, TOK_RCULRY);

    stmt->kind = STMT_PROC_BLOCK;
    stmt->body = block;
}

//static void block(Parser *p, Stmt *stmt)
//{
//    Stmt_Block *block = malloc(sizeof(Stmt_Block));
//    if (block == NULL) {
//        perror("parse_block");
//        exit(1);
//    }

//    consume(p, TOK_LCURLY);

//    meminit(&block->stmts, sizeof(Stmt), 64);

//    while (p->tok->kind != TOK_RCULRY && p->tok->kind != TOK_EOF) {
//        Stmt *tmp = memnext(&block->stmts);
//        statement(p, tmp);
//    }

//    consume(p, TOK_RCULRY);

//    stmt->kind = STMT_BLOCK;
//    stmt->body = block;
//}

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
    case TOK_RET:
        _return(p, stmt);
        break;
    default:
        fprintf(stderr, "%s:%d: expected statement\n", p->tok->pos.file, p->tok->pos.line);
        exit(1);
    }
}

static void file_variable(Parser *p, Decl *decl)
{
    Decl_Var *var = variable(p);

    constant_expression(p, &var->value);
    consume(p, TOK_SEMICOLON);

    decl->kind = DECL_VAR;
    decl->body = var;
}

static void procedure(Parser *p, Decl *decl)
{
    Decl_Proc *proc = malloc(sizeof(Decl_Proc));
    if (proc == NULL) {
        perror("procedure");
        exit(1);
    }
    mem_make(&proc->params, 256);

    consume(p, TOK_PROC);

    proc->ident = p->tok;

    consume(p, TOK_IDENT);
    consume(p, TOK_LPAREN);

    if (p->tok->kind != TOK_RPAREN) {
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

            param->type = p->tok;
            consume_type(p);

            if (p->tok->kind != TOK_COMMA) {
                break;
            }

            advance(p);
        }
    }

    consume(p, TOK_RPAREN);
    consume(p, TOK_COLON);

    proc->ret_type = p->tok;
    consume_type(p);

    proc_block(p, &proc->body);

    decl->kind = DECL_PROC;
    decl->body = proc;
}

static void declaration(Parser *p, Decl *decl)
{
    switch (p->tok->kind) {
    case TOK_PROC:
        procedure(p, decl);
        break;
    case TOK_LET:
        file_variable(p, decl);
        break;
    default:
        fprintf(stderr, "%s:%d: expected declaration\n", p->tok->pos.file, p->tok->pos.line);
        exit(1);
    }
}

void parser_parse(Parser *p, Decls *decls)
{
    Decl *d;

    while (p->tok->kind != TOK_EOF) {
        mem_grow(decls);
        d = mem_next(decls);

        declaration(p, d);
    }
}

void parser_make(Parser *p, Scanner *s)
{
    mem_make(&p->toks, 256);

    scanner_scan(s, &p->toks);

    p->tok = p->toks.buf;
    p->cur = 0;
}
