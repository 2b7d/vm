#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "../lib/mem.h"
#include "../lib/path.h"
#include "../lib/sstring.h"

#include "../vm.h"
#include "scanner.h"
#include "parser.h"

struct section {
    enum vm_section kind;
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
    struct symtab st;
    struct relocations rels;
    struct parsed_values values;
    struct section data, text;
    char *outfile;
    FILE *out;

    argc--;
    argv++;

    if (argc < 1) {
        fprintf(stderr, "Provide file to assemble\n");
        return 1;
    }

    outfile = create_outfile(*argv, "o");
    if (outfile == NULL) {
        perror(NULL);
        return 1;
    }

    out = fopen(outfile, "w");
    if (out == NULL) {
        perror(NULL);
        return 1;
    }

    make_parser(&p, *argv);
    meminit(&st, sizeof(struct symbol), 128);
    meminit(&rels, sizeof(struct relocation), 128);
    meminit(&values, sizeof(struct parsed_value), 256);

    parse(&p, &st, &rels, &values);

    data.kind = SECTION_DATA;
    text.kind = SECTION_TEXT;
    meminit(&data.code, 1, 256);
    meminit(&text.code, 1, 256);

    for (int i = 0; i < values.len; ++i) {
        struct parsed_value *pv;
        struct data_label *dl;
        struct text_label *tl;
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
        case PARSVAL_TEXT_LABEL:
            tl = pv->value;
            for (int i = 0; i < tl->len; ++i) {
                struct mnemonic *m;
                struct symbol *sym;

                m = tl->buf + i;

                memgrow(&text.code);
                text.code.buf[text.code.len++] = m->opcode;

                if (with_operand(m->opcode) == 0) {
                    continue;
                }

                old_len = text.code.len;
                text.code.len += m->operand.size;
                memgrow(&text.code);

                switch (m->operand.kind) {
                case OPERAND_NUM:
                    memcpy(text.code.buf + old_len, &m->operand.as.num,
                           m->operand.size);
                    break;
                case OPERAND_SYM:
                    sym = m->operand.as.sym;
                    if (sym->kind != SYM_EXTERN && sym->is_resolved == 0) {
                        // TODO(art): add file position
                        fprintf(stderr, "undefined symbol %.*s\n", sym->label.len, sym->label.ptr);
                        return 1;
                    }
                    memcpy(text.code.buf + old_len, &sym->addr,
                           m->operand.size);
                    break;
                default:
                    assert(0 && "unreachable");
                }
            }
            break;
        default:
            assert(0 && "unreachable");
        }
    }

    // write header
    fwrite(&st.len, 2, 1, out);
    fwrite(&rels.len, 2, 1, out);

    // write data
    fwrite(&data.code.len, 2, 1, out);
    if (data.code.len > 0) {
        fwrite(data.code.buf, 1, data.code.len, out);
    }

    // write text
    fwrite(&text.code.len, 2, 1, out);
    if (text.code.len > 0) {
        fwrite(text.code.buf, 1, text.code.len, out);
    }

    // write symbols
    for (int i = 0; i < st.len; ++i) {
        struct symbol *s;

        s = st.buf + i;
        fwrite(&s->kind, 1, 1, out);
        fwrite(&s->sec, 1, 1, out);
        fwrite(&i, 2, 1, out);
        fwrite(&s->addr, 2, 1, out);
        fwrite(&s->label.len, 2, 1, out);
        fwrite(s->label.ptr, 1, s->label.len, out);
    }

    // write relocations
    for (int i = 0; i < rels.len; ++i) {
        struct relocation *rel;

        rel = rels.buf + i;

        fwrite(&rel->loc, 2, 1, out);
        fwrite(&rel->symidx, 2, 1, out);
    }

    fclose(out);
    return 0;
}
