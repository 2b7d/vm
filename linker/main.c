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
    enum { TYPE_UNDEF, TYPE_DEF } type;
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

struct symbol_array {
    unsigned long size;
    unsigned long cap;
    struct symbol *buf;
};

struct module {
    unsigned char *name;

    unsigned char *data;

    unsigned char *src;
    unsigned char *cur;
    unsigned long size;

    struct header h;

    struct symbol_array sa;
    struct relocation_array ra;
};

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

    m->size = statbuf.st_size;
    m->src = malloc(m->size);
    if (m->src == NULL) {
        perror("malloc failed");
        exit(1);
    }

    if (read(fd, m->src, m->size) < 0) {
        perror("read failed");
        exit(1);
    }

    close(fd);

    m->name = (unsigned char *) pathname;
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
    for (unsigned short i = 0; i < m->h.nsyms; ++i) {
        struct symbol *s;

        memgrow(&m->sa, sizeof(struct symbol));
        s = m->sa.buf + m->sa.size;
        m->sa.size++;

        s->number = i + 1;
        s->name = m->cur;
        m->cur += strlen((char *) m->cur) + 1;

        read_bytes(m, &s->value, 2);

        switch (*m->cur++) {
        case 'D':
            s->type = TYPE_DEF;
            break;

        case 'U':
            s->type = TYPE_UNDEF;
            break;

        default:
            fprintf(stderr, "unknown symbol type\n");
            exit(1);
        }
    }

    read_separator(m);
}

static void read_relocations(struct module *m)
{
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

int main(int argc, char **argv)
{
    struct module m;

    argc--;
    argv++;

    if (argc < 1) {
        fprintf(stderr, "expected object file[s]\n");
        return 1;
    }

    module_init(&m, *argv); // LEAK: os is freeing
    meminit(&m.sa, sizeof(struct symbol), 16); // LEAK: os is freeing
    meminit(&m.ra, sizeof(struct symbol), 16); // LEAK: os is freeing

    read_header(&m);
    read_symbols(&m);
    read_relocations(&m);
    read_data(&m);

    return 0;
}
