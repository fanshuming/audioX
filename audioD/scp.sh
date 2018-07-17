scp lib/libpocketsphinx.so.3.0.0 lib/libsphinxbase.so.3.0.0 lib/libsphinxad.so.3.0.0 root@192.168.100.1:/usr/lib
scp -r smodel root@192.168.100.1:/root
scp -r etc/command.dic etc/command.jsgf resource/xfm10213.ldr  root@192.168.100.1:/root
scp bin/audioD root@192.168.100.1:/usr/bin
scp /home/fan/qz_proj/audio_dongle/openwrt/bin/ramips/packages/base/kmod-gpio_drv_3.18.84-1_ramips_24kec.ipk root@192.168.100.1:/tmp
scp /home/fan/qz_proj/audio_dongle/openwrt/bin/ramips/packages/base/kmod-dht11_3.18.84-1_ramips_24kec.ipk root@192.168.100.1:/tmp
