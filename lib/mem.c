#include <stdlib.h>
#include <stdio.h>

#include "mem.h"

struct mem {
    int len;
    int cap;
    int data_size;
    void *buf;
};

enum { INIT_CAP = 8 };

void meminit(void *mem, int data_size, int cap)
{
    struct mem *m = mem;

    if (cap == 0) {
        cap = INIT_CAP;
    }

    m->len = 0;
    m->cap = cap;
    m->data_size = data_size;

    m->buf = malloc(m->cap * m->data_size);
    if (m->buf == NULL) {
        perror("meminit malloc");
        exit(1);
    }
}

void memgrow(void *mem)
{
    struct mem *m = mem;

    if (m->len >= m->cap) {
        m->cap = m->len * 2;
        m->buf = realloc(m->buf, m->cap * m->data_size);
        if (m->buf == NULL) {
            perror("memgrow malloc");
            exit(1);
        }
    }
}

void *memnext(void *mem)
{
    struct mem *m = mem;
    void *slot;

    memgrow(m);

    slot = (char *) m->buf + m->len * m->data_size;
    m->len++;

    return slot;
}

void memfree(void *mem)
{
    struct mem *m = mem;

    free(m->buf);
}
