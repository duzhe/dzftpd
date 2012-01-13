#include "global.h"
#include "ftp_dir.h"

static int ensure_dir(const char *pathname)
{
	DEBUG("Temporary Implementation: ensure_dir\n");
	return 0;
}
int ftp_dir::cdup()
{
	std::string current = dir;
	std::string::size_type pos = current.rfind('/');
	if(std::string::npos == pos){
		return -1;
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
	if('/' == *pathname){
		if(ensure_dir(pathname) != 0){
			return -1;
		}
		dir = pathname;
	}
	else{
		std::string fullpathname;
		if(dir.length() == 1){
			fullpathname = dir + pathname;
		}
		else{
			fullpathname = dir + '/' + pathname;
		}
		if(ensure_dir(fullpathname.c_str() )!= 0){
			return -1;
		}
		std::swap(dir, fullpathname);
	}
	return 0;
}

const char *ftp_dir::pwd()const
{
	return dir.c_str();
}

