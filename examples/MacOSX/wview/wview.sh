##
# wview daemons for MacOSX
##

. /etc/rc.common


# fully qualified program names
WVIEWD_FILE=`cat $CONF_DIRECTORY/wview-binary`
WVIEWD_BIN=$WVIEW_INSTALL_DIR/$WVIEWD_FILE
HTMLD_BIN=$WVIEW_INSTALL_DIR/htmlgend
FTPD_BIN=$WVIEW_INSTALL_DIR/wviewftpd
SSHD_BIN=$WVIEW_INSTALL_DIR/wviewsshd
ALARMD_BIN=$WVIEW_INSTALL_DIR/wvalarmd
CWOPD_BIN=$WVIEW_INSTALL_DIR/wvcwopd
HTTP_BIN=$WVIEW_INSTALL_DIR/wvhttpd
SQLD_BIN=$WVIEW_INSTALL_DIR/wviewsqld
RADROUTER_BIN=$WVIEW_INSTALL_DIR/radmrouted
PMOND_BIN=$WVIEW_INSTALL_DIR/wvpmond

# pid names
WVIEWD_PID=$RUN_DIRECTORY/wviewd.pid
HTMLD_PID=$RUN_DIRECTORY/htmlgend.pid
FTPD_PID=$RUN_DIRECTORY/wviewftpd.pid
SSHD_PID=$RUN_DIRECTORY/wviewsshd.pid
ALARMD_PID=$RUN_DIRECTORY/wvalarmd.pid
CWOPD_PID=$RUN_DIRECTORY/wvcwopd.pid
HTTP_PID=$RUN_DIRECTORY/wvhttpd.pid
SQLD_PID=$RUN_DIRECTORY/wviewsqld.pid
RADROUTER_PID=$RUN_DIRECTORY/radmrouted.pid
PMOND_PID=$RUN_DIRECTORY/wvpmond.pid

StartService ()
{
	ConsoleMessage "Starting wview daemons"
	if [ -f $RADROUTER_PID ]; then
		echo "radlib router pid file $RADROUTER_PID exists - killing existing process"
		kill -15 `cat $RADROUTER_PID`
		rm -f $RADROUTER_PID
	fi
	if [ -f $WVIEWD_PID ]; then
		echo "wviewd pid file $WVIEWD_PID exists - killing existing process"
		kill -15 `cat $WVIEWD_PID`
		rm -f $WVIEWD_PID
	fi
	if [ -f $HTMLD_PID ]; then
		echo "htmlgend pid file $HTMLD_PID exists - killing existing process"
		kill -15 `cat $HTMLD_PID`
		rm -f $HTMLD_PID
	fi
	if [ -f $FTPD_PID ]; then
		echo "wviewftpd pid file $FTPD_PID exists - killing existing process"
		kill -15 `cat $FTPD_PID`
		rm -f $FTPD_PID
	fi
	if [ -f $SSHD_PID ]; then
		echo "wviewsshd pid file $SSHD_PID exists - killing existing process"
		kill -15 `cat $SSHD_PID`
		rm -f $SSHD_PID
	fi
	if [ -f $ALARMD_PID ]; then
		echo "wvalarmd pid file $ALARMD_PID exists - killing existing process"
		kill -15 `cat $ALARMD_PID`
		rm -f $ALARMD_PID
	fi
	if [ -f $CWOPD_PID ]; then
		echo "wvcwopd pid file $CWOPD_PID exists - killing existing process"
		kill -15 `cat $CWOPD_PID`
		rm -f $CWOPD_PID
	fi
	if [ -f $HTTP_PID ]; then
		echo "wvhttpd pid file $HTTP_PID exists - killing existing process"
		kill -15 `cat $HTTP_PID`
		rm -f $HTTP_PID
	fi
	if [ -f $SQLD_PID ]; then
		echo "wviewsqld pid file $SQLD_PID exists - killing existing process"
		kill -15 `cat $SQLD_PID`
		rm -f $SQLD_PID
	fi
	if [ -f $PMOND_PID ]; then
		echo "wvpmond pid file $PMOND_PID exists - killing existing process"
		kill -15 `cat $PMOND_PID`
		rm -f $PMOND_PID
	fi

	if [ -x $RADROUTER_BIN ]; then
	    $RADROUTER_BIN 1 $RUN_DIRECTORY
	else
	    echo "Cannot find $RADROUTER_BIN - exiting!"
	    exit 10
	fi
	sleep 1
	$WVIEWD_BIN
	sleep 1
	$HTMLD_BIN
	$ALARMD_BIN
	$CWOPD_BIN
	$HTTP_BIN
	$FTPD_BIN
	$SSHD_BIN
    $PMOND_BIN

	ConsoleMessage -S
}

StopService ()
{
	ConsoleMessage "Stopping wview daemons"
	if [ -f $PMOND_PID ]; then
	    kill -15 `cat $PMOND_PID`
	fi
	if [ -f $HTTP_PID ]; then
	    kill -15 `cat $HTTP_PID`
	fi
	if [ -f $CWOPD_PID ]; then
		kill -15 `cat $CWOPD_PID`
	fi
	if [ -f $ALARMD_PID ]; then
		kill -15 `cat $ALARMD_PID`
	fi
	if [ -f $SSHD_PID ]; then
		kill -15 `cat $SSHD_PID`
	fi
	if [ -f $FTPD_PID ]; then
		kill -15 `cat $FTPD_PID`
	fi
	if [ -f $HTMLD_PID ]; then
		kill -15 `cat $HTMLD_PID`
	fi
	if [ -f $WVIEWD_PID ]; then
	    kill -15 `cat $WVIEWD_PID`
	fi
	sleep 1
	if [ -f $RADROUTER_PID ]; then
	    kill -15 `cat $RADROUTER_PID`
	fi

	ConsoleMessage -S
}

RestartService ()
{
	StopService
	sleep 2
	StartService
}

RunService "$1" 
