#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "os.h"

int read_file(char *filepath, char **buf)
{
    struct stat statbuf;
    int fd;

    fd = open(filepath, O_RDONLY);
    if (fd < 0) {
        return -1;
    }

    if (fstat(fd, &statbuf) < 0) {
        return -1;
    }

    *buf = malloc(statbuf.st_size + 1);
    if (*buf == NULL) {
        return -1;
    }

    if (read(fd, *buf, statbuf.st_size) < 0) {
        free(*buf);
        return -1;
    }

    close(fd);

    (*buf)[statbuf.st_size] = '\0';
    return statbuf.st_size;
}
