#ifndef DZFTP_FTP_CLIENTINFO_H_INCLUDE
#define DZFTP_FTP_CLIENTINFO_H_INCLUDE

class ftp_clientinfo_internal;
class ftp_clientinfo
{
public:
	ftp_clientinfo();
	~ftp_clientinfo();
	const char *get_username()const;
	void set_username(const char *newname);
private:
	ftp_clientinfo_internal *internal;
};


#endif
