// #include "lib/sstring.h"
// #include "vm.h"

struct ln_header {
    int nsyms;
    int nrels;
};

enum ln_symkind {
    SYM_LOCAL = 0,
    SYM_GLOBAL,
    SYM_EXTERN
};

struct ln_symbol {
    enum ln_symkind kind;
    enum vm_section sec;
    int idx;
    int addr;
    string label;
};

struct ln_relocation {
    int loc;
    int symidx;
};
