################################################################################
#
# File:           wview-pgsql-export.sh
#
# Description:    Provide a script to export SQLite3 wview data to a PostgreSQL 
#                 database. Should be run periodically via cron.
#
# Usage:          wview-pgsql-export
#                 Arguments:
#                   debug        - indicate to log verbosely and to
#                                  create the PostgreSQL debug log file 
#                                  $prefix/etc/wview/export/pgsql_import.log
#
# History:
# Engineer	  Date	    Ver   Comments
# MS Teel	  04/04/09   1    Original
#
# Notes:          The marker file approach is inspired by Jerry Fiddler.
#                 Requires: sqlite3 CLI utility and pgsql CLI utility.
#
################################################################################

################################################################################
#################################  M A C R O S  ################################
################################################################################

## DO NOT CHANGE ANY DEFS IN THIS BLOCK!

export PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

SECONDS_IN_DAY=86400
SECONDS_IN_HOUR=3600

ARCHIVE_DB_FILE=$WVIEW_DATA_DIR/archive/wview-archive.sdb
HILOW_DB_FILE=$WVIEW_DATA_DIR/archive/wview-hilow.sdb
NOAA_DB_FILE=$WVIEW_DATA_DIR/archive/wview-noaa.sdb

## The defaults match the SQLite table names: 
SQL_ARCHIVE_TABLE=archive
SQL_HILOW_TABLE_INTEMP=inTemp
SQL_HILOW_TABLE_OUTTEMP=outTemp
SQL_HILOW_TABLE_INHUMIDITY=inHumidity
SQL_HILOW_TABLE_OUTHUMIDITY=outHumidity
SQL_HILOW_TABLE_BP=baromPressure
SQL_HILOW_TABLE_WINDSPEED=windSpeed
SQL_HILOW_TABLE_WINDGUST=windGust
SQL_HILOW_TABLE_DEWPOINT=dewPoint
SQL_HILOW_TABLE_RAIN=rain
SQL_HILOW_TABLE_RAINRATE=rainRate
SQL_HILOW_TABLE_WINDCHILL=windChill
SQL_HILOW_TABLE_HEATINDEX=heatIndex
SQL_HILOW_TABLE_ET=ET
SQL_HILOW_TABLE_UV=UV
SQL_HILOW_TABLE_SOLARRAD=solarRadiation
SQL_HILOW_TABLE_HAIL=hail
SQL_HILOW_TABLE_HAILRATE=hailRate
SQL_HILOW_TABLE_WINDDIR=windDir
SQL_NOAA_TABLE=noaaHistory

