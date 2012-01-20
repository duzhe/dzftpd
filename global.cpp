#include "global.h"

union ip4_addr
{
	unsigned char addr_byte[4];
	int 		  addr_int;
};

//int get_serve_addr()
//{
//	ip4_addr addr;
//	addr.addr_byte[0] = 192;
//	addr.addr_byte[1] = 168;
//	addr.addr_byte[2] = 1;
//	addr.addr_byte[3] = 80;
//	return addr.addr_int;
//}

const char *get_serve_addr()
{
//	return "192,168,1,80";
	return "192,168,35,131";
}
