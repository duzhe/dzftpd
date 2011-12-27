#include "global.h"
#include "ftp_clientinfo.h"
#include <string>


class ftp_clientinfo_internal
{
public:
	std::string username;

};

ftp_clientinfo::ftp_clientinfo()
{
	internal = new ftp_clientinfo_internal;
}

ftp_clientinfo::~ftp_clientinfo()
{
	delete internal;
}

const char *ftp_clientinfo::get_username()const
{
	return internal->username.c_str();
}

void ftp_clientinfo::set_username(const char *newname)
{
	internal->username = newname;
}

