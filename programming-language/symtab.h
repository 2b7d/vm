// #include "lib/sstring.h
// #include "scanner.h"

typedef enum {
    SYM_VAR = 0,
    SYM_PROC
} Symbol_Kind;

typedef enum {
    SCOPE_FILE = 0,
    SCOPE_BLOCK
} Symbol_Scope;

typedef struct {
    int idx;

    Symbol_Kind kind;
    Symbol_Scope scope;

    string name;
    Token_Kind type;
    int block_idx;

    Token_Kind ret_type;
    struct {
        int len;
        int cap;
        Token_Kind *buf;
    } params;
} Symbol;

typedef struct {
    int len;
    int cap;
    Symbol *buf;
} Symtab;


Symbol *symtab_lookup(Symtab *st, string *name, int block_idx);
Symbol *symtab_full_lookup(Symtab *st, string *name, int block_idx);
Symbol *symtab_insert(Symtab *st, string *name, int block_idx, Symbol_Kind kind, Symbol_Scope scope, Token_Kind type, Token_Kind ret_type);
Symbol *symtab_insert_var(Symtab *st, string *name, int block_idx, Symbol_Scope scope, Token_Kind type);
Symbol *symtab_insert_proc(Symtab *st, string *name, Token_Kind ret_type);

/*
idx | kind | scope |   name   | type | ret_type | block_idx | params |
----|------|-------|----------------------------|---------------------
0   | var  | file  | file_a   | u16  |          | -1        |        |

1   | proc | file  | inc      |      | u16      | -1        | 1[u16] |
2   | var  | block | arg_a    | u16  |          |  1        |        |
3   | var  | block | block_a  | u16  |          |  1        |        |

4   | proc | file  | main     |      | void     | -1        | 0[]    |
5   | var  | block | block_a  | u16  |          |  4        |        |
6   | var  | block | block_b  | u16  |          |  4        |        |
*/
