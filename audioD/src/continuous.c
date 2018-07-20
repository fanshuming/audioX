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
#include <signal.h>

#if defined(_WIN32) && !defined(__CYGWIN__)
#include <windows.h>
#else
#include <sys/select.h>
#endif

#include <sphinxbase/err.h>
#include <sphinxbase/ad.h>

#include "pocketsphinx.h"

#include "xfm10213_i2c.h"
#include "tty_com.h"
#include "led.h"
#include "dht11_app.h"
#include "sub_client.h"

int blink_cnt = 0;

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

    fname = cmd_ln_str_r(config, "-infile");
    if ((rawfd = fopen(fname, "rb")) == NULL) {
        E_FATAL_SYSTEM("Failed to open file '%s' for reading",
                       fname);
    }
   /* 
    if (strlen(fname) > 4 && strcmp(fname + strlen(fname) - 4, ".wav") == 0) {
        char waveheader[44];
	fread(waveheader, 1, 44, rawfd);
	if (!check_wav_header(waveheader, (int)cmd_ln_float32_r(config, "-samprate")))
    	    E_FATAL("Failed to process file '%s' due to format mismatch.\n", fname);
    }

    if (strlen(fname) > 4 && strcmp(fname + strlen(fname) - 4, ".mp3") == 0) {
	E_FATAL("Can not decode mp3 files, convert input file to WAV 16kHz 16-bit mono before decoding.\n");
    }
*/    
    ps_start_utt(ps);
    utt_started = FALSE;

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


/*
* led
*/

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

/*
 * Main utterance processing loop:
 *     for (;;) {
 *        start utterance and wait for speech to process
 *        decoding till end-of-utterance silence will be detected
 *        print utterance result;
 *     }
 */
