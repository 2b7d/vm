/*
 * Object file
 *
 * Header
 *   nsecs - 1 byte
 *   nsyms - 2 bytes
 *   ndrel - 2 bytes
 *   ntrel - 2 bytes
 *
 * Symbols
 *   kind   - 1 byte
 *   idx    - 2 bytes
 *   addr   - 2 bytes
 *   nlabel - 2 bytes
 *   label  - nlabel bytes
 *
 * Data Relocations
 *   loc    - 2 bytes
 *   symidx - 2 bytes
 *
 * Text Relocations
 *   loc    - 2 bytes
 *   symidx - 2 bytes
 *
 * Sections
 *   kind - 1 byte
 *   len  - 2 bytes
 *   code - len bytes
 */

#include <stdio.h>
#include <stdlib.h>

#include "../lib/sstring.h"
#include "../lib/mem.h"

#include "../vm.h"

struct header {
    int nsecs;
    int nsyms;
    int ndrel;
    int ntrel;
};

struct symbol {
    enum {
        SYM_LOCAL = 0,
        SYM_GLOBAL,
        SYM_EXTERN
    } kind;
    int idx;
    int addr;
    string label;
};

struct relocation {
    int loc;
    int symidx;
};

struct module {
    struct header header;

    struct {
        int len;
        int cap;
        int data_size;
        struct symbol *buf;
    } symtab;

    struct {
        int len;
        int cap;
        int data_size;
        struct relocation *buf;
    } data_rel;

    struct {
        int len;
        int cap;
        int data_size;
        struct relocation *buf;
    } text_rel;

    string data;
    string text;
};

void read_header(struct module *m, FILE *stream)
{
    struct header h;

    h = m->header;

    fread(&h.nsecs, 1, 1, stream);
    fread(&h.nsyms, 2, 1, stream);
    fread(&h.ndrel, 2, 1, stream);
    fread(&h.ntrel, 2, 1, stream);
}

void read_symbols(struct module *m, FILE *stream)
{
    meminit(&m->symtab, sizeof(struct symbol), 256);

    for (int i = 0; i < m->header.nsyms; ++i) {
        struct symbol *sym;
        int nlabel;

        sym = memnext(&m->symtab);
        fread(&sym->kind, 1, 1, stream);
        fread(&sym->idx, 2, 1, stream);
        fread(&sym->addr, 2, 1, stream);
        fread(&nlabel, 2, 1, stream);

        string_init(&sym->label, nlabel);

        fread(&sym->label.ptr, 1, nlabel, stream);
    }
}

void read_data_relocations(struct module *m, FILE *stream)
{
    meminit(&m->data_rel, sizeof(struct relocation), 128);

    for (int i = 0; i < m->header.ndrel; ++i) {
        struct relocation *rel;

        rel = memnext(&m->data_rel);
        fread(&rel->loc, 2, 1, stream);
        fread(&rel->symidx, 2, 1, stream);
    }
}

void read_text_relocations(struct module *m, FILE *stream)
{
    meminit(&m->text_rel, sizeof(struct relocation), 128);

    for (int i = 0; i < m->header.ntrel; ++i) {
        struct relocation *rel;

        rel = memnext(&m->text_rel);
        fread(&rel->loc, 2, 1, stream);
        fread(&rel->symidx, 2, 1, stream);
    }
}

void read_sections(struct module *m, FILE *stream)
{
    for (int i = 0; i < m->header.nsecs; ++i) {
        enum vm_section kind;
        int len;

        fread(&kind, 1, 1, stream);
        fread(&len, 2, 1, stream);

        switch (kind) {
        case SECTION_DATA:
            string_init(&m->data, len);
            fread(&m->data.ptr, 1, len, stream);
            break;
        case SECTION_TEXT:
            string_init(&m->text, len);
            fread(&m->text.ptr, 1, len, stream);
            break;
        default:
            fprintf(stderr, "unknown section kind %d\n", kind);
            exit(1);
        }
    }
}

int main(int argc, char **argv)
{
    struct {
        int len;
        int cap;
        int data_size;
        struct moduel *buf;
    } modules;

    FILE *out;

    argc--;
    argv++;

    if (argc < 1) {
        fprintf(stderr, "Provide object files to link\n");
        return 1;
    }

    meminit(&modules, sizeof(struct module), 16);

    while (*argv != NULL) {
        struct module *mod;
        FILE *in;

        in = fopen(*argv, "r");
        if (in == NULL) {
            perror(NULL);
            return 1;
        }

        mod = memnext(&modules);

        read_header(mod, in);

        if (mod->header.nsyms > 0) {
            read_symbols(mod, in);
        }

        if (mod->header.ndrel > 0) {
            read_data_relocations(mod, in);
        }

        if (mod->header.ntrel > 0) {
            read_text_relocations(mod, in);
        }

        if (mod->header.nsecs > 0) {
            read_sections(mod, in);
        }

        fclose(in);
        argv++;
    }

    out = fopen("out.vm", "w");
    if (out == NULL) {
        perror(NULL);
        return 1;
    }

    return 0;
}
