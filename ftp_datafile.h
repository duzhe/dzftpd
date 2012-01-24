#ifndef DZFTP_FTP_DATAFILE_H_INCLUDE
#define DZFTP_FTP_DATAFILE_H_INCLUDE


enum data_mode
{
	NOSP,
	PASV,
	PORT,
};

class ftp_datafile
{
public:
	data_mode mode;
public:
	ftp_datafile();
	unsigned short random_bind(int host);
	void reset();
	void accept_connection();
	int write_file(const char *filename);
	ssize_t write(const void *buf, size_t count);
private:
	int listenfd;
	int datafd;
};

#define NO_DATA_CONNECTION		-1
#define OPEN_FILE_ERROR			-2
#define READ_FILE_ERROR			-3
#define CLIENT_CLOSE_DATA_CONNECTION -4
#endif
