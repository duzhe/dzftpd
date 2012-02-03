#include "global.h"
#include "ftp_server.h"
//#include "ftp_client.h"
#include "ftp_ctrlfile.h"
#include "ftp_config.h"
#include "ftp_datafile.h"
#include "request.h"
#include "ftp_dir.h"
#include "ftp_user.h"
#include "messages.h"
#include <strings.h>
#include <sys/stat.h>
#include <dirent.h>
#include <assert.h> 
#include <fcntl.h>
#include <stdarg.h>  /* for va_list */
#include <string.h>  /* for strlen */
#include <stdlib.h>  /* for malloc and free */
#include <string>
#include <vector>


#define ERROR_CLIENT_CLOSED -2

// command process function return values:
#define CP_DONE 			0
#define CP_CONTINUE			1
#define CP_CLIENT_QUIT		-1
#define CP_INTERNAL_ERROR	-2

enum trans_type
{
	type_A,
	type_I,
};

class ftp_server_internal
{
public:
	ftp_server_internal(){type = type_I;}
	int serve_it(int ctrlfd);

	int response(response_code_t, const char *);
	int response_format(response_code_t, const char *format, ...);
	int do_response();

	int sent_file(const char *filename);
private:
	int clearup();
	int wait_request(request *r);
	int serve();
//DECLEARE_COMMAND_PROCESS_FUNCTION(COMMAND)
#define DCPF(COMMAND) int command_##COMMAND(const char *param)
//DECLEARE_COMMAND_ABORT_FUNCTION(FUNCTION)
#define DCAF(FUNCTION)	int FUNCTION()
//DECLEARE_COMMAND_ENSURE_FUNCTION(ENSURE_FUNCTION) int ENSURE_FUNCTION()
#define DCEF(ENSURE_FUNCTION)	int ENSURE_FUNCTION()
	int process_request(request *r);

	DCAF(do_welcome);
	DCAF(not_loggedin);
	DCAF(client_not_ready);
	DCAF(no_data_connection);
	DCAF(command_not_support);

	DCEF(ensure_loggedin);
	DCEF(ensure_data_connection);
	int ensure_file_access(const char *filename, char item);

	DCPF(user); // minimum implementation
	DCPF(pass); // minimum implementation
	DCPF(acct);
	DCPF(port); // minimum implementation
	DCPF(pasv);
	DCPF(quit);
	DCPF(pwd );
	DCPF(cwd);
	DCPF(rmd);
	DCPF(mkd);
	DCPF(dele);
	DCPF(cdup);
	DCPF(type); // minimum implementation
	DCPF(mode);
	DCPF(stru); // minimum implementation
	DCPF(retr); // minimum implementation
	DCPF(stor); // minimum implementation
	DCPF(noop); // minimum implementation
	DCPF(list);
	DCPF(syst);
	DCPF(feat);

