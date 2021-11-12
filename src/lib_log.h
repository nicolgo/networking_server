#ifndef LIB_LOG_H
#define LIB_LOG_H

typedef enum log_level_enum{
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_MSG,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR
}log_level_enum;

void net_log(log_level_enum level,const char*msg);
void net_logx(log_level_enum level,const char* errstr, const char *fmt,va_list ap);
void net_msgx(const char *fmt,...);
void net_debugx(const char *fmt,...);

#define LOG_MSG(msg) net_log(LOG_LEVEL_MSG,msg)
#define LOG_ERROR(msg) net_log(LOG_LEVEL_ERROR,msg)


#endif