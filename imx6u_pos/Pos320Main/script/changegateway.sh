#!/bin/bash
lineno=0
linenum=$(sed -n '/Wired or wireless interfaces/=' /etc/network/interfaces  )
linenum=$(($linenum+5))

if [ $#=1 ]; then
	
	sed -i ''"$linenum"'c \\tgateway '"$1"'' /etc/network/interfaces
	echo 0
else
	echo -1
fi

