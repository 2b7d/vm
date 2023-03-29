#include <stdio.h>
#include <stdint.h>

#include "scanner.h"
#include "compiler.h"
#include "../util.h" // TODO: make path absolute

#include "mem.h" // lib

int main(int argc, char **argv)
{
    struct scanner s;
    struct parser p;
    struct chunk_array ca;

    argc--;
    argv++;

    if (argc < 1) {
        fprintf(stderr, "assembly file is required\n");
        return 1;
    }

    scanner_init(&s, read_file(*argv)); // LEAK: os is freeing
    parser_init(&p, &s); // LEAK: os is freeing
    meminit(&ca, sizeof(struct chunk), 0); // LEAK: os is freeing

    compile(&p, &ca);

    for (size_t i = 0; i < ca.size; ++i) {
        struct chunk *c = ca.buf + i;

        printf("name: %u\n", c->name);
        printf("    size: %lu\n", c->data.size);
        printf("    data: {");
        for (size_t j = 0; j < c->data.size; ++j) {
            printf("%x", c->data.buf[j]);
            if (j < c->data.size - 1) {
                printf(", ");
            }
        }
        printf("}\n");
    }

    return 0;
}
