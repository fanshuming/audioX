#include <signal.h>
#include <stdio.h>
#include <unistd.h>
/*
int cnt = 0;

void sigalrm_fn(int sig)
{
    printf("alarm!\n");
    	if(cnt < 3)
	{
		cnt++;
    		alarm(1);
	}
    return;
}

int main(void)
{
    signal(SIGALRM, sigalrm_fn);
    alarm(1);

    while(1) pause();
}
*/
