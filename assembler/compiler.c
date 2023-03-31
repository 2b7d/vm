#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdint.h>

#include "scanner.h"
#include "compiler.h"
//#include "../vm.h" // TODO: make path absolute

#include "mem.h" // lib

static struct token *peek(struct parser *p)
{
    return p->cur;
}

static int has_toks(struct parser *p)
{
    return peek(p)->kind != TOK_EOF;
}

static struct token *advance(struct parser *p)
{
    struct token *t = peek(p);

    if (has_toks(p) == 1) {
        p->cur++;
    }

    return t;
}

static int next(struct parser *p, enum token_kind k)
{
    return peek(p)->kind == k;
}

static int next_some(struct parser *p, enum token_kind *ks, size_t ksize)
{
    for (size_t i = 0; i < ksize; ++i) {
        if (next(p, ks[i]) == 1) {
            return 1;
        }
    }

    return 0;
}

static struct token *consume(struct parser *p, enum token_kind k, char *name)
{
    if (next(p, k) == 0) {
        fprintf(stderr, "expected %s\n", name);
        exit(1);
    }

    return advance(p);
}

static struct sym *find_symbol(struct parser *p, char *name, size_t len)
{
    for (unsigned long i = 0; i < p->sa.size; ++i) {
        struct sym *s = p->sa.buf + i;

        if (len == s->namelen && memcmp(s->name, name, len) == 0) {
            return s;
        }
    }

    return NULL;
}

static struct sym *add_symbol(struct parser *p, char *name, size_t len)
{
    struct sym *s;
    unsigned long index = p->sa.size;

    memgrow((struct mem *) &p->sa);
    s = p->sa.buf + p->sa.size;
    p->sa.size++;

    s->name = name;
    s->namelen = len;
    s->is_resolved = 0;
    s->index = index;

    return s;
}

static struct rel *add_rel(struct parser *p, char *name, size_t namelen,
                           size_t index, int resolved)
{
    struct rel *r;

    memgrow((struct mem *) &p->ra);
    r = p->ra.buf + p->ra.size;
    p->ra.size++;

    r->loc = p->code.size;
    r->ref = index;
    r->is_resolved = resolved;
    r->name = name;
    r->namelen = namelen;

    return r;
}

static void symbol_type_declaration(struct parser *p, enum sym_type type)
{
    for (;;) {
        struct token *t = consume(p, TOK_SYMBOL, "symbol");
        struct sym *s;

        s = find_symbol(p, t->start, t->len);
        if (s != NULL) {
            fprintf(stderr, "symbol already defined\n");
            exit(1);
        }

        s = add_symbol(p, t->start, t->len);
        s->type = type;

        if (type == TYPE_EXTERN) {
            s->is_resolved = 1;
        }

        if (peek(p)->kind == TOK_COMMA) {
            advance(p);
        } else {
            break;
        }
    }
}

static int number(struct token *t, size_t max_size)
{
    char valbuf[6];
    size_t bufsize = sizeof(valbuf);
    size_t value;

    if (t->len > bufsize - 1) {
        fprintf(stderr, "number is too big\n");
        exit(1);
    }

    memset(valbuf, 0, bufsize);
    memcpy(valbuf, t->start, t->len);

    value = atoi(valbuf);
    if (value > max_size) {
        fprintf(stderr, "number is too big\n");
        exit(1);
    }

    return value;
}

