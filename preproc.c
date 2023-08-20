/*
 * define_fn    = "#" "define" ident "(" ident ("," ident)* ")" ANY LF
 * define_const = "#" "define" ident ANY LF
 *
 * ident = char (digit|char)*
 *
 * digit = "0".."9"
 * char  = "a".."z"|"A".."Z"|"_"
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/os.h"
#include "lib/path.h"
#include "lib/mem.h"

typedef struct {
    char *ptr;
    int len;
} string;

void make_string(string *s, char *ptr, int len)
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

struct define {
    enum {
        DEF_CONST = 0,
        DEF_FN
    } kind;

    string name;
    string value;

    struct {
        int len;
        int cap;
        int data_size;
        string *buf;
    } args;
};

struct define_array {
    int len;
    int cap;
    int data_size;
    struct define *buf;
};

int is_lower(char c)
{
    return c >= 'a' && c <= 'z';
}

int is_char(char c)
{
    return is_lower(c) == 1 || (c >= 'A' && c <= 'Z') || c == '_';
}

int is_digit(char c)
{
    return c >= '0' && c <= '9';
}

int is_alnum(char c)
{
    return is_char(c) == 1 || is_digit(c) == 1;
}

int skip_space(char *ptr, int index)
{
    while (ptr[index] == ' ') {
        index++;
    }

    return index;
}

int skip_until_newline(char *ptr, int index)
{
    while (ptr[index] != '\n' && ptr[index] != '\0') {
        index++;
    }

    return index;
}

struct define *defcmp(struct define_array *defs, string *ident)
{
    for (int i = 0; i < defs->len; ++i) {
        struct define *d;

        d = defs->buf + i;
        if (string_cmp(&d->name, ident) == 1) {
            return d;
        }
    }

    return NULL;
}

int main(int argc, char **argv)
{
    struct define_array defs;
    char *src, *newsrc, *outfile;
    int src_len, i, j, is_newline;
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

    newsrc = malloc(src_len);
    if (newsrc == NULL) {
        perror(NULL);
        return 1;
    }

    outfile = create_outfile(*argv, "i");
    if (outfile == NULL) {
        perror(NULL);
        return 1;
    }

    out = fopen(outfile, "w");
    if (out == NULL) {
        perror(NULL);
        return 1;
    }

    meminit(&defs, sizeof(struct define), 32);

    i = 0;
    j = 0;
    is_newline = 1;
    while (src[i] != '\0') {
        struct define *def;
        int begin, end, len;

        if (src[i] == '\n') {
            is_newline = 1;
            newsrc[j++] = src[i++];
            continue;
        }

        if (is_newline == 0) {
            newsrc[j++] = src[i++];
            continue;
        }

        begin = i;
        end = i;

        if (is_newline == 1) {
            end = skip_space(src, end);
            if (src[end] != '#') {
                is_newline = 0;
                len = end - begin;
                memcpy(newsrc + j, src + begin, len);
                j += len;
                i = end;
                continue;
            }
        }

        end++;
        begin = end;

        while (is_lower(src[end]) == 1) {
            end++;
        }

        len = end - begin;
        if (len != 6 || memcmp("define", src + begin, len) != 0) {
            fprintf(stderr, "expected define but got %.*s\n", len, src + begin);
            exit(1);
        }

        def = memnext(&defs);
        meminit(&def->args, sizeof(string), 16);

        end = skip_space(src, end);
        begin = end;

        while (is_alnum(src[end]) == 1) {
            end++;
        }

        len = end - begin;
        make_string(&def->name, src + begin, len);

        end = skip_space(src, end);

        if (src[end] == '(') {
            string *arg;

            def->kind = DEF_FN;
            end++;

            while (src[end] != ')' && src[end] != '\0') {
                end = skip_space(src, end);

                begin = end;
                while (is_alnum(src[end]) == 1) {
                    end++;
                }

                len = end - begin;
                arg = memnext(&def->args);
                make_string(arg, src + begin, len);

                end = skip_space(src, end);
                if (src[end] != ',') {
                    break;
                }

                end++;
            }

            if (src[end] == '\0') {
                fprintf(stderr, "missing closing ')'");
                exit(1);
            }

            if (def->args.len == 0) {
                fprintf(stderr, "empty argument list, use constant define\n");
                exit(1);
            }

            end++;
            end = skip_space(src, end);
        } else {
            def->kind = DEF_CONST;
        }

        begin = end;
        end = skip_until_newline(src, end);
        len = end - begin;
        make_string(&def->value, src + begin, len);

        end++;
        i = end;
        is_newline = 1;
    }

    newsrc[j++] = '\0';

    i = 0;
    src = newsrc;
    while (src[i] != '\0') {
        int begin, end, len;
        struct define *def;
        string str;

        begin = i;
        while (is_char(src[i]) == 0 && src[i] != '\0') {
            switch (src[i]) {
            case '"':
                i++;
                while (src[i] != '"' && src[i] != '\0') {
                    i++;
                }
                if (src[i] == '"') {
                    i++;
                }
                break;
            case '/':
                if (src[i + 1] == '/') {
                    i = skip_until_newline(src, i);
                } else {
                    i++;
                }
                break;
            default:
                i++;
            }
        }
        fwrite(src + begin, 1, i - begin, out);

        begin = i;
        while (is_alnum(src[i]) == 1) {
            i++;
        }

        len = i - begin;
        make_string(&str, src + begin, len);
        def = defcmp(&defs, &str);
        if (def == NULL) {
            fwrite(src + begin, 1, len, out);
            continue;
        }

        if (def->kind == DEF_CONST) {
            fwrite(def->value.ptr, 1, def->value.len, out);
            continue;
        }

        if (def->kind != DEF_FN) {
            fprintf(stderr, "unknown define kind %d\n", def->kind);
            exit(1);
        }

        {
            int step, found;
            struct {
                int len;
                int cap;
                int data_size;
                string *buf;
            } arg_values;

            meminit(&arg_values, sizeof(string), def->args.len);

            step = 0;
            while (step < def->args.len) {
                string *av;

                if (src[i] == '\0') {
                    fprintf(stderr, "not enough arguments\n");
                    exit(1);
                }

                av = memnext(&arg_values);
                i = skip_space(src, i);
                begin = i;
                while (src[i] != ' ' && src[i] != '\n' && src[i] != '\0') {
                    i++;
                }
                make_string(av, src + begin, i - begin);
                step++;
            }

            begin = 0;
            end = 0;
            while (begin < def->value.len) {
                end = skip_space(def->value.ptr, end);
                fwrite(def->value.ptr + begin, 1, end - begin, out);

                begin = end;
                while (def->value.ptr[end] != ' ' && end < def->value.len) {
                    end++;
                }
                make_string(&str, def->value.ptr + begin, end - begin);

                found = 0;
                for (int i = 0; i < def->args.len; ++i) {
                    string *arg, *arg_val;

                    arg = def->args.buf + i;
                    if (string_cmp(arg, &str) == 1) {
                        found = 1;
                        arg_val = arg_values.buf + i;
                        fwrite(arg_val->ptr, 1, arg_val->len, out);
                        break;
                    }
                }

                if (found == 0) {
                    fwrite(str.ptr, 1, str.len, out);
                }

                begin = end;
            }
        }
    }

    fclose(out);
    return 0;
}
