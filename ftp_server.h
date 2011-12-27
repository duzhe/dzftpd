#ifndef DZFTP_FTP_SERVER_H_INCLUDE
#define DZFTP_FTP_SERVER_H_INCLUDE
#include "classes.h"

class ftp_server_internal;
class ftp_server
{
public:
	ftp_server();
	~ftp_server();
	int run(ftp_config *conf);
//	int serve_it(ftp_client *);
//	int serve_it_standalone(ftp_client *client);
private:
	ftp_server_internal *internal;
	ftp_server(const ftp_server&);
	ftp_server& operator=(const ftp_server&);
};

#endif
