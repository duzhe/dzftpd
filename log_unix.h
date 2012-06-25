#ifndef DZFTPD_LOG_UNIX_H_INCLUDE
#define DZFTPD_LOG_UNIX_H_INCLUDE

#include <syslog.h>
class log_unix
{
public:
	void init();
	void log(int level, const char* format, ...);
private:
};
#endif
