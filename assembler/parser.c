#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../lib/mem.h"

#include "../vm.h"
#include "scanner.h"
#include "parser.h"

struct op_entry {
    enum token_kind tok;
    enum vm_opcode op;
};

static struct op_entry opcodes[] = {
    { .tok = TOK_HALT,  .op = OP_HALT },
    { .tok = TOK_PUSH,  .op = OP_PUSH },
    { .tok = TOK_PUSHB, .op = OP_PUSHB },
    { .tok = TOK_CTW,   .op = OP_CTW },
    { .tok = TOK_CTB,   .op = OP_CTB },
    { .tok = TOK_ADD,   .op = OP_ADD },
    { .tok = TOK_ADDB,  .op = OP_ADDB },
    { .tok = TOK_SUB,   .op = OP_SUB },
    { .tok = TOK_SUBB,  .op = OP_SUBB },
    { .tok = TOK_NEG,   .op = OP_NEG },
    { .tok = TOK_NEGB,  .op = OP_NEGB },
    { .tok = TOK_EQ,    .op = OP_EQ },
    { .tok = TOK_EQB,   .op = OP_EQB },
    { .tok = TOK_LT,    .op = OP_LT },
    { .tok = TOK_LTB,   .op = OP_LTB },
    { .tok = TOK_GT,    .op = OP_GT },
    { .tok = TOK_GTB,   .op = OP_GTB },
    { .tok = TOK_JMP,   .op = OP_JMP },
    { .tok = TOK_CJMP,  .op = OP_CJMP },

    { .tok = TOK_ERR, .op = 0 } // art: end of array
};

static enum vm_opcode lookup_opcode(enum token_kind kind)
{
    for (struct op_entry *e = opcodes; e->tok != TOK_ERR; ++e) {
        if (e->tok == kind) {
            return e->op;
        }
    }

    assert(0 && "unreachable");
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

static struct symbol *insert_symbol(struct symtable *st, char *label,
                                    int label_len, int addr, int is_resolved)
{
    struct symbol *s;

    s = lookup_symbol(st, label, label_len);
    if (s != NULL) {
        return s;
    }

    s = memnext(st);
    s->label = label;
    s->label_len = label_len;
    s->addr = addr;
    s->is_resolved = is_resolved;

    return s;
}

static void advance(struct parser *p)
{
    if (p->tok->kind == TOK_EOF) {
        return;
    }

    p->cur++;
    p->tok = p->toks.buf + p->cur;
}

static void consume(struct parser *p, enum token_kind kind)
{
    if (p->tok->kind != kind) {
        fprintf(stderr, "%s:%d: expected %s but got %s\n", p->s.filepath, p->tok->line, tok_to_str(kind), tok_to_str(p->tok->kind));
        exit(1);
    }

    advance(p);
}

static int parse_num(struct parser *p, int size)
{
    struct token *num;
    char buf[6];

    num = p->tok;

    if (size == 1 && num->lex_len > 3) {
        fprintf(stderr, "%s:%d: invalid 8bit value %.*s\n", p->s.filepath, num->line, num->lex_len, num->lex);
        exit(1);
    }

    if (size == 2 && num->lex_len > 5) {
        fprintf(stderr, "%s:%d: invalid 16bit value %.*s\n", p->s.filepath, num->line, num->lex_len, num->lex);
        exit(1);
    }

    memcpy(buf, num->lex, num->lex_len);
    buf[num->lex_len] = '\0';

    // TODO(art): should overflow be an error?
    return atoi(buf);
}

static void parse_data(struct parser *p, struct symtable *st,
                       struct parsed_values *values)
{
    struct token *label;
    struct symbol *sym;
    struct data_label *dl;
    struct parsed_value *pv;
    int offset, old_len;

    offset = 0;

