#!/bin/bash


mydate="`date +%Y-%m-%d`" 

mytime="`date +%H:%M:%S`"

sh ./killprocess.sh

echo "[$mydate $mytime] NKTY Pos Program Started"  >>log.txt
./Pos320Main >>posmainlog.txt &
echo "[$mydate $mytime]===============Pos320Main Started=============" >>log.txt
sleep 1
./secscreen >>secscreen.txt &
echo "[$mydate $mytime]===============secscreen Started=============" >>log.txt
sleep 1
./scanqrser >>scanqr.txt &
echo "[$mydate $mytime]===============scanqrser Started=============" >>log.txt
sleep 1
./consumeser >>consumelog.txt &
echo "[$mydate $mytime]===============ConsumeSer Started=============" >>log.txt
sleep 1
./terminalProject -platform wayland>>terminalProject.txt &  
echo "[$mydate $mytime]===============terminalProject Started=============" >>log.txt


