#ifndef __FMS_LOG_H__
#define __FMS_LOG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <unistd.h>
#include <stdarg.h>

#include "fms_type.h"
#include "fms_log_config.h"

//need?useful?
#ifndef FMS_LOG_TAG
#define FMS_LOG_TAG NULL
#endif


enum {
    FMS_LOG_DEBUG = 0,
    FMS_LOG_INFO,
    FMS_LOG_WARN,
    FMS_LOG_ERROR,
    FMS_LOG_FATAL,
};



#ifdef FMS_NDEBUG
#ifndef LOG_PRINT_THRESHOLD
#define LOG_PRINT_THRESHOLD FMS_LOG_DEBUG
#endif
#ifndef LOG_WRITE_THRESHOLD
#define LOG_WRITE_THRESHOLD FMS_LOG_DEBUG
#endif
#else
#ifndef LOG_PRINT_THRESHOLD
#define LOG_PRINT_THRESHOLD FMS_LOG_WARN
#endif
#ifndef LOG_WRITE_THRESHOLD
#define LOG_WRITE_THRESHOLD FMS_LOG_WARN
#endif
#endif


fms_void dsprint(fms_s8 *const str, const fms_s8 *fmt, ...); 
fms_void fms_log_output(const fms_u8 priority, const fms_u8 print_threshold, 
							  const fms_u8 write_threshold, const fms_s8 *const msg);
fms_void fms_log_init(const fms_s32 size, const fms_s8 *path);	


//日期+时间+线程ID+(TAG) +LEVEL+文件名+行号+函数名+错误信息
//{DATE TIME} ID (TAG)   [LEVEL] <FILE:LINE:FUNCTION > MSG

#define FMS_OUT(priority, ...) \
	do { \
        fms_s8 buff[LOG_MAX_SIZE] = {0}; \
        dsprint(buff, "[%s] <%s:%d:%s> ", "FATAL", __FILE__, __LINE__, __FUNCTION__); \
        dsprint(&buff[strlen(buff)],  ##__VA_ARGS__);\
        fms_log_output(priority, LOG_PRINT_THRESHOLD, LOG_WRITE_THRESHOLD, buff); \
    } while(0)

#define FMS_DEBUG(...) FMS_OUT(FMS_LOG_DEBUG, ##__VA_ARGS__)
#define FMS_INFO(...) FMS_OUT(FMS_LOG_INFO, ##__VA_ARGS__)
#define FMS_WARN(...) FMS_OUT(FMS_LOG_WARN, ##__VA_ARGS__)
#define FMS_ERROR(...) FMS_OUT(FMS_LOG_ERROR, ##__VA_ARGS__)
#define FMS_FATAL(...) FMS_OUT(FMS_LOG_FATAL, ##__VA_ARGS__)


#define FMS_EQUAL_RETURN(arg1, arg2)\
    do {\
        if ((arg1) == (arg2)) {\
            FMS_ERROR(#arg1"=="#arg2" return\n");\
			return;\
        }\
    } while(0);


#define FMS_EQUAL_RETURN_VALUE(arg1, arg2, ret)\
    do {\
        if ((arg1) == (arg2)) {\
            FMS_ERROR(#arg1"=="#arg2" return "#ret"\n");\
            return (ret);\
        }\
    } while(0);

#define FMS_ASSERT(expression)\
    do {\
        if ((expression) != (FMS_TRUE)) {\
            FMS_FATAL(#expression" is false\n");\
			abort();\
        }\
    } while(0);



#ifdef __cplusplus
}
#endif

#endif
