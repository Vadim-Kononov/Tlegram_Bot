#! /bin/bash
ps -A | grep track.sh > /dev/null
if [[ $? = "1" ]]; then
/home/pi/bot/track.sh
fi