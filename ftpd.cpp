#include "global.h"
#include "ftpd.h"
#include "ftp_server.h"
#include "ftp_client.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <string>

class ftpd_internal
{
public:
	int init();
	int serve();
private:

};

ftpd::ftpd()
{
	internal = new ftpd_internal();
}

ftpd::~ftpd()
{
	delete internal;
}

int ftpd::init()
{
	return internal->init();
}

int ftpd::serve()
{
	return internal->serve();
}

int ftpd_internal::init()
{
	return 0;
}

int ftpd_internal::serve()
{
	ftp_config config;
	ftp_server server;
	return server.run(&config);
}

