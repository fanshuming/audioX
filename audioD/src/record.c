#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alsa/asoundlib.h>
#include <errno.h>
#include <config.h>
#include <unistd.h>

#include "prim_type.h"
#include "ad.h"

#include "xfm10213_i2c.h"

#include "continuous.h"
#include "sem.h"
#include "microphone.h"

#define AUDIO_FORMAT SND_PCM_SFMT_S16_LE        /* 16-bit signed, little endian */
#define INPUT_GAIN 85
#define SPS_EPSILON 200

//int16 data_buf[4096]={0};
int16 data_buf[2048]={0};
//int16 data_buf1[2048]={0};
//char data_buf[4096]={0};

static int setparams(snd_pcm_t * handle)
{
	snd_pcm_hw_params_t *hwparams;
    	unsigned int out_sps, buffer_time, period_time;
    	int err;
    	int dir = 0;
	int val = 0;

    	snd_pcm_hw_params_alloca(&hwparams);
    	err = snd_pcm_hw_params_any(handle, hwparams);
    	if (err < 0) {
        	fprintf(stderr, "Can not configure this PCM device: %s\n",snd_strerror(err));
        	return -1;
    	}

    	err = snd_pcm_hw_params_set_access(handle, hwparams,SND_PCM_ACCESS_RW_INTERLEAVED);
    	if (err < 0) {
        	fprintf(stderr,"Failed to set PCM device to interleaved: %s\n",snd_strerror(err));
        	return -1;
    	}

    	err = snd_pcm_hw_params_set_format(handle, hwparams, SND_PCM_FORMAT_S16_LE);
    	if (err < 0) {
        	fprintf(stderr,"Failed to set PCM device to 16-bit signed PCM: %s\n",snd_strerror(err));
        	return -1;
    	}

    	err = snd_pcm_hw_params_set_channels(handle, hwparams, 1);
    	//err = snd_pcm_hw_params_set_channels(handle, hwparams, 2);
    	if (err < 0) {
        	fprintf(stderr, "Failed to set PCM device to mono: %s\n",snd_strerror(err));
        	return -1;
    	}
	//snd_pcm_hw_params_get_channels(hwparams, &val);
        //printf("=====================================================channels:%d\n", val);

    	out_sps = 16000;
    	err = snd_pcm_hw_params_set_rate_near(handle, hwparams, &out_sps, &dir);
    	if (err < 0) {
        	fprintf(stderr, "Failed to set sampling rate: %s\n",snd_strerror(err));
        	return -1;
    	}

    	/* Set buffer time to the maximum. */

    	err = snd_pcm_hw_params_get_buffer_time_max(hwparams, &buffer_time, 0);
    	printf("buffer time:%d\n",buffer_time);
    
    	period_time = buffer_time / 4;
    	err = snd_pcm_hw_params_set_period_time_near(handle, hwparams,
                                                 &period_time, 0);
    	if (err < 0) {
        	fprintf(stderr, "Failed to set period time to %u: %s\n",period_time, snd_strerror(err));
       		return -1;
   	 }
    	//buffer_time = buffer_time *4;
    	err = snd_pcm_hw_params_set_buffer_time_near(handle, hwparams,&buffer_time, 0);
    	if (err < 0) {
        	fprintf(stderr, "Failed to set buffer time to %u: %s\n",buffer_time, snd_strerror(err));
        	return -1;
    	}
    	err = snd_pcm_hw_params_get_buffer_time_max(hwparams, &buffer_time, 0);
    	printf("buffer time:%d\n",buffer_time);

    	//snd_pcm_uframes_t frames = 80;
    	snd_pcm_uframes_t frames = 32;
    	//snd_pcm_uframes_t frames = 320;
    	snd_pcm_hw_params_set_period_size_near(handle, hwparams, &frames, &dir);

    	err = snd_pcm_hw_params(handle, hwparams);
	if (err < 0) {
        	fprintf(stderr, "Failed to set hwparams: %s\n", snd_strerror(err));
        	return -1;
    	}

    	snd_pcm_hw_params_get_period_size(hwparams, &frames, NULL);
    	printf("-----------------frames:%d\n",frames);

	//set i2c to enablle ifly mic
	//upgrage_pak();

    	xfm_i2c();

	//set_mic_enable(true);
	err = snd_pcm_prepare(handle);
    	if (err < 0) {
        	fprintf(stderr, "snd_pcm_prepare failed: %s\n", snd_strerror(err));
        	return -1;
    	}   
    	err = snd_pcm_start(handle);
    	if (err < 0) {
        	fprintf(stderr, "snd_pcm_start failed: %s\n", snd_strerror(err));
		return -1;
    	}   


    err = snd_pcm_nonblock(handle, 1);
    if (err < 0) {
        fprintf(stderr, "Failed to set non-blocking mode: %s\n",
                snd_strerror(err));
        return -1;
    }

    	return 0;
}


