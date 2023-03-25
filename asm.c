#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "vm.h"
#include "util.h"
#include "mem.h" // lib

struct scanner {
    char *cur;
    char *start;
    char *src;
};

enum token_kind {
    TOK_NUM,
    TOK_STR,
    TOK_OPCODE,
    TOK_KWD,
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

enum kwd_kind {
    KWD_DB,
    KWD_EXTERN,
    KWD_GLOBAL,
    KWD_SECTION,
    KWD_TEXT,
    KWD_DATA,
    KWD_COUNT
};

char *kwds_str[KWD_COUNT] = {
    [KWD_DB]      = "db",
    [KWD_EXTERN]  = "extern",
    [KWD_GLOBAL]  = "global",
    [KWD_SECTION] = "section",
    [KWD_TEXT]    = "text",
    [KWD_DATA]    = "data"
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

int main(int argc, char **argv)
{
    struct scanner s;
    struct token_array toks;
    //char *outpath;

    argc--;
    argv++;

    if (argc < 1) {
        printf("assembly file is required\n");
        return 1;
    }

    meminit(&toks, sizeof(struct token), 16); // LEAK: os is freeing

    //outpath = create_outpath(*argv, "o"); // LEAK: os is freeing
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

                while (isdigit(*s.cur) != 0) {
                    s.cur++;
                }

                memgrow(&toks, sizeof(struct token));
                t = toks.buf + toks.size;
                toks.size++;

                t->kind = TOK_NUM;
                t->start = s.start;
                t->len = s.cur - s.start;

                if (t->len > sizeof(valbuf) - 1) {
                    printf("number %.*s is greater than 16bit\n", (int) t->len,
                                                                  t->start);
                    exit(1);
                }

                memset(valbuf, 0, sizeof(valbuf));
                memcpy(valbuf, t->start, t->len);
                t->value = atoi(valbuf);
                if (t->value > (1<<16)) {
                    printf("number %.*s is greater than 16bit\n", (int) t->len,
                                                                  t->start);
                    exit(1);
                }
            } else if (ischar(*s.cur) == 1) {
                while (ischar(*s.cur) == 1 || isdigit(*s.cur) != 0) {
                    s.cur++;
                }

                memgrow(&toks, sizeof(struct token));
                t = toks.buf + toks.size;
                toks.size++;

                t->start = s.start;
                t->len = s.cur - s.start;
                t->kind = TOK_SYMBOL;

                for (size_t i = 0; i < OP_COUNT; ++i) {
                    char *op = opcodes_str[i];

                    if (t->len == strlen(op) &&
                            memcmp(op, t->start, t->len) == 0) {
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
                            memcmp(kwd, t->start, t->len) == 0) {
                        t->kind = TOK_KWD;
                        t->value = i;
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

    for (size_t i = 0; i < toks.size; ++i) {
        struct token *t = toks.buf + i;

        printf("kind: %d - lexeme: %.*s - value: %d\n", t->kind, (int) t->len, t->start, t->value);
    }

    return 0;
}
