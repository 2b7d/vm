#include <stdio.h>

#include "scanner.h"

int main(int argc, char **argv)
{
    struct scanner s;

    argc--;
    argv++;

    if (argc < 1) {
        fprintf(stderr, "Provide file to assemble\n");
        return 1;
    }

    make_scanner(&s, *argv);

    for (;;) {
        struct token t;

        t.kind = TOK_ERR;
        t.lex = NULL;

        scan_token(&s, &t);

        printf("token: %d", t.kind);
        if (t.lex != NULL) {
            printf(" %s", t.lex);
        }
        printf("\n");

        if (t.kind == TOK_EOF) {
            break;
        }
    }

    return 0;
}
