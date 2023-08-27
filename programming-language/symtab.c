#include <stdio.h>
#include <stdlib.h>

#include "../lib/sstring.h"
#include "../lib/mem.h"

#include "scanner.h"
#include "symtab.h"

Symbol *symtab_lookup(Symtab *st, string *name, int block_idx)
{
    for (int i = 0; i < st->len; ++i) {
        Symbol *sym = st->buf + i;

        if (sym->block_idx == block_idx && string_cmp(&sym->name, name) == 1) {
            return sym;
        }
    }

    return NULL;
}

Symbol *symtab_full_lookup(Symtab *st, string *name, int block_idx)
{
    Symbol *sym = symtab_lookup(st, name, block_idx);
    if (sym != NULL) {
        return sym;
    }

    if (block_idx == -1) {
        return NULL;
    }

    return symtab_lookup(st, name, -1);
}

Symbol *symtab_insert(Symtab *st, string *name, int block_idx,
                      Symbol_Kind kind, Symbol_Scope scope, Token_Kind type,
                      Token_Kind ret_type)
{
    int idx = st->len;

    Symbol *sym;

    if (symtab_lookup(st, name, block_idx) != NULL) {
        return NULL;
    }

    mem_grow(st);
    sym = mem_next(st);

    string_dup(&sym->name, name);

    sym->idx = idx;
    sym->block_idx = block_idx;
    sym->kind = kind;
    sym->scope = scope;
    sym->type = type;
    sym->ret_type = ret_type;

    if (kind == SYM_PROC) {
        mem_make(&sym->params, 256);
    }

    return sym;
}

Symbol *symtab_insert_var(Symtab *st, string *name, int block_idx,
                          Symbol_Scope scope, Token_Kind type)
{
    return symtab_insert(st, name, block_idx, SYM_VAR, scope, type, 0);
}

Symbol *symtab_insert_proc(Symtab *st, string *name, Token_Kind ret_type)
{
    return symtab_insert(st, name, -1, SYM_PROC, SCOPE_FILE, 0, ret_type);
}
