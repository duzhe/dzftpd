#ifndef DZFTP_FTP_USER_H_INCLUDE
#define DZFTP_FTP_USER_H_INCLUDE

#include <string>
class ftp_user
{
public:
	ftp_user(){successed = false;};
	int set_username(const char *username);
	int login(const char *passwd);
	int loggedin()const;
	const char *get_username()const;
	const char *homepath()const;
private:
	std::string username;
	bool successed;
};

#endif
