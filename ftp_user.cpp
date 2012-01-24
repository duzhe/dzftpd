#include "global.h"
#include "ftp_user.h"

int ftp_user::set_username(const char *username)
{
	this->username = username;
	return 1;
}

int ftp_user::login(const char *passwd)
{
	successed = true;
	return 1;
}

const char *ftp_user::get_username()const
{
	return username.c_str();
}

int ftp_user::loggedin()const
{
	return successed;
}

const char *ftp_user::homepath()const
{
	return "/";
}

