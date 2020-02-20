#!/bin/bash

flag=$1
 
if [ $flag == 1 ]; then
	#echo switch on
	sudo iwconfig wlan0 ap auto
	ifdown wlan0
	wpa_supplicant -B -i wlan0 -c /etc/wpa_supplicant.conf
	sudo ifup wlan0  
	sleep 1

	sudo ifdown eth0
	sleep 1
	ip addr flush dev eth0	
else
	echo swicth off
	ifdown wlan0
fi

