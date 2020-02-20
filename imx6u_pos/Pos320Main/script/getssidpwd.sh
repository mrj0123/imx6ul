#!/bin/bash
lineno=0
linenum=$(sed -n '/#network={/=' /etc/wpa_supplicant.conf)
linenum=$(($linenum+11))
#if [ $#=1 ]; then
	
	data=$(	sed -n ''"$linenum"'p '  /etc/wpa_supplicant.conf)
		
	echo $data | cut -c 6-$(expr ${#data}) | sed 's/.$//'


