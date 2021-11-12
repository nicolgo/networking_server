#include "lib_gutil.h"

void net_log(log_level_enum level,const char*msg)
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
    (void)fprintf(stdout,"<%s> %s\n",log_level_str,msg);
}