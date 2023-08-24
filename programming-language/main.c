/*
 * program = statement* EOF
 *
 * statement = block_stmt|let_stmt|assign_stmt|proc_stmt
 *
 * block_stmt  = "{" statement* "}"
 * let_stmt    = "let" ident ("," ident)* ";"
 * assign_stmt = ident "=" expression ";"
 * proc_stmt   = "proc" ident "(" ")" block_stmt
 * ret_stmt    = "return" ";"
 * halt_sttm   = "halt" ";"
 *
 * expression = term
 *
 * term    = primary ("+" primary)*
 * primary = ident|number
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
    int value;
} Var;

struct {
    int len;
    int cap;
    int data_size;
    Var *buf;
} variables;

void declare_var(string *name, int offset)
{
    Var *var;

    var = memnext(&variables);

    string_dup(&var->name, name);
    var->offset = offset;
    var->value = 0;
}

Var *lookup_var(string *name)
{
    for (int i = 0; i < variables.len; ++i) {
        Var *tmp;

        tmp = variables.buf + i;
        if (string_cmp(&tmp->name, name) == 1) {
            return tmp;
        }
    }

    return NULL;
}

void eval(Expr *e, FILE *out)
{
    Var *var;
    char *lex;

    Expr_Lit *lit;
    Expr_Binary *binary;

    switch (e->kind) {
    case EXPR_LIT:
        lit = e->body;

        switch (lit->value->kind) {
        case TOK_NUM:
            lex = string_toc(&lit->value->lex);
            fprintf(out, "push %d\n", atoi(lex));
            free(lex);
            break;
        case TOK_IDENT:
            var = lookup_var(&lit->value->lex);
            assert(var != NULL);
            fprintf(out, "push _fp ld push %d add ld\n", var->offset * 2);
            break;
        default:
            assert(0 && "unreachable");
        }

        break;
    case EXPR_BINARY:
        binary = e->body;

        eval(&binary->x, out);
        eval(&binary->y, out);

        switch (binary->op->kind) {
        case TOK_PLUS:
            fprintf(out, "add\n");
            break;

        default:
            assert(0 && "unreachable");
        }

        break;
    default:
        assert(0 && "unreachable");
    }
}

void generate(Stmt *s, FILE *out)
{
    Var *var;

    Stmt_Proc *proc;
    Stmt_Block *block;
    Stmt_Let *let;
    Stmt_Assign *assign;

    switch (s->kind) {
    case STMT_BLOCK:
        block = s->body;

        for (int i = 0; i < block->stmts.len; ++i) {
            generate(block->stmts.buf + i, out);
        }
        break;
    case STMT_LET:
        let = s->body;

        for (int i = 0; i < let->idents.len; ++i) {
            Token *tmp;

            tmp = let->idents.buf[i];
            declare_var(&tmp->lex, i + 1);
            fprintf(out, "push 0\n");
        }
        break;
    case STMT_ASSIGN:
        assign = s->body;

        eval(&assign->value, out);
        var = lookup_var(&assign->ident->lex);
        assert(var != NULL);

        fprintf(out, "push _fp ld push %d add st\n", var->offset * 2);
        break;
    case STMT_PROC:
        proc = s->body;

        fprintf(out, "%.*s:\n", proc->ident->lex.len, proc->ident->lex.ptr);
        generate(&proc->body, out);
        break;
    case STMT_RET:
        fprintf(out, "ret\n");
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
    Stmts ss;

    argc--;
    argv++;

    if (argc < 1) {
        fprintf(stderr, "Provide file to compile\n");
        return 1;
    }

    meminit(&variables, sizeof(Var), 256);
    meminit(&ss, sizeof(Stmt), 256);
    make_parser(&p, *argv);

    parse(&p, &ss);

    outpath = create_outfile(*argv, "asm");
    out = fopen(outpath, "w");
    if (out == NULL) {
        perror(NULL);
        return 1;
    }

    fprintf(out, ".global _start\n"
                 "_start:\n"
                 "call main\n"
                 "halt\n");

    for (int i = 0; i < ss.len; ++i) {
        Stmt *s;

        s = ss.buf + i;

        generate(s, out);
    }

    return 0;
}
