#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../lib/mem.h"

#include "../vm.h"
#include "scanner.h"
#include "parser.h"

struct opcode_entry {
    char *str;
    int str_len;
    enum vm_opcode opcode;
};

static struct opcode_entry opcodes[] = {
    { .str = "halt",  .str_len = 4, .opcode = OP_HALT },

    { .str = "push",  .str_len = 4, .opcode = OP_PUSH },
    { .str = "pushb", .str_len = 5, .opcode = OP_PUSHB },

    { .str = "ctw",  .str_len = 3, .opcode = OP_CTW },
    { .str = "ctb",  .str_len = 3, .opcode = OP_CTB },

    { .str = "add",  .str_len = 3, .opcode = OP_ADD },
    { .str = "addb", .str_len = 4, .opcode = OP_ADDB },
    { .str = "sub",  .str_len = 3, .opcode = OP_SUB },
    { .str = "subb", .str_len = 4, .opcode = OP_SUBB },
    { .str = "neg",  .str_len = 3, .opcode = OP_NEG },
    { .str = "negb", .str_len = 4, .opcode = OP_NEGB },

    { .str = "eq",   .str_len = 2, .opcode = OP_EQ },
    { .str = "eqb",  .str_len = 3, .opcode = OP_EQB },
    { .str = "lt",   .str_len = 2, .opcode = OP_LT },
    { .str = "ltb",  .str_len = 3, .opcode = OP_LTB },
    { .str = "gt",   .str_len = 2, .opcode = OP_GT },
    { .str = "gtb",  .str_len = 3, .opcode = OP_GTB },

    { .str = "jmp",  .str_len = 3, .opcode = OP_JMP },
    { .str = "cjmp", .str_len = 4, .opcode = OP_CJMP },

    { .str = NULL, .str_len = 0, .opcode = 0 } // art: end of array
};

static int lookup_opcode(struct parser *p, struct inst *inst)
{
    for (struct opcode_entry *e = opcodes; e->str != NULL; e++) {
        if (p->tok.lex_len == e->str_len &&
                memcmp(p->tok.lex, e->str, e->str_len) == 0) {
            inst->opcode = e->opcode;
            return 1;
        }
    }

    return 0;
}

static struct symbol *lookup_symbol(struct symtable *st, char *label,
                                    int label_len)
{
    struct symbol *s;

    for (int i = 0; i < st->len; ++i) {
        s = st->buf + i;
        if (s->label_len == label_len &&
                memcmp(s->label, label, label_len) == 0) {
            return s;
        }
    }

    return NULL;
}

static void advance(struct parser *p)
{
    p->tok.kind = TOK_ERR;
    p->tok.lex = NULL;
    p->tok.lex_len = 0;
    p->tok.line = 0;

    scan_token(&p->s, &p->tok);
}

static int next(struct parser *p, enum token_kind kind)
{
    return p->tok.kind == kind;
}

static int next2(struct parser *p, enum token_kind kind)
{
    struct token tok;
    int result;

    memcpy(&tok, &p->tok, sizeof(struct token));
    advance(p);

    result = next(p, kind);

    undo_scan(&p->s);
    memcpy(&p->tok, &tok, sizeof(struct token));

    return result;
}

static void parse_symbol(struct parser *p, struct symtable *st,
                         struct inst *inst)
{
    if (lookup_opcode(p, inst) == 0) {
        fprintf(stderr, "%s:%d: unknown symbol %.*s\n", p->s.filepath, p->tok.line, p->tok.lex_len, p->tok.lex);
        exit(1);
    }

    advance(p);

    if (inst->opcode == OP_PUSH || inst->opcode == OP_PUSHB) {
        if (next(p, TOK_SYMBOL) == 1) {
            struct symbol *s;

            if (inst->opcode == OP_PUSHB) {
                fprintf(stderr, "%s:%d: can not push label with pushb, use push\n", p->s.filepath, p->tok.line);
                exit(1);
            }

            s = lookup_symbol(st, p->tok.lex, p->tok.lex_len);
            if (s == NULL) {
                fprintf(stderr, "%s:%d: unknown symbol %.*s\n", p->s.filepath, p->tok.line, p->tok.lex_len, p->tok.lex);
                exit(1);
            }

            inst->operand = s->value;
            inst->operand_size = 2;

            advance(p);
        } else if (next(p, TOK_NUM) == 1) {
            // art: should be enough for 8-16bit int
            char buf[6];
            int maxlen;

            maxlen = 5;
            inst->operand_size = 2;
            if (inst->opcode == OP_PUSHB) {
                maxlen = 3;
                inst->operand_size = 1;
                if (p->tok.lex_len > maxlen) {
                    fprintf(stderr, "%s:%d: invalid value %.*s\n", p->s.filepath, p->tok.line, p->tok.lex_len, p->tok.lex);
                    exit(1);
                }
            }

            memcpy(buf, p->tok.lex, p->tok.lex_len);
            buf[p->tok.lex_len] = '\0';

            // TODO(art): should overflow be error?
            inst->operand = atoi(buf);

            advance(p);
        } else {
            fprintf(stderr, "%s:%d: expected number or label but got %s\n", p->s.filepath, p->tok.line, tok_to_str(p->tok.kind));
            exit(1);
        }
    }
}

int parse_instruction(struct parser *p, struct symtable *st, struct inst *inst)
{
parse_again:
    if (next(p, TOK_SYMBOL) == 1) {
        if (next2(p, TOK_COLON) == 1) {
            advance(p);
            advance(p);
            goto parse_again;
        }
        parse_symbol(p, st, inst);
        return 1;
    }

    if (next(p, TOK_EOF) == 1) {
        return 0;
    }

    assert(0 && "unreachable");
}

void populate_symbols(struct parser *p, struct symtable *st)
{
    struct symbol *s;
    struct inst inst;
    int offset = 0;

    while (next(p, TOK_EOF) == 0) {
        if (next(p, TOK_SYMBOL) == 1) {
            if (next2(p, TOK_COLON) == 1) {
                s = memnext(st);
                s->label = p->tok.lex;
                s->label_len = p->tok.lex_len;
                s->value = offset;
                advance(p);
                advance(p);
            } else {
                offset++;
                if (lookup_opcode(p, &inst) == 1) {
                    advance(p);
                    if (inst.opcode == OP_PUSH) {
                        offset += 2;
                        advance(p);
                    } else if (inst.opcode == OP_PUSHB) {
                        offset++;
                        advance(p);
                    }
                } else {
                    advance(p);
                }
            }
        } else {
            assert(0 && "unreachable");
        }
    }

    // art: reset
    p->s.cur = 0;
    p->s.pos = 0;
    p->s.line = 1;
    p->s.ch = p->s.src[0];
    advance(p);
}

void make_parser(struct parser *p, char *filepath)
{
    make_scanner(&p->s, filepath);
    advance(p);
}
