#! /bin/bash
### BEGIN INIT INFO
# Provides: scriptName
# Required-Start: $all
# Required-Stop: $all
# Default-Start: 2 3 4 5
# Default-Stop: 0 1 6
# Short-Description: Example my init script
# Description: This file should be used to construct scripts to be placed in /etc/init.d
### END INIT INFO
#wc -c /home/pi/bot/log.txt
#wc -c /home/pi/bot/log.txt | awk '{print $1}'
sudo /home/pi/bot/bot > /home/pi/bot/log.txt
sleep 5
while 1>0
do
ps -A | grep bot > /dev/null
if [[ $? = "1" ]]; then
sudo pkill bot
sudo pkill bot
sudo /home/pi/bot/bot > /home/pi/bot/log.txt
fi
sleep 10
done