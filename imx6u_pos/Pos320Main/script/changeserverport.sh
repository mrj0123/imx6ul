#!/bin/bash
linenum=0
if [ $# -eq 2 ]; then
	  linenum=$(sed -n '/\[server'"$1"'\]/=' /usr/local/nkty/script/nktyserver.conf)

	if [ ! -n "$linenum" ] ;then
		echo  -2
	else
		((linenum=$linenum+2))
		sed -i ''"$linenum"'c \\tport='"$2"'' /usr/local/nkty/script/nktyserver.conf
		echo 0
	fi
else
	echo -1
fi

