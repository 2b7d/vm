#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <assert.h>

struct scanner {
    size_t cur;
    size_t start;
    char *src;
    size_t srclen;
};

enum opcode_kind {
    OP_ADD,
    OP_SUB,
    OP_LIT,
    OP_ST,
    OP_LD,
    OP_JMP,
    OP_HALT
};

struct opcode {
    enum opcode_kind kind;
    uint16_t value;
    int imm;
};

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

int has_src(struct scanner *s)
{
    return s->cur < s->srclen;
}

char advance(struct scanner *s)
{
    return s->src[s->cur++];
}

char peek(struct scanner *s)
{
    if (has_src(s) == 0) {
        return '\0';
    }

    return s->src[s->cur];
}

int next(struct scanner *s, char c)
{
    if (has_src(s) == 0) {
        return 0;
    }

    return peek(s) == c;
}

int compare(struct scanner *s, char *word)
{
    size_t len = s->cur - s->start;

    if (strlen(word) != len) {
        return 0;
    }

    if (strncmp(s->src + s->start, word, len) == 0) {
        return 1;
    }

    return 0;
}

int scan(struct scanner *s, struct opcode *op)
{
    char c;

    for (;;) {
        if (has_src(s) == 0) {
            return 0;
        }

        s->start = s->cur;
        c = advance(s);

        switch (c) {
        case ' ':
        case '\t':
        case '\r':
        case '\n':
            break;

        case '/':
            if (next(s, '/') == 1) {
                while (next(s, '\n') == 0 && has_src(s) == 1) {
                    advance(s);
                }
            }
            break;

        default:
            if (isalpha(c) != 0) {
                while(isalpha(peek(s)) != 0) {
                    advance(s);
                }

                if (compare(s, "push") == 1) {
                    op->kind = OP_LIT;
                } else if (compare(s, "add") == 1) {
                    op->kind = OP_ADD;
                } else if (compare(s, "halt") == 1) {
                    op->kind = OP_HALT;
                }else {
                    printf("ERROR: unknown word `%.*s`\n", (int)(s->cur - s->start), s->src + s->start);
                    return 0;
                }
                return 1;
            } else if (isdigit(c) != 0) {
                char val[6];

                while(isdigit(peek(s)) != 0) {
                    advance(s);
                }

                assert(s->cur - s->start < 6);
                memset(val, 0, sizeof(val));
                memcpy(val, s->src + s->start, s->cur - s->start);
                op->imm = 1;
                op->value = atoi(val);
                return 1;
            } else {
                printf("ERROR: unknown character `%c`\n", c);
                return 0;
            }
        }
    }
}

int main(int argc, char **argv)
{
    char *src;
    struct scanner s;
    struct opcode op;
    FILE *out;

    argc--;
    argv++;

    if (argc < 1) {
        printf("assembly file is required\n");
        return 1;
    }

    out = fopen("a.out", "w");
    assert(out != NULL);

    src = read_file(*argv);
    s.src = src;
    s.srclen = strlen(src);
    s.cur = 0;
    s.start = 0;

    for (;;) {
        memset(&op, 0, sizeof(struct opcode));
        if (scan(&s, &op) == 0) {
            break;
        }

        if (op.imm == 1) {
            fwrite(&op.value, sizeof(uint16_t), 1, out);
        } else {
            fwrite(&op.kind, sizeof(uint16_t), 1, out);
        }
    }

    fclose(out);
    return 0;
}
