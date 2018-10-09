#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/syscall.h>

#define __WARNNING_LOG__
#define __DEBUG_LOG__
#define __ERROR_LOG__

#ifdef __DEBUG_LOG__
#define LOGD(format,...) printf("%s %d %d D "__FILE__"[%05d]: "format"",          \
			timeString(), (int)getpid(), (int)syscall(SYS_gettid), __LINE__, ##__VA_ARGS__)
#else
#define LOGD(format,...)
#endif

#ifdef __ERROR_LOG__
#define LOGE(format,...) printf("%s %d %d E "__FILE__"[%05d]: "format"",          \
			timeString(), (int)getpid(), (int)syscall(SYS_gettid), __LINE__, ##__VA_ARGS__)
#else
#define LOGE(format,...)
#endif

#ifdef __WARNNING_LOG__
#define LOGW(format,...) printf("%s %d %d W "__FILE__"[%05d]: "format"",          \
			timeString(), (int)getpid(), (int)syscall(SYS_gettid), __LINE__, ##__VA_ARGS__)
#else
#define LOGW(format,...)
#endif

#define LOGN

char *timeString();
