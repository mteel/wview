### BEGIN INIT INFO
# Provides:          wview
# Required-Start:    $local_fs $network $time $syslog
# Required-Stop:     $local_fs $network $time $syslog
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: Start wview daemons at boot time
# Description:       Start wview daemons at boot time.
### END INIT INFO
# config:            $prefix/etc/wview
# pidfiles:          $prefix/var/wview/*.pid
################################################################################

if [ -f $CONF_DIRECTORY/wview-user ]; then
  WVIEW_USER=`cat $CONF_DIRECTORY/wview-user`
fi
: ${WVIEW_USER:=root}

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

RADROUTER_BIN=$WVIEW_INSTALL_DIR/radmrouted

PMOND_BIN=$WVIEW_INSTALL_DIR/wvpmond
test -x $PMOND_BIN || exit 10

RADROUTER_PID=$RUN_DIRECTORY/radmrouted.pid
WVIEWD_PID=$RUN_DIRECTORY/wviewd.pid
HTMLD_PID=$RUN_DIRECTORY/htmlgend.pid
FTPD_PID=$RUN_DIRECTORY/wviewftpd.pid
SSHD_PID=$RUN_DIRECTORY/wviewsshd.pid
ALARMD_PID=$RUN_DIRECTORY/wvalarmd.pid
CWOPD_PID=$RUN_DIRECTORY/wvcwopd.pid
HTTP_PID=$RUN_DIRECTORY/wvhttpd.pid
PMOND_PID=$RUN_DIRECTORY/wvpmond.pid

wait_for_time_set() {
    THOUSAND=1000
    CURRVAL=`date +%s`
    while [ "$CURRVAL" -lt "$THOUSAND" ]; do
        sleep 1
        CURRVAL=`date +%s`
    done
}

cleanup_pid_files() {
    for pidfile in `ls -1 $RUN_DIRECTORY/*.pid 2>/dev/null`; do
        testpid=`cat $pidfile`;
        if [ -n "$testpid" ]; then
            result=`ps --no-headers -o pid $testpid`;
        else
            result=""
        fi;
        if [ -z "$result" ]; then
            echo "Removing stale PID file $pidfile";
            rm -f $pidfile;
        fi;
    done;
}

case "$1" in
  start)
    cleanup_pid_files
    wait_for_time_set

	echo "Starting wview daemons:"

	if [ -x $RADROUTER_BIN ]; then
		start-stop-daemon --start --oknodo --pidfile $RADROUTER_PID \
			--chuid $WVIEW_USER --exec $RADROUTER_BIN 1 $RUN_DIRECTORY

	else
	    echo "Cannot find $RADROUTER_BIN - exiting!"
	    exit 10
	fi
	sleep 1
	start-stop-daemon --start --oknodo --pidfile $WVIEWD_PID \
		--exec $WVIEWD_BIN --chuid $WVIEW_USER
	sleep 1
	start-stop-daemon --start --oknodo --pidfile $HTMLD_PID \
		--exec $HTMLD_BIN --chuid $WVIEW_USER
	start-stop-daemon --start --oknodo --pidfile $ALARMD_PID \
		--exec $ALARMD_BIN --chuid $WVIEW_USER
	start-stop-daemon --start --oknodo --pidfile $CWOPD_PID \
		--exec $CWOPD_BIN --chuid $WVIEW_USER
	start-stop-daemon --start --oknodo --pidfile $HTTP_PID \
		--exec $HTTP_BIN --chuid $WVIEW_USER
	start-stop-daemon --start --oknodo --pidfile $FTPD_PID \
		--exec $FTPD_BIN --chuid $WVIEW_USER
	start-stop-daemon --start --oknodo --pidfile $SSHD_PID \
		--exec $SSHD_BIN --chuid $WVIEW_USER
	start-stop-daemon --start --oknodo --pidfile $PMOND_PID \
		--exec $PMOND_BIN --chuid $WVIEW_USER
    ;;
  start-trace)
	echo "Starting wview daemons (tracing to $RUN_DIRECTORY):"
	echo "Warning: traced processes run very slowly and may effect performance."

	if [ -x $RADROUTER_BIN ]; then
		start-stop-daemon --start --oknodo --pidfile $RADROUTER_PID \
			--chuid $WVIEW_USER --exec $RADROUTER_BIN 1 $RUN_DIRECTORY

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
	start-stop-daemon --stop --oknodo --pidfile $PMOND_PID \
		--exec $PMOND_BIN --signal 15 --retry 5
	start-stop-daemon --stop --oknodo --pidfile $HTTP_PID \
		--exec $HTTP_BIN --signal 15 --retry 5
	start-stop-daemon --stop --oknodo --pidfile $CWOPD_PID \
		--exec $CWOPD_BIN --signal 15 --retry 5
	start-stop-daemon --stop --oknodo --pidfile $ALARMD_PID \
		--exec $ALARMD_BIN --signal 15 --retry 5
	start-stop-daemon --stop --oknodo --pidfile $SSHD_PID \
		--exec $SSHD_BIN --signal 15 --retry 5
	start-stop-daemon --stop --oknodo --pidfile $FTPD_PID \
		--exec $FTPD_BIN --signal 15 --retry 5
	start-stop-daemon --stop --oknodo --pidfile $HTMLD_PID \
		--exec $HTMLD_BIN --signal 15 --retry 5
	start-stop-daemon --stop --oknodo --pidfile $WVIEWD_PID \
		--exec $WVIEWD_BIN --signal 15 --retry 5
	start-stop-daemon --stop --oknodo --pidfile $RADROUTER_PID \
		--exec $RADROUTER_BIN --signal 15 --retry 5
    ;;
  restart)
	$0 stop  && sleep 2
	$0 start
    ;;
  force-reload)
	$0 stop  && sleep 2
	$0 start
    ;;
  status)
	ps aux | grep "wv"
	ps aux | grep "htmlgend"
    ;;
  *)
	echo "Usage: $0 {start|start-trace|stop|restart|force-reload|status}"
	exit 1
esac

exit 0

