#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdint.h>

#include "vm.h"
#include "util.h"
#include "mem.h" // lib

struct scanner {
        char *cur;
        char *start;
        char *src;
};

enum token_kind {
        // keywords need to be zero aligned to use as indices of array
        TOK_DB,
        TOK_EXTERN,
        TOK_GLOBAL,
        TOK_SECTION,
        TOK_TEXT,
        TOK_DATA,

        TOK_NUM,
        TOK_STR,
        TOK_OPCODE,
        TOK_SYMBOL,
        TOK_COMMA,
        TOK_COLON,

        TOK_EOF
};

struct token {
        enum token_kind kind;
        char *start;
        size_t len;
        int value;
};

struct token_array {
        size_t size;
        size_t cap;
        struct token *buf;
};

struct compiler {
        size_t cur;
        struct token_array *toks;
        int has_data;
        int has_text;
        FILE *out;
};

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
        char *name;
        size_t len;
        size_t value;
        size_t size;
        size_t tpos;
        int isresolved;
        enum symbol_bind bind;
        enum symbol_type type;
};

struct symbol_array {
        size_t size;
        size_t cap;
        struct symbol *buf;
};

enum { KWD_COUNT = 6 };
char *kwds_str[KWD_COUNT] = {
       [TOK_DB]      = "db",
       [TOK_EXTERN]  = "extern",
       [TOK_GLOBAL]  = "global",
       [TOK_SECTION] = "section",
       [TOK_TEXT]    = "text",
       [TOK_DATA]    = "data"
};

char *opcodes_str[OP_COUNT] = {
        [OP_ST]       = "st",
        [OP_LD]       = "ld",
        [OP_ADD]      = "add",
        [OP_SUB]      = "sub",
        [OP_MUL]      = "mul",
        [OP_DIV]      = "div",
        [OP_MOD]      = "mod",
        [OP_INC]      = "inc",
        [OP_DEC]      = "dec",
        [OP_PUSH]     = "push",
        [OP_DUP]      = "dup",
        [OP_OVER]     = "over",
        [OP_SWAP]     = "swap",
        [OP_DROP]     = "drop",
        [OP_RSPUSH]   = "rspush",
        [OP_RSPOP]    = "rspop",
        [OP_RSCOPY]   = "rscopy",
        [OP_RSDROP]   = "rsdrop",
        [OP_RSP]      = "rsp",
        [OP_RSPSET]   = "rspset",
        [OP_BRK]      = "brk",
        [OP_BRKSET]   = "brkset",
        [OP_EQ]       = "eq",
        [OP_GT]       = "gt",
        [OP_LT]       = "lt",
        [OP_OR]       = "or",
        [OP_AND]      = "and",
        [OP_NOT]      = "not",
        [OP_JMP]      = "jmp",
        [OP_JMPIF]    = "jmpif",
        [OP_HALT]     = "halt",
        [OP_SYSCALL]  = "syscall",
        [OP_CALL]     = "call",
        [OP_RET]      = "ret"
};

int ischar(char c)
{
        return isalpha(c) != 0 || c == '_' || c == '.';
}

int isbyteop(struct token *t)
{
        return t->start[t->len - 1] == '8';
}

struct token *tok_peek(struct compiler *c)
{
        return c->toks->buf + c->cur;
}

int has_toks(struct compiler *c)
{
        return tok_peek(c)->kind != TOK_EOF;
}

struct token *tok_advance(struct compiler *c)
{
        struct token *t = tok_peek(c);

        if (has_toks(c) == 1) {
                c->cur++;
        }

        return t;
}

int tok_next(struct compiler *c, enum token_kind k)
{
        if (has_toks(c) == 0) {
                return 0;
        }

        return tok_peek(c)->kind == k;
}

int tok_next2(struct compiler *c, enum token_kind k)
{
        struct token *t;

        if (c->cur + 1 >= c->toks->size) {
                return 0;
        }

        t = c->toks->buf + c->cur + 1;
        return t->kind == k;
}

struct token *tok_consume(struct compiler *c, enum token_kind k, char *name)
{
        struct token *t = tok_advance(c);

