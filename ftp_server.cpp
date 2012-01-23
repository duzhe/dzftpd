#include "global.h"
#include "ftp_server.h"
//#include "ftp_client.h"
#include "ftp_ctrlfile.h"
#include "ftp_config.h"
#include "ftp_datafile.h"
#include "request.h"
#include "ftp_dir.h"
#include "messages.h"
#include <strings.h>
#include <sys/stat.h>
#include <dirent.h>
#include <assert.h> 
#include <stdarg.h>  /* for va_list */
#include <string.h>  /* for strlen */
#include <stdlib.h>  /* for malloc and free */
#include <string>
#include <vector>


#define ERROR_CLIENT_CLOSED -2

// command process function return values:
#define CP_DONE 		0
#define CP_CONTINUE			1
#define CP_CLIENT_QUIT -1


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

class ftp_server_internal
{
public:
	ftp_server_internal(){state = ready;type = type_I;}
	int serve_it(int ctrlfd);

	int response(response_code_t, const char *);
	int response_format(response_code_t, const char *format, ...);
	int do_response();
private:
	int clearup();
	int wait_request(request *r);
	int serve();
#define DECLEARE_COMMAND_PROCESS_FUNCTION(COMMAND) int command_##COMMAND(const char *param)
#define DCPF(C) DECLEARE_COMMAND_PROCESS_FUNCTION(C)
#define DECLEARE_COMMAND_ABORT_FUNCTION(FUNCTION) int FUNCTION()
#define DCAF(F)	DECLEARE_COMMAND_ABORT_FUNCTION(F)
#define DECLEARE_COMMAND_ENSURE_FUNCTION(ENSURE_FUNCTION) int ENSURE_FUNCTION()
#define DCEF(F)	DECLEARE_COMMAND_ENSURE_FUNCTION(F)
	int process_request(request *r);

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


	ftp_datafile dfile;
	ftp_ctrlfile *ctrlfile;
	std::vector<const char *> response_list;
	typedef std::vector<const char *>::iterator Iter;
	response_code_t response_code;
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
	return internal->serve_it(client_ctrlfd);
//	ftp_client client(client_ctrlfd);
//	return internal->serve_it(&client);
}

int ftp_server::response(response_code_t code, const char *message)
{
	return internal->response(code, message);
}

int ftp_server::response_format(response_code_t code, const char *format, ...)
{
	char message[MAX_RESPONSE_LENGTH];
	va_list ap;
	va_start(ap, format);
	vsnprintf(message, sizeof(message), format, ap);
	va_end(ap);	
	return internal->response(code, message);
}

int ftp_server::do_response()
{
	return internal->do_response();
}

//---------vvvvvvvvvvvv------- internal methods -------vvvvvvvvvvvvvv---------

int ftp_server_internal::serve_it(int ctrlfd)
{
	ctrlfile = new ftp_ctrlfile(ctrlfd);
	return serve();
}

int ftp_server_internal::serve()
{
	do_welcome();
	for(;;){
		request r;
		if(wait_request(&r) != 0){
			DEBUG("request error!\n");
			response(500, REPLY_SYNTAX_ERROR);
			do_response();
			continue;
		}
		int ret_val =  process_request(&r);
		if(ret_val != 0){
			if( ret_val == CP_CLIENT_QUIT){
				break;
			}
		}
	}
	clearup();
	return 0;
}

int ftp_server_internal::response(response_code_t code, const char *message)
{
	if(code != 0){
		response_code = code;
	}
	if(message!= NULL){
		int length = strlen(message);
		char *entry = (char *)malloc(length +1);
		memcpy(entry, message, length +1);
		response_list.push_back(entry);
	}
	return 0;
}

int ftp_server_internal::response_format(response_code_t code, const char *format, ...)
{
	char message[MAX_RESPONSE_LENGTH];
	va_list ap;
	va_start(ap, format);
	vsnprintf(message, sizeof(message), format, ap);
	va_end(ap);	
	return response(code, message);
}

