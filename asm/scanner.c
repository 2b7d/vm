#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "scanner.h"
#include "../vm.h" // TODO: make path absolute

struct ident {
    char *name;
    enum token_kind kind;
};

struct ident identifiers[IDENT_COUNT] = {
    { .kind = TOK_DB,       .name = "db"      },
    { .kind = TOK_EXTERN,   .name = "extern"  },
    { .kind = TOK_GLOBAL,   .name = "global"  },
    { .kind = TOK_SECTION,  .name = "section" },
    { .kind = TOK_TEXT,     .name = "text"    },
    { .kind = TOK_DATA,     .name = "data"    },
    { .kind = TOK_ST,       .name = "st"      },
    { .kind = TOK_LD,       .name = "ld"      },
    { .kind = TOK_ADD,      .name = "add"     },
    { .kind = TOK_SUB,      .name = "sub"     },
    { .kind = TOK_MUL,      .name = "mul"     },
    { .kind = TOK_DIV,      .name = "div"     },
    { .kind = TOK_MOD,      .name = "mod"     },
    { .kind = TOK_INC,      .name = "inc"     },
    { .kind = TOK_DEC,      .name = "dec"     },
    { .kind = TOK_PUSH,     .name = "push"    },
    { .kind = TOK_DUP,      .name = "dup"     },
    { .kind = TOK_OVER,     .name = "over"    },
    { .kind = TOK_SWAP,     .name = "swap"    },
    { .kind = TOK_DROP,     .name = "drop"    },
    { .kind = TOK_RSPUSH,   .name = "rspush"  },
    { .kind = TOK_RSPOP,    .name = "rspop"   },
    { .kind = TOK_RSCOPY,   .name = "rscopy"  },
    { .kind = TOK_RSDROP,   .name = "rsdrop"  },
    { .kind = TOK_RSP,      .name = "rsp"     },
    { .kind = TOK_RSPSET,   .name = "rspset"  },
    { .kind = TOK_BRK,      .name = "brk"     },
    { .kind = TOK_BRKSET,   .name = "brkset"  },
    { .kind = TOK_EQ,       .name = "eq"      },
    { .kind = TOK_GT,       .name = "gt"      },
    { .kind = TOK_LT,       .name = "lt"      },
    { .kind = TOK_OR,       .name = "or"      },
    { .kind = TOK_AND,      .name = "and"     },
    { .kind = TOK_NOT,      .name = "not"     },
    { .kind = TOK_JMP,      .name = "jmp"     },
    { .kind = TOK_JMPIF,    .name = "jmpif"   },
    { .kind = TOK_HALT,     .name = "halt"    },
    { .kind = TOK_SYSCALL,  .name = "syscall" },
    { .kind = TOK_CALL,     .name = "call"    },
    { .kind = TOK_RET,      .name = "ret"     }
};

static int is_byteop(struct token *t)
{
    return t->start[t->len - 1] == '8';
}

static int find_identifier(struct token *t)
{
    size_t len = t->len;

    if (is_byteop(t) == 1) {
        len--;
    }

    for (int i = 0; i < IDENT_COUNT; ++i) {
        struct ident *id = identifiers + i;

        if (len == strlen(id->name) && memcmp(id->name, t->start, len) == 0) {
            return id->kind;
        }
    }

    return -1;
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
    while (advance(s) != '"' && has_src(s) == 1) {
    }

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
    int kind;

    while (is_alnum(peek(s)) == 1) {
        advance(s);
    }

    make_token(s, t, TOK_SYMBOL);

    kind = find_identifier(t);
    if (kind >= 0) {
        t->kind = kind;
    }
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
