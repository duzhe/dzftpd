#include "global.h"
#include "ftp_server.h"
#include "ftp_client.h"
#include "ftp_config.h"
#include "ftp_client_datafile.h"
#include "request.h"
#include "messages.h"
#include <strings.h>
#include <string.h>
#include <string>

enum serve_state 
{
	ready,
	loggedin,
	datacnn_wait,
	datacnn_ready,
};

enum trans_type
{
	type_A,
	type_I,
};
#define ERROR_CLIENT_QUIT -1

class ftp_server_internal
{
public:
	ftp_server_internal(){state = ready;type = type_I;}
	int serve_it(ftp_client *client);
private:
	int process_request(ftp_client *client, request *r);
	int do_welcome(ftp_client *client);
	int command_user(ftp_client *client, const char *param);
	int command_pass(ftp_client *client, const char *param);
	int command_pasv(ftp_client *client, const char *param);
	int command_quit(ftp_client *client, const char *param);
	int command_pwd( ftp_client *client, const char *param);
	int command_type(ftp_client *client, const char *param);
	int command_mode(ftp_client *client, const char *param);
	int command_stru(ftp_client *client, const char *param);
	int command_retr(ftp_client *client, const char *param);
	int command_stor(ftp_client *client, const char *param);
	int command_noop(ftp_client *client, const char *param);
	int command_list(ftp_client *client, const char *param);
	int command_not_support(ftp_client *client);

	serve_state get_state();
private:
	serve_state state;
	trans_type type;
	std::string username;
	std::string workpath;

};

ftp_server::ftp_server()
{
	internal = new ftp_server_internal;
}

ftp_server::~ftp_server()
{
	delete internal;
}

int ftp_server::serve_it(int client_ctrlfd)
{
	ftp_client client(client_ctrlfd);
	return internal->serve_it(&client);
}

//---------vvvvvvvvvvvv------- internal methods -------vvvvvvvvvvvvvv---------

#define PROCESS_MAP_BEGIN() if(0){}

#define PROCESS_MAP(COMMAND, PROCESS_FUNCTION) \
	else if(strcmp(command, COMMAND) == 0 ) { \
		ret_val = PROCESS_FUNCTION(client, r->params); \
	}
#define PROCESS_MAP_END(NOT_FOUND_FUNCTION) \
	else{ \
		DEBUG("Command Not Support:%s %s\n", command, r->params == NULL?"":r->params);	\
		ret_val = NOT_FOUND_FUNCTION(client);\
	}

int ftp_server_internal::process_request(ftp_client *client, request *r)
{
	int ret_val = 0;
	const char *command = r->command;

	PROCESS_MAP_BEGIN()
	PROCESS_MAP("USER", command_user)
	PROCESS_MAP("PASS", command_pass)
	PROCESS_MAP("QUIT", command_quit)
	PROCESS_MAP("PASV", command_pasv)
	PROCESS_MAP("PWD", 	command_pwd)
	PROCESS_MAP("TYPE", command_type)
	PROCESS_MAP("MODE", command_mode)
	PROCESS_MAP("STRU", command_stru)
	PROCESS_MAP("RETR", command_retr)
	PROCESS_MAP("STOR", command_stor)
	PROCESS_MAP("NOOP", command_noop)
	PROCESS_MAP("LIST", command_list)
	PROCESS_MAP_END(command_not_support)

	return ret_val;
}

