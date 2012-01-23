#include "global.h"
#include "ftp_datafile.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <strings.h> /* for bzero */

ftp_datafile::ftp_datafile()
{
	mode = NOSP;
	listenfd = -1;
	datafd = -1;
}

void ftp_datafile::reset()
{
	if(listenfd != -1){
		::close(listenfd);
	}
	if(datafd != -1){
		::close(datafd);
	}
	mode = NOSP;
}

unsigned short ftp_datafile::random_bind()
{
	if(mode != PASV){
		return 0;
	}
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if(listenfd == -1){
		return 0;
	}

	struct sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr) );
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(0);

	int ret_val = bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr) );
	if(ret_val == -1){
		return 0;
	}

	ret_val = listen(listenfd, 1);
	if(ret_val == -1){
		return 0;
	}

	socklen_t addrsize = sizeof(servaddr);

	ret_val = getsockname(listenfd, (struct sockaddr *)&servaddr, &addrsize);
	if(ret_val == -1){
		return 0;
	}

	DEBUG("Before random_bind return\n");
	return (servaddr.sin_port);
}

int ftp_datafile::write_file(const char *filename)
{
	if(datafd == -1){
		return NO_DATA_CONNECTION;
	}
	int file = open(filename, O_RDONLY);
	if(file == -1){
		return OPEN_FILE_ERROR;
	}
	char buf[1024];
	while(true){
		ssize_t ret_val = ::read(file, buf, 1024);
		if(ret_val == -1){
			close(file);
			close(datafd);
			datafd = -1;
			return READ_FILE_ERROR;
		}
		if(ret_val == 0){
			close(file);
			close(datafd);
			datafd = -1;
			return 0;
		}
		
		ret_val = ::write(datafd, buf, ret_val);
		if(ret_val == -1){
			close(file);
			close(datafd);
			datafd = -1;
			return CLIENT_CLOSE_DATA_CONNECTION;
		}
	}
}

void ftp_datafile::accept_connection()
{
	datafd = ::accept(listenfd, NULL, NULL);
	close(listenfd);
	DEBUG("data connected:datafd: %d\n", datafd);
}

ssize_t ftp_datafile::write(const void *buf, size_t count)
{
	//DEBUG
	::write(STDOUT_FILENO, buf, count);
	if(datafd == -1){
		return NO_DATA_CONNECTION;
	}
	do{
		ssize_t ret_val = ::write(datafd, buf, count);
		if(ret_val == -1){
			switch(errno){
				case EINTR:
					continue;
				default:
					return CLIENT_CLOSE_DATA_CONNECTION;
			}
		}
		return 0;
	}
	while(true);
	// never run to here;
	return 0;
}

