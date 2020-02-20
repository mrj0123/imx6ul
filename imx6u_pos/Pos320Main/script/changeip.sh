#!/bin/bash
lineno=0
linenum=$(sed -n '/Wired or wireless interfaces/=' /etc/network/interfaces  )
linenum=$(($linenum+3))

if [ $#=1 ]; then
	
	#sed -i ''"$linenum"'c \\taddress 192.168.18.200' interfaces 
	sed -i ''"$linenum"'c \\taddress '"$1"'' /etc/network/interfaces
fi
echo 0