TIME_MARKER_FILE=$WVIEW_CONF_DIR/export/pgsql_export_marker
ARCHIVE_MARKER_FILE=$WVIEW_CONF_DIR/export/archive_marker
HILOW_MARKER_FILE=$WVIEW_CONF_DIR/export/hilow_marker
NOAA_MARKER_FILE=$WVIEW_CONF_DIR/export/noaa_marker
ARCHIVE_EXPORT_FILE=$WVIEW_CONF_DIR/export/$SQL_ARCHIVE_TABLE.csv
HILOW_EXPORT_FILE_INTEMP=$WVIEW_CONF_DIR/export/$SQL_HILOW_TABLE_INTEMP.csv
HILOW_EXPORT_FILE_OUTTEMP=$WVIEW_CONF_DIR/export/$SQL_HILOW_TABLE_OUTTEMP.csv
HILOW_EXPORT_FILE_INHUMIDITY=$WVIEW_CONF_DIR/export/$SQL_HILOW_TABLE_INHUMIDITY.csv
HILOW_EXPORT_FILE_OUTHUMIDITY=$WVIEW_CONF_DIR/export/$SQL_HILOW_TABLE_OUTHUMIDITY.csv
HILOW_EXPORT_FILE_BP=$WVIEW_CONF_DIR/export/$SQL_HILOW_TABLE_BP.csv
HILOW_EXPORT_FILE_WINDSPEED=$WVIEW_CONF_DIR/export/$SQL_HILOW_TABLE_WINDSPEED.csv
HILOW_EXPORT_FILE_WINDGUST=$WVIEW_CONF_DIR/export/$SQL_HILOW_TABLE_WINDGUST.csv
HILOW_EXPORT_FILE_DEWPOINT=$WVIEW_CONF_DIR/export/$SQL_HILOW_TABLE_DEWPOINT.csv
HILOW_EXPORT_FILE_RAIN=$WVIEW_CONF_DIR/export/$SQL_HILOW_TABLE_RAIN.csv
HILOW_EXPORT_FILE_RAINRATE=$WVIEW_CONF_DIR/export/$SQL_HILOW_TABLE_RAINRATE.csv
HILOW_EXPORT_FILE_WINDCHILL=$WVIEW_CONF_DIR/export/$SQL_HILOW_TABLE_WINDCHILL.csv
HILOW_EXPORT_FILE_HEATINDEX=$WVIEW_CONF_DIR/export/$SQL_HILOW_TABLE_HEATINDEX.csv
HILOW_EXPORT_FILE_ET=$WVIEW_CONF_DIR/export/$SQL_HILOW_TABLE_ET.csv
HILOW_EXPORT_FILE_UV=$WVIEW_CONF_DIR/export/$SQL_HILOW_TABLE_UV.csv
HILOW_EXPORT_FILE_SOLARRAD=$WVIEW_CONF_DIR/export/$SQL_HILOW_TABLE_SOLARRAD.csv
HILOW_EXPORT_FILE_HAIL=$WVIEW_CONF_DIR/export/$SQL_HILOW_TABLE_HAIL.csv
HILOW_EXPORT_FILE_HAILRATE=$WVIEW_CONF_DIR/export/$SQL_HILOW_TABLE_HAILRATE.csv
HILOW_EXPORT_FILE_WINDDIR=$WVIEW_CONF_DIR/export/$SQL_HILOW_TABLE_WINDDIR.csv
NOAA_EXPORT_FILE=$WVIEW_CONF_DIR/export/$SQL_NOAA_TABLE.csv

ARCHIVE_ENABLE=1
HILOW_ENABLE=1
NOAA_ENABLE=1
UPDATE_MARKER=0

PGSQL_IMPORT_LOG_FILE=$WVIEW_CONF_DIR/export/pgsql_import.log

## Test command line arguments:
if [ "$1" = "help" ]; then
    echo "USAGE:"
    echo "wview-pgsql-export [help | debug]"
    echo "    <no arguments>                           - (default behavior) export SQLite3 data to PostgreSQL, typically executed as a cron job"
    echo ""
    echo "    help                                     - print out usage directions and exit"
    echo ""
    echo "    debug                                    - log verbosely and create the PostgreSQL debug log file $WVIEW_CONF_DIR/export/pgsql_import.log"
    echo ""
    echo ""
    echo "REQUIRES: sqlite3 CLI utility and pgsql CLI utility."
    echo ""
    exit 0
fi

if [ "$1" = "debug" ]; then
    VERBOSE=1
else
    VERBOSE=0
fi


################################################################################
#######################  D E F I N E  F U N C T I O N S  #######################
################################################################################
syslog()
{
	if [ $VERBOSE != 0 ]; then
        logger -t WV_EXPORT $1 $2
    fi
}

syslog_error()
{
    logger -t WV_EXPORT_ERROR $1 $2
}

# $1 = DB file (full path)
# $2 = table name
# $3 = dateTime stamp
# $4 = CSV output file
sqlite3_export()
{
    echo ".mode list" > /tmp/commands.sql
    echo ".separator ," >> /tmp/commands.sql
    echo ".nullvalue '\N'" >> /tmp/commands.sql
    echo ".timeout 30000" >> /tmp/commands.sql
    echo ".output $4" >> /tmp/commands.sql
    echo "select * from $2 where dateTime > '$3' order by dateTime ASC;" >> /tmp/commands.sql
    echo ".exit" >> /tmp/commands.sql

    echo ".read /tmp/commands.sql" | sqlite3 $1

    rm -rf /tmp/commands.sql
}

