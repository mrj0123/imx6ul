#!/bin/bash
mydate="`date +%Y-%m-%d`" 
mytime="`date +%H:%M:%S`"
sh killprocess.sh
#./AF_Unix_Wayland -platform wayland 

echo "[$mydate $mytime] NKTY Pos Program Started"  >>log.txt

./terminalProject -platform wayland>>terminalProject.txt &  
echo "[$mydate $mytime]===============terminalProject Started=============" >>log.txt

./Pos320Main >>posmainlog.txt &
echo "[$mydate $mytime]===============Pos320Main Started=============" >>log.txt
./secscreen >>secscreen.txt &
echo "[$mydate $mytime]===============secscreen Started=============" >>log.txt
./scanqrser >>scanqr.txt &
echo "[$mydate $mytime]===============scanqrser Started=============" >>log.txt
./consumeser >>consumelog.txt &
echo "[$mydate $mytime]===============ConsumeSer Started=============" >>log.txt


