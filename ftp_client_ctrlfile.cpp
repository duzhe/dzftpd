#include "global.h"
#include "ftp_client_ctrlfile.h"
#include <unistd.h>  //for write

ftp_client_ctrlfile::ftp_client_ctrlfile(int client_ctrlfd)
{
	fd = client_ctrlfd;
	buf_left = sizeof(write_buf);
	write_buf_pos = write_buf;
}

ftp_client_ctrlfile::~ftp_client_ctrlfile()
{
	close();
}

int ftp_client_ctrlfile::close()
{
	if(fd != -1){
		::close(fd);
		fd = -1;
	}
}

int ftp_client_ctrlfile::puts(const char *message)
{
	return 0;
}

int ftp_client_ctrlfile::printf(const char *format, ...)
{
	char buf[RESPONSE_MAX_LENGTH];

	va_list va;
	va_start(va, format);
	int length = vsnprintf(buf, sizeof(buf), format, va);
	va_end(va);

	if(length < 0){
		return -1;
	}
	if(length > buf_left){
		flush();
	}
	if(length > buf_left){
		return -1;
	}
	memcpy(write_buf_pos, buf, length);
	write_buf_pos += length;
	buf_left -= length;
	return 0;
}

int ftp_client_ctrlfile::flush()
{
	int return_code = ::write(fd, write_buf, write_buf_pos - write_buf);
	if(return_code == -1){
		return  -1;
	}
	write_buf_pos = write_buf;
	buf_left = sizeof(write_buf);
	return 0;
}

