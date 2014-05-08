#!/bin/sh

# Author: Jerry Fiddler

# a script to replace wvftpd, which I can't get to work reliably
# this runs automatically after htmlgend, if it's named /etc/wview/
post-generate.sh
#
# it looks in the wview img directory, and ftp's anything newer than
the last time the script was run

# These are parameters that are user-specific, and must be set for the
specific user

FTP_USER=XXXXXXX
FTP_PASS=XXXXXX
FTP_HOST_DIR=MyISP.com/public_html/wview/img/
FTP_ARGS=iV
#FTP_ARGS-iv

# These are probably fine, but can be changed if desired, or if images
are stored elsewhere

IMG_DIR=/var/wview/img
FTP_LAST_SENT_FILE=ftp-last-sent
FTP_ERROR_LOG=/var/wview/ftperrors

# Set VERBOSE to 1 to log all ftp transers to /var/log/messages.  0
turns logging off.

VERBOSE=0
LOG_TAG=wview_post-generate

#Nothing else should need to be changed

URL=ftp://$FTP_USER:$FTP_PASS@$FTP_HOST_DIR

cd $IMG_DIR

if [ -e $FTP_LAST_SENT_FILE ]
then
	FILES_TO_SEND=`find . -type f -newer $FTP_LAST_SENT_FILE -print`
else
	FILES_TO_SEND=`find . -type f -print`
	fi

if [ ! -z "$FILES_TO_SEND"  ]
then
	if [ $VERBOSE != 0 ]
		then
		logger -t $LOG_TAG ftp $FILES_TO_SEND
		fi

	# echo >> $FTP_ERROR_LOG $URL $FILES_TO_SEND

	ftp 2>> $FTP_ERROR_LOG -$FTP_ARGS -u $URL $FILES_TO_SEND
	RETVAL=$?

	if [ $RETVAL = 0 ]
	then
		cat /dev/null >$FTP_LAST_SENT_FILE	# to mark the last transfer
	else
		logger -t $LOG_TAG ftp returned error code $RETVAL!
		date >> $FTP_ERROR_LOG
	fi
else
	# no files to send
	if [ $VERBOSE != 0 ]
		then
		logger -t $LOG_TAG no files to send
		fi
fi

exit 0
