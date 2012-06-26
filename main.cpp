#include "global.h"
#include "ftpd.h"
#include "log_unix.h"
#include "ftp_config.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>

static void do_daemonize(const char*);
static void sig_child(int);
log_unix * g_logger;
fs_unix * g_fs;

int main()
{
	ftpd d;
	ftp_config conf;
	log_unix logger;
	fs_unix fs;
	g_logger = & logger;
	g_fs = &fs;
	d.init(&conf);
	do_daemonize(PROGRAME_NAME);
	signal(SIGCHLD, sig_child);
	return d.serve();
}

static void err_quit(const char *format, ...)
{
	va_list va;
	va_start(va, format);
	fprintf(stderr, format , va);
	va_end(va);
	exit(-1);
}

// modified from apue.2e 13.1
static void do_daemonize(const char *cmd)
{
//	int					i, fd0, fd1, fd2;
	int					fd0, fd1, fd2;
	pid_t				pid;
//	struct rlimit		rl;
	struct sigaction	sa;

	/*
	 * Clear file creation mask.
	 */
	umask(0);

	/*
	 * Get maximum number of file descriptors.
	 */
//	if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
//		err_quit("%s: can't get file limit", cmd);

	/*
	 * Become a session leader to lose controlling TTY.
	 */
	if ((pid = fork()) < 0)
		err_quit("%s: can't fork", cmd);
	else if (pid != 0) /* parent */
		exit(0);
	setsid();

	/*
	 * Ensure future opens won't allocate controlling TTYs.
	 */
	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(SIGHUP, &sa, NULL) < 0)
		err_quit("%s: can't ignore SIGHUP");
	if ((pid = fork()) < 0)
		err_quit("%s: can't fork", cmd);
	else if (pid != 0) /* parent */
		exit(0);

	/*
	 * Change the current working directory to the root so
	 * we won't prevent file systems from being unmounted.
	 */
	if (chdir("/") < 0)
		err_quit("%s: can't change directory to /");

	/*
	 * Close all open file descriptors.
	 */
//	if (rl.rlim_max == RLIM_INFINITY)
//		rl.rlim_max = 1024;
//	for (i = 0; i < rl.rlim_max; i++)
//		close(i);
	/* process it easy */
	close(0);
	close(1);
	close(2);

	/*
	 * Attach file descriptors 0, 1, and 2 to /dev/null.
	 */
	fd0 = open("/dev/null", O_RDWR);
	fd1 = dup(0);
	fd2 = dup(0);

	/*
	 * Initialize the log file.
	 */
//	openlog(cmd, LOG_CONS, LOG_DAEMON);
//	if (fd0 != 0 || fd1 != 1 || fd2 != 2) {
//		syslog(LOG_ERR, "unexpected file descriptors %d %d %d",
//		  fd0, fd1, fd2);
//		exit(1);
//	}
}


static void sig_child(int)
{
	while(true){
		int exit_val = 0;
		pid_t ret_val = waitpid(-1, &exit_val, WNOHANG);
		if(ret_val <= 0){
			break;
		}
		DEBUG("child process exit: %d", exit_val);
	}
}

