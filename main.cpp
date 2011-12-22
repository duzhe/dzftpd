#include "ftpd.h"

int main()
{
	ftpd d;
	d.init();
	return d.serve();
}

