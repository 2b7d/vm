#include <stdio.h>

#include "scanner.h"
#include "../vm.h"
#include "parser.h"

int main(int argc, char **argv)
{
    struct parser p;
    struct inst inst;
    FILE *out;

    argc--;
    argv++;

    if (argc < 1) {
        fprintf(stderr, "Provide file to assemble\n");
        return 1;
    }

    out = fopen("out.vm", "w");
    if (out == NULL) {
        perror(NULL);
        return 1;
    }

    make_parser(&p, *argv);

    while (parse_instruction(&p, &inst) == 1) {
        fwrite(&inst.opcode, 1, 1, out);
        if (inst.opcode == OP_PUSH || inst.opcode == OP_PUSHB) {
            fwrite(&inst.operand, inst.operand_size, 1, out);
        }
    }

    fclose(out);

    return 0;
}
