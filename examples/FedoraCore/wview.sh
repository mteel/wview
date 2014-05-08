# chkconfig: - 89 11
# description: wview is a unix weather program
#
# wview          Start/Stop the wview daemons
#
# processnames: wviewd htmlgend wviewftpd wvalarmd wvcwopd wviewsshd wvhttpd
# config: $prefix/etc/wview
# pidfiles: $prefix/var/wview/*.pid

# Source function library:
. /etc/init.d/functions

WVIEWD_FILE=`cat $CONF_DIRECTORY/wview-binary`
WVIEWD_BIN=$WVIEW_INSTALL_DIR/$WVIEWD_FILE
test -x $WVIEWD_BIN || exit 5

HTMLD_BIN=$WVIEW_INSTALL_DIR/htmlgend
test -x $HTMLD_BIN || exit 6

FTPD_BIN=$WVIEW_INSTALL_DIR/wviewftpd
test -x $FTPD_BIN || exit 7

SSHD_BIN=$WVIEW_INSTALL_DIR/wviewsshd
test -x $SSHD_BIN || exit 7

ALARMD_BIN=$WVIEW_INSTALL_DIR/wvalarmd
test -x $ALARMD_BIN || exit 8

CWOPD_BIN=$WVIEW_INSTALL_DIR/wvcwopd
test -x $CWOPD_BIN || exit 9

HTTP_BIN=$WVIEW_INSTALL_DIR/wvhttpd

SQLD_BIN=$WVIEW_INSTALL_DIR/wviewsqld

RADROUTER_BIN=$WVIEW_INSTALL_DIR/radmrouted

PMOND_BIN=$WVIEW_INSTALL_DIR/wvpmond
test -x $PMOND_BIN || exit 10

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

wait_for_time_set() {
    THOUSAND=1000
    CURRVAL=`date +%s`
    while [ "$CURRVAL" -lt "$THOUSAND" ]; do
        sleep 1
        CURRVAL=`date +%s`
    done
}

kill_running_processes() {
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
	if [ -f $PMOND_PID ]; then
		echo "wvpmond pid file $PMOND_PID exists - killing existing process"
		kill -15 `cat $PMOND_PID`
		rm -f $PMOND_PID
	fi
}

case "$1" in
  start)
	kill_running_processes

	wait_for_time_set

	echo "Starting wview daemons:"

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
    ;;
  start-trace)
	kill_running_processes

	echo "Starting wview daemons (tracing to $RUN_DIRECTORY):"
	echo "Warning: traced processes run very slowly and may effect performance."

	if [ -x $RADROUTER_BIN ]; then
	    $RADROUTER_BIN 1 $RUN_DIRECTORY
	else
	    echo "Cannot find $RADROUTER_BIN - exiting!"
	    exit 10
	fi
	sleep 1
	strace -o $RUN_DIRECTORY/$WVIEWD_FILE.trace $WVIEWD_BIN -f &> /dev/null &
	sleep 1
	strace -o $RUN_DIRECTORY/htmlgend.trace $HTMLD_BIN -f &> /dev/null &
	strace -o $RUN_DIRECTORY/wvalarmd.trace $ALARMD_BIN -f &> /dev/null &
	strace -o $RUN_DIRECTORY/wvcwopd.trace $CWOPD_BIN -f &> /dev/null &
	strace -o $RUN_DIRECTORY/wvhttpd.trace $HTTP_BIN -f &> /dev/null &
	strace -o $RUN_DIRECTORY/wviewftpd.trace $FTPD_BIN -f &> /dev/null &
	strace -o $RUN_DIRECTORY/wviewsshd.trace $SSHD_BIN -f &> /dev/null &
	strace -o $RUN_DIRECTORY/wvpmond.trace $PMOND_BIN -f &> /dev/null &
	;;
  stop)
	echo "Shutting down wview daemons..."
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
    ;;
  restart)
	$0 stop  && sleep 2
	$0 start
    ;;
  *)
	echo "Usage: $0 {start|start-trace|stop|restart}"
	exit 1
esac

exit 0

