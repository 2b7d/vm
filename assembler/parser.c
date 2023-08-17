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
        if (p->tok->lex_len == e->str_len &&
                memcmp(p->tok->lex, e->str, e->str_len) == 0) {
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
    if (p->tok->kind == TOK_EOF) {
        return;
    }

    p->cur++;
    p->tok = p->ta.buf + p->cur;
}

static int next(struct parser *p, enum token_kind kind)
{
    int next;

    next = p->cur + 1;
    return next < p->ta.len && p->ta.buf[next].kind == kind;
}

static void parse_symbol(struct parser *p, struct inst *inst)
{
    if (lookup_opcode(p, inst) == 0) {
        fprintf(stderr, "%s:%d: unknown symbol %.*s\n", p->s.filepath, p->tok->line, p->tok->lex_len, p->tok->lex);
        exit(1);
    }

    advance(p);

    if (inst->opcode == OP_PUSH || inst->opcode == OP_PUSHB) {
        if (p->tok->kind == TOK_SYMBOL) {
            if (inst->opcode == OP_PUSHB) {
                fprintf(stderr, "%s:%d: can not push label with pushb, use push\n", p->s.filepath, p->tok->line);
                exit(1);
            }
            inst->operand.as_tok = p->tok;
            inst->operand_size = 2;
            inst->is_resolved = 0;
            advance(p);
        } else if (p->tok->kind == TOK_NUM) {
            char buf[6];
            int maxlen;

            maxlen = 5;
            inst->operand_size = 2;
            if (inst->opcode == OP_PUSHB) {
                maxlen = 3;
                inst->operand_size = 1;
                if (p->tok->lex_len > maxlen) {
                    fprintf(stderr, "%s:%d: invalid value %.*s\n", p->s.filepath, p->tok->line, p->tok->lex_len, p->tok->lex);
                    exit(1);
                }
            }

            memcpy(buf, p->tok->lex, p->tok->lex_len);
            buf[p->tok->lex_len] = '\0';

            // TODO(art): should overflow be error?
            inst->operand.as_int = atoi(buf);
            inst->is_resolved = 1;
            advance(p);
        } else {
            fprintf(stderr, "%s:%d: expected number or label but got %s\n", p->s.filepath, p->tok->line, tok_to_str(p->tok->kind));
            exit(1);
        }

        p->offset += inst->operand_size;
    }

    p->offset++;
}

void parse_label(struct parser *p, struct symtable *st)
{
    struct symbol *s;

    s = lookup_symbol(st, p->tok->lex, p->tok->lex_len);
    if (s != NULL) {
        fprintf(stderr, "%s:%d: symbol %.*s already exist\n", p->s.filepath, p->tok->line, p->tok->lex_len, p->tok->lex);
        exit(1);
    }

    s = memnext(st);
    s->label = p->tok->lex;
    s->label_len = p->tok->lex_len;
    s->value = p->offset;

    advance(p);
    advance(p);
}

void parse_instructions(struct parser *p, struct symtable *st,
                        struct inst_array *ia)
{
    struct inst *inst;

    while (p->tok->kind != TOK_EOF) {
        if (p->tok->kind != TOK_SYMBOL) {
            fprintf(stderr, "%s:%d: expected symbol but got %s\n", p->s.filepath, p->tok->line, tok_to_str(p->tok->kind));
            exit(1);
        }

        if (next(p, TOK_COLON) == 1) {
            parse_label(p, st);
        } else {
            inst = memnext(ia);
            parse_symbol(p, inst);
        }
    }
}

void resolve_labels(struct parser *p, struct symtable *st,
                    struct inst_array *ia)
{
    struct inst *inst;
    struct symbol *s;
    struct token *t;

    for (int i = 0; i < ia->len; ++i) {
        inst = ia->buf + i;
        if (inst->opcode == OP_PUSH || inst->opcode == OP_PUSHB) {
            if (inst->is_resolved == 1) {
                continue;
            }

            t = inst->operand.as_tok;
            s = lookup_symbol(st, t->lex, t->lex_len);
            if (s == NULL) {
                fprintf(stderr, "%s:%d: symbol %.*s does not exist\n", p->s.filepath, t->line, t->lex_len, t->lex);
                exit(1);
            }

            inst->operand.as_int = s->value;
            inst->is_resolved = 1;
        }
    }
}

void make_parser(struct parser *p, char *filepath)
{
    make_scanner(&p->s, filepath);
    meminit(&p->ta, sizeof(struct token), 256);

    scan_tokens(&p->s, &p->ta);

    p->cur = 0;
    p->offset = 0;
    p->tok = p->ta.buf;
}
