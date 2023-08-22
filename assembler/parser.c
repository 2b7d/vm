#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../lib/mem.h"
#include "../lib/sstring.h"

#include "../vm.h"
#include "../linker/ln.h"

#include "scanner.h"
#include "parser.h"

struct special_entry {
    string value;
    int addr;
};

static struct special_entry specials[] = {
    {.value = {.ptr = "_sp", .len = 3}, .addr = 0},
    {.value = {.ptr = "_fp", .len = 3}, .addr = 2},
    {.value = {.ptr = "_rp", .len = 3}, .addr = 4},

    {.value = {.ptr = NULL, .len = 0}, .addr = 0} // art: end of array
};

static int check_special(string *sym, int *addr)
{
    if (sym->len == 0) {
        return 0;
    }

    if (sym->ptr[0] != '_') {
        return 0;
    }

    for (struct special_entry *e = specials; e->value.ptr != NULL; ++e) {
        if (string_cmp(sym, &e->value) == 1) {
            *addr = e->addr;
            return 1;
        }
    }

    return 0;
}

struct op_entry {
    enum token_kind tok;
    enum vm_opcode op;
};

static struct op_entry opcodes[] = {
    {.tok = TOK_HALT,    .op = OP_HALT},
    {.tok = TOK_PUSH,    .op = OP_PUSH},
    {.tok = TOK_PUSHB,   .op = OP_PUSHB},
    {.tok = TOK_DROP,    .op = OP_DROP},
    {.tok = TOK_DROPB,   .op = OP_DROPB},
    {.tok = TOK_LD,      .op = OP_LD},
    {.tok = TOK_LDB,     .op = OP_LDB},
    {.tok = TOK_ST,      .op = OP_ST},
    {.tok = TOK_STB,     .op = OP_STB},
    {.tok = TOK_CTW,     .op = OP_CTW},
    {.tok = TOK_CTB,     .op = OP_CTB},
    {.tok = TOK_ADD,     .op = OP_ADD},
    {.tok = TOK_ADDB,    .op = OP_ADDB},
    {.tok = TOK_SUB,     .op = OP_SUB},
    {.tok = TOK_SUBB,    .op = OP_SUBB},
    {.tok = TOK_NEG,     .op = OP_NEG},
    {.tok = TOK_NEGB,    .op = OP_NEGB},
    {.tok = TOK_EQ,      .op = OP_EQ},
    {.tok = TOK_EQB,     .op = OP_EQB},
    {.tok = TOK_LT,      .op = OP_LT},
    {.tok = TOK_LTB,     .op = OP_LTB},
    {.tok = TOK_GT,      .op = OP_GT},
    {.tok = TOK_GTB,     .op = OP_GTB},
    {.tok = TOK_JMP,     .op = OP_JMP},
    {.tok = TOK_CJMP,    .op = OP_CJMP},
    {.tok = TOK_CALL,    .op = OP_CALL},
    {.tok = TOK_RET,     .op = OP_RET},
    {.tok = TOK_SYSCALL, .op = OP_SYSCALL},