        if (t->kind != k) {
                printf("expected %s but got %.*s\n",
                                name, (int) t->len, t->start);
                exit(1);
        }

        return t;
}

struct symbol *symbol_get(struct symbol_array *syms, char *name, size_t len)
{
        struct symbol *s;

        for (size_t i = 0; i < syms->size; ++i) {
                s = syms->buf + i;
                if (s->len == len && memcmp(name, s->name, s->len) == 0) {
                        return s;
                }
        }

        return NULL;
}

struct symbol *symbol_add(struct symbol_array *syms, char *name, size_t len)
{
        struct symbol *s;

        memgrow(syms, sizeof(struct symbol));
        s = syms->buf + syms->size;
        syms->size++;

        s->name = name;
        s->len = len;
        s->value = 0;
        s->size = 0;
        s->tpos = 0;
        s->isresolved = 0;
        s->bind = BIND_UNDEF;
        s->type = TYPE_UNDEF;

        return s;
}

void compile_symbol_list(struct compiler *c, struct symbol_array *syms,
                         enum symbol_bind bind)
{
        struct token *t;
        struct symbol *s;

        for (;;) {
                t = tok_consume(c, TOK_SYMBOL, "symbol");
                s = symbol_get(syms, t->start, t->len);
                if (s == NULL) {
                        s = symbol_add(syms, t->start, t->len);
                }
                s->bind = bind;
                if (bind == BIND_EXTERN) {
                        s->isresolved = 1;
                }

                if (tok_next(c, TOK_COMMA) == 0) {
                        break;
                }
                tok_advance(c);
        }
}

struct token *compile_var_declaration(struct compiler *c, size_t *size)
{
        struct token *decl;
        size_t valuesize = 2;

        if (tok_next(c, TOK_SYMBOL) == 0) {
                return NULL;
        }

        decl = tok_advance(c);
        tok_consume(c, TOK_COLON, "colon");

        if (tok_next(c, TOK_DB) == 1) {
                valuesize = 1;
                tok_advance(c);
        }

        for (;;) {
                struct token *t;

                if (tok_next(c, TOK_STR) == 1) {
                        t = tok_advance(c);
                        *size += t->len;
                        fwrite(t->start, 1, t->len, c->out);
                        continue;
                }

                if (tok_next(c, TOK_NUM) == 1) {
                        t = tok_advance(c);
                        *size += valuesize;
                        fwrite(&t->value, valuesize, 1, c->out);
                        continue;
                }

                break;
        }

        return decl;
}

struct token *compile_func_declaration(struct compiler *c,
                                       struct symbol_array *syms, size_t *size)
{
        struct token *t, *prev, *decl;

        if (tok_next(c, TOK_SYMBOL) == 0) {
                return NULL;
        }

        decl = tok_advance(c);
        tok_consume(c, TOK_COLON, "colon");
        prev = tok_peek(c);

        for (;;) {
                uint16_t op;
                uint8_t op8;
                struct symbol *s;
                size_t valuesize = 2;

                t = tok_advance(c);

                if (t->kind < TOK_NUM || t->kind > TOK_SYMBOL) {
                        break;
                }

                switch (t->kind) {
                case TOK_NUM:
                        if (prev->kind == TOK_OPCODE && isbyteop(prev) == 1) {
                                valuesize = 1;
                        }
                        *size += valuesize;
                        fwrite(&t->value, valuesize, 1, c->out);
                        break;

                case TOK_STR:
                        if (t->len > 1) {
                                puts("expected character");
                                exit(1);
                        }
                        *size += 1;
                        fwrite(t->start, 1, 1, c->out);
                        break;

                case TOK_OPCODE:
                        op8 = (1 << 7) | t->value;

                        *size += 1;
                        fwrite(&op8, 1, 1, c->out);
                        break;

                case TOK_SYMBOL:
                        op = 0;
                        s = symbol_get(syms, t->start, t->len);
                        if (s == NULL) {
                            s = symbol_add(syms, t->start, t->len);
                            s->bind = BIND_LOCAL;
                        }
                        s->tpos = *size;
                        *size += 2;
                        fwrite(&op, 2, 1, c->out);
                        break;

                default:
                        puts("compile_text: unreachable");
                        exit(1);
                }

                if (tok_next(c, TOK_SYMBOL) == 1 &&
                                tok_next2(c, TOK_COLON) == 1) {
                        break;
                }

                prev = t;
        }

