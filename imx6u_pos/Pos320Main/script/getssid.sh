#!/bin/bash
lineno=0
#linenum=$(sed -n '/#network={/=' /etc/wpa_supplicant.conf)
linenum=$(sed -n '/ssid="/=' /etc/wpa_supplicant.conf)
#linenum=$(($linenum+5))

	data=$(	sed -n ''"$linenum"'p '  /etc/wpa_supplicant.conf) 
	
	echo $data | cut -c 7-$(expr ${#data}) | sed 's/.$//' 
		

