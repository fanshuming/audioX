echo "gpio btn wps!"

#route add -net 239.255.255.250 netmask 255.255.255.255 apcli0
#wscd -w /etc/xml/ -m 3 -d 3 -i apcli0 -D

iwpriv apcli0 set WscConfMode=1
iwpriv apcli0 set  WscMode=2
iwpriv apcli0 set WscGetConf=1
