/** @file mysyslog.h
 *  @brief Describes mysyslog API
*/

#ifndef MYSYSLOG_H
#define MYSYSLOG_H

/** @enum e_driver
 *  @brief This enum defines logging format drivers. 
 */
typedef enum {
    drv_ascii,
    drv_json
} e_driver;

/** @enum e_level
 *  @brief This enum defines type of displayed message. 
 */
typedef enum {
    loglvl_DEBUG,
    loglvl_INFO,
    loglvl_WARN,
    loglvl_ERROR,
    loglvl_CRITICAL
} e_level;

/** @enum e_format
 *  @brief This enum defines logging file output format. 
 */
typedef enum {
    fmt_none,
    fmt_txt,
    fmt_log,
    fmt_json
} e_format;

/** @enum e_syslog_error
 *  @brief This enum defines return values of mysyslog function.
 */
typedef enum {
    slerror_success,
    slerror_invalid_args,
    slerror_no_mod,
    slerror_bad_descriptor
} e_syslog_error;

/** @fn int mysyslog(const char *msg, int level, int driver, int format, const char *path)
 *  @brief An interface function for logging.
 *  @param msg      a message displayed in logs;
 *  @param level    type of a message (see `e_level`);
 *  @param driver   driver defines output format (see `e_driver`);
 *  @param format   format defines file extension (see `e_format`);
 *  @param path     path to output log file;
 *  @returns On error returns a non-0 value from `e_syslog_error` and sets `errno` appropriately; 
 */
extern int mysyslog(const char *msg, int level, int driver, int format, const char *path);

#endif
