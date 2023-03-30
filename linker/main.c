#include <stdio.h>
#include <stdlib.h>

#include "mem.h"

struct header {
    unsigned short nsyms;
    unsigned short nrels;
    unsigned short ndata;
};

struct symbol {
    unsigned short number;
    unsigned short value;
    unsigned char name[256];
    enum { TYPE_UNDEF, TYPE_DEF } type;
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

static void read_bytes(void *buf, size_t size, FILE *f)
{
    fread(buf, size, 1, f);
    if (ferror(f) != 0) {
        fprintf(stderr, "read failed\n");
        exit(1);
    }
}

static void read_separator(FILE *f)
{
    unsigned char c;

    read_bytes(&c, sizeof(c), f);
    if (c != '\n') {
        fprintf(stderr, "expected separator\n");
        exit(1);
    }
}

static void read_header(struct header *h, FILE *f)
{
    read_bytes(&h->nsyms, sizeof(h->nsyms), f);
    read_bytes(&h->nrels, sizeof(h->nrels), f);
    read_bytes(&h->ndata, sizeof(h->ndata), f);

    read_separator(f);
}

static void read_symbol(struct symbol *s, FILE *f)
{
    unsigned short i = 0;
    unsigned char c;

    for (;;) {
        read_bytes(&c, sizeof(c), f);

        if (i >= sizeof(s->name)) {
            fprintf(stderr, "symbol name is t0o long\n");
            exit(1);
        }

        s->name[i++] = c;
        if (c == '\0') {
            break;
        }
    }

    read_bytes(&s->value, sizeof(s->value), f);

    read_bytes(&c, sizeof(c), f);
    switch (c) {
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

static void read_symbols(struct symbol_array *sa, struct header *h, FILE *f)
{
    for (unsigned short i = 0; i < h->nsyms; ++i) {
        struct symbol *s;

        memgrow(sa, sizeof(struct symbol));
        s = sa->buf + sa->size;
        sa->size++;

        s->number = i + 1;
        read_symbol(s, f);
    }

    read_separator(f);
}

static void read_relocations(struct relocation_array *ra, struct header *h,
                             FILE *f)
{
    for (unsigned short i = 0; i < h->nrels; ++i) {
        struct relocation *r;

        memgrow(ra, sizeof(struct relocation));
        r = ra->buf + ra->size;
        ra->size++;

        read_bytes(&r->loc, sizeof(r->loc), f);
        read_bytes(&r->ref, sizeof(r->ref), f);
    }

    read_separator(f);
}

static unsigned char *read_data(unsigned long size, FILE *f)
{
    unsigned char *data;

    if (size == 0) {
        return NULL;
    }

    data = malloc(size);
    if (data == NULL) {
        perror("malloc failed");
        exit(1);
    }

    read_bytes(data, size, f);
    return data;
}

int main(int argc, char **argv)
{
    FILE *f;
    struct symbol_array sa;
    struct relocation_array ra;
    struct header h;
    unsigned char *data;

    argc--;
    argv++;

    if (argc < 1) {
        fprintf(stderr, "expected object file[s]\n");
        return 1;
    }

    meminit(&sa, sizeof(struct symbol), 16); // LEAK: os is freeing
    meminit(&ra, sizeof(struct relocation), 16); // LEAK: os is freeing

    f = fopen(*argv, "r");
    if (f == NULL) {
        perror("fopen failed");
        return 1;
    }

    read_header(&h, f);
    read_symbols(&sa, &h, f);
    read_relocations(&ra, &h, f);
    data = read_data(h.ndata * sizeof(unsigned char), f);

    printf("nsyms: %d, nrels: %d, ndata: %d\n", h.nsyms, h.nrels, h.ndata);

    for (unsigned long i = 0; i < sa.size; ++i) {
        struct symbol *s = sa.buf + i;
        printf("%u %s %u %u\n", s->number, s->name, s->value, s->type);
    }

    for (unsigned long i = 0; i < ra.size; ++i) {
        struct relocation *r = ra.buf + i;
        printf("%u %u\n", r->loc, r->ref);
    }

    if (data != NULL) {
        printf("{");
        for (unsigned short i = 0; i < h.ndata; ++i) {
            printf("%2x", data[i]);
            if (i < h.ndata - 1) {
                printf(", ");
            }
        }
        printf("}\n");
    }

    return 0;
}
