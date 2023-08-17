#include <stdio.h>

#include "../lib/mem.h"

#include "scanner.h"
#include "../vm.h"
#include "parser.h"

int main(int argc, char **argv)
{
    struct parser p;
    struct inst_array ia;
    struct symtable st;
    FILE *out;

    argc--;
    argv++;

    if (argc < 1) {
        fprintf(stderr, "Provide file to assemble\n");
        return 1;
    }

    // TODO(art): take name from input file?
    out = fopen("out.vm", "w");
    if (out == NULL) {
        perror(NULL);
        return 1;
    }

    make_parser(&p, *argv);
    meminit(&st, sizeof(struct symbol), 128);
    meminit(&ia, sizeof(struct inst), 256);

    parse_instructions(&p, &st, &ia);
    resolve_labels(&p, &st, &ia);

    for (int i = 0; i < ia.len; ++i) {
        struct inst *inst;

        inst = ia.buf + i;
        fwrite(&inst->opcode, 1, 1, out);
        if (inst->opcode == OP_PUSH || inst->opcode == OP_PUSHB) {
            fwrite(&inst->operand.as_int, inst->operand_size, 1, out);
        }
    }

    fclose(out);

    return 0;
}
