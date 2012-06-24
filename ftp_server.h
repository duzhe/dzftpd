#ifndef DZFTP_FTP_SERVER_H_INCLUDE
#define DZFTP_FTP_SERVER_H_INCLUDE
#include "classes.h"

class ftp_server_internal;
class ftp_server
{
public:
	ftp_server();
	~ftp_server();
	int serve_it(int client_ctrlfd);

private:
	ftp_server_internal *internal;
	ftp_server(const ftp_server&);
	ftp_server& operator=(const ftp_server&);
};

#endif
