#!/bin/sh

##  Notes:
## - nobody@nobody.org should be changed to the intended recipient address
## - make sure the mail binary is in the root's path or add the full path

echo "`date`:OutTempMax: threshold $2, trigger $3" | mail -s "`date`: $1 Alarm" nobody@nobody.org

