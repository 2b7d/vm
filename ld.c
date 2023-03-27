#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "mem.h"

enum symbol_bind {
    BIND_UNDEF,
    BIND_LOCAL,
    BIND_GLOBAL,
    BIND_EXTERN
};

enum symbol_type {
    TYPE_UNDEF,
    TYPE_VAR,
    TYPE_FUNC
};

struct symbol {
    char name[256];
    size_t len;
    size_t value;
    size_t size;
    size_t tpos;
    enum symbol_bind bind;
    enum symbol_type type;
};

struct symbol_array {
    size_t size;
    size_t cap;
    struct symbol *buf;
};

int main(int argc, char **argv)
{
    struct symbol_array syms;

    argc--;
    argv++;

    if (argc < 1) {
        puts("object file[s] required");
        return 1;
    }

    meminit(&syms, sizeof(struct symbol), 16);

    while (*argv != NULL) {
        int fd, s;
        struct stat statbuf;
        char buf[1024] = {0},
             data[1024] = {0},
             text[1024] = {0},
             symtab[1024] = {0};
        size_t i, j, size, datasz = 0, textsz = 0, symtabsz = 0;

        fd = open(*argv, O_RDONLY);
        if (fd < 0) {
            perror("failed to open file");
            return 1;
        }

        if (fstat(fd, &statbuf) < 0) {
            perror("failed to get file info");
            return 1;
        }

        read(fd, buf, statbuf.st_size);
        size = (size_t) statbuf.st_size;

        s = -1;
        for (i = 0; i < size; ++i) {
            if (buf[i] == '.') {
                if (memcmp(buf+i, ".data", 5) == 0) {
                    i += 5;
                    s = 1;
                    j = 0;
                } else if (memcmp(buf+i, ".text", 5) == 0) {
                    i += 5;
                    s = 2;
                    j = 0;
                } else if (memcmp(buf+i, ".symtab", 7) == 0) {
                    i += 7;
                    s = 3;
                    j = 0;
                } else {
                    puts("this must be unreachable or object file is invalid");
                    return 1;
                }
            }

            if (s == 1) {
                data[j++] = buf[i];
                datasz++;
            } else if (s == 2) {
                text[j++] = buf[i];
                textsz++;
            } else if (s == 3) {
                symtab[j++] = buf[i];
                symtabsz++;
            } else {
                puts("this must be unreachable or object file is invalid");
                return 1;
            }
        }

        close(fd);

        for (i = 0; i < symtabsz;) {
            struct symbol *s;

            memgrow(&syms, sizeof(struct symbol));
            s = syms.buf + syms.size;
            syms.size++;

            memcpy(&s->value, symtab+i, 2);
            i += 2;
            memcpy(&s->size, symtab+i, 2);
            i += 2;
            memcpy(&s->tpos, symtab+i, 2);
            i += 2;
            memcpy(&s->type, symtab+i, 1);
            i += 1;
            memcpy(&s->bind, symtab+i, 1);
            i += 1;
            memcpy(&s->len, symtab+i, 2);
            i += 2;
            assert(s->len < sizeof(s->name));
            memcpy(s->name, symtab+i, s->len);
            i += s->len;
        }

        argv++;
    }

    for (size_t i = 0; i < syms.size; ++i) {
        struct symbol *s = syms.buf + i;
        printf("value: %3lu, size: %3lu, tpos: %3ld, "
               "bind: %d, type: %d, %.*s\n", s->value, s->size, s->tpos,
                                             s->bind, s->type,
                                             (int) s->len, s->name);
    }

    return 0;
}
