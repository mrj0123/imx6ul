#!/bin/bash
lineno=0
linenum=$(sed -n '/psk=/=' /etc/wpa_supplicant.conf)
#linenum=$(($linenum+11))
#if [ $#=1 ]; then
	
	sed -i ''"$linenum"'c \\tpsk=\"'"$1"''\" /etc/wpa_supplicant.conf
#fi
echo 0
