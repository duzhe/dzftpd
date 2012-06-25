#ifndef DZFTPD_FS_UNIX_H_INCLUDE
#define DZFTPD_FS_UNIX_H_INCLUDE
#include <dirent.h>
#include <sys/types.h>
#include <stdio.h>
#include <string>

class fs_unix
{
public:
	FILE * fopen(const char *filename, const char *mode);
	int open(const char *pathname, int flags);
	DIR * opendir(const char *name);
	int lstat(const char *path, struct stat *buf);
	int mkdir(const char *pathname, mode_t mode);
	int rmdir(const char *filename);
	int unlink(const char *filename);
	bool test_access(const char *filename, char item);

public:
	void set_chroot(const char *chroot);
private:
	std::string chroot;
};
#endif

