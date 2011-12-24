#include "global.h"
#include "ftp_client_ctrlfile.h"
#include <unistd.h>  //for write and read
#include <string.h> /* for memmove */
#include <stdio.h>
#include <stdarg.h> /* for va_list */

ftp_client_ctrlfile::ftp_client_ctrlfile(int client_ctrlfd)
{
	fd = client_ctrlfd;
	write_buf_left = sizeof(write_buf);
	write_buf_pos = write_buf;
}

ftp_client_ctrlfile::~ftp_client_ctrlfile()
{
	close();
}

int ftp_client_ctrlfile::close()
{
	if(fd != -1){
		::close(fd);
		fd = -1;
	}
}

int ftp_client_ctrlfile::puts(const char *message)
{
	return 0;
}

int ftp_client_ctrlfile::printf(const char *format, ...)
{
	char buf[MAX_RESPONSE_LENGTH];

	va_list va;
	va_start(va, format);
	int length = vsnprintf(buf, sizeof(buf), format, va);
	va_end(va);

	if(length < 0){
		return -1;
	}
	if(length > write_buf_left){
		flush();
	}
	if(length > write_buf_left){
		return -1;
	}
	memcpy(write_buf_pos, buf, length);
	write_buf_pos += length;
	write_buf_left -= length;
	return 0;
}

int ftp_client_ctrlfile::flush()
{
	int return_code = ::write(fd, write_buf, write_buf_pos - write_buf);
	if(return_code == -1){
		return  -1;
	}
	write_buf_pos = write_buf;
	write_buf_left = sizeof(write_buf);
	return 0;
}

int ftp_client_ctrlfile::readline(char *linebuf)
{
	char *line_end = read_buf;
	while(true){
		while(line_end < read_buf_pos-1){
			if(line_end[0] == '\r' && line_end[1] == '\n'){
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
		}
		read();
	}
	return 0;// will never run to here;
}

int ftp_client_ctrlfile::read()
{
	int read_count = ::read(fd, read_buf_pos, read_buf_left);
	if(read_count == -1){
		return -1;
	}
	read_buf_pos += read_count;
	read_buf_left -= read_count;
	return 0;
}

