#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>

#include "util.h"

char *read_file(char *pathname)
{
    int fd;
    struct stat statbuf;
    char *src;

    fd = open(pathname, O_RDONLY);
    if (fd < 0) {
        perror("failed to open file");
        exit(1);
    }

    if (fstat(fd, &statbuf) < 0) {
        perror("failed to get file info");
        exit(1);
    }

    src = malloc(statbuf.st_size + 1);
    if (src == NULL) {
        perror("failed to allocate memory for source");
        exit(1);
    }

    if (read(fd, src, statbuf.st_size) < 0) {
        perror("failed read file content into source");
        exit(1);
    }

    src[statbuf.st_size] = '\0';

    close(fd);
    return src;
}

char *create_outpath(char *inpath, char *ext)
{
    int extlen = 0;

    char *path, *start, *end, *ch;
    int len;

    start = inpath;
    end = start + strlen(inpath);

    ch = strrchr(inpath, '/');
    if (ch != NULL) {
        start = ch + 1;
    }

    ch = strrchr(inpath, '.');
    if (ch != NULL) {
        end = ch;
    }

    len = end - start;
    if (ext != NULL) {
        extlen = strlen(ext);
    }

    path = malloc(len + extlen + 1);
    if (path == NULL) {
        perror("create_outpath: malloc failed");
        exit(1);
    }

    memcpy(path, start, len);
    if (ext != NULL) {
        path[len] = '.';
        ++len;
        memcpy(path + len, ext, extlen);
    }

    path[len + extlen] = '\0';
    return path;
}
