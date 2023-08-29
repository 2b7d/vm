#include <assert.h>

#include "token.h"

char *token_str(Token_Kind kind)
{
    switch (kind) {
    case TOK_IDENT:
		return "identifier";
    case TOK_NUM:
		return "number";

    case TOK_EQ:
		return "=";

    case TOK_PLUS:
		return "+";
    case TOK_MINUS:
		return "-";

    case TOK_COMMA:
		return ",";
    case TOK_COLON:
		return ":";
    case TOK_SEMICOLON:
		return ";";

    case TOK_LPAREN:
		return "(";
    case TOK_RPAREN:
		return ")";
    case TOK_LCURLY:
		return "{";
    case TOK_RCULRY:
		return "}";

    case TOK_VAR:
		return "var";
    case TOK_PROC:
		return "proc";
    case TOK_RET:
		return "return";
    case TOK_EXTERN:
		return "extern";
    case TOK_GLOBAL:
		return "global";

    case TOK_VOID:
		return "void";
    case TOK_U16:
		return "u16";
    case TOK_U8:
		return "u8";

    case TOK_EOF:
		return "<end of file>";
    default:
        assert(0 && "unreachable");
    }
}

int token_is_storage(Token_Kind kind)
{
    return kind > TOK_storage_begin && kind < TOK_storage_end;
}

int token_is_type(Token_Kind kind)
{
    return kind > TOK_type_begin && kind < TOK_type_end;
}
