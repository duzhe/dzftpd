#ifndef DZFTP_FTP_CLIENT_CTRLFILE_H_INCLUDE
#define DZFTP_FTP_CLIENT_CTRLFILE_H_INCLUDE

class ftp_client_ctrlfile
{
public:
	ftp_client_ctrlfile(int ctrlfd);
	~ftp_client_ctrlfile();
	int close();
	int puts(const char *message);
	int printf(const char *format, ...);
	int readline(char *linebuf);
	int flush();
private:
	int fd;

	size_t write_buf_left;
	char write_buf[MAX_RESPONSE_LENGTH];
	char *write_buf_pos;

	size_t read_buf_left;
	char read_buf[MAX_REQUEST_LENGTH];
	char *read_buf_pos;
	int read();
};

#endif
