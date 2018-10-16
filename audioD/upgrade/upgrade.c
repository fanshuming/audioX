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

#include "../include/curl_down.h"


#include<unistd.h>

void main()
{
    
	char * binUrl;

    // upgrate img from ota

    if(system("ping -c 1 -w 2 dongle.topqizhi.com") == 0)
    {

        get_version();

        if((access("/tmp/version.json", F_OK)) != -1)
        {
                binUrl = (char *) parse_json("/tmp/version.json");
                if(binUrl != NULL)
                {
                        LOGD("new version found, download ...\n");
                        url2file(binUrl, "/tmp/upgrade.bin");
                        
                        LOGD("upgrade ...\n");
                        led_off();
                        
                        system("sysupgrade  /tmp/upgrade.bin");
                }else{
                        LOGD("current version is the latest.\n");
                }
        }else{  
                led_on();
                LOGD("the file /tmp/version.json don't exist!\n");
         }

    }else{
        LOGD("Don't check the version, net is not connected!\n");
    }
}
