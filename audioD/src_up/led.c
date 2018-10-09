#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>


#define LED_ON 	0 //0 home//1 custom
#define LED_OFF 1

int ledFd;
static int led_status = 0;

int led_init()
{
	ledFd = open("/dev/gpio_drv" , O_RDWR|O_NONBLOCK);
        if(ledFd < 0)
        {
                printf("can't open /dev/gpio_drv\n");
                return -1;
        }
	return 0;
}

void led_on()
{
	ledFd = open("/dev/gpio_drv" , O_RDWR|O_NONBLOCK);
	ioctl(ledFd, LED_ON);
	close(ledFd);
	led_status = 1;

	return;	
}

void led_off()
{
	 ledFd = open("/dev/gpio_drv" , O_RDWR|O_NONBLOCK);
	 ioctl(ledFd, LED_OFF);
	 close(ledFd);
	 led_status = 0;

	 return;
}

int get_led_status()
{
	return led_status;
}


/*
void print_usage(char *file)
{
    printf("Usage:\n");
    printf("eg. \n");
    printf("%s led on\n", file);
    printf("%s led off\n", file);
}

int main(int argc , char** argv)
{
	int ledFd;

	if(argc != 3)
	{
		print_usage(argv[0]);
		return 0;
	}
	
	ledFd = open("/dev/gpio_drv" , O_RDWR|O_NONBLOCK);
	if(ledFd < 0)
	{
		printf("can't open /dev/gpio_drv\n");
		return -1;
	}

	if(!strcmp("led",argv[1]))
	{
		if(!strcmp("on", argv[2]))
		{
			ioctl(ledFd,MYLEDS_LED1_ON);
		}
		else if(!strcmp("off", argv[2]))
		{
			ioctl(ledFd,MYLEDS_LED1_OFF);
		}
		else
		{
			print_usage(argv[0]);
			return 0;
		}
	}
	else
	{
	 	print_usage(argv[0]);
		return 0;
	}
	return 0;
}
*/
