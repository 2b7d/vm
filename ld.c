#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "mem.h"

enum symbol_bind {
    BIND_UNDEF,
    BIND_LOCAL,
    BIND_GLOBAL,
    BIND_EXTERN
};

enum symbol_type {
    TYPE_UNDEF,
    TYPE_REF,
    TYPE_VAR,
    TYPE_FUNC
};

struct symbol {
    char name[256];
    size_t len;
    size_t fid;
    size_t value;
    size_t size;
    size_t tpos;
    enum symbol_bind bind;
    enum symbol_type type;
};

struct symbol_array {
    size_t size;
    size_t cap;
    struct symbol *buf;
};

struct file {
    char *name;
    size_t size;
    char *data;
    size_t datasize;
    char *text;
    size_t textsize;
    char *symtab;
    size_t symtabsize;
};

struct file_array {
    size_t size;
    size_t cap;
    struct file *buf;
};

enum section { S_UNDEF, S_TEXT, S_DATA, S_SYMTAB };

int main(int argc, char **argv)
{
    FILE *out;
    struct symbol_array syms;
    struct file_array files;
    struct symbol *start = NULL;
    size_t c = 0;
    int funcs_relocated = 0, vars_relocated = 0;

    argc--;
    argv++;

    if (argc < 1) {
        puts("object file[s] required");
        return 1;
    }

    meminit(&syms, sizeof(struct symbol), 16); // LEAK: os is freeing
    meminit(&files, sizeof(struct file), 0);   // LEAK: os is freeing

    while (*argv != NULL) {
        struct file *f;
        FILE *in;
        struct stat statbuf;
        char *buf;
        size_t i;
        enum section section;

        memgrow(&files, sizeof(struct file));
        f = files.buf + files.size;
        files.size++;

        f->name = *argv++;
        f->datasize = 0;
        f->textsize = 0;
        f->symtabsize = 0;

        in = fopen(f->name, "r");
        if (in == NULL) {
            perror("fopen failed");
            exit(1);
        }

        if (stat(f->name, &statbuf) < 0) {
            perror("stat failed");
            exit(1);
        }

        f->size = statbuf.st_size;
        f->data = malloc(f->size);   // LEAK: os is freeing
        f->text = malloc(f->size);   // LEAK: os is freeing
        f->symtab = malloc(f->size); // LEAK: os is freeing
        buf = malloc(f->size);       // LEAK: os is freeing

        if (f->data == NULL || f->text == NULL ||
                f->symtab == NULL || buf == NULL) {
            perror("malloc failed");
            exit(1);
        }

        fread(buf, 1, f->size, in);
        if (ferror(in) != 0) {
            fprintf(stderr, "failed to read file %s\n", f->name);
            exit(1);
        }
        fclose(in);

        i = 0;
        section = S_UNDEF;
        while (i < f->size) {
            if (buf[i] == '.') {
                if (memcmp(buf + i, ".data", 5) == 0) {
                    i += 5;
                    section = S_DATA;
                } else if (memcmp(buf+i, ".text", 5) == 0) {
                    i += 5;
                    section = S_TEXT;
                } else if (memcmp(buf+i, ".symtab", 7) == 0) {
                    i += 7;
                    section = S_SYMTAB;
                } else {
                    section = S_UNDEF;
                }
            }

            switch (section) {
            case S_DATA:
                f->data[f->datasize++] = buf[i++];
                break;

            case S_TEXT:
                f->text[f->textsize++] = buf[i++];
                break;

            case S_SYMTAB:
                f->symtab[f->symtabsize++] = buf[i++];
                break;

            case S_UNDEF:
                fprintf(stderr, "invalid object file %s\n", f->name);
                exit(1);
            }
        }

        i = 0;
        while (i < f->symtabsize) {
            struct symbol *s;

            memgrow(&syms, sizeof(struct symbol));
            s = syms.buf + syms.size;
            syms.size++;

            s->fid = c;

            memcpy(&s->value, f->symtab + i, 2);
            i += 2;
            memcpy(&s->size, f->symtab + i, 2);
            i += 2;
            memcpy(&s->tpos, f->symtab + i, 2);
            i += 2;
            memcpy(&s->type, f->symtab + i, 1);
            i += 1;
            memcpy(&s->bind, f->symtab + i, 1);
            i += 1;
            memcpy(&s->len, f->symtab + i, 2);
            i += 2;
            if (s->len > sizeof(s->name)) {
                fprintf(stderr, "symbol name is to long: %ld\n", s->len);
            }
            memcpy(s->name, f->symtab + i, s->len);
            i += s->len;
        }
        c++;
    }

    for (size_t i = 0; i < syms.size; ++i) {
        struct symbol *s = syms.buf + i;
        int found = 0;

        switch (s->bind) {
        case BIND_GLOBAL:
            if (s->len == 6 && memcmp(s->name, "_start", 6) == 0) {
                start = s;
            }

            for (size_t j = i + 1; j < syms.size; ++j) {
                struct symbol *s2 = syms.buf + j;

                if (s->len == s2->len &&
                        memcmp(s->name, s2->name, s2->len) == 0 &&
                        s2->bind == BIND_GLOBAL) {
                    fprintf(stderr, "multiple global symbol %.*s\n",
                                    (int) s->len, s->name);
                    exit(1);
                }
            }
            break;

        case BIND_EXTERN:
            for (size_t j = 0; j < syms.size; ++j) {
                struct symbol *s2;

                if (j == i) {
                    continue;
                }

                s2 = syms.buf + j;

                if (s->len == s2->len &&
                        memcmp(s->name, s2->name, s2->len) == 0 &&
                        s2->bind == BIND_GLOBAL) {
                    found = 1;
                }
            }
            if (found == 0) {
                fprintf(stderr, "undefined extern symbol %.*s\n",
                                (int) s->len, s->name);
                exit(1);
            }
            break;

        default:
            break;
        }
    }

    if (start == NULL) {
        fprintf(stderr, "_start entry point not found\n");
        exit(1);
    }


    {
        size_t offset = 0;
    for (size_t i = 0; i < syms.size; ++i) {
        struct symbol *s = syms.buf + i;

        switch (s->type) {
        case TYPE_VAR:
            if (vars_relocated == 1) {
                break;
            }

            s->value = 0;
            offset = s->size;
            for (size_t j = i + 1; j < syms.size; ++j) {
                struct symbol *s2 = syms.buf + j;

                if (s2->type != TYPE_VAR) {
                    continue;
                }

                s2->value = offset;
                offset += s2->size;
            }

            vars_relocated = 1;
            break;

        default:
            break;
        }
    }

    for (size_t i = 0; i < syms.size; ++i) {
        struct symbol *s = syms.buf + i;

        switch (s->type) {
        case TYPE_FUNC:
            if (funcs_relocated == 1) {
                break;
            }

            s->value = offset;
            offset += s->size;
            for (size_t j = i + 1; j < syms.size; ++j) {
                struct symbol *s2 = syms.buf + j;

                if (s2->type != TYPE_FUNC) {
                    continue;
                }

                s2->value = offset;
                offset += s2->size;
            }

            funcs_relocated = 1;
            break;

        default:
            break;
        }
    }

    }
    puts("\n-- 3 --");
    for (size_t i = 0; i < syms.size; ++i) {
        struct symbol *s = syms.buf + i;
        printf("fid: %3lu, value: %3lu, size: %3lu, tpos: %3ld, "
               "bind: %d, type: %d, %.*s\n", s->fid, s->value, s->size,
                                             s->tpos, s->bind, s->type,
                                             (int) s->len, s->name);
    }

    for (size_t i = 0; i < syms.size; ++i) {
        struct symbol *sdef = syms.buf + i;

        switch (sdef->type) {
        case TYPE_VAR:
        case TYPE_FUNC:
            for (size_t j = 0; j < syms.size; ++j) {
                struct symbol *s = syms.buf + j;
                struct file *f;

                if (j == i || s->type != TYPE_REF) {
                    continue;
                }

                if (sdef->len == s->len &&
                        memcmp(sdef->name, s->name, s->len) == 0) {
                    f = files.buf + s->fid;
                    memcpy(f->text + s->tpos, &sdef->value, 2);
                }
            }
            break;

        default:
            break;
        }
    }

    out = fopen("a.out", "w");
    if (out == NULL) {
        perror("fopen failed");
        exit(1);
    }


    fwrite(&start->value, 2, 1, out);

    for (size_t i = 0; i < files.size; ++i) {
        struct file *f = files.buf + i;
        fwrite(f->data, 1, f->datasize, out);
    }

    for (size_t i = 0; i < files.size; ++i) {
        struct file *f = files.buf + i;
        fwrite(f->text, 1, f->textsize, out);
    }

    fclose(out);

    return 0;
}
