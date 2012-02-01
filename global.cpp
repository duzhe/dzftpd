#include "global.h"
#include <fcntl.h>
#include <unistd.h>

static const char *get_parent_path(const char *filename)
{
	static char parent_path[MAX_PATH];
//	char *p1 = parent_path;
//	const char *p2 = filename;
//	const char *slash = filename;
//	while(*slash != '\0'){
//		while(*slash != '/' && *slash!= '\0'){
//			slash++;
//		}
//		while(p2 < slash){
//			*p1++ = *p2++;
//		}
//		slash++;
//	}
	char *p1 = parent_path;
	const char *p2 = filename;
	for(; *p2 != '\0';){
		*p1++ = *p2++;
	}
	while(*p1 != '/' ){
		--p1;
	}
	*p1 = '\0';
	return parent_path;
}

bool test_access(const char *filename, char item)
{
	int mode;
	switch(item){
		case 'r':
			mode = R_OK;
			break;
		case 'w':
			mode = W_OK;
			break;
		case 'x':
			mode = X_OK;
			break;
		case 'f':
			mode = F_OK;
			break;
		default:
			return false;
	}
	int ret_val = access(filename, mode);
	if(ret_val == 0){
		DEBUG("test_access OK: %s %c \n", filename, item);
		return true;
	}
	else if(item != 'w'){
		DEBUG("test_access Fail: %s %c \n", filename, item);
		return false;
	}
	const char *parent_path = get_parent_path(filename);
	return test_access(parent_path, 'w');
}
