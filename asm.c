#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#include "vm.h"
#include "util.h"
#include "mem.h" // lib

struct scanner {
    char *src;
    char *cur;
};

enum token_kind {
    TOKEN_OPCODE,
    TOKEN_IMMEDIATE,
    TOKEN_LABEL
};

struct token {
    enum vm_opcode opcode;
    enum token_kind kind;
    uint16_t value;
    char *lex;
    size_t lexlen;
};

struct token_array {
    size_t size;
    size_t cap;
    struct token *buf;
};

struct label {
    uint16_t addr;
    char *start;
    size_t len;
};

struct label_array {
    size_t size;
    size_t cap;
    struct label *buf;
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

void scan(struct scanner *s, struct token_array *tokens,
          struct label_array *labels)
{
    while (*s->cur != '\0') {
        char *start = s->cur;

        switch (*s->cur) {
        case ' ':
        case '\t':
        case '\r':
        case '\n':
            s->cur++;
            break;

        case ';':
            while (*s->cur != '\n' && *s->cur != '\0') {
                s->cur++;
            }
            break;

        case '"':
            s->cur++;
            while (*s->cur != '"' && *s->cur != '\0') {
                s->cur++;
            }

            if (*s->cur == '\0') {
                printf("unterminated string %.5s...\n", start);
                exit(1);
            }

            for (int i = 1; i < s->cur - start; ++i) {
                struct token *t;
                char val = start[i];

                memgrow(tokens, sizeof(struct token));
                t = tokens->buf + tokens->size;
                tokens->size++;

                t->kind = TOKEN_IMMEDIATE;
                t->value = val;
            }

            s->cur++;
            break;

        default:
            if (isalpha(*s->cur) != 0 || *s->cur == '_') {
                struct token *t;
                size_t lexlen;

                while (isalnum(*s->cur) != 0 || *s->cur == '_') {
                    s->cur++;
                }

                lexlen = s->cur - start;

                if (*s->cur == ':') {
                    struct label *l;

                    for (size_t i = 0; i < labels->size; ++i) {
                        l = labels->buf + i;

                        if (lexlen == l->len &&
                                memcmp(start, l->start, l->len) == 0) {
                            printf("label %.*s already declared\n",
                                    (int) l->len, l->start);
                            exit(1);
                        }
                    }

                    memgrow(labels, sizeof(struct label));
                    l = labels->buf + labels->size;
                    labels->size++;

                    l->addr = tokens->size;
                    l->start = start;
                    l->len = lexlen;
                    s->cur++;
                    break;
                }

                memgrow(tokens, sizeof(struct token));
                t = tokens->buf + tokens->size;
                tokens->size++;

                t->kind = TOKEN_LABEL;
                t->lex = start;
                t->lexlen = lexlen;

                for (size_t i = 0; i < OP_COUNT; ++i) {
                    char *op = opcodes_str[i];

                    if (lexlen == strlen(op) &&
                            memcmp(start, op, lexlen) == 0) {
                        t->kind = TOKEN_OPCODE;
                        t->opcode = i;
                        break;
                    }
                }
                break;
            }

            if (isdigit(*s->cur) != 0) {
                struct token *t;
                char valbuf[6];
                size_t lexlen, val;

                memgrow(tokens, sizeof(struct token));
                t = tokens->buf + tokens->size;
                tokens->size++;

                while (isdigit(*s->cur) != 0) {
                    s->cur++;
                }

                lexlen = s->cur - start;

                if (lexlen >= sizeof(valbuf))  {
                    printf("value %.*s is greater than 16bit\n", (int) lexlen,
                                                                 start);
                    exit(1);
                }

                memset(valbuf, 0, sizeof(valbuf));
                memcpy(valbuf, start, lexlen);

                val = atoi(valbuf);
                if (val >= 1 << 16) {
                    printf("value %lu is greater than 16bit\n", val);
                    exit(1);
                }

                t->kind = TOKEN_IMMEDIATE;
                t->value = val;
                break;
            }

            printf("unknown character `%c`\n", *s->cur);
            exit(1);
        }
    }
}

void resolve_labels(struct token_array *tokens, struct label_array *labels)
{
    for (size_t i = 0; i < tokens->size; ++i) {
        int label_found = 0;
        struct token *t = tokens->buf + i;

        if (t->kind != TOKEN_LABEL) {
            continue;
        }

        for (size_t j = 0; j < labels->size; ++j) {
            struct label *l = labels->buf + j;

            if (t->lexlen == l->len && memcmp(t->lex, l->start, l->len) == 0) {
                label_found = 1;
                t->value = l->addr;
                break;
            }
        }

        if (label_found == 0) {
            printf("undefined label %.*s\n", (int) t->lexlen, t->lex);
            exit(1);
        }
    }
}

void write_opcodes(char *pathname, struct token_array *tokens,
                   struct label_array *labels)
{
    FILE *f;
    uint16_t main_addr;
    char *main = "_start";
    int main_found = 0;

    for (size_t i = 0; i < labels->size; ++i) {
        struct label *l = labels->buf + i;

        if (l->len == strlen(main) && memcmp(main, l->start, l->len) == 0) {
            main_found = 1;
            main_addr = l->addr;
            break;
        }
    }

    if (main_found == 0) {
        printf("%s entry point is missing\n", main);
        exit(1);
    }

    f = fopen(pathname, "w");
    if (f == NULL) {
        perror("failed to open file");
        exit(1);
    }

    fwrite(&main_addr, sizeof(uint16_t), 1, f);

    for (size_t i = 0; i < tokens->size; ++i) {
        struct token *t = tokens->buf + i;
        uint16_t val = t->opcode;

        if (t->kind != TOKEN_OPCODE) {
            val = t->value;
        }

        fwrite(&val, sizeof(uint16_t), 1, f);
    }

    fclose(f);
}

int main(int argc, char **argv)
{
    struct scanner s;
    struct token_array tokens;
    struct label_array labels;
    char *outpath;

    argc--;
    argv++;

    if (argc < 1) {
        printf("assembly file is required\n");
        return 1;
    }

    outpath = create_outpath(*argv, NULL); // LEAK: os is freeing
    s.src = read_file(*argv); // LEAK: os is freeing
    s.cur = s.src;

    meminit(&tokens, sizeof(struct token), 64); // LEAK: os is freeing
    meminit(&labels, sizeof(struct label), 32); // LEAK: os is freeing

    scan(&s, &tokens, &labels);
    resolve_labels(&tokens, &labels);
    write_opcodes(outpath, &tokens, &labels);

    return 0;
}
