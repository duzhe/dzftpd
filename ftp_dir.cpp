#include "global.h"
#include "ftp_dir.h"

ftp_dir::ftp_dir(const char *homepath)
{
	dir = homepath;
}

int ftp_dir::cdup()
{
	std::string::size_type pos = dir.rfind('/');
	if(std::string::npos == pos){
		return -1;
	}

	if(pos == 0 && dir.length() == 1){
		return 0;
	}

	std::string new_dir(dir, pos);
	if(test_access(new_dir.c_str(), 'x') == false){
		return -1;
	}
	dir.swap(new_dir);
	return 0;
}

int ftp_dir::cd(const char *pathname)
{
	const std::string &new_dir = getfullpathname(pathname);
	if(test_access(new_dir.c_str(), 'x') == false ){
		return -1;
	}
	dir = new_dir;
	return 0;
}

const char *ftp_dir::pwd()const
{
	return dir.c_str();
}

std::string ftp_dir::getfullpathname(const char *param)const
{
	if(param == NULL){
		return pwd();
	}
	if('/' == *param){
		return param;
	}

	std::string fullpathname;
	if(dir.length() == 1){
		fullpathname = dir + param;
	}
	else{
		fullpathname = dir + '/' + param;
	}
	return fullpathname;
}
	
