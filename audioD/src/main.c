/* -*- c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* ====================================================================
 * Copyright (c) 1999-2010 Carnegie Mellon University.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * This work was supported in part by funding from the Defense Advanced 
 * Research Projects Agency and the National Science Foundation of the 
 * United States of America, and the CMU Sphinx Speech Consortium.
 *
 * THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND 
 * ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
 * NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ====================================================================
 *
 */
/*
 * continuous.c - Simple pocketsphinx command-line application to test
 *                both continuous listening/silence filtering from microphone
 *                and continuous file transcription.
 */

/*
 * This is a simple example of pocketsphinx application that uses continuous listening
 * with silence filtering to automatically segment a continuous stream of audio input
 * into utterances that are then decoded.
 * 
 * Remarks:
 *   - Each utterance is ended when a silence segment of at least 1 sec is recognized.
 *   - Single-threaded implementation for portability.
 *   - Uses audio library; can be replaced with an equivalent custom library.
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>

#if defined(_WIN32) && !defined(__CYGWIN__)
#include <windows.h>
#else
#include <sys/select.h>
#endif

#include <sphinxbase/err.h>
#include <sphinxbase/ad.h>

#include "pocketsphinx.h"

#include "xfm10213_i2c.h"
#include "ringbuffer.h"
#include "record.h"
#include "tty_com.h"

#include <signal.h>
#include "led.h"
#include "sub_client.h"
#include "microphone.h"
#include "sem.h"
#include "timer.h"
#include "dht11_app.h"

#include <time.h>

#include "curl_down.h"

#define   FIFO_SIZE     4194304

int blink_cnt = 0;

unsigned int blink_times = 0;

//int ttyfd;

static const arg_t cont_args_def[] = {
    POCKETSPHINX_OPTIONS,
    /* Argument file. */
    {"-argfile",
     ARG_STRING,
     NULL,
     "Argument file giving extra arguments."},
    {"-adcdev",
     ARG_STRING,
     NULL,
     "Name of audio device to use for input."},
    {"-infile",
     ARG_STRING,
     NULL,
     "Audio file to transcribe."},
    {"-inmic",
     ARG_BOOLEAN,
     "no",
     "Transcribe audio from microphone."},
    {"-time",
     ARG_BOOLEAN,
     "no",
     "Print word times in file transcription."},
    CMDLN_EMPTY_OPTION
};

static ps_decoder_t *ps;
static cmd_ln_t *config;
static FILE *rawfd;

static void
print_word_times()
{
    int frame_rate = cmd_ln_int32_r(config, "-frate");
    ps_seg_t *iter = ps_seg_iter(ps);
    while (iter != NULL) {
        int32 sf, ef, pprob;
        float conf;

        ps_seg_frames(iter, &sf, &ef);
        pprob = ps_seg_prob(iter, NULL, NULL, NULL);
        conf = logmath_exp(ps_get_logmath(ps), pprob);
        printf("%s %.3f %.3f %f\n", ps_seg_word(iter), ((float)sf / frame_rate),
               ((float) ef / frame_rate), conf);
        iter = ps_seg_next(iter);
    }
}

static int
check_wav_header(char *header, int expected_sr)
{
    int sr;

    if (header[34] != 0x10) {
        E_ERROR("Input audio file has [%d] bits per sample instead of 16\n", header[34]);
        return 0;
    }
    if (header[20] != 0x1) {
        E_ERROR("Input audio file has compression [%d] and not required PCM\n", header[20]);
        return 0;
    }
    if (header[22] != 0x1) {
        E_ERROR("Input audio file has [%d] channels, expected single channel mono\n", header[22]);
        return 0;
    }
    sr = ((header[24] & 0xFF) | ((header[25] & 0xFF) << 8) | ((header[26] & 0xFF) << 16) | ((header[27] & 0xFF) << 24));
    if (sr != expected_sr) {
        E_ERROR("Input audio file has sample rate [%d], but decoder expects [%d]\n", sr, expected_sr);
        return 0;
    }
    return 1;
}

