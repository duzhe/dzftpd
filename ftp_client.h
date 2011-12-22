#ifndef DZFTP_FTP_CLIENT_H_INCLUDE
#define DZFTP_FTP_CLIENT_H_INCLUDE

class ftp_client_ctrlfile;
class ftp_client_datafile;
class request;

class ftp_client
{
public:
	ftp_client(int client_ctrlfd);
	~ftp_client();

	int wait_request(request *r);
	int close();
	int response(response_code, const char *);
	int response_format(response_code, const char *format, ...);
	int do_response();
private:
	ftp_client_ctrlfile *ctrlfile;
	ftp_client_datafile *datafile;
};

#endif

