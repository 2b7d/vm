#include <stdio.h>

#include "scanner.h"
#include "../util.h" // TODO: make path absolute
#include "mem.h" // lib

struct token_array {
    size_t size;
    size_t cap;
    struct token *buf;
};

int main(int argc, char **argv)
{
    struct token_array toks;
    struct scanner s;

    argc--;
    argv++;

    if (argc < 1) {
        puts("assembly file is required\n");
        return 1;
    }

    scanner_init(&s, read_file(*argv)); // LEAK: os is freeing
    meminit(&toks, sizeof(struct token), 32); // LEAK: os is freeing

    for (;;) {
        struct token *t;

        memgrow(&toks, sizeof(struct token));
        t = toks.buf + toks.size;
        toks.size++;

        scan_token(&s, t);
        if (t->kind == TOK_EOF) {
            break;
        }
    }

    for (size_t i = 0; i < toks.size; ++i) {
        struct token *t = toks.buf + i;

        printf("%3u -> %.*s\n", t->kind, (int) t->len, t->start);
    }

    return 0;
}
