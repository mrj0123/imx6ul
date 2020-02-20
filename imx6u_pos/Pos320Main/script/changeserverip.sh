#!/bin/bash
linenum=0

if [ $# -eq 2 ]; then
	  linenum=$(sed -n '/\[server'"$1"'\]/=' /usr/local/nkty/script/nktyserver.conf)
	  if [ $linenum >0 ]; then
		((linenum=$linenum+1))
		sed -i ''"$linenum"'c \\tipaddress='"$2"'' /usr/local/nkty/script/nktyserver.conf
		echo 0
	fi
else
	echo -1
fi

