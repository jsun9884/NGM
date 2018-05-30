#!/bin/bash

sleep 10
hciconfig hci0 piscan
cd /home/linaro/WebUI/server
while :
do
	if [ -n `pidof nodejs` ]; then
		killall nodejs
	fi
	nodejs app.js > /dev/null 2> /dev/null
done

