################################################################################
#
# File:           wview-mysql-create.sh
#
# Description:    Provide a script to create the wview database for MySQL.
#
# Usage:          wview-mysql-create <db_admin> <db_admin_passwd> <db_user> <db_passwd> <db_name>
#                 Arguments:
#                   db_admin       MySQL server admin with create and grant
#                                  privileges.
#                   db_admin_passwd MySQL server admin password.
#                   db_user        MySQL server user with create and grant
#                                  privileges.
#                   db_passwd      MySQL server password.
#                   db_name        MySQL database name to create and grant on.
#
# History:
# Engineer	  Date	    Ver   Comments
# MS Teel	  03/28/09   1    Original
#
################################################################################

show_usage()
{
    echo "USAGE:"
    echo "wview-mysql-create <db_admin> <db_admin_passwd> <db_user> <db_passwd> <db_name>"
    echo ""
    echo "    db_admin             - MySQL server admin with create and grant privileges."
    echo ""
    echo "    db_admin_passwd      - MySQL server admin password."
    echo ""
    echo "    db_user              - MySQL server user for export acivity (can be same as db_admin)."
    echo ""
    echo "    db_passwd            - MySQL server password."
    echo ""
    echo "    db_name              - MySQL database name to create and grant privileges for."
    echo ""
    echo "REQUIRES: mysql CLI utility for database creation."
    echo "MUST: be executed on the MySQL server."
    echo ""
}

################################################################################
#################################  M A C R O S  ################################
################################################################################

## DO NOT CHANGE ANY DEFS IN THIS BLOCK!

export PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

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

## Test command line arguments:
if [ "$1" = "help" ]; then
    show_usage
    exit 0
fi

## Check arguments:
if [ x"$1" = x ]; then
    echo "You must specify the MySQL admin username to create export database - use 'wview-mysql-create help' for details"
    show_usage
    exit 1
elif [ x"$2" = x ]; then
    echo "You must specify the MySQL admin password to create export database - use 'wview-mysql-create help' for details"
    show_usage
    exit 1
elif [ x"$3" = x ]; then
    echo "You must specify the MySQL username to create export database - use 'wview-mysql-create help' for details"
    show_usage
    exit 1
elif [ x"$4" = x ]; then
    echo "You must specify the MySQL password to create export database - use 'wview-mysql-create help' for details"
    show_usage
    exit 1
elif [ x"$5" = x ]; then
    echo "You must specify the MySQL database name to create export database - use 'wview-mysql-create help' for details"
    show_usage
    exit 1
else
    MYSQL_ADMIN_USERNAME=$1
    MYSQL_ADMIN_PASSWORD=$2
    MYSQL_USERNAME=$3
    MYSQL_PASSWORD=$4
    MYSQL_DBNAME=$5
fi

################################################################################
#######################  D E F I N E  F U N C T I O N S  #######################
################################################################################
syslog()
{
    logger -t WV_EXPORT $1 $2
}

syslog_error()
{
    logger -t WV_EXPORT_ERROR $1 $2
}

