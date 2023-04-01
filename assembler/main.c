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
    char *src, *outpath, c;
    FILE *out;

    argc--;
    argv++;

    if (argc < 1) {
        fprintf(stderr, "assembly file is required\n");
        return 1;
    }

    src = read_file(*argv); // LEAK: os is freeing

    scanner_init(&s, src);
    parser_init(&p);

    scan_tokens(&s, &p.ta);
    compile(&p);

    outpath = create_outpath(*argv, "o"); // LEAK os is freeing
    out = fopen(outpath, "w");
    if (out == NULL) {
        perror("fopen failed");
        return 1;
    }

    fwrite(&p.sa.size, 2, 1, out);
    fwrite(&p.ra.size, 2, 1, out);
    fwrite(&p.code.size, 2, 1, out);

    c = '\n';
    fwrite(&c, 1, 1, out);

    for (int i = 0; i < p.sa.size; ++i) {
        struct sym *s = p.sa.buf + i;

        fwrite(s->name, 1, s->namelen, out);
        c = '\0';
        fwrite(&c, 1, 1, out);

        fwrite(&s->value, 2, 1, out);
        fwrite(&s->type, 1, 1, out);
    }

    c = '\n';
    fwrite(&c, 1, 1, out);

    for (int i = 0; i < p.ra.size; ++i) {
        struct rel *r = p.ra.buf + i;
        fwrite(&r->loc, 2, 1, out);
        fwrite(&r->ref, 2, 1, out);
    }

    c = '\n';
    fwrite(&c, 1, 1, out);

    fwrite(p.code.buf, 1, p.code.size, out);
    fclose(out);

    return 0;
}
