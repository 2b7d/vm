#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../lib/sstring.h"
#include "../lib/mem.h"
#include "../lib/os.h"

#include "vmc.h"

typedef struct {
    char *str;
    int len;
    Token_Kind kind;
} Kwd_Entry;

static Kwd_Entry keywords[] = {
    {.str = "var",    .len = 3, .kind = TOK_VAR},
    {.str = "proc",   .len = 4, .kind = TOK_PROC},
    {.str = "return", .len = 6, .kind = TOK_RET},
    {.str = "extern", .len = 6, .kind = TOK_EXTERN},
    {.str = "global", .len = 6, .kind = TOK_GLOBAL},
    {.str = "void",   .len = 4, .kind = TOK_VOID},
    {.str = "u16",    .len = 3, .kind = TOK_U16},
    {.str = "u8",     .len = 2, .kind = TOK_U8},


    {.str = NULL, .len = 0, .kind = 0} // art: end of array
};

static Token_Kind lookup_keyword(Scanner *s)
{
    char *str = s->src + s->start;
    int len = s->cur - s->start;

    for (Kwd_Entry *e = keywords; e->str != NULL; ++e) {
        if (e->len == len && memcmp(e->str, str, len) == 0) {
            return e->kind;
        }
    }

    return TOK_IDENT;
}

static void advance(Scanner *s)
{
    if (s->ch == '\0') {
        return;
    }

    s->cur++;
    s->ch = s->src[s->cur];
}

static int next(Scanner *s, char c)
{
    if (s->ch == '\0') {
        return 0;
    }

    return s->src[s->cur + 1] == c;
}

static void make_pos(Scanner *s, Token *tok)
{
    tok->pos.file = s->file;
    tok->pos.line = s->line;
}

static void make_lex(Scanner *s, Token *tok)
{
    string_make(&tok->lex, s->src + s->start, s->cur - s->start);
}

static void make_num(Token *tok)
{
    char *lex = string_toc(&tok->lex);

    // TODO(art): handle convert errors
    tok->value.as_num = atoi(lex);
    free(lex);
}

static void make_tok(Scanner *s, Token *tok, Token_Kind kind)
{
    tok->kind = kind;
    make_pos(s, tok);
    make_lex(s, tok);

    if (tok->kind == TOK_NUM) {
        make_num(tok);
    }
}

static int is_char(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static int is_digit(char c)
{
    return c >= '0' && c <= '9';
}

static int is_alnum(char c)
{
    return is_char(c) == 1 || is_digit(c) == 1;
}

static void scan(Scanner *s, Token *tok)
{
scan_again:
    s->start = s->cur;

    if (s->ch == '\0') {
        make_tok(s, tok, TOK_EOF);
        return;
    }

    switch (s->ch) {
    case ' ':
    case '\t':
    case '\n':
    case '\r':
        if (s->ch == '\n') {
            s->line++;
        }
        advance(s);
        goto scan_again;

    case '=':
        advance(s);
        make_tok(s, tok, TOK_EQ);
        break;

    case '+':
        advance(s);
        make_tok(s, tok, TOK_PLUS);
        break;
    case '-':
        advance(s);
        make_tok(s, tok, TOK_MINUS);
        break;
    case '/':
        if (next(s, '/') == 1) {
            while (s->ch != '\n' && s->ch != '\0') {
                advance(s);
            }
            goto scan_again;
        }
        if (next(s, '*') == 1) {
            advance(s);
            advance(s);
            for (;;) {
                if (s->ch == '*' && next(s, '/') == 1) {
                    advance(s);
                    advance(s);
                    break;
                }
                if (s->ch == '\0') {
                    fprintf(stderr, "%s:%d: unterminated multiline comment\n", s->file, s->line);
                    exit(1);
                }
                advance(s);
            }
            goto scan_again;
        }
        fprintf(stderr, "%s:%d: unexpected character %c\n", s->file, s->line, s->ch);
        exit(1);

    case ',':
        advance(s);
        make_tok(s, tok, TOK_COMMA);
        break;
    case ':':
        advance(s);
        make_tok(s, tok, TOK_COLON);
        break;
    case ';':
        advance(s);
        make_tok(s, tok, TOK_SEMICOLON);
        break;

    case '(':
        advance(s);
        make_tok(s, tok, TOK_LPAREN);
        break;
    case ')':
        advance(s);
        make_tok(s, tok, TOK_RPAREN);
        break;
    case '{':
        advance(s);
        make_tok(s, tok, TOK_LCURLY);
        break;
    case '}':
        advance(s);
        make_tok(s, tok, TOK_RCULRY);
        break;

    default:
        if (is_char(s->ch) == 1) {
            Token_Kind kind;

            while (is_alnum(s->ch) == 1) {
                advance(s);
            }

            kind = lookup_keyword(s);
            make_tok(s, tok, kind);
        } else if (is_digit(s->ch) == 1) {
            while (is_digit(s->ch) == 1) {
                advance(s);
            }
            make_tok(s, tok, TOK_NUM);
        } else {
            fprintf(stderr, "%s:%d: unexpected character %c\n", s->file, s->line, s->ch);
            exit(1);
        }
    };
}

void scanner_scan(Scanner *s, Tokens *toks)
{
    for (;;) {
        Token *tok;

        mem_grow(toks);
        tok = mem_next(toks);

        scan(s, tok);

        if (tok->kind == TOK_EOF) {
            break;
        }
    }
}

void scanner_make(Scanner *s, char *file)
{
    if (os_read_file(file, &s->src) < 0) {
        perror(NULL);
        exit(1);
    }

    s->file = file;
    s->cur = 0;
    s->start = 0;
    s->line = 1;
    s->ch = s->src[0];
}
