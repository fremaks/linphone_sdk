#include <memory>
#include <time.h>
#include <pthread.h>
#include "fms_log.h"
#include "fms_dir.h"
#include "FmsSpinLock.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef OS_ANDROID
#include <android/log.h>
#endif

struct fms_log {
	~fms_log();
	FILE *fp;
	pid_t pid;
	fms_s32 file_cur_ize;
	fms_s32 file_max_size;
	fms_s8 file_path[PATH_MAX_LEN];
};


#define LOG_ERROR(...)\
    do { \
        fprintf(stderr, "[ERR<%s,%d>]\t", __FILE__, __LINE__);\
        fprintf(stderr, __VA_ARGS__);\
    } while(0)
    
#define LOG_EQUAL_RETURN(arg1, arg2)\
    do {\
        if ((arg1) == (arg2)) {\
            LOG_ERROR(#arg1"=="#arg2"\n");\
			return;\
        }\
    } while(0);
	
#define LOG_EQUAL_RETURN_VALUE(arg1, arg2, ret)\
    do {\
        if ((arg1) == (arg2)) {\
            LOG_ERROR(#arg1"=="#arg2"\n");\
            return (ret);\
        }\
    } while(0);	
	
#define LOG_ASSERT(expression)\
    do {\
        if ((expression) != (FMS_TRUE)) {\
            LOG_ERROR(#expression" is false\n");\
			abort();\
        }\
    } while(0);

	
static std::auto_ptr<fms_log> scopePtr(NULL);
static fms_atomic g_log_lock = SPIN_LOCK_INIT_VALUE;
static fms_log *g_log = NULL;
static fms_s32 g_file_size = 0;
static fms_s8 g_file_path[PATH_MAX_LEN] = {0};



static const fms_s32 get_self_app_name(fms_s8 *const name, const fms_s32 len) {
	fms_s8 *pos = NULL;
	fms_s8 tmp_name[APP_NAME_MAX_LEN] = {0};
	fms_u32 tmp_len = 0;

	LOG_EQUAL_RETURN_VALUE(name, NULL, FMS_FAILED);
	LOG_EQUAL_RETURN_VALUE(len <= 0, FMS_TRUE, FMS_FAILED);
	
	tmp_len = readlink("/proc/self/exe", tmp_name, len);
	
	LOG_EQUAL_RETURN_VALUE(tmp_len <= 0 || tmp_len > len, FMS_TRUE, FMS_FAILED);

	pos = strrchr(tmp_name, '/');
	LOG_EQUAL_RETURN_VALUE(pos, NULL, FMS_FAILED);
	
	strcpy(name, ++pos);

	return FMS_SUCCESS;
}

static fms_log *fms_log_new(const fms_s32 file_size, const fms_s8 *file_path) {
	fms_s8 app_name[64] = {0};
	fms_s8 log_file_full_path[PATH_MAX_LEN] = {0};
	fms_log *log = NULL;

	log = (fms_log *)malloc(sizeof(fms_log));
	LOG_ASSERT(log != NULL);
	
	if (file_size <= 0) {
		log->file_max_size = DEF_LOG_FILE_SIZE;
	} else {
		log->file_max_size = file_size;
	}

	if (get_self_app_name(app_name, sizeof(app_name)) != FMS_SUCCESS) {
		LOG_ERROR("get_self_app_name fail\n");
		strcpy(app_name, DEF_APP_NAME);
	}

	log->pid = getpid();
	if (log->pid <= 1) {
		LOG_ERROR("getpid fail\n");
		log->pid = DEF_PID;
	}
	
	if (NULL == file_path || file_path[0] == '\0' || fms_dir_create(file_path, 0777) != FMS_SUCCESS) {
		file_path = DEF_LOG_FILE_PATH;	
	} 
	
	sprintf(log_file_full_path, "%s%s_%d.log", file_path, app_name, log->pid);
	strcpy(log->file_path, log_file_full_path);

	return log;
}

fms_log::~fms_log() {
	if (g_log != NULL) {
		if (g_log->fp != NULL) {
			fclose(g_log->fp);
		}
	}
}


static fms_void fms_log_write(fms_log *const log, const fms_s8 *const msg) {
	struct tm *tm = NULL;
	time_t timep;
	fms_s8 log_msg[LOG_MAX_SIZE] = {0};

	//LOG_EQUAL_RETURN(log, NULL); have checked before
	//LOG_EQUAL_RETURN(msg, NULL); have checked before
	
	if (time(&timep) != -1) {
		tm = localtime(&timep);
		if (tm != NULL) {
			memset(&log_msg, 0, sizeof(log_msg));
			sprintf(log_msg, "{%04d-%02d-%02d %02d:%02d:%02d} %lu %s", 
					tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
					tm->tm_hour, tm->tm_min, tm->tm_sec, pthread_self(), msg);
		} 
	}
	printf("##################fms_log_write 0, log=%p\n", log);
	if (NULL == log->fp) {
		printf("###################fms_log_write:%s\n", log->file_path);
		log->fp = fopen(log->file_path, "w+"); 	
		printf("###################fms_log_write:log->fp=%p\n", log->fp);
		LOG_EQUAL_RETURN(log->fp, NULL);
		log->file_cur_ize = 0;
	}
	printf("##################fms_log_write 0-1,log->fp=%p\n", log->fp);
	fprintf(log->fp, "%s", log_msg);
	printf("##################fms_log_write 0-2\n");
	fflush(log->fp);
	printf("##################fms_log_write 0-3\n");
	log->file_cur_ize += strlen(log_msg);
	printf("##################fms_log_write:%d\n", log->file_cur_ize);
	if (log->file_cur_ize > log->file_max_size) {
		fms_s32 offset = 0;
		fms_s8 buff[LOG_MAX_SIZE] = {0};
		fms_s32 remainLen = 0;
		fms_s32 startLen = 0;
		fms_s32 endLen = 0;
		fms_s8 *remainBuff = NULL;
		
		fseek(log->fp, log->file_cur_ize/2, SEEK_SET);
		offset = ftell(log->fp);
		printf("##################fms_log_write 1\n");
		if (fgets(buff, LOG_MAX_SIZE, log->fp) != NULL) {
			if (buff[0] != '{' || buff[20] != '}') { // not a full record
				offset += strlen(buff);
			} 
		} 
		printf("##################fms_log_write 2\n");
		fseek(log->fp, offset, SEEK_SET);
		remainLen =  log->file_cur_ize - offset; 

		if (remainLen > 0) {
			remainBuff = (fms_s8 *)malloc(remainLen);
			if (remainBuff != NULL) {
				memset(remainBuff, 0, remainLen);
				remainLen = fread(remainBuff, 1, remainLen, log->fp);
			} else {
				remainLen = 0;
			}
		}
		printf("##################fms_log_write 3\n");
		fclose(log->fp);
		log->fp = NULL;
		log->file_cur_ize = 0;
		
		log->fp = fopen(log->file_path, "w+");
		if (NULL == log->fp) {
			LOG_ERROR("fopen %s fail\n", log->file_path);
			return;
		} 
		printf("##################fms_log_write 4\n");
		if (remainLen > 0) {
			log->file_cur_ize += fwrite(remainBuff, 1, remainLen, log->fp);
		}
		printf("##################fms_log_write 5\n");
	}
	printf("##################fms_log_write 6\n");
}


fms_void fms_log_init(const fms_s32 size, const fms_s8 *const path) {
	FmsSpinLock spinLock(&g_log_lock);

	LOG_EQUAL_RETURN(size <= 0, FMS_TRUE);
	
	if (size > 0) {
		g_file_size = size;
	} 
	
	LOG_EQUAL_RETURN(path, NULL);
	LOG_EQUAL_RETURN(strlen(path) <= 0 || strlen(path) >= PATH_MAX_LEN - 1, FMS_TRUE);

	memset(g_file_path, 0, PATH_MAX_LEN);
	strcpy(g_file_path, path);
}


fms_void dsprint(fms_s8 *const str, const fms_s8 * fmt, ...) {
    va_list ap;
	
    va_start(ap, fmt);
    vsprintf(str, fmt, ap);
    va_end(ap);
}


fms_void fms_log_output(const fms_u8 priority, const fms_u8 print_threshold, 
							  const fms_u8 write_threshold, const fms_s8 *const msg) {
	FmsSpinLock spinLock(&g_log_lock);

	LOG_EQUAL_RETURN(msg, NULL);
							  
	if (priority >= print_threshold) {
#ifdef OS_ANDROID
	__android_log_print(ANDROID_LOG_INFO, "fremaks_log", msg); //暂时这样，之后可以设计与android的兼容，包括tag等
#else
	fprintf(stderr, "%s", msg);
	fflush(stderr);
#endif
	}
#if 0
	if (priority >= write_threshold) {
		printf("++++++++++fms_log_output 1\n");
		if (NULL == g_log) {
			printf("++++++++++fms_log_output 2\n");
			g_log = fms_log_new(g_file_size, g_file_path);
			scopePtr = std::auto_ptr<fms_log>(g_log);
		}
		printf("++++++++++fms_log_output 3\n");
		LOG_EQUAL_RETURN(g_log, NULL);
		printf("++++++++++fms_log_output 4\n");
		fms_log_write(g_log, msg);
		printf("++++++++++fms_log_output 5\n");
	}
#endif
}

