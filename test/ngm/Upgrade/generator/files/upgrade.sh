#!/bin/bash

#killall -s KILL ngm
cp -f ./ngm /home/linaro/ngm
cp -f ./Configuration.xml /home/linaro/Configuration.xml
cp -f ./Configuration.xml /home/linaro/Configuration.xml.bak
cp -f ./download.sh /home/linaro/upgrade/download.sh

mount /dev/mmcblk1p1 /boot
rm -f /boot/imx6q-var-dart.dtb
cp -f ./imx6q-var-dart.dtb /boot/
sync
umount /dev/mmcblk1p1

cp -f ./ifmetric /usr/sbin/
cp -f ./logrotate /usr/sbin/
cp -f ./logrotate.conf /etc/
mkdir -p /etc/logrotate.d/
cp -f ./rsyslog /etc/logrotate.d/
cp -f ./WebUI.tar.gz /home/linaro/
cd /home/linaro/ && tar -xf ./WebUI.tar.gz
sync

/usr/sbin/logrotate /etc/logrotate.conf

