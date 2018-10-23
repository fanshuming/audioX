#include <stdio.h>
#include <string.h>
#include <assert.h>

#if defined(_WIN32) && !defined(__CYGWIN__)
#include <windows.h>
#else
#include <sys/select.h>
#endif



#include "../include/led.h"
#include "../include/timer.h"

#include <time.h>
#include <signal.h>
#include "led.h"
#include "sem.h"
#include "timer.h"



#include "../include/curl_down.h"
#include "../include/log.h"


#include<unistd.h>

unsigned int ledOn = 0;

unsigned int doneLoad = 0;

pthread_t alarm_handle_upgrade_id;

void * alarm_handle_upgrade()
{
	for(;;){
	//printf("Led :%d\n", ledOn);
		usleep(200000);
        	if(ledOn == 0)
        	{
                	led_on();
			ledOn = 1;
        	}else
        	{
                	led_off();
			ledOn = 0;
        	}

		if (doneLoad)
		{
			printf("img file have been done!\n");
			led_on();
			return;
		}
	}
}

void main()
{
    
	char * binUrl;

    // upgrate img from ota

    //if(system("ping -c 1 -w 2 120.27.138.117") == 0)
    if(system("ping -c 1 -w 2 dongle.topqizhi.com") == 0)
    {

        get_version();

        if((access("/tmp/version.json", F_OK)) != -1)
        {
                binUrl = (char *) parse_json("/tmp/version.json");
                if(binUrl != NULL)
                {

			pthread_create(&alarm_handle_upgrade_id, NULL, alarm_handle_upgrade, NULL);
		
                        LOGD("new version found, download ...\n");
			system("kill -9 $(pidof audioD)");
                        url2file(binUrl, "/tmp/upgrade.bin");
                        
                        LOGD("upgrade ...\n");
               		doneLoad = 1;         
			led_on();

                        system("sysupgrade  /tmp/upgrade.bin");
                }else{
			sleep(5);
			led_off();
                        LOGD("current version is the latest.\n");
                }
        }else{  
		sleep(3);
                led_off();
                LOGD("the file /tmp/version.json don't exist!\n");
         }

    }else{
	//sleep(2);
	led_off();
        LOGD("Don't check the version, net is not connected!\n");
    }
	pthread_join(alarm_handle_upgrade_id, NULL);
}
