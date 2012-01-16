#include "global.h"
#include "ftp_server.h"
#include "ftp_client.h"
#include "ftp_config.h"
#include "ftp_client_datafile.h"
#include "request.h"
#include "ftp_dir.h"
#include "messages.h"
#include <strings.h>
#include <string.h>
#include <sys/stat.h>
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
#define DECLEARE_COMMAND_PROCESS_FUNCTION(COMMAND) int command_##COMMAND(ftp_client *client, const char *param)
#define DCPF(C) DECLEARE_COMMAND_PROCESS_FUNCTION(C)
#define DECLEARE_COMMAND_ABORT_FUNCTION(FUNCTION) int FUNCTION(ftp_client *client)
#define DCAF(F)	DECLEARE_COMMAND_ABORT_FUNCTION(F)
#define DECLEARE_COMMAND_ENSURE_FUNCTION(ENSURE_FUNCTION) int ENSURE_FUNCTION(ftp_client *client)
#define DCEF(F)	DECLEARE_COMMAND_ENSURE_FUNCTION(F)
	int process_request(ftp_client *client, request *r);

	DCAF(do_welcome);
	DCAF(not_loggedin);
	DCAF(client_not_ready);
	DCAF(no_data_connection);
	DCAF(command_not_support);
//	DCAF(ensure_data_connection);

	DCEF(ensure_loggedin);
	DCEF(ensure_data_connection);

	DCPF(user);
	DCPF(pass);
	DCPF(acct);
	DCPF(port);
	DCPF(pasv);
	DCPF(quit);
	DCPF(pwd );
	DCPF(cwd);
	DCPF(cdup);
	DCPF(type);
	DCPF(mode);
	DCPF(stru);
	DCPF(retr);
	DCPF(stor);
	DCPF(noop);
	DCPF(list);
	DCPF(syst);
	DCPF(feat);

	serve_state get_state();
	void reset_data_connection();
private:
	serve_state state;
	trans_type type;
	std::string username;
	ftp_dir	working_dir;
	ftp_client_datafile dfile;
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

void ftp_server_internal::reset_data_connection()
{
	if(state == datacnn_wait || state == datacnn_ready){
		dfile.reset();
		state = loggedin;
	}
}

int ftp_server_internal::do_welcome(ftp_client *client)
{
	client->response(220, "Welcome.");
	client->do_response();
	return 0;
}

serve_state ftp_server_internal::get_state()
{
	return state;
}

int ftp_server_internal::client_not_ready(ftp_client *client)
{
	return command_not_support(client);
}

int ftp_server_internal::not_loggedin(ftp_client *client)
{
	client->response(530, REPLY_NOT_LOGGED_IN);
	client->do_response();
	return 0;
}

int ftp_server_internal::no_data_connection(ftp_client *client)
{
	client->response(425, REPLY_NO_DATA_CONNECTION);
	client->do_response();
	return 0;
}

int ftp_server_internal::command_not_support(ftp_client *client)
{
	client->response(502, REPLY_NOT_IMPLEMENTED);
	client->do_response();
	return 0;
}

#define IMPLEMENT_COMMAND_ENSURE_FUNCTION(F) int ftp_server_internal::F(ftp_client *client)
#define ICEF(F) IMPLEMENT_COMMAND_ENSURE_FUNCTION(F) 
ICEF(ensure_data_connection)
{
	if(state == datacnn_wait){
		dfile.accept_connection();	
	}
	client->response(150, REPLY_DATACNN_CONNECTED);
	client->do_response();
	return 0;
}

ICEF(ensure_loggedin)
{
	if(state == ready){
		(void)not_loggedin(client);
		return -1;
	}
	return 0;
}

#define IMPLEMENT_COMMAND_PROCESS_FUNCTION(COMMAND) int ftp_server_internal::command_##COMMAND( \
		ftp_client *client, const char *param)