int ftp_server_internal::do_response()
{
	assert(response_code != 0);
	for(Iter iter = response_list.begin();
			iter != response_list.end(); ++iter){
		ctrlfile->printf("%3d%c%s\r\n", response_code, 
				iter+1 == response_list.end()? ' ':'-',
				*iter);
	}
	ctrlfile->flush();
	for(Iter iter = response_list.begin(); iter != response_list.end(); ++iter){
		free( (void*)(*iter) );
	}
	response_list.clear();
	return 0;
}

int ftp_server_internal::wait_request(request *r)
{
	char request_line[MAX_REQUEST_LENGTH];
	int ret_val = ctrlfile->readline(request_line);
	if(ret_val != 0){
		return ERROR_CLIENT_CLOSED;
	}
	DEBUG("Request Line:%s\n", request_line);
	return r->parse_from_line(request_line);
}

int ftp_server_internal::clearup()
{
	DEBUG("ftp_server_internal::clearup not implemented\n");
	return 0;
}

void ftp_server_internal::reset_data_connection()
{
	if(state == datacnn_wait || state == datacnn_ready){
		dfile.reset();
		state = loggedin;
	}
}

int ftp_server_internal::do_welcome()
{
	response(220, "Welcome.");
	do_response();
	return 0;
}

serve_state ftp_server_internal::get_state()
{
	return state;
}

int ftp_server_internal::client_not_ready()
{
	return command_not_support();
}

int ftp_server_internal::not_loggedin()
{
	response(530, REPLY_NOT_LOGGED_IN);
	do_response();
	return CP_DONE;
}

int ftp_server_internal::no_data_connection()
{
	response(425, REPLY_NO_DATA_CONNECTION);
	do_response();
	return CP_DONE;
}

int ftp_server_internal::command_not_support()
{
	response(502, REPLY_NOT_IMPLEMENTED);
	do_response();
	return CP_DONE;
}

#define IMPLEMENT_COMMAND_ENSURE_FUNCTION(F) int ftp_server_internal::F()
#define ICEF(F) IMPLEMENT_COMMAND_ENSURE_FUNCTION(F) 
ICEF(ensure_data_connection)
{
	if(state < datacnn_wait){
		return no_data_connection();
	}
	if(state == datacnn_wait){
		dfile.accept_connection();	
		response(150, REPLY_DATACNN_CONNECTED);
	}
	else if(state == datacnn_ready){
		response(125, REPLY_DATACNN_ALREADY_OPEN);
	}
	do_response();
	return CP_CONTINUE;
}

ICEF(ensure_loggedin)
{
	if(state == ready){
		return not_loggedin();
	}
	return CP_CONTINUE;
}

#define IMPLEMENT_COMMAND_PROCESS_FUNCTION(COMMAND) int ftp_server_internal::command_##COMMAND( \
		const char *param)
#define ICPF(C) IMPLEMENT_COMMAND_PROCESS_FUNCTION(C)
ICPF(user)
{
	DEBUG("command user\n");
	const serve_state state = get_state();
	if(state >= loggedin){
		if(strcmp(username.c_str(), param) == 0 ){
			response(331, REPLY_ANY_PSWD);
		}
		else{
			response(530, REPLY_CANNOT_CHANGE_USER);
		}
	}
	else if(param == NULL || *param == '\0' ){
		response(332, REPLY_NEED_USER);
	}
//	else if(!valid_username(param) ){
//		response(530, REPLY_NOT_LOGGED_IN);
	else{
		username = param;
		response(331, REPLY_NEED_PSWD);
	}
	do_response();
	return CP_DONE;
}

ICPF(pass)
{
	DEBUG("command pass\n");
	DEBUG("Temporary Implementation: pass\n");
	state = loggedin;
	response(230 ,REPLY_LOGGED_IN);
	do_response();
	const char *home = "/";
	working_dir.cd(home);
	return CP_DONE;
}


ICPF(quit)
{
	response(221, REPLY_BYE);
	do_response();
	return CP_CLIENT_QUIT;
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
		response_format(227, REPLY_ENTRY_PASV_MODE"(%s,%d,%d)",
				get_serve_addr(), (int)((unsigned char *)(&port))[0],
				(int)((unsigned char*)(&port))[1] );
	}
	do_response();
	return CP_DONE;
}

