#!/bin/bash
lineno=0
linenum=$(sed -n '/Wired or wireless interfaces/=' /etc/network/interfaces  )
linenum=$(($linenum+6))
if [ $# -eq 1 ]; then
	sed -i ''"$linenum"'c \\tdns-nameservers '"$1"' ' /etc/network/interfaces
	echo 0
elif [ $# -eq 2 ]; then
	sed -i ''"$linenum"'c \\tdns-nameservers '"$1"' '"$2"' ' /etc/network/interfaces
	echo 0
fi

