/*
 * program = declaration* EOF
 *
 * declaration = procedure|file_variable
 * procedure = "proc" ident "(" ")" block
 * file_variable = "let" ident "=" number ";"
 * block_variable = "let" ident "=" expression ";"
 *
 * statement = assign|return|block
 * assign    = ident "=" expression ";"
 * return    = "return" ";"
 * block = "{" block_variable* statement* "}"
 *
 * expression = term
 * term = primary (("+"|"-") primary)*
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

typedef struct {
    string name;
    int offset;
} Symbol;

typedef struct scope {
    struct {
        int len;
        int cap;
        int data_size;
        Symbol *buf;
    } symtab;
    struct scope *prev;
} Scope;

void gen_decl(Decl *decl, Scope *scope, FILE *out);

void scope_make(Scope *new, Scope *prev)
{
    new->prev = prev;
    meminit(&new->symtab, sizeof(Symbol), 128);
}

void scope_declare_var(Scope *s, string *name, int is_file_var)
{
    Symbol *sym = memnext(&s->symtab);
    int offset = s->symtab.len;

    if (is_file_var) {
        offset = -1;
    }

    sym->offset = offset;
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
            if (sym->offset == -1) {
                fprintf(out, "push %.*s ld\n", sym->name.len, sym->name.ptr);
            } else {
                fprintf(out, "push _fp ld push %d add ld\n", sym->offset * 2);
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
    default:
        assert(0 && "unreachable");
    }
}

void gen_stmt(Stmt *stmt, Scope *scope, FILE *out)
{
    Stmt_Assign *assign;
    Stmt_Block *block;
    Scope new_scope;
    Symbol *sym;

    switch (stmt->kind) {
    case STMT_BLOCK:
        block = stmt->body;
        scope_make(&new_scope, scope);

        for (int i = 0; i < block->decls.len; ++i) {
            Decl *tmp = block->decls.buf + i;
            gen_decl(tmp, &new_scope, out);
        }

        for (int i = 0; i < block->stmts.len; ++i) {
            Stmt *tmp = block->stmts.buf + i;
            gen_stmt(tmp, &new_scope, out);
        }

        break;
    case STMT_ASSIGN:
        assign = stmt->body;
        sym = scope_lookup_var(scope, &assign->ident->lex);

        gen_expr(&assign->value, scope, out);
        if (sym->offset == -1) {
            fprintf(out, "push %.*s\n", sym->name.len, sym->name.ptr);
        } else {
            fprintf(out, "push _fp ld push %d add st\n", sym->offset * 2);
        }
        break;
    case STMT_RET:
        fprintf(out, "ret\n");
        break;
    default:
        assert(0 && "unreachable");
    }
}

void gen_decl(Decl *decl, Scope *scope, FILE *out)
{
    Decl_Var *var;
    Decl_Proc *proc;

    switch (decl->kind) {
    case DECL_FILE_VAR:
        var = decl->body;
        scope_declare_var(scope, &var->ident->lex, 1);
        fprintf(out, "%.*s:", var->ident->lex.len, var->ident->lex.ptr);
        gen_expr(&var->value, scope, out);
        break;
    case DECL_BLOCK_VAR:
        var = decl->body;
        scope_declare_var(scope, &var->ident->lex, 0);
        gen_expr(&var->value, scope, out);
        break;
    case DECL_PROC:
        proc = decl->body;
        fprintf(out, "%.*s:\n", proc->ident->lex.len, proc->ident->lex.ptr);
        gen_stmt(&proc->body, scope, out);
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
