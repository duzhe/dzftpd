#ifndef DZFTP_FTP_DIR_H_INCLUDE
#define DZFTP_FTP_DIR_H_INCLUDE

#include <string>

// chrooted
class ftp_user;
class ftp_dir
{
public:
	std::string cwd;
	std::string root;
public:
	ftp_dir(){};
	ftp_dir(const char *homepath);
	int init(const ftp_user* user);
	int cdup();
	int cd(const char *pathname);
	const char *pwd()const;
	std::string getfullpathname(const char *param)const;
};
#endif
