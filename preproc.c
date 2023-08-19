#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/os.h"
#include "lib/mem.h"

struct define {
    char *name;
    int name_len;
    char *value;
    int value_len;
    int has_args;
    int args_len;
    struct { char *name; int len; } args[10];
};

struct define_array {
    int len;
    int cap;
    int data_size;
    struct define *buf;
};

struct output {
    int len;
    int cap;
    int data_size;
    char *buf;
};

struct define *defcmp(struct define_array *defs, char *str, int str_len)
{
    for (int i = 0; i < defs->len; ++i) {
        struct define *d;

        d = defs->buf + i;
        if (d->name_len == str_len && memcmp(d->name, str, str_len) == 0) {
            return d;
        }
    }

    return NULL;
}

char *skip_whitespace(char *s)
{
    while (*s == ' ' || *s == '\t') {
        s++;
    }

    return s;
}

int main(int argc, char **argv)
{
    struct define_array defs;
    struct output output;
    struct define *def;
    char *src, *start;
    int len, src_len;
    FILE *out;

    argc--;
    argv++;

    if (argc < 1) {
        fprintf(stderr, "Provide file to preprocess\n");
        return 1;
    }

    src_len = read_file(*argv, &src);
    if (src_len < 0) {
        perror(NULL);
        return 1;
    }

    out = fopen("out.i", "w");
    if (out == NULL) {
        perror(NULL);
        return 1;
    }

    meminit(&defs, sizeof(struct define), 32);
    meminit(&output, 1, src_len + 256);


define_scan:
    switch (*src) {
    case ' ':
    case '\t':
    case '\r':
    case '\n':
        src++;
        goto define_scan;
    case '#':
        src++;
        start = src;
        while (*src >= 'a' && *src <= 'z') {
            src++;
        }
        len = src - start;
        if (len != 6 || memcmp("define", start, len) != 0) {
            fprintf(stderr, "expected define but got %.*s\n", len, start);
            exit(1);
        }
        src = skip_whitespace(src);
        start = src;
        while (*src != ' ' && *src != '\0') {
            if (*src == '\n') {
                fprintf(stderr, "unexpected new line\n");
                exit(1);
            }
            src++;
        }
        len = src - start;

        def = memnext(&defs);
        def->name = start;
        def->name_len = len;
        def->has_args = 0;
        def->args_len = 0;

        src = skip_whitespace(src);
        start = src;

        if (*src == '(') {
            src++;
            def->has_args = 1;
            while (*src != ')' && *src != '\0') {
                int i;

                i = 0;
                for (;;) {
                    if (i >= 10) { // TODO(art): magic number
                        fprintf(stderr, "only 10 arguments supported\n");
                        exit(1);
                    }
                    start = src;
                    def->args[i].name = start;
                    while (*src >= 'a' && *src <= 'z') {
                        src++;
                    }
                    def->args[i].len = src - start;
                    i++;
                    if (*src != ' ') {
                        break;
                    }
                    src++;
                }
                def->args_len = i;
            }
            if (*src == '\0') {
                fprintf(stderr, "missing closing ')'\n");
                exit(1);
            }
            src++;
        }

        src = skip_whitespace(src);
        start = src;
        while (*src != '\n' && *src != '\0') {
            src++;
        }
        len = src - start;

        def->value = start;
        def->value_len = len;

        goto define_scan;
    default:
        goto define_done;
    }
define_done:

    while (*src) {
        struct define *def;
        int old_len;

        start = src;
        while (*src == ' ' || *src == '\n' || *src == '\t' || *src == '\r') {
            src++;
        }
        len = src - start;
        if (len > 0) {
            old_len = output.len;
            output.len += len;
            memgrow(&output);
            memcpy(output.buf + old_len, start, len);
        }

        start = src;
        while (*src != ' ' && *src != '\n' && *src != '\0') {
            src++;
        }
        len = src - start;

        def = defcmp(&defs, start, len);
        if (def != NULL) {
            if (def->has_args) {
                struct { char *value; int len; } arg_values[10];
                char *end, *b, *e;
                int i, found;

                i = 0;
                while (i < def->args_len) {
                    src = skip_whitespace(src);
                    arg_values[i].value = src;
                    while (*src != ' ' && *src != '\n' && *src != '\0') {
                        src++;
                    }
                    arg_values[i].len = src - arg_values[i].value;
                    i++;
                }

                start = def->value;
                end = def->value + def->value_len;
                b = def->value;
                e = def->value;
                while (e < end) {
                    while (*e != ' ' && *e != '\n' && e < end) {
                        e++;
                    }
                    len = e - b;
                    found = 0;
                    for (int i = 0; i < def->args_len; ++i) {
                        if (len == def->args[i].len &&
                                memcmp(b, def->args[i].name, len) == 0) {
                            found = 1;
                            old_len = output.len;
                            output.len += arg_values[i].len;
                            memgrow(&output);
                            memcpy(output.buf + old_len, arg_values[i].value,
                                   arg_values[i].len);
                            break;
                        }
                    }

                    if (found == 0) {
                        old_len = output.len;
                        output.len += len;
                        memgrow(&output);
                        memcpy(output.buf + old_len, b, len);
                    }
                    memgrow(&output);
                    output.buf[output.len++] = *e++;
                    b = e;
                }
            } else {
                old_len = output.len;
                output.len += def->value_len;
                memgrow(&output);
                memcpy(output.buf + old_len, def->value, def->value_len);
            }
        } else {
            old_len = output.len;
            output.len += len;
            memgrow(&output);
            memcpy(output.buf + old_len, start, len);
        }
    }

    fwrite(output.buf, 1, output.len, out);

    return 0;
}
