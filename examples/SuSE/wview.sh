#
# /etc/init.d/wview
#
# Start/Stop the wview daemons
#
# processnames: wviewd htmlgend wviewftpd wvalarmd wvcwopd wviewsshd wvhttpd
#
# SuSE control script
#


# load the configuration
#
test -s /etc/rc.status && \
    . /etc/rc.status

WVIEWD_FILE=`cat $CONF_DIRECTORY/wview-binary`
WVIEWD_BIN=$WVIEW_INSTALL_DIR/$WVIEWD_FILE
test -x $WVIEWD_BIN || exit 3

HTMLD_BIN=$WVIEW_INSTALL_DIR/htmlgend
test -x $HTMLD_BIN || exit 4

FTPD_BIN=$WVIEW_INSTALL_DIR/wviewftpd
test -x $FTPD_BIN || exit 5

SSHD_BIN=$WVIEW_INSTALL_DIR/wviewsshd
test -x $SSHD_BIN || exit 6

ALARMD_BIN=$WVIEW_INSTALL_DIR/wvalarmd
test -x $ALARMD_BIN || exit 7

CWOPD_BIN=$WVIEW_INSTALL_DIR/wvcwopd
test -x $CWOPD_BIN || exit 8

HTTP_BIN=$WVIEW_INSTALL_DIR/wvhttpd

SQLD_BIN=$WVIEW_INSTALL_DIR/wviewsqld

RADROUTER_BIN=$WVIEW_INSTALL_DIR/radmrouted

PMOND_BIN=$WVIEW_INSTALL_DIR/wvpmond
test -x $PMOND_BIN || exit 10

rc_reset

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

#
# The echo return value for success (defined in /etc/rc.status).
#
return=$rc_done

wait_for_time_set() {
    THOUSAND=1000
    CURRVAL=`date +%s`
    while [ "$CURRVAL" -lt "$THOUSAND" ]; do
        sleep 1
        CURRVAL=`date +%s`
    done
}


#
# main part
#
case "$1" in
    start)
	echo "Starting wview daemons:"
	if [ -f $RADROUTER_PID ]; then
		echo "radlib router pid file $RADROUTER_PID exists - killing existing process"
		/sbin/killproc -G -TERM $RADROUTER_BIN
		rm -f $RADROUTER_PID
	fi
	if [ -f $WVIEWD_PID ]; then
		echo "wviewd pid file $WVIEWD_PID exists - killing existing process"
		/sbin/killproc -G -TERM $WVIEWD_BIN
		rm -f $WVIEWD_PID
	fi
	if [ -f $HTMLD_PID ]; then
		echo "htmlgend pid file $HTMLD_PID exists - killing existing process"
		/sbin/killproc -G -TERM $HTMLD_BIN
		rm -f $HTMLD_PID
	fi
	if [ -f $FTPD_PID ]; then
		echo "wviewftpd pid file $FTPD_PID exists - killing existing process"
		/sbin/killproc -G -TERM $FTPD_BIN
		rm -f $FTPD_PID
	fi
	if [ -f $SSHD_PID ]; then
		echo "wviewsshd pid file $SSHD_PID exists - killing existing process"
		/sbin/killproc -G -TERM $SSHD_BIN
		rm -f $SSHD_PID
	fi
	if [ -f $ALARMD_PID ]; then
		echo "wvalarmd pid file $ALARMD_PID exists - killing existing process"
		/sbin/killproc -G -TERM $ALARMD_BIN
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
	if [ -f $PMOND_PID ]; then
		echo "wvpmond pid file $PMOND_PID exists - killing existing process"
		kill -15 `cat $PMOND_PID`
		rm -f $PMOND_PID
	fi

	wait_for_time_set

	/sbin/startproc -v -t 1 $RADROUTER_BIN 1 $RUN_DIRECTORY
	ret=$?
	if test $ret != 0; then
		echo "$RADROUTER_BIN failed"
		rm -f $RADROUTER_PID
		rc_failed $ret
		rc_status -v
		rc_exit
	fi

	/sbin/startproc -v -t 1 $WVIEWD_BIN
	ret=$?
	if test $ret != 0; then
		echo "$WVIEWD_BIN failed"
		rm -f $WVIEWD_PID
		rc_failed $ret
		rc_status -v
		rc_exit
	fi

	/sbin/startproc -v -t 1 $HTMLD_BIN
	/sbin/startproc -v $ALARMD_BIN
	/sbin/startproc -v $CWOPD_BIN
    /sbin/startproc -v $HTTP_BIN
	/sbin/startproc -v $FTPD_BIN
	/sbin/startproc -v $SSHD_BIN
    /sbin/startproc -v $PMOND_BIN

	rc_failed $ret
	rc_status -v
    ;;

    stop)
	echo -n "Shutting down wview daemons"
	/sbin/killproc -p $PMOND_PID -TERM $PMOND_BIN
    /sbin/killproc -p $HTTP_PID -TERM $HTTP_BIN
	/sbin/killproc -p $CWOPD_PID -TERM $CWOPD_BIN
	/sbin/killproc -p $ALARMD_PID -TERM $ALARMD_BIN
	/sbin/killproc -p $SSHD_PID -TERM $SSHD_BIN
	/sbin/killproc -p $FTPD_PID -TERM $FTPD_BIN
	/sbin/killproc -p $HTMLD_PID -TERM $HTMLD_BIN
	/sbin/killproc -p $WVIEWD_PID -TERM $WVIEWD_BIN
	sleep 1
	/sbin/killproc -p $RADROUTER_PID -TERM $RADROUTER_BIN
	rc_status -v
    ;;

    restart)
	$0 stop  && sleep 2
	$0 start
	# Remember status and be quiet
        rc_status
    ;;

    status)
        echo "Checking for wview daemons:"
	/sbin/checkproc $HTTP_BIN
	if test $? != 0; then
		echo WunderGround daemon not running...
	else
		echo WunderGround daemon running...
	fi
	/sbin/checkproc $CWOPD_BIN
	if test $? != 0; then
		echo CWOP daemon not running...
	else
		echo CWOP daemon running...
	fi
	/sbin/checkproc $ALARMD_BIN
	if test $? != 0; then
		echo alarm daemon not running...
	else
		echo alarm daemon running...
	fi
	/sbin/checkproc $SSHD_BIN
	if test $? != 0; then
		echo ssh daemon not running...
	else
		echo ssh daemon running...
	fi
	/sbin/checkproc $FTPD_BIN
	if test $? != 0; then
		echo FTP daemon not running...
	else
		echo FTP daemon running...
	fi
	/sbin/checkproc $HTMLD_BIN
	if test $? != 0; then
		echo HTML daemon not running...
	else
		echo HTML daemon running...
	fi
	/sbin/checkproc $WVIEWD_BIN
	if test $? != 0; then
		echo station daemon not running...
	else
		echo station daemon running...
	fi
	/sbin/checkproc $RADROUTER_BIN
	if test $? != 0; then
		echo radlib msg router daemon not running...
	else
		echo radlib msg router daemon running...
	fi
	rc_status
    ;;

    *)
	echo "Usage: $0 {start|stop|status|restart}"
	exit 1
esac


# Inform the caller verbosely and set an exit status.
rc_exit

