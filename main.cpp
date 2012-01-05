#include "global.h"
#include "ftpd.h"
#include "ftp_config.h"

int main()
{
	ftpd d;
	ftp_config conf;
	d.init(&conf);
	return d.serve();
}

