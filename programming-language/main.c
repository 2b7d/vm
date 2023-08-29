/*
 * program = declaration* EOF
 *
 * declaration = var|proc
 * var  = storage? "var" IDENT ":" type ("=" expression)? ";"
 * proc = storage? "proc" IDENT "(" params? ")" ":" type (";"|proc_body)
 * proc_body = "{" var* statement* "}"
 *
 * statement = assign|return|stmt_expr
 * assign    = IDENT "=" expression ";"
 * return    = "return" expression? ";"
 * stmt_expr = expression ";"
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

#include "token.h"
#include "scanner.h"
#include "parser.h"

int main(int argc, char **argv)
{
    Parser p;
    Scanner s;
    Decls decls;

    argc--;
    argv++;

    if (argc < 1) {
        fprintf(stderr, "Provide file to compile\n");
        return 1;
    }

    mem_make(&decls, 256);
    scanner_make(&s, *argv);
    parser_make(&p, &s);

    parser_parse(&p, &decls);

    for (int i = 0; i < decls.len; ++i) {
        Decl d;

        d = decls.buf[i];
    }

    return 0;
}
