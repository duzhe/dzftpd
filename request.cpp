#include "global.h"
#include "request.h"
#include "stdlib.h"
#include <map>
#include <string>
#include <string.h>
#include <locale>


static inline bool legal_command_length(int length)
{
	//DEBUG("Command length:%d\n", length);
	if(length == 3 || length == 4){
		return true;
	}
	return false;
}

//char * strnstr(const char *s, const char *find, size_t slen)
//{
//	char c, sc;
//	size_t len;
//
//	if ((c = *find++) != '\0') {
//		len = strlen(find);
//		do {
//			do {
//				if (slen-- < 1 || (sc = *s++) == '\0')
//					return (NULL);
//			} while (sc != c);
//			if (len > slen)
//				return (NULL);
//		} while (strncmp(s, find, len) != 0);
//		s--;
//	}
//	return ((char *)s);
//}


int request::parse_from_line(const char *line)
{
	if(line == NULL){
		DEBUG("Error: parse_from_lien :line is NULL\n");
		return -1;
	}
	if(params != NULL){
		free(params);
		params = NULL;
	}
//	const char *line_end = strnstr(line, "\r\n", data_length);
//	if(line_end == NULL){
//		return 0;
//	}
	const char *command_end = line;
	for(; *command_end != ' ' && *command_end != '\0'; ++command_end)
		; 
	int command_length = command_end - line;
	if(!legal_command_length(command_length ) ){
		DEBUG("Error: parse_from_line: legal_command_length\n");
		return -1;
	}
	
	// copy command
	for(int i=0; i< command_length; ++i){
		if( !isalpha( line[i] ) ){
			return -1;
		}
		command[i] = toupper(line[i] );
	}
	command[command_length] = '\0';
		

	// copy param
	const char *param_begin = command_end;
	while( isspace(*param_begin) ){
		param_begin++;
	}
	if( *param_begin != '\0'){
		int params_length = strlen(param_begin);
		params = (char *)malloc(params_length +1);
		if(params == NULL){
			return -1;
		}
		memcpy(params, param_begin, params_length+1); // include the '\0'
	}
	return 0;
}
