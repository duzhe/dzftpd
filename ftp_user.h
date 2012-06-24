#ifndef DZFTP_FTP_USER_H_INCLUDE
#define DZFTP_FTP_USER_H_INCLUDE

#include <string>

#define LOGIN_ERROR_SUCCESS 		0
#define LOGIN_ERROR_NO_SUCH_USER	-1
#define LOGIN_ERROR_CANNOT_AUTH		-2
#define LOGIN_ERROR_WRONG_PASSWORD	-3
class ftp_user
{
public:
	ftp_user(){authenticated = false;};
	int set_username(const char *username);
	int login(const char *passwd);
	int loggedin()const;
	const char *get_username()const;
	const char *homepath()const;
	const char *rootpath()const;
private:
	std::string username;
	mutable std::string homedir;
	bool authenticated;
};

#endif
