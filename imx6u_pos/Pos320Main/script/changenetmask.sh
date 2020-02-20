#!/bin/bash
lineno=0
linenum=$(sed -n '/Wired or wireless interfaces/=' /etc/network/interfaces  )
linenum=$(($linenum+4))

if [ $#=1 ]; then
	
	sed -i ''"$linenum"'c \\tnetmask '"$1"'' /etc/network/interfaces
	echo 0
fi

