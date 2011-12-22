#include "global.h"
#include "ftp_client.h"
#include "request.h"
#include <unistd.h>
#include <string.h> /* for memmove */

class ftp_client_internal
{
public:
	ftp_client_internal();
	~ftp_client_internal();
	int wait_request(request *r);
	int response(response_code, const char *);
	int do_response();
private:
	int parse_request(request *r);
//	int ctrlfd;

	ftp_client_ctrlfile *ctrlfile;

	char read_buf[REQUEST_MAX_LENGH];
	size_t data_in_read_buf;

};


ftp_client::ftp_client(int client_ctrlfd)
{
	ctrlfile = new ftp_client_ctrlfile(client_ctrlfd);
//	internal = new ftp_client_internal(client_ctrlfd);
//	internal->ctrlfile = new ftp_client_ctrlfile(client_ctrlfd);
//	internal->ctrlfd = client_ctrlfd;
//	internal->data_in_read_buf = 0;
//	internal->data_in_write_buf = 0;
}


ftp_client::~ftp_client()
{
	if(ctrlfile != NULL){
		
//	if(internal->ctrlfd != -1){
//		::close(internal->ctrlfd);
//	}
//	delete internal;
}

int ftp_client::close()
{
	return ::close(internal->ctrlfd);
}

int ftp_client::response(response_code, const char *message)
{
	internal->response(response_code, message);
	return 0;
}

int ftp_client::response_formart(response_code code, const char *format, ...)
{
	char message[MAX_RESPONSE_LENGTH];
	va_list ap;
	va_start(ap, format);
	vsprintf(message, sizeof(message_buf), format, ap);
	va_end(ap);	
	return response(code, message)
}

int ftp_client::do_response()
{
	return internal->do_response();
}

int ftp_client::wait_request(request *r)
{
	return internal->wait_request(r);
}

ftp_client_internal::ftp_client_internal(
int ftp_client_internal::wait_request(request *r)
{
	if(ctrlfd == -1)
		return -1;

	while(true){
		int read_count = read(ctrlfd, read_buf+data_in_read_buf,
				sizeof(read_buf) -1 -data_in_read_buf);
		data_in_read_buf += read_count;
		if( parse_request(r) != NOT_COMPLETE){
			continue;
		}
	}
	return 0;
}

int ftp_client_internal::parse_request(request *r)
{
	int used_data_count =  r->parse_from_commandline(read_buf, data_in_read_buf);
	if(used_data_count == 0){
		return NOT_COMPLETE;
	}
	else if(used_data_count > data_in_read_buf){
		// need log error
		return -1;
	}
	data_in_read_buf -= used_data_count;
	if(data_in_read_buf != 0){
		memmove(read_buf, read_buf+used_data_count, data_in_read_buf);
	}
	return 0;
}

int ftp_client_internal::response(response_code code, const char *message)
{
	
	return 0;
}

int ftp_client_internal::do_response()
{
	return 0;
}


