#ifndef USE_BACKEND_MYSYSLOG
#error "This file should not be used directly. Use mysyslog API function instead"
#endif

#ifndef MYSYSLOG_BACK_H
#define MYSYSLOG_BACK_H
#include <stdlib.h>
#include <unistd.h>

#include "mysyslog_makefile.h"

#define LOGS_FILE_FLAGS O_CREAT | O_WRONLY
// #define LOGS_FILE_FLAGS O_CREAT | O_WRONLY | O_APPEND
#define LOGS_FILE_PERMS 0660

typedef struct {
    int         enum_val;
    const char *str;
} enum_str_pair;

enum_str_pair driver_so_filename_str[] = {
    { drv_ascii,    "libmysyslog_text.so" },
    { drv_json,     "libmysyslog_json.so" },
};
unsigned long int driver_so_filename_amount = sizeof(driver_so_filename_str) / sizeof(enum_str_pair);

enum_str_pair driver_names[] = {
    { drv_ascii,    "ascii" },
    { drv_json,     "json" }
};
unsigned long int driver_names_len = sizeof(driver_names) / sizeof(enum_str_pair);

enum_str_pair level_str[] = {
    { loglvl_DEBUG,     "DEBUG" },
    { loglvl_INFO,      "INFO" },
    { loglvl_WARN,      "WARN" },
    { loglvl_ERROR,     "ERROR" },
    { loglvl_CRITICAL,  "CRITICAL" }
};
unsigned long int levels_amount = sizeof(level_str) / sizeof(enum_str_pair);

enum_str_pair format_str[] = {
    { fmt_none, "" },
    { fmt_txt,  ".txt" },
    { fmt_log,  ".log" },
    { fmt_json, ".json" },
};
unsigned long int formats_amount = sizeof(format_str) / sizeof(enum_str_pair);

const char *str_from_enum(int enum_val, const enum_str_pair *array, size_t array_size) {
    const char *str = NULL;
    for (size_t i = 0; i < array_size; ++i) {
        if (enum_val == array[i].enum_val) {
            str = array[i].str;
            break;
        }
    }
    return str;
}

int enum_from_str(const char *str, const enum_str_pair *array, size_t array_size) {
    if (str == NULL)
        return -1;
    
    int enum_val = -1;
    for (size_t i = 0; i < array_size; ++i) {
        if (strcmp(array[i].str, str) == 0) {
            enum_val = array[i].enum_val;
            break;
        }
    }
    return enum_val;
}

void get_process_name(char *path, size_t pathlen) {
    memset(path, 0, pathlen);
    readlink("/proc/self/exe", path, pathlen);
}

#endif
