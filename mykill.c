#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

int main(int argc, char *argv[])
{
    if (argc <= 3)
    {
        fprintf(stderr, "No signal info specified\n");
        return EXIT_FAILURE;
    }
    pid_t pid = atoi(argv[1]);
	int sig = atoi(argv[2]);
	union sigval value;
	value.sival_int = atoi(argv[3]);
    if (sigqueue(pid, sig, value) != 0)
    {
        perror("Couldn't send signal");
        return EXIT_FAILURE;
    }
    printf("Signal %d for PID %d with data %d sent successfully\n", sig, pid, value.sival_int);
	return EXIT_SUCCESS;
}
