#include "global.h"
#include "ftp_client.h"
#include "request.h"
#include "ftp_clientinfo.h"
//#include <unistd.h>
//#include <string.h> /* for memmove */
#include <vector>
#include "ftp_client_ctrlfile.h"
#include <stdio.h>
#include <stdarg.h>  /* for va_list */
#include <string.h>  /* for strlen */
#include <stdlib.h>  /* for malloc and free */
#include <assert.h> 

class ftp_client_internal
{
public:
	ftp_client_internal(int ctrlfd);
	~ftp_client_internal();
	int wait_request(request *r);
	int response(response_code_t, const char *);
	int do_response();
private:
	int parse_request(request *r);
//	int ctrlfd;

	ftp_client_ctrlfile *ctrlfile;
	//ftp_client_datafile *datafile;
	std::vector<const char *> response_list;
	typedef std::vector<const char *>::iterator Iter;
	response_code_t response_code;
public:
	ftp_clientinfo info;
};


ftp_client::ftp_client(int client_ctrlfd)
{
	internal = new ftp_client_internal(client_ctrlfd);
//	internal->ctrlfile = new ftp_client_ctrlfile(client_ctrlfd);
//	internal->ctrlfd = client_ctrlfd;
//	internal->data_in_read_buf = 0;
//	internal->data_in_write_buf = 0;
}


ftp_client::~ftp_client()
{
	close();
	if(internal != NULL){
		delete internal;
		internal = NULL;
	}
}

int ftp_client::close()
{
//	if(internal != NULL){
//		ctrlfile->close();
//		delete ctrlfile;
//		ctrlfile = NULL;
//	}
	return 0;
}

int ftp_client::response(response_code_t code, const char *message)
{
	return internal->response(code, message);
}

int ftp_client::response_format(response_code_t code, const char *format, ...)
{
	char message[MAX_RESPONSE_LENGTH];
	va_list ap;
	va_start(ap, format);
	vsnprintf(message, sizeof(message), format, ap);
	va_end(ap);	
	return internal->response(code, message);
}

int ftp_client::do_response()
{
	return internal->do_response();
}

int ftp_client::wait_request(request *r)
{
	return internal->wait_request(r);
}

ftp_client_internal::ftp_client_internal(int ctrlfd)
{
	ctrlfile = new ftp_client_ctrlfile(ctrlfd);
}

ftp_client_internal::~ftp_client_internal()
{
	if(ctrlfile != NULL){
		delete ctrlfile;
	}
}

int ftp_client_internal::wait_request(request *r)
{
	char request_line[MAX_REQUEST_LENGTH];
	int ret_val = ctrlfile->readline(request_line);
	if(ret_val != 0){
		return ERROR_CLIENT_CLOSED;
	}
	DEBUG("Request Line:%s\n", request_line);
	return r->parse_from_line(request_line);
}

int ftp_client_internal::response(response_code_t code, const char *message)
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

int ftp_client_internal::do_response()
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

ftp_clientinfo *ftp_client::get_clientinfo()
{
	return &(internal->info);
}

client_status ftp_client::get_status()
{
	return ready;
}

