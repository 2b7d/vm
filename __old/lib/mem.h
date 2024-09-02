// #include <stdio.h>
// #include <stdlib.h>

#define mem_make(m, c) do {                          \
    (m)->len = 0;                                    \
    (m)->cap = (c);                                  \
    (m)->buf = malloc((m)->cap * sizeof(*(m)->buf)); \
    if ((m)->buf == NULL) {                          \
        perror("mem_make");                          \
        exit(1);                                     \
    }                                                \
} while (0)

#define mem_grow(m) do {                                            \
    if ((m)->len >= (m)->cap) {                                     \
        (m)->cap = (m)->len * 2;                                    \
        (m)->buf = realloc((m)->buf, (m)->cap * sizeof(*(m)->buf)); \
        if ((m)->buf == NULL) {                                     \
            perror("mem_grow");                                     \
            exit(1);                                                \
        }                                                           \
    }                                                               \
} while (0)

#define mem_next(m) (m)->buf + (m)->len++
#define mem_free(m) free((m)->buf)