/*
 * Continuous recognition from a file
 */
static void
recognize_from_file()
{
    int16 adbuf[2048];
    const char *fname;
    const char *hyp;
    int32 k;
    uint8 utt_started, in_speech;
    int32 print_times = cmd_ln_boolean_r(config, "-time");

    time_t timer;
    struct tm *tblock;

    fname = cmd_ln_str_r(config, "-infile");
    if ((rawfd = fopen(fname, "rb")) == NULL) {
        E_FATAL_SYSTEM("Failed to open file '%s' for reading",
                       fname);
    }
    
    if (strlen(fname) > 4 && strcmp(fname + strlen(fname) - 4, ".wav") == 0) {
        char waveheader[44];
	fread(waveheader, 1, 44, rawfd);
	if (!check_wav_header(waveheader, (int)cmd_ln_float32_r(config, "-samprate")))
    	    E_FATAL("Failed to process file '%s' due to format mismatch.\n", fname);
    }

    if (strlen(fname) > 4 && strcmp(fname + strlen(fname) - 4, ".mp3") == 0) {
	E_FATAL("Can not decode mp3 files, convert input file to WAV 16kHz 16-bit mono before decoding.\n");
    }
    
    ps_start_utt(ps);
    utt_started = FALSE;

    	    timer = time(NULL);
	    tblock = localtime(&timer);
            printf("start time is: %s\n", asctime(tblock));
    while ((k = fread(adbuf, sizeof(int16), 2048, rawfd)) > 0) {
        ps_process_raw(ps, adbuf, k, FALSE, FALSE);
        in_speech = ps_get_in_speech(ps);
        if (in_speech && !utt_started) {
            utt_started = TRUE;
        } 
        if (!in_speech && utt_started) {
            ps_end_utt(ps);
            hyp = ps_get_hyp(ps, NULL);
            if (hyp != NULL)
        	printf("%s\n", hyp);
    	    timer = time(NULL);
	    tblock = localtime(&timer);
            printf("end time is: %s\n", asctime(tblock));
            if (print_times)
        	print_word_times();
            fflush(stdout);

            ps_start_utt(ps);
            utt_started = FALSE;
        }
    }
    ps_end_utt(ps);
    if (utt_started) {
        hyp = ps_get_hyp(ps, NULL);
        if (hyp != NULL) {
    	    printf("%s\n", hyp);
    	    timer = time(NULL);
	    tblock = localtime(&timer);
            printf("end time is: %s\n", asctime(tblock));
    	    if (print_times) {
    		print_word_times();
	    }
	}
    }
    
    fclose(rawfd);
}

/* Sleep for specified msec */
static void
sleep_msec(int32 ms)
{
#if (defined(_WIN32) && !defined(GNUWINCE)) || defined(_WIN32_WCE)
    Sleep(ms);
#else
    /* ------------------- Unix ------------------ */
    struct timeval tmo;

    tmo.tv_sec = 0;
    tmo.tv_usec = ms * 1000;

    select(0, NULL, NULL, NULL, &tmo);
#endif
}

void sigalrm_led(int sig)
{
        if(blink_cnt < 4)
        {
                if((blink_cnt % 2) == 0)
                {
                        led_off();
                }else{
                        led_on();
                }
                blink_cnt++;
                alarm(1);
        }else{
                led_off();
                blink_cnt = 0;
        }
    return;
}

void alarm_handle_recognize_ok(int sig)
{
	//LOGD("200ms timeout\n");
	blink_times++;
	
	if((blink_times % 2) == 0)
	{
		led_off();
	}else
	{
		led_on();
	}	

	if(blink_times > 5)
	{
		led_off();
		unset_time();
	}
}

void alarm_handle_recognize_fail(int sig)
{
        printf("1s timeout\n");
        blink_times++;

        if((blink_times % 2) == 0)
        {
                led_off();
        }else
        {
                led_on();
        }

        if(blink_times > 5)
        {
                unset_time();
                led_off();
        }
}

