#ifndef DZFTP_FTPD_H_INCLUDE
#define DZFTP_FTPD_H_INCLUDE

class ftp_config;
class ftpd_internal;
class ftpd
{
public:
	ftpd();
	~ftpd();
	int init(ftp_config *);
	int serve();
private:
	ftpd_internal* internal;
};

#endif
