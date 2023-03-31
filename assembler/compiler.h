enum sym_type { TYPE_LOCAL, TYPE_GLOBAL, TYPE_EXTERN };

struct token_array {
    size_t size;
    size_t cap;
    size_t data_size;
    struct token *buf;
};

struct sym {
    char *name;
    size_t namelen;
    unsigned short value;
    int is_resolved;
    enum sym_type type;
    unsigned long index;
};

struct sym_array {
    size_t size;
    size_t cap;
    size_t data_size;
    struct sym *buf;
};

struct rel {
    unsigned short loc;
    unsigned short ref;
    int is_resolved;
    char *name;
    size_t namelen;
};

struct rel_array {
    size_t size;
    size_t cap;
    size_t data_size;
    struct rel *buf;
};

struct parser {
    struct token *cur;
    struct sym_array sa;
    struct rel_array ra;
    struct token_array toks;
    struct {
        size_t size;
        size_t cap;
        uint8_t *buf;
    } code;
};

void compile(struct parser *p);
