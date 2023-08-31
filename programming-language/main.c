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

#include "../lib/mem.h"
#include "../lib/sstring.h"

#include "vmc.h"

#include "resolver.h"
#include "semantic.h"

int main(int argc, char **argv)
{
    Parser p;
    Scanner s;
    Stmts stmts;
    Resolver r;
    Checker c;

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

    parser_parse(&p, &stmts);
    resolver_resolve(&r, &stmts);
    semantic_check(&c, &stmts);

    return 0;
}
