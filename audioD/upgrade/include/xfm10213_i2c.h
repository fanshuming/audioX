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

#include "log.h"

#define xfm10213_ADDR           (0x3E)
#define DELAY_MS(ms)			usleep((ms) * 1000)
#define I2C_DEFDEV_NAME			"/dev/i2c-0"
#define I2C_BUF_LEN				(128)
#define MAX_PATH				(260)

extern int xfm_i2c(void);
