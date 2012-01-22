#ifndef DZFTP_FTP_DIR_H_INCLUDE
#define DZFTP_FTP_DIR_H_INCLUDE

#include <string>

// chrooted
class ftp_dir
{
public:
	std::string dir;
public:
	ftp_dir(){};
	ftp_dir(const char *homepath);
	int cdup();
	int cd(const char *pathname);
	const char *pwd()const;
	std::string getfullpathname(const char *param)const;
};
#endif
