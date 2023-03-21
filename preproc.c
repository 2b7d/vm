#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>

#include "mem.h" // lib

char *read_file(char *pathname)
{
    int fd;
    struct stat statbuf;
    char *src;

    fd = open(pathname, O_RDONLY);
    if (fd < 0) {
        perror("failed to open file");
        exit(1);
    }

    if (fstat(fd, &statbuf) < 0) {
        perror("failed to get file info");
        exit(1);
    }

    src = malloc(statbuf.st_size + 1);
    if (src == NULL) {
        perror("failed to allocate memory for source");
        exit(1);
    }

    if (read(fd, src, statbuf.st_size) < 0) {
        perror("failed read file content into source");
        exit(1);
    }

    src[statbuf.st_size] = '\0';

    close(fd);
    return src;
}

struct parser {
    char *src;
    char *cur;
    char *start;
};

struct define {
    char *start;
    size_t len;

    char *name;
    size_t namelen;

    char *value;
    size_t valuelen;
};

struct define_array {
    size_t size;
    size_t cap;
    struct define *buf;
};

int main(int argc, char **argv)
{
    struct parser p;
    struct define_array defs;
    FILE *out;

    argc--;
    argv++;

    out = fopen("out.i", "w");
    if (out == NULL) {
        printf("failed to open output file\n");
        return 1;
    }

    meminit(&defs, sizeof(struct define), 0);
    p.src = read_file(*argv);
    p.cur = p.src;

    for (;;) {
        struct define *d;

        if (*p.cur == '\0') {
            break;
        }

        p.start = p.cur;

        if (*p.cur != '#') {
            p.cur++;
            continue;
        }

        while (*p.cur != ' ' && *p.cur != '\0') {
            p.cur++;
        }

        if (memcmp(p.start + 1, "define", p.cur - p.start - 1) != 0) {
            printf("Unknown directive %.*s\n", (int) (p.cur - p.start),
                                               p.start);
            return 1;
        }

        memgrow(&defs, sizeof(struct define));
        d = defs.buf + defs.size;
        defs.size++;

        d->start = p.start;

        p.cur++;
        p.start = p.cur;
        while (*p.cur != ' ' && *p.cur != '\0') {
            p.cur++;
        } // TODO: handle error

        d->name = p.start;
        d->namelen = p.cur - p.start;

        p.cur++;
        p.start = p.cur;
        while (*p.cur != '\n' && *p.cur != '\0') {
            p.cur++;
        } // TODO: handle error

        d->value = p.start;
        d->valuelen = p.cur - p.start;
        d->len = p.cur - d->start;
        p.cur++;
    }

    p.cur = p.src;
    while (*p.cur != '\0') {
        for (size_t i = 0; i < defs.size; ++i) {
            struct define *d = defs.buf + i;

            if (p.cur == d->start) {
                p.cur += d->len + 1;
                i = 0;
            }
        }

        p.start = p.cur;
        while (*p.cur != ' ' && *p.cur != '\n' && *p.cur != '\0') {
            p.cur++;
        }

        {
            int found = 0;

            for (size_t i = 0; i < defs.size; ++i) {
                struct define *d = defs.buf + i;

                if ((size_t) (p.cur - p.start) != d->namelen) {
                    continue;
                }

                if (memcmp(p.start, d->name, d->namelen) == 0) {
                    found = 1;
                    fwrite(d->value, d->valuelen, 1, out);
                    break;
                }
            }

            if (found == 0) {
                p.cur++;
                fwrite(p.start, p.cur - p.start, 1, out);
            }
        }
    }

    return 0;
}