ICPF(list)
{
	const std::string &request_path = working_dir.getfullpathname(param);
	DIR *dir = opendir(request_path.c_str() );
	if(dir == NULL){
		dfile.reset();
		response(426, REPLY_TRANS_ABOUT);
		do_response();
		return CP_DONE;
	}
	ftp_dir request_dir(request_path.c_str() );
	int ret_val = 0;
	for(struct dirent *dire = readdir(dir); dire != NULL ;dire = readdir(dir) ){
		// file name
		if(dire->d_name[0] == '.' ){
			continue;
		}

		struct stat statbuf;
		std::string fullpathname = request_dir.getfullpathname(dire->d_name);
		char item_line[MAX_PATH + 256];
		if(lstat(fullpathname.c_str(), &statbuf) < 0){
			continue;
		}
		
		// file type
		if(!S_ISDIR(statbuf.st_mode) && !S_ISREG(statbuf.st_mode) ){
			continue;
		}

		// access permission
		int count = sprintf(item_line, "%c%c%c%c%c%c%c%c%c%c %4d %5d %5d %12ld ",
				S_ISREG(statbuf.st_mode)?'-':'d',
				(statbuf.st_mode & S_IRUSR)?'r':'-',
                (statbuf.st_mode & S_IWUSR)?'w':'-',
                (statbuf.st_mode & S_IXUSR)?'x':'-',
                (statbuf.st_mode & S_IRGRP)?'r':'-',
                (statbuf.st_mode & S_IWGRP)?'w':'-',
                (statbuf.st_mode & S_IXGRP)?'x':'-',
                (statbuf.st_mode & S_IROTH)?'r':'-',
                (statbuf.st_mode & S_IWOTH)?'w':'-',
                (statbuf.st_mode & S_IXOTH)?'x':'-',
				statbuf.st_nlink,
				statbuf.st_uid,
				statbuf.st_gid,
				statbuf.st_size
			   );
		char *pos = item_line + count;
		const char *mtime = ctime(&statbuf.st_mtime);
		memcpy(pos, mtime+4, 12);

		pos += 12;
		*pos++ = ' ';
		for(const char *p = dire->d_name; *p != '\0'; p++){
			*pos++ = *p;
		}
		*pos++ = '\r';
		*pos++ = '\n';

		ret_val = dfile.write(item_line, pos-item_line);
		switch(ret_val){
			case 0:
				continue;
			case NO_DATA_CONNECTION:
				response(425, REPLY_CANNOT_OPEN_DATACNN);
				break;
			case OPEN_FILE_ERROR:
				response(452, REPLY_CANNOT_OPEN_FILE);
				break;
			case CLIENT_CLOSE_DATA_CONNECTION:
				response(426, REPLY_DATACNN_ABORT);
				break;
			default:
				response(426, REPLY_DATACNN_ABORT);
				break;
		}
		break;
	}
	if(ret_val == 0){
		response(226, REPLY_CLOSING_DATACNN);
	}
	do_response();
	dfile.reset();
	::closedir(dir);

	do_response();
	state = loggedin;
	return CP_DONE;
}

		
ICPF(port)
{
	return command_not_support();
}

ICPF(pwd)
{
	DEBUG("Temporary Implementation: pwd\n");
	response_format(257, "\"%s\"", working_dir.pwd() );
	do_response();
	return CP_DONE;
}

ICPF(type)
{
	if(param == NULL || param[0] == '\0' || param[1] != '\0'){
		response(504, REPLY_NOT_IMPL_FOR_PARAM);
	}
	else{
		DEBUG("Temporary Implementation: type\n");
		switch(toupper(*param)){
			case 'A':
				this->type = type_A;
				response(200, REPLY_COMMAND_OK);
				break;
			case 'I':
				this->type = type_I;
				response(200, REPLY_COMMAND_OK);
				break;
			default:
				response(504, REPLY_NOT_IMPL_FOR_PARAM);
				break;
		}
	}
	do_response();
	return CP_DONE;
}

ICPF(mode)
{
	return command_not_support();
}

ICPF(stru)
{
	return command_not_support();
}

