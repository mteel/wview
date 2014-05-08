################################################################################
#
# File:           wviewcleardata.sh
#
# Description:    Provide a script to remove all weather data from the databases
#
# Usage:          (must be root)
#                 wviewcleardata
#
# History:
# Engineer	  Date	    Ver   Comments
# MS Teel	  01/24/10   1    Original
#
################################################################################


################################################################################
#################################  M A C R O S  ################################
################################################################################
WVIEWD_PID=$WVIEW_DATA_DIR/wviewd.pid

################################################################################
#######################  D E F I N E  F U N C T I O N S  #######################
################################################################################

show_usage()
{
    echo ""
    echo "wviewcleardata"
    echo "    removes all weather data from the databases"
    echo ""
}


interactive_intro()
{
    echo "################################################################################"
    echo " !!!!!!!!!!!!!!!!          READ THIS BEFORE PROCEEDING         !!!!!!!!!!!!!!!!"
    echo ""
    echo "--> wview database purging utility"
    echo ""
    echo "--> Note: This script will REMOVE all archive records and DELETE the following"
    echo "          databases:"
    echo "          wview-hilow.sdb (HILOW data)"
    echo "          wview-history.sdb (daily history)"
    echo "          wview-noaa.sdb (NOAA data)"
    echo ""
    echo "--> Use this script to reset all data - i.e., remove simulator data to begin"
    echo "    using your real station data."
    echo ""
    echo "--> It is highly recommended that you BACKUP YOUR ARCHIVE DIRECTORY before doing"
    echo "    this unless YOU ARE SURE you want to purge all weather data - it will not be"
    echo "    recoverable after this action!"
    echo ""
    echo "    IF THAT IS NOT WHAT YOU WANT, HIT CTRL-C NOW TO ABORT THIS SCRIPT!"
    echo ""
    echo "################################################################################"
    echo ""
    echo -n "Pausing 30 seconds - CTRL-C to ABORT"
    for i in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30
    do
        sleep 1
        echo -n "."
    done
    echo ""
    echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" 
    echo "Are you sure you want to DESTROY all of your collected weather data?"
    echo -n "(NO/yes): "
    read INVAL
    if [ "yes" != "$INVAL" ]; then
        echo "Aborting database purge operation."
        exit 0
    else
        echo "Purging all weather data!"
    fi
}


purge_archive_data()
{
	echo "delete from archive where dateTime >= '0';" >> $WVIEW_DATA_DIR/commands.sql
	echo ".read $WVIEW_DATA_DIR/commands.sql" | sqlite3 $WVIEW_DATA_DIR/archive/wview-archive.sdb
	rm -rf $WVIEW_DATA_DIR/commands.sql
}


################################################################################
##################  S C R I P T  E X E C U T I O N  S T A R T  #################
################################################################################

# First test to make sure that wview is not running:
if [ -f $WVIEWD_PID ]; then
    echo "wviewd is running - stop wview before running this script..."
    exit 3
fi

# Make sure that the archive DB is there:
if [ ! -f $WVIEW_DATA_DIR/archive/wview-archive.sdb ]; then
    echo "wview archive database NOT FOUND"
    exit 4
fi

## Intro:
interactive_intro

## Purge all archive records:
echo "Deleting all archive records:"
purge_archive_data

## Delete dependent databases:
echo "Deleting auto-generated databases:"
rm -rf $WVIEW_DATA_DIR/archive/wview-hilow.sdb
rm -rf $WVIEW_DATA_DIR/archive/wview-history.sdb
rm -rf $WVIEW_DATA_DIR/archive/wview-noaa.sdb

echo ""
echo "################################################################################"
echo "--> ALL wview weather data purged!"
echo "################################################################################"

exit 0

