#! /bin/bash
ps -A | grep restart.sh > /dev/null
if [[ $? = "1" ]]; then
/home/pi/bot/restart.sh > /dev/null
fi