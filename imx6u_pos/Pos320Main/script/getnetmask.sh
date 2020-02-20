#!/bin/bash
lineno=0
linenum=$(sed -n '/# Wired or wireless interfaces/=' /etc/network/interfaces)
linenum=$(($linenum+4))
	
	data=$(	sed -n ''"$linenum"'p '  /etc/network/interfaces)
	echo $data | cut -c 9-250
	
	
