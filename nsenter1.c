#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

extern char **environ;

static void enter_ns(pid_t pid);
static pid_t dockerd_pid();

int main(int argc, char **argv) {
	char *shell = "/bin/sh";
	char *def[] = {shell, NULL};
	char *cmd = shell;
	char **args = def;
	
	enter_ns(1);
	enter_ns(dockerd_pid());

	if (argc > 1) {
		cmd = argv[1];
		args = argv + 1;
	}

	if (execve(cmd, args, environ) == -1) {
		perror("execve");
		exit(1);
	}
	exit(0);	
}

static void enter_ns(pid_t pid)
{
	char buffer[512];

	sprintf(buffer, "/proc/%d/ns/mnt", pid);
	int fdm = open(buffer, O_RDONLY);
	sprintf(buffer, "/proc/%d/ns/net", pid);
	int fdn = open(buffer, O_RDONLY);
	sprintf(buffer, "/proc/%d/ns/ipc", pid);
	int fdi = open(buffer, O_RDONLY);
	sprintf(buffer, "/proc/%d/root", pid);
	int froot = open(buffer, O_RDONLY);

	if (fdm == -1 || fdn == -1 || fdi == -1 || froot == -1) {
		fprintf(stderr, "Failed to open /proc/1 files, are you root?\n");
		exit(1);
	}

	if (setns(fdm, 0) == -1) {
		perror("setns");
		exit(1);
	}
	if (setns(fdn, 0) == -1) {
		perror("setns");
		exit(1);
	}
	if (setns(fdi, 0) == -1) {
		perror("setns");
		exit(1);
	}
	if (fchdir(froot) == -1) {
		perror("fchdir");
		exit(1);
	}
	if (chroot(".") == -1) {
		perror("chroot");
		exit(1);
	}
}

static pid_t dockerd_pid()
{
	const int LEN = 64;
	char buffer[LEN];

//	FILE *pidofcmd = popen("pidof dockerd", "r");
    FILE *pidofcmd = fopen("/var/run/docker.pid", "r");
	fgets(buffer, LEN, pidofcmd);
	pid_t pid = strtoul(buffer, NULL, 10);
//	pclose(pidofcmd);
	fclose(pidofcmd);
	
	return pid;
}