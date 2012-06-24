#include "global.h"
#include "ftp_dir.h"
#include "ftp_user.h"

ftp_dir::ftp_dir(const char *homepath)
{
	cwd = homepath;
}

int ftp_dir::init(const ftp_user* user)
{
	root = user->rootpath();
	cwd = user->homepath();
	if (root == "/" && cwd == "/" ){
		root = "";
	}
	return 0;
}
	
int ftp_dir::cdup()
{
	std::string::size_type pos = cwd.rfind('/');
	if(std::string::npos == pos){
		return -1;
	}

	if(pos == 0 && cwd.length() == 1){
		return 0;
	}

	std::string new_dir(cwd, 0, pos);
	if(test_access(new_dir.c_str(), 'x') == false){
		return -1;
	}
	cwd.swap(new_dir);
	return 0;
}

int ftp_dir::cd(const char *pathname)
{
	const std::string &new_dir = getfullpathname(pathname);
	if(test_access(new_dir.c_str(), 'x') == false ){
		return -1;
	}
	cwd = new_dir;
	return 0;
}

const char *ftp_dir::pwd()const
{
	return cwd.c_str();
}

std::string ftp_dir::getfullpathname(const char *param)const
{
	std::string fullpathname;
	if (param == NULL){
		fullpathname = root + cwd;
	}
	else if (param[0] == '/'){
		fullpathname = root + param;
	}
	else if(cwd.length() == 1){
		fullpathname = root + cwd + param;
	}
	else{
		fullpathname = root + cwd + '/' + param;
	}
	return fullpathname;
}
	