mysql_create()
{
    echo "create database $MYSQL_DBNAME;" > /tmp/commands.sql
    echo "grant all privileges on $MYSQL_DBNAME.* to $MYSQL_USERNAME@\"localhost\" identified by '$MYSQL_PASSWORD';" >> /tmp/commands.sql
    echo "grant all privileges on $MYSQL_DBNAME.* to $MYSQL_USERNAME@\"%\" identified by '$MYSQL_PASSWORD';" >> /tmp/commands.sql
    echo "use $MYSQL_DBNAME;" >> /tmp/commands.sql

    echo "create table $SQL_ARCHIVE_TABLE (" >> /tmp/commands.sql
    echo "	dateTime INTEGER NOT NULL PRIMARY KEY," >> /tmp/commands.sql
    echo "	usUnits INTEGER NOT NULL," >> /tmp/commands.sql
    echo "	arcInt INTEGER NOT NULL," >> /tmp/commands.sql
    echo "	barometer REAL," >> /tmp/commands.sql
    echo "	pressure REAL," >> /tmp/commands.sql
    echo "	altimeter REAL," >> /tmp/commands.sql
    echo "	inTemp REAL," >> /tmp/commands.sql
    echo "	outTemp REAL," >> /tmp/commands.sql
    echo "	inHumidity REAL," >> /tmp/commands.sql
    echo "	outHumidity REAL," >> /tmp/commands.sql
    echo "	windSpeed REAL," >> /tmp/commands.sql
    echo "	windDir REAL," >> /tmp/commands.sql
    echo "	windGust REAL," >> /tmp/commands.sql
    echo "	windGustDir REAL," >> /tmp/commands.sql
    echo "	rainRate REAL," >> /tmp/commands.sql
    echo "	rain REAL," >> /tmp/commands.sql
    echo "	dewpoint REAL," >> /tmp/commands.sql
    echo "	windchill REAL," >> /tmp/commands.sql
    echo "	heatindex REAL," >> /tmp/commands.sql
    echo "	ET REAL," >> /tmp/commands.sql
    echo "	radiation REAL," >> /tmp/commands.sql
    echo "	UV REAL," >> /tmp/commands.sql
    echo "	extraTemp1 REAL," >> /tmp/commands.sql
    echo "	extraTemp2 REAL," >> /tmp/commands.sql
    echo "	extraTemp3 REAL," >> /tmp/commands.sql
    echo "	soilTemp1 REAL," >> /tmp/commands.sql
    echo "	soilTemp2 REAL," >> /tmp/commands.sql
    echo "	soilTemp3 REAL," >> /tmp/commands.sql
    echo "	soilTemp4 REAL," >> /tmp/commands.sql
    echo "	leafTemp1 REAL," >> /tmp/commands.sql
    echo "	leafTemp2 REAL," >> /tmp/commands.sql
    echo "	extraHumid1 REAL," >> /tmp/commands.sql
    echo "	extraHumid2 REAL," >> /tmp/commands.sql
    echo "	soilMoist1 REAL," >> /tmp/commands.sql
    echo "	soilMoist2 REAL," >> /tmp/commands.sql
    echo "	soilMoist3 REAL," >> /tmp/commands.sql
    echo "	soilMoist4 REAL," >> /tmp/commands.sql
    echo "	leafWet1 REAL," >> /tmp/commands.sql
    echo "	leafWet2 REAL," >> /tmp/commands.sql
    echo "	rxCheckPercent REAL," >> /tmp/commands.sql
    echo "	txBatteryStatus REAL," >> /tmp/commands.sql
    echo "	consBatteryVoltage REAL," >> /tmp/commands.sql
    echo "	hail REAL," >> /tmp/commands.sql
    echo "	hailRate REAL," >> /tmp/commands.sql
    echo "	heatingTemp REAL," >> /tmp/commands.sql
    echo "	heatingVoltage REAL," >> /tmp/commands.sql
    echo "	supplyVoltage REAL," >> /tmp/commands.sql
    echo "	referenceVoltage REAL," >> /tmp/commands.sql
    echo "	windBatteryStatus REAL," >> /tmp/commands.sql
    echo "	rainBatteryStatus REAL," >> /tmp/commands.sql
    echo "	outTempBatteryStatus REAL," >> /tmp/commands.sql
    echo "	inTempBatteryStatus REAL" >> /tmp/commands.sql
    echo ");" >> /tmp/commands.sql

    echo "create table $SQL_HILOW_TABLE_INTEMP (" >> /tmp/commands.sql
    echo "	dateTime INTEGER NOT NULL PRIMARY KEY," >> /tmp/commands.sql
    echo "	low REAL," >> /tmp/commands.sql
    echo "	timeLow INTEGER," >> /tmp/commands.sql
    echo "	high REAL," >> /tmp/commands.sql
    echo "	timeHigh INTEGER," >> /tmp/commands.sql
    echo "	whenHigh REAL," >> /tmp/commands.sql
    echo "	cumulative REAL," >> /tmp/commands.sql
    echo "	samples INTEGER" >> /tmp/commands.sql
    echo ");" >> /tmp/commands.sql

    echo "create table $SQL_HILOW_TABLE_OUTTEMP (" >> /tmp/commands.sql
    echo "	dateTime INTEGER NOT NULL PRIMARY KEY," >> /tmp/commands.sql
    echo "	low REAL," >> /tmp/commands.sql
    echo "	timeLow INTEGER," >> /tmp/commands.sql
    echo "	high REAL," >> /tmp/commands.sql
    echo "	timeHigh INTEGER," >> /tmp/commands.sql
    echo "	whenHigh REAL," >> /tmp/commands.sql
    echo "	cumulative REAL," >> /tmp/commands.sql
    echo "	samples INTEGER" >> /tmp/commands.sql
    echo ");" >> /tmp/commands.sql

    echo "create table $SQL_HILOW_TABLE_INHUMIDITY (" >> /tmp/commands.sql
    echo "	dateTime INTEGER NOT NULL PRIMARY KEY," >> /tmp/commands.sql
    echo "	low REAL," >> /tmp/commands.sql
    echo "	timeLow INTEGER," >> /tmp/commands.sql
    echo "	high REAL," >> /tmp/commands.sql
    echo "	timeHigh INTEGER," >> /tmp/commands.sql
    echo "	whenHigh REAL," >> /tmp/commands.sql
    echo "	cumulative REAL," >> /tmp/commands.sql
    echo "	samples INTEGER" >> /tmp/commands.sql
    echo ");" >> /tmp/commands.sql

    echo "create table $SQL_HILOW_TABLE_OUTHUMIDITY (" >> /tmp/commands.sql
    echo "	dateTime INTEGER NOT NULL PRIMARY KEY," >> /tmp/commands.sql
    echo "	low REAL," >> /tmp/commands.sql
    echo "	timeLow INTEGER," >> /tmp/commands.sql
    echo "	high REAL," >> /tmp/commands.sql
    echo "	timeHigh INTEGER," >> /tmp/commands.sql
    echo "	whenHigh REAL," >> /tmp/commands.sql
    echo "	cumulative REAL," >> /tmp/commands.sql
    echo "	samples INTEGER" >> /tmp/commands.sql
    echo ");" >> /tmp/commands.sql

    echo "create table $SQL_HILOW_TABLE_BP (" >> /tmp/commands.sql
    echo "	dateTime INTEGER NOT NULL PRIMARY KEY," >> /tmp/commands.sql
    echo "	low REAL," >> /tmp/commands.sql
    echo "	timeLow INTEGER," >> /tmp/commands.sql
    echo "	high REAL," >> /tmp/commands.sql
    echo "	timeHigh INTEGER," >> /tmp/commands.sql
    echo "	whenHigh REAL," >> /tmp/commands.sql
    echo "	cumulative REAL," >> /tmp/commands.sql
    echo "	samples INTEGER" >> /tmp/commands.sql
    echo ");" >> /tmp/commands.sql

    echo "create table $SQL_HILOW_TABLE_WINDSPEED (" >> /tmp/commands.sql
    echo "	dateTime INTEGER NOT NULL PRIMARY KEY," >> /tmp/commands.sql
    echo "	low REAL," >> /tmp/commands.sql
    echo "	timeLow INTEGER," >> /tmp/commands.sql
    echo "	high REAL," >> /tmp/commands.sql
    echo "	timeHigh INTEGER," >> /tmp/commands.sql
    echo "	whenHigh REAL," >> /tmp/commands.sql
    echo "	cumulative REAL," >> /tmp/commands.sql
    echo "	samples INTEGER" >> /tmp/commands.sql
    echo ");" >> /tmp/commands.sql

    echo "create table $SQL_HILOW_TABLE_WINDGUST (" >> /tmp/commands.sql
    echo "	dateTime INTEGER NOT NULL PRIMARY KEY," >> /tmp/commands.sql
    echo "	low REAL," >> /tmp/commands.sql
    echo "	timeLow INTEGER," >> /tmp/commands.sql
    echo "	high REAL," >> /tmp/commands.sql
    echo "	timeHigh INTEGER," >> /tmp/commands.sql
    echo "	whenHigh REAL," >> /tmp/commands.sql
    echo "	cumulative REAL," >> /tmp/commands.sql
    echo "	samples INTEGER" >> /tmp/commands.sql
    echo ");" >> /tmp/commands.sql

    echo "create table $SQL_HILOW_TABLE_DEWPOINT (" >> /tmp/commands.sql
    echo "	dateTime INTEGER NOT NULL PRIMARY KEY," >> /tmp/commands.sql
    echo "	low REAL," >> /tmp/commands.sql
    echo "	timeLow INTEGER," >> /tmp/commands.sql
    echo "	high REAL," >> /tmp/commands.sql
    echo "	timeHigh INTEGER," >> /tmp/commands.sql
    echo "	whenHigh REAL," >> /tmp/commands.sql
    echo "	cumulative REAL," >> /tmp/commands.sql
    echo "	samples INTEGER" >> /tmp/commands.sql
    echo ");" >> /tmp/commands.sql

    echo "create table $SQL_HILOW_TABLE_RAIN (" >> /tmp/commands.sql
    echo "	dateTime INTEGER NOT NULL PRIMARY KEY," >> /tmp/commands.sql
    echo "	low REAL," >> /tmp/commands.sql
    echo "	timeLow INTEGER," >> /tmp/commands.sql
    echo "	high REAL," >> /tmp/commands.sql
    echo "	timeHigh INTEGER," >> /tmp/commands.sql
    echo "	whenHigh REAL," >> /tmp/commands.sql
    echo "	cumulative REAL," >> /tmp/commands.sql
    echo "	samples INTEGER" >> /tmp/commands.sql
    echo ");" >> /tmp/commands.sql

    echo "create table $SQL_HILOW_TABLE_RAINRATE (" >> /tmp/commands.sql
    echo "	dateTime INTEGER NOT NULL PRIMARY KEY," >> /tmp/commands.sql
    echo "	low REAL," >> /tmp/commands.sql
    echo "	timeLow INTEGER," >> /tmp/commands.sql
    echo "	high REAL," >> /tmp/commands.sql
    echo "	timeHigh INTEGER," >> /tmp/commands.sql
    echo "	whenHigh REAL," >> /tmp/commands.sql
    echo "	cumulative REAL," >> /tmp/commands.sql
    echo "	samples INTEGER" >> /tmp/commands.sql
    echo ");" >> /tmp/commands.sql

    echo "create table $SQL_HILOW_TABLE_WINDCHILL (" >> /tmp/commands.sql
    echo "	dateTime INTEGER NOT NULL PRIMARY KEY," >> /tmp/commands.sql
    echo "	low REAL," >> /tmp/commands.sql
    echo "	timeLow INTEGER," >> /tmp/commands.sql
    echo "	high REAL," >> /tmp/commands.sql
    echo "	timeHigh INTEGER," >> /tmp/commands.sql
    echo "	whenHigh REAL," >> /tmp/commands.sql
    echo "	cumulative REAL," >> /tmp/commands.sql
    echo "	samples INTEGER" >> /tmp/commands.sql
    echo ");" >> /tmp/commands.sql

    echo "create table $SQL_HILOW_TABLE_HEATINDEX (" >> /tmp/commands.sql
    echo "	dateTime INTEGER NOT NULL PRIMARY KEY," >> /tmp/commands.sql
    echo "	low REAL," >> /tmp/commands.sql
    echo "	timeLow INTEGER," >> /tmp/commands.sql
    echo "	high REAL," >> /tmp/commands.sql
    echo "	timeHigh INTEGER," >> /tmp/commands.sql
    echo "	whenHigh REAL," >> /tmp/commands.sql
    echo "	cumulative REAL," >> /tmp/commands.sql
    echo "	samples INTEGER" >> /tmp/commands.sql
    echo ");" >> /tmp/commands.sql

    echo "create table $SQL_HILOW_TABLE_ET (" >> /tmp/commands.sql
    echo "	dateTime INTEGER NOT NULL PRIMARY KEY," >> /tmp/commands.sql
    echo "	low REAL," >> /tmp/commands.sql
    echo "	timeLow INTEGER," >> /tmp/commands.sql
    echo "	high REAL," >> /tmp/commands.sql
    echo "	timeHigh INTEGER," >> /tmp/commands.sql
    echo "	whenHigh REAL," >> /tmp/commands.sql
    echo "	cumulative REAL," >> /tmp/commands.sql
    echo "	samples INTEGER" >> /tmp/commands.sql
    echo ");" >> /tmp/commands.sql

    echo "create table $SQL_HILOW_TABLE_UV (" >> /tmp/commands.sql
    echo "	dateTime INTEGER NOT NULL PRIMARY KEY," >> /tmp/commands.sql
    echo "	low REAL," >> /tmp/commands.sql
    echo "	timeLow INTEGER," >> /tmp/commands.sql
    echo "	high REAL," >> /tmp/commands.sql
    echo "	timeHigh INTEGER," >> /tmp/commands.sql
    echo "	whenHigh REAL," >> /tmp/commands.sql
    echo "	cumulative REAL," >> /tmp/commands.sql
    echo "	samples INTEGER" >> /tmp/commands.sql
    echo ");" >> /tmp/commands.sql

    echo "create table $SQL_HILOW_TABLE_SOLARRAD (" >> /tmp/commands.sql
    echo "	dateTime INTEGER NOT NULL PRIMARY KEY," >> /tmp/commands.sql
    echo "	low REAL," >> /tmp/commands.sql
    echo "	timeLow INTEGER," >> /tmp/commands.sql
    echo "	high REAL," >> /tmp/commands.sql
    echo "	timeHigh INTEGER," >> /tmp/commands.sql
    echo "	whenHigh REAL," >> /tmp/commands.sql
    echo "	cumulative REAL," >> /tmp/commands.sql
    echo "	samples INTEGER" >> /tmp/commands.sql
    echo ");" >> /tmp/commands.sql

    echo "create table $SQL_HILOW_TABLE_HAIL (" >> /tmp/commands.sql
    echo "	dateTime INTEGER NOT NULL PRIMARY KEY," >> /tmp/commands.sql
    echo "	low REAL," >> /tmp/commands.sql
    echo "	timeLow INTEGER," >> /tmp/commands.sql
    echo "	high REAL," >> /tmp/commands.sql
    echo "	timeHigh INTEGER," >> /tmp/commands.sql
    echo "	whenHigh REAL," >> /tmp/commands.sql
    echo "	cumulative REAL," >> /tmp/commands.sql
    echo "	samples INTEGER" >> /tmp/commands.sql
    echo ");" >> /tmp/commands.sql

    echo "create table $SQL_HILOW_TABLE_HAILRATE (" >> /tmp/commands.sql
    echo "	dateTime INTEGER NOT NULL PRIMARY KEY," >> /tmp/commands.sql
    echo "	low REAL," >> /tmp/commands.sql
    echo "	timeLow INTEGER," >> /tmp/commands.sql
    echo "	high REAL," >> /tmp/commands.sql
    echo "	timeHigh INTEGER," >> /tmp/commands.sql
    echo "	whenHigh REAL," >> /tmp/commands.sql
    echo "	cumulative REAL," >> /tmp/commands.sql
    echo "	samples INTEGER" >> /tmp/commands.sql
    echo ");" >> /tmp/commands.sql

    echo "create table $SQL_HILOW_TABLE_WINDDIR (" >> /tmp/commands.sql
    echo "	dateTime INTEGER NOT NULL PRIMARY KEY," >> /tmp/commands.sql
    echo "	bin0 INTEGER," >> /tmp/commands.sql
    echo "	bin1 INTEGER," >> /tmp/commands.sql
    echo "	bin2 INTEGER," >> /tmp/commands.sql
    echo "	bin3 INTEGER," >> /tmp/commands.sql
    echo "	bin4 INTEGER," >> /tmp/commands.sql
    echo "	bin5 INTEGER," >> /tmp/commands.sql
    echo "	bin6 INTEGER," >> /tmp/commands.sql
    echo "	bin7 INTEGER," >> /tmp/commands.sql
    echo "	bin8 INTEGER," >> /tmp/commands.sql
    echo "	bin9 INTEGER," >> /tmp/commands.sql
    echo "	bin10 INTEGER," >> /tmp/commands.sql
    echo "	bin11 INTEGER," >> /tmp/commands.sql
    echo "	bin12 INTEGER," >> /tmp/commands.sql
    echo "	bin13 INTEGER," >> /tmp/commands.sql
    echo "	bin14 INTEGER," >> /tmp/commands.sql
    echo "	bin15 INTEGER," >> /tmp/commands.sql
    echo "	bin16 INTEGER," >> /tmp/commands.sql
    echo "	bin17 INTEGER," >> /tmp/commands.sql
    echo "	bin18 INTEGER," >> /tmp/commands.sql
    echo "	bin19 INTEGER" >> /tmp/commands.sql
    echo ");" >> /tmp/commands.sql

    echo "create table $SQL_NOAA_TABLE (" >> /tmp/commands.sql
    echo "	dateTime INTEGER NOT NULL PRIMARY KEY," >> /tmp/commands.sql
    echo "	meanTemp REAL," >> /tmp/commands.sql
    echo "	highTemp REAL," >> /tmp/commands.sql
    echo "	highTempTime INTEGER," >> /tmp/commands.sql
    echo "	lowTemp REAL," >> /tmp/commands.sql
    echo "	lowTempTime INTEGER," >> /tmp/commands.sql
    echo "	heatDegDays REAL," >> /tmp/commands.sql
    echo "	coolDegDays REAL," >> /tmp/commands.sql
    echo "	rain REAL," >> /tmp/commands.sql
    echo "	avgWind REAL," >> /tmp/commands.sql
    echo "	highWind REAL," >> /tmp/commands.sql
    echo "	highWindTime INTEGER," >> /tmp/commands.sql
    echo "	domWindDir INTEGER" >> /tmp/commands.sql
    echo ");" >> /tmp/commands.sql

    mysql --user=$MYSQL_ADMIN_USERNAME --password=$MYSQL_ADMIN_PASSWORD < /tmp/commands.sql > /tmp/mysql_create.log
    rm -rf /tmp/commands.sql
}

################################################################################
##################  S C R I P T  E X E C U T I O N  S T A R T  #################
################################################################################

mysql_create

## Done!
exit 0

