.PHONY: clean

DATE = $(shell date "+%Y%m%d")  

#$(info $(DATE) )

CURRENT_DIR = $(shell pwd)

CC = mipsel-openwrt-linux-gcc
CFLAGS = -I ./include -O3
LDFLAGS = -L ./lib -lpocketsphinx -lsphinxbase -lsphinxad -lpthread -lcares -lcurl -lmbedtls -lcrypto -lssl -L ./lib/alsa_lib -lasound -lmosquitto -lm
#LDFLAGS = -L ./lib -lpocketsphinx -lsphinxbase -lsphinxad -lpthread -lcares  -lcrypto -lssl  -lmosquitto

INSTALL = install
STRIP = mipsel-openwrt-linux-uclibc-strip


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
	@echo $(DATE) > $(CURRENT_DIR)/resource/version

uninstall :
	-rm -f $(BIN_TARGET)

clean:
	@rm -f ./src/*.o $(TARGET) $(BIN_TARGET)
