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
    char *lexeme;
    size_t lexemelen;
};

struct token_array {
    size_t size;
    size_t cap;
    struct token *buf;
};

struct label {
    size_t addr;
    char *start;
    size_t len;
};

struct label_array {
    size_t size;
    size_t cap;
    struct label *buf;
};

char *opcode_strings[OP_COUNT] = {
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
    [OP_RETPUSH]  = "retpush",
    [OP_RETPOP]   = "retpop",
    [OP_RETCOPY]  = "retcopy",
    [OP_RETDROP]  = "retdrop",
    [OP_RETSP]    = "retsp",
    [OP_RETSPSET] = "retspset",
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

int compare_lexeme(char *start, size_t len, char *word)
{
    if (strlen(word) != len) {
        return 0;
    }

    if (strncmp(start, word, len) == 0) {
        return 1;
    }

    return 0;
}

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
                int is_opcode = 0;

                while (isalnum(*s->cur) != 0 || *s->cur == '_') {
                    s->cur++;
                }

                if (*s->cur == ':') {
                    struct label *l;

                    memgrow(labels, sizeof(struct label));
                    l = labels->buf + labels->size;
                    labels->size++;

                    l->addr = tokens->size;
                    l->start = start;
                    l->len = s->cur - start;
                    s->cur++;
                    break;
                }

                memgrow(tokens, sizeof(struct token));
                t = tokens->buf + tokens->size;
                tokens->size++;

                t->kind = TOKEN_OPCODE;
                for (size_t i = 0; i < OP_COUNT; ++i) {
                    if (compare_lexeme(start,
                                       s->cur - start,
                                       opcode_strings[i]) == 1) {
                        t->opcode = i;
                        is_opcode = 1;
                        break;
                    }
                }

                if (is_opcode == 0) {
                    t->kind = TOKEN_LABEL;
                    t->lexeme = start;
                    t->lexemelen = s->cur - start;
                }

                break;
            }

            if (isdigit(*s->cur) != 0) {
                struct token *t;
                char val[6];

                memgrow(tokens, sizeof(struct token));
                t = tokens->buf + tokens->size;
                tokens->size++;

                while (isdigit(*s->cur) != 0) {
                    s->cur++;
                }

                if ((size_t) (s->cur - start) >= sizeof(val))  {
                    printf("value %.*s is greater than 16bit\n",
                           (int) (s->cur - start),
                           start);
                    exit(1);
                }

                memset(val, 0, sizeof(val));
                memcpy(val, start, s->cur - start);
                t->kind = TOKEN_IMMEDIATE;
                t->value = atoi(val);
                break;
            }

            printf("ERROR: unknown character `%c`\n", *s->cur);
            exit(1);
        }
    }
}

int main(int argc, char **argv)
{
    struct scanner s;
    struct token_array tokens;
    struct label_array labels;
    FILE *out;

    argc--;
    argv++;

    if (argc < 1) {
        printf("assembly file is required\n");
        return 1;
    }

    out = fopen(create_outpath(*argv, NULL), "w");
    if (out == NULL) {
        printf("failed to open out file\n");
        return 1;
    }

    s.src = read_file(*argv); // LEAK: OS is freeing
    s.cur = s.src;

    meminit(&tokens, sizeof(struct token), 64); // LEAK: OS is freeing
    meminit(&labels, sizeof(struct label), 32); // LEAK: OS is freeing

    scan(&s, &tokens, &labels);

    {
        int start_found = 0;
        char *start = "_start";
        size_t len = strlen(start);

        for (size_t i = 0; i < labels.size; ++i) {
            struct label *l = labels.buf + i;

            if (l->len != len) {
                continue;
            }

            if (memcmp(l->start, start, len) == 0) {
                start_found = 1;
                fwrite(&l->addr, sizeof(uint16_t), 1, out);
                break;
            }
        }

        if (start_found == 0) {
            printf("_start entry point is missing\n");
            return 1;
        }
    }

    for (size_t i = 0; i < tokens.size; ++i) {
        int label_found = 0;
        struct token *t = tokens.buf + i;

        if (t->kind != TOKEN_LABEL) {
            continue;
        }

        for (size_t j = 0; j < labels.size; ++j) {
            struct label *l = labels.buf + j;

            if (t->lexemelen != l->len) {
                continue;
            }

            if (memcmp(t->lexeme, l->start, l->len) == 0) {
                label_found = 1;
                t->value = l->addr;
                break;
            }
        }

        if (label_found == 0) {
            printf("Undefined label %.*s\n", (int) t->lexemelen, t->lexeme);
            return 1;
        }
    }

    for (size_t i = 0; i < tokens.size; ++i) {
        struct token *t = tokens.buf + i;
        uint16_t val = t->opcode;

        if (t->kind != TOKEN_OPCODE) {
            val = t->value;
        }

        fwrite(&val, sizeof(uint16_t), 1, out);
    }

    fclose(out);
    return 0;
}
