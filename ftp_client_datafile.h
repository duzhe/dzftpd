#ifndef DZFTP_FTP_CLIENT_DATAFILE_H_INCLUDE
#define DZFTP_FTP_CLIENT_DATAFILE_H_INCLUDE


enum data_mode
{
	PASV,
	PORT,
};

class ftp_client_datafile
{
public:
	data_mode mode;
public:
	ftp_client_datafile();
	unsigned short random_bind();
	void reset();
	void accept_connection();
	int write_file(const char *filename);
private:
	int listenfd;
	int datafd;
};

#define NO_DATA_CONNECTION		-1
#define OPEN_FILE_ERROR			-2
#define READ_FILE_ERROR			-3
#define CLIENT_CLOSE_DATA_CONNECTION -4
#endif
