#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "../lib/mem.h"

#include "scanner.h"
#include "../vm.h"
#include "parser.h"

struct segment {
    enum vm_segment kind;
    struct {
        int len;
        int cap;
        int data_size;
        char *buf;
    } code;
};

int main(int argc, char **argv)
{
    struct parser p;
    struct symtable st;
    struct parsed_values values;
    struct segment data, text;
    FILE *out;
    int _start_addr, nsecs;

    argc--;
    argv++;

    if (argc < 1) {
        fprintf(stderr, "Provide file to assemble\n");
        return 1;
    }

    // TODO(art): extract name from input file
    out = fopen("out.vm", "w");
    if (out == NULL) {
        perror(NULL);
        return 1;
    }

    make_parser(&p, *argv);
    meminit(&st, sizeof(struct symbol), 128);
    meminit(&values, sizeof(struct parsed_value), 256);

    parse(&p, &st, &values);

    _start_addr = -1;

    for (int i = 0; i < st.len; ++i) {
        struct symbol *s;

        s = st.buf + i;
        if (s->label_len == 6 &&
                memcmp("_start", s->label, s->label_len) == 0) {
            _start_addr = s->addr;
            break;
        }
    }

    // TODO(art): this should be done by linker, not assembler
    if (_start_addr == -1) {
        fprintf(stderr, "_start entry point is not defined\n");
        return 1;
    }

    fwrite(&_start_addr, 2, 1, out);

    data.kind = SEGMENT_DATA;
    text.kind = SEGMENT_TEXT;
    meminit(&data.code, 1, 256);
    meminit(&text.code, 1, 256);

    for (int i = 0; i < values.len; ++i) {
        struct parsed_value *pv;
        struct data_label *dl;
        struct mnemonic *m;
        struct mnemonic_push *mp;
        struct symbol *sym;
        int old_len;

        pv = values.buf + i;

        switch (pv->kind) {
        case PARSVAL_DATA_LABEL:
            dl = pv->value;
            old_len = data.code.len;
            data.code.len += dl->value_size * dl->values.len;
            memgrow(&data.code);
            for (int i = 0; i < dl->values.len; ++i) {
                memcpy(data.code.buf + old_len + i * dl->value_size,
                       dl->values.buf + i, dl->value_size);
            }
            break;
        case PARSVAL_MNEMONIC:
            m = pv->value;
            memgrow(&text.code);
            text.code.buf[text.code.len++] = m->opcode;
            break;
        case PARSVAL_MNEMONIC_PUSH:
            mp = pv->value;
            old_len = text.code.len;
            text.code.len += 1 + mp->operand.size;
            memgrow(&text.code);
            text.code.buf[old_len++] = mp->opcode;

            switch (mp->operand.kind) {
            case PUSH_NUM:
                memcpy(text.code.buf + old_len, &mp->operand.as.num,
                       mp->operand.size);
                break;
            case PUSH_SYM:
                sym = mp->operand.as.sym;
                if (sym->is_resolved == 0) {
                    fprintf(stderr, "undefined symbol %.*s\n", sym->label_len, sym->label);
                    return 1;
                }
                memcpy(text.code.buf + old_len, &sym->addr, mp->operand.size);
                break;
            default:
                assert(0 && "unreachable");
            }
            break;
        default:
            assert(0 && "unreachable");
        }
    }

    nsecs = 0;
    if (data.code.len > 0) {
        nsecs++;
    }
    if (text.code.len > 0) {
        nsecs++;
    }

    fwrite(&nsecs, 2, 1, out);

    if (data.code.len > 0) {
        fwrite(&data.kind, 1, 1, out);
        fwrite(&data.code.len, 2, 1, out);
        fwrite(data.code.buf, 1, data.code.len, out);
    }

    if (text.code.len > 0) {
        fwrite(&text.kind, 1, 1, out);
        fwrite(&text.code.len, 2, 1, out);
        fwrite(text.code.buf, 1, text.code.len, out);
    }

    fclose(out);
    return 0;
}
