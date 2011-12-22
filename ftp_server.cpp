#include "global.h"
#include "ftp_server.h"
#include "ftp_client.h"
#include "ftp_config.h"
#include "request.h"
#include <unistd.h>
#include <string.h>
#include <map>

struct client_status
{
	bool 			login;
	const char 		*username;
	bool			pasv;
	bool			data_connected;
	client_status()
	{
		login = false;
		username = NULL;
		pasv = false;
		data_connected = false;
	}
	~client_status()
	{
		if(username != NULL){
			free(username);
		}
	}
	int set_username(const char *username)
	{
		if(this->username != NULL){
			free(this->username);
		}
		this->username = strdup(username);
		return 0;
	}
}

class ftp_server_internal
{
//	friend class ftp_server;
public:
	int init(ftp_config *conf);
	int serve();
private:
	int serve_it(ftp_client *client);
	int serve_it_standalone(ftp_client *client);
	int process_request(ftp_client *client, request *r);
	int do_welcome(ftp_client *client);
	int command_user(ftp_client *client, const char *param);
	int command_not_support(ftp_client *client);
private:
	std::map<ftp_client *, client_status> clients_status;
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
	servaddr.sin_port = htons(config->listen_port);

	bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr) );

	listen(listenfd, LISTENQ);

	for( ; ; ){
		client_ctrlfd = accept(listenfd, (struct sockaddr *)NULL, NULL);
		ftp_client *client = new client(client_ctrlfd);
		serve_it(&client);
		//serve_it_standalone(client);
	}
		
	return 0;
}

int ftp_server_internal::serve_it(ftp_client *client)
{
#ifdef FORK  /* test for automake */
	pid_t pid;
	// child process serve the client and exit
	if( (pid= fork() ) == 0){
		close(listenfd);
		serve_it_standalone(client);
		client->close();
		exit(0);
	}
	// parent process close the client and return to wait another client;
	client->close();
#endif
	return 0;
}

int ftp_server::serve_it_standalone(ftp_client *client)
{
	return internal->serve_it_standalone(client);
}

int ftp_server_internal::serve_it_standalone(ftp_client *client)
{
	do_welcome(client);
	request r;
	for(;;){
		client->wait_request(&r);
		process_request(client, &r);
	}
	client->close();
	return 0;
}

#define PROCESS_MAP_BEGIN() if(0){}

#define PROCESS_MAP(COMMAND, PROCESS_FUNCTION) \
	else if(strcmp(command, COMMAND) == 0 ) { \
		result = PROCESS_FUNCTION(client, r->params); \
	}
#define PROCESS_MAP_END() \
	else{ \
		result = command_not_support(client);\
	}
int ftp_server_internal::process_request(ftp_client *client, request *r)
{
	int return_code = 0;
	while(true){
		const char *command = r->command;
		int result = 0;

		PROCESS_MAP_BEGIN()
		PROCESS_MAP("USER", command_user)
//		PROCESS_MAP("PASS", command_pass)
//		PROCESS_MAP("PASV", command_pasv)
		PROCESS_MAP_END()

	}
	return return_code;
}

int ftp_server_internal::do_welcome(ftp_client *client)
{
	return 0;
}

int ftp_server_internal::command_user(ftp_client *client, const char *param)
{
	if(clients_status[client].logged_in() ){
		if(client_status[client].sameuser(param) ){
			client->response(331, REPLY_ANY_PSWD);
		}
		else{
			client->response(530, REPLY_CANNOT_CHANGE_USER);
		}
	}
	else if(param == NULL || *param == '\0' ){
		client->response(332, REPLY_NEED_USER);
	}
	else if(!valid_username(param) ){
		client->response(530, REPLY_NOT_LOGGED_IN);
	else{
		clients_status[client].set_username(param);
		client->response(331, REPLY_NEED_PSWD);
	}
	return 0;
}

int ftp_server_internal::command_not_support(ftp_client *client)
{
	return 0;
}

