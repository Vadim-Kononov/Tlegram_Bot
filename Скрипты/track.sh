#!/bin/bash
while 1>0
do
var_1=`wc -c /home/pi/bot/log.txt | awk '{print $1}'`
sleep 10
var_2=`wc -c /home/pi/bot/log.txt | awk '{print $1}'`
#echo "$var_1"
#echo "$var_2"
if [ "$var_1" = "$var_2" ]; then
pkill bot
pkill bot
pkill bot
/home/pi/bot/bot > /home/pi/bot/log.txt
fi
done