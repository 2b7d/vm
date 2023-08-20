#include "sstring.h"

void string_make(string *s, char *ptr, int len)
{
    s->ptr = ptr;
    s->len = len;
}

int string_cmp(string *s1, string *s2)
{
    if (s1->len != s2->len) {
        return 0;
    }

    for (int i = 0; i < s1->len; ++i) {
        if (s1->ptr[i] != s2->ptr[i]) {
            return 0;
        }
    }

    return 1;
}
