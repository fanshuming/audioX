.PHONY: clean
  
CURRENT_DIR = $(shell pwd)

#toolchain path
#CROSS_COMPILE=$(HOME)/openwrt/staging_dir/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/bin/

CC = mipsel-openwrt-linux-gcc
CFLAGS = -I ./include -O3
#LDFLAGS = -L ./lib -lpocketsphinx -lsphinxad -lsphinxbase -lpthread
LDFLAGS = -L ./lib -lpocketsphinx -lsphinxbase -lsphinxad -lpthread -L ./lib/alsa_lib -lasound
#LDFLAGS = -L ./lib -lpocketsphinx -lsphinxbase  -lpthread -L ./lib/alsa_lib -lasound

INSTALL = install
STRIP = mipsel-openwrt-linux-uclibc-strip

#LDFLAGS_RD = -L $(CURRENT_DIR)/third_party/mips/alsa_lib -lasound 
#CFLAGS_RD = -I $(CURRENT_DIR)/third_party/mips/alsa_lib/include -I $(CURRENT_DIR)/third_party/mips/alsa-utils/include

#CFLAGS_ALEXA = -I $(CURRENT_DIR)/third_party/mips/libcurl/include/curl
#LDFLAGS_ALEXA = -L $(CURRENT_DIR)/third_party/mips/libcurl/lib -lcurl -L $(CURRENT_DIR)/third_party/mips/libng/lib -lnghttp2 -lm -L $(CURRENT_DIR)/third_party/mips/zlib/lib/ -lz -L $(CURRENT_DIR)/third_party/mips/libssl102/lib -lcrypto -L $(CURRENT_DIR)/third_party/mips/libssl102/lib/ -lssl

#LDFLAGS_MOSQ = -L $(CURRENT_DIR)/third_party/mips/mosquitto-1.4.10/lib -lmosquitto  -L ./lib -lcares


TARGET = audioD
BIN_TARGET = bin/audioD

OBJECTS := $(patsubst %.c,%.o,$(wildcard ./src/*.c))

$(TARGET) : $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@



install : $(TARGET)
	$(INSTALL) -d ./bin
	$(INSTALL) -s --strip-program=${STRIP} $(TARGET) $(BIN_TARGET)

uninstall :
	-rm -f $(BIN_TARGET)

clean:
	@rm -f ./src/*.o $(TARGET) $(BIN_TARGET)
