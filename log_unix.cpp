#include "global.h"
#include "log_unix.h"
#include <syslog.h>

void log_unix::init()
{
	openlog(PROGRAME_NAME, LOG_CONS | LOG_PID, LOG_FTP);
}

void log_unix::log(int level, const char *format, ...)
{
	va_list va;
	va_start(va, format);
	vsyslog(level, format , va);
	va_end(va);
}
