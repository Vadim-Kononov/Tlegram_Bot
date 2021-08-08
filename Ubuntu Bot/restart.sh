#!/bin/bash
/home/kononov/bot/bot >> /home/kononov/bot/log.log
while 1>0
do
var_1=`wc -c /home/kononov/bot/log.log | awk '{print $1}'`
sleep 30
var_2=`wc -c /home/kononov/bot/log.log | awk '{print $1}'`
#echo "$var_1"
#echo "$var_2"
if [ "$var_1" = "$var_2" ]; then
pkill bot
/home/kononov/bot/bot >> /home/kononov/bot/log.log
fi
done