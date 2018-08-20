#include <stdio.h>
#include <stdint.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>

#include "microphone.h"

#include "log.h"

#define xfm10213_ADDR           (0x3E)
#define DELAY_MS(ms)			usleep((ms) * 1000)
#define I2C_DEFDEV_NAME			"/dev/i2c-0"
#define I2C_BUF_LEN				(128)
#define MAX_PATH				(260)

static int i2c_write_proc(int fd, unsigned char addr, unsigned char reg[], unsigned char *val, unsigned char len) {
	struct i2c_rdwr_ioctl_data cmd;
	struct i2c_msg msg[2];
	msg[0].addr	= addr;
	msg[0].flags = 0;
	msg[0].len = 2;
	msg[0].buf = reg;

	msg[1].addr	= addr;
	msg[1].flags = 0 /* | I2C_M_NOSTART*/;
	msg[1].len = len;
	msg[1].buf = val;

	/* Construct the i2c_rdwr_ioctl_data struct */
	cmd.msgs =  msg;
	cmd.nmsgs = sizeof(msg) / sizeof(struct i2c_msg);

	/* Send the request to the kernel and get the result back */
	if (ioctl(fd, I2C_RDWR, &cmd) < 0) {
		LOGD("Unable to send data!\n");
		return -1;
	}

	return 0;
}

static int i2c_read_proc(int fd, unsigned char addr, unsigned char reg[], unsigned char *val, unsigned char len) {
	struct i2c_rdwr_ioctl_data cmd;
	struct i2c_msg msg[2];

	msg[0].addr	= addr;
	msg[0].flags = 0;
	msg[0].len = 2;
	msg[0].buf = reg;

	msg[1].addr	= addr;
	msg[1].flags = I2C_M_RD /* | I2C_M_NOSTART*/;
	msg[1].len = len;
	msg[1].buf = val;

	/* Construct the i2c_rdwr_ioctl_data struct */
	cmd.msgs = msg;
	cmd.nmsgs = sizeof(msg) / sizeof(struct i2c_msg);

	/* Send the request to the kernel and get the result back */
	if (ioctl(fd, I2C_RDWR, &cmd) < 0) {
		LOGD("Unable to send data!\n");
		return -1;
	}

	return 0;
}

//
static int i2c_download_proc(int fd,unsigned char addr,unsigned char *val,unsigned char len){

	struct i2c_rdwr_ioctl_data cmd;
	struct i2c_msg msg[1];
	msg[0].addr = addr;
	msg[0].flags = 0;
	msg[0].len = len;
	msg[0].buf = val;

	cmd.msgs = msg;
	cmd.nmsgs = sizeof(msg) / sizeof(struct i2c_msg);

	/* Send the request to the kernel and get the result back */
	if (ioctl(fd, I2C_RDWR, &cmd) < 0) {
		LOGD("Unable to send data!\n");
		return -1;
	}

	return 0;

}