#define ICPF(C) IMPLEMENT_COMMAND_PROCESS_FUNCTION(C)
ICPF(user)
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

ICPF(pass)
{
	DEBUG("command pass\n");
	DEBUG("Temporary Implementation: pass\n");
	state = loggedin;
	client->response(230 ,REPLY_LOGGED_IN);
	client->do_response();
//	client->set_status(logged_in);
	const char *home = "/";
	working_dir.cd(home);
	return 0;
}


ICPF(quit)
{
	client->response(221, REPLY_BYE);
	client->do_response();
	return ERROR_CLIENT_QUIT;
}

ICPF(pasv)
{
	const serve_state state = get_state();
	if(state == datacnn_wait || datacnn_ready){
		reset_data_connection();
	}
	if(state == loggedin){
		dfile.mode = PASV;
		unsigned short port = dfile.random_bind();
		DEBUG("Random port:%d\n", (int)port);
		this->state = datacnn_wait;
		client->response_format(227, REPLY_ENTRY_PASV_MODE":(%s,%d,%d)",
				get_serve_addr(), (int)((unsigned char *)(&port))[0],
				(int)((unsigned char*)(&port))[1] );
	}
	client->do_response();
	return 0;
}

ICPF(list)
{
	int ret_val = ensure_data_connection(client);
	if(ret_val != 0){
		return no_data_connection(client);
	}		
	DEBUG("Temporary Implementation: list\n");

//	struct stat stat_buf;
//	ret_val = stat(param, &stat_buf);
//	if(ret_val == 0){
//		if(!S_ISREG(stat_buf.st_mode) && S_ISDIR(stat_buf.st_mode) ){
//			continue;
//		}
//
//	}
//	else{
//		client->response(226, REPLY_CLOSING_DATACNN);
//	}
	ret_val = dfile.write_file("/home/duzhe/repo/dzftp/debug/list.txt");
	switch(ret_val){
		case 0:
			client->response(226, REPLY_CLOSING_DATACNN);
			break;
		case NO_DATA_CONNECTION:
			client->response(425, REPLY_CANNOT_OPEN_DATACNN);
			break;
		case OPEN_FILE_ERROR:
			client->response(452, REPLY_CANNOT_OPEN_FILE);
			break;
		case CLIENT_CLOSE_DATA_CONNECTION:
			client->response(426, REPLY_DATACNN_ABORT);
			break;
		default:
			client->response(426, REPLY_DATACNN_ABORT);
			break;
	}
	client->do_response();
	state = loggedin;
	return 0;
}

		
ICPF(port)
{
	return command_not_support(client);
}

ICPF(pwd)
{
	DEBUG("Temporary Implementation: pwd\n");
	client->response_format(257, "\"%s\"", working_dir.pwd() );
	client->do_response();
	return 0;
}

