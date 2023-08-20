#include <stdlib.h>
#include <stdio.h>

#include "sstring.h"

void string_make(string *s, char *ptr, int len)
{
    s->ptr = ptr;
    s->len = len;
}

void string_fromc(string *s, char *cstr)
{
    int len;

    len = 0;
    while (cstr[len] != '\0') {
        len++;
    }

    string_make(s, cstr, len);
}

void string_dup(string *dst, string *src)
{
    dst->ptr = src->ptr;
    dst->len = src->len;
}

void string_cpy(string *dst, string *src)
{
    dst->len = src->len;
    dst->ptr = malloc(dst->len);

    if (dst->ptr == NULL) {
        perror("string_copy");
        exit(1);
    }

    for (int i = 0; i < dst->len; ++i) {
        dst->ptr[i] = src->ptr[i];
    }
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

int string_cmpc(string *s, char *cstr)
{
    string s2;

    string_fromc(&s2, cstr);
    return string_cmp(s, s2);
}
