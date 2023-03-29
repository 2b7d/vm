#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdint.h>

#include "scanner.h"
#include "compiler.h"
#include "../vm.h" // TODO: make path absolute

#include "mem.h" // lib

static struct token *peek(struct parser *p)
{
    return p->cur;
}

static int has_toks(struct parser *p)
{
    return peek(p)->kind != TOK_EOF;
}

static struct token *peek2(struct parser *p)
{
    if (has_toks(p) == 0) {
        return peek(p);
    }

    return p->cur + 1;
}

static struct token *advance(struct parser *p)
{
    struct token *t = peek(p);

    if (has_toks(p) == 1) {
        p->cur++;
    }

    return t;
}

static struct token *peek_back(struct parser *p)
{
    if (p->cur - p->toks.buf < 2) {
        return p->cur;
    }

    return p->cur - 2;
}

static int next(struct parser *p, enum token_kind k)
{
    return peek(p)->kind == k;
}

static int next2(struct parser *p, enum token_kind k)
{
    return peek2(p)->kind == k;
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

static struct token *consume_some(struct parser *p, enum token_kind *ks,
                                  size_t ksize, char *name)
{
    int found = 0;

    for (size_t i = 0; i < ksize; ++i) {
        if (next(p, ks[i]) == 1) {
            found = 1;
            break;
        }
    }

    if (found == 0) {
        fprintf(stderr, "expected %s\n", name);
        exit(1);
    }

    return advance(p);
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

static void emit_bytes(struct chunk *c, void *value, size_t len)
{
    size_t old = c->data.size;

    c->data.size += len;
    memgrow(&c->data, sizeof(uint8_t));
    memcpy(c->data.buf + old, value, len);
}

static void word_size(struct parser *p, struct chunk *c)
{
    while (next(p, TOK_NUM) == 1) {
        struct token *t = advance(p);
        int value = number(t, USHRT_MAX);
        emit_bytes(c, &value, 2);
    }
}

static void byte_size(struct parser *p, struct chunk *c)
{
    enum token_kind ks[2] = {TOK_STR, TOK_NUM};

    while (next_some(p, ks, 2) == 1) {
        struct token *t = advance(p);

        if (t->kind == TOK_STR) {
            char *start = t->start + 1;
            size_t len = t->len - 2;
            emit_bytes(c, start, len);
        } else {
            int value = number(t, UCHAR_MAX);
            emit_bytes(c, &value, 1);
        }
    }
}

static void var_declaration(struct parser *p, struct chunk *c)
{
    size_t value_size = 2;

    consume(p, TOK_SYMBOL, "symbol");
    consume(p, TOK_COLON, "colon");

    if (next(p, TOK_DB) == 1) {
        value_size = 1;
        advance(p);
    }

    if (value_size == 1) {
        byte_size(p, c);
    } else {
        word_size(p, c);
    }
}

static void section_data(struct parser *p, struct chunk_array *ca)
{
    while (next(p, TOK_SYMBOL) == 1 && next2(p, TOK_COLON) == 1) {
        struct chunk *c;

        memgrow(ca, sizeof(struct chunk));
        c = ca->buf + ca->size;
        ca->size++;

        c->name = TOK_DATA;
        var_declaration(p, c);
    }
}

static void func_declaration(struct parser *p, struct chunk *c)
{
    enum token_kind ks[4] = {TOK_OPCODE, TOK_SYMBOL, TOK_NUM, TOK_STR};

    consume(p, TOK_SYMBOL, "symbol");
    consume(p, TOK_COLON, "colon");

    while (next_some(p, ks, 4) == 1 &&
            (next(p, TOK_SYMBOL) == 0 || next2(p, TOK_COLON) == 0)) {
        struct token *prev, *t = advance(p);
        int value;
        uint8_t op8;
        uint16_t op;
        char *start;
        size_t len;

        switch (t->kind) {
        case TOK_STR:
            start = t->start + 1;
            len = t->len - 2;

            if (len != 1) {
                fprintf(stderr, "expected 1 character string\n");
                exit(1);
            }

            emit_bytes(c, start, 1);
            break;

        case TOK_NUM:
            // TODO: this should be easier, without checking previous token
            prev = peek_back(p);
            if (prev->kind != TOK_OPCODE) {
                fprintf(stderr, "expected opcode before constant\n");
                exit(1);
            }

            if (prev->is_byteop == 1) {
                value = number(t, UCHAR_MAX);
                emit_bytes(c, &value, 1);
            } else {
                value = number(t, USHRT_MAX);
                emit_bytes(c, &value, 2);
            }
            break;

        case TOK_OPCODE:
            op8 = (1 << 7) | t->opcode;
            if (t->is_byteop == 1) {
                op8 = t->opcode;
            }

            emit_bytes(c, &op8, 1);
            break;

        case TOK_SYMBOL:
            op = 0xaffa;
            emit_bytes(c, &op, 2);
            break;

        default:
            fprintf(stdout, "unreachable\n");
            exit(1);
        }
    }
}

static void section_text(struct parser *p, struct chunk_array *ca)
{
    while (next(p, TOK_SYMBOL) == 1 && next2(p, TOK_COLON) == 1) {
        struct chunk *c;

        memgrow(ca, sizeof(struct chunk));
        c = ca->buf + ca->size;
        ca->size++;

        c->name = TOK_TEXT;
        func_declaration(p, c);
    }
}

static void section(struct parser *p, struct chunk_array *ca)
{
    struct token *t;
    enum token_kind kinds[2] = {TOK_DATA, TOK_TEXT};

    t = consume_some(p, kinds, 2, "data or text");

    if (t->kind == TOK_DATA) {
        section_data(p, ca);
    } else {
        section_text(p, ca);
    }
}

void compile(struct parser *p, struct chunk_array *ca)
{
    for (;;) {
        struct token *t;

        if (has_toks(p) == 0) {
            break;
        }

        t = advance(p);

        if (t->kind == TOK_SECTION) {
            section(p, ca);
        }
    }
}

void parser_init(struct parser *p, struct scanner *s)
{
    meminit(&p->toks, sizeof(struct token), 32); // LEAK: os is freeing

    for (;;) {
        struct token *t;

        memgrow(&p->toks, sizeof(struct token));
        t = p->toks.buf + p->toks.size;
        p->toks.size++;

        scan_token(s, t);
        if (t->kind == TOK_EOF) {
            break;
        }
    }

    p->cur = p->toks.buf;
}
