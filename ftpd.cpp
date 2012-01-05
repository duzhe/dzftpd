#include "global.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h> /* for fork */
#include <stdlib.h> /* for exit */
#include <string.h>
#include <strings.h>
#include <string>
#include "ftpd.h"
#include "ftp_server.h"
#include "ftp_config.h"

class ftpd_internal
{
public:
	int init(ftp_config *conf);
	int run();
	int serve_it(int client_ctrlfd);
	int serve_it_standalone(int client_ctrlfd);
private:
	ftp_config *conf;
	int listenfd;
};

ftpd::ftpd()
{
	internal = new ftpd_internal();
}

ftpd::~ftpd()
{
	delete internal;
}

int ftpd::init(ftp_config *conf)
{
	return internal->init(conf);
}

int ftpd::serve()
{
	return internal->run();
}

//---------vvvvvvvvvvvv------- internal methods -------vvvvvvvvvvvvvv---------

int ftpd_internal::init(ftp_config *conf)
{
	this->conf = conf;
	struct sockaddr_in servaddr;
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
//	if(listenfd == -1){
//		die("create socket error:%d\n", errno);
//	}

	bzero((char*)&servaddr, sizeof(servaddr) );
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(conf->listen_port);

	int ret_val = bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr) );
//	if(ret_val == -1){
//		die("bind error :%d\n", errno);
//	}

	listen(listenfd, LISTENQ);

	return 0;
}

int ftpd_internal::run()
{
	int client_ctrlfd;
	for( ; ; ){
		client_ctrlfd = accept(listenfd, (struct sockaddr *)NULL, NULL);
		serve_it_standalone(client_ctrlfd);
	}
	return 0;
}

int ftpd_internal::serve_it_standalone(int client_ctrlfd)
{
	ftp_server server;
	return server.serve_it(client_ctrlfd);
}

int ftpd_internal::serve_it(int client_ctrlfd)
{
	pid_t pid;
	// child process serve the client and exit
	if( (pid= fork() ) == 0){
		ftp_server server;
		close(listenfd);
		server.serve_it(client_ctrlfd);
		exit(0);
	}
	// parent process close the client and return to wait another client;
	else{
		close(client_ctrlfd);
	}
	return 0;
}

