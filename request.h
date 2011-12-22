#ifndef DZFTP_FTP_REQUEST_H_INCLUDE
#define DZFTP_FTP_REQUEST_H_INCLUDE

typedef unsigned int response_code;
class request
{
public:
//	request();
//	~request();

	size_t parse_from_commandline(const char *commandbuf, size_t data_length);
	char command[8];
	char *params;
};

#endif