/*
 * Main utterance processing loop:
 *     for (;;) {
 *        start utterance and wait for speech to process
 *        decoding till end-of-utterance silence will be detected
 *        print utterance result;
 *     }
 */
static void
recognize_from_microphone( struct ringbuffer * ringB)
{
    ad_rec_t *ad;
    int16 adbuf1[1024];
    int16 adbuf2[1024];
    //int16 adbuf2[1024];
    int16 adbuf[2048];
    uint8 utt_started, in_speech;
    uint32 k;
    int ret = 0;

    char const *hyp;

    //time_t timer;
    //struct tm *tblock;

    struct ringbuffer *ring_buf = ringB;


    if (ps_start_utt(ps) < 0)
        E_FATAL("Failed to start utterance\n");
    utt_started = FALSE;
    E_INFO("Ready....\n");

// add read data for debug
    //FILE *outcaptureFp = fopen("/tmp/outcapture.pcm","wb");

    for (;;) {

        if (ringbuffer_is_empty(ring_buf)) 
	{
        	//sleep_msec(100);
		usleep(10000);
		continue;
        }

//       k = ringbuffer_get(ring_buf, adbuf, 2048);

       k = ringbuffer_get(ring_buf, adbuf1, 2048);
       k = ringbuffer_get(ring_buf, adbuf2, 2048);
	
	int16 *p1= adbuf1;
	int16 *p2= adbuf2;
	int i=0;
	for(i = 0; i< 1024 ;i++){
		adbuf[i] = *p1++;
		adbuf[1024+i] = *p2++;
	}
	
	// add read data for debug
	//fwrite(adbuf, 2, k, outcaptureFp);

        ps_process_raw(ps, adbuf, k, FALSE, FALSE);
        //ps_process_raw(ps, adbuf, k, FALSE, TRUE);
        in_speech = ps_get_in_speech(ps);
        if (in_speech && !utt_started) {
            utt_started = TRUE;
/*		
    	    timer = time(NULL);
	    tblock = localtime(&timer);
            printf("start time is: %s\n", asctime(tblock));
*/
            E_INFO("Listening...\n");
        }
        if (!in_speech && utt_started) {
            ps_end_utt(ps);
            hyp = ps_get_hyp(ps, NULL );
            if (hyp != NULL) {
                printf("%s\n", hyp);
/*			
    		timer = time(NULL);
	    	tblock = localtime(&timer);
            	printf("end time is: %s\n", asctime(tblock));
*/
		blink_times = 0;
		unset_time();
	        signal(SIGALRM, alarm_handle_recognize_ok);

   		set_time(0,300000);

		memset(send_cmd_to_com, 0 , strlen(send_cmd_to_com));
		memcpy(send_cmd_to_com, hyp, strlen(hyp) / sizeof(char));
		send_data_to_com(ttyfd);
		
		set_mic_enable(false);

                fflush(stdout);
            }else{
		LOGD("voice command invalid\n");
		blink_times = 0;
                unset_time();
                signal(SIGALRM, alarm_handle_recognize_fail);

                set_time(1,0);
	    }
	    
	    ringbuffer_reset(ring_buf);

            if (ps_start_utt(ps) < 0)
                E_FATAL("Failed to start utterance\n");
            utt_started = FALSE;
            E_INFO("Ready....\n");
        }
	
        sleep_msec(100);

    }
	
    ad_close(ad);

}




