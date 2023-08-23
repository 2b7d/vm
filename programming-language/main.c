/*
 * program = statement* EOF
 *
 * statement = block_stmt|let_stmt|assign_stmt|proc_stmt
 *
 * block_stmt  = "{" statement* "}"
 * let_stmt    = "let" ident ":" "word" ";"
 * assign_stmt = ident "=" expression ";"
 * proc_stmt   = "proc" ident "(" ")" ":" "void" block_stmt
 * ret_stmt    = "return" ";"
 *
 * expression = term
 *
 * term    = primary (("+"|"-") primary)*
 * primary = ident|number
 *
 * ident  = char (char|digit)
 * number = digit+
 *
 * char  = "a".."z"|"A".."Z"|"_"
 * digit = "0".."9"
 */

#include <stdio.h>

#include "../lib/sstring.h"
#include "../lib/mem.h"
#include "../lib/path.h"

#include "scanner.h"
#include "parser.h"

void generate(Stmt *s, FILE *out)
{
    switch (s->kind) {
    case STMT_BLOCK:
        break;
    case STMT_LET:
        break;
    case STMT_ASSIGN:
        break;
    case STMT_PROC:
        break;
    case STMT_RET:
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

    meminit(&ss, sizeof(Stmt), 256);
    make_parser(&p, *argv);

    parse(&p, &ss);

    outpath = create_outfile(*argv, "asm");
    out = fopen(outpath, "w");
    if (out == NULL) {
        perror(NULL);
        return 1;
    }

    for (int i = 0; i < ss.len; ++i) {
        Stmt *s;

        s = ss.buf + i;

        generate(s, out);
    }

    return 0;
}
