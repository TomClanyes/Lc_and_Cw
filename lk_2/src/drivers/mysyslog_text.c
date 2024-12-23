#include <time.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include "mysyslog_back.h"

extern int mysyslog_back(const char *msg, const char *level, const char *application, const char *path) {
    int fd = open(path, LOGS_FILE_FLAGS, LOGS_FILE_PERMS);
    if (fd == -1)
        return slerror_invalid_args;

    lseek(fd, 0, SEEK_END);
    if (dprintf(fd, "%lu %s %s: %s\n", time(NULL), level, application, msg) < 0) {
        close(fd);
        return slerror_bad_descriptor;
    }

    close(fd);
    return slerror_success;
}
