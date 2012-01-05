#include <stddef.h>
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
#define MAX_REQUEST_LENGTH 4096
#define MAX_RESPONSE_LENGTH 4096

#define BANNER "--- Welcome to use dzftp ---"

#define USER 0

#define DZ_OK		0
#define DZ_AGAIN	1
#define NOT_COMPLETE -1


typedef unsigned int response_code_t;

#include <stdio.h>
#define DEBUG ::printf

#include "classes.h"

const char *get_serve_addr();