ICPF(retr)
{
	DEBUG("Temporary Implementation: retr\n");
//	if(*param == '/'){
//		ret_val = dfile.write_file(param);
//	}
//	else{
//		std::string fullfilename = working_dir.pwd() ;
//		fullfilename.append("/");
//		fullfilename.append(param);
//		ret_val = dfile.write_file(fullfilename.c_str() );
//	}
	const std::string &fullpathname = working_dir.getfullpathname(param);
	int ret_val = dfile.write_file(fullpathname.c_str() );
	switch(ret_val){
		case 0:
			response(226, REPLY_CLOSING_DATACNN);
			break;
		case NO_DATA_CONNECTION:
			response(425, REPLY_CANNOT_OPEN_DATACNN);
			break;
		case OPEN_FILE_ERROR:
			response(452, REPLY_CANNOT_OPEN_FILE);
			break;
		case CLIENT_CLOSE_DATA_CONNECTION:
			response(426, REPLY_DATACNN_ABORT);
			break;
		default:
			response(426, REPLY_DATACNN_ABORT);
			break;
	}
	do_response();
	state = loggedin;
	return CP_DONE;
}

ICPF(stor)
{
	return command_not_support();
}

ICPF(syst)
{
	return command_not_support();
}

ICPF(feat)
{
	return command_not_support();
}

ICPF(acct)
{
	return command_not_support();
}

ICPF(noop)
{
	return command_not_support();
}

ICPF(cwd)
{
	if(working_dir.cd(param) != 0){
		response_format(550, "Can't change working directory to %s", working_dir.pwd() );
	}
	else{
		response_format(250, "OK. Current working directory is %s", working_dir.pwd() );
	}
	do_response();
	return CP_DONE;
}

ICPF(cdup)
{
	if(working_dir.cdup() != 0){
		response_format(550, "Can't change working directory to %s", working_dir.pwd() );
	}
	else{
		response_format(250, "OK. Current working directory is %s", working_dir.pwd() );
	}
	do_response();
	return CP_DONE;
}

#define PROCESS_MAP_BEGIN()	\
do{ 

#define PROCESS_MAP(COMMAND, PROCESS_FUNCTION) \
	if(strcmp(command, COMMAND) == 0 ) { \
		ret_val = PROCESS_FUNCTION(r->params); \
		break; \
	}

#define PROCESS_ENSURE(ENSURE_FUNCTION) \
	ret_val = ENSURE_FUNCTION(); \
	if(ret_val != CP_CONTINUE){ \
		break; \
	}\

#define PROCESS_MAP_END(NOT_SUPPORT_FUNCTION) \
	else{ \
		DEBUG("Command Not Support:%s %s\n", command, r->params == NULL?"":r->params);	\
		ret_val = NOT_SUPPORT_FUNCTION();\
	}		\
}while(false);

int ftp_server_internal::process_request(request *r)
{
	int ret_val = 0;
	const char *command = r->command;

	PROCESS_MAP_BEGIN()
	PROCESS_MAP("USER", command_user)
	PROCESS_MAP("PASS", command_pass)
	PROCESS_MAP("ACCT", command_acct)
	PROCESS_MAP("QUIT", command_quit)
	PROCESS_MAP("TYPE", command_type)
	PROCESS_MAP("MODE", command_mode)
	PROCESS_MAP("STRU", command_stru)
	PROCESS_ENSURE(ensure_loggedin)
	PROCESS_MAP("PASV", command_pasv)
	PROCESS_MAP("PORT", command_port)
	PROCESS_MAP("PWD", 	command_pwd)
	PROCESS_MAP("CWD", 	command_cwd)
	PROCESS_MAP("CDUP",	command_cdup)
	PROCESS_MAP("NOOP", command_noop)
	PROCESS_MAP("SYST", command_syst)
	PROCESS_MAP("FEAT", command_feat)
	PROCESS_ENSURE(ensure_data_connection)
	PROCESS_MAP("RETR", command_retr)
	PROCESS_MAP("STOR", command_stor)
	PROCESS_MAP("LIST", command_list)
	PROCESS_MAP_END(command_not_support)

	return ret_val;
}

