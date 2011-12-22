#ifndef DZFTP_FTP_CLIENT_CTRLFILE_H_INCLUDE
#define DZFTP_FTP_CLIENT_CTRLFILE_H_INCLUDE

class ftp_client_ctrlfile
{
public:
	ftp_client_ctrlfile();
	~ftp_client_ctrlfile();
	int close();
	int puts(const char *message);
	int printf(const char *format, ...);
	int flush();
private:
	int fd;
	size_t buf_left;
	char write_buf[RESPONSE_MAX_LENGTH];
	char *write_buf_pos;
};

#endif
