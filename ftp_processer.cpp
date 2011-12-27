#include "global.h"
#include "ftp_processer.h"
#include "ftp_client.h"
#include "ftp_clientinfo.h"
#include "request.h"
#include "messages.h"
#include <string.h>

static int command_user(ftp_client *client, const char *param)
{
	DEBUG("command user\n");
	const client_status state = client->get_status();
	ftp_clientinfo *info = client->get_clientinfo();
	if(state >= loggedin){
		if(strcmp(info->get_username(), param) == 0 ){
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
		info->set_username(param);
		client->response(331, REPLY_NEED_PSWD);
	}
	client->do_response();
	return 0;
}

static int command_pass(ftp_client *client, const char *param)
{
	DEBUG("command pass\n");
	client->response(230 ,REPLY_LOGGED_IN);
	client->do_response();
	return 0;
}

static int command_not_support(ftp_client *client)
{
	client->response(502, REPLY_NOT_IMPLEMENTED);
	client->do_response();
	return 0;
}

static int command_quit(ftp_client *client, const char *param)
{
	client->response(221, REPLY_BYE);
	client->do_response();
	return ERROR_CLIENT_QUIT;
}

static int command_pasv(ftp_client *client, const char *param)
{
	client->
#define PROCESS_MAP_BEGIN() if(0){}

#define PROCESS_MAP(COMMAND, PROCESS_FUNCTION) \
	else if(strcmp(command, COMMAND) == 0 ) { \
		DEBUG("Match Command: %s %s\n", command, COMMAND); \
		ret_val = PROCESS_FUNCTION(client, r->params); \
	}
#define PROCESS_MAP_END(NOT_FOUND_FUNCTION) \
	else{ \
		DEBUG("Command Not Support:%s %s\n", command, r->params == NULL?"":r->params);	\
		ret_val = NOT_FOUND_FUNCTION(client);\
	}

int ftp_processer::process_request(ftp_client *client, request *r)
{
	int ret_val = 0;
	const char *command = r->command;

	PROCESS_MAP_BEGIN()
	PROCESS_MAP("USER", command_user)
	PROCESS_MAP("PASS", command_pass)
	PROCESS_MAP("QUIT", command_quit)
	PROCESS_MAP_END(command_not_support)

	return ret_val;
}

