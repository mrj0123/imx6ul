#!/bin/bash
if [ -f /usr/local/nkty/temp/dns.txt ]; then
	rm /usr/local/nkty/temp/dns.txt
fi
linenum=0
linenum=$(sed -n '/# Wired or wireless interfaces/=' /etc/network/interfaces)
linenum=$(($linenum+6))
	
	data=$(	sed -n ''"$linenum"'p '  /etc/network/interfaces)
        
	echo ${data#*servers }  >/usr/local/nkty/temp/1.out #删除server 左边的字符
	
	
	cat /usr/local/nkty/temp/1.out | while read line #按行读入并赋值给line
	do
     		#echo $line
		 i=1
	    	while :
    		do
	        str=` echo $line | cut -d " " -f $i `
        	#echo $i, $str
	        if [ "$str" == "$line" ];then
        	    arr[j]=$str
	            break
        	elif [ "$str" != "" ];then
	            arr[j]=$str
        	else
	            break
        	fi
		i=$(($i+1))
	        j=$(($j+1))
	    done
		#echo $i,$j
	    if [ "${arr[0]}" != "" ]; then
	    echo ${arr[0]}>/usr/local/nkty/temp/dns.txt
  	    fi	
  	    if [ $i -gt 1 ]; then 
	      echo ${arr[1]}>>/usr/local/nkty/temp/dns.txt
	    fi
 	done
	rm /usr/local/nkty/temp/1.out

