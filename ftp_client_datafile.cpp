#include "global.h"
#include "ftp_client_datafile.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <strings.h> /* for bzero */


unsigned short ftp_client_datafile::random_bind()
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

	socklen_t addrsize = sizeof(servaddr);

	ret_val = getsockname(listenfd, (struct sockaddr *)&servaddr, &addrsize);
	if(ret_val == -1){
		return 0;
	}

	DEBUG("Before random_bind return\n");
	return (servaddr.sin_port);
}
