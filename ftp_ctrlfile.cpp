#include "global.h"
#include "ftp_ctrlfile.h"
#include <unistd.h>  //for write and read
#include <sys/socket.h> /* for setsockopt */
#include <sys/types.h>  /* for setsockopt */
#include <netinet/in.h> 
#include <netinet/tcp.h> 
#include <string.h> /* for memmove */
#include <stdio.h>
#include <stdarg.h> /* for va_list */

ftp_ctrlfile::ftp_ctrlfile(int client_ctrlfd)
{
	fd = client_ctrlfd;

	write_buf_left = sizeof(write_buf);
	write_buf_pos = write_buf;

	read_buf_left = sizeof(read_buf);
	read_buf_pos = read_buf;

	set_nodelay();

	addr = 0;
}

ftp_ctrlfile::~ftp_ctrlfile()
{
	close();
}

void ftp_ctrlfile::set_nodelay()
{
	int flag = 1;
	int ret = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(flag) );
	if( ret == -1){
		DEBUG("set_nodelay: setsockopt error\n");
	}
}


int ftp_ctrlfile::close()
{
	if(fd != -1){
		::close(fd);
		fd = -1;
	}
	return 0;
}

int ftp_ctrlfile::puts(const char *message)
{
	return 0;
}

int ftp_ctrlfile::printf(const char *format, ...)
{
	char buf[MAX_RESPONSE_LENGTH];

	va_list va;
	va_start(va, format);
	int length = vsnprintf(buf, sizeof(buf), format, va);
	va_end(va);

	DEBUG("Client printf:%s\n", buf);
	if(length < 0){
		return -1;
	}
	if((size_t)length > write_buf_left){
		flush();
	}
	if((size_t)length > write_buf_left){
		return -1;
	}
	memcpy(write_buf_pos, buf, length);
	write_buf_pos += length;
	write_buf_left -= length;
	return 0;
}

int ftp_ctrlfile::flush()
{
//	DEBUG("client flush\n");
again:
	int write_count = ::write(fd, write_buf, write_buf_pos - write_buf);
	if(write_count == -1){
		switch(errno){
			case EINTR:
				goto again;
			default:
				return -1;
		}
	}
	write_buf_pos -= write_count;
	write_buf_left += write_count;
	return 0;
}

int ftp_ctrlfile::readline(char *linebuf)
{
	char *line_end = read_buf;
	for(;;){
		for(;line_end < read_buf_pos-1;){
			if(line_end[0] != '\r' || line_end[1] != '\n'){
				++line_end;
				continue;
			}

			// right trim
			while(*(line_end-1) == ' '){
				--line_end;
			}
			// copy line to buf
			int line_length = line_end - read_buf;
			memcpy(linebuf, read_buf, line_length);
			linebuf[line_length] = '\0';
			// move rest bytes to front
			char *rest_begin = line_end;
			while(*(rest_begin++) != '\n')
				;
			int rest_byte_count = read_buf_pos - rest_begin;
			if(rest_byte_count != 0){
				memmove(read_buf, rest_begin, rest_byte_count);
			}
			read_buf_pos  = read_buf + rest_byte_count;
			read_buf_left = sizeof(read_buf) - rest_byte_count;
			
			return 0;
		}
		int ret_val = read();
		if(ret_val != 0){
			return ret_val;
		}
	}
#ifdef DEBUG
	this->printf("%s\r\n",linebuf);
#endif
	return 0;// will never run to here;
}

int ftp_ctrlfile::read()
{
	do{
		ssize_t read_count = ::read(fd, read_buf_pos, read_buf_left);
		if(read_count == 0){
			return  -1;
		}
		if(read_count == -1){
			switch(errno){
				case EINTR:
					continue;
				default:
					break;
			}
			return -1;
		}
		read_buf_pos += read_count;
		read_buf_left -= read_count;
		return 0;
	}
	while(1);
}

in_addr_t ftp_ctrlfile::get_host_addr()const
{
	if(addr == 0){
		struct sockaddr_in addrin;
		bzero(&addrin, sizeof(addrin) );
		addrin.sin_family = AF_INET;

		socklen_t addrsize = sizeof(addrin);
		int ret_val = getsockname(fd, (struct sockaddr *)&addrin, &addrsize);
		if(ret_val == -1){
			DEBUG("getsockname error\n");
			return 0;
		}
		addr = addrin.sin_addr.s_addr;
	}
	return addr;
}
