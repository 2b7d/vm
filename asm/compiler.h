// #include <stddef.h>
// #include <stdint.h>
// #include "scanner.h"

struct token_array {
    size_t size;
    size_t cap;
    struct token *buf;
};

struct chunk {
    enum token_kind name;
    struct {
        size_t size;
        size_t cap;
        uint8_t *buf;
    } data;
};

struct chunk_array {
    size_t size;
    size_t cap;
    struct chunk *buf;
};

struct parser {
    struct token *cur;
    struct token_array toks;
};

void parser_init(struct parser *p, struct scanner *s);
void compile(struct parser *p, struct chunk_array *ca);