int upgrage_pak(void) {
	int fd;
	int ret;

	fd = open(I2C_DEFDEV_NAME, O_RDWR);
	if (fd < 0) {
		LOGD("Error opening file: %s\n", strerror(errno));
		return 1;
	}
	if (ioctl(fd, I2C_SLAVE, xfm10213_ADDR) < 0) {
		LOGD("ioctl error: %s\n", strerror(errno));
		return 1;
	}else{
		LOGD("open success\n");
	}

#if 0
//open xfm10213.ldr
	FILE *src_fp_update = fopen("./xfm10213.ldr","rb");
	if(src_fp_update == NULL)
	{
		LOGD("xfm10213.ldr file not found\n");
		return 0;
	}
	fseek(src_fp_update, 0L, SEEK_END);
	unsigned int src_fp_len_update = ftell(src_fp_update);
	char *buf_update = (char *)malloc(src_fp_len_update);
	memset(buf_update,0,src_fp_len_update);
	fseek(src_fp_update,0L,SEEK_SET);

	fread(buf_update,1,src_fp_len_update,src_fp_update);
	unsigned char r2w_update = 32;//255;
	int down_count_update = 0;
	while(1)
	{
		if(src_fp_len_update >= r2w_update)
		{
			down_count_update++;
			ret = i2c_download_proc(fd, xfm10213_ADDR, buf_update, r2w_update);
			if(ret == -1) {
				LOGE("download error\n");
				break;
			}
			src_fp_len_update -= r2w_update;
			buf_update += r2w_update;
		}
		else
		{
			down_count_update++;
			i2c_download_proc(fd,xfm10213_ADDR,buf_update,src_fp_len_update);
			LOGD("====================down_count=%d\n", down_count_update);
			break;
		}
	}
	fclose(src_fp_update);
	DELAY_MS(2000);
#endif
	//open upgrade.pak
	FILE *src_fp = fopen("./upgrade.pak","rb");
	//FILE *src_fp = fopen("./xfm10213.ldr","rb");
	if(src_fp == NULL)
	{
		LOGD("xfm10213.ldr file not found\n");
		return 0;
	}
	fseek(src_fp, 0L, SEEK_END);
	unsigned int src_fp_len = ftell(src_fp);
	char *buf = (char *)malloc(src_fp_len);
	memset(buf,0,src_fp_len);
	fseek(src_fp,0L,SEEK_SET);

	fread(buf,1,src_fp_len,src_fp);
	LOGD("src_fp_len:%d\n",src_fp_len);

	unsigned char r2w = 32;//255;//32;
	int down_count = 0;
	while(1)
	{
		if(src_fp_len >= r2w)
		{
			down_count++;
			ret = i2c_download_proc(fd, xfm10213_ADDR, buf, r2w);
			if(ret == -1) {
				LOGE("download error\n");
				break;
			}
			src_fp_len -= r2w;
			buf += r2w;
		}
		else
		{
			down_count++;
			i2c_download_proc(fd,xfm10213_ADDR,buf,src_fp_len);
			LOGD("====================down_count=%d\n", down_count);
			break;
		}
	}
	fclose(src_fp);
	DELAY_MS(2000);

#if 0
	unsigned int temp = 0x0000;
	//
	unsigned int data_mode = 0x0200;      //写入的是0002
	unsigned int data_mode2 = 0x0400;     //写入的是0004
//	unsigned int data_chan = 0x0010;		//写入的是1256
//	unsigned int data_chan = 0x5010;		//写入的是1256
	unsigned int data_chan = 0x5612;		//写入的是1256
	unsigned char reg101[2] = {0x01,0x01};
	unsigned char reg110[2] = {0x01,0x10};
	unsigned char reg111[2] = {0x01,0x11};

#if 1
	i2c_read_proc(fd, xfm10213_ADDR, reg101, (unsigned char *)&temp, 2);
	DELAY_MS(200);
	LOGD("read 101 data is %04x\n", temp);

	i2c_write_proc(fd,xfm10213_ADDR, reg110, (char *)&data_mode2, 2);
	DELAY_MS(200);
	LOGD("write 110 data is %04x\n", data_mode2);

	i2c_read_proc(fd, xfm10213_ADDR, reg101, (unsigned char *)&temp, 2);
	DELAY_MS(200);
	LOGD("read 101 data is %04x\n", temp);

	i2c_read_proc(fd, xfm10213_ADDR, reg110, (unsigned char *)&temp, 2);
	DELAY_MS(200);
	LOGD("read 110 data is %04x\n\n", temp);
#endif

#if 1

	i2c_write_proc(fd,xfm10213_ADDR, reg111, (unsigned char *)&data_chan, 2);
	DELAY_MS(200);
	LOGD("write 111 data is %04x\n", data_chan);

	i2c_read_proc(fd, xfm10213_ADDR, reg101, (unsigned char *)&temp, 2);
	DELAY_MS(200);
	LOGD("read 101 data is %04x\n", temp);

	i2c_read_proc(fd, xfm10213_ADDR, reg111, (unsigned char *)&temp, 2);
	DELAY_MS(300);
	//set_mic_enable(true);
	LOGD("read 111 data is %04x\n", temp);
#endif


#endif
	return 0;
}