    {.tok = TOK_ERR, .op = 0} // art: end of array
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

int with_operand(enum vm_opcode op)
{
    return op == OP_PUSH || op == OP_PUSHB || op == OP_CALL;
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

static int next(struct parser *p, enum token_kind kind)
{
    int next;

    next = p->cur + 1;
    return next < p->toks.len && p->toks.buf[next].kind == kind;
}

static struct symbol *symtab_lookup(struct symtab *st, string *label)
{
    for (int i = 0; i < st->len; ++i) {
        struct symbol *s;

        s = st->buf + i;
        if (string_cmp(&s->label, label) == 1) {
            return s;
        }
    }

    return NULL;
}

static struct symbol *symtab_insert(struct symtab *st, enum ln_symkind kind,
                                    enum vm_section sec, string *label,
                                    int addr, int is_resolved, int idx)
{
    struct symbol *s;

    s = memnext(st);
    s->kind = kind;
    s->sec = sec;
    s->addr = addr;
    s->is_resolved = is_resolved;
    s->idx = idx;
    string_dup(&s->label, label);

    return s;
}
static int parse_num(struct parser *p, int size)
{
    struct token *num;
    char buf[6];

    num = p->tok;

    if (size == 1 && num->lex.len > 3) {
        fprintf(stderr, "%s:%d: invalid 8bit value %.*s\n", p->s.filepath, num->line, num->lex.len, num->lex.ptr);
        exit(1);
    }

    if (size == 2 && num->lex.len > 5) {
        fprintf(stderr, "%s:%d: invalid 16bit value %.*s\n", p->s.filepath, num->line, num->lex.len, num->lex.ptr);
        exit(1);
    }

    memcpy(buf, num->lex.ptr, num->lex.len);
    buf[num->lex.len] = '\0';

    // TODO(art): should overflow be an error?
    return atoi(buf);
}

static int parse_data_label(struct parser *p, struct parsed_value *pv)
{
    struct data_label *dl;
    int offset;

    offset = 0;

    consume(p, TOK_DOT);

    dl = malloc(sizeof(struct data_label));
    if (dl == NULL) {
        perror("parse_data_label");
        exit(1);
    }

    meminit(&dl->values, sizeof(int), 64);

    if (p->tok->kind == TOK_BYTE) {
        dl->value_size = 1;
    } else if (p->tok->kind == TOK_WORD) {
        dl->value_size = 2;
    } else {
        fprintf(stderr, "%s:%d: unknown data directive %.*s\n", p->s.filepath, p->tok->line, p->tok->lex.len, p->tok->lex.ptr);
        exit(1);
    }

    advance(p);

    for (;;) {
        if (p->tok->kind == TOK_NUM) {
            memgrow(&dl->values);
            dl->values.buf[dl->values.len++] = parse_num(p, dl->value_size);
            offset += dl->value_size;
        } else if (p->tok->kind == TOK_STR) {
            int old_len;

            old_len = dl->values.len;
            dl->values.len += p->tok->lex.len;
            memgrow(&dl->values);
            offset += p->tok->lex.len * dl->value_size;
            for (int i = 0; i < p->tok->lex.len; ++i) {
                dl->values.buf[old_len + i] = p->tok->lex.ptr[i];
            }
        } else {
            fprintf(stderr, "%s:%d: expected number or string but got %s\n", p->s.filepath, p->tok->line, tok_to_str(p->tok->kind));
            exit(1);
        }

        advance(p);

        if (p->tok->kind != TOK_COMMA) {
            break;
        }

        advance(p);
    }

    pv->kind = PARSVAL_DATA_LABEL;
    pv->value = dl;
    return offset;
}

static int parse_mnemonic(struct parser *p, struct symtab *st,
                          struct relocations *rels, struct text_label *tl,
                          int off)
{
    struct mnemonic *m;
    int offset;

    offset = 0;

    m = memnext(tl);
    m->opcode = lookup_opcode(p->tok->kind);

    advance(p);
    offset++;

    if (with_operand(m->opcode) == 0) {
        return offset;
    }

    m->operand.size = 2;
    if (m->opcode == OP_PUSHB) {
        m->operand.size = 1;
    }

    if (p->tok->kind == TOK_NUM) {
        m->operand.kind = OPERAND_NUM;
        m->operand.as.num = parse_num(p, m->operand.size);
    } else if (p->tok->kind == TOK_SYM) {
        if (m->opcode == OP_PUSHB) {
            fprintf(stderr, "%s:%d: attempted to push symbol with pushb, use push\n", p->s.filepath, p->tok->line);
            exit(1);
        }
        if (check_special(&p->tok->lex, &m->operand.as.num) == 1) {
            m->operand.kind = OPERAND_NUM;
        } else {
            struct symbol *sym;
            struct ln_relocation *rel;

            sym = symtab_lookup(st, &p->tok->lex);
            if (sym == NULL) {
                sym = symtab_insert(st, SYM_LOCAL, SECTION_TEXT, &p->tok->lex,
                                    0, 0, st->len);
            }

            m->operand.as.sym = sym;
            m->operand.kind = OPERAND_SYM;

            rel = memnext(rels);
            rel->loc = off + offset; // art: skip mnemonic
            rel->symidx = sym->idx;
        }
    } else if (p->tok->kind == TOK_STR) {
        if (m->opcode == OP_CALL) {
            fprintf(stderr, "%s:%d: expected symbol or number\n", p->s.filepath, p->tok->line);
            exit(1);
        }
        if (p->tok->lex.len == 0) {
            fprintf(stderr, "%s:%d: can not push empty string\n", p->s.filepath, p->tok->line);
            exit(1);
        }
        if (p->tok->lex.len > 1) {
            fprintf(stderr, "%s:%d: can not push more than one character\n", p->s.filepath, p->tok->line);
            exit(1);
        }
        m->operand.kind = OPERAND_NUM;
        m->operand.as.num = p->tok->lex.ptr[0];
    } else {
        fprintf(stderr, "%s:%d: expected number, symbol or string but got %s\n", p->s.filepath, p->tok->line, tok_to_str(p->tok->kind));
        exit(1);
    }

    advance(p);
    offset += m->operand.size;
    return offset;
}

static int parse_text_label(struct parser *p, struct symtab *st,
                            struct relocations *rels, struct parsed_value *pv,
                            int off)
{
    struct text_label *tl;
    int offset;

    offset = 0;

    tl = malloc(sizeof(struct text_label));
    if (tl == NULL) {
        perror("parse_text_label");
        exit(1);
    }

    meminit(tl, sizeof(struct mnemonic), 128);

    while (is_mnemonic(p->tok->kind) == 1) {
        offset += parse_mnemonic(p, st, rels, tl, off + offset);
    }

    if (p->tok->kind != TOK_EOF &&
            (p->tok->kind != TOK_SYM || next(p, TOK_COLON) == 0)) {
        fprintf(stderr, "%s:%d: unknown mnemonic %.*s\n", p->s.filepath, p->tok->line, p->tok->lex.len, p->tok->lex.ptr);
        exit(1);
    }

    pv->kind = PARSVAL_TEXT_LABEL;
    pv->value = tl;
    return offset;
}

static void parse_symbol_directive(struct parser *p, struct symtab *st)
{
    enum ln_symkind kind;

    consume(p, TOK_DOT);

    if (p->tok->kind != TOK_GLOBAL && p->tok->kind != TOK_EXTERN) {
        fprintf(stderr, "%s:%d: expected global or extern but got %s\n", p->s.filepath, p->tok->line, tok_to_str(p->tok->kind));
        exit(1);
    }

    kind = SYM_GLOBAL;
    if (p->tok->kind == TOK_EXTERN) {
        kind = SYM_EXTERN;
    }

    advance(p);

    for (;;) {
        struct token *symtok;
        struct symbol *sym;

        symtok = p->tok;
        consume(p, TOK_SYM);

        sym = symtab_lookup(st, &symtok->lex);
        if (sym != NULL) {
            sym->kind = kind;
        } else {
            symtab_insert(st, kind, SECTION_TEXT, &symtok->lex, 0, 0, st->len);
        }

        if (p->tok->kind != TOK_COMMA) {
            break;
        }

        advance(p);
    }
}

void parse(struct parser *p, struct symtab *st, struct relocations *rels,
           struct parsed_values *values)
{
    int data_offset, text_offset;

    data_offset = 0;
    text_offset = 0;

    while (p->tok->kind != TOK_EOF) {
        struct token *label;
        struct symbol *sym;
        struct parsed_value *pv;

        if (p->tok->kind == TOK_DOT) {
            parse_symbol_directive(p, st);
            continue;
        }

        label = p->tok;

        consume(p, TOK_SYM);
        consume(p, TOK_COLON);

        sym = symtab_lookup(st, &label->lex);
        if (sym == NULL) {
            sym = symtab_insert(st, SYM_LOCAL, SECTION_TEXT, &label->lex,
                                0, 1, st->len);
        } else {
            sym->is_resolved = 1;
        }

        pv = memnext(values);

        if (p->tok->kind == TOK_DOT) {
            sym->addr = data_offset;
            sym->sec = SECTION_DATA;
            data_offset += parse_data_label(p, pv);
        } else if (is_mnemonic(p->tok->kind) == 1) {
            sym->addr = text_offset;
            sym->sec = SECTION_TEXT;
            text_offset += parse_text_label(p, st, rels, pv, text_offset);
        } else {
            fprintf(stderr, "%s:%d: expected directive or mnemonic but got %s\n", p->s.filepath, p->tok->line, tok_to_str(p->tok->kind));
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
