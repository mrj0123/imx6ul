#!/bin/bash


mydate="`date +%Y-%m-%d`" 

mytime="`date +%H:%M:%S`"

#sh killprocess.sh

#check wlan0 exists

count1=$(ifconfig -a | grep wlan0 -c )
echo $count1

if [ $count1 == 0 ]; then
	echo wlan0 not found 
	sh script/checkwlan.sh	
	sleep 1
fi

count2=$(ifconfig -a | grep wlan0 -c )
if [ $count2 == 0 ]; then
        echo not found wlan0
	sh script/checkwlan.sh
fi
echo $count2

echo "[$mydate $mytime] NKTY Pos Program Started"  >>log.txt  

if test $( pgrep -f "terminalProject" | wc -l ) -eq 0  
then  
	echo "terminalProject不存在"
	./terminalProject -platform wayland>>terminalProject.txt &  
	echo "[$mydate $mytime]===============terminalProject Started============" >>log.txt
else
	echo "terminalProject 已启动"
fi

if test $( pgrep -f "Pos320Main" | wc -l ) -eq 0
then
	
	./Pos320Main >>posmainlog.txt &                                                
	echo "[$mydate $mytime]===============Pos320Main Started=============" >>log.txt
	sleep 0.2                                                                       
else
	echo "Pos320Main 已启"
fi

if test $( pgrep -f "secscreen" | wc -l ) -eq 0  
then                                                                            
        ./secscreen >>secscreen.txt &
	echo "[$mydate $mytime]===============secscreen Started=============" >>log.txt
	sleep 1
else
	echo "secscreen 已启动"
fi


if test $( pgrep -f "scanqrser" | wc -l ) -eq 0                                         
then           
	echo "进程scanqrser不存在"
	./scanqrser >>scanqr.txt &
	echo "[$mydate $mytime]===============scanqrser Started=============" >>log.txt
	sleep 0.2
else                                                                                    
        echo "scanqrser 已启动"  
fi

if test $( pgrep -f "consumeser" | wc -l ) -eq 0                                        
then  
	echo "consumeser 不存在"
	./consumeser >>consumelog.txt &
	echo "[$mydate $mytime]===============ConsumeSer Started=============" >>log.txt
	sleep 0.2
else                                                                                    
        echo "sonsumeser 已启动"  
fi

if test $( pgrep -f "terminalProject" | wc -l ) -eq 0  
then  
	echo "terminalProject不存在"
	./terminalProject -platform wayland>>terminalProject.txt &  
	echo "[$mydate $mytime]===============terminalProject Started============" >>log.txt
else
	echo "terminalProject 已启动"
fi
