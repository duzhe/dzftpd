#ifndef DZFTP_FTP_CLIENT_H_INCLUDE
#define DZFTP_FTP_CLIENT_H_INCLUDE

class client_status
{
public:
	enum status
	{
		ready,
		loggedin,
		dataconnected,
	}
	status state;
	bool pasv;
};

class request;
class ftp_client_internal;
class ftp_client
{
public:
	ftp_client(int client_ctrlfd);
	~ftp_client();

	int wait_request(request *r);
	int close();
	int response(response_code_t, const char *);
	int response_format(response_code_t, const char *format, ...);
	int do_response();
	client_status *get_status();
private:
	ftp_client_internal *internal;
};

#endif

