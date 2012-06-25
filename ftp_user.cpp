// This version of ftp_user assume that ftpd use fork to serve client, use unix
// user authentication mechanism, and chroot user at his home path if user is 
// not root
#include "global.h"
#include "ftp_user.h"
#include <sys/types.h>
#include <pwd.h>
#include <shadow.h>
#include <string.h>
#include <unistd.h>

int ftp_user::set_username(const char *username)
{
	this->username = username;
	return 1;
}

int ftp_user::login(const char *passwd)
{
	// NOTE: getspnam may only working at linux, i cannot find a method defined by POSIX.1 or XSI
	struct spwd* sp = getspnam(username.c_str() );
	if (sp == NULL){
		if (errno == 0){
			return LOGIN_ERROR_NO_SUCH_USER;
		}
		else {
			return LOGIN_ERROR_CANNOT_AUTH;
		}
	}
	const char *cpwd = sp->sp_pwdp;
	if ( strcmp(crypt(passwd, cpwd), cpwd) != 0){
		return LOGIN_ERROR_WRONG_PASSWORD;
	}
	
	do{
		struct passwd *result = getpwnam(username.c_str() );
		if (result == NULL){
			switch (errno){
			case EINTR:
				continue;
			case EIO:
			case EMFILE:
			case ENFILE:
			case ENOMEM:
			case ERANGE:
				return LOGIN_ERROR_CANNOT_AUTH;
			case 0:
			default:
				return LOGIN_ERROR_NO_SUCH_USER;
			}
		}
		setuid(result->pw_uid);
		if (result->pw_uid == 0){
			homedir = "/";
		}
		else {
			homedir = result->pw_dir;
		}
		break;
	}
	while (true);
	authenticated = true;
	return LOGIN_ERROR_SUCCESS;
}

const char *ftp_user::get_username()const
{
	return username.c_str();
}

int ftp_user::loggedin()const
{
	return authenticated;
}

const char *ftp_user::rootpath()const
{
	if (!homedir.empty() ){
		return homedir.c_str();
	}
	do{
		struct passwd *result = getpwnam(username.c_str() );
		if (result == NULL){
			switch (errno){
			case EINTR:
				continue;
			case EIO:
			case EMFILE:
			case ENFILE:
			case ENOMEM:
			case ERANGE:
				return "/_nosuchpath_";
			case 0:
			default:
				return "/_nosuchpath_";
			}
		}
		if (result->pw_uid == 0){
			// root user
			return "/";
		}
		else {
			homedir = result->pw_dir;
			return homedir.c_str();
		}
	}
	while (true);
	return homedir.c_str();
}

const char *ftp_user::homepath()const
{
	return "/";
}