int ftp_server_internal::serve_it(ftp_client *client)
{
	do_welcome(client);
	for(;;){
		request r;
		if(client->wait_request(&r) != 0){
			DEBUG("request error!\n");
			client->response(500, REPLY_SYNTAX_ERROR);
			client->do_response();
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

int ftp_server_internal::do_welcome(ftp_client *client)
{
	client->response(220, "Welcome");
	client->do_response();
	return 0;
}

serve_state ftp_server_internal::get_state()
{
	return state;
}

int ftp_server_internal::command_user(ftp_client *client, const char *param)
{
	DEBUG("command user\n");
	const serve_state state = get_state();
	if(state >= loggedin){
		if(strcmp(username.c_str(), param) == 0 ){
			client->response(331, REPLY_ANY_PSWD);
		}
		else{
			client->response(530, REPLY_CANNOT_CHANGE_USER);
		}
	}
	else if(param == NULL || *param == '\0' ){
		client->response(332, REPLY_NEED_USER);
	}
//	else if(!valid_username(param) ){
//		client->response(530, REPLY_NOT_LOGGED_IN);
	else{
		username = param;
		client->response(331, REPLY_NEED_PSWD);
	}
	client->do_response();
	return 0;
}

int ftp_server_internal::command_pass(ftp_client *client, const char *param)
{
	DEBUG("command pass\n");
	state = loggedin;
	client->response(230 ,REPLY_LOGGED_IN);
	client->do_response();
//	client->set_status(logged_in);
	return 0;
}

int ftp_server_internal::command_not_support(ftp_client *client)
{
	client->response(502, REPLY_NOT_IMPLEMENTED);
	client->do_response();
	return 0;
}

int ftp_server_internal::command_quit(ftp_client *client, const char *param)
{
	client->response(221, REPLY_BYE);
	client->do_response();
	return ERROR_CLIENT_QUIT;
}

int ftp_server_internal::command_pasv(ftp_client *client, const char *param)
{
	const serve_state state = get_state();
	if(state == ready){
		client->response(530, REPLY_NOT_LOGGED_IN);
	}
	else if(state == loggedin){
		ftp_client_datafile *dfile = new ftp_client_datafile;
		dfile->mode = PASV;
		unsigned short port = dfile->random_bind();
		DEBUG("Random port:%d\n", (int)port);
		this->state = datacnn_wait;
		client->response_format(227, REPLY_ENTRY_PASV_MODE":(%s,%d,%d)",
				get_serve_addr(), (int)((unsigned char *)(&port))[0],
				(int)((unsigned char*)(&port))[1] );
	}
	client->do_response();
	return 0;
}

		

int ftp_server_internal::command_pwd(ftp_client *client, const char *param)
{
	client->response_format(257, "\"%s\"", "/");
	client->do_response();
	return 0;
}

int ftp_server_internal::command_type(ftp_client *client, const char *param)
{
	if(param == NULL || param[0] == '\0' || param[1] != '\0'){
		client->response(504, REPLY_NOT_IMPL_FOR_PARAM);
	}
	else{
		switch(toupper(*param)){
			case 'A':
				this->type = type_A;
				client->response(200, REPLY_COMMAND_OK);
				break;
			case 'I':
				this->type = type_I;
				client->response(200, REPLY_COMMAND_OK);
				break;
			default:
				client->response(504, REPLY_NOT_IMPL_FOR_PARAM);
				break;
		}
	}
	client->do_response();
	return 0;
}

int ftp_server_internal::command_mode(ftp_client *client, const char *param)
{
	client->response(502, REPLY_NOT_IMPLEMENTED);
	client->do_response();
	return 0;
}

int ftp_server_internal::command_stru(ftp_client *client, const char *param)
{
	client->response(502, REPLY_NOT_IMPLEMENTED);
	client->do_response();
	return 0;
}

int ftp_server_internal::command_retr(ftp_client *client, const char *param)
{
	client->response(502, REPLY_NOT_IMPLEMENTED);
	client->do_response();
	return 0;
}

int ftp_server_internal::command_stor(ftp_client *client, const char *param)
{
	client->response(502, REPLY_NOT_IMPLEMENTED);
	client->do_response();
	return 0;
}

int ftp_server_internal::command_noop(ftp_client *client, const char *param)
{
	client->response(502, REPLY_NOT_IMPLEMENTED);
	client->do_response();
	return 0;
}

int ftp_server_internal::command_list(ftp_client *client, const char *param)
{
	client->response(502, REPLY_NOT_IMPLEMENTED);
	client->do_response();
	return 0;
}

