/*
 * program = declaration* EOF
 *
 * declaration =
 * extern_var = "extern" "var" IDENT ";"
 * var = "global"? "var" IDENT "=" LIT ";"
 *
 * statement =
 * stmt_expr =
 *
 * expression = primary
 * primary = IDENT|LIT|"(" expression ")"
 *
 * LIT = NUM|STR|ARR|OBJ|"true"|"false"
 *
 * OBJ   = "{" IDENT ":" LIT ("," IDENT ":" LIT)* "}"
 * ARR   = "[" LIT ("," LIT)* "]"
 * IDENT = char (char|digit)*
 * STR   = "<everything except ">"
 * NUM   = digit+
 *
 * char  = "a".."z"|"A".."Z"|"_"
 * digit = "0".."9"
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "../lib/mem.h"
#include "../lib/path.h"
#include "../lib/sstring.h"

#include "vmc.h"

static void expression(Expr *e, FILE *out)
{
    (void)out;
    switch (e->kind) {
    case EXPR_LIT:
        break;
    case EXPR_UNARY:
        break;
    case EXPR_BINARY:
        break;
    default:
        assert(0 && "unreachable");
    }
}

static void statement(Stmt *s, FILE *out)
{
    switch (s->kind) {
    case STMT_EXPR:
        expression(&s->as.stmt_expr.expr, out);
        break;
    default:
        assert(0 && "unreachable");
    }
}

int main(int argc, char **argv)
{
    Parser p;
    Scanner s;
    Stmts stmts;
    char *outpath;
    FILE *out;

    argc--;
    argv++;

    if (argc < 1) {
        fprintf(stderr, "Provide file to compile\n");
        return 1;
    }

    mem_make(&stmts, 256);
    scanner_make(&s, *argv);
    parser_make(&p, &s);

    parser_parse(&p, &stmts);

    outpath = create_outfile(*argv, "vmc");
    out = fopen(outpath, "w");
    if (out == NULL) {
        perror(NULL);
        return 1;
    }

    fprintf(out, ".global _start\n"
                 "_start:\n");

    for (int i = 0; i < stmts.len; ++i) {
        Stmt *s = stmts.buf + i;
        statement(s, out);
    }

    fprintf(out, "halt\n");

    fclose(out);

    return 0;
}
