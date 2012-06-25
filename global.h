#include <stddef.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
typedef int command_id_t;
//struct request_t
//{
//	command_id_t 		command_id;
//	std::string 		request_param;
//	process_function_t 	process_function;
//};

//struct request_t;
//typedef int (*process_function_t)(const request_t* r) ;
//typedef struct registered_request_entry_d
//{
//	const char				*command_string;
//	int						command_string_length;
//	command_id_t 			command_id;
//	process_function_t 		process_function;
//}registered_request_entry_t;


//extern registered_request_entry_t registered_requests[];
#define NOT_COMPLETE -1


#define FTP_PORT 10021
#define LISTENQ 500
#define BUF_SIZE 512
#define MAX_PATH			4096
#define MAX_REQUEST_LENGTH	MAX_PATH+16
#define MAX_RESPONSE_LENGTH MAX_PATH+16

#define BANNER "--- Welcome to use dzftpd ---"
#define PROGRAME_NAME "dzftpd"

#define USER 0

#define DZ_OK		0
#define DZ_AGAIN	1
#define NOT_COMPLETE -1


typedef unsigned int response_code_t;

#include <stdio.h>
#define DEBUG(...) g_logger->log(LOG_DEBUG, __VA_ARGS__)

#include "classes.h"
#include "log_unix.h"
#include "fs_unix.h"

//bool test_access(const char *filename, char item);

class log_unix;
extern log_unix * g_logger;

class fs_unix;
extern fs_unix * g_fs;
