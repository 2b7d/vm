// #include <stdint.h>
// #include "scanner.h"

enum sym_type { TYPE_LOCAL, TYPE_GLOBAL, TYPE_EXTERN };

struct sym {
    enum sym_type type;

    char *name;
    int namelen;

    int value;
    int index;

    int is_resolved;
};

struct sym_array {
    int size;
    int cap;
    int data_size;
    struct sym *buf;
};

struct rel {
    char *name;
    int namelen;

    int loc;
    int ref;

    int is_resolved;
};

struct rel_array {
    int size;
    int cap;
    int data_size;
    struct rel *buf;
};

struct parser {
    struct token *cur;
    struct token_array ta;

    struct sym_array sa;
    struct rel_array ra;
    struct {
        int size;
        int cap;
        int data_size;
        uint8_t *buf;
    } code;
};

void parser_init(struct parser *p);
void compile(struct parser *p);
void write_object_file(struct parser *p, char *outpath);
