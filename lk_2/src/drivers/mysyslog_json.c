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

    size_t file_size = lseek(fd, 0, SEEK_END);
    if (file_size == 0)
        write(fd, "[ ", 2);
    else {
        lseek(fd, file_size - 2, SEEK_SET);
        write(fd, ", ", 2);
    }

    if (dprintf(fd, "{\"timestamp\": %lu, \"log_level\": \"%s\", \"process\": \"%s\", \"message\": \"%s\"}", time(NULL), level, application, msg) < 0) {
        write(fd, " ]", 2);
        close(fd);
        return slerror_bad_descriptor;
    }
    write(fd, " ]", 2);

    close(fd);
    return slerror_success;
}
