#include <stdlib.h>
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

    in_len = strlen(in);
    ext_len = strlen(ext);
    out_len = in_len + ext_len + 1; // art: +1 is for '.'

    out = malloc(out_len + 1);
    if (out == NULL) {
        return NULL;
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

    return out;
}