        return decl;
}

void compile_section(struct compiler *c, struct symbol_array *syms,
                     enum token_kind section)
{
        size_t value = 0;
        enum symbol_type type;

        if (section == TOK_DATA) {
                type = TYPE_VAR;
                fwrite(".data", 1, 5, c->out);
        } else {
                type = TYPE_FUNC;
                fwrite(".text", 1, 5, c->out);
        }

        for (;;) {
                struct token *decl = NULL;
                struct symbol *s;
                size_t size = 0;

                if (section == TOK_DATA) {
                        decl = compile_var_declaration(c, &size);
                } else {
                        decl = compile_func_declaration(c, syms, &size);
                }

                if (decl == NULL) {
                        break;
                }

                s = symbol_get(syms, decl->start, decl->len);
                if (s == NULL) {
                        s = symbol_add(syms, decl->start, decl->len);
                        s->bind = BIND_LOCAL;
                }

                s->value = value;
                s->size = size;
                s->type = type;
                s->isresolved = 1;

                value += size;
        }
}

int main(int argc, char **argv)
{
        struct scanner s;
        struct compiler c;
        struct symbol_array syms;
        struct token_array toks;
        char *outpath;

        argc--;
        argv++;

        if (argc < 1) {
                printf("assembly file is required\n");
                return 1;
        }

        meminit(&toks, sizeof(struct token), 16); // LEAK: os is freeing
        meminit(&syms, sizeof(struct symbol), 16); // LEAK: os is freeing

        outpath = create_outpath(*argv, "o"); // LEAK: os is freeing
        s.src = read_file(*argv); // LEAK: os is freeing
        s.cur = s.src;

        for (;;) {
                struct token *t;

                if (*s.cur == '\0') {
                        memgrow(&toks, sizeof(struct token));
                        t = toks.buf + toks.size;
                        toks.size++;
                        t->kind = TOK_EOF;
                        break;
                }

                s.start = s.cur;

                switch (*s.cur) {
                case ' ':
                case '\t':
                case '\r':
                case '\n':
                        s.cur++;
                        break;

                case ',':
                        s.cur++;

                        memgrow(&toks, sizeof(struct token));
                        t = toks.buf + toks.size;
                        toks.size++;

                        t->kind = TOK_COMMA;
                        t->start = s.start;
                        t->len = s.cur - s.start;
                        break;

                case ':':
                        s.cur++;

                        memgrow(&toks, sizeof(struct token));
                        t = toks.buf + toks.size;
                        toks.size++;

                        t->kind = TOK_COLON;
                        t->start = s.start;
                        t->len = s.cur - s.start;
                        break;

                case ';':
                        while (*s.cur != '\n' && *s.cur != '\0') {
                                s.cur++;
                        }
                        break;

                case '"':
                        s.cur++;
                        while (*s.cur != '"' && *s.cur != '\0') {
                                s.cur++;
                        }
                        if (*s.cur == '\0') {
                                printf("unterminated string literal %.*s\n",
                                                (int) (s.cur - s.start),
                                                s.start);
                                exit(1);
                        }
                        s.cur++;

                        memgrow(&toks, sizeof(struct token));
                        t = toks.buf + toks.size;
                        toks.size++;

                        t->kind = TOK_STR;
                        t->start = s.start + 1;
                        t->len = s.cur-1 - (s.start + 1);
                        break;

                default:
                        if (isdigit(*s.cur) != 0) {
                                char valbuf[6];
                                size_t buflen = sizeof(valbuf) - 1;

                                while (isdigit(*s.cur) != 0) {
                                        s.cur++;
                                }

                                memgrow(&toks, sizeof(struct token));
                                t = toks.buf + toks.size;
                                toks.size++;

                                t->kind = TOK_NUM;
                                t->start = s.start;
                                t->len = s.cur - s.start;

                                if (t->len > buflen) {
                                        t->len = buflen;
                                }

                                // TODO: report overflow error
                                memset(valbuf, 0, buflen + 1);
                                memcpy(valbuf, t->start, t->len);
                                t->value = atoi(valbuf);
                        } else if (ischar(*s.cur) == 1) {
                                size_t oplen;

                                while (ischar(*s.cur) == 1 ||
                                                isdigit(*s.cur) != 0) {
                                        s.cur++;
                                }

                                memgrow(&toks, sizeof(struct token));
                                t = toks.buf + toks.size;
                                toks.size++;

                                t->start = s.start;
                                t->len = s.cur - s.start;
                                t->kind = TOK_SYMBOL;

                                oplen = t->len;
                                if (isbyteop(t) == 1) {
                                        oplen--;
                                }
                                for (size_t i = 0; i < OP_COUNT; ++i) {
                                        char *op = opcodes_str[i];

                                        if (oplen == strlen(op) &&
                                                        memcmp(op,
                                                               t->start,
                                                               oplen) == 0) {
                                                t->kind = TOK_OPCODE;
                                                t->value = i;
                                                break;
                                        }
                                }

                                if (t->kind != TOK_SYMBOL) {
                                        break;
                                }

                                for (size_t i = 0; i < KWD_COUNT; ++i) {
                                        char *kwd = kwds_str[i];

                                        if (t->len == strlen(kwd) &&
                                                        memcmp(kwd,
                                                               t->start,
                                                               t->len) == 0) {
                                                t->kind = i;
                                                break;
                                        }
                                }
                        } else {
                                printf("unknown character %c\n", *s.cur);
                                exit(1);
                        }
                        break;
                }
        }

        c.toks = &toks;
        c.cur = 0;
        c.has_data = 0;
        c.has_text = 0;
        c.out = fopen(outpath, "w");
        if (c.out == NULL) {
                perror("failed to open output file");
                exit(1);
        }

        while (has_toks(&c)) {
                struct token *t = tok_advance(&c);

                switch (t->kind) {
                case TOK_GLOBAL:
                        compile_symbol_list(&c, &syms, BIND_GLOBAL);
                        break;

                case TOK_EXTERN:
                        compile_symbol_list(&c, &syms, BIND_EXTERN);
                        break;

                case TOK_SECTION:
                        t = tok_advance(&c);
                        switch (t->kind) {
                        case TOK_DATA:
                                if (c.has_data == 1) {
                                        puts("data section already defined");
                                        exit(1);
                                }
                                compile_section(&c, &syms, t->kind);
                                c.has_data = 1;
                                break;

                        case TOK_TEXT:
                                if (c.has_text == 1) {
                                        puts("text section already defined");
                                        exit(1);
                                }
                                compile_section(&c, &syms, t->kind);
                                c.has_text = 1;
                                break;

                        default:
                                printf("unknown section name %.*s\n",
                                                (int) t->len, t->start);
                                exit(1);
                        }
                        break;

                default:
                        printf("unexpected token %.*s\n",
                                        (int) t->len, t->start);
                        exit(1);
                }
        }

        fwrite(".symtab", 1, 7, c.out);
        for (size_t i = 0; i < syms.size; ++i) {
                struct symbol *s = syms.buf + i;

                if (s->isresolved == 0) {
                        printf("symbol %.*s not defined\n",
                                        (int) s->len, s->name);
                        if (unlink(outpath) < 0) {
                                perror("unlink failed");
                        }
                        exit(1);
                }

                fwrite(&s->value, 2, 1, c.out);
                fwrite(&s->size, 2, 1, c.out);
                fwrite(&s->tpos, 2, 1, c.out);
                fwrite(&s->type, 1, 1, c.out);
                fwrite(&s->bind, 1, 1, c.out);
                fwrite(&s->len, 2, 1, c.out);
                fwrite(s->name, 1, s->len, c.out);
        }

        fclose(c.out);

        return 0;
}
