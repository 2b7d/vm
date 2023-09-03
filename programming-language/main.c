/*
 * program = (var|proc)* EOF
 *
 * var       = storage? "var" IDENT ":" type ("=" NUM)? ";"
 * proc      = storage? "proc" IDENT "(" params? ")" ":" type (";"|proc_body)
 * proc_var  = "var" IDENT ":" type ("=" expression)? ";"
 * assign    = IDENT "=" expression ";"
 * return    = "return" expression? ";"
 * stmt_expr = expression ";"
 *
 * proc_body = "{" proc_var* statement* "}"
 * statement = assign|return|stmt_expr
 *
 * expression = term
 * term    = call (("+"|"-") call)*
 * call    = primary make_call?
 * primary = IDENT|NUM|"(" expression ")"
 *
 * storage   = "extern"|"global"
 * type      = "u8"|"u16"|"void"
 * params    = param ("," param)*
 * param     = IDENT ":" type
 * make_call = "(" args? ")"
 * args      = expression ("," expression)*
 *
 * IDENT = char (char|digit)
 * NUM   = digit+
 *
 * char  = "a".."z"|"A".."Z"|"_"
 * digit = "0".."9"
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "../lib/mem.h"
#include "../lib/sstring.h"

#include "vmc.h"

#include "resolver.h"
#include "semantic.h"

typedef struct {
    string name;
    int offset;

    enum {
        SYM_PROC = 0,
        SUM_VAR,
        SYM_LOC,
        SYM_ARG
    } kind;
} Symbol;

typedef struct scope {
    struct scope *prev;
    int stack_offset;
    struct {
        int len;
        int cap;
        Symbol *buf;
    } symtab;
} Scope;

typedef struct {
    Scope *scope;
    Stmt_Proc *proc;
    FILE *out;
} Gen;

static void generate_expression(Expr *expr, Token_Kind size, FILE *out)
{
    Expr_Lit *lit;
    Expr_Var *var;
    Expr_Call *call;
    Expr_Binary *binary;
    Symbol *sym;

    switch (expr->kind) {
    case EXPR_LIT:
        lit = expr->body;

        if (cur_proc == NULL) {
            if (size == TOK_U8) {
                fprintf(out, ".byte");
            } else {
                fprintf(out, ".word");
            }
        } else {
            fprintf(out, "push");
            if (size == TOK_U8) {
                fprintf(out, "b");
            }
        }

        if (lit == NULL) {
            fprintf(out, " 0\n");
        } else {
            switch (lit->value->kind) {
            case TOK_NUM:
                fprintf(out, " %d\n", lit->value->value.as_num);
                break;
            default:
                assert(0 && "unreachable");
            }
        }
		break;
    case EXPR_BINARY:
        binary = expr->body;

        generate_expression(&binary->x, size, out);
        generate_expression(&binary->y, size, out);

        switch (binary->op->kind) {
        case TOK_PLUS:
            fprintf(out, "add");
            break;
        case TOK_MINUS:
            fprintf(out, "sub");
            break;
        default:
            assert(0 && "unreachable");
        }

        if (size == TOK_U8) {
            fprintf(out, "b\n");
        } else {
            fprintf(out, "\n");
        }
		break;
    case EXPR_VAR:
        assert(is_proc == 1);

        var = expr->body;

        // TODO(art): lookup symbol

        switch (sym->kind) {
        case SYM_PROC:
            fprintf(out, "call %.*s\n", sym->name.len, sym->name.ptr);
            return;
        case SYM_VAR:
            fprintf(out, "push %.*s ", sym->name.len, sym->name.ptr);
            break;
        case SYM_LOC:
            fprintf(out, "push _fp ld push %d add ", sym->offset);
            break;
        case SYM_ARG:
            fprintf(out, "push _fp ld push %d sub ", sym->offset);
            break;
        default:
            assert(0 && "unreachable");
        }

        if (sym->type == TOK_U8) {
            fprintf(out, "ldb\n");
        } else {
            fprintf(out, "ld\n");
        }
		break;
    case EXPR_CALL:
        call = expr->body;

        // TODO(art): lookup symbol

        for (int i = 0; i < call->args.len; ++i) {
            Expr *arg = call->args.buf + i;
            Token_Kind arg_size = sym->params.buf + i;

            generate_expression(arg, arg_size, out);
        }

        generate_expression(&call->callee, size, out);

        for (int i = 0; i < call->args.len; ++i) {
            fprintf(out, "drop\n");
        }

        if (sym->type != TOK_VOID) {
            fprintf(out, "push _rp ");
            if (sym->type == TOK_U8) {
                fprintf(out, "ldb\n");
            } else {
                fprintf(out, "ld\n");
            }
        }
		break;
    default:
        assert(0 && "unreachable");
    }
}

static void generate_statement(Gen *g, Stmt *stmt)
{
    Stmt_Var *var;
    Stmt_Ret *ret;
    Stmt_Proc *proc;
    Stmt_Assign *assign;
    Stmt_Expr *stmt_expr;
    Stmt_Proc_Var *proc_var;
    Symbol *sym;

    switch (stmt->kind) {
    case STMT_VAR:
        var = stmt->body;

        // TODO(art): add symbol

        switch (var->storage) {
        case TOK_GLOBAL:
            fprintf(g->out, ".global %.*s\n", var->ident->lex.len, var->ident->lex.ptr);
            break;
        case TOK_EXTERN:
            fprintf(g->out, ".extern %.*s\n", var->ident->lex.len, var->ident->lex.ptr);
            return;
        default:
            break;
        }

        fprintf(g->out, "%.*s:", var->ident->lex.len, var->ident->lex.ptr);
        generate_expression(g, &var->value, var->type);
		break;
    case STMT_PROC:
        proc = stmt->body;

        if (proc->storage == TOK_EXTERN) {
            fprintf(g->out, ".extern %.*s\n", proc->ident->lex.len, proc->ident->lex.ptr);
            return;
        }

        if (proc->stmts.buf == NULL) {
            return;
        }

        if (proc->storage == TOK_GLOBAL) {
            fprintf(g->out, ".global %.*s\n", proc->ident->lex.len, proc->ident->lex.ptr);
        }

        fprintf(g->out, "%.*s:\n", proc->ident->lex.len, proc->ident->lex.ptr);

        // TODO(art): add symbol

        g->proc = proc;
        // TODO(art): push scope

        for (int i = 0; i < proc->params.len; ++i) {
            Proc_Param *param = proc->params.buf + i;
            // TODO(art): add symbol
        }

        codegen_generate(g, &proc->vars);
        codegen_generate(g, &proc->stmts);

        // TODO(art): pop scope
        g->proc = NULL;
		break;
    case STMT_PROC_VAR:
        proc_var = stmt->body;

        // TODO(art): add symbol

        generate_expression(g, &proc_var->value, proc_var->type);
		break;
    case STMT_ASSIGN:
        assign = stmt->body;

        // TODO(art): lookup symbol

        generate_expression(g, &assign->value, sym->type);

        switch (sym->kind) {
        case SYM_VAR:
            fprintf(g->out, "push %.*s ", sym->name.len, sym->name.ptr);
            break;
        case SYM_LOC:
            fprintf(g->out, "push _fp ld push %d add ", sym->offset);
            break;
        case SYM_ARG:
            fprintf(g->out, "push _fp ld push %d sub ", sym->offset);
            break;
        case SYM_PROC:
        default:
            assert(0 && "unreachable");
        }

        if (sym->type == TOK_U8) {
            fprintf(g->out, "stb\n");
        } else {
            fprintf(g->out, "st\n");
        }
		break;
    case STMT_RET:
        ret = stmt->body;

        if (ret->value.body != NULL) {
            generate_expression(g, &ret->value, g->proc->ret_type);
            // TODO(art): maybe create separate instruction like retb?
            if (g->proc->ret_type == TOK_U8) {
                fprintf(out, "ctw\n");
            }
        }

        fprintf(out, "ret\n");
		break;
    case STMT_EXPR:
        3 + 2 - a;
        stmt_expr = stmt->body;
        generate_expression(g, &stmt_expr->expr, 0);
		break;
    default:
        assert(0 && "unreachable");
    }
}

void codegen_generate(Gen *g, Stmts *stmts);
{
    for (int i = 0; i < stmts->len; ++i) {
        Stmt *s = stmts->buf + i;
        generate_statement(g, s);
    }
}

void codegen_make(Gen *g, char *inpath)
{
    char *outpath;

    g->scope = make_scope(NULL);
    g->proc = NULL;

    outpath = create_outfile(inpath, "asm");
    g->out = fopen(outpath, "w");
    if (g->out == NULL) {
        perror(NULL);
        exit(1);
    }
}

int main(int argc, char **argv)
{
    Parser p;
    Scanner s;
    Stmts stmts;
    Resolver r;
    Checker c;
    Gen g;

    argc--;
    argv++;

    if (argc < 1) {
        fprintf(stderr, "Provide file to compile\n");
        return 1;
    }

    mem_make(&stmts, 256);
    scanner_make(&s, *argv);
    parser_make(&p, &s);
    resolver_make(&r);
    semantic_make_checker(&c);
    codegen_make(&g, *argv);

    parser_parse(&p, &stmts);
    resolver_resolve(&r, &stmts);
    semantic_check(&c, &stmts);
    codegen_generate(&g, &stmts);

    fprintf(g.out, ".global _start\n"
                   "_start:\n"
                   "call main\n"
                   "halt\n");

    return 0;
}
