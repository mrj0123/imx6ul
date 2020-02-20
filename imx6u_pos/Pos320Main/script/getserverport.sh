#!/bin/bash
linenum=0
if [ $# -gt 0 ]; then
	if [ $# -eq 1 ]; then
		#有一个参数
		linenum=$(sed -n '/server'"$1"'/=' /usr/local/nkty/script/nktyserver.conf)
		
		if [ $linenum > 0 ]; then
			((linenum=$linenum+2))
			data=$( sed -n ''"$linenum"'p '  /usr/local/nkty/script/nktyserver.conf)
			echo $data | cut -c 6-200

		else 
			echo -1
		fi
	fi
else
	echo -1
fi
		

