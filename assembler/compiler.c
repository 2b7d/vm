#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "scanner.h"
#include "compiler.h"

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
    struct token *t = peek(p);

    if (t->kind != k) {
        fprintf(stderr, "expected %s, but got %.*s\n", name, t->len, t->lex);
        exit(1);
    }

    return advance(p);
}

static struct sym *find_symbol(struct parser *p, struct token *t)
{
    for (int i = 0; i < p->sa.size; ++i) {
        struct sym *s = p->sa.buf + i;

        if (t->len == s->namelen && memcmp(s->name, t->lex, t->len) == 0) {
            return s;
        }
    }

    return NULL;
}

static struct sym *add_symbol(struct parser *p, struct token *t)
{
    int index = p->sa.size;
    struct sym *s = memnext((struct mem *) &p->sa);

    s->name = t->lex;
    s->namelen = t->len;
    s->is_resolved = 0;
    s->index = index;

    return s;
}

static struct rel *add_rel(struct parser *p, struct sym *s, struct token *t)
{
    struct rel *r = memnext((struct mem *) &p->ra);
    int index = 0;
    int resolved = 0;

    if (s != NULL) {
        resolved = 1;
        index = s->index;
    }

    r->loc = p->code.size;
    r->ref = index;
    r->is_resolved = resolved;
    r->name = t->lex;
    r->namelen = t->len;

    return r;
}

static void symbol_type_declaration(struct parser *p, enum sym_type type)
{
    for (;;) {
        struct token *t = consume(p, TOK_SYMBOL, "symbol");
        struct sym *s;

        s = find_symbol(p, t);
        if (s != NULL) {
            fprintf(stderr, "symbol %.*s already defined\n", t->len, t->lex);
            exit(1);
        }

        s = add_symbol(p, t);
        s->type = type;

        if (type == TYPE_EXTERN) {
            s->is_resolved = 1;
        }

        if (next(p, TOK_COMMA) == 1) {
            advance(p);
        } else {
            break;
        }
    }
}

static void resolve_symbol(struct parser *p, struct token *decl)
{
    struct sym *s;

    s = find_symbol(p, decl);
    if (s == NULL) {
        s = add_symbol(p, decl);
        s->type = TYPE_LOCAL;
    }
    s->value = p->code.size;
    s->is_resolved = 1;

    for (int i = 0; i < p->ra.size; ++i) {
        struct rel *r = p->ra.buf + i;

        if (s->namelen == r->namelen &&
                memcmp(r->name, s->name, s->namelen) == 0) {
            r->ref = s->index;
            r->is_resolved = 1;
        }
    }
}

static void emit_bytes(struct parser *p, void *buf, int size)
{
    int old_size = p->code.size;

    p->code.size += size;
    memgrow((struct mem *) &p->code);
    memcpy(p->code.buf + old_size, buf, size);
}

static void opcode_statement(struct parser *p, struct token *opcode)
{
    int valsize = 2;
    uint8_t op = (1 << 7) | (opcode->value & 0xff);
    enum token_kind expected_kinds[3] = {TOK_NUM, TOK_STR, TOK_SYMBOL};

    if (opcode->kind == TOK_OPCODE_BYTE) {
        valsize = 1;
        op = opcode->value;
    }

    emit_bytes(p, &op, 1);

    if (next_some(p, expected_kinds, 3) == 1) {
        struct token *t = advance(p);

        switch (t->kind) {
        case TOK_NUM:
            emit_bytes(p, &t->value, valsize);
            break;

        case TOK_STR:
            if (t->len > 1) {
                fprintf(stderr, "expected character but got %.*s\n",
                                t->len, t->lex);
                exit(1);
            }
            emit_bytes(p, t->lex, 1);
            break;

        case TOK_SYMBOL: {
            // NOTE: just random value; its going to be relocated anyway
            int value = 0;
            struct sym *s = find_symbol(p, t);

            add_rel(p, s, t);
            emit_bytes(p, &value, 2);
        } break;

        default:
            fprintf(stderr, "unreachable; token: %.*s\n", t->len, t->lex);
            exit(1);
        }
    }
}

static void symbol_declaration(struct parser *p)
{
    enum token_kind expected_kinds[5] = {TOK_STR, TOK_NUM, TOK_BYTES,
                                         TOK_OPCODE, TOK_OPCODE_BYTE};

    consume(p, TOK_COLON, ":");

    while (next_some(p, expected_kinds, 5) == 1) {
        struct token *t = advance(p);

        switch (t->kind) {
        case TOK_OPCODE:
        case TOK_OPCODE_BYTE:
            opcode_statement(p, t);
            break;

        case TOK_STR:
            emit_bytes(p, t->lex, t->len);
            break;

        case TOK_NUM:
            emit_bytes(p, &t->value, 2);
            break;

        case TOK_BYTES:
            t = consume(p, TOK_NUM, "number");
            emit_bytes(p, &t->value, 1);

            while (next(p, TOK_NUM) == 1) {
                t = advance(p);
                emit_bytes(p, &t->value, 1);
            }
            break;

        default:
            fprintf(stderr, "unreachable; token %.*s\n", t->len, t->lex);
            exit(1);
        }
    }
}

void compile(struct parser *p)
{
    for (;;) {
        struct token *t;

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
            resolve_symbol(p, t);
            symbol_declaration(p);
            break;

        default:
            fprintf(stderr, "unknown token %.*s\n", t->len, t->lex);
            exit(1);
        }
    }

    for (int i = 0; i < p->sa.size; ++i) {
        struct sym *s = p->sa.buf + i;
        if (s->is_resolved == 0) {
            fprintf(stderr, "undefined symbol %.*s\n", s->namelen, s->name);
            exit(1);
        }
    }

    for (int i = 0; i < p->ra.size; ++i) {
        struct rel *r = p->ra.buf + i;
        if (r->is_resolved == 0) {
            fprintf(stderr, "undefined symbol %.*s\n", r->namelen, r->name);
            exit(1);
        }
    }
}

void parser_init(struct parser *p)
{
    meminit((struct mem *) &p->ta, sizeof(struct token), 32); // LEAK: os free
    meminit((struct mem *) &p->sa, sizeof(struct sym), 16); // LEAK: os free
    meminit((struct mem *) &p->ra, sizeof(struct rel), 16); // LEAK: os free
    meminit((struct mem *) &p->code, sizeof(uint8_t), 64); // LEAK: os free

    p->cur = p->ta.buf;
}