ICPF(type)
{
	if(param == NULL || param[0] == '\0' || param[1] != '\0'){
		client->response(504, REPLY_NOT_IMPL_FOR_PARAM);
	}
	else{
		DEBUG("Temporary Implementation: type\n");
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

ICPF(mode)
{
	return command_not_support(client);
}

ICPF(stru)
{
	return command_not_support(client);
}

ICPF(retr)
{
	int ret_val = ensure_data_connection(client);
	if(ret_val != 0){
		return no_data_connection(client);
	}		
	DEBUG("Temporary Implementation: retr\n");
	if(*param == '/'){
		ret_val = dfile.write_file(param);
	}
	else{
		std::string fullfilename = working_dir.pwd() ;
		fullfilename.append("/");
		fullfilename.append(param);
		ret_val = dfile.write_file(fullfilename.c_str() );
	}
	switch(ret_val){
		case 0:
			client->response(226, REPLY_CLOSING_DATACNN);
			break;
		case NO_DATA_CONNECTION:
			client->response(425, REPLY_CANNOT_OPEN_DATACNN);
			break;
		case OPEN_FILE_ERROR:
			client->response(452, REPLY_CANNOT_OPEN_FILE);
			break;
		case CLIENT_CLOSE_DATA_CONNECTION:
			client->response(426, REPLY_DATACNN_ABORT);
			break;
		default:
			client->response(426, REPLY_DATACNN_ABORT);
			break;
	}
	client->do_response();
	state = loggedin;
	return 0;
}

ICPF(stor)
{
	return command_not_support(client);
}

ICPF(syst)
{
	return command_not_support(client);
}

ICPF(feat)
{
	return command_not_support(client);
}

ICPF(acct)
{
	return command_not_support(client);
}

ICPF(noop)
{
	return command_not_support(client);
}

ICPF(cwd)
{
	if(working_dir.cd(param) != 0){
		client->response_format(550, "Can't change working directory to %s", working_dir.pwd() );
	}
	else{
		client->response_format(250, "OK. Current working directory is %s", working_dir.pwd() );
	}
	client->do_response();
	return 0;
}

ICPF(cdup)
{
	if(working_dir.cdup() != 0){
		client->response_format(550, "Can't change working directory to %s", working_dir.pwd() );
	}
	else{
		client->response_format(250, "OK. Current working directory is %s", working_dir.pwd() );
	}
	client->do_response();
	return 0;
}

#define PROCESS_MAP_BEGIN()	\
do{ 

#define PROCESS_MAP(COMMAND, PROCESS_FUNCTION) \
	if(strcmp(command, COMMAND) == 0 ) { \
		ret_val = PROCESS_FUNCTION(client, r->params); \
		break; \
	}

#define STATE_THROUGH(STATE, NOT_ALLOW_FUNCTION) \
	if(state < STATE) {\
		DEBUG("State Not through:%s %s\n", command, r->params == NULL?"":r->params);	\
		ret_val = NOT_ALLOW_FUNCTION(client);\
		break; \
	}	\

#define PROCESS_ENSURE(ENSURE_FUNCTION) \
	ret_val = ENSURE_FUNCTION(client); \
	if(ret_val < 0){ \
		break; \
	}\

#define PROCESS_MAP_END(NOT_SUPPORT_FUNCTION) \
	else{ \
		DEBUG("Command Not Support:%s %s\n", command, r->params == NULL?"":r->params);	\
		ret_val = NOT_SUPPORT_FUNCTION(client);\
	}		\
}while(false);

int ftp_server_internal::process_request(ftp_client *client, request *r)
{
	int ret_val = 0;
	const char *command = r->command;

	PROCESS_MAP_BEGIN()
//	STATE_THROUGH(ready, client_not_ready)
	PROCESS_MAP("USER", command_user)
	PROCESS_MAP("PASS", command_pass)
	PROCESS_MAP("ACCT", command_acct)
	PROCESS_MAP("QUIT", command_quit)
	PROCESS_MAP("TYPE", command_type)
	PROCESS_MAP("MODE", command_mode)
	PROCESS_MAP("STRU", command_stru)
//	STATE_THROUGH(loggedin, not_loggedin)
	PROCESS_ENSURE(ensure_loggedin)
	PROCESS_MAP("PASV", command_pasv)
	PROCESS_MAP("PORT", command_port)
	PROCESS_MAP("PWD", 	command_pwd)
	PROCESS_MAP("CWD", 	command_cwd)
	PROCESS_MAP("CDUP",	command_cdup)
	PROCESS_MAP("NOOP", command_noop)
	PROCESS_MAP("SYST", command_syst)
	PROCESS_MAP("FEAT", command_feat)
//	STATE_THROUGH(datacnn_wait, no_data_connection)
	PROCESS_ENSURE(ensure_data_connection)
	PROCESS_MAP("RETR", command_retr)
	PROCESS_MAP("STOR", command_stor)
	PROCESS_MAP("LIST", command_list)
	PROCESS_MAP_END(command_not_support)

	return ret_val;
}

