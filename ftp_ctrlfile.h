#ifndef DZFTP_FTP_CTRLFILE_H_INCLUDE
#define DZFTP_FTP_CTRLFILE_H_INCLUDE
#include <netinet/in.h>
class ftp_ctrlfile
{
public:
	ftp_ctrlfile(int ctrlfd);
	~ftp_ctrlfile();
	in_addr_t get_host_addr()const;
	int close();
	int puts(const char *message);
	int printf(const char *format, ...);
	int readline(char *linebuf);
	int flush();
private:
	void set_nodelay();
private:
	int fd;
	mutable in_addr_t addr;

	size_t write_buf_left;
	char write_buf[MAX_RESPONSE_LENGTH];
	char *write_buf_pos;

	size_t read_buf_left;
	char read_buf[MAX_REQUEST_LENGTH];
	char *read_buf_pos;
	int read();
};

#endif