void * record_from_dev(void * ringb)
{

	int16 buf[2048];
	int16 testbuf[2048];

	//int16 buf[4096];
        int32 length;
	snd_pcm_t * recHandle;
	int err;
	int ret = 0;

//	FILE *incaptureFp = fopen("/tmp/incapture.pcm","wb");
//	FILE *incaptureFp1 = fopen("/tmp/incapture1.pcm","wb");

	struct ringbuffer *ring_buf = (struct ringbuffer *)ringb;

	err = snd_pcm_open(&recHandle, "plughw:0,0", SND_PCM_STREAM_CAPTURE, 0);
	if(err < 0)
	{
		fprintf(stderr,"Error opening audio device plughw:0,0 for capture: %s\n", snd_strerror(err));
        	return NULL;
	}

	setparams(recHandle);

	for(;;){

		if(!get_mic_status())	
		{
			snd_pcm_readi(recHandle, testbuf, 2048);
			ringbuffer_reset(ring_buf);
			continue;
		}

		//length = snd_pcm_readi(recHandle, buf, 2048);
		length = snd_pcm_readi(recHandle, buf, 2048);
                //printf("%d data has been read from dev\n",length);

    		if (length == -EAGAIN) {
        		length = 0;
    		}else if (length == -EPIPE) 
		{
        		fprintf(stderr, "Input overrun, read calls are too rare (non-fatal)\n");
        		err = snd_pcm_prepare(recHandle);
        		if (err < 0) {
                		fprintf(stderr, "Can't recover from underrun: %s\n",snd_strerror(err));
                		return ;//AD_ERR_GEN;
        		}
        		length = 0;
    		}else if (length == -ESTRPIPE) 
		{
        		fprintf(stderr, "Resuming sound driver (non-fatal)\n");
        		while ((err = snd_pcm_resume(recHandle)) == -EAGAIN)
                		usleep(10000); /* Wait for the driver to wake up */
        		if (err < 0) {
                		err = snd_pcm_prepare(recHandle);
                		if (err < 0) {
                        		fprintf(stderr, "Can't recover from underrun: %s\n",snd_strerror(err));
                        		return;//AD_ERR_GEN;
                		}	
        		}
        		length = 0;
    		}else if (length < 0) 
		{
        		fprintf(stderr, "Audio read error: %s\n",snd_strerror(length));
        		return ;//AD_ERR_GEN;
    		}

		//add record debug

	    int16 *p = buf;
    	    int i = 0;
    	    int j = 0;
	    //int k = 0;

           for(i = 0;i<length*1*1;)
           {
                        data_buf[j++]=*(p++);
                        i+=1;
                        p+=1;
			//data_buf1[k++] = *(p);
    	   }

//    	   fwrite(data_buf, 1, j, incaptureFp);
//    	   fwrite(data_buf1, 1, k, incaptureFp1);

           	if (ringbuffer_is_full(ring_buf)) {
                        printf("buffer is full !\n");
                        ringbuffer_reset(ring_buf);
                        continue;
                }
	   //printf("send length :%d data to ring buffer\n",length);
                ringbuffer_put(ring_buf, data_buf, 2048);
	   //	sleep(0.5);
	}
}

