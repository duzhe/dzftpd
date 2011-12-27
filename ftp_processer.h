#ifndef DZFTP_FTP_PROCESSER_H_INCLUDE
#define DZFTP_FTP_PROCESSER_H_INCLUDE

#define ERROR_CLIENT_QUIT -1
class ftp_processer
{
public:
	static int process_request(ftp_client *client, request *r);
};

#endif