    while (p->tok->kind != TOK_SECTION && p->tok->kind != TOK_EOF) {
        label = p->tok;

        consume(p, TOK_SYM);
        consume(p, TOK_COLON);

        sym = lookup_symbol(st, label->lex, label->lex_len);
        if (sym == NULL) {
            sym = memnext(st);
            sym->label = label->lex;
            sym->label_len = label->lex_len;
        }
        sym->addr = offset;
        sym->is_resolved = 1;

        dl = malloc(sizeof(struct data_label));
        if (dl == NULL) {
            perror("parse_data");
            exit(1);
        }

        dl->value_size = 2;
        meminit(&dl->values, sizeof(int), 128);

        pv = memnext(values);
        pv->kind = PARSVAL_DATA_LABEL;
        pv->value = dl;

        if (p->tok->kind == TOK_DOT) {
            advance(p);
            consume(p, TOK_BYTE);
            dl->value_size = 1;
        }

        for (;;) {
            switch (p->tok->kind) {
            case TOK_NUM:
                memgrow(&dl->values);
                dl->values.buf[dl->values.len++] = parse_num(p, dl->value_size);
                offset += dl->value_size;
                break;
            case TOK_STR:
                old_len = dl->values.len;
                dl->values.len += p->tok->lex_len;
                memgrow(&dl->values);
                offset += p->tok->lex_len * dl->value_size;
                for (int i = 0; i < p->tok->lex_len; ++i) {
                    dl->values.buf[old_len + i] = p->tok->lex[i];
                }
                break;
            default:
                fprintf(stderr, "%s:%d: expected number or string but got %s\n", p->s.filepath, p->tok->line, tok_to_str(p->tok->kind));
                exit(1);
            }

            advance(p);

            if (p->tok->kind != TOK_COMMA) {
                break;
            }

            advance(p);
        }
    }
}

static void parse_text(struct parser *p, struct symtable *st,
                       struct parsed_values *values)
{
    struct token *label;
    struct symbol *sym;
    struct mnemonic *m;
    struct mnemonic_push *mp;
    struct parsed_value *pv;
    enum vm_opcode opcode;
    int offset;

    offset = 0;

    while (p->tok->kind != TOK_SECTION && p->tok->kind != TOK_EOF) {
        label = p->tok;

        consume(p, TOK_SYM);
        consume(p, TOK_COLON);

        sym = lookup_symbol(st, label->lex, label->lex_len);
        if (sym == NULL) {
            sym = memnext(st);
            sym->label = label->lex;
            sym->label_len = label->lex_len;
        }
        sym->addr = offset;
        sym->is_resolved = 1;

        for (;;) {
            if (is_mnemonic(p->tok->kind) == 0) {
                break;
            }

            pv = memnext(values);

            opcode = lookup_opcode(p->tok->kind);
            advance(p);
            offset++;

            if (opcode == OP_PUSH || opcode == OP_PUSHB) {
                mp = malloc(sizeof(struct mnemonic_push));
                if (mp == NULL) {
                    perror("parse_text");
                    exit(1);
                }

                pv->kind = PARSVAL_MNEMONIC_PUSH;
                pv->value = mp;

                mp->opcode = opcode;
                mp->operand.size = 2;
                if (mp->opcode == OP_PUSHB) {
                    mp->operand.size = 1;
                }

                offset += mp->operand.size;

                switch (p->tok->kind) {
                case TOK_NUM:
                    mp->operand.kind = PUSH_NUM;
                    mp->operand.as.num = parse_num(p, mp->operand.size);
                    break;
                case TOK_SYM:
                    if (mp->opcode == OP_PUSHB) {
                        fprintf(stderr, "%s:%d: attempted to push symbol with pushb, use push\n", p->s.filepath, p->tok->line);
                        exit(1);
                    }
                    sym = insert_symbol(st, p->tok->lex, p->tok->lex_len, 0, 0);
                    mp->operand.kind = PUSH_SYM;
                    mp->operand.as.sym = sym;
                    break;
                case TOK_STR:
                    if (p->tok->lex_len == 0) {
                        fprintf(stderr, "%s:%d: can not push empty string\n", p->s.filepath, p->tok->line);
                        exit(1);
                    }
                    if (p->tok->lex_len > 1) {
                        fprintf(stderr, "%s:%d: can not push more than one character\n", p->s.filepath, p->tok->line);
                        exit(1);
                    }
                    mp->operand.kind = PUSH_NUM;
                    mp->operand.as.num = p->tok->lex[0];
                    break;
                default:
                    fprintf(stderr, "%s:%d: expected number, symbol or string but got %s\n", p->s.filepath, p->tok->line, tok_to_str(p->tok->kind));
                    exit(1);
                }

                advance(p);
                continue;
            }

            m = malloc(sizeof(struct mnemonic));
            if (m == NULL) {
                perror("parse_text");
                exit(1);
            }

            m->opcode = opcode;

            pv->kind = PARSVAL_MNEMONIC;
            pv->value = m;
        }
    }
}

void parse(struct parser *p, struct symtable *st, struct parsed_values *values)
{
    while (p->tok->kind != TOK_EOF) {
        consume(p, TOK_SECTION);

        switch (p->tok->kind) {
        case TOK_DATA:
            advance(p);
            parse_data(p, st, values);
            break;
        case TOK_TEXT:
            advance(p);
            parse_text(p, st, values);
            break;
        default:
            fprintf(stderr, "%s:%d: expected data or text but got %s\n", p->s.filepath, p->tok->line, tok_to_str(p->tok->kind));
            exit(1);
        }
    }
}

void make_parser(struct parser *p, char *filepath)
{
    make_scanner(&p->s, filepath);
    meminit(&p->toks, sizeof(struct token), 256);

    scan_tokens(&p->s, &p->toks);

    p->tok = p->toks.buf;
    p->cur = 0;
}
