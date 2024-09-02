#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "path.h"

static char *default_ext = "out";

char *create_outfile(char *in, char *ext)
{
    char *out;
    int in_len, ext_len, out_len, offset;

    if (ext == NULL) {
        ext = default_ext;
    }

    in = path_base(in);
    in_len = strlen(in);
    ext_len = strlen(ext);
    out_len = in_len + ext_len + 1; // art: +1 is for '.'

    out = malloc(out_len + 1);
    if (out == NULL) {
        perror("create_outfile malloc");
        exit(1);
    }

    offset = in_len - 1;
    while (in[offset] != '.' && offset >= 0) {
        offset--;
    }

    if (offset < 0) {
        offset = in_len;
    }

    memcpy(out, in, offset);
    out[offset++] = '.';
    memcpy(out + offset, ext, ext_len);

    offset += ext_len;
    out[offset] = '\0';

    free(in);
    return out;
}

char *path_base(char *path)
{
    char *base;
    int len, base_len, offset;

    len = strlen(path);
    offset = len - 1;

    while (path[offset] != '/' && offset >= 0) {
        offset--;
    }

    if (offset < 0) {
        offset = 0;
    } else {
        offset++;
    }

    base_len = len - offset;
    base = malloc(base_len + 1);
    if (base == NULL) {
        perror("path_base malloc");
        exit(1);
    }

    memcpy(base, path + offset, base_len);
    base[base_len] = 0;

    return base;
}