# No function args
get_sql_server_config()
{
    echo "select value from config where name='STATION_SQLDB_HOST';" > /tmp/commands.sql
    echo ".exit" >> /tmp/commands.sql
    echo ".read /tmp/commands.sql" | sqlite3 $WVIEW_CONF_DIR/wview-conf.sdb > /tmp/result.txt
    SQL_HOST=`cat /tmp/result.txt`
    rm -rf /tmp/commands.sql /tmp/result.txt

    echo "select value from config where name='STATION_SQLDB_DB_NAME';" > /tmp/commands.sql
    echo ".exit" >> /tmp/commands.sql
    echo ".read /tmp/commands.sql" | sqlite3 $WVIEW_CONF_DIR/wview-conf.sdb > /tmp/result.txt
    SQL_DB_NAME=`cat /tmp/result.txt`
    rm -rf /tmp/commands.sql /tmp/result.txt
}

# $1 = import table name
# $2 = import filename
pgsql_import()
{
    echo -n "\\" > /tmp/commands.sql
    echo "copy $1 from $2 delimiter as ','" >> /tmp/commands.sql

    if [ "$VERBOSE" = "1" ]; then
        date >> $PGSQL_IMPORT_LOG_FILE;
        psql --host $SQL_HOST $SQL_DB_NAME < /tmp/commands.sql > $PGSQL_IMPORT_LOG_FILE
    else
        psql --host $SQL_HOST $SQL_DB_NAME < /tmp/commands.sql > /dev/null;
    fi

    rm -rf /tmp/commands.sql
}

################################################################################
##################  S C R I P T  E X E C U T I O N  S T A R T  #################
################################################################################

UPDATE_TIME_SAVE=`date +%s`

## Check to see if there is new data to export:
if [ ! -e $TIME_MARKER_FILE ]; then
    LAST_UPDATE=0
    syslog "First time to export databases"
else
    LAST_UPDATE=`cat $TIME_MARKER_FILE`
    syslog "Exporting databases after: " $LAST_UPDATE
fi

if [ -e $ARCHIVE_MARKER_FILE ]; then
    if [ $LAST_UPDATE -ge `cat $ARCHIVE_MARKER_FILE` ]; then
        ARCHIVE_ENABLE=0
    fi
else
    syslog $ARCHIVE_MARKER_FILE " not found"
fi
if [ -e $HILOW_MARKER_FILE ]; then
    if [ $LAST_UPDATE -ge `cat $HILOW_MARKER_FILE` ]; then
        HILOW_ENABLE=0
    fi
else
    syslog $HILOW_MARKER_FILE " not found"
fi
if [ -e $NOAA_MARKER_FILE ]; then
    if [ $LAST_UPDATE -ge `cat $NOAA_MARKER_FILE` ]; then
        NOAA_ENABLE=0
    fi
else
    syslog $NOAA_MARKER_FILE " not found"
fi


## Export from the wview SQLite3 databases:
if [ "$ARCHIVE_ENABLE" = "1" ]; then
    UPDATE_MARKER=1
    syslog "Exporting to: " $ARCHIVE_EXPORT_FILE
    rm -rf $ARCHIVE_EXPORT_FILE
    sqlite3_export $ARCHIVE_DB_FILE $SQL_ARCHIVE_TABLE $LAST_UPDATE $ARCHIVE_EXPORT_FILE
fi

SAVE_UPDATE_TIME=$LAST_UPDATE
LAST_UPDATE=`expr $LAST_UPDATE - $SECONDS_IN_HOUR`

