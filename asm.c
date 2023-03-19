#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <assert.h>

#include "vm.h"
#include "mem.h"

struct scanner {
    size_t cur;
    size_t start;
    char *src;
    size_t srclen;
};

enum token_kind {
    TOKEN_OPCODE,
    TOKEN_IMMEDIATE,
    TOKEN_LABEL
};

struct token {
    enum vm_opcode opcode;
    enum token_kind kind;
    uint16_t literal;
};

struct tokens {
    size_t size;
    size_t cap;
    struct token *buf;
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

char peek(struct scanner *s)
{
    if (has_src(s) == 0) {
        return '\0';
    }

    return s->src[s->cur];
}

char advance(struct scanner *s)
{
    char c = peek(s);

    if (c != '\0') {
        s->cur++;
    }

    return c;
}

int next(struct scanner *s, char c)
{
    return peek(s) == c;
}

size_t lexeme_len(struct scanner *s)
{
    return s->cur - s->start;
}

char *lexeme_start(struct scanner *s)
{
    return s->src + s->start;
}

int compare_lexeme(struct scanner *s, char *word)
{
    size_t len = lexeme_len(s);

    if (strlen(word) != len) {
        return 0;
    }

    if (strncmp(lexeme_start(s), word, len) == 0) {
        return 1;
    }

    return 0;
}

int scan(struct scanner *s, struct tokens *toks)
{
    char c;
    struct token *t;

    for (;;) {
        if (has_src(s) == 0) {
            return 1;
        }

        memgrow(toks, sizeof(struct token));
        t = toks->buf + toks->size;

        s->start = s->cur;
        c = advance(s);

        switch (c) {
        case ' ':
        case '\t':
        case '\r':
        case '\n':
            break;

        case ';':
            while (next(s, '\n') == 0 && has_src(s) == 1) {
                advance(s);
            }
            break;

        default:
            if (isalpha(c) != 0) {
                while(isalpha(peek(s)) != 0) {
                    advance(s);
                }

                if (compare_lexeme(s, "st") == 1) {
                    t->opcode = OP_ST;
                } else if (compare_lexeme(s, "ld") == 1) {
                    t->opcode = OP_LD;
                } else if (compare_lexeme(s, "push") == 1) {
                    t->opcode = OP_LIT;
                } else if (compare_lexeme(s, "add") == 1) {
                    t->opcode = OP_ADD;
                } else if (compare_lexeme(s, "sub") == 1) {
                    t->opcode = OP_SUB;
                } else if (compare_lexeme(s, "eq") == 1) {
                    t->opcode = OP_EQ;
                } else if (compare_lexeme(s, "gt") == 1) {
                    t->opcode = OP_GT;
                } else if (compare_lexeme(s, "lt") == 1) {
                    t->opcode = OP_LT;
                } else if (compare_lexeme(s, "or") == 1) {
                    t->opcode = OP_OR;
                } else if (compare_lexeme(s, "and") == 1) {
                    t->opcode = OP_AND;
                } else if (compare_lexeme(s, "not") == 1) {
                    t->opcode = OP_NOT;
                } else if (compare_lexeme(s, "jmp") == 1) {
                    t->opcode = OP_JMP;
                } else if (compare_lexeme(s, "ifjmp") == 1) {
                    t->opcode = OP_IFJMP;
                } else if (compare_lexeme(s, "halt") == 1) {
                    t->opcode = OP_HALT;
                } else {
                    printf("ERROR: unknown word `%.*s`\n",
                           (int) lexeme_len(s),
                           lexeme_start(s));
                    return 0;
                }

                t->kind = TOKEN_OPCODE;
                toks->size++;
                break;
            }

            if (isdigit(c) != 0) {
                char val[6];

                while(isdigit(peek(s)) != 0) {
                    advance(s);
                }

                assert(lexeme_len(s) < 6);
                memset(val, 0, sizeof(val));
                memcpy(val, lexeme_start(s), lexeme_len(s));
                t->kind = TOKEN_IMMEDIATE;
                t->literal = atoi(val);
                toks->size++;
                break;
            }

            printf("ERROR: unknown character `%c`\n", c);
            return 0;
        }
    }
}

int main(int argc, char **argv)
{
    struct scanner s;
    struct tokens toks;
    FILE *out;

    argc--;
    argv++;

    if (argc < 1) {
        printf("assembly file is required\n");
        return 1;
    }

    // TODO: unhardcode outfile name
    out = fopen("a.out", "w");
    if (out == NULL) {
        printf("failed to open out file\n");
        return 1;
    }

    s.src = read_file(*argv);
    s.srclen = strlen(s.src);
    s.cur = 0;
    s.start = 0;

    meminit(&toks, sizeof(struct token), 64);

    if (scan(&s, &toks) == 0) {
        return 1;
    }

    for (size_t i = 0; i < toks.size; ++i) {
        struct token *t = toks.buf + i;
        uint16_t val = t->opcode;

        if (t->kind == TOKEN_IMMEDIATE) {
            val = t->literal;
        }

        fwrite(&val, sizeof(uint16_t), 1, out);
    }

    memfree(&toks);
    free(s.src);
    fclose(out);
    return 0;
}
