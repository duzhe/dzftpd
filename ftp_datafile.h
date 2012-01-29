#ifndef DZFTP_FTP_DATAFILE_H_INCLUDE
#define DZFTP_FTP_DATAFILE_H_INCLUDE
#include <netinet/in.h>

enum data_mode
{
	NOSP,
	PASV,
	PORT,
};

enum datacnn_state
{
	ready,
	pasv_wait,
	port_wait,
	connected,
};

class ftp_datafile
{
public:
	data_mode mode;
public:
	ftp_datafile();
	in_port_t random_bind(in_addr_t host);
	void reset();
	void accept_connection();
	int write_file(const char *filename);
	ssize_t read(void *buf, size_t count)const;
	ssize_t write(const void *buf, size_t count);
	datacnn_state state()const;
private:
	int listenfd;
	int datafd;
};

#define NO_DATA_CONNECTION		-1
#define OPEN_FILE_ERROR			-2
#define READ_FILE_ERROR			-3
#define CLIENT_CLOSE_DATA_CONNECTION -4
#endif