if [ "$HILOW_ENABLE" = "1" ]; then
    UPDATE_MARKER=1
    syslog "Exporting HILOW tables:"
    rm -rf $HILOW_EXPORT_FILE_INTEMP
    sqlite3_export $HILOW_DB_FILE $SQL_HILOW_TABLE_INTEMP $LAST_UPDATE $HILOW_EXPORT_FILE_INTEMP
    rm -rf $HILOW_EXPORT_FILE_OUTTEMP
    sqlite3_export $HILOW_DB_FILE $SQL_HILOW_TABLE_OUTTEMP $LAST_UPDATE $HILOW_EXPORT_FILE_OUTTEMP
    rm -rf $HILOW_EXPORT_FILE_INHUMIDITY
    sqlite3_export $HILOW_DB_FILE $SQL_HILOW_TABLE_INHUMIDITY $LAST_UPDATE $HILOW_EXPORT_FILE_INHUMIDITY
    rm -rf $HILOW_EXPORT_FILE_OUTHUMIDITY
    sqlite3_export $HILOW_DB_FILE $SQL_HILOW_TABLE_OUTHUMIDITY $LAST_UPDATE $HILOW_EXPORT_FILE_OUTHUMIDITY
    rm -rf $HILOW_EXPORT_FILE_BP
    sqlite3_export $HILOW_DB_FILE $SQL_HILOW_TABLE_BP $LAST_UPDATE $HILOW_EXPORT_FILE_BP
    rm -rf $HILOW_EXPORT_FILE_WINDSPEED
    sqlite3_export $HILOW_DB_FILE $SQL_HILOW_TABLE_WINDSPEED $LAST_UPDATE $HILOW_EXPORT_FILE_WINDSPEED
    rm -rf $HILOW_EXPORT_FILE_WINDGUST
    sqlite3_export $HILOW_DB_FILE $SQL_HILOW_TABLE_WINDGUST $LAST_UPDATE $HILOW_EXPORT_FILE_WINDGUST
    rm -rf $HILOW_EXPORT_FILE_DEWPOINT
    sqlite3_export $HILOW_DB_FILE $SQL_HILOW_TABLE_DEWPOINT $LAST_UPDATE $HILOW_EXPORT_FILE_DEWPOINT
    rm -rf $HILOW_EXPORT_FILE_RAIN
    sqlite3_export $HILOW_DB_FILE $SQL_HILOW_TABLE_RAIN $LAST_UPDATE $HILOW_EXPORT_FILE_RAIN
    rm -rf $HILOW_EXPORT_FILE_RAINRATE
    sqlite3_export $HILOW_DB_FILE $SQL_HILOW_TABLE_RAINRATE $LAST_UPDATE $HILOW_EXPORT_FILE_RAINRATE
    rm -rf $HILOW_EXPORT_FILE_WINDCHILL
    sqlite3_export $HILOW_DB_FILE $SQL_HILOW_TABLE_WINDCHILL $LAST_UPDATE $HILOW_EXPORT_FILE_WINDCHILL
    rm -rf $HILOW_EXPORT_FILE_HEATINDEX
    sqlite3_export $HILOW_DB_FILE $SQL_HILOW_TABLE_HEATINDEX $LAST_UPDATE $HILOW_EXPORT_FILE_HEATINDEX
    rm -rf $HILOW_EXPORT_FILE_ET
    sqlite3_export $HILOW_DB_FILE $SQL_HILOW_TABLE_ET $LAST_UPDATE $HILOW_EXPORT_FILE_ET
    rm -rf $HILOW_EXPORT_FILE_UV
    sqlite3_export $HILOW_DB_FILE $SQL_HILOW_TABLE_UV $LAST_UPDATE $HILOW_EXPORT_FILE_UV
    rm -rf $HILOW_EXPORT_FILE_SOLARRAD
    sqlite3_export $HILOW_DB_FILE $SQL_HILOW_TABLE_SOLARRAD $LAST_UPDATE $HILOW_EXPORT_FILE_SOLARRAD
    rm -rf $HILOW_EXPORT_FILE_HAIL
    sqlite3_export $HILOW_DB_FILE $SQL_HILOW_TABLE_HAIL $LAST_UPDATE $HILOW_EXPORT_FILE_HAIL
    rm -rf $HILOW_EXPORT_FILE_HAILRATE
    sqlite3_export $HILOW_DB_FILE $SQL_HILOW_TABLE_HAILRATE $LAST_UPDATE $HILOW_EXPORT_FILE_HAILRATE
    rm -rf $HILOW_EXPORT_FILE_WINDDIR
    sqlite3_export $HILOW_DB_FILE $SQL_HILOW_TABLE_WINDDIR $LAST_UPDATE $HILOW_EXPORT_FILE_WINDDIR
