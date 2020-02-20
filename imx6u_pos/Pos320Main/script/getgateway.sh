#!/bin/bash
lineno=0
linenum=$(sed -n '/# Wired or wireless interfaces/=' /etc/network/interfaces)
linenum=$(($linenum+5))
	
	data=$(	sed -n ''"$linenum"'p '  /etc/network/interfaces)
	echo ${data#*gateway }
	
	
