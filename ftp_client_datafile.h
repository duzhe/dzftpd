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
	unsigned short random_bind();
private:
	int listenfd;
	int datafd;
};

#endif
