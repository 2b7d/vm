#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "mem.h" // lib

struct header {
    unsigned short nsyms;
    unsigned short nrels;
    unsigned short ndata;
};

struct symbol {
    unsigned short number;
    unsigned short value;
    unsigned char *name;
    enum { TYPE_LOCAL, TYPE_GLOBAL, TYPE_EXTERN } type;
};

struct symbol_array {
    unsigned long size;
    unsigned long cap;
    struct symbol *buf;
};

struct relocation {
    unsigned short loc;
    unsigned short ref;
};

struct relocation_array {
    unsigned long size;
    unsigned long cap;
    struct relocation *buf;
};

struct module {
    unsigned char *data;

    unsigned char *src;
    unsigned char *cur;

    unsigned long start;
    unsigned long index;

    struct header h;

    struct symbol_array sa;
    struct relocation_array ra;
};

struct module_array {
    unsigned long size;
    unsigned long cap;
    struct module *buf;
};

struct gsymbol {
    unsigned char *name;
    unsigned long module;
    unsigned long symnum;
};

struct gsymbol_array {
    unsigned long size;
    unsigned long cap;
    struct gsymbol *buf;
};

static struct gsymbol_array gsa;
static struct module_array ma;

static void gsymbol_add(unsigned char *name, unsigned long mod, unsigned long num)
{
    struct gsymbol *gs;

    for (unsigned long i = 0; i < gsa.size; ++i) {
        if (strcmp((char *) gsa.buf[i].name, (char *) name) == 0) {
            fprintf(stderr, "global symbol %s already defined\n", name);
            exit(1);
        }
    }

    memgrow(&gsa, sizeof(struct gsymbol));
    gs = gsa.buf + gsa.size;
    gsa.size++;

    gs->name = name;
    gs->module = mod;
    gs->symnum = num;
}

static struct gsymbol *gsymbol_get(unsigned char *name)
{
    for (unsigned long i = 0; i < gsa.size; ++i) {
        if (strcmp((char *) gsa.buf[i].name, (char *) name) == 0) {
            return gsa.buf + i;
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

static void read_bytes(struct module *m, void *dest, unsigned long size)
{
    memcpy(dest, m->cur, size);
    m->cur += size;
}

static void read_header(struct module *m)
{
    read_bytes(m, &m->h.nsyms, 2);
    read_bytes(m, &m->h.nrels, 2);
    read_bytes(m, &m->h.ndata, 2);

    read_separator(m);
}

static void read_symbols(struct module *m)
{
    if (m->h.nsyms == 0) {
        return;
    }

    for (unsigned short i = 0; i < m->h.nsyms; ++i) {
        struct symbol *s;

        memgrow(&m->sa, sizeof(struct symbol));
        s = m->sa.buf + m->sa.size;
        m->sa.size++;

        s->number = i;
        s->name = m->cur;
        m->cur += strlen((char *) m->cur) + 1;

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

    for (unsigned short i = 0; i < m->h.nrels; ++i) {
        struct relocation *r;

        memgrow(&m->ra, sizeof(struct relocation));
        r = m->ra.buf + m->ra.size;
        m->ra.size++;

        read_bytes(m, &r->loc, 2);
        read_bytes(m, &r->ref, 2);
    }

    read_separator(m);
}

static void read_data(struct module *m)
{
    if (m->h.ndata == 0) {
        m->data = NULL;
        return;
    }

    m->data = malloc(m->h.ndata);
    if (m->data == NULL) {
        perror("malloc failed");
        exit(1);
    }

    read_bytes(m, m->data, m->h.ndata);
}

static void relocate_symbols(struct module *m)
{
    for (unsigned long i = 0; i < m->ra.size; ++i) {
        struct relocation *r = m->ra.buf + i;
        struct symbol *s;
        struct gsymbol *gs;
        unsigned short value;

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

            sm = ma.buf + gs->module;
            s = sm->sa.buf + gs->symnum;
            value = s->value + sm->start;
        } else {
            value = s->value + m->start;
        }

        memcpy(m->data + r->loc, &value, 2);
    }
}

int main(int argc, char **argv)
{
    unsigned long start, module_index;
    FILE *out;
    struct gsymbol *gs;
    struct symbol *s;
    struct module *m;
    unsigned short value;

    argc--;
    argv++;

    if (argc < 1) {
        fprintf(stderr, "expected object file[s]\n");
        return 1;
    }

    meminit(&ma, sizeof(struct module), 0); // LEAK: os is freeing
    meminit(&gsa, sizeof(struct gsymbol), 16); // LEAK: os is freeing

    start = 0;
    module_index = 0;
    while (*argv != NULL) {
        struct module *m;

        memgrow(&ma, sizeof(struct module));
        m = ma.buf + ma.size;
        ma.size++;

        module_init(m, *argv); // LEAK: os is freeing
        meminit(&m->sa, sizeof(struct symbol), 16); // LEAK: os is freeing
        meminit(&m->ra, sizeof(struct symbol), 16); // LEAK: os is freeing

        m->start = start;
        m->index = module_index;

        read_header(m);
        read_symbols(m);
        read_relocations(m);
        read_data(m);

        start += m->h.ndata;
        module_index++;
        argv++;
    }

    for (unsigned long i = 0; i < ma.size; ++i) {
        relocate_symbols(ma.buf + i);
    }

    gs = gsymbol_get((unsigned char *) "_start");
    if (gs == NULL) {
        fprintf(stderr, "_start entry point is not defined\n");
        exit(1);
    }
    m = ma.buf + gs->module;
    s = m->sa.buf + gs->symnum;

    out = fopen("a.out", "w");
    if (out == NULL) {
        perror("fopen failed");
        exit(1);
    }

    value = s->value + m->start;
    fwrite(&value, sizeof(value), 1, out);

    for (unsigned long i = 0; i < ma.size; ++i) {
        m = ma.buf + i;
        fwrite(m->data, 1, m->h.ndata, out);
    }

    fclose(out);

    return 0;
}
