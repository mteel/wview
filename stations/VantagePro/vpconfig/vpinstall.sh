###########################################################################
#
# File:           vpinstall.sh
#
# Description:    Provide a script to interactively configure a new
#                 Vantage Pro installation.
#
# Usage:          vpinstall
#
# History:
# Engineer	  Date	    Ver   Comments
# MS Teel	  04/14/05   1    Original
#
###########################################################################
#
#
################### MACROS ######################
#
SCRIPTNAME=vpinstall
SCRIPTNAME_VERSION="1.0.1"

WVIEWD_PID=/var/wview/wviewd.pid
VPCONFIG_BIN=$WVIEW_INSTALL_DIR/vpconfig
test -x $VPCONFIG_BIN || (echo "vpconfig not installed!"; exit 2)

# default values
DEVNAME=/dev/ttyS0
ARCINT=5
ELEVATION=0
LATITUDE=0
LONGITUDE=0
RAINSTART=1
RAINTODATE=0
ETTODATE=0

################## INCLUDE ######################
#

############### HELP FUNCTIONS ##################
#

#################### MAIN #######################
#

# First test to make sure wview is not running...
if [ -f $WVIEWD_PID ]; then
    echo "wviewd is running - stop wview before configuring the VP console"
    exit 3
fi

echo "################################################################################"
echo ""
echo "--> Installation Configuration for Vantage Pro(2) Console"
echo ""
echo "--> To reset historical highs and lows, use the Vantage Pro Console"
echo ""
echo "--> Note: values in parenthesis are used if you hit enter at a prompt..."
echo ""
echo "################################################################################"
echo ""

echo ""
echo "-------------------------------------------------------------"
echo "--NOTE-- ** Serial and USB data loggers are both considered serial -"
echo "--NOTE--    examples of this are: /dev/ttyS0, /dev/ttyUSB0, /dev/cuaa0, etc."
echo "--NOTE-- ** If your Vantage Pro is connected to a terminal or serial server,"
echo "--NOTE--    you should enter the host and port in the format host:port -"
echo "--NOTE--    examples of this are: 10.10.10.10:3001, xyplex1:2102, etc."
echo "Serial port or host:port the VP console is connected to (i.e. /dev/ttyS0, mss1:3001)"
echo -n "(/dev/ttyS0): "
read INVAL
if [ "" != "$INVAL" ]; then
    DEVNAME=$INVAL
fi

echo ""
echo "-------------------------------------------------------------" 
echo "Desired archive interval (minutes) (5,10,15,30,60)"
echo -n "(5): "
read INVAL
if [ "" != "$INVAL" ]; then
    ARCINT=$INVAL
fi

echo ""
echo "-------------------------------------------------------------" 
echo "Station elevation (feet) ((-2000) - (15000))"
echo -n "(0): " 
read INVAL
if [ "" != "$INVAL" ]; then
    ELEVATION=$INVAL
fi

echo ""
echo "-------------------------------------------------------------" 
echo "Station latitude (tenths of degrees) ((-900) - (900), (-) => South)"
echo -n "(0): "
read INVAL
if [ "" != "$INVAL" ]; then
    LATITUDE=$INVAL
fi

echo ""
echo "-------------------------------------------------------------" 
echo "Station longitude (tenths of degrees) ((-1799) - (1800), (-) => West)"
echo -n "(0): "
read INVAL
if [ "" != "$INVAL" ]; then
    LONGITUDE=$INVAL
fi

echo ""
echo "-------------------------------------------------------------" 
echo "Rain season start (month) (1 - 12)"
echo -n "(1): "
read INVAL
if [ "" != "$INVAL" ]; then
    RAINSTART=$INVAL
fi

echo ""
echo "-------------------------------------------------------------" 
echo "Year-to-Date Rain (inches) (0.00 - 200.00)"
echo -n "(0.00): "
read INVAL
if [ "" != "$INVAL" ]; then
    RAINTODATE=$INVAL
fi

echo ""
echo "-------------------------------------------------------------" 
echo "Year-to-Date ET (inches) (0.00 - 200.00)"
echo -n "(0.00): "
read INVAL
if [ "" != "$INVAL" ]; then
    ETTODATE=$INVAL
fi

echo ""
echo ""
echo "-------------------------------------------------------------" 
echo "Let's review your answers:"
echo ""
echo "VP Console Device:         $DEVNAME"
echo "Archive Interval:          $ARCINT (minutes)"
echo "Station Elevation:         $ELEVATION (feet)"
echo "Station Latitude:          $LATITUDE (tenths of a degree)"
echo "Station Longitude:         $LONGITUDE (tenths of a degree)"
echo "Rain Season Start Month:   $RAINSTART"
echo "Year-to-Date Rain:         $RAINTODATE"
echo "Year-to-Date ET:           $ETTODATE"
echo "-------------------------------------------------------------" 
echo ""
echo "WARNING: All archive records will be erased on the VP console!"
echo "Do you want to proceed with these values? (y/n)"
echo -n "(n): "
read INVAL
if [ "y" = "$INVAL" ]; then
    echo ""
    echo "Configuring with new values (takes some time, be patient) ..."
    echo ""
else
    echo ""
    echo "Aborting Vantage Pro Configuration ..."
    exit 4
fi

# OK, here we go ...

# 1) Set the elevation
$VPCONFIG_BIN $DEVNAME setelevation $ELEVATION
if [ "$?" != "0" ]; then
    echo "set elevation failed!"
    exit 7;
fi

sleep 1

# 2) Set the rain season start
$VPCONFIG_BIN $DEVNAME setrainseasonstart $RAINSTART
if [ "$?" != "0" ]; then
    echo "set rain season start failed!"
    exit 7
fi

sleep 1

# 3) Set the rain year-to-date
$VPCONFIG_BIN $DEVNAME setyeartodaterain $RAINTODATE
if [ "$?" != "0" ]; then
    echo "set year-to-date rain failed!"
    exit 7
fi

sleep 1

# 4) Set the ET year-to-date
$VPCONFIG_BIN $DEVNAME setyeartodateET $ETTODATE
if [ "$?" != "0" ]; then
    echo "set year-to-date ET failed!"
    exit 7
fi

sleep 1

# 5) Set the archive interval
$VPCONFIG_BIN $DEVNAME setinterval $ARCINT
if [ "$?" != "0" ]; then
    echo "set Archive Interval failed!"
    exit 7
fi

sleep 5

# 6) Set the LAT/LONG
$VPCONFIG_BIN $DEVNAME setlatlong $LATITUDE $LONGITUDE
if [ "$?" != "0" ]; then
    echo "set LAT/LONG failed!"
    exit 7
fi


echo ""
echo -n "Please wait - allowing VP console to digest new settings "
sleep 2
echo -n "."
sleep 2
echo -n "."
sleep 2
echo -n "."
sleep 2
echo -n "."
sleep 2
echo -n "."
echo ""
echo ""

$VPCONFIG_BIN $DEVNAME show

echo ""
echo "################################################################################"
echo "It is advisable to wait $ARCINT minutes before starting wview for the" 
echo "first time so the Vantage Pro can become consistent with the new settings..."
echo ""
echo "In the meantime, be sure you have deleted any archive files in"
echo "/var/wview/archive if you have changed the archive interval..."
echo ""
echo "Vantage Pro configuration complete!"
echo ""
echo "################################################################################"
exit 0

