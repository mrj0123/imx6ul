#!/bin/bash

linenum=$(sed -n '/ssid="/=' /etc/wpa_supplicant.conf)
echo $linenum
	sed -i ''"$linenum"' c \\tssid=\"'"$1"''\" /etc/wpa_supplicant.conf

echo 0
