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
    char *src, *outpath;

    --argc;
    ++argv;

    if (argc < 1) {
        fprintf(stderr, "assembly file is required\n");
        return 1;
    }

    src = read_file(*argv);
    scanner_init(&s, src);

    parser_init(&p);

    scan_tokens(&s, &p.ta);
    compile(&p);

    outpath = create_outpath(*argv, "o");
    write_object_file(&p, outpath);

    return 0;
}
