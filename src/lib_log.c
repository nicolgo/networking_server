#include "lib_gutil.h"
#include <stdarg.h>  

void net_log(log_level_enum level, const char* msg)
{
    const char* log_level_str;
    switch (level)
    {
    case LOG_LEVEL_DEBUG:
        log_level_str = "debug";
        break;
    case LOG_LEVEL_MSG:
        log_level_str = "msg";
        break;
    case LOG_LEVEL_WARN:
        log_level_str = "warn";
        break;
    case LOG_LEVEL_ERROR:
        log_level_str = "err";
        break;
    default:
        log_level_str = "???";
        break;
    }
    (void)fprintf(stdout, "<%s> %s\n", log_level_str, msg);
}

void net_logx(log_level_enum level, const char* errstr, const char* fmt, va_list ap)
{
    char buf[1024];
    size_t len;

    if (fmt != NULL)
        vsnprintf(buf, sizeof(buf), fmt, ap);
    else
        buf[0] = '\0';

    if (errstr) {
        len = strlen(buf);
        if (len < sizeof(buf) - 3) {
            snprintf(buf + len, sizeof(buf) - len, ": %s", errstr);
        }
    }

    net_log(level, buf);
}
void net_msgx(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    net_logx(LOG_LEVEL_MSG, NULL, fmt, ap);
    va_end(ap);
}
void net_debugx(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    net_logx(LOG_LEVEL_DEBUG, NULL, fmt, ap);
    va_end(ap);
}