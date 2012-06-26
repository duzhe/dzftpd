#include "global.h"
#include "fs_unix.h"
#include <fcntl.h>

void fs_unix::set_chroot(const char *chroot)
{
	this->chroot = chroot;
}

FILE * fs_unix::fopen(const char *filename, const char *mode)
{
	return ::fopen((chroot+filename).c_str() , mode);
}

DIR * fs_unix::opendir(const char *name)
{
	return ::opendir((chroot+name).c_str() );
}

int fs_unix::open(const char *pathname, int flags)
{
	return ::open( (chroot+pathname).c_str(), flags);
}

int fs_unix::rmdir(const char *pathname)
{
	return ::rmdir((chroot+pathname).c_str() );
}

int fs_unix::mkdir(const char *pathname, mode_t mode)
{
	return ::mkdir((chroot+pathname).c_str(), mode);
}

int fs_unix::unlink(const char *pathname)
{
	return ::unlink((chroot+pathname).c_str() );
}

int fs_unix::lstat(const char *path, struct stat *buf)
{
	return ::lstat((chroot+path).c_str(), buf);
}

static const char *get_parent_path(const char *filename)
{
	static char parent_path[MAX_PATH];
	char *p1 = parent_path;
	const char *p2 = filename;
	for(; *p2 != '\0';){
		*p1++ = *p2++;
	}
	while(*p1 != '/' ){
		--p1;
	}
	*p1 = '\0';
	return parent_path;
}


bool fs_unix::test_access(const char *filename, char item)
{
	std::string realfilename = chroot +filename;
	DEBUG("test_access: %s %c", realfilename.c_str(), item);
	int mode;
	switch(item){
		case 'r':
			mode = R_OK;
			break;
		case 'w':
			mode = W_OK;
			break;
		case 'x':
			mode = X_OK;
			break;
		case 'f':
			mode = F_OK;
			break;
		default:
			return false;
	}
	int ret_val = access(realfilename.c_str(), mode);
	if(ret_val == 0){
		DEBUG("test_access OK: %s %c", realfilename.c_str(), item);
		return true;
	}
	else if(item != 'w'){
		DEBUG("test_access Fail: %s %c", realfilename.c_str(), item);
		return false;
	}
	const char *parent_path = get_parent_path(realfilename.c_str());
	return test_access(parent_path, 'w');
}
