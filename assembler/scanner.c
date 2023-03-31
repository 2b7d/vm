#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "scanner.h"
#include "../vm.h" // TODO: make path absolute

static char *opcodes_str[OP_COUNT] = {
    "st",    "ld",      "add",    "sub",  "mul",
    "div",   "mod",     "inc",    "dec",  "push",
    "dup",   "over",    "swap",   "drop", "rspush",
    "rspop", "rscopy",  "rsdrop", "rsp",  "rspset",
    "brk",   "brkset",  "eq",     "gt",   "lt",
    "or",    "and",     "not",    "jmp",  "jmpif",
    "halt",  "syscall", "call",   "ret"
};

static int check_opcode(struct token *t)
{
    unsigned long len = t->len;

    for (int i = 0; i < OP_COUNT; ++i) {
        char *op = opcodes_str[i];

        if (len == strlen(op) && memcmp(op, t->start, len) == 0) {
            t->kind = TOK_OPCODE;
            return 1;
        }
    }

    return 0;
}

static int is_space(char c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

static int is_digit(char c)
{
    return c >= '0' && c <= '9';
}

static int is_lower(char c)
{
    return c >= 'a' && c <= 'z';
}

static int is_upper(char c)
{
    return c >= 'A' && c <= 'Z';
}

static int is_char(char c)
{
    return is_lower(c) == 1 || is_upper(c) == 1 || c == '_' || c == '.';
}

static char peek(struct scanner *s)
{
    return *s->cur;
}

static int has_src(struct scanner *s)
{
    return peek(s) != '\0';
}

static char advance(struct scanner *s)
{
    char c = peek(s);

    if (has_src(s) == 1) {
        s->cur++;
    }

    return c;
}

static void skip_whitespace(struct scanner *s)
{
    while (is_space(peek(s)) == 1) {
        advance(s);
    }
}

static void skip_comment(struct scanner *s)
{
    while (advance(s) != '\n' && has_src(s) == 1);
}

static void make_token(struct scanner *s, struct token *t, enum token_kind k)
{
    t->kind = k;
    t->start = s->start;
    t->len = s->cur - s->start;
}

static void scan_string(struct scanner *s, struct token *t)
{
    while (advance(s) != '"' && has_src(s) == 1);

    if (has_src(s) == 0) {
        fprintf(stderr, "unterminated string literal");
        exit(1);
    }

    make_token(s, t, TOK_STR);
}

static void scan_directive(struct scanner *s, struct token *t)
{
    while (is_space(peek(s)) == 0) {
        advance(s);
    }

    make_token(s, t, TOK_EOF);

    if (t->len == 6 && memcmp(".bytes", t->start, t->len) == 0) {
        t->kind = TOK_BYTES;
    } else if (t->len == 7 && memcmp(".global", t->start, t->len) == 0) {
        t->kind = TOK_GLOBAL;
    } else if (t->len == 7 && memcmp(".extern", t->start, t->len) == 0) {
        t->kind = TOK_EXTERN;
    } else {
        fprintf(stderr, "unknown directive\n");
        exit(1);
    }
}

static void scan_number(struct scanner *s, struct token *t)
{
    while (is_digit(peek(s)) == 1) {
        advance(s);
    }

    make_token(s, t, TOK_NUM);
}

static void scan_opcode(struct scanner *s, struct token *t)
{
    while (is_lower(peek(s)) == 1) {
        advance(s);
    }

    make_token(s, t, TOK_OPCODE);

    if (check_opcode(t) == 0) {
        t->kind = TOK_SYMBOL;
    }

    if (peek(s) == '8') {
        advance(s);
        t->len++;
        if (t->kind == TOK_OPCODE) {
            t->kind = TOK_OPCODE_BYTE;
        }
    }
}

static void scan_symbol(struct scanner *s, struct token *t)
{
    while (is_char(peek(s)) == 1 || is_digit(peek(s)) == 1) {
        advance(s);
    }

    make_token(s, t, TOK_SYMBOL);
}

void scan_token(struct scanner *s, struct token *t)
{
    char c;

scan_again:
    s->start = s->cur;
    c = advance(s);

    switch (c) {
    case '\0':
        t->kind = TOK_EOF;
        return;

    case ' ':
    case '\t':
    case '\r':
    case '\n':
        skip_whitespace(s);
        goto scan_again;

    case ';':
        skip_comment(s);
        goto scan_again;

    case ':':
        make_token(s, t, TOK_COLON);
        return;

    case ',':
        make_token(s, t, TOK_COMMA);
        return;

    case '.':
        scan_directive(s, t);
        return;

    case '"':
        scan_string(s, t);
        return;

    default:
        if (is_digit(c) == 1) {
            scan_number(s, t);
            return;
        } else if (is_lower(c) == 1) {
            scan_opcode(s, t);
            return;
        } else if (is_char(c) == 1) {
            scan_symbol(s, t);
            return;
        }

        fprintf(stderr, "unknown character %c\n", c);
        exit(1);
    }
}

void scanner_init(struct scanner *s, char *src)
{
    s->src = src;
    s->cur = s->src;
    s->start = s->cur;
}
