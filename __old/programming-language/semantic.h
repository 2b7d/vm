//#include "lib/sstring.h:
//#include "vmc.h:

struct scope;

typedef struct {
    struct scope *scope;
    Stmt_Proc *cur_proc;
} Checker;

void semantic_check(Checker *c, Stmts *stmts);
void semantic_make_checker(Checker *c);
