/*
 * program = declaration* EOF
 *
 * declaration    = procedure|file_variable
 * procedure      = "proc" ident "(" params? ")" ":" type proc_block
 * file_variable  = variable number ";"
 * block_variable = variable expression ";"
 *
 * variable = "let" ident ":" type "="
 * params   = ident ":" type ("," ident ":" type)*
 * type     = "u16"|"void"
 *
 * statement = assign|return|stmt_expr
 * assign    = ident "=" expression ";"
 * return    = "return" expression? ";"
 * stmt_expr = expression ";"
 * block     = "{" statement* "}"
 * proc_bloc = "{" block_variable* statement* "}"
 *
 * expression = term
 * term       = call (("+"|"-") call)*
 * call       = primary ("(" args? ")")?
 * primary    = ident|number|"(" expression ")"
 *
 * args      = expression ("," expression)*
 *
 * ident  = char (char|digit)
 * number = digit+
 *
 * char  = "a".."z"|"A".."Z"|"_"
 * digit = "0".."9"
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "../lib/mem.h"
#include "../lib/sstring.h"

#include "scanner.h"
#include "symtab.h"
#include "parser.h"

Token_Kind check_expr(Expr *expr, int block_idx, Symtab *st)
{
    Token_Kind type_x, type_y;
    Expr_Binary *binary;
    Expr_Call *call;
    Expr_Lit *lit;
    Symbol *sym;

    switch (expr->kind) {
    case EXPR_CONST:
    case EXPR_LIT:
        lit = expr->body;

        switch (lit->value->kind) {
        case TOK_NUM:
            return TOK_NUM;
        case TOK_IDENT:
            sym = symtab_full_lookup(st, &lit->value->lex, block_idx);
            if (sym == NULL) {
                fprintf(stderr, "%s:%d: symbol %.*s does not exist\n", lit->value->pos.file, lit->value->pos.line, lit->value->lex.len, lit->value->lex.ptr);
                exit(1);
            }
            if (sym->kind == SYM_PROC) {
                fprintf(stderr, "%s:%d: proc pointers are not supported yet\n", lit->value->pos.file, lit->value->pos.line);
                exit(1);
            }
            return sym->type;
        default:
            assert(0 && "unreachable");
        }
    case EXPR_BINARY:
        binary = expr->body;

        type_x = check_expr(&binary->x, block_idx, st);
        type_y = check_expr(&binary->y, block_idx, st);

        if (type_x == TOK_NUM && type_y == TOK_NUM) {
            return TOK_NUM;
        } else if (type_x == TOK_NUM) {
            type_x = type_y;
        } else if (type_y == TOK_NUM) {
            type_y = type_x;
        }

        switch (binary->op->kind) {
        case TOK_PLUS:
        case TOK_MINUS:
            if (scanner_is_numtype(type_x) == 1 &&
                    scanner_is_numtype(type_y) == 1 &&
                    type_x == type_y) {
                return type_x;
            }
            fprintf(stderr, "%s:%d: unexpected types %s, %s for %s operator\n", binary->op->pos.file, binary->op->pos.line, scanner_tokstr(type_x), scanner_tokstr(type_y), scanner_tokstr(binary->op->kind));
            exit(1);
        default:
            assert(0 && "unreachable");
        }
    case EXPR_CALL:
        call = expr->body;
        lit = call->callee.body;

        sym = symtab_full_lookup(st, &lit->value->lex, block_idx);
        if (sym == NULL) {
            fprintf(stderr, "%s:%d: symbol %.*s does not exist\n", lit->value->pos.file, lit->value->pos.line, lit->value->lex.len, lit->value->lex.ptr);
            exit(1);
        }
        if (sym->kind == SYM_VAR) {
            fprintf(stderr, "%s:%d: attempted to call a var\n", lit->value->pos.file, lit->value->pos.line);
            exit(1);
        }

        if (call->args.len != sym->params.len) {
            fprintf(stderr, "%s:%d: expected %d amount of args but got %d\n", lit->value->pos.file, lit->value->pos.line, sym->params.len, call->args.len);
            exit(1);
        }

        for (int i = 0; i < sym->params.len; ++i) {
            type_x = sym->params.buf[i];
            type_y = check_expr(call->args.buf + i, block_idx, st);

            if (type_y == TOK_NUM && scanner_is_numtype(type_x) == 1) {
                type_y = type_x;
            }

            if (type_x != type_y) {
                fprintf(stderr, "%s:%d: expected type %s of arg %d but got %s\n", lit->value->pos.file, lit->value->pos.line, scanner_tokstr(type_x), i + 1, scanner_tokstr(type_y));
                exit(1);
            }
        }

        return sym->ret_type;
    default:
        assert(0 && "unreachable");
    }
}

void check_stmt(Stmt *stmt, Symbol *block, Symtab *st)
{
    Stmt_Expr *stmt_expr;
    Stmt_Assign *assign;
    Stmt_Ret *ret;
    Symbol *sym;
    Token_Kind type;

    switch (stmt->kind) {
    case STMT_ASSIGN:
        assign = stmt->body;

        sym = symtab_full_lookup(st, &assign->ident->lex, block->idx);
        if (sym == NULL) {
            fprintf(stderr, "%s:%d: symbol %.*s does not exist\n", assign->ident->pos.file, assign->ident->pos.line, assign->ident->lex.len, assign->ident->lex.ptr);
            exit(1);
        }

        if (sym->kind == SYM_PROC) {
            fprintf(stderr, "%s:%d: can not reassign procs\n", assign->ident->pos.file, assign->ident->pos.line);
            exit(1);
        }

        type = check_expr(&assign->value, block->idx, st);
        if (sym->type != type) {
            fprintf(stderr, "%s:%d: assigning type %s to %s\n", assign->ident->pos.file, assign->ident->pos.line, scanner_tokstr(type), scanner_tokstr(sym->type));
            exit(1);
        }
		break;
    case STMT_RET:
        ret = stmt->body;

        if (ret->has_value == 1 && block->ret_type == TOK_VOID) {
            fprintf(stderr, "%s:%d: expected type void\n", ret->tok->pos.file, ret->tok->pos.line);
            exit(1);
        }

        if (ret->has_value == 1) {
            type = check_expr(&ret->value, block->idx, st);
            if (block->ret_type != type) {
                fprintf(stderr, "%s:%d: expected type %s but got %s\n", ret->tok->pos.file, ret->tok->pos.line, scanner_tokstr(block->ret_type), scanner_tokstr(type));
                exit(1);
            }
        }
		break;
    case STMT_EXPR:
        stmt_expr = stmt->body;
        check_expr(&stmt_expr->expr, block->idx, st);
		break;

    case STMT_BLOCK:
        assert(0 && "check_stmt STMT_BLOCK: not implemented");
    case STMT_PROC_BLOCK:
        assert(0 && "check_stmt STMT_PROC_BLOCK: should not get there");
    default:
        assert(0 && "unreachable");
    }
}

void check_var(Decl_Var *var, int block_idx, Symtab *st)
{
    Token_Kind type;
    Symbol *var_sym;
    Symbol_Scope scope = SCOPE_BLOCK;

    if (block_idx == -1) {
        scope = SCOPE_FILE;
    }

    var_sym = symtab_insert_var(st, &var->ident->lex, block_idx, scope,
                                var->type->kind);
    if (var_sym == NULL) {
        fprintf(stderr, "%s:%d: var %.*s already defined\n", var->ident->pos.file, var->ident->pos.line, var->ident->lex.len, var->ident->lex.ptr);
        exit(1);
    }

    if (var_sym->type == TOK_VOID) {
        fprintf(stderr, "%s:%d: can not use void for var\n", var->ident->pos.file, var->ident->pos.line);
        exit(1);
    }

    type = check_expr(&var->value, block_idx, st);
    if (type == TOK_NUM) {
        type = var->type->kind;
    }

    if (var->type->kind != type) {
        fprintf(stderr, "%s:%d: expected type %s but got %s\n", var->ident->pos.file, var->ident->pos.line, scanner_tokstr(var->type->kind), scanner_tokstr(type));
        exit(1);
    }
}

void check_proc(Decl_Proc *proc, Symtab *st)
{
    Stmt_Proc_Block *block = proc->body.body;
    Symbol *proc_sym = symtab_insert_proc(st, &proc->ident->lex,
                                          proc->ret_type->kind);

    if (proc_sym == NULL) {
        fprintf(stderr, "%s:%d: proc %.*s already defined\n", proc->ident->pos.file, proc->ident->pos.line, proc->ident->lex.len, proc->ident->lex.ptr);
        exit(1);
    }

    for (int i = 0; i < proc->params.len; ++i) {
        Proc_Param *p = proc->params.buf + i;
        Symbol *param_sym;

        mem_grow(&proc_sym->params);
        proc_sym->params.buf[proc_sym->params.len++] = p->type->kind;

        param_sym = symtab_insert_var(st, &p->ident->lex, proc_sym->idx,
                                      SCOPE_BLOCK, p->type->kind);
        if (param_sym == NULL) {
            fprintf(stderr, "%s:%d: param %.*s already defined\n", p->ident->pos.file, p->ident->pos.line, p->ident->lex.len, p->ident->lex.ptr);
            exit(1);
        }
    }

    for (int i = 0; i < block->decls.len; ++i) {
        Decl_Var *var = block->decls.buf[i].body;
        check_var(var, proc_sym->idx, st);
    }

    for (int i = 0; i < block->stmts.len; ++i) {
        Stmt *s = block->stmts.buf + i;
        check_stmt(s, proc_sym, st);
    }
}

void static_check(Decl *decl, Symtab *st)
{
    Decl_Proc *proc;
    Decl_Var *var;

    switch (decl->kind) {
    case DECL_PROC:
        proc = decl->body;
        check_proc(proc, st);
        break;
    case DECL_VAR:
        var = decl->body;
        check_var(var, -1, st);
        break;
    default:
        assert(0 && "unreachable");
    }
}

int main(int argc, char **argv)
{
    Parser p;
    Scanner s;
    Symtab st;
    Decls decls;

    argc--;
    argv++;

    if (argc < 1) {
        fprintf(stderr, "Provide file to compile\n");
        return 1;
    }

    mem_make(&decls, 256);
    mem_make(&st, 256);
    scanner_make(&s, *argv);
    parser_make(&p, &s);

    parser_parse(&p, &decls);

    for (int i = 0; i < decls.len; ++i) {
        Decl *d = decls.buf + i;
        static_check(d, &st);
    }

    return 0;
}
