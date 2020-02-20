#!bin/bash
linenum=0
	sudo ifup wlan0
	sudo iwlist wlan0 scanning | grep ESSID >/usr/local/nkty/temp/wifilist.txt
	
 	
	if [ -f /usr/local/nkty/temp/wifilist2.txt ]; then 
 		rm -rf /usr/local/nkty/temp/wifilist2.txt
	 fi  
  cat /usr/local/nkty/temp/wifilist.txt| while read line #按行读入并赋值给line
        do
                echo ${line#*ESSID:}>>/usr/local/nkty/temp/wifilist2.txt
        done
	rm -rf /usr/local/nkty/temp/wifilist.txt
       
