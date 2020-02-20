#!/bin/bash

echo starting download.....

rm -rf /usr/local/nkty/update/Pos320Main
rm -rf /usr/local/nkty/update/scanqrser
rm -rf /usr/local/nkty/update/terminalProject
rm -rf /usr/local/nkty/update/consumeser
rm -rf /usr/local/nkty/update/secscreen

for line in `cat /usr/local/nkty/script/update.conf` 
do                           
 echo '--------------'
done



wget -O /usr/local/nkty/update/Pos320Main $line/Pos320Main
wget -O /usr/local/nkty/update/consumeser $line/consumeser
wget -O /usr/local/nkty/update/terminalProject $line/terminalProject
wget -O /usr/local/nkty/update/scanqrser $line/scanqrser
wget -O /usr/local/nkty/update/secscreen $line/secscreen

 
echo download finished
#echo stoping Main Process.....¨‹

	#=============================kill Pos320Main===============================
	flag1=$(ps -ef|grep -v grep|grep "Pos320Main" -c)                                                           
        if [ $flag1 -gt 0 ]; then                                                                                   
        #echo --------------exists,kill process----------------                                                     
        ps -ef|grep -v  grep|grep "Pos320Main"  |cut -c 9-15 |xargs kill -9                                         
        #kill process finish                                                                                        
        sleep i                                                                                                     
                                                                                                                    
        fi                                                                                                          
        #============================kill consumeser================================                                
        flag2=$(ps -ef|grep -v grep|grep "consumeser" -c)                                                           
        if [ $flag2 -gt 0 ]; then                                                                                   
                #echo --------------exists,kill process----------------                                             
                ps -ef|grep -v  grep|grep "consumeser"  |cut -c 9-15 |xargs kill -9                                 
                #kill process finish                                                                                
                sleep 3                                                                                             
                #echo sleep 3                                                                                       
        fi
	sleep 1                                                                                                          
        #=============================kill scanqrser==================================                              
        flag3=$(ps -ef|grep -v grep|grep "scanqrser" -c)                                                            
        if [ $flag2 -gt 0 ]; then                                                                                   
                #echo --------------exists,kill process----------------                                             
                ps -ef|grep -v  grep|grep "scanqrser"  |cut -c 9-15 |xargs kill -9                                  
                #kill process finish                                                                                
                sleep 3                                                                                             
                #echo sleep 3                                                                                       
        fi     
	sleep 1                                                                                                     
        #=============================kill secscreen==================================                              
        flag3=$(ps -ef|grep -v grep|grep "secscreen" -c)                                                            
        if [ $flag3 -gt 0 ]; then                                                                                   
                #echo --------------exists,kill process----------------                                             
                ps -ef|grep -v  grep|grep "secscreen"  |cut -c 9-15 |xargs kill -9                                  
                #kill process finish                                                                                
                sleep 3                                                                                             
                #echo sleep 3                                                                                       
        fi     
	sleep 1                                                                                                     
        #=============================kill consumeser==================================                             
        flag4=$(ps -ef|grep -v grep|grep "consumeser" -c)                                                           
        if [ $flag4 -gt 0 ]; then                                                                                   
                #echo --------------exists,kill process----------------                                             
                ps -ef|grep -v  grep|grep "consumeser"  |cut -c 9-15 |xargs kill -9                                 
                #kill process finish                                                                                
                sleep 3                                                                                             
                #echo sleep 3                                                                                       
        fi     
	sleep 1                                                        
	#=============================kill terminalProject==================================                        
        flag5=$(ps -ef|grep -v grep|grep "terminalProject" -c)                                                      
        if [ $flag5 -gt 0 ]; then                                                                                   
                #echo --------------exists,kill process----------------                                             
                ps -ef|grep -v  grep|grep "terminalProject"  |cut -c 9-15 |xargs kill -9                            
                #kill process finish                                                                                
                sleep 3                                                                                             
                #echo sleep 3                                                                                       
        fi     
	sleep 1
	
#======================replace Pos320Main====================================
cp /usr/local/nkty/update/Pos320Main /usr/local/nkty/Pos320Main
chmod 777 /usr/local/nkty/Pos320Main

cp /usrl/local/nkty/update/scanqrser /usr/local/nkty/scanqrser
chmod 777 /usr/local/nkty/scanqrser  

cp /usr/local/nkty/update/terminalProject /usr/local/nkty/terminalProject
chmod 777 /usr/local/nkty/terminalProject

cp /usr/local/nkty/update/consumeser /usr/local/nkty/consumeser
chmod 777 /usr/local/nkty/consumeser

cp /usr/local/nkty/update/secscreen /usr/local/nkty/secscreen
chmod 777 /usr/local/nkty/secscreen

echo Main process update finished
#echo starting Main Process

#echo Main Process Started......
sh /usr/local/nkty/script/startup.sh

