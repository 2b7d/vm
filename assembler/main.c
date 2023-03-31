#include <stdio.h>
#include <stdint.h>

#include "scanner.h"
#include "compiler.h"
#include "../util.h" // TODO: make path absolute

#include "mem.h" // lib
                 //
int main(int argc, char **argv)
{
    struct scanner s;
    struct parser p;

    argc--;
    argv++;

    if (argc < 1) {
        fprintf(stderr, "assembly file is required\n");
        return 1;
    }

    scanner_init(&s, read_file(*argv)); // LEAK: os is freeing
    meminit(&p.toks, sizeof(struct token), 32); // LEAK: os is freeing
    meminit(&p.sa, sizeof(struct sym), 16); // LEAK: os is freeing
    meminit(&p.ra, sizeof(struct rel), 16); // LEAK: os is freeing
    meminit(&p.code, sizeof(uint8_t), 16); // LEAK: os is freeing

    for (;;) {
        struct token *t;

        memgrow(&p.toks, sizeof(struct token));
        t = p.toks.buf + p.toks.size;
        p.toks.size++;

        scan_token(&s, t);
        if (t->kind == TOK_EOF) {
            break;
        }
    }

    p.cur = p.toks.buf;

    compile(&p);

    return 0;
}
