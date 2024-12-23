#include <errno.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mysyslog_back.h"

char proc_full_name[4096];
size_t proc_name_len = sizeof(proc_full_name);

int mysyslog(const char *msg, int level, int driver, int format, const char *path) {
    // Parameters validation
    if (msg == NULL || path == NULL) {
        errno = EINVAL;
        return slerror_invalid_args;
    }

    // Converting enums to strings
    const char *module_name = str_from_enum(driver, driver_so_filename_str, driver_so_filename_amount);
    if (module_name == NULL) {
        errno = EINVAL;
        return slerror_invalid_args;
    }

    const char *lvl_str = str_from_enum(level, level_str, levels_amount);
    if (lvl_str == NULL) {
        errno = EINVAL;
        return slerror_invalid_args;
    }

    const char *fmt_str = str_from_enum(format, format_str, formats_amount);
    if (fmt_str == NULL) {
        errno = EINVAL;
        return slerror_invalid_args;
    }

    // Getting application full name (with path)
    size_t total_len = strlen(path) + strlen(fmt_str);
    char *full_path = malloc(total_len);
    strncpy(full_path, path, total_len);
    strcat(full_path, fmt_str);

    // Setting application name once
    if (proc_full_name[0] == 0)
        get_process_name(proc_full_name, proc_name_len);

    // Defining function pointer
    int (*mysyslog_function)(const char *, const char *, const char *, const char *);

    // Loading SO driver
    void *so_handle = dlopen(module_name, RTLD_LAZY | RTLD_LOCAL);
    if (so_handle == NULL) {
        free(full_path);
        fputs(dlerror(), stderr);
        return slerror_no_mod;
    }

    // Loading SO function
    mysyslog_function = dlsym(so_handle, "mysyslog_back");
    if (mysyslog == NULL) {
        free(full_path);
        fputs(dlerror(), stderr);
        dlclose(so_handle);
        return slerror_no_mod;
    }
    
    // Logging
    int error = mysyslog_function(msg, lvl_str, proc_full_name, full_path);
    if (error != slerror_success) {
        free(full_path);
        fputs(strerror(errno), stderr);
        dlclose(so_handle);
        return error;
    }

    free(full_path);
    dlclose(so_handle);
    return slerror_success;
}
