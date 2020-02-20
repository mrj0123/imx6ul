#!/bin/bash
  	              
        flag1=$(ps -ef|grep -v grep|grep "Pos320Main" -c)
	if [ $flag1 -gt 0 ]; then
	#echo --------------exists,kill process----------------
	ps -ef|grep -v  grep|grep "Pos320Main"  |cut -c 9-15 |xargs kill -9
	#kill process finish
	sleep 1
	 
	fi
	#============================kill consumeser================================
	flag2=$(ps -ef|grep -v grep|grep "consumeser" -c)
	if [ $flag2 -gt 0 ]; then
        	#echo --------------exists,kill process----------------
        	ps -ef|grep -v  grep|grep "consumeser"  |cut -c 9-15 |xargs kill -9
        	#kill process finish
        	sleep 1
        	#echo sleep 3
	fi
	#=============================kill scanqrser==================================
	flag3=$(ps -ef|grep -v grep|grep "scanqrser" -c)
        if [ $flag2 -gt 0 ]; then
                #echo --------------exists,kill process----------------
                ps -ef|grep -v  grep|grep "scanqrser"  |cut -c 9-15 |xargs kill -9
                #kill process finish                                   
                sleep 1                                               
                #echo sleep 3                                          
        fi  
	#=============================kill secscreen==================================
        flag3=$(ps -ef|grep -v grep|grep "secscreen" -c)                                          
        if [ $flag3 -gt 0 ]; then                        
                #echo --------------exists,kill process----------------
                ps -ef|grep -v  grep|grep "secscreen"  |cut -c 9-15 |xargs kill -9
                #kill process finish                                               
                sleep 1                                                
                #echo sleep 3
        fi
	                           
                           
	#=============================kill terminalProject==================================
        flag5=$(ps -ef|grep -v grep|grep "terminalProject" -c)                              
        if [ $flag5 -gt 0 ]; then                                                      
                #echo --------------exists,kill process----------------                           
                ps -ef|grep -v  grep|grep "terminalProject"  |cut -c 9-15 |xargs kill -9
                #kill process finish                                               
                sleep 1                                                            
                #echo sleep 3                                                      
        fi  

