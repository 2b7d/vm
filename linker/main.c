#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

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
    char *cur;

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

static void module_init(struct module *m, char *pathname)
{
    int fd;
    struct stat statbuf;

    fd = open(pathname, O_RDONLY);
    if (fd < 0) {
        perror("failed to open file");
        exit(1);
    }

    if (fstat(fd, &statbuf) < 0) {
        perror("failed to get file info");
        exit(1);
    }

    m->src = malloc(statbuf.st_size);
    if (m->src == NULL) {
        perror("malloc failed");
        exit(1);
    }

    if (read(fd, m->src, statbuf.st_size) < 0) {
        perror("read failed");
        exit(1);
    }

    close(fd);

    m->cur = m->src;
}

static void read_separator(struct module *m)
{
    if (*m->cur++ != '\n') {
        fprintf(stderr, "expected separator\n");
        exit(1);
    }
}

static void read_bytes(struct module *m, void *dest, int size)
{
    memcpy(dest, m->cur, size);
    m->cur += size;
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
        s->name = m->cur;
        m->cur += strlen(m->cur) + 1;

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

static void read_data(struct module *m)
{
    if (m->h.ncode == 0) {
        m->code = NULL;
        return;
    }

    m->code = malloc(m->h.ncode);
    if (m->code == NULL) {
        perror("malloc failed");
        exit(1);
    }

    read_bytes(m, m->code, m->h.ncode);
}

static void relocate_symbols(struct module *m)
{
    for (int i = 0; i < m->ra.size; ++i) {
        struct rel *r = m->ra.buf + i;
        struct sym *s;
        struct gsym *gs;
        int value;

        if (r->ref >= m->sa.size) {
            fprintf(stderr, "invalid relocation reference\n");
            exit(1);
        }

        s = m->sa.buf + r->ref;
        if (s->type == TYPE_EXTERN) {
            struct module *sm;

            gs = gsymbol_get(s->name);
            if (gs == NULL) {
                fprintf(stderr, "symbol %s is not defined\n", s->name);
                exit(1);
            }

            sm = modules.buf + gs->module;
            s = sm->sa.buf + gs->symnum;
            value = s->value + sm->start;
        } else {
            value = s->value + m->start;
        }

        memcpy(m->code + r->loc, &value, 2);
    }
}

int main(int argc, char **argv)
{
    FILE *out;
    struct gsym *gs;
    struct sym *s;
    struct module *m;
    int start, value;

    --argc;
    ++argv;

    if (argc < 1) {
        fprintf(stderr, "expected object file[s]\n");
        return 1;
    }

    meminit((struct mem *) &modules, sizeof(struct module), 0); // LEAK: os is freeing
    meminit((struct mem *) &gsymbols, sizeof(struct gsym), 16); // LEAK: os is freeing

    start = 0;
    for (int i = 0; *argv != NULL; ++i) {
        struct module *m = memnext((struct mem *) &modules);

        module_init(m, *argv); // LEAK: os is freeing
        meminit((struct mem *) &m->sa, sizeof(struct sym), 16); // LEAK: os is freeing
        meminit((struct mem *) &m->ra, sizeof(struct sym), 16); // LEAK: os is freeing

        m->start = start;
        m->index = i;

        read_header(m);
        read_symbols(m);
        read_relocations(m);
        read_data(m);

        start += m->h.ncode;
        ++argv;
    }

    for (int i = 0; i < modules.size; ++i) {
        relocate_symbols(modules.buf + i);
    }

    gs = gsymbol_get("_start");
    if (gs == NULL) {
        fprintf(stderr, "_start entry point is not defined\n");
        exit(1);
    }
    m = modules.buf + gs->module;
    s = m->sa.buf + gs->symnum;

    out = fopen("a.out", "w");
    if (out == NULL) {
        perror("fopen failed");
        exit(1);
    }

    value = s->value + m->start;
    fwrite(&value, sizeof(value), 1, out);

    for (int i = 0; i < modules.size; ++i) {
        m = modules.buf + i;
        fwrite(m->code, 1, m->h.ncode, out);
    }

    fclose(out);

    return 0;
}
