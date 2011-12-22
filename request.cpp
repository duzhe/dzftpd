#include "global.h"
#include "request.h"
#include "stdlib.h"
#include <map>
#include <string>
#include <string.h>
#include <locale>

static inline bool legal_command_length(int length)
{
	if(length == 3 || length == 4){
		return true;
	}
	return false;
}

char * strnstr(const char *s, const char *find, size_t slen)
{
	char c, sc;
	size_t len;

	if ((c = *find++) != '\0') {
		len = strlen(find);
		do {
			do {
				if (slen-- < 1 || (sc = *s++) == '\0')
					return (NULL);
			} while (sc != c);
			if (len > slen)
				return (NULL);
		} while (strncmp(s, find, len) != 0);
		s--;
	}
	return ((char *)s);
}

size_t request::parse_from_commandline(const char *commandline, size_t data_length)
{
	const char *commandline_end = strnstr(commandline, "\r\n", data_length);
	if(commandline_end == NULL){
		return 0;
	}
	const char *command_end = commandline;
	for(; *command_end != ' ' && *command_end != '\r'; ++command_end); // do nothing in loop
	int command_length = command_end - commandline;
	if(!legal_command_length(command_length ) ){
		return -1;
	}
	
	// copy command
//	memcpy(command, commandline, command_length );
//	command[command_length] = '\0';
	for(int i=0; i< command_length; ++i){
		if( !isalpha( commandline[i] ) ){
			return -1;
		}
		command[i] = toupper(commandline[i] );
	}
	command[command_length] = '\0';
		

	// copy param
	if( *command_end == ' '){
		int params_length = commandline_end -(command_end+1);
		params = (char *)malloc(params_length +1);
		if(params == NULL){
			return -1;
		}
		memcpy(params, command_end+1, params_length);
		params[params_length] = '\0' ;
	}
	return (size_t)(commandline_end +2 - commandline);// 2 is the length of "\r\n"
}

