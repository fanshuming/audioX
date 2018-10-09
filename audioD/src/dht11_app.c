#include <stdio.h>
#include <curses.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "tty_com.h"

int dhtFd;

int dht11_init()
{
	       //打开温度传感器驱动模块
        dhtFd = open("/dev/dht11", O_RDWR | O_NONBLOCK);
        if (dhtFd < 0)
        {
                printf("can't open /dev/dht11\n");
                return -1;
        }
	
	return 0;

}

int get_temp_humi(unsigned int * T, unsigned int * H)
{
	unsigned int dht11 = 0;

	read(dhtFd, &dht11, sizeof(dht11));

        *T = dht11>>8;
        *H = dht11 &0x000000ff;
        printf("the current temperature is: %d\n",*T);
        printf("the current humidity is:    %d\n",*H);


        close(dhtFd);
	return 0;
}


void * dht11_loop()
{
	int fd;
	unsigned int dht11 = 0;
	unsigned int humi,temp;

	//打开温度传感器驱动模块
	fd = open("/dev/dht11", O_RDWR | O_NONBLOCK);
	if (fd < 0)
	{
		printf("can't open /dev/dht11\n");
		return;
	}
	while(1){
		read(fd, &dht11, sizeof(dht11));

		temp = dht11>>8;
		humi = dht11 &0x000000ff;
		printf("\nthe current temperature is: \n %d\n",temp);
		printf("the current humidity is:    \n %d\n",humi);

		//sleep(100);
		sleep(3);

	}
	close(fd);
	
	return 0;
}

