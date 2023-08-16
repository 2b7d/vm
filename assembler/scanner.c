#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../lib/os.h"

#include "scanner.h"

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

static void make_token(struct scanner *s, struct token *t,
                       enum token_kind kind)
{
    t->kind = kind;
    t->lex = s->src + s->pos;
    t->lex_len = s->cur - s->pos;
    t->line = s->line;
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
        tok->line = s->line;
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

    case ':':
        make_token(s, tok, TOK_COLON);
        advance(s);
        break;

    default:
        if (is_char(s->ch) == 1) {
            while (is_alnum(s->ch) == 1) {
                advance(s);
            }
            make_token(s, tok, TOK_SYMBOL);
        } else if (is_digit(s->ch) == 1) {
            while (is_digit(s->ch) == 1) {
                advance(s);
            }
            make_token(s, tok, TOK_NUM);
        } else {
scan_error:
            fprintf(stderr, "%s:%d: unexpected character %c\n", s->filepath, s->line, s->ch);
            exit(1);
        }
    }
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

void undo_scan(struct scanner *s)
{
    s->cur = s->pos;
    s->ch = s->src[s->cur];
}

char *tok_to_str(enum token_kind kind)
{
    switch (kind) {
    case TOK_SYMBOL:
        return "symbol";
    case TOK_NUM:
        return "number";

    case TOK_COLON:
        return ":";

    case TOK_EOF:
        return "<end of file>";
    case TOK_ERR:
        return "<error>";
    default:
        assert(0 && "unreachable");
    }
}
