#include "global.h"
#include "ftp_server.h"
#include "ftp_client.h"
#include "ftp_config.h"
#include "ftp_processer.h"
#include "request.h"
#include <unistd.h>
#include <stdlib.h> /* for free */
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>  /* for htonl */
#include <strings.h>


class ftp_server_internal
{
public:
	int init(ftp_config *conf);
	int serve();
private:
	int serve_it(ftp_client *client);
	int serve_it_standalone(ftp_client *client);
	int process_request(ftp_client *client, request *r);
	int do_welcome(ftp_client *client);
private:
	ftp_config *conf;
	int listenfd;
};

ftp_server::ftp_server()
{
	internal = new ftp_server_internal;
}

ftp_server::~ftp_server()
{
	delete internal;
}

int ftp_server::run(ftp_config *conf)
{
	internal->init(conf);
	internal->serve();
	return 0;
}

int ftp_server_internal::init(ftp_config *conf)
{
	this->conf = conf;
	return 0;
}

int ftp_server_internal::serve()
{
	int listenfd, client_ctrlfd;
	struct sockaddr_in servaddr;
	listenfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero((char*)&servaddr, sizeof(servaddr) );
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
//	servaddr.sin_port = htons(FTP_PORT);
	servaddr.sin_port = htons(conf->listen_port);

	bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr) );

	listen(listenfd, LISTENQ);

	for( ; ; ){
		client_ctrlfd = accept(listenfd, (struct sockaddr *)NULL, NULL);
		ftp_client *client = new ftp_client(client_ctrlfd);
		serve_it(client);
		//serve_it_standalone(client);
	}
		
	return 0;
}

int ftp_server_internal::serve_it(ftp_client *client)
{
	pid_t pid;
	// child process serve the client and exit
	if( (pid= fork() ) == 0){
		close(listenfd);
		serve_it_standalone(client);
		delete client;
		exit(0);
	}
	// parent process close the client and return to wait another client;
	delete client;
	return 0;
}

int ftp_server_internal::serve_it_standalone(ftp_client *client)
{
	do_welcome(client);
	for(;;){
		request r;
		if(client->wait_request(&r) != 0){
			DEBUG("request error!");
			continue;
		}
		int ret_val =  process_request(client, &r);
		if(ret_val != 0){
			if( ret_val == ERROR_CLIENT_QUIT){
				break;
			}
		}
	}
	client->close();
	return 0;
}

int ftp_server_internal::process_request(ftp_client *client, request *r)
{
	DEBUG("process_request\n");
	return ftp_processer::process_request(client, r);
}

int ftp_server_internal::do_welcome(ftp_client *client)
{
	client->response(220, "Welcome");
	client->do_response();
	return 0;
}

