/*
 * program = declaration* EOF
 *
 * declaration    = procedure|file_variable
 * procedure      = "proc" ident "(" (ident ("," ident)*)? ")" proc_block
 * file_variable  = "let" ident "=" number ";"
 * block_variable = "let" ident "=" expression ";"
 *
 * statement = assign|return|stmt_expr
 * assign    = ident "=" expression ";"
 * return    = "return" ";"
 * stmt_expr = expression ";"
 * block     = "{" statement* "}"
 * proc_bloc = "{" block_variable* statement* "}"
 *
 * expression = term
 * term = call (("+"|"-") call)*
 * call = primary ("("(expression ("," expression)*)?")")?
 * primary = ident|number|"(" expression ")"
 *
 * ident  = char (char|digit)
 * number = digit+
 *
 * char  = "a".."z"|"A".."Z"|"_"
 * digit = "0".."9"
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "../lib/sstring.h"
#include "../lib/mem.h"
#include "../lib/path.h"

#include "scanner.h"
#include "parser.h"

typedef enum {
    SYM_FILE = 0,
    SYM_PROC,
    SYM_BLOCK,
    SYM_ARG
} Symbol_Kind;

typedef struct {
    string name;
    int offset;
    Symbol_Kind kind;
} Symbol;

typedef struct scope {
    struct {
        int len;
        int cap;
        int data_size;
        Symbol *buf;
    } symtab;

    int offset;
    int arg_offset;

    struct scope *prev;
} Scope;

void gen_decl(Decl *decl, Scope *scope, FILE *out);

void scope_make(Scope *new, Scope *prev)
{
    meminit(&new->symtab, sizeof(Symbol), 128);

    new->prev = prev;
    new->offset = 1;
    new->arg_offset = 1;
}

void scope_declare_var(Scope *s, string *name, Symbol_Kind kind)
{
    Symbol *sym = memnext(&s->symtab);
    int offset = -1;

    if (kind == SYM_BLOCK) {
        offset = s->offset;
        s->offset++;
    } else if (kind == SYM_ARG) {
        offset = s->arg_offset;
        s->arg_offset++;
    }

    sym->offset = offset;
    sym->kind = kind;
    string_dup(&sym->name, name);
}

Symbol *scope_lookup_var(Scope *s, string *name)
{
    while (s != NULL) {
        for (int i = 0; i < s->symtab.len; ++i) {
            Symbol *sym = s->symtab.buf + i;

            if (string_cmp(&sym->name, name) == 1) {
                return sym;
            }
        }

        s = s->prev;
    }

    return NULL;
}

void gen_expr(Expr *expr, Scope *scope, FILE *out)
{
    Expr_Binary *binary;
    Expr_Call *call;
    Expr_Lit *lit;
    Symbol *sym;
    char *lex;

    switch (expr->kind) {
    case EXPR_CONST:
        lit = expr->body;

        assert(lit->value->kind == TOK_NUM);

        lex = string_toc(&lit->value->lex);
        fprintf(out, ".word %d\n", atoi(lex));
        free(lex);
        break;
    case EXPR_LIT:
        lit = expr->body;

        switch (lit->value->kind) {
        case TOK_NUM:
            lex = string_toc(&lit->value->lex);
            fprintf(out, "push %d\n", atoi(lex));
            free(lex);
            break;
        case TOK_IDENT:
            sym = scope_lookup_var(scope, &lit->value->lex);
            assert(sym != NULL);

            switch (sym->kind) {
            case SYM_FILE:
                fprintf(out, "push %.*s ld\n", sym->name.len, sym->name.ptr);
                break;
            case SYM_BLOCK:
                fprintf(out, "push _fp ld push %d add ld\n", sym->offset * 2);
                break;
            case SYM_ARG:
                fprintf(out, "push _fp ld push %d sub ld\n", sym->offset * 2 + 2);
                break;
            case SYM_PROC:
                fprintf(out, "call %.*s\n", sym->name.len, sym->name.ptr);
                break;
            default:
                assert(0 && "unreachable");
            }
            break;
        default:
            assert(0 && "unreachable");
        }

        break;
    case EXPR_BINARY:
        binary = expr->body;

        gen_expr(&binary->x, scope, out);
        gen_expr(&binary->y, scope, out);

        switch (binary->op->kind) {
        case TOK_PLUS:
            fprintf(out, "add\n");
            break;
        case TOK_MINUS:
            fprintf(out, "sub\n");
            break;
        default:
            assert(0 && "unreachable");
        }

        break;
    case EXPR_CALL:
        call = expr->body;

        for (int i = 0; i < call->args.len; ++i) {
            Expr *arg = call->args.buf + i;
            gen_expr(arg, scope, out);
        }

        gen_expr(&call->callee, scope, out);

        for (int i = 0; i < call->args.len; ++i) {
            fprintf(out, "drop\n");
        }

        fprintf(out, "push _rp ld\n");

        break;
    default:
        assert(0 && "unreachable");
    }
}

void gen_stmt(Stmt *stmt, Scope *scope, FILE *out)
{
    Stmt_Proc_Block *proc_block;
    Stmt_Expr *stmt_expr;
    Stmt_Assign *assign;
    Stmt_Block *block;
    Stmt_Ret *ret;
    Symbol *sym;

    switch (stmt->kind) {
    case STMT_PROC_BLOCK:
        proc_block = stmt->body;

        for (int i = 0; i < proc_block->decls.len; ++i) {
            Decl *tmp = proc_block->decls.buf + i;
            gen_decl(tmp, scope, out);
        }

        for (int i = 0; i < proc_block->stmts.len; ++i) {
            Stmt *tmp = proc_block->stmts.buf + i;
            gen_stmt(tmp, scope, out);
        }

        break;
    case STMT_BLOCK:
        block = stmt->body;

        for (int i = 0; i < block->stmts.len; ++i) {
            Stmt *tmp = block->stmts.buf + i;
            gen_stmt(tmp, scope, out);
        }

        break;
    case STMT_ASSIGN:
        assign = stmt->body;

        sym = scope_lookup_var(scope, &assign->ident->lex);
        gen_expr(&assign->value, scope, out);

        switch (sym->kind) {
        case SYM_FILE:
            fprintf(out, "push %.*s st\n", sym->name.len, sym->name.ptr);
            break;
        case SYM_BLOCK:
            fprintf(out, "push _fp ld push %d add st\n", sym->offset * 2);
            break;
        case SYM_ARG:
            fprintf(out, "push _fp ld push %d sub st\n", sym->offset * 2 + 2);
            break;
        case SYM_PROC:
            fprintf(stderr, "can not reassign procedure\n");
            exit(1);
        default:
            assert(0 && "unreachable");
        }
        break;
    case STMT_RET:
        ret = stmt->body;

        if (ret->has_value) {
            gen_expr(&ret->value, scope, out);
        }

        fprintf(out, "ret\n");
        break;
    case STMT_EXPR:
        stmt_expr = stmt->body;
        gen_expr(&stmt_expr->expr, scope, out);
        break;
    default:
        assert(0 && "unreachable");
    }
}

void gen_decl(Decl *decl, Scope *scope, FILE *out)
{
    Decl_Var *var;
    Decl_Proc *proc;
    Scope new_scope;

    switch (decl->kind) {
    case DECL_FILE_VAR:
        var = decl->body;

        scope_declare_var(scope, &var->ident->lex, SYM_FILE);
        fprintf(out, "%.*s:", var->ident->lex.len, var->ident->lex.ptr);
        gen_expr(&var->value, scope, out);
        break;
    case DECL_BLOCK_VAR:
        var = decl->body;

        scope_declare_var(scope, &var->ident->lex, SYM_BLOCK);
        gen_expr(&var->value, scope, out);
        break;
    case DECL_PROC:
        proc = decl->body;

        scope_declare_var(scope, &proc->ident->lex, SYM_PROC);
        fprintf(out, "%.*s:\n", proc->ident->lex.len, proc->ident->lex.ptr);

        scope_make(&new_scope, scope);

        for (int i = 0; i < proc->params.len; ++i) {
            Token *arg = proc->params.buf[i];
            scope_declare_var(&new_scope, &arg->lex, SYM_ARG);
        }

        gen_stmt(&proc->body, &new_scope, out);
        break;
    default:
        assert(0 && "unreachable");
    }
}

int main(int argc, char **argv)
{
    char *outpath;
    FILE *out;
    Parser p;
    Decls decls;
    Scope global_scope;

    argc--;
    argv++;

    if (argc < 1) {
        fprintf(stderr, "Provide file to compile\n");
        return 1;
    }

    scope_make(&global_scope, NULL);
    meminit(&decls, sizeof(Decl), 256);
    make_parser(&p, *argv);

    parse(&p, &decls);

    outpath = create_outfile(*argv, "asm");
    out = fopen(outpath, "w");
    if (out == NULL) {
        perror(NULL);
        return 1;
    }

    // TODO(art): bug in assembler if put this after generation
    fprintf(out, ".global _start\n"
                 "_start:\n"
                 "call main\n"
                 "halt\n");

    for (int i = 0; i < decls.len; ++i) {
        Decl *decl;

        decl = decls.buf + i;
        gen_decl(decl, &global_scope, out);
    }

    return 0;
}
