#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "scanner.h"
#include "../vm.h" // TODO: make path absolute

static char *keywords_str[KWD_COUNT] = {
    [TOK_DB]      = "db",
    [TOK_EXTERN]  = "extern",
    [TOK_GLOBAL]  = "global",
    [TOK_SECTION] = "section",
    [TOK_TEXT]    = "text",
    [TOK_DATA]    = "data"
};

static char *opcodes_str[OP_COUNT] = {
    [OP_ST]      = "st",
    [OP_LD]      = "ld",
    [OP_ADD]     = "add",
    [OP_SUB]     = "sub",
    [OP_MUL]     = "mul",
    [OP_DIV]     = "div",
    [OP_MOD]     = "mod",
    [OP_INC]     = "inc",
    [OP_DEC]     = "dec",
    [OP_PUSH]    = "push",
    [OP_DUP]     = "dup",
    [OP_OVER]    = "over",
    [OP_SWAP]    = "swap",
    [OP_DROP]    = "drop",
    [OP_RSPUSH]  = "rspush",
    [OP_RSPOP]   = "rspop",
    [OP_RSCOPY]  = "rscopy",
    [OP_RSDROP]  = "rsdrop",
    [OP_RSP]     = "rsp",
    [OP_RSPSET]  = "rspset",
    [OP_BRK]     = "brk",
    [OP_BRKSET]  = "brkset",
    [OP_EQ]      = "eq",
    [OP_GT]      = "gt",
    [OP_LT]      = "lt",
    [OP_OR]      = "or",
    [OP_AND]     = "and",
    [OP_NOT]     = "not",
    [OP_JMP]     = "jmp",
    [OP_JMPIF]   = "jmpif",
    [OP_HALT]    = "halt",
    [OP_SYSCALL] = "syscall",
    [OP_CALL]    = "call",
    [OP_RET]     = "ret"
};

static int is_byteop(struct token *t)
{
    return t->start[t->len - 1] == '8';
}

static int check_opcode(struct token *t)
{
    size_t len = t->len;

    if (is_byteop(t) == 1) {
        len--;
        t->is_byteop = 1;
    }

    for (int i = 0; i < OP_COUNT; ++i) {
        char *op = opcodes_str[i];

        if (len == strlen(op) && memcmp(op, t->start, len) == 0) {
            t->kind = TOK_OPCODE;
            t->opcode = i;
            return 1;
        }
    }

    return 0;
}

static int check_keyword(struct token *t)
{
    for (int i = 0; i < KWD_COUNT; ++i) {
        char *kwd = keywords_str[i];

        if (t->len == strlen(kwd) && memcmp(kwd, t->start, t->len) == 0) {
            t->kind = i;
            return 1;
        }
    }

    return 0;
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

static int is_space(char c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

static int is_digit(char c)
{
    return c >= '0' && c <= '9';
}

static int is_alpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static int is_alnum(char c)
{
    return is_alpha(c) == 1 || is_digit(c) == 1 || c == '_' || c == '.';
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
    advance(s);
}

static void scan_number(struct scanner *s, struct token *t)
{
    while (is_digit(peek(s)) == 1) {
        advance(s);
    }

    make_token(s, t, TOK_NUM);
}

static void scan_identifier(struct scanner *s, struct token *t)
{
    while (is_alnum(peek(s)) == 1) {
        advance(s);
    }

    make_token(s, t, TOK_SYMBOL);

    if (check_opcode(t) == 1) {
        return;
    }

    check_keyword(t);
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

    case '"':
        scan_string(s, t);
        break;

    default:
        if (is_digit(c) == 1) {
            scan_number(s, t);
            return;
        } else if (is_alnum(c) == 1) {
            scan_identifier(s, t);
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
