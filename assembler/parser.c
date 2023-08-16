#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

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

static void advance(struct parser *p)
{
    p->tok.kind = TOK_ERR;
    p->tok.lex = NULL;
    p->tok.lex_len = 0;
    p->tok.line = 0;

    scan_token(&p->s, &p->tok);
}

static void consume(struct parser *p, enum token_kind kind)
{
    if (p->tok.kind != kind) {
        fprintf(stderr, "%s:%d expected %s but got %s\n", p->s.filepath, p->tok.line, tok_to_str(kind), tok_to_str(p->tok.kind));
        exit(1);
    }

    advance(p);
}

static void parse_symbol(struct parser *p, struct inst *inst)
{
    if (lookup_opcode(p, inst) == 0) {
        fprintf(stderr, "%s:%d unknown symbol: %*.s\n", p->s.filepath, p->tok.line, p->tok.lex_len, p->tok.lex);
        exit(1);
    }

    consume(p, TOK_SYMBOL);

    if (inst->opcode == OP_PUSH || inst->opcode == OP_PUSHB) {
        // art: should be enough for 8-16bit int
        char buf[6];
        int maxlen;


        maxlen = 5;
        inst->operand_size = 2;
        if (inst->opcode == OP_PUSHB) {
            maxlen = 3;
            inst->operand_size = 1;
            if (p->tok.lex_len > maxlen) {
                fprintf(stderr, "%s:%d invalid value: %*.s\n", p->s.filepath, p->tok.line, p->tok.lex_len, p->tok.lex);
                exit(1);
            }
        }

        memcpy(buf, p->tok.lex, p->tok.lex_len);
        buf[p->tok.lex_len] = '\0';

        inst->operand = atoi(buf);

        consume(p, TOK_NUM);
    }
}

int parse_instruction(struct parser *p, struct inst *inst)
{
    if (p->tok.kind == TOK_SYMBOL) {
        parse_symbol(p, inst);
        return 1;
    }

    if (p->tok.kind == TOK_EOF) {
        return 0;
    }

    assert(0 && "unreachable");
}

void make_parser(struct parser *p, char *filepath)
{
    make_scanner(&p->s, filepath);
    advance(p);
}
