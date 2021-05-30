#!/bin/bash
if [ -f "/boot/led" ];then
cp /boot/led /home/
chmod +x /home/led
rm /boot/led
sleep 3 
echo "文件存在"
else
echo "文件不存在"
fi
if [ -f "/boot/2424.bdf" ];then
cp /boot/2424.bdf /home/
chmod +x /home/led
rm /boot/2424.bdf
sleep 3 
echo "文件存在"
else
echo "文件不存在"
fi
/home/led

service console