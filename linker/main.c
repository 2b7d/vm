#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../util.h"

#include "mem.h" // lib

struct header {
    int nsyms;
    int nrels;
    int ncode;
};

struct sym {
    enum { TYPE_LOCAL, TYPE_GLOBAL, TYPE_EXTERN } type;
    char *name;
    int value;
    int number;
};

struct sym_array {
    int size;
    int cap;
    int data_size;
    struct sym *buf;
};

struct rel {
    int loc;
    int ref;
};

struct rel_array {
    int size;
    int cap;
    int data_size;
    struct rel *buf;
};

struct module {
    char *code;

    char *src;

    int start;
    int index;

    struct header h;

    struct sym_array sa;
    struct rel_array ra;
};

struct module_array {
    int size;
    int cap;
    int data_size;
    struct module *buf;
};

struct gsym {
    char *name;
    int module;
    int symnum;
};

struct gsym_array {
    int size;
    int cap;
    int data_size;
    struct gsym *buf;
};

static struct gsym_array gsymbols;
static struct module_array modules;

static void gsymbol_add(char *name, int mod, int num)
{
    struct gsym *gs;

    for (int i = 0; i < gsymbols.size; ++i) {
        if (strcmp(gsymbols.buf[i].name, name) == 0) {
            fprintf(stderr, "global symbol %s already defined\n", name);
            exit(1);
        }
    }

    gs = memnext((struct mem *) &gsymbols);

    gs->name = name;
    gs->module = mod;
    gs->symnum = num;
}

static struct gsym *gsymbol_get(char *name)
{
    for (int i = 0; i < gsymbols.size; ++i) {
        if (strcmp(gsymbols.buf[i].name, name) == 0) {
            return gsymbols.buf + i;
        }
    }

    return NULL;
}

static int gsymbol_get_addr(char *name)
{
    struct module *m;
    struct gsym *gs = gsymbol_get(name);
    struct sym *s;

    if (gs == NULL) {
        return -1;
    }

    m = modules.buf + gs->module;
    s = m->sa.buf + gs->symnum;

    return s->value + m->start;
}

static void read_separator(struct module *m)
{
    if (*m->src != '\n') {
        fprintf(stderr, "expected separator but got %c\n", *m->src);
        exit(1);
    }

    ++m->src;
}

static void read_bytes(struct module *m, void *dest, int size)
{
    memcpy(dest, m->src, size);
    m->src += size;
}

static void read_header(struct module *m)
{
    read_bytes(m, &m->h.nsyms, 2);
    read_bytes(m, &m->h.nrels, 2);
    read_bytes(m, &m->h.ncode, 2);

    read_separator(m);
}

static void read_symbols(struct module *m)
{
    if (m->h.nsyms == 0) {
        return;
    }

    for (int i = 0; i < m->h.nsyms; ++i) {
        struct sym *s = memnext((struct mem *) &m->sa);

        s->number = i;
        s->name = m->src;
        m->src += strlen(m->src) + 1;

        read_bytes(m, &s->value, 2);
        read_bytes(m, &s->type, 1);

        if (s->type == TYPE_GLOBAL) {
            gsymbol_add(s->name, m->index, s->number);
        }
    }

    read_separator(m);
}

static void read_relocations(struct module *m)
{
    if (m->h.nrels == 0) {
        return;
    }

    for (int i = 0; i < m->h.nrels; ++i) {
        struct rel *r = memnext((struct mem *) &m->ra);

        read_bytes(m, &r->loc, 2);
        read_bytes(m, &r->ref, 2);
    }

    read_separator(m);
}

static void relocate_symbols(struct module *m)
{
    for (int i = 0; i < m->ra.size; ++i) {
        struct rel *r = m->ra.buf + i;
        struct sym *s;
        int addr;

        assert(r->ref >= 0);

        if (r->ref >= m->sa.size) {
            fprintf(stderr, "invalid relocation reference {loc: %d ref: %d}\n",
                            r->loc, r->ref);
            exit(1);
        }

        s = m->sa.buf + r->ref;
        addr = s->value + m->start;

        if (s->type == TYPE_EXTERN) {
            addr = gsymbol_get_addr(s->name);
            if (addr < 0) {
                fprintf(stderr, "symbol %s is not defined\n", s->name);
                exit(1);
            }
        }

        memcpy(m->code + r->loc, &addr, 2);
    }
}

static void write_vm_executable()
{
    FILE *out;
    int entry_addr = gsymbol_get_addr("_start");

    if (entry_addr < 0) {
        fprintf(stderr, "_start entry point is not defined\n");
        exit(1);
    }

    out = fopen("a.out", "w");
    if (out == NULL) {
        perror("fopen failed");
        exit(1);
    }

    fwrite(&entry_addr, 2, 1, out);

    for (int i = 0; i < modules.size; ++i) {
        struct module *m = modules.buf + i;

        fwrite(m->code, 1, m->h.ncode, out);
    }

    fclose(out);
}

int main(int argc, char **argv)
{
    int start;

    --argc;
    ++argv;

    if (argc < 1) {
        fprintf(stderr, "expected object file[s]\n");
        return 1;
    }

    meminit((struct mem *) &modules, sizeof(struct module), 0);
    meminit((struct mem *) &gsymbols, sizeof(struct gsym), 16);

    start = 0;
    for (int i = 0; *argv != NULL; ++i) {
        struct module *m = memnext((struct mem *) &modules);

        m->src = read_file(*argv);
        m->start = start;
        m->index = i;

        meminit((struct mem *) &m->sa, sizeof(struct sym), 16);
        meminit((struct mem *) &m->ra, sizeof(struct rel), 16);

        read_header(m);

        if (m->h.ncode == 0) {
            fprintf(stdout, "%s is empty; skipping\n", *argv);
            --i;
            --modules.size;
            free(m->src);
            memfree((struct mem *) &m->sa);
            memfree((struct mem *) &m->ra);
            continue;
        }

        read_symbols(m);
        read_relocations(m);
        m->code = m->src;

        start += m->h.ncode;
        ++argv;
    }

    for (int i = 0; i < modules.size; ++i) {
        relocate_symbols(modules.buf + i);
    }

    write_vm_executable();

    return 0;
}
