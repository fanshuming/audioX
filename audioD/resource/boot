#!/bin/sh /etc/rc.common
# Copyright (C) 2006-2011 OpenWrt.org

START=10
STOP=98

uci_apply_defaults() {
	. /lib/functions/system.sh

	cd /etc/uci-defaults || return 0
	files="$(ls)"
	[ -z "$files" ] && return 0
	mkdir -p /tmp/.uci
	for file in $files; do
		( . "./$(basename $file)" ) && rm -f "$file"
	done
	uci commit
}

boot() {
	[ -f /proc/mounts ] || /sbin/mount_root
	[ -f /proc/jffs2_bbc ] && echo "S" > /proc/jffs2_bbc
	[ -f /proc/net/vlan/config ] && vconfig set_name_type DEV_PLUS_VID_NO_PAD

	mkdir -p /var/run
	mkdir -p /var/log
	mkdir -p /var/lock
	mkdir -p /var/state
	mkdir -p /tmp/.uci
	chmod 0700 /tmp/.uci
	mkdir -p /tmp/.jail
	touch /var/log/wtmp
	touch /var/log/lastlog
	touch /tmp/resolv.conf.auto
	ln -sf /tmp/resolv.conf.auto /tmp/resolv.conf
	grep -q debugfs /proc/filesystems && /bin/mount -o noatime -t debugfs debugfs /sys/kernel/debug
	[ "$FAILSAFE" = "true" ] && touch /tmp/.failsafe

	/sbin/kmodloader

	# allow wifi modules time to settle
	sleep 1

	/sbin/wifi detect > /tmp/wireless.tmp
	[ -s /tmp/wireless.tmp ] && {
		cat /tmp/wireless.tmp >> /etc/config/wireless
	}
	rm -f /tmp/wireless.tmp

	/bin/board_detect
	uci_apply_defaults
	
	# temporary hack until configd exists
	/sbin/reload_config
	#add by shuming fan
	/etc/init.d/audioDongleBoot enable

	# add by shuming fan
	[ ! -f /dev/gpio_drv ] && {
		/bin/opkg install /root/kmod-gpio_drv_3.18.84-1_ramips_24kec.ipk
		/bin/opkg install /root/kmod-dht11_3.18.84-1_ramips_24kec.ipk
		/usr/sbin/insmod  /lib/modules/3.18.84/dht11.ko 
		/usr/sbin/insmod  /lib/modules/3.18.84/gpio_drv.ko
	}

	# add by shuming fan
	[ ! -f /usr/lib/libpocketsphinx.so.3 ] && {
		/bin/ln -s /usr/lib/libpocketsphinx.so.3.0.0 /usr/lib/libpocketsphinx.so.3
		/bin/ln -s /usr/lib/libsphinxbase.so.3.0.0 /usr/lib/libsphinxbase.so.3
		/bin/ln -s /usr/lib/libsphinxad.so.3.0.0 /usr/lib/libsphinxad.so.3
		/bin/ln -s /usr/lib/libupnp.so.2.0.1 /usr/lib/libupnp.so.2
		/bin/ln -s /usr/lib/libthreadutil.so.2.0.0 /usr/lib/libthreadutil.so.2
		/bin/ln -s /usr/lib/libixml.so.2.0.0 /usr/lib/libixml.so.2
	}

	
	# create /dev/root if it doesn't exist
	[ -e /dev/root -o -h /dev/root ] || {
		rootdev=$(awk 'BEGIN { RS=" "; FS="="; } $1 == "root" { print $2 }' < /proc/cmdline)
		[ -n "$rootdev" ] && ln -s "$rootdev" /dev/root
	}
}
