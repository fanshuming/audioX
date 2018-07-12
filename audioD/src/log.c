#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
//#include <curl/curl.h>

#include "log.h"

char *timeString()
{
	struct timespec ts;
	struct tm * timeinfo = localtime(&ts.tv_sec);
	static char timeStr[20];

	clock_gettime(CLOCK_REALTIME, &ts);
	sprintf(timeStr, "%.2d-%.2d %.2d:%.2d:%.2d.%.3ld",
			timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour,
			timeinfo->tm_min, timeinfo->tm_sec, ts.tv_nsec / 1000000);

	return timeStr;
}
