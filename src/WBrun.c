#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>

void init_daemon(void) {
	int pid;
	if(pid = fork()) {
		exit(0);
	} else if(pid < 0) {
		exit(1);
	} 

	setsid();
	if(pid = fork()) {
		exit(0);
	} else if(pid < 0) {
		exit(1);
	}
	system("./WB");
	for(int i = 0; i < NOFILE; ++i) {
		close(i);
	}
	chdir("/");
	umask(0);
	return;
}

int main() {
	init_daemon();
	return 0;
}
