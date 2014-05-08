#!/bin/sh

mkdir -p /usr/local/var/wview/alarms
echo "`date`:OutTempMin: type $1, threshold $2, trigger $3" >> /usr/local/var/wview/alarms/alarm.log

