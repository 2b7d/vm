#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../lib/os.h"

#include "scanner.h"

struct kwd_entry {
    char *str;
    int str_len;
    enum token_kind tok;
};

static struct kwd_entry keywords[] = {
    { .str = "halt",  .str_len = 4, .tok = TOK_HALT },

    { .str = "push",  .str_len = 4, .tok = TOK_PUSH },
    { .str = "pushb", .str_len = 5, .tok = TOK_PUSHB },

    { .str = "ctw",   .str_len = 3, .tok = TOK_CTW },
    { .str = "ctb",   .str_len = 3, .tok = TOK_CTB },

    { .str = "add",   .str_len = 3, .tok = TOK_ADD },
    { .str = "addb",  .str_len = 4, .tok = TOK_ADDB },
    { .str = "sub",   .str_len = 3, .tok = TOK_SUB },
    { .str = "subb",  .str_len = 4, .tok = TOK_SUBB },
    { .str = "neg",   .str_len = 3, .tok = TOK_NEG },
    { .str = "negb",  .str_len = 4, .tok = TOK_NEGB },

    { .str = "eq",    .str_len = 2, .tok = TOK_EQ },
    { .str = "eqb",   .str_len = 3, .tok = TOK_EQB },
    { .str = "lt",    .str_len = 2, .tok = TOK_LT },
    { .str = "ltb",   .str_len = 3, .tok = TOK_LTB },
    { .str = "gt",    .str_len = 2, .tok = TOK_GT },
    { .str = "gtb",   .str_len = 3, .tok = TOK_GTB },

    { .str = "jmp",   .str_len = 3, .tok = TOK_JMP },
    { .str = "cjmp",  .str_len = 4, .tok = TOK_CJMP },

    { .str = "",      .str_len = 0, .tok = TOK_ERR } // art: end of array
};

static enum token_kind lookupKeyword(struct scanner *s)
{
    char *ident;
    int len;

    ident = s->src + s->pos;
    len = s->cur - s->pos;

    for (struct kwd_entry *kwd = keywords; kwd->tok != TOK_ERR; ++kwd) {
        if (len == kwd->str_len && memcmp(ident, kwd->str, len) == 0) {
            return kwd->tok;
        }
    }

    return TOK_IDENT;
}

void make_scanner(struct scanner *s, char *filepath)
{
    s->src_len = read_file(filepath, &s->src);
    if (s->src_len < 0) {
        perror(NULL);
        exit(1);
    }

    s->filepath = filepath;
    s->cur = 0;
    s->pos = 0;
    s->line = 1;

    s->ch = s->src[0];
}

static void advance(struct scanner *s)
{
    if (s->ch == '\0') {
        return;
    }

    s->cur++;
    s->ch = s->src[s->cur];
}

static int next(struct scanner *s, char ch)
{
    int next;

    next = s->cur + 1;
    return next < s->src_len && s->src[next] == ch;
}

static char *get_lexeme(struct scanner *s)
{
    int len;
    char *lex;

    len = s->cur - s->pos;
    lex = malloc(len + 1);
    if (lex == NULL) {
        perror("get_lexeme");
        exit(1);
    }
    memcpy(lex, s->src + s->pos, len);
    lex[len] = '\0';

    return lex;
}

static int is_digit(char ch)
{
    return ch >= '0' && ch <= '9';
}

static int is_char(char ch)
{
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_';
}

static int is_alnum(char ch)
{
    return is_char(ch) == 1 || is_digit(ch) == 1;
}

void scan_token(struct scanner *s, struct token *tok)
{
scan_again:
    if (s->ch == '\0') {
        tok->kind = TOK_EOF;
        return;
    }

    s->pos = s->cur;

    switch (s->ch) {
    case ' ':
    case '\t':
    case '\r':
    case '\n':
        if (s->ch == '\n') {
            s->line++;
        }
        advance(s);
        goto scan_again;

    case '/':
        if (next(s, '/') == 1) {
            while (s->ch != '\n') {
                advance(s);
            }
            goto scan_again;
        }
        goto scan_error;

    default:
        if (is_char(s->ch) == 1) {
            while (is_alnum(s->ch) == 1) {
                advance(s);
            }

            tok->kind = lookupKeyword(s);
            if (tok->kind == TOK_IDENT) {
                tok->lex = get_lexeme(s);
            }
        } else if (is_digit(s->ch) == 1) {
            while (is_digit(s->ch) == 1) {
                advance(s);
            }

            tok->kind = TOK_NUM;
            tok->lex = get_lexeme(s);
        } else {
scan_error:
            fprintf(stderr, "%s:%d unexpected characted: %c\n", s->filepath, s->line, s->ch);
            exit(1);
        }
    }
}
