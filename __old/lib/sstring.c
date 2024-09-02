#include <stdlib.h>
#include <stdio.h>

#include "sstring.h"

static int cstr_len(char *s)
{
    int len;

    len = 0;
    while (s[len] != '\0') {
        len++;
    }

    return len;
}

void string_make(string *s, char *ptr, int len)
{
    s->ptr = ptr;
    s->len = len;
}

void string_fromc(string *s, char *cstr)
{
    int len;

    len = cstr_len(cstr);
    string_make(s, cstr, len);
}

void string_init(string *s, int len)
{
    s->len = len;
    s->ptr = malloc(len);

    if (s->ptr == NULL) {
        perror("string_init malloc");
        exit(1);
    }
}

void string_init_fromc(string *s, char *cstr)
{
    int len;

    len = cstr_len(cstr);
    string_init(s, len);

    for (int i = 0; i < len; ++i) {
        s->ptr[i] = cstr[i];
    }
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
    return string_cmp(s, &s2);
}

char *string_toc(string *s)
{
    char *c;

    c = malloc(s->len + 1);
    if (c == NULL) {
        perror("string_toc");
        exit(1);
    }

    for (int i = 0; i < s->len; ++i) {
        c[i] = s->ptr[i];
    }

    c[s->len] = '\0';
    return c;
}
