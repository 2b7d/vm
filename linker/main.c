/*
 * Object file
 *
 * Header
 *   nsyms - 2 bytes
 *   nrels - 2 bytes
 *
 * Data
 *   len  - 2 bytes
 *   code - len bytes
 *
 * Text
 *   len  - 2 bytes
 *   code - len bytes
 *
 * Symbols
 *   kind   - 1 byte
 *   sec    - 1 byte
 *   idx    - 2 bytes
 *   addr   - 2 bytes
 *   nlabel - 2 bytes
 *   label  - nlabel bytes
 *
 * Relocations
 *   loc    - 2 bytes
 *   symidx - 2 bytes
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../lib/sstring.h"
#include "../lib/mem.h"

#include "../vm.h"

#include "ln.h"

struct gsymbol {
    int modidx;
    int symidx;
};

struct module {
    int idx;

    struct ln_header header;

    struct {
        int len;
        int cap;
        int data_size;
        struct ln_symbol *buf;
    } symtab;

    struct {
        int len;
        int cap;
        int data_size;
        struct ln_relocation *buf;
    } rels;

    string data;
    string text;
};

struct {
    int len;
    int cap;
    int data_size;
    struct gsymbol *buf;
} gsymtab;

struct {
    int len;
    int cap;
    int data_size;
    struct module *buf;
} modules;

int _start_addr = -1;

int valid_idx(int idx, int max)
{
    return idx >= 0 && idx < max;
}

struct ln_symbol *gsymtab_get_symbol(struct gsymbol *gsym)
{
    struct module *mod;
    struct ln_symbol *sym;

    if (valid_idx(gsym->modidx, modules.len) == 0) {
        fprintf(stderr, "module index %d is out of bound\n", gsym->modidx);
        exit(1);
    }

    mod = modules.buf + gsym->modidx;
    if (valid_idx(gsym->symidx, mod->symtab.len) == 0) {
        fprintf(stderr, "symbol index %d is out of bound\n", gsym->symidx);
        exit(1);
    }

    sym = mod->symtab.buf + gsym->symidx;
    return sym;
}

struct ln_symbol *gsymtab_lookup(string *label)
{
    for (int i = 0; i < gsymtab.len; ++i) {
        struct gsymbol *gs;
        struct ln_symbol *s;

        gs = gsymtab.buf + i;
        s = gsymtab_get_symbol(gs);

        if (string_cmp(&s->label, label) == 1) {
            return s;
        }
    }

    return NULL;
}

void read_header(struct module *m, FILE *stream)
{
    fread(&m->header.nsyms, 2, 1, stream);
    fread(&m->header.nrels, 2, 1, stream);
}

void read_sections(struct module *m, FILE *stream)
{
    int len;

    m->data.len = 0;
    m->text.len = 0;

    fread(&len, 2, 1, stream);
    if (len > 0) {
        string_init(&m->data, len);
        fread(m->data.ptr, 1, len, stream);
    }

    fread(&len, 2, 1, stream);
    if (len > 0) {
        string_init(&m->text, len);
        fread(m->text.ptr, 1, len, stream);
    }
}

void read_symbols(struct module *m, int doff, int toff, FILE *stream)
{
    if (m->header.nsyms == 0) {
        return;
    }

    meminit(&m->symtab, sizeof(struct ln_symbol), 256);

    for (int i = 0; i < m->header.nsyms; ++i) {
        struct ln_symbol *sym;
        int nlabel;

        nlabel = 0;

        sym = memnext(&m->symtab);

        fread(&sym->kind, 1, 1, stream);
        fread(&sym->sec, 1, 1, stream);
        fread(&sym->idx, 2, 1, stream);
        fread(&sym->addr, 2, 1, stream);

        fread(&nlabel, 2, 1, stream);
        string_init(&sym->label, nlabel);
        fread(sym->label.ptr, 1, nlabel, stream);

        switch (sym->sec) {
        case SECTION_DATA:
            sym->addr += doff;
            break;
        case SECTION_TEXT:
            sym->addr += toff;
            break;
        default:
            fprintf(stderr, "unknown section kind %d\n", sym->sec);
            exit(1);
        }

        if (sym->kind == SYM_GLOBAL) {
            struct gsymbol *gsym;

            if (gsymtab_lookup(&sym->label) != NULL) {
                fprintf(stderr, "multiple definition of symbol %.*s\n", sym->label.len, sym->label.ptr);
                exit(1);
            }

            gsym = memnext(&gsymtab);
            gsym->modidx = m->idx;
            gsym->symidx = sym->idx;
        }
    }
}

void read_relocations(struct module *m, FILE *stream)
{
    if (m->header.nrels == 0) {
        return;
    }

    meminit(&m->rels, sizeof(struct ln_relocation), 128);

    for (int i = 0; i < m->header.nrels; ++i) {
        struct ln_relocation *rel;

        rel = memnext(&m->rels);
        fread(&rel->loc, 2, 1, stream);
        fread(&rel->symidx, 2, 1, stream);
    }
}

void relocate_symbols()
{
    for (int i = 0; i < modules.len; ++i) {
        struct module *mod;

        mod = modules.buf + i;

        for (int j = 0; j < mod->rels.len; ++j) {
            struct ln_relocation *rel;
            struct ln_symbol *sym;
            int addr;

            rel = mod->rels.buf + j;
            if (valid_idx(rel->symidx, mod->symtab.len) == 0) {
                fprintf(stderr, "invalid relocation symbol idx %d\n", rel->symidx);
                exit(1);
            }

            sym = mod->symtab.buf + rel->symidx;
            addr = sym->addr;
            if (sym->kind == SYM_EXTERN) {
                struct ln_symbol *global;

                global = gsymtab_lookup(&sym->label);
                if (global == NULL) {
                    fprintf(stderr, "symbol %.*s is not defined\n", sym->label.len, sym->label.ptr);
                    exit(1);
                }

                addr = global->addr;
            }

            if (valid_idx(rel->loc, mod->text.len) == 0) {
                fprintf(stderr, "invalid relocation position %d\n", rel->loc);
                exit(1);
            }
            memcpy(mod->text.ptr + rel->loc, &addr, 2);
        }
    }
}

int main(int argc, char **argv)
{
    struct {
        int len;
        int cap;
        int data_size;
        char *buf;
    } output_data, output_text;

    int data_offset, text_offset;
    FILE *out;

    argc--;
    argv++;

    if (argc < 1) {
        fprintf(stderr, "Provide object files to link\n");
        return 1;
    }

    meminit(&output_data, 1, 4096);
    meminit(&output_text, 1, 4096);
    meminit(&modules, sizeof(struct module), 16);
    meminit(&gsymtab, sizeof(struct gsymbol), 256);
    data_offset = 6; // TODO(art): magic number
    text_offset = 0;

    for (int i = 0; i < argc; ++i) {
        struct module *mod;
        FILE *in;

        in = fopen(argv[i], "r");
        if (in == NULL) {
            perror(NULL);
            return 1;
        }

        mod = memnext(&modules);
        mod->idx = i;

        read_header(mod, in);
        read_sections(mod, in);
        read_symbols(mod, data_offset, text_offset, in);
        read_relocations(mod, in);

        if (ferror(in) == 1) {
            fprintf(stderr, "failed to read object file %s\n", argv[i]);
            perror(NULL);
            exit(1);
        }

        data_offset += mod->data.len;
        text_offset += mod->text.len;

        fclose(in);
    }

    relocate_symbols();

    out = fopen("out.vm", "w");
    if (out == NULL) {
        perror(NULL);
        return 1;
    }

    for (int i = 0; i < modules.len; ++i) {
        struct module *mod;
        int old;

        mod = modules.buf + i;

        old = output_data.len;
        output_data.len += mod->data.len;
        memgrow(&output_data);
        memcpy(output_data.buf + old, mod->data.ptr, mod->data.len);

        old = output_text.len;
        output_text.len += mod->text.len;
        memgrow(&output_text);
        memcpy(output_text.buf + old, mod->text.ptr, mod->text.len);
    }

    for (int i = 0; i < gsymtab.len; ++i) {
        struct gsymbol *gsym;
        struct ln_symbol *sym;

        gsym = gsymtab.buf + i;
        sym = gsymtab_get_symbol(gsym);
        if (string_cmpc(&sym->label, "_start") == 1) {
            _start_addr = sym->addr;
            break;
        }
    }

    if (_start_addr == -1) {
        fprintf(stderr, "_start entry point is not defined\n");
        return 1;
    }

    fwrite(&_start_addr, 2, 1, out);

    fwrite(&output_data.len, 2, 1, out);
    fwrite(output_data.buf, 1, output_data.len, out);

    fwrite(&output_text.len, 2, 1, out);
    fwrite(output_text.buf, 1, output_text.len, out);

    fclose(out);
    return 0;
}
