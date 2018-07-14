#ifndef __DHT11_H__
#define __DHT11_H__

int dht11_init();
int get_temp_humi(unsigned int * T, unsigned int * H);
void * dht11_loop();

#endif
