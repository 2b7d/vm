#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../lib/sstring.h"
#include "../lib/mem.h"

#include "vmc.h"

#include "semantic.h"

typedef struct {
    Token_Kind type;
    string name;
    struct {
        int len;
        int cap;
        Proc_Param *buf;
    } params;
} Symbol;

typedef struct {
    Token *ident;
    Token_Kind type;
    Symbol *sym;
} Eval_Value;

typedef struct scope {
    struct scope *prev;
    struct {
        int len;
        int cap;
        Symbol *buf;
    } symtab;
} Scope;

void semantic_check(Checker *c, Stmts *stmts);

static Symbol *add_symbol(Checker *c, Token_Kind type, string *name, int proc)
{
    Symbol *sym;

    mem_grow(&c->scope->symtab);
    sym = mem_next(&c->scope->symtab);

    string_dup(&sym->name, name);
    sym->type = type;

    if (proc == 1) {
        mem_make(&sym->params, 256);
    }

    return sym;
}

static Symbol *get_symbol(Checker *c, string *name)
{
    for (Scope *s = c->scope; s != NULL; s = s->prev) {
        for (int i = 0; i < s->symtab.len; ++i) {
            Symbol *sym = s->symtab.buf + i;

            if (string_cmp(&sym->name, name) == 1) {
                return sym;
            }
        }
    }

    return NULL;
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

static void push_scope(Checker *c)
{
    c->scope = make_scope(c->scope);
}

static void pop_scope(Checker *c)
{
    Scope *prev;

    assert(c->scope->prev != NULL);

    prev = c->scope->prev;
    free(c->scope);

    c->scope = prev;
}

static void check_expression(Checker *c, Expr *expr, Eval_Value *ev)
{
    Expr_Lit *lit;
    Expr_Var *var;
    Expr_Call *call;
    Expr_Binary *binary;
    Eval_Value x_ev, y_ev;
    Symbol *sym;

    switch (expr->kind) {
    case EXPR_LIT:
        lit = expr->body;
        ev->ident = lit->value;

        switch (lit->value->kind) {
        case TOK_NUM:
            ev->type = TOK_NUM;
            break;
        default:
            assert(0 && "unreachable");
        }
        break;
    case EXPR_BINARY:
        binary = expr->body;

        memset(&x_ev, 0, sizeof(Eval_Value));
        memset(&y_ev, 0, sizeof(Eval_Value));

        check_expression(c, &binary->x, &x_ev);
        check_expression(c, &binary->y, &y_ev);

        switch (binary->op->kind) {
        case TOK_PLUS:
        case TOK_MINUS:
            if (token_is_numtype(x_ev.type) == 1 &&
                    token_is_numtype(y_ev.type) == 1 &&
                    x_ev.type == y_ev.type) {
                memcpy(ev, &x_ev, sizeof(Eval_Value));
                return;
            } else if (token_is_numtype(x_ev.type) == 1 &&
                    y_ev.type == TOK_NUM) {
                memcpy(ev, &x_ev, sizeof(Eval_Value));
                return;
            } else if (x_ev.type == TOK_NUM &&
                    token_is_numtype(y_ev.type) == 1) {
                memcpy(ev, &y_ev, sizeof(Eval_Value));
                return;
            } else if (x_ev.type == TOK_NUM && y_ev.type == TOK_NUM) {
                memcpy(ev, &x_ev, sizeof(Eval_Value));
                return;
            }

            fprintf(stderr, "%s:%d: invalid types %s %s for operator %s\n", binary->op->pos.file, binary->op->pos.line, token_str(x_ev.type), token_str(y_ev.type), token_str(binary->op->kind));
            exit(1);
        default:
            assert(0 && "unreachable");
        }
        break;
    case EXPR_VAR:
        var = expr->body;

        sym = get_symbol(c, &var->ident->lex);
        assert(sym != NULL);

        ev->ident = var->ident;
        ev->type = sym->type;
        ev->sym = sym;
        break;
    case EXPR_CALL:
        call = expr->body;

        memset(&x_ev, 0, sizeof(Eval_Value));
        check_expression(c, &call->callee, &x_ev);
        memcpy(ev, &x_ev, sizeof(Eval_Value));

        if (call->args.len != x_ev.sym->params.len) {
            fprintf(stderr, "%s:%d: expected %d arguments but got %d\n", x_ev.ident->pos.file, x_ev.ident->pos.line, x_ev.sym->params.len, call->args.len);
            exit(1);
        }

        for (int i = 0; i < call->args.len; ++i) {
            Expr *arg = call->args.buf + i;
            Proc_Param *param = x_ev.sym->params.buf + i;

            memset(&y_ev, 0, sizeof(Eval_Value));
            check_expression(c, arg, &y_ev);

            if (y_ev.type == TOK_NUM && token_is_numtype(param->type) == 1) {
                y_ev.type = param->type;
            }

            if (param->type != y_ev.type) {
                fprintf(stderr, "%s:%d: expected type %s but got %s\n", x_ev.ident->pos.file, x_ev.ident->pos.line, token_str(param->type), token_str(y_ev.type));
                exit(1);
            }
        }
		break;
    default:
        assert(0 && "unreachable");
    }
}

static void check_statement(Checker *c, Stmt *stmt)
{
    Stmt_Var *var;
    Stmt_Ret *ret;
    Stmt_Proc *proc;
    Stmt_Assign *assign;
    Stmt_Expr *stmt_expr;
    Stmt_Proc_Var *proc_var;
    Symbol *sym;
    Eval_Value ev;

    switch (stmt->kind) {
    case STMT_VAR:
        var = stmt->body;
        if (var->storage == TOK_EXTERN && var->value.body != NULL) {
            fprintf(stderr, "%s:%d: redeclaration of external variable\n", var->ident->pos.file, var->ident->pos.line);
            exit(1);
        }

        if (var->type == TOK_VOID) {
            fprintf(stderr, "%s:%d: can not use void type for variable\n", var->ident->pos.file, var->ident->pos.line);
            exit(1);
        }

        add_symbol(c, var->type, &var->ident->lex, 0);

        if (var->value.body != NULL) {
            memset(&ev, 0, sizeof(Eval_Value));

            check_expression(c, &var->value, &ev);

            if (ev.type == TOK_NUM && token_is_numtype(var->type) == 1) {
                ev.type = var->type;
            }

            if (var->type != ev.type) {
                fprintf(stderr, "%s:%d: expected type %s but got %s\n", var->ident->pos.file, var->ident->pos.line, token_str(var->type), token_str(ev.type));
                exit(1);
            }
        }
		break;
    case STMT_PROC:
        proc = stmt->body;

        if (proc->storage == TOK_EXTERN && proc->stmts.buf != NULL) {
            fprintf(stderr, "%s:%d: redeclaration of external procedure\n", proc->ident->pos.file, proc->ident->pos.line);
            exit(1);
        }

        sym = get_symbol(c, &proc->ident->lex);
        if (sym == NULL) {
            sym = add_symbol(c, proc->ret_type, &proc->ident->lex, 1);
        } else {
            if (sym->params.len != proc->params.len) {
                fprintf(stderr, "%s:%d: signature mismatch of procedure and prototype\n", proc->ident->pos.file, proc->ident->pos.line);
                exit(1);
            }

            if (sym->type != proc->ret_type) {
                fprintf(stderr, "%s:%d: signature mismatch of procedure and prototype\n", proc->ident->pos.file, proc->ident->pos.line);
                exit(1);
            }

            for (int i = 0; i < proc->params.len; ++i) {
                Proc_Param *procp, *protop;

                procp = proc->params.buf + i;
                protop = sym->params.buf + i;

                if (procp->type != protop->type) {
                    fprintf(stderr, "%s:%d: signature mismatch of procedure and prototype\n", proc->ident->pos.file, proc->ident->pos.line);
                    exit(1);
                }
            }
        }

        c->cur_proc = proc;
        push_scope(c);
        for (int i = 0; i < proc->params.len; ++i) {
            Proc_Param *pp, *sp;

            pp = proc->params.buf + i;

            add_symbol(c, pp->type, &pp->ident->lex, 0);

            mem_grow(&sym->params);
            sp = mem_next(&sym->params);
            memcpy(sp, pp, sizeof(Proc_Param));
        }

        if (proc->stmts.buf != NULL) {
            semantic_check(c, &proc->vars);
            semantic_check(c, &proc->stmts);
        }

        pop_scope(c);
        c->cur_proc = NULL;
		break;
    case STMT_PROC_VAR:
        proc_var = stmt->body;

        if (proc_var->type == TOK_VOID) {
            fprintf(stderr, "%s:%d: can not use void type for variable\n", proc_var->ident->pos.file, proc_var->ident->pos.line);
            exit(1);
        }

        add_symbol(c, proc_var->type, &proc_var->ident->lex, 0);

        if (proc_var->value.body != NULL) {
            memset(&ev, 0, sizeof(Eval_Value));

            check_expression(c, &proc_var->value, &ev);

            if (ev.type == TOK_NUM && token_is_numtype(proc_var->type) == 1) {
                ev.type = proc_var->type;
            }

            if (proc_var->type != ev.type) {
                fprintf(stderr, "%s:%d: expected type %s but got %s\n", proc_var->ident->pos.file, proc_var->ident->pos.line, token_str(proc_var->type), token_str(ev.type));
                exit(1);
            }
        }
		break;
    case STMT_ASSIGN:
        assign = stmt->body;

        sym = get_symbol(c, &assign->ident->lex);
        assert(sym != NULL);

        memset(&ev, 0, sizeof(Eval_Value));

        check_expression(c, &assign->value, &ev);

        if (ev.type == TOK_NUM && token_is_numtype(sym->type) == 1) {
            ev.type = sym->type;
        }

        if (sym->type != ev.type) {
            fprintf(stderr, "%s:%d: assigning type %s to %s\n", assign->ident->pos.file, assign->ident->pos.line, token_str(ev.type), token_str(sym->type));
            exit(1);
        }
		break;
    case STMT_RET:
        ret = stmt->body;
        if (c->cur_proc == NULL) {
            fprintf(stderr, "%s:%d: use of return outside of procedure\n", ret->tok->pos.file, ret->tok->pos.line);
            exit(1);
        }

        if (ret->value.body != NULL) {
            memset(&ev, 0, sizeof(Eval_Value));
            check_expression(c, &ret->value, &ev);

            if (ev.type == TOK_NUM &&
                    token_is_numtype(c->cur_proc->ret_type) == 1) {
                ev.type = c->cur_proc->ret_type;
            }

            if (c->cur_proc->ret_type != ev.type) {
                fprintf(stderr, "%s:%d: expected return type %s but got %s\n", ret->tok->pos.file, ret->tok->pos.line, token_str(c->cur_proc->ret_type), token_str(ev.type));
                exit(1);
            }
        } else {
            if (c->cur_proc->ret_type != TOK_VOID) {
                fprintf(stderr, "%s:%d: expected return type %s but got void\n", ret->tok->pos.file, ret->tok->pos.line, token_str(c->cur_proc->ret_type));
                exit(1);
            }
        }
		break;
    case STMT_EXPR:
        stmt_expr = stmt->body;
        memset(&ev, 0, sizeof(Eval_Value));
        check_expression(c, &stmt_expr->expr, &ev);
		break;
    default:
        assert(0 && "unreachable");
    }
}

void semantic_check(Checker *c, Stmts *stmts)
{
    for (int i = 0; i < stmts->len; ++i) {
        Stmt *s = stmts->buf + i;
        check_statement(c, s);
    }
}

void semantic_make_checker(Checker *c)
{
    c->scope = make_scope(NULL);
    c->cur_proc = NULL;
}