static void symbol_declaration(struct parser *p)
{
    struct token *t;
    struct sym *s;
    size_t old_size, len, valsize;
    char *start;
    int value;
    uint8_t op;
    enum token_kind ks[3] = {TOK_NUM, TOK_STR, TOK_SYMBOL};
    enum token_kind sks[5] = {TOK_OPCODE, TOK_OPCODE_BYTE, TOK_STR, TOK_NUM, TOK_BYTES};

    while (next_some(p, sks, 5) == 1) {
        t = advance(p);

        switch (t->kind) {
        case TOK_OPCODE:
        case TOK_OPCODE_BYTE:
            valsize = 2;
            op = (1 << 7) | t->opcode;
            if (t->kind == TOK_OPCODE_BYTE) {
                valsize = 1;
                op = t->opcode;
            }
            memgrow((struct mem *) &p->code);
            p->code.buf[p->code.size++] = op;

            if (next_some(p, ks, 3) == 1) {
                t = advance(p);

                switch (t->kind) {
                case TOK_NUM:
                    old_size = p->code.size;
                    value = number(t, valsize == 2 ? USHRT_MAX : UCHAR_MAX);
                    p->code.size += valsize;
                    memgrow((struct mem *) &p->code);
                    memcpy(p->code.buf + old_size, &value, valsize);
                    break;

                case TOK_STR:
                    start = t->start + 1;
                    len = t->len - 2;
                    old_size = p->code.size;

                    if (len != 1) {
                        fprintf(stderr, "expected character\n");
                        exit(1);
                    }

                    memgrow((struct mem *) &p->code);
                    p->code.buf[p->code.size++] = *start;
                    break;

                case TOK_SYMBOL:
                    {
                        size_t index = 0;
                        int resolved = 0;

                        s = find_symbol(p, t->start, t->len);
                        if (s != NULL) {
                            resolved = 1;
                            index = s->index;
                        }
                        add_rel(p, t->start, t->len, index, resolved);
                    }
                    old_size = p->code.size;
                    p->code.size += 2;
                    value = 0;
                    memgrow((struct mem *) &p->code);
                    memcpy(p->code.buf + old_size, &value, 2);
                    break;

                default:
                    fprintf(stderr, "unreachable\n");
                    exit(1);
                }
            }
            break;

        case TOK_STR:
            start = t->start + 1;
            len = t->len - 2;
            old_size = p->code.size;

            if (len == 0) {
                fprintf(stderr, "empty string\n");
                exit(1);
            }

            p->code.size += len;
            memgrow((struct mem *) &p->code);
            memcpy(p->code.buf + old_size, start, len);
            break;

        case TOK_NUM:
            old_size = p->code.size;
            value = number(t, USHRT_MAX);
            p->code.size += 2;
            memgrow((struct mem *) &p->code);
            memcpy(p->code.buf + old_size, &value, 2);
            break;

        case TOK_BYTES:
            while (next(p, TOK_NUM) == 1) {
                struct token *num = advance(p);
                value = number(num, UCHAR_MAX);
                memgrow((struct mem *) &p->code);
                p->code.buf[p->code.size++] = value;
            }
            break;

        default:
            fprintf(stderr, "unexpected token\n");
            exit(1);
        }
    }
}

void compile(struct parser *p)
{
    FILE *out;
    char c;

    for (;;) {
        struct token *t;
        struct sym *s;

        if (has_toks(p) == 0) {
            break;
        }

        t = advance(p);

        switch (t->kind) {
        case TOK_GLOBAL:
            symbol_type_declaration(p, TYPE_GLOBAL);
            break;

        case TOK_EXTERN:
            symbol_type_declaration(p, TYPE_EXTERN);
            break;

        case TOK_SYMBOL:
            consume(p, TOK_COLON, ":");

            s = find_symbol(p, t->start, t->len);
            if (s == NULL) {
                s = add_symbol(p, t->start, t->len);
                s->type = TYPE_LOCAL;
            }

            s->value = p->code.size;
            s->is_resolved = 1;

            for (size_t i = 0; i < p->ra.size; ++i) {
                struct rel *r = p->ra.buf + i;

                if (s->namelen == r->namelen &&
                        memcmp(r->name, s->name, s->namelen) == 0) {
                    r->ref = s->index;
                    r->is_resolved = 1;
                }
            }

            symbol_declaration(p);
            break;

        default:
            fprintf(stderr, "unknown token\n");
            exit(1);
        }
    }

    for (size_t i = 0; i < p->sa.size; ++i) {
        struct sym *s = p->sa.buf + i;
        if (s->is_resolved == 0) {
            fprintf(stderr, "undefined symbol\n");
            exit(1);
        }
    }

    for (size_t i = 0; i < p->ra.size; ++i) {
        struct rel *r = p->ra.buf + i;
        if (r->is_resolved == 0) {
            fprintf(stderr, "undefined symbol\n");
            exit(1);
        }
    }

    out = fopen("out.o", "w");
    if (out == NULL) {
        perror("fopen failed");
        exit(1);
    }

    fwrite(&p->sa.size, 2, 1, out);
    fwrite(&p->ra.size, 2, 1, out);
    fwrite(&p->code.size, 2, 1, out);

    c = '\n';
    fwrite(&c, 1, 1, out);

    for (size_t i = 0; i < p->sa.size; ++i) {
        struct sym *s = p->sa.buf + i;

        fwrite(s->name, 1, s->namelen, out);
        c = '\0';
        fwrite(&c, 1, 1, out);

        fwrite(&s->value, 2, 1, out);
        fwrite(&s->type, 1, 1, out);
    }

    c = '\n';
    fwrite(&c, 1, 1, out);

    for (size_t i = 0; i < p->ra.size; ++i) {
        struct rel *r = p->ra.buf + i;
        fwrite(&r->loc, 2, 1, out);
        fwrite(&r->ref, 2, 1, out);
    }

    c = '\n';
    fwrite(&c, 1, 1, out);

    fwrite(p->code.buf, 1, p->code.size, out);
    fclose(out);
}