static void
recognize_from_microphone()
{
    ad_rec_t *ad;
    int16 adbuf[2048];
    //int16 adbuf[6400];
    uint8 utt_started, in_speech;
    int32 k;
    char const *hyp;

    //for tty com

    int ttyFd;
    int sendlen;
    char send_buf[20]="uartx_test\n"; 
    char rcv_buf[20]={0};

    //ttyFd = UARTx_Open(ttyFd,"/dev/ttyS0");

    if ((ad = ad_open_dev(cmd_ln_str_r(config, "-adcdev"),(int) cmd_ln_float32_r(config,"-samprate"))) == NULL)
        E_FATAL("Failed to open audio device\n");

	xfm_i2c();

    if (ad_start_rec(ad) < 0)
        E_FATAL("Failed to start recording\n");

    if (ps_start_utt(ps) < 0)
        E_FATAL("Failed to start utterance\n");
    utt_started = FALSE;
    E_INFO("Ready....\n");

    FILE *captureFp = fopen("/tmp/capture.pcm","wb");

    for (;;) {

        if ((k = ad_read(ad, adbuf, 2048)) < 0)
            E_FATAL("Failed to read audio\n");

      	fwrite(adbuf, 1, k, captureFp);
 
        ps_process_raw(ps, adbuf, k, FALSE, FALSE);
        in_speech = ps_get_in_speech(ps);
        if (in_speech && !utt_started) {
            utt_started = TRUE;
            E_INFO("Listening...\n");
        }
        if (!in_speech && utt_started) {
            // speech -> silence transition, time to start new utterance 
            ps_end_utt(ps);
            hyp = ps_get_hyp(ps, NULL );
            if (hyp != NULL) {
		//char * ts = enc_utf8_to_unicode_one(hyp);
                printf("%s\n", hyp);

		//add led
		//blink_cnt = 0;
		signal(SIGALRM, sigalrm_led);
		led_on();
   		alarm(1);

		//add tty com

		if(!strcmp(hyp, "head up"))
    		{
        		memcpy(send_buf, head_up_buf, sizeof(head_up_buf) / sizeof(char));
    		}else if(!strcmp(hyp, "head down")){
        		memcpy(send_buf, head_down_buf, sizeof(head_up_buf) / sizeof(char));
		}else if(!strcmp(hyp, "foot up")){
        		memcpy(send_buf, foot_up_buf, sizeof(head_up_buf) / sizeof(char));
                }else if(!strcmp(hyp, "foot down")){
        		memcpy(send_buf, foot_down_buf, sizeof(head_up_buf) / sizeof(char));
                }else if(!strcmp(hyp, "leg up")){
        		memcpy(send_buf, leg_up_buf, sizeof(head_up_buf) / sizeof(char));
                }else if(!strcmp(hyp, "leg down")){
        		memcpy(send_buf, leg_down_buf, sizeof(head_up_buf) / sizeof(char));
                }else if(!strcmp(hyp, "lumbar up")){
        		memcpy(send_buf, lumbar_up_buf, sizeof(head_up_buf) / sizeof(char));
                }else if(!strcmp(hyp, "lumbar down")){
        		memcpy(send_buf, lumbar_down_buf, sizeof(head_up_buf) / sizeof(char));
                }else if(!strcmp(hyp, "stop")){
        		memcpy(send_buf, stop_buf, sizeof(head_up_buf) / sizeof(char));
                }else if(!strcmp(hyp, "flat")){
        		memcpy(send_buf, flat_buf, sizeof(head_up_buf) / sizeof(char));
                }else if(!strcmp(hyp, "antisnore")){
        		memcpy(send_buf, antisnore_buf, sizeof(head_up_buf) / sizeof(char));
                }else if(!strcmp(hyp, "lounge")){
        		memcpy(send_buf, lounge_buf, sizeof(head_up_buf) / sizeof(char));
                }else if(!strcmp(hyp, "zero gravity")){
        		memcpy(send_buf, zero_gravity_buf, sizeof(head_up_buf) / sizeof(char));
                }else if(!strcmp(hyp, "incline")){
        		memcpy(send_buf, incline_buf, sizeof(head_up_buf) / sizeof(char));
                }else if(!strcmp(hyp, "lounge program")){
        		memcpy(send_buf, lounge_program_buf, sizeof(head_up_buf) / sizeof(char));
                }else if(!strcmp(hyp, "zero gravity program")){
        		memcpy(send_buf, zero_gravity_program_buf, sizeof(head_up_buf) / sizeof(char));
                }else if(!strcmp(hyp, "incline program")){
        		memcpy(send_buf, incline_program_buf, sizeof(head_up_buf) / sizeof(char));
                }else if(!strcmp(hyp, "massage on")){
        		memcpy(send_buf, massage_on_buf, sizeof(head_up_buf) / sizeof(char));
                }else if(!strcmp(hyp, "wave one")){
        		memcpy(send_buf, wave_one_buf, sizeof(head_up_buf) / sizeof(char));
                }else if(!strcmp(hyp, "wave two")){
        		memcpy(send_buf, wave_two_buf, sizeof(head_up_buf) / sizeof(char));
                }else if(!strcmp(hyp, "wave three")){
        		memcpy(send_buf, wave_three_buf, sizeof(head_up_buf) / sizeof(char));
                }else if(!strcmp(hyp, "wave four")){
        		memcpy(send_buf, wave_four_buf, sizeof(head_up_buf) / sizeof(char));
                }else if(!strcmp(hyp, "full body one")){
        		memcpy(send_buf, full_body_one_buf, sizeof(head_up_buf) / sizeof(char));
                }else if(!strcmp(hyp, "full body two")){
        		memcpy(send_buf, full_body_two_buf, sizeof(head_up_buf) / sizeof(char));
                }else if(!strcmp(hyp, "massage up")){
        		memcpy(send_buf, massage_up_buf, sizeof(head_up_buf) / sizeof(char));
                }else if(!strcmp(hyp, "massage down")){
        		memcpy(send_buf, massage_down_buf, sizeof(head_up_buf) / sizeof(char));
                }else if(!strcmp(hyp, "massage stop")){
        		memcpy(send_buf, massage_stop_buf, sizeof(head_up_buf) / sizeof(char));
                }else if(!strcmp(hyp, "light on")){
        		memcpy(send_buf, light_on_buf, sizeof(head_up_buf) / sizeof(char));
                }else if(!strcmp(hyp, "lights on")){
        		memcpy(send_buf, lights_on_buf, sizeof(head_up_buf) / sizeof(char));
                }else if(!strcmp(hyp, "light off")){
        		memcpy(send_buf, light_off_buf, sizeof(head_up_buf) / sizeof(char));
                }else if(!strcmp(hyp, "lights off")){
        		memcpy(send_buf, lights_off_buf, sizeof(head_up_buf) / sizeof(char));
                }else if(!strcmp(hyp, "toggle lights")){
        		memcpy(send_buf, toggle_lights_buf, sizeof(head_up_buf) / sizeof(char));
                }else if(!strcmp(hyp, "toggle light")){
        		memcpy(send_buf, toggle_light_buf, sizeof(head_up_buf) / sizeof(char));
                }else if(!strcmp(hyp, "feedback off")){
                        memcpy(send_buf, "None", sizeof("None"));
                }else if(!strcmp(hyp, "feedback on")){
                        memcpy(send_buf, "None", sizeof("None"));
                }
		else{
			memcpy(send_buf, "None", sizeof("None"));
		}

                sendlen = UARTx_Send(ttyFd, send_buf, sizeof(head_up_buf) / sizeof(char));
                sendlen = UARTx_Send(ttyFd, head_up_buf, sizeof(head_up_buf) / sizeof(char));
		if(sendlen > 0){  
            		printf("\nsend data:%x %x %x %x %x %x successful len=%d \n",send_buf[0],send_buf[1],send_buf[2],send_buf[3],send_buf[4],send_buf[5], sendlen);  
		}else{ 
            		printf("send data failed!\n");
		}
		
		//read(ttyFd, rcv_buf, 20);
		//printf("rcv:%s\n",rcv_buf);		
                fflush(stdout);
            }

            if (ps_start_utt(ps) < 0)
                E_FATAL("Failed to start utterance\n");
            utt_started = FALSE;
		//ad_read(ad, adbuf, 4608);
		ad_read(ad, adbuf, 2048);
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
    unsigned int temp;
    unsigned int  humi;

    pthread_t dht11_pid;
    pthread_t mosq_pid;

    config = cmd_ln_parse_r(NULL, cont_args_def, argc, argv, TRUE);

    /* Handle argument file as -argfile. */
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

    //add blue led 
//    led_init();
    //get temperature and huminity
    //dht11_init();
    //get_temp_humi(&temp, &humi);
    //printf("temp:%d, humi:%d\n",temp, humi);

    pthread_create(&dht11_pid, NULL, dht11_loop, NULL);
    pthread_create(&mosq_pid, NULL, mosq_loop, NULL);

    E_INFO("%s COMPILED ON: %s, AT: %s\n\n", argv[0], __DATE__, __TIME__);

    if (cmd_ln_str_r(config, "-infile") != NULL) {
        recognize_from_file();
    } else if (cmd_ln_boolean_r(config, "-inmic")) {
        recognize_from_microphone();
    }

    pthread_join(dht11_pid, NULL);
    pthread_join(mosq_pid, NULL);

    ps_free(ps);
    cmd_ln_free_r(config);

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
