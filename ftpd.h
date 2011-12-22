#ifndef DZFTP_FTPD_H_INCLUDE
#define DZFTP_FTPD_H_INCLUDE

class ftpd_internal;
class ftpd
{
public:
	ftpd();
	~ftpd();
	int init();
	int serve();
private:
	ftpd_internal* internal;
};

#endif
