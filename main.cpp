#include "global.h"
#include "ftpd.h"
#include "ftp_config.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

static void sig_child(int);

int main()
{
	ftpd d;
	ftp_config conf;
	d.init(&conf);
	signal(SIGCHLD, sig_child);
	return d.serve();
}


static void sig_child(int)
{
	while(true){
		int exit_val = 0;
		pid_t ret_val = waitpid(-1, &exit_val, WNOHANG);
		if(ret_val <= 0){
			break;
		}
		DEBUG("child process exit: %d\n", exit_val);
	}
}