int
main(int argc, char *argv[])
{
    char const *cfg;

    struct ringbuffer *ring_buf;
    pthread_t record_pid;
   // pthread_t send_data_to_com_thread_pid;
    pthread_t mosq_pid;
    pthread_t mic_wakeup_thread_id;
    pthread_t dht11_loop_id;

/*
    char * binUrl;

    // upgrate img from ota

    if(system("ping -c 1 -w 2 120.27.138.117") == 0)
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
			led_on();

    			system("sysupgrade  /tmp/upgrade.bin");
		}else{
			LOGD("current version is the latest.\n");
		}
    	}else{
		led_off();
		LOGD("the file /tmp/version.json don't exist!\n");
   	 }

    }else{
	LOGD("Don't check the version, net is not connected!\n");	
    }

*/

//delete offline recognize
//    config = cmd_ln_parse_r(NULL, cont_args_def, argc, argv, TRUE);
//    sem_init(&sem_mic_wakeup, 0, 0);

    led_off();
    sleep(1);
    led_on();

    sleep(1);
    led_off();
    sleep(1);
    led_on();
    sleep(1);
    led_off();

    ttyfd = UARTx_Open(ttyfd, "/dev/ttyS0");

    if(FALSE == ttyfd)
    {
    	exit(1);
    }


    // Handle argument file as -argfile. 
//delete offline recognize
/*
    if (config && (cfg = cmd_ln_str_r(config, "-argfile")) != NULL) {
        config = cmd_ln_parse_file_r(config, cont_args_def, cfg, FALSE);
    }

    if (config == NULL || (cmd_ln_str_r(config, "-infile") == NULL && cmd_ln_boolean_r(config, "-inmic") == FALSE)) {
	E_INFO("Specify '-infile <file.wav>' to recognize from file or '-inmic yes' to recognize from microphone.\n");
        cmd_ln_free_r(config);
	return 1;
    }

    ps_default_search_args(config);
    ps = ps_init(config);
    if (ps == NULL) {
        cmd_ln_free_r(config);
        return 1;
    }


    E_INFO("%s COMPILED ON: %s, AT: %s\n\n", argv[0], __DATE__, __TIME__);

    if (cmd_ln_str_r(config, "-infile") != NULL) {
        recognize_from_file();
    } else if (cmd_ln_boolean_r(config, "-inmic")) {
	// record data from dev
        ring_buf = ringbuffer_create(FIFO_SIZE);
	pthread_create(&record_pid, NULL, record_from_dev, ring_buf);
	//pthread_create(&send_data_to_com_thread_pid, NULL, send_data_to_com_thread, NULL);
	pthread_create(&mosq_pid, NULL, mosq_loop, NULL);
	pthread_create(&mic_wakeup_thread_id, NULL, microphone_wakeup_poll_thread, NULL);
	//pthread_create(&dht11_loop_id, NULL, dht11_loop, NULL);
	
	// recognize
        recognize_from_microphone(ring_buf);
    }

    ps_free(ps);
    cmd_ln_free_r(config);
*/
   pthread_create(&dht11_loop_id, NULL, dht11_loop, NULL);

   printf("\n#####################################################\n");
   printf("\n#######################System boot done!#############\n"); 
   printf("\n#####################################################\n");
/*
   if(net_check())
   {
	pthread_create(&mosq_pid, NULL, mosq_loop, NULL);
   }
*/


   for(;;){
		
		if(system("ping -c 1 -w 2 dongle.topqizhi.com") == 0){
    			pthread_create(&mosq_pid, NULL, mosq_loop, NULL);
			break;
		}else{
			sleep(5);
		}
	}


//    pthread_join(record_pid, NULL);
   // pthread_join(send_data_to_com_thread_pid, NULL);
    pthread_join(mosq_pid, NULL);
  //  pthread_join(mic_wakeup_thread_id, NULL);
    pthread_join(dht11_loop_id, NULL);

    //ringbuffer_destroy(ring_buf);

    return 0;
}

#if defined(_WIN32_WCE)
#pragma comment(linker,"/entry:mainWCRTStartup")
#include <windows.h>
//Windows Mobile has the Unicode main only
int
wmain(int32 argc, wchar_t * wargv[])
{
    char **argv;
    size_t wlen;
    size_t len;
    int i;

    argv = malloc(argc * sizeof(char *));
    for (i = 0; i < argc; i++) {
        wlen = lstrlenW(wargv[i]);
        len = wcstombs(NULL, wargv[i], wlen);
        argv[i] = malloc(len + 1);
        wcstombs(argv[i], wargv[i], wlen);
    }

    //assuming ASCII parameters
    return main(argc, argv);
}
#endif
