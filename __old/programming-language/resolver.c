#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "../lib/mem.h"
#include "../lib/sstring.h"

#include "vmc.h"

#include "resolver.h"

typedef struct {
    string name;
    int defined;
    Token_Kind storage;
    enum {
        SYM_VAR = 0,
        SYM_PROC
    } kind;
} Symbol;

typedef struct scope {
    struct scope *prev;
    struct {
        int len;
        int cap;
        Symbol *buf;
    } symtab;
} Scope;

static Symbol *lookup(Resolver *r, string *name)
{
    for (Scope *s = r->scope; s != NULL; s = s->prev) {
        for (int i = 0; i < s->symtab.len; ++i) {
            Symbol *sym = s->symtab.buf + i;

            if (string_cmp(&sym->name, name) == 1) {
                return sym;
            }
        }
    }

    return NULL;
}

static void declare_proc(Resolver *r, Stmt_Proc *proc)
{
    Symbol *sym = lookup(r, &proc->ident->lex);

    if (sym != NULL) {
        if (sym->kind == SYM_PROC &&
                sym->storage == proc->storage &&
                sym->defined == 0) {
            sym->defined = 1;
            return;
        }
        fprintf(stderr, "%s:%d: %.*s already declared\n", proc->ident->pos.file, proc->ident->pos.line, proc->ident->lex.len, proc->ident->lex.ptr);
        exit(1);
    }

    mem_grow(&r->scope->symtab);
    sym = mem_next(&r->scope->symtab);

    string_dup(&sym->name, &proc->ident->lex);
    sym->kind = SYM_PROC;
    sym->storage = proc->storage;
    sym->defined = proc->stmts.buf != NULL;
}

static void declare_var(Resolver *r, Token *ident, Token_Kind storage,
                        int defined)
{
    Symbol *sym = lookup(r, &ident->lex);

    if (sym != NULL) {
        fprintf(stderr, "%s:%d: %.*s already declared\n", ident->pos.file, ident->pos.line, ident->lex.len, ident->lex.ptr);
        exit(1);
    }

    mem_grow(&r->scope->symtab);
    sym = mem_next(&r->scope->symtab);

    string_dup(&sym->name, &ident->lex);
    sym->kind = SYM_VAR;
    sym->storage = storage;
    if (storage == TOK_EXTERN) {
        defined = 1;
    }
    sym->defined = defined;
}

static void define(Resolver *r, Token *ident)
{
    Symbol *sym = lookup(r, &ident->lex);

    if (sym == NULL) {
        fprintf(stderr, "%s:%d: %.*s does not exist\n", ident->pos.file, ident->pos.line, ident->lex.len, ident->lex.ptr);
        exit(1);
    }

    sym->defined = 1;
}

static Scope *make_scope(Scope *prev)
{
    Scope *s = malloc(sizeof(Scope));
    if (s == NULL) {
        perror("make_scope");
        exit(1);
    }

    mem_make(&s->symtab, 256);
    s->prev = prev;

    return s;
}

static void push_scope(Resolver *r)
{
    r->scope = make_scope(r->scope);
}

static void pop_scope(Resolver *r)
{
    Scope *prev;

    assert(r->scope->prev != NULL);

    prev = r->scope->prev;
    free(r->scope);

    r->scope = prev;
}

static void resolve_expression(Resolver *r, Expr *expr)
{
    Symbol *sym;
    Expr_Var *var;
    Expr_Call *call;
    Expr_Binary *binary;

    switch (expr->kind) {
    case EXPR_LIT:
		break;
    case EXPR_BINARY:
        binary = expr->body;
        resolve_expression(r, &binary->x);
        resolve_expression(r, &binary->y);
		break;
    case EXPR_VAR:
        var = expr->body;
        sym = lookup(r, &var->ident->lex);

        if (sym == NULL) {
            fprintf(stderr, "%s:%d: %.*s does not exist\n", var->ident->pos.file, var->ident->pos.line, var->ident->lex.len, var->ident->lex.ptr);
            exit(1);
        }

        if (sym->kind == SYM_VAR && sym->defined == 0) {
            fprintf(stderr, "%s:%d: %.*s is not initialized\n", var->ident->pos.file, var->ident->pos.line, var->ident->lex.len, var->ident->lex.ptr);
            exit(1);
        }
        break;
    case EXPR_CALL:
        call = expr->body;
        resolve_expression(r, &call->callee);
        for (int i = 0; i < call->args.len; ++i) {
            Expr *arg = call->args.buf + i;
            resolve_expression(r, arg);
        }
		break;
    default:
        assert(0 && "unreachable");
    }
}

static void resolve_statement(Resolver *r, Stmt *stmt)
{
    Stmt_Var *var;
    Stmt_Ret *ret;
    Stmt_Proc *proc;
    Stmt_Assign *assign;
    Stmt_Expr *stmt_expr;
    Stmt_Proc_Var *proc_var;

    switch (stmt->kind) {
    case STMT_VAR:
        var = stmt->body;

        declare_var(r, var->ident, var->storage, var->value.body != NULL);
        if (var->value.body != NULL) {
            resolve_expression(r, &var->value);
        }
		break;
    case STMT_PROC:
        proc = stmt->body;

        declare_proc(r, proc);
        if (proc->stmts.buf != NULL) {
            push_scope(r);

            for (int i = 0; i < proc->params.len; ++i) {
                Proc_Param *param = proc->params.buf + i;
                declare_var(r, param->ident, 0, 1);
            }

            resolver_resolve(r, &proc->vars);
            resolver_resolve(r, &proc->stmts);

            pop_scope(r);
        }
		break;
    case STMT_PROC_VAR:
        proc_var = stmt->body;

        declare_var(r, proc_var->ident, 0, proc_var->value.body != NULL);
        if (proc_var->value.body != NULL) {
            resolve_expression(r, &proc_var->value);
        }
		break;
    case STMT_ASSIGN:
        assign = stmt->body;
        define(r, assign->ident);
        resolve_expression(r, &assign->value);
		break;
    case STMT_RET:
        ret = stmt->body;
        resolve_expression(r, &ret->value);
		break;
    case STMT_EXPR:
        stmt_expr = stmt->body;
        resolve_expression(r, &stmt_expr->expr);
		break;
    default:
        assert(0 && "unreachable");
    }
}

void resolver_resolve(Resolver *r, Stmts *stmts)
{
    for (int i = 0; i < stmts->len; ++i) {
        Stmt *s = stmts->buf + i;
        resolve_statement(r, s);
    }

    for (int i = 0; i < r->scope->symtab.len; ++i) {
        Symbol *proc = r->scope->symtab.buf + i;
        if (proc->kind != SYM_PROC) {
            continue;
        }

        if (proc->defined == 0 && proc->storage != TOK_EXTERN) {
            fprintf(stderr, "%.*s is not defined\n", proc->name.len, proc->name.ptr);
            exit(1);
        }
    }
}

void resolver_make(Resolver *r)
{
    r->scope = make_scope(NULL);
}
