#!/bin/bash
/home/pi/bot/bot >> /home/pi/bot/log.log
while 1>0
do
var_1=`wc -c /home/pi/bot/log.log | awk '{print $1}'`
sleep 30
var_2=`wc -c /home/pi/bot/log.log | awk '{print $1}'`
#echo "$var_1"
#echo "$var_2"
if [ "$var_1" = "$var_2" ]; then
pkill bot
/home/pi/bot/bot >> /home/pi/bot/log.log
fi
done