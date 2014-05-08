################################################################################
#
# File:           wview-pgsql-create.sh
#
# Description:    Provide a script to create the wview database for PostgreSQL.
#
# Usage:          wview-pgsql-create <db_user> <db_passwd> <db_name>
#                 Arguments:
#                   db_user        PostgreSQL server user with create and grant
#                                  privileges.
#                   db_name        PostgreSQL database name to create and grant on.
#
# History:
# Engineer	  Date	    Ver   Comments
# MS Teel	  03/28/09   1    Original
#
################################################################################

show_usage()
{
    echo "USAGE:"
    echo "wview-pgsql-create <db_name>"
    echo ""
    echo "    db_name              - PostgreSQL database name to create and grant privileges for."
    echo ""
    echo "REQUIRES: psql CLI utility for database creation."
    echo "MUST: be executed on the PostgreSQL server."
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
    echo "You must specify the PostgreSQL database name to create export database - use 'wview-pgsql-create help' for details"
    show_usage
    exit 1
else
    PGSQL_USERNAME=root
    PGSQL_DBNAME=$1
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
#   create the plpgsql language;
    createlang plpgsql wviewDB

    echo "create table $SQL_ARCHIVE_TABLE (" > /tmp/commands.sql
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

    echo "CREATE OR REPLACE FUNCTION repl_archive() RETURNS trigger AS \$BODY\$ BEGIN IF (EXISTS(SELECT 1 FROM $SQL_ARCHIVE_TABLE C WHERE C.dateTime = NEW.dateTime)) THEN UPDATE $SQL_ARCHIVE_TABLE SET usUnits = NEW.usUnits, arcInt = NEW.arcInt, barometer = NEW.barometer, pressure = NEW.pressure, altimeter = NEW.altimeter, inTemp = NEW.inTemp, outTemp = NEW.outTemp, inHumidity = NEW.inHumidity, outHumidity = NEW.outHumidity, windSpeed = NEW.windSpeed, windDir = NEW.windDir, windGust = NEW.windGust, windGustDir = NEW.windGustDir, rainRate = NEW.rainRate, rain = NEW.rain, dewpoint = NEW.dewpoint, windchill = NEW.windchill, heatindex = NEW.heatindex, ET = NEW.ET, radiation = NEW.radiation, UV = NEW.UV, extraTemp1 = NEW.extraTemp1, extraTemp2 = NEW.extraTemp2, extraTemp3 = NEW.extraTemp3, soilTemp1 = NEW.soilTemp1, soilTemp2 = NEW.soilTemp2, soilTemp3 = NEW.soilTemp3, soilTemp4 = NEW.soilTemp4, leafTemp1 = NEW.leafTemp1, leafTemp2 = NEW.leafTemp2, extraHumid1 = NEW.extraHumid1, extraHumid2 = NEW.extraHumid2, soilMoist1 = NEW.soilMoist1, soilMoist2 = NEW.soilMoist2, soilMoist3 = NEW.soilMoist3, soilMoist4 = NEW.soilMoist4, leafWet1 = NEW.leafWet1, leafWet2 = NEW.leafWet2, rxCheckPercent = NEW.rxCheckPercent, txBatteryStatus = NEW.txBatteryStatus, consBatteryVoltage = NEW.consBatteryVoltage, hail = NEW.hail, hailRate = NEW.hailRate, heatingTemp = NEW.heatingTemp, heatingVoltage = NEW.heatingVoltage, supplyVoltage = NEW.supplyVoltage, referenceVoltage = NEW.referenceVoltage, windBatteryStatus = NEW.windBatteryStatus, rainBatteryStatus = NEW.rainBatteryStatus, outTempBatteryStatus = NEW.outTempBatteryStatus, inTempBatteryStatus = NEW.inTempBatteryStatus WHERE dateTime = NEW.dateTime; RETURN OLD; ELSE RETURN NEW; END IF; END; \$BODY\$ LANGUAGE plpgsql VOLATILE COST 100; ALTER FUNCTION repl_archive() OWNER TO postgres; CREATE TRIGGER repl_archive BEFORE INSERT ON $SQL_ARCHIVE_TABLE FOR EACH ROW EXECUTE PROCEDURE repl_archive();" >> /tmp/commands.sql

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

    echo "CREATE OR REPLACE FUNCTION repl_intemp() RETURNS trigger AS \$BODY\$ BEGIN IF (EXISTS(SELECT 1 FROM $SQL_HILOW_TABLE_INTEMP C WHERE C.dateTime = NEW.dateTime)) THEN UPDATE $SQL_HILOW_TABLE_INTEMP SET low = NEW.low, timeLow = NEW.timeLow, high = NEW.high, timeHigh = NEW.timeHigh, whenHigh = NEW.whenHigh, cumulative = NEW.cumulative, samples = NEW.samples WHERE dateTime = NEW.dateTime; RETURN OLD; ELSE RETURN NEW; END IF; END; \$BODY\$ LANGUAGE plpgsql VOLATILE COST 100; ALTER FUNCTION repl_intemp() OWNER TO postgres; CREATE TRIGGER repl_intemp BEFORE INSERT ON $SQL_HILOW_TABLE_INTEMP FOR EACH ROW EXECUTE PROCEDURE repl_intemp();" >> /tmp/commands.sql

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

    echo "CREATE OR REPLACE FUNCTION repl_outtemp() RETURNS trigger AS \$BODY\$ BEGIN IF (EXISTS(SELECT 1 FROM $SQL_HILOW_TABLE_OUTTEMP C WHERE C.dateTime = NEW.dateTime)) THEN UPDATE $SQL_HILOW_TABLE_OUTTEMP SET low = NEW.low, timeLow = NEW.timeLow, high = NEW.high, timeHigh = NEW.timeHigh, whenHigh = NEW.whenHigh, cumulative = NEW.cumulative, samples = NEW.samples WHERE dateTime = NEW.dateTime; RETURN OLD; ELSE RETURN NEW; END IF; END; \$BODY\$ LANGUAGE plpgsql VOLATILE COST 100; ALTER FUNCTION repl_outtemp() OWNER TO postgres; CREATE TRIGGER repl_outtemp BEFORE INSERT ON $SQL_HILOW_TABLE_OUTTEMP FOR EACH ROW EXECUTE PROCEDURE repl_outtemp();" >> /tmp/commands.sql

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

    echo "CREATE OR REPLACE FUNCTION repl_inhumid() RETURNS trigger AS \$BODY\$ BEGIN IF (EXISTS(SELECT 1 FROM $SQL_HILOW_TABLE_INHUMIDITY C WHERE C.dateTime = NEW.dateTime)) THEN UPDATE $SQL_HILOW_TABLE_INHUMIDITY SET low = NEW.low, timeLow = NEW.timeLow, high = NEW.high, timeHigh = NEW.timeHigh, whenHigh = NEW.whenHigh, cumulative = NEW.cumulative, samples = NEW.samples WHERE dateTime = NEW.dateTime; RETURN OLD; ELSE RETURN NEW; END IF; END; \$BODY\$ LANGUAGE plpgsql VOLATILE COST 100; ALTER FUNCTION repl_inhumid() OWNER TO postgres; CREATE TRIGGER repl_inhumid BEFORE INSERT ON $SQL_HILOW_TABLE_INHUMIDITY FOR EACH ROW EXECUTE PROCEDURE repl_inhumid();" >> /tmp/commands.sql

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

    echo "CREATE OR REPLACE FUNCTION repl_outhumid() RETURNS trigger AS \$BODY\$ BEGIN IF (EXISTS(SELECT 1 FROM $SQL_HILOW_TABLE_OUTHUMIDITY C WHERE C.dateTime = NEW.dateTime)) THEN UPDATE $SQL_HILOW_TABLE_OUTHUMIDITY SET low = NEW.low, timeLow = NEW.timeLow, high = NEW.high, timeHigh = NEW.timeHigh, whenHigh = NEW.whenHigh, cumulative = NEW.cumulative, samples = NEW.samples WHERE dateTime = NEW.dateTime; RETURN OLD; ELSE RETURN NEW; END IF; END; \$BODY\$ LANGUAGE plpgsql VOLATILE COST 100; ALTER FUNCTION repl_outhumid() OWNER TO postgres; CREATE TRIGGER repl_outhumid BEFORE INSERT ON $SQL_HILOW_TABLE_OUTHUMIDITY FOR EACH ROW EXECUTE PROCEDURE repl_outhumid();" >> /tmp/commands.sql

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

    echo "CREATE OR REPLACE FUNCTION repl_bp() RETURNS trigger AS \$BODY\$ BEGIN IF (EXISTS(SELECT 1 FROM $SQL_HILOW_TABLE_BP C WHERE C.dateTime = NEW.dateTime)) THEN UPDATE $SQL_HILOW_TABLE_BP SET low = NEW.low, timeLow = NEW.timeLow, high = NEW.high, timeHigh = NEW.timeHigh, whenHigh = NEW.whenHigh, cumulative = NEW.cumulative, samples = NEW.samples WHERE dateTime = NEW.dateTime; RETURN OLD; ELSE RETURN NEW; END IF; END; \$BODY\$ LANGUAGE plpgsql VOLATILE COST 100; ALTER FUNCTION repl_bp() OWNER TO postgres; CREATE TRIGGER repl_bp BEFORE INSERT ON $SQL_HILOW_TABLE_BP FOR EACH ROW EXECUTE PROCEDURE repl_bp();" >> /tmp/commands.sql

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

    echo "CREATE OR REPLACE FUNCTION repl_speed() RETURNS trigger AS \$BODY\$ BEGIN IF (EXISTS(SELECT 1 FROM $SQL_HILOW_TABLE_WINDSPEED C WHERE C.dateTime = NEW.dateTime)) THEN UPDATE $SQL_HILOW_TABLE_WINDSPEED SET low = NEW.low, timeLow = NEW.timeLow, high = NEW.high, timeHigh = NEW.timeHigh, whenHigh = NEW.whenHigh, cumulative = NEW.cumulative, samples = NEW.samples WHERE dateTime = NEW.dateTime; RETURN OLD; ELSE RETURN NEW; END IF; END; \$BODY\$ LANGUAGE plpgsql VOLATILE COST 100; ALTER FUNCTION repl_speed() OWNER TO postgres; CREATE TRIGGER repl_speed BEFORE INSERT ON $SQL_HILOW_TABLE_WINDSPEED FOR EACH ROW EXECUTE PROCEDURE repl_speed();" >> /tmp/commands.sql

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

    echo "CREATE OR REPLACE FUNCTION repl_gust() RETURNS trigger AS \$BODY\$ BEGIN IF (EXISTS(SELECT 1 FROM $SQL_HILOW_TABLE_WINDGUST C WHERE C.dateTime = NEW.dateTime)) THEN UPDATE $SQL_HILOW_TABLE_WINDGUST SET low = NEW.low, timeLow = NEW.timeLow, high = NEW.high, timeHigh = NEW.timeHigh, whenHigh = NEW.whenHigh, cumulative = NEW.cumulative, samples = NEW.samples WHERE dateTime = NEW.dateTime; RETURN OLD; ELSE RETURN NEW; END IF; END; \$BODY\$ LANGUAGE plpgsql VOLATILE COST 100; ALTER FUNCTION repl_gust() OWNER TO postgres; CREATE TRIGGER repl_gust BEFORE INSERT ON $SQL_HILOW_TABLE_WINDGUST FOR EACH ROW EXECUTE PROCEDURE repl_gust();" >> /tmp/commands.sql

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

    echo "CREATE OR REPLACE FUNCTION repl_dewpt() RETURNS trigger AS \$BODY\$ BEGIN IF (EXISTS(SELECT 1 FROM $SQL_HILOW_TABLE_DEWPOINT C WHERE C.dateTime = NEW.dateTime)) THEN UPDATE $SQL_HILOW_TABLE_DEWPOINT SET low = NEW.low, timeLow = NEW.timeLow, high = NEW.high, timeHigh = NEW.timeHigh, whenHigh = NEW.whenHigh, cumulative = NEW.cumulative, samples = NEW.samples WHERE dateTime = NEW.dateTime; RETURN OLD; ELSE RETURN NEW; END IF; END; \$BODY\$ LANGUAGE plpgsql VOLATILE COST 100; ALTER FUNCTION repl_dewpt() OWNER TO postgres; CREATE TRIGGER repl_dewpt BEFORE INSERT ON $SQL_HILOW_TABLE_DEWPOINT FOR EACH ROW EXECUTE PROCEDURE repl_dewpt();" >> /tmp/commands.sql

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

    echo "CREATE OR REPLACE FUNCTION repl_rain() RETURNS trigger AS \$BODY\$ BEGIN IF (EXISTS(SELECT 1 FROM $SQL_HILOW_TABLE_RAIN C WHERE C.dateTime = NEW.dateTime)) THEN UPDATE $SQL_HILOW_TABLE_RAIN SET low = NEW.low, timeLow = NEW.timeLow, high = NEW.high, timeHigh = NEW.timeHigh, whenHigh = NEW.whenHigh, cumulative = NEW.cumulative, samples = NEW.samples WHERE dateTime = NEW.dateTime; RETURN OLD; ELSE RETURN NEW; END IF; END; \$BODY\$ LANGUAGE plpgsql VOLATILE COST 100; ALTER FUNCTION repl_rain() OWNER TO postgres; CREATE TRIGGER repl_rain BEFORE INSERT ON $SQL_HILOW_TABLE_RAIN FOR EACH ROW EXECUTE PROCEDURE repl_rain();" >> /tmp/commands.sql

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

    echo "CREATE OR REPLACE FUNCTION repl_rainrate() RETURNS trigger AS \$BODY\$ BEGIN IF (EXISTS(SELECT 1 FROM $SQL_HILOW_TABLE_RAINRATE C WHERE C.dateTime = NEW.dateTime)) THEN UPDATE $SQL_HILOW_TABLE_RAINRATE SET low = NEW.low, timeLow = NEW.timeLow, high = NEW.high, timeHigh = NEW.timeHigh, whenHigh = NEW.whenHigh, cumulative = NEW.cumulative, samples = NEW.samples WHERE dateTime = NEW.dateTime; RETURN OLD; ELSE RETURN NEW; END IF; END; \$BODY\$ LANGUAGE plpgsql VOLATILE COST 100; ALTER FUNCTION repl_rainrate() OWNER TO postgres; CREATE TRIGGER repl_rainrate BEFORE INSERT ON $SQL_HILOW_TABLE_RAINRATE FOR EACH ROW EXECUTE PROCEDURE repl_rainrate();" >> /tmp/commands.sql

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

    echo "CREATE OR REPLACE FUNCTION repl_chill() RETURNS trigger AS \$BODY\$ BEGIN IF (EXISTS(SELECT 1 FROM $SQL_HILOW_TABLE_WINDCHILL C WHERE C.dateTime = NEW.dateTime)) THEN UPDATE $SQL_HILOW_TABLE_WINDCHILL SET low = NEW.low, timeLow = NEW.timeLow, high = NEW.high, timeHigh = NEW.timeHigh, whenHigh = NEW.whenHigh, cumulative = NEW.cumulative, samples = NEW.samples WHERE dateTime = NEW.dateTime; RETURN OLD; ELSE RETURN NEW; END IF; END; \$BODY\$ LANGUAGE plpgsql VOLATILE COST 100; ALTER FUNCTION repl_chill() OWNER TO postgres; CREATE TRIGGER repl_chill BEFORE INSERT ON $SQL_HILOW_TABLE_WINDCHILL FOR EACH ROW EXECUTE PROCEDURE repl_chill();" >> /tmp/commands.sql

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

    echo "CREATE OR REPLACE FUNCTION repl_heat() RETURNS trigger AS \$BODY\$ BEGIN IF (EXISTS(SELECT 1 FROM $SQL_HILOW_TABLE_HEATINDEX C WHERE C.dateTime = NEW.dateTime)) THEN UPDATE $SQL_HILOW_TABLE_HEATINDEX SET low = NEW.low, timeLow = NEW.timeLow, high = NEW.high, timeHigh = NEW.timeHigh, whenHigh = NEW.whenHigh, cumulative = NEW.cumulative, samples = NEW.samples WHERE dateTime = NEW.dateTime; RETURN OLD; ELSE RETURN NEW; END IF; END; \$BODY\$ LANGUAGE plpgsql VOLATILE COST 100; ALTER FUNCTION repl_heat() OWNER TO postgres; CREATE TRIGGER repl_heat BEFORE INSERT ON $SQL_HILOW_TABLE_HEATINDEX FOR EACH ROW EXECUTE PROCEDURE repl_heat();" >> /tmp/commands.sql

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

    echo "CREATE OR REPLACE FUNCTION repl_et() RETURNS trigger AS \$BODY\$ BEGIN IF (EXISTS(SELECT 1 FROM $SQL_HILOW_TABLE_ET C WHERE C.dateTime = NEW.dateTime)) THEN UPDATE $SQL_HILOW_TABLE_ET SET low = NEW.low, timeLow = NEW.timeLow, high = NEW.high, timeHigh = NEW.timeHigh, whenHigh = NEW.whenHigh, cumulative = NEW.cumulative, samples = NEW.samples WHERE dateTime = NEW.dateTime; RETURN OLD; ELSE RETURN NEW; END IF; END; \$BODY\$ LANGUAGE plpgsql VOLATILE COST 100; ALTER FUNCTION repl_et() OWNER TO postgres; CREATE TRIGGER repl_et BEFORE INSERT ON $SQL_HILOW_TABLE_ET FOR EACH ROW EXECUTE PROCEDURE repl_et();" >> /tmp/commands.sql

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

    echo "CREATE OR REPLACE FUNCTION repl_uv() RETURNS trigger AS \$BODY\$ BEGIN IF (EXISTS(SELECT 1 FROM $SQL_HILOW_TABLE_UV C WHERE C.dateTime = NEW.dateTime)) THEN UPDATE $SQL_HILOW_TABLE_UV SET low = NEW.low, timeLow = NEW.timeLow, high = NEW.high, timeHigh = NEW.timeHigh, whenHigh = NEW.whenHigh, cumulative = NEW.cumulative, samples = NEW.samples WHERE dateTime = NEW.dateTime; RETURN OLD; ELSE RETURN NEW; END IF; END; \$BODY\$ LANGUAGE plpgsql VOLATILE COST 100; ALTER FUNCTION repl_uv() OWNER TO postgres; CREATE TRIGGER repl_uv BEFORE INSERT ON $SQL_HILOW_TABLE_UV FOR EACH ROW EXECUTE PROCEDURE repl_uv();" >> /tmp/commands.sql

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

    echo "CREATE OR REPLACE FUNCTION repl_solar() RETURNS trigger AS \$BODY\$ BEGIN IF (EXISTS(SELECT 1 FROM $SQL_HILOW_TABLE_SOLARRAD C WHERE C.dateTime = NEW.dateTime)) THEN UPDATE $SQL_HILOW_TABLE_SOLARRAD SET low = NEW.low, timeLow = NEW.timeLow, high = NEW.high, timeHigh = NEW.timeHigh, whenHigh = NEW.whenHigh, cumulative = NEW.cumulative, samples = NEW.samples WHERE dateTime = NEW.dateTime; RETURN OLD; ELSE RETURN NEW; END IF; END; \$BODY\$ LANGUAGE plpgsql VOLATILE COST 100; ALTER FUNCTION repl_solar() OWNER TO postgres; CREATE TRIGGER repl_solar BEFORE INSERT ON $SQL_HILOW_TABLE_SOLARRAD FOR EACH ROW EXECUTE PROCEDURE repl_solar();" >> /tmp/commands.sql

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

    echo "CREATE OR REPLACE FUNCTION repl_hail() RETURNS trigger AS \$BODY\$ BEGIN IF (EXISTS(SELECT 1 FROM $SQL_HILOW_TABLE_HAIL C WHERE C.dateTime = NEW.dateTime)) THEN UPDATE $SQL_HILOW_TABLE_HAIL SET low = NEW.low, timeLow = NEW.timeLow, high = NEW.high, timeHigh = NEW.timeHigh, whenHigh = NEW.whenHigh, cumulative = NEW.cumulative, samples = NEW.samples WHERE dateTime = NEW.dateTime; RETURN OLD; ELSE RETURN NEW; END IF; END; \$BODY\$ LANGUAGE plpgsql VOLATILE COST 100; ALTER FUNCTION repl_hail() OWNER TO postgres; CREATE TRIGGER repl_hail BEFORE INSERT ON $SQL_HILOW_TABLE_HAIL FOR EACH ROW EXECUTE PROCEDURE repl_hail();" >> /tmp/commands.sql

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

    echo "CREATE OR REPLACE FUNCTION repl_hailrate() RETURNS trigger AS \$BODY\$ BEGIN IF (EXISTS(SELECT 1 FROM $SQL_HILOW_TABLE_HAILRATE C WHERE C.dateTime = NEW.dateTime)) THEN UPDATE $SQL_HILOW_TABLE_HAILRATE SET low = NEW.low, timeLow = NEW.timeLow, high = NEW.high, timeHigh = NEW.timeHigh, whenHigh = NEW.whenHigh, cumulative = NEW.cumulative, samples = NEW.samples WHERE dateTime = NEW.dateTime; RETURN OLD; ELSE RETURN NEW; END IF; END; \$BODY\$ LANGUAGE plpgsql VOLATILE COST 100; ALTER FUNCTION repl_hailrate() OWNER TO postgres; CREATE TRIGGER repl_hailrate BEFORE INSERT ON $SQL_HILOW_TABLE_HAILRATE FOR EACH ROW EXECUTE PROCEDURE repl_hailrate();" >> /tmp/commands.sql

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

    echo "CREATE OR REPLACE FUNCTION repl_winddir() RETURNS trigger AS \$BODY\$ BEGIN IF (EXISTS(SELECT 1 FROM $SQL_HILOW_TABLE_WINDDIR C WHERE C.dateTime = NEW.dateTime)) THEN UPDATE $SQL_HILOW_TABLE_WINDDIR SET bin0 = NEW.bin0, bin1 = NEW.bin1, bin2 = NEW.bin2, bin3 = NEW.bin3, bin4 = NEW.bin4, bin5 = NEW.bin5, bin6 = NEW.bin6, bin7 = NEW.bin7, bin8 = NEW.bin8, bin9 = NEW.bin9, bin10 = NEW.bin10, bin11 = NEW.bin11, bin12 = NEW.bin12, bin13 = NEW.bin13, bin14 = NEW.bin14, bin15 = NEW.bin15, bin16 = NEW.bin16, bin17 = NEW.bin17, bin18 = NEW.bin18, bin19 = NEW.bin19 WHERE dateTime = NEW.dateTime; RETURN OLD; ELSE RETURN NEW; END IF; END; \$BODY\$ LANGUAGE plpgsql VOLATILE COST 100; ALTER FUNCTION repl_winddir() OWNER TO postgres; CREATE TRIGGER repl_winddir BEFORE INSERT ON $SQL_HILOW_TABLE_WINDDIR FOR EACH ROW EXECUTE PROCEDURE repl_winddir();" >> /tmp/commands.sql

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

    echo "CREATE OR REPLACE FUNCTION repl_noaa() RETURNS trigger AS \$BODY\$ BEGIN IF (EXISTS(SELECT 1 FROM $SQL_NOAA_TABLE C WHERE C.dateTime = NEW.dateTime)) THEN UPDATE $SQL_NOAA_TABLE SET meanTemp = NEW.meanTemp, highTemp = NEW.highTemp, highTempTime = NEW.highTempTime, lowTemp = NEW.lowTemp, lowTempTime = NEW.lowTempTime, heatDegDays = NEW.heatDegDays, coolDegDays = NEW.coolDegDays, rain = NEW.rain, avgWind = NEW.avgWind, highWind = NEW.highWind, highWindTime = NEW.highWindTime, domWindDir = NEW.domWindDir WHERE dateTime = NEW.dateTime; RETURN OLD; ELSE RETURN NEW; END IF; END; \$BODY\$ LANGUAGE plpgsql VOLATILE COST 100; ALTER FUNCTION repl_noaa() OWNER TO postgres; CREATE TRIGGER repl_noaa BEFORE INSERT ON $SQL_NOAA_TABLE FOR EACH ROW EXECUTE PROCEDURE repl_noaa();" >> /tmp/commands.sql

    psql $PGSQL_DBNAME < /tmp/commands.sql > /tmp/pgsql_create.log
    rm -rf /tmp/commands.sql
}

################################################################################
##################  S C R I P T  E X E C U T I O N  S T A R T  #################
################################################################################

mysql_create

## Done!
exit 0

