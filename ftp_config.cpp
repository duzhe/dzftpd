#include "global.h"
#include "ftp_config.h"

ftp_config::ftp_config()
{
	listen_port = (FTP_PORT);
}

int ftp_config::load(const char *config_file)
{
	return 0;
}


