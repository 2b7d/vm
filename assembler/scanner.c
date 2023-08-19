#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../lib/mem.h"
#include "../lib/os.h"

#include "scanner.h"

struct kwd_entry {
    char *str;
    int str_len;
    enum token_kind tok;
};

static struct kwd_entry keywords[] = {
    { .str = "halt",    .str_len = 4, .tok = TOK_HALT },
    { .str = "push",    .str_len = 4, .tok = TOK_PUSH },
    { .str = "pushb",   .str_len = 5, .tok = TOK_PUSHB },
    { .str = "drop",    .str_len = 4, .tok = TOK_DROP },
    { .str = "dropb",   .str_len = 5, .tok = TOK_DROPB },
    { .str = "ld",      .str_len = 2, .tok = TOK_LD },
    { .str = "ldb",     .str_len = 3, .tok = TOK_LDB },
    { .str = "st",      .str_len = 2, .tok = TOK_ST },
    { .str = "stb",     .str_len = 3, .tok = TOK_STB },
    { .str = "ctw",     .str_len = 3, .tok = TOK_CTW },
    { .str = "ctb",     .str_len = 3, .tok = TOK_CTB },
    { .str = "add",     .str_len = 3, .tok = TOK_ADD },
    { .str = "addb",    .str_len = 4, .tok = TOK_ADDB },
    { .str = "sub",     .str_len = 3, .tok = TOK_SUB },
    { .str = "subb",    .str_len = 4, .tok = TOK_SUBB },
    { .str = "neg",     .str_len = 3, .tok = TOK_NEG },
    { .str = "negb",    .str_len = 4, .tok = TOK_NEGB },
    { .str = "eq",      .str_len = 2, .tok = TOK_EQ },
    { .str = "eqb",     .str_len = 3, .tok = TOK_EQB },
    { .str = "lt",      .str_len = 2, .tok = TOK_LT },
    { .str = "ltb",     .str_len = 3, .tok = TOK_LTB },
    { .str = "gt",      .str_len = 2, .tok = TOK_GT },
    { .str = "gtb",     .str_len = 3, .tok = TOK_GTB },
    { .str = "jmp",     .str_len = 3, .tok = TOK_JMP },
    { .str = "cjmp",    .str_len = 4, .tok = TOK_CJMP },
    { .str = "call",    .str_len = 4, .tok = TOK_CALL },
    { .str = "ret",     .str_len = 3, .tok = TOK_RET },
    { .str = "syscall", .str_len = 7, .tok = TOK_SYSCALL },

    { .str = "byte", .str_len = 4, .tok = TOK_BYTE },
    { .str = "word", .str_len = 4, .tok = TOK_WORD },

    { .str = NULL, .str_len = 0, .tok = 0 } // art: end of array
};

static enum token_kind lookup_keyword(char *sym, int sym_len)
{
    for (struct kwd_entry *e = keywords; e->str != NULL; ++e) {
        if (e->str_len == sym_len && memcmp(e->str, sym, sym_len) == 0) {
            return e->tok;
        }
    }

    return TOK_SYM;
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

static void scan(struct scanner *s, struct token *tok)
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

    case '.':
        advance(s);
        make_token(s, tok, TOK_DOT);
        break;
    case ',':
        advance(s);
        make_token(s, tok, TOK_COMMA);
        break;
    case ':':
        advance(s);
        make_token(s, tok, TOK_COLON);
        break;

    case '"':
        advance(s);
        while (s->ch != '"' && s->ch != '\0') {
            advance(s);
        }
        if (s->ch == '\0') {
            fprintf(stderr, "%s:%d: unterminated string\n", s->filepath, s->line);
            exit(1);
        }
        make_token(s, tok, TOK_STR);
        tok->lex++;
        tok->lex_len--;
        advance(s);
        break;

    default:
        if (is_char(s->ch) == 1) {
            while (is_alnum(s->ch) == 1) {
                advance(s);
            }
            make_token(s, tok, TOK_ERR);
            tok->kind = lookup_keyword(tok->lex, tok->lex_len);
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

void scan_tokens(struct scanner *s, struct tokens *toks)
{
    struct token *t;

    for (;;) {
        t = memnext(toks);
        scan(s, t);

        if (t->kind == TOK_EOF) {
            return;
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

char *tok_to_str(enum token_kind kind)
{
    switch (kind) {
    case TOK_ERR:
		return "<error>";

    case TOK_NUM:
		return "number";
    case TOK_SYM:
		return "symbol";
    case TOK_STR:
		return "string";

    case TOK_DOT:
		return ".";
    case TOK_COMMA:
		return ",";
    case TOK_COLON:
		return ":";

    case TOK_HALT:
		return "halt";
    case TOK_PUSH:
		return "push";
    case TOK_PUSHB:
		return "pushb";
    case TOK_DROP:
		return "drop";
    case TOK_DROPB:
		return "dropb";
    case TOK_LD:
		return "ld";
    case TOK_LDB:
		return "ldb";
    case TOK_ST:
		return "st";
    case TOK_STB:
		return "stb";
    case TOK_CTW:
		return "ctw";
    case TOK_CTB:
		return "ctb";
    case TOK_ADD:
		return "add";
    case TOK_ADDB:
		return "addb";
    case TOK_SUB:
		return "sub";
    case TOK_SUBB:
		return "subb";
    case TOK_NEG:
		return "neg";
    case TOK_NEGB:
		return "negb";
    case TOK_EQ:
		return "eq";
    case TOK_EQB:
		return "eqb";
    case TOK_LT:
		return "lt";
    case TOK_LTB:
		return "ltb";
    case TOK_GT:
		return "gt";
    case TOK_GTB:
		return "gtb";
    case TOK_JMP:
		return "jmp";
    case TOK_CJMP:
		return "cjmp";
    case TOK_CALL:
		return "call";
    case TOK_RET:
		return "ret";
    case TOK_SYSCALL:
		return "syscall";

    case TOK_BYTE:
		return "byte";
    case TOK_WORD:
		return "word";

    case TOK_EOF:
		return "<end of file>";

    case TOK_mnemonic_start:
    case TOK_mnemonic_end:
    default:
        assert(0 && "unreachable");
    }
}

int is_mnemonic(enum token_kind kind)
{
    return kind > TOK_mnemonic_start && kind < TOK_mnemonic_end;
}
