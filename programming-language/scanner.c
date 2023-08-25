#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../lib/sstring.h"
#include "../lib/mem.h"
#include "../lib/os.h"

#include "scanner.h"

typedef struct {
    string s;
    Token_Kind kind;
} Kwd_Entry;

static Kwd_Entry keywords[] = {
    {.s = {.ptr = "let",    .len = 3}, .kind = TOK_LET},
    {.s = {.ptr = "proc",   .len = 4}, .kind = TOK_PROC},
    {.s = {.ptr = "return", .len = 6}, .kind = TOK_RET},

    {.s = {.ptr = "", .len = 0}, .kind = TOK_ERR} // art: end of array
};

static void make_lexeme(Scanner *s, string *buf)
{
    string_make(buf, s->src + s->pos, s->cur - s->pos);
}

static Token_Kind lookup_keyword(Scanner *s)
{
    string ident;

    make_lexeme(s, &ident);

    for (Kwd_Entry *e = keywords; e->kind != TOK_ERR; ++e) {
        if (string_cmp(&ident, &e->s) == 1) {
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

static void make_token(Scanner *s, Token *tok, Token_Kind kind)
{
    tok->kind = kind;
    tok->line = s->line;
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
    if (s->ch == '\0') {
        make_token(s, tok, TOK_EOF);
        return;
    }

    s->pos = s->cur;

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
        make_token(s, tok, TOK_EQ);
        advance(s);
        break;

    case '+':
        make_token(s, tok, TOK_PLUS);
        advance(s);
        break;
    case '-':
        make_token(s, tok, TOK_MINUS);
        advance(s);
        break;

    case ';':
        make_token(s, tok, TOK_SEMICOLON);
        advance(s);
        break;
    case '(':
        make_token(s, tok, TOK_LPAREN);
        advance(s);
        break;
    case ')':
        make_token(s, tok, TOK_RPAREN);
        advance(s);
        break;
    case '{':
        make_token(s, tok, TOK_LCURLY);
        advance(s);
        break;
    case '}':
        make_token(s, tok, TOK_RCULRY);
        advance(s);
        break;

    default:
        if (is_char(s->ch) == 1) {
            Token_Kind kind;

            while (is_alnum(s->ch) == 1) {
                advance(s);
            }

            kind = lookup_keyword(s);
            make_token(s, tok, kind);
            if (kind == TOK_IDENT) {
                make_lexeme(s, &tok->lex);
            }
        } else if (is_digit(s->ch) == 1) {
            while (is_digit(s->ch) == 1) {
                advance(s);
            }
            make_token(s, tok, TOK_NUM);
            make_lexeme(s, &tok->lex);
        } else {
            fprintf(stderr, "%s:%d: unexpected character %c\n", s->filepath, s->line, s->ch);
            exit(1);
        }
    };
}

void scan_tokens(Scanner *s, Tokens *toks)
{
    Token *tok;

    for (;;) {
        tok = memnext(toks);
        memset(tok, 0, sizeof(Token));

        scan(s, tok);

        if (tok->kind == TOK_EOF) {
            break;
        }
    }
}

void make_scanner(Scanner *s, char *filepath)
{
    if (read_file(filepath, &s->src) < 0) {
        perror(NULL);
        exit(1);
    }

    s->filepath = filepath;
    s->cur = 0;
    s->pos = 0;
    s->line = 1;
    s->ch = s->src[0];
}

char *tokstr(Token_Kind kind)
{
    switch (kind) {
    case TOK_ERR:
		return "<error>";

    case TOK_IDENT:
		return "identifier";
    case TOK_NUM:
		return "number";

    case TOK_EQ:
		return "eq";

    case TOK_PLUS:
		return "+";
    case TOK_MINUS:
		return "-";

    case TOK_SEMICOLON:
		return ";";

    case TOK_LPAREN:
		return "(";
    case TOK_RPAREN:
		return ")";
    case TOK_LCURLY:
		return "{";
    case TOK_RCULRY:
		return "}";

    case TOK_LET:
		return "let";
    case TOK_PROC:
		return "proc";
    case TOK_RET:
		return "return";

    case TOK_EOF:
		return "<end of file>";
    default:
        assert(0 && "unreachable");
    }
}