	void reset_data_connection();
private:
	trans_type type;
	ftp_user   user;
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
		int ret_val = wait_request(&r);
		if(ret_val < 0){
			switch(ret_val){
				case ERROR_CLIENT_CLOSED:
					DEBUG("Client closed\n");
					return 0;
				default:
					DEBUG("request error!\n");
					response(500, REPLY_SYNTAX_ERROR);
					do_response();
					continue;
			}
		}
		ret_val =  process_request(&r);
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

int ftp_server_internal::sent_file(const char *filename)
{
	int fd = open(filename, O_RDONLY);
	if(fd == -1){
		return OPEN_FILE_ERROR;
	}

	char buf[4096];
	for(;;){
		ssize_t read_count = ::read(fd, buf, 4096);
		if(read_count == -1){
			switch(errno){
				case EINTR:
					continue;
				default:
					close(fd);
					dfile.reset();
					return READ_FILE_ERROR;
			}
		}
		if(read_count == 0){
			close(fd);
			dfile.reset();
			break;
		}

		ssize_t ret_val = dfile.write(buf, read_count);
		if(ret_val != 0){
			close(fd);
			dfile.reset();
			return ret_val;
		}
	}
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
	datacnn_state state = dfile.state();
	if(state != ready){
		dfile.reset();
	}
}

int ftp_server_internal::do_welcome()
{
	response(220, "Welcome.");
	do_response();
	return 0;
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

//IMPLEMENT_COMMAND_ENSURE_FUNCTION(F)
#define ICEF(F) int ftp_server_internal::F()
ICEF(ensure_data_connection)
{
	DEBUG("Temporary Implementation: ensure_data_connection\n");
	datacnn_state state = dfile.state();
	if(state == ready){
		return no_data_connection();
	}
	if(state == pasv_wait){
		dfile.accept_connection();	
		response(150, REPLY_DATACNN_CONNECTED);
	}
	else if(state == connected){
		response(125, REPLY_DATACNN_ALREADY_OPEN);
	}
	else if(state == port_wait){
		DEBUG("Port not implemented\n");
		return no_data_connection();
	}
	do_response();
	return CP_CONTINUE;
}

ICEF(ensure_loggedin)
{
	if(!user.loggedin() ){
		return not_loggedin();
	}
	return CP_CONTINUE;
}

int ftp_server_internal::ensure_file_access(const char *filename, char item)
{
	if(test_access(filename, item) == true){
		return CP_CONTINUE;
	}
	else{
		response(550, REPLY_CANNOT_OPEN_FILE);
		do_response();
	}
	return CP_DONE;
}

//IMPLEMENT_COMMAND_PROCESS_FUNCTION(COMMAND)
#define ICPF(CMD) int ftp_server_internal::command_##CMD(const char *param)

// ENSURE MACROS
#define ENSURE_PARAM()	\
	if(param == NULL){	\
		response(501, REPLY_SYNTAX_ERROR_IN_PARAM);	\
		do_response();	\
		return CP_DONE;\
	}\

#define ENSURE_FULLPATHNAME()\
	const std::string &fullpathname = working_dir.getfullpathname(param);

#define ENSURE_FILEACCESS(ITEM)\
ENSURE_FULLPATHNAME()\
{\
	int ret_val = ensure_file_access(fullpathname.c_str(), ITEM);\
	if(ret_val != CP_CONTINUE){\
		return ret_val;\
	}\
}

#define ENSURE_DATACONN()\
{\
	int ret_val = ensure_data_connection();\
	if(ret_val != CP_CONTINUE){\
		return ret_val;\
	}\
}

	

ICPF(user)
{
	if(user.loggedin() ){
		if(strcmp(user.get_username(), param) == 0 ){
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
		user.set_username(param);
		response(331, REPLY_NEED_PSWD);
	}
	do_response();
	return CP_DONE;
}

ICPF(pass)
{
	DEBUG("Temporary Implementation: pass\n");
	if(user.login(param) ){
		const char *homepath = user.homepath();
		working_dir.cd(homepath);
		response(230 ,REPLY_LOGGED_IN);
	}
	else{
		response(530, REPLY_NOT_LOGGED_IN);
	}
	do_response();
	return CP_DONE;
}


ICPF(quit)
{
	response(221, REPLY_BYE);
	do_response();
	return CP_CLIENT_QUIT;
}

ICPF(port)
{
//	dfile.reset();
//	dfile.mode = PORT;
	return command_not_support();
}

ICPF(pasv)
{
	dfile.reset();
	dfile.mode = PASV;
	in_addr_t addr = ctrlfile->get_host_addr();
	in_port_t port = dfile.random_bind(addr);
	DEBUG("Random port:%d\n", (int)port);
	response_format(227, REPLY_ENTRY_PASV_MODE" (%d,%d,%d,%d,%d,%d)",
			(int)((unsigned char *)(&addr))[0],
			(int)((unsigned char *)(&addr))[1],
			(int)((unsigned char *)(&addr))[2],
			(int)((unsigned char *)(&addr))[3],
			(int)((unsigned char *)(&port))[0],
			(int)((unsigned char *)(&port))[1] );

	do_response();
	return CP_DONE;
}

static inline bool show_item(struct dirent *dire)
{
	// file name
	if(dire->d_name[0] == '.')
		return false;
	// access permission
	// not implemented
	return true;
}

ICPF(list)
{
	ENSURE_FILEACCESS('r')
	ENSURE_DATACONN();
	DIR *dir = opendir(fullpathname.c_str() );
	if(dir == NULL){
		DEBUG("open dir return NULL,param:%s\n", fullpathname.c_str() );
		reset_data_connection();
		response(426, REPLY_TRANS_ABOUT);
		do_response();
		return CP_DONE;
	}
	ftp_dir request_dir(fullpathname.c_str() );
	int ret_val = 0;
	for(struct dirent *dire = readdir(dir); dire != NULL ;dire = readdir(dir) ){
		if(!show_item(dire)){
			continue;
		}

		struct stat statbuf;
		std::string itemfullpathname = request_dir.getfullpathname(dire->d_name);
		char item_line[MAX_PATH + 256];
		if(lstat(itemfullpathname.c_str(), &statbuf) < 0){
			continue;
		}
		
		// file type
		if(!S_ISDIR(statbuf.st_mode) && !S_ISREG(statbuf.st_mode) ){
			continue;
		}

		// access permission             1 2 3 4 5 6 7 8 9 0 link uid gid length
		//                               - r w x r - x - - -
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

		// ctime convert time_t into a null-terminated string of the form:
		//Wed Jun 30 21:49:08 1993\n"
		// the form we need is:
		//	  Jun 30 21:49
		//0123456789012345678901234
		const char *mtime = ctime(&statbuf.st_mtime);
		memcpy(pos, mtime+4, 16-4);
		pos += (16-4);

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
	dfile.reset();
	::closedir(dir);
	do_response();

	return CP_DONE;
}

		
ICPF(pwd)
{
	const char *cwd = working_dir.pwd();
	if(cwd != NULL){
		response_format(257, REPLY_PATHNAME_S_IS_YOUR_CWD, cwd);
	}
	else{
		response(550, REPLY_CANNOT_GET_CWD);
	}

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
				response(200, REPLY_OK);
				break;
			case 'I':
				this->type = type_I;
				response(200, REPLY_OK);
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
	ENSURE_PARAM();
	// Only support "Stream" mode
	// Stream mode
	if(strcmp(param, "S") == 0){
		response(200, REPLY_OK);
	}
	// Block mode
	else if(strcmp(param, "B") == 0){
		response(504, REPLY_NOT_IMPL_FOR_PARAM);
	}
	else{
		response(501, REPLY_SYNTAX_ERROR_IN_PARAM);
	}
	do_response();
	return CP_DONE;
}

ICPF(stru)
{
	ENSURE_PARAM();
	// only support "File" structure
	if(strcmp(param, "F") == 0){
		response(200, REPLY_OK);
	}
	else if(strcmp(param, "P") == 0){
		response(504, REPLY_NOT_IMPL_FOR_PARAM);
	}
	else if(strcmp(param, "R") == 0){
		response(504, REPLY_NOT_IMPL_FOR_PARAM);
	}
	else{
		response(501, REPLY_SYNTAX_ERROR_IN_PARAM);
	}
	do_response();
	return CP_DONE;
}

ICPF(retr)
{
	ENSURE_FULLPATHNAME();
	ENSURE_DATACONN();
	int ret_val = sent_file(fullpathname.c_str() );
	switch(ret_val){
		case 0:
			response(226, REPLY_CLOSING_DATACNN);
			break;
		case NO_DATA_CONNECTION:
			response(425, REPLY_CANNOT_OPEN_DATACNN);
			break;
		case OPEN_FILE_ERROR:
			response(451, REPLY_CANNOT_OPEN_FILE);
			break;
		case CLIENT_CLOSE_DATA_CONNECTION:
			response(426, REPLY_DATACNN_ABORT);
			break;
		default:
			response(426, REPLY_DATACNN_ABORT);
			break;
	}
	dfile.reset();
	do_response();
	return CP_DONE;
}

ICPF(stor)
{
	ENSURE_FILEACCESS('w');

	ENSURE_DATACONN();
	DEBUG("Temporary Implementation: stor\n");
	FILE *fp = fopen(fullpathname.c_str(), "wb");
	if(fp == NULL){
		response(553, REPLY_CANNOT_OPENFILE_FOR_WRITE);
		do_response();
		return CP_DONE;
	}
	char buf[BUF_SIZE];
	for(;;){
		int read_count = dfile.read(buf, BUF_SIZE);
		if(read_count > 0){
			fwrite(buf, 1, read_count, fp);
			continue;
		}
		switch(read_count){
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
		break;
	}
	fclose(fp);
	dfile.reset();
	do_response();
	return CP_DONE;
}

ICPF(syst)
{
	DEBUG("Not Implemented: syst\n");
	return command_not_support();
}

ICPF(feat)
{
	DEBUG("Not Implemented: feat\n");
	return command_not_support();
}

ICPF(acct)
{
	DEBUG("Not Implemented: acct\n");
	return command_not_support();
}

ICPF(noop)
{
	response(200, REPLY_OK);
	do_response();
	return CP_DONE;
}

ICPF(cwd)
{
	if(working_dir.cd(param) != 0){
		response_format(550, REPLY_CANT_CD_TO_PATHNAME_S, working_dir.pwd() );
	}
	else{
		response_format(250, REPLY_OK_WD_IS_PATHNAME_S, working_dir.pwd() );
	}
	do_response();
	return CP_DONE;
}

ICPF(cdup)
{
	if(working_dir.cdup() != 0){
		response_format(550, REPLY_CANT_CD_TO_PATHNAME_S, working_dir.pwd() );
	}
	else{
		response_format(250, REPLY_OK_WD_IS_PATHNAME_S, working_dir.pwd() );
	}
	do_response();
	return CP_DONE;
}

ICPF(rmd)
{
	ENSURE_PARAM();
	ENSURE_FILEACCESS('w');
	int ret_val = rmdir(fullpathname.c_str() );
	if(ret_val == 0){
		response(250, REPLY_RMDIR_SUCCESS);
	}
	else{
		response(550, REPLY_RMDIR_NOT_SUCCESS);
	}
	do_response();
	return CP_DONE;
}

#define RWXRWXRWX (S_IRWXU | S_IRWXG | S_IRWXO)
ICPF(mkd)
{
	ENSURE_PARAM();
	ENSURE_FULLPATHNAME();
	int ret_val = ::mkdir(fullpathname.c_str(), RWXRWXRWX);
	if(ret_val == 0){
		response(257,REPLY_PATHNAME_S_CREATED);
	}
	else{
		response(550,REPLY_MKDIR_NOT_SUCCESS);
	}
	do_response();
	return CP_DONE;
}

ICPF(dele)
{
	ENSURE_PARAM();
	ENSURE_FULLPATHNAME();
	int ret_val = ::unlink(fullpathname.c_str() );
	if(ret_val == 0){
		response(250, REPLY_FILE_DELETED);
	}
	else{
		response(550, REPLY_FILE_NOT_DELETED);
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
	PROCESS_MAP("RMD", 	command_rmd)
	PROCESS_MAP("MKD", 	command_mkd)
	PROCESS_MAP("DELE", command_dele)
	PROCESS_MAP("CDUP",	command_cdup)
	PROCESS_MAP("NOOP", command_noop)
	PROCESS_MAP("SYST", command_syst)
	PROCESS_MAP("FEAT", command_feat)
//	PROCESS_ENSURE(ensure_data_connection)
	PROCESS_MAP("RETR", command_retr)
	PROCESS_MAP("STOR", command_stor)
	PROCESS_MAP("LIST", command_list)
	PROCESS_MAP_END(command_not_support)

	return ret_val;
}