fi

LAST_UPDATE=`expr $SAVE_UPDATE_TIME - $SECONDS_IN_DAY`

if [ "$NOAA_ENABLE" = "1" ]; then
    UPDATE_MARKER=1
    syslog "Exporting to: " $NOAA_EXPORT_FILE
    rm -rf $NOAA_EXPORT_FILE
    sqlite3_export $NOAA_DB_FILE $SQL_NOAA_TABLE $LAST_UPDATE $NOAA_EXPORT_FILE
fi


## Get MySQL server configuration:
get_sql_server_config
if [ x"$SQL_HOST" = x ]; then
    syslog_error "Failed to retrieve PostgreSQL export parameters from $WVIEW_CONF_DIR/wview-conf.sdb"
    exit 1
else
    syslog "Exporting with: $SQL_HOST:$SQL_USERNAME:<password omitted>:$SQL_DB_NAME"
fi


## Import to PostgreSQL:
if [ "$ARCHIVE_ENABLE" = "1" ]; then
    syslog "Importing: " $ARCHIVE_EXPORT_FILE
    pgsql_import $SQL_ARCHIVE_TABLE $ARCHIVE_EXPORT_FILE
fi

if [ "$HILOW_ENABLE" = "1" ]; then
    syslog "Importing HILOW tables:"
    pgsql_import $SQL_HILOW_TABLE_INTEMP $HILOW_EXPORT_FILE_INTEMP
    pgsql_import $SQL_HILOW_TABLE_OUTTEMP $HILOW_EXPORT_FILE_OUTTEMP
    pgsql_import $SQL_HILOW_TABLE_INHUMIDITY $HILOW_EXPORT_FILE_INHUMIDITY
    pgsql_import $SQL_HILOW_TABLE_OUTHUMIDITY $HILOW_EXPORT_FILE_OUTHUMIDITY
    pgsql_import $SQL_HILOW_TABLE_BP $HILOW_EXPORT_FILE_BP
    pgsql_import $SQL_HILOW_TABLE_WINDSPEED $HILOW_EXPORT_FILE_WINDSPEED
    pgsql_import $SQL_HILOW_TABLE_WINDGUST $HILOW_EXPORT_FILE_WINDGUST
    pgsql_import $SQL_HILOW_TABLE_DEWPOINT $HILOW_EXPORT_FILE_DEWPOINT
    pgsql_import $SQL_HILOW_TABLE_RAIN $HILOW_EXPORT_FILE_RAIN
    pgsql_import $SQL_HILOW_TABLE_RAINRATE $HILOW_EXPORT_FILE_RAINRATE
    pgsql_import $SQL_HILOW_TABLE_WINDCHILL $HILOW_EXPORT_FILE_WINDCHILL
    pgsql_import $SQL_HILOW_TABLE_HEATINDEX $HILOW_EXPORT_FILE_HEATINDEX
    pgsql_import $SQL_HILOW_TABLE_ET $HILOW_EXPORT_FILE_ET
    pgsql_import $SQL_HILOW_TABLE_UV $HILOW_EXPORT_FILE_UV
    pgsql_import $SQL_HILOW_TABLE_SOLARRAD $HILOW_EXPORT_FILE_SOLARRAD
    pgsql_import $SQL_HILOW_TABLE_HAIL $HILOW_EXPORT_FILE_HAIL
    pgsql_import $SQL_HILOW_TABLE_HAILRATE $HILOW_EXPORT_FILE_HAILRATE
    pgsql_import $SQL_HILOW_TABLE_WINDDIR $HILOW_EXPORT_FILE_WINDDIR
fi

if [ "$NOAA_ENABLE" = "1" ]; then
    syslog "Importing: " $NOAA_EXPORT_FILE
    pgsql_import $SQL_NOAA_TABLE $NOAA_EXPORT_FILE
fi


## Save the current time marker:
if [ "$UPDATE_MARKER" = "1" ]; then
    echo $UPDATE_TIME_SAVE > $TIME_MARKER_FILE
fi

## Done!
exit 0

