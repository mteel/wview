#!/bin/bash
################################################################################
# wviewftp.debug.sh
#
# Author:  Oscar Barlow
# Date:	   Dec 16, 2008
#
# Description:  This shell script logs the parameters sent to the ftp process
#     by wview.  It logs the ftp command that wview would use to ftp your files.
#     After logging the information, it attempts to execute the ftp process.
# This script appends to the log file so each time the ftp process is called by 
# wview, the information is logged.  To debug the ftp process, copy the ftp 
# command from the log file to a shell prompt and see what errors you get.
#
# To use the script, follow these steps:
#
# 1.  create a directory under root called wview
# 2.  move this file to that directory.  Make sure the file is executable
#         chmod +x wviewftp.debug.sh
# 3.  modify the LOGFILE and wviewVAR variables below:
#
LOGFILE=/root/wview/wview_ftp_debug.txt
wviewVAR=/usr/local/var/wview
wviewFTP_BINARY=/usr/bin/tnftp
#
# 4.  modify your wviewftp.conf file and have wview execute this script for 
#     the ftp process.  Example below:
#         ftp_binary /root/wview/wviewftp.debug.sh 
#
# After you have resolved the issue, you can change the myDEBUG variable to 
# false and the script won't log the next time wview calls the script.
#
myDEBUG=True
#
################################################################################
#
#
if [[ $myDEBUG = "True" ]]
then
    echo start $(date) >> ${LOGFILE}
    pwd >> ${LOGFILE}
    echo parm1=$1 >> ${LOGFILE}
    echo parm2=$2 >> ${LOGFILE}
    echo parm3=$3 >> ${LOGFILE}
    echo parm4=$4 >> ${LOGFILE}
    echo parm5=$5 >> ${LOGFILE}
    echo parm6=$6 >> ${LOGFILE}
    echo parm7=$7 >> ${LOGFILE}
    echo parm8=$8 >> ${LOGFILE}
    echo parm9=$9 >> ${LOGFILE}
    echo parm10=${10} >> ${LOGFILE}
    echo parm11=${11} >> ${LOGFILE}
    echo parm12=${12} >> ${LOGFILE}
    echo parm13=${13} >> ${LOGFILE}
    echo parm14=${14} >> ${LOGFILE}
    echo parm15=${15} >> ${LOGFILE}
    echo FTP COMMAND BELOW
    echo "${wviewFTP_BINARY} ${1} $2 $3 $4 $5 $6 $7 $8 $9 ${10} ${11} ${12} ${13} $(14) ${15}" >> ${LOGFILE}
fi

cd ${wviewVAR}
${wviewFTP_BINARY} ${1} $2 $3 $4 $5 $6 $7 $8 $9 ${10} ${11} ${12} ${13} ${14} ${15}
cd 

if [[ $myDEBUG = "True" ]]
then
    echo end $(date) >> ${LOGFILE}
fi
