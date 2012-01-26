#include "global.h"
#include "ftp_dir.h"

static int ensure_dir(const char *pathname)
{
	DEBUG("Temporary Implementation: ensure_dir\n");
	return 0;
}

ftp_dir::ftp_dir(const char *homepath)
{
	dir = homepath;
}

int ftp_dir::cdup()
{
	std::string current = dir;
	std::string::size_type pos = current.rfind('/');
	if(std::string::npos == pos){
		return -1;
	}

	if(pos == 0 && current.length() == 1){
		return 0;
	}

	current.resize(pos);
	if(ensure_dir(current.c_str() ) != 0){
		return -1;
	}
	std::swap(current, dir);
	return 0;
}

int ftp_dir::cd(const char *pathname)
{
	const std::string &fullpathname = getfullpathname(pathname);
	if(ensure_dir(fullpathname.c_str() ) != 0 ){
		return -1;
	}
	dir = fullpathname;
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
	
