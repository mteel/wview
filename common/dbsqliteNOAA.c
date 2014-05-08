//----------------------------------------------------------------------------
//
//  FILENAME:
//        dbsqliteNOAA.c
//
//  PURPOSE:
//        Provide the weather station NOAA database utilities.
//
//  REVISION HISTORY:
//        Date            Engineer        Revision        Remarks
//        03/05/2009      M.S. Teel       0               Original
//
//  NOTES:
//
//
//  LICENSE:
//        Copyright (c) 2009, Mark S. Teel (mark@teel.ws)
//
//        This source code is released for free distribution under the terms
//        of the GNU General Public License.
//
//----------------------------------------------------------------------------

//  ... System include files
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <termios.h>
#include <time.h>
#include <errno.h>

//  ... Library include files
#include <radmsgLog.h>

//  ... Local include files
#include <dbsqlite.h>

#if (defined(BUILD_HTMLGEND) || defined(BUILD_UTILITIES))


//  ... local memory:

static SQLITE_DATABASE_ID   noaaDB = NULL;
static WAVG                 noaaWindAvg;


//  ... ----- static (local) methods -----

static const char* noaaGetDBFilename(void)
{
    static char     dbNOAAFilename[_MAX_PATH];

    if (strlen(dbsqliteArchiveGetPath()) > 0)
    {
        sprintf (dbNOAAFilename, "%s/%s", dbsqliteArchiveGetPath(), WVIEW_NOAA_DATABASE);
    }
    else
    {
        sprintf (dbNOAAFilename, "%s/%s", wvutilsGetArchivePath(), WVIEW_NOAA_DATABASE);
    }

    return dbNOAAFilename;
}

static int noaaExtractValues (SQLITE_DIRECT_ROW row, NOAA_DAY_REC* newRec)
{
    SQLITE_FIELD_ID         field;
    struct tm               bknTime;
    time_t                  tempTime;

    field = radsqlitedirectFieldGet(row, "meanTemp");
    if (field == NULL)
    {
        return ERROR;
    }
    else
    {
        newRec->meanTemp = (float)radsqliteFieldGetDoubleValue(field);
    }

    field = radsqlitedirectFieldGet(row, "highTemp");
    if (field == NULL)
    {
        return ERROR;
    }
    else
    {
        newRec->highTemp = (float)radsqliteFieldGetDoubleValue(field);
    }

    field = radsqlitedirectFieldGet(row, "highTempTime");
    if (field == NULL)
    {
        return ERROR;
    }
    else
    {
        tempTime = (time_t)radsqliteFieldGetBigIntValue(field);
        localtime_r(&tempTime, &bknTime);
        sprintf(newRec->highTempTime, "%2.2d:%2.2d",
                bknTime.tm_hour,
                bknTime.tm_min);
    }

    field = radsqlitedirectFieldGet(row, "lowTemp");
    if (field == NULL)
    {
        return ERROR;
    }
    else
    {
        newRec->lowTemp = (float)radsqliteFieldGetDoubleValue(field);
    }

    field = radsqlitedirectFieldGet(row, "lowTempTime");
    if (field == NULL)
    {
        return ERROR;
    }
    else
    {
        tempTime = (time_t)radsqliteFieldGetBigIntValue(field);
        localtime_r(&tempTime, &bknTime);
        sprintf(newRec->lowTempTime, "%2.2d:%2.2d",
                bknTime.tm_hour,
                bknTime.tm_min);
    }

    field = radsqlitedirectFieldGet(row, "heatDegDays");
    if (field == NULL)
    {
        return ERROR;
    }
    else
    {
        newRec->heatDegDays = (float)radsqliteFieldGetDoubleValue(field);
    }

    field = radsqlitedirectFieldGet(row, "coolDegDays");
    if (field == NULL)
    {
        return ERROR;
    }
    else
    {
        newRec->coolDegDays = (float)radsqliteFieldGetDoubleValue(field);
    }

    field = radsqlitedirectFieldGet(row, "rain");
    if (field == NULL)
    {
        return ERROR;
    }
    else
    {
        newRec->rain = (float)radsqliteFieldGetDoubleValue(field);
    }

    field = radsqlitedirectFieldGet(row, "avgWind");
    if (field == NULL)
    {
        return ERROR;
    }
    else
    {
        newRec->avgWind = (float)radsqliteFieldGetDoubleValue(field);
    }

    field = radsqlitedirectFieldGet(row, "highWind");
    if (field == NULL)
    {
        return ERROR;
    }
    else
    {
        newRec->highWind = (float)radsqliteFieldGetDoubleValue(field);
    }

    field = radsqlitedirectFieldGet(row, "highWindTime");
    if (field == NULL)
    {
        return ERROR;
    }
    else
    {
        tempTime = (time_t)radsqliteFieldGetBigIntValue(field);
        localtime_r(&tempTime, &bknTime);
        sprintf(newRec->highWindTime, "%2.2d:%2.2d",
                bknTime.tm_hour,
                bknTime.tm_min);
    }

    field = radsqlitedirectFieldGet(row, "domWindDir");
    if (field == NULL)
    {
        return ERROR;
    }
    else
    {
        newRec->domWindDir = (int)radsqliteFieldGetBigIntValue(field);
    }

    return OK;
}

static int noaaGetRecord (time_t dateTime, NOAA_DAY_REC* newRec)
{
    char                    query[DB_SQLITE_QUERY_LENGTH_MAX];
    SQLITE_DIRECT_ROW       row;

    if (noaaDB == NULL)
    {
        radMsgLog (PRI_HIGH, "noaaGetRecord: failed to open %s!", noaaGetDBFilename());
        return ERROR;
    }

    // grab the entire row:
    sprintf (query, "SELECT * FROM %s WHERE dateTime = '%d'", 
             WVIEW_NOAA_TABLE, (int)dateTime);

    // Execute the query:
    if (radsqlitedirectQuery(noaaDB, query, TRUE) == ERROR)
    {
        return ERROR;
    }

    row = radsqlitedirectGetRow(noaaDB);
    if (row == NULL)
    {
        radsqlitedirectReleaseResults(noaaDB);
        return ERROR;
    }

    // copy it to the internal sensor:
    if (noaaExtractValues(row, newRec) == ERROR)
    {
        radsqlitedirectReleaseResults(noaaDB);
        radMsgLog (PRI_MEDIUM, "noaaGetRecord: radsqlitedirectFieldGet failed!");
        return ERROR;
    }

    radsqlitedirectReleaseResults(noaaDB);
    return OK;
}

static int noaaDeleteRecord (time_t dateTime)
{
    char                    query[DB_SQLITE_QUERY_LENGTH_MAX];

    if (noaaDB == NULL)
    {
        radMsgLog (PRI_HIGH, "noaaGetRecord: failed to open %s!", noaaGetDBFilename());
        return ERROR;
    }

    // delete the entire row:
    sprintf (query, "DELETE FROM %s WHERE dateTime = '%d'", 
             WVIEW_NOAA_TABLE, (int)dateTime);

    // Execute the query:
    if (radsqlitedirectQuery(noaaDB, query, FALSE) == ERROR)
    {
        return ERROR;
    }

    return OK;
}

static int noaaInsertData(time_t timestamp, SENSOR_STORE* sensorStore)
{
    SQLITE_ROW_ID           row;
    SQLITE_FIELD_ID         field;
    time_t                  noaaDayTime;
    struct tm               bknTime;
    NOAA_DAY_REC            store;
    char                    query[DB_SQLITE_QUERY_LENGTH_MAX];
    double                  tempd, sum;
    int                     tempint;
    WV_SENSOR*              sensors = sensorStore->sensor[STF_DAY];

    noaaDayTime = timestamp;
    localtime_r(&noaaDayTime, &bknTime);
    bknTime.tm_hour  = 0;
    bknTime.tm_min   = 0;
    bknTime.tm_sec   = 0;
    bknTime.tm_isdst = -1;
    noaaDayTime = mktime(&bknTime);

    // First see if the record exists:
    if (noaaGetRecord(noaaDayTime, &store) == OK)
    {
        // Delete so we can replace it:
        noaaDeleteRecord(noaaDayTime);
    }

    // create a new record:
    row = radsqliteTableDescriptionGet(noaaDB, WVIEW_NOAA_TABLE);
    if (row == NULL)
    {
        radMsgLog (PRI_HIGH, "dbsqliteNOAA: databaseTableDescriptionGet failed!");
        return ERROR;
    }

    field = radsqliteFieldGet(row, "dateTime");
    if (field == NULL)
    {
        radMsgLog (PRI_HIGH, "dbsqliteNOAA: radsqliteFieldGet failed!");
        radsqliteRowDescriptionDelete(row);
        return ERROR;
    }
    else
    {
        radsqliteFieldSetBigIntValue(field, (uint64_t)noaaDayTime);
    }

    field = radsqliteFieldGet(row, "meanTemp");
    if (field == NULL)
    {
        radMsgLog (PRI_HIGH, "dbsqliteNOAA: radsqliteFieldGet failed!");
        radsqliteRowDescriptionDelete(row);
        return ERROR;
    }
    else
    {
        if (sensors[SENSOR_OUTTEMP].samples > 0)
        {
            tempd = (double)sensors[SENSOR_OUTTEMP].cumulative;
            tempd /= (double)sensors[SENSOR_OUTTEMP].samples;
            radsqliteFieldSetDoubleValue(field, tempd);
        }
        else
        {
            radsqliteFieldSetToNull(field);
        }
    }

    field = radsqliteFieldGet(row, "highTemp");
    if (field == NULL)
    {
        radMsgLog (PRI_HIGH, "dbsqliteNOAA: radsqliteFieldGet failed!");
        radsqliteRowDescriptionDelete(row);
        return ERROR;
    }
    else
    {
        if (sensors[SENSOR_OUTTEMP].samples > 0)
        {
            tempd = (double)sensors[SENSOR_OUTTEMP].high;
            radsqliteFieldSetDoubleValue(field, tempd);
        }
        else
        {
            radsqliteFieldSetToNull(field);
        }
    }

    field = radsqliteFieldGet(row, "highTempTime");
    if (field == NULL)
    {
        radMsgLog (PRI_HIGH, "dbsqliteNOAA: radsqliteFieldGet failed!");
        radsqliteRowDescriptionDelete(row);
        return ERROR;
    }
    else
    {
        if (sensors[SENSOR_OUTTEMP].time_high > 0)
        {
            radsqliteFieldSetBigIntValue(field, 
                                         (uint64_t)sensors[SENSOR_OUTTEMP].time_high);
        }
        else
        {
            radsqliteFieldSetToNull(field);
        }
    }

    field = radsqliteFieldGet(row, "lowTemp");
    if (field == NULL)
    {
        radMsgLog (PRI_HIGH, "dbsqliteNOAA: radsqliteFieldGet failed!");
        radsqliteRowDescriptionDelete(row);
        return ERROR;
    }
    else
    {
        if (sensors[SENSOR_OUTTEMP].samples > 0)
        {
            tempd = (double)sensors[SENSOR_OUTTEMP].low;
            radsqliteFieldSetDoubleValue(field, tempd);
        }
        else
        {
            radsqliteFieldSetToNull(field);
        }
    }

    field = radsqliteFieldGet(row, "lowTempTime");
    if (field == NULL)
    {
        radMsgLog (PRI_HIGH, "dbsqliteNOAA: radsqliteFieldGet failed!");
        radsqliteRowDescriptionDelete(row);
        return ERROR;
    }
    else
    {
        if (sensors[SENSOR_OUTTEMP].time_low > 0)
        {
            radsqliteFieldSetBigIntValue(field, 
                                         (uint64_t)sensors[SENSOR_OUTTEMP].time_low);
        }
        else
        {
            radsqliteFieldSetToNull(field);
        }
    }

    field = radsqliteFieldGet(row, "heatDegDays");
    if (field == NULL)
    {
        radMsgLog (PRI_HIGH, "dbsqliteNOAA: radsqliteFieldGet failed!");
        radsqliteRowDescriptionDelete(row);
        return ERROR;
    }
    else
    {
        if (sensors[SENSOR_OUTTEMP].samples > 0)
        {
            sum = (sensors[SENSOR_OUTTEMP].high + sensors[SENSOR_OUTTEMP].low)/2;
            tempd = ((sum < 65) ? 65 - sum : 0);
            radsqliteFieldSetDoubleValue(field, tempd);
        }
        else
        {
            radsqliteFieldSetToNull(field);
        }
    }

    field = radsqliteFieldGet(row, "coolDegDays");
    if (field == NULL)
    {
        radMsgLog (PRI_HIGH, "dbsqliteNOAA: radsqliteFieldGet failed!");
        radsqliteRowDescriptionDelete(row);
        return ERROR;
    }
    else
    {
        if (sensors[SENSOR_OUTTEMP].samples > 0)
        {
            sum = (sensors[SENSOR_OUTTEMP].high + sensors[SENSOR_OUTTEMP].low)/2;
            tempd = ((sum > 65) ? sum - 65 : 0);
            radsqliteFieldSetDoubleValue(field, tempd);
        }
        else
        {
            radsqliteFieldSetToNull(field);
        }
    }

    field = radsqliteFieldGet(row, "rain");
    if (field == NULL)
    {
        radMsgLog (PRI_HIGH, "dbsqliteNOAA: radsqliteFieldGet failed!");
        radsqliteRowDescriptionDelete(row);
        return ERROR;
    }
    else
    {
        if (sensors[SENSOR_RAIN].samples > 0)
        {
            tempd = (double)sensors[SENSOR_RAIN].cumulative;
            radsqliteFieldSetDoubleValue(field, tempd);
        }
        else
        {
            radsqliteFieldSetToNull(field);
        }
    }

    field = radsqliteFieldGet(row, "avgWind");
    if (field == NULL)
    {
        radMsgLog (PRI_HIGH, "dbsqliteNOAA: radsqliteFieldGet failed!");
        radsqliteRowDescriptionDelete(row);
        return ERROR;
    }
    else
    {
        if (sensors[SENSOR_WSPEED].samples > 0)
        {
            tempd = (double)sensors[SENSOR_WSPEED].cumulative/sensors[SENSOR_WSPEED].samples;
            radsqliteFieldSetDoubleValue(field, tempd);
        }
        else
        {
            radsqliteFieldSetToNull(field);
        }
    }

    field = radsqliteFieldGet(row, "highWind");
    if (field == NULL)
    {
        radMsgLog (PRI_HIGH, "dbsqliteNOAA: radsqliteFieldGet failed!");
        radsqliteRowDescriptionDelete(row);
        return ERROR;
    }
    else
    {
        if (sensors[SENSOR_WGUST].samples > 0)
        {
            tempd = (double)sensors[SENSOR_WGUST].high;
            radsqliteFieldSetDoubleValue(field, tempd);
        }
        else
        {
            radsqliteFieldSetToNull(field);
        }
    }

    field = radsqliteFieldGet(row, "highWindTime");
    if (field == NULL)
    {
        radMsgLog (PRI_HIGH, "dbsqliteNOAA: radsqliteFieldGet failed!");
        radsqliteRowDescriptionDelete(row);
        return ERROR;
    }
    else
    {
        if (sensors[SENSOR_WGUST].time_high > 0)
        {
            radsqliteFieldSetBigIntValue(field, 
                                         (uint64_t)sensors[SENSOR_WGUST].time_high);
        }
        else
        {
            radsqliteFieldSetToNull(field);
        }
    }

    field = radsqliteFieldGet(row, "domWindDir");
    if (field == NULL)
    {
        radMsgLog (PRI_HIGH, "dbsqliteNOAA: radsqliteFieldGet failed!");
        radsqliteRowDescriptionDelete(row);
        return ERROR;
    }
    else
    {
        if (sensors[SENSOR_WSPEED].samples > 0)
        {
            tempint = windAverageCompute(&sensorStore->wind[STF_DAY]);
            radsqliteFieldSetBigIntValue(field, (uint64_t)tempint);
        }
        else
        {
            radsqliteFieldSetToNull(field);
        }
    }

    // insert the row:
    if (radsqliteTableInsertRow(noaaDB, WVIEW_NOAA_TABLE, row) == ERROR)
    {
        radMsgLog (PRI_HIGH, "dbsqlite: radsqliteTableInsertRow (hilow) failed!");
        radsqliteRowDescriptionDelete(row);
        return ERROR;
    }

    radsqliteRowDescriptionDelete(row);
    return OK;
}

static time_t noaaGetFirstUpdateDay (void)
{
    char                    query[DB_SQLITE_QUERY_LENGTH_MAX];
    SQLITE_DIRECT_ROW       rowDescr;
    SQLITE_FIELD_ID         field;
    time_t                  retVal;

    if (noaaDB == NULL)
    {
        radMsgLog (PRI_HIGH, "getNewestDateTime: failed to open %s!", noaaGetDBFilename());
        return ERROR;
    }

    // Build the query:
    sprintf (query, "SELECT MIN(dateTime) AS 'min' FROM %s", WVIEW_NOAA_TABLE);

    // Execute the query:
    if (radsqlitedirectQuery(noaaDB, query, TRUE) == ERROR)
    {
        radMsgLog (PRI_HIGH, "getNewestDateTime: radsqlitedirectQuery failed!");
        return ERROR;
    }

    rowDescr = radsqlitedirectGetRow(noaaDB);
    if (rowDescr == NULL)
    {
        radMsgLog (PRI_MEDIUM, 
                   "getNewestDateTime: radsqlitedirectGetRow failed!");
        radsqlitedirectReleaseResults(noaaDB);
        return ERROR;
    }

    field = radsqlitedirectFieldGet(rowDescr, "min");
    if (field == NULL)
    {
        radMsgLog (PRI_MEDIUM, 
                   "getNewestDateTime: radsqlitedirectFieldGet failed!");
        radsqlitedirectReleaseResults(noaaDB);
        return ERROR;
    }

    retVal = (time_t)radsqliteFieldGetBigIntValue(field);

    // Clean up:
    radsqlitedirectReleaseResults(noaaDB);

    return retVal;
}

static time_t noaaGetLastUpdateDay (void)
{
    char                    query[DB_SQLITE_QUERY_LENGTH_MAX];
    SQLITE_DIRECT_ROW       rowDescr;
    SQLITE_FIELD_ID         field;
    time_t                  retVal;

    if (noaaDB == NULL)
    {
        radMsgLog (PRI_HIGH, "noaaGetLastUpdateDay: failed to open %s!", noaaGetDBFilename());
        return ERROR;
    }

    // Build the query:
    sprintf (query, "SELECT MAX(dateTime) AS 'max' FROM %s", WVIEW_NOAA_TABLE);

    // Execute the query:
    if (radsqlitedirectQuery(noaaDB, query, TRUE) == ERROR)
    {
        radMsgLog (PRI_HIGH, "noaaGetLastUpdateDay: radsqlitedirectQuery failed!");
        return ERROR;
    }

    rowDescr = radsqlitedirectGetRow(noaaDB);
    if (rowDescr == NULL)
    {
        radMsgLog (PRI_MEDIUM, 
                   "noaaGetLastUpdateDay: radsqlitedirectGetRow failed!");
        radsqlitedirectReleaseResults(noaaDB);
        return ERROR;
    }

    field = radsqlitedirectFieldGet(rowDescr, "max");
    if (field == NULL)
    {
        radMsgLog (PRI_MEDIUM, 
                   "noaaGetLastUpdateDay: radsqlitedirectFieldGet failed!");
        radsqlitedirectReleaseResults(noaaDB);
        return ERROR;
    }

    retVal = (time_t)radsqliteFieldGetBigIntValue(field);

    // Clean up:
    radsqlitedirectReleaseResults(noaaDB);

    return retVal;
}


////////////////////////////////////////////////////////////////////////////////

// Initialize the NOAA database (returns OK or ERROR):
int dbsqliteNOAAInit(void)
{
    SQLITE_ROW_ID       rowDesc, newrow;
    SQLITE_FIELD_ID     field;
    SENSOR_TYPES        index;
    int                 retVal, numrecs = 0, numNOAARecs = 0;
    int                 i, done = FALSE;
    char                tableName[64];
    ARCHIVE_PKT         archiveRec;
    SENSOR_STORE        sensorStore;
    time_t              archiveTime, startTime, stopTime, hilowTime;
    char                binName[16];
    struct tm           bknTime;
    time_t              LastNOAAUpdateTime, LastArchiveTime;

    noaaDB = radsqliteOpen(noaaGetDBFilename());
    if (noaaDB == NULL)
    {
        radMsgLog (PRI_HIGH, "dbsqliteNOAAInit: failed to open %s!", noaaGetDBFilename());
        return ERROR;
    }


    // Does the NOAA table exist?
    if (! radsqliteTableIfExists(noaaDB, WVIEW_NOAA_TABLE))
    {
        // Make writes faster (and less safe) by avoiding fsyncs:
        dbsqliteNOAAPragmaSet("synchronous", "off");

        // We need to create the table:
        // Define the row first:
        rowDesc = radsqliteRowDescriptionCreate();
        if (rowDesc == NULL)
        {
            radsqliteClose(noaaDB);
            radMsgLog(PRI_HIGH, "dbsqliteNOAAInit: radsqliteRowDescriptionCreate failed!");
            return ERROR;
        }
    
        // Populate the table:
        retVal = radsqliteRowDescriptionAddField(rowDesc, 
                                                 "dateTime", 
                                                 SQLITE_FIELD_BIGINT | SQLITE_FIELD_PRI_KEY, 
                                                 0);
        if (retVal == ERROR)
        {
            radsqliteClose(noaaDB);
            radMsgLog(PRI_HIGH, "dbsqliteNOAAInit: databaseRowDescriptionAddField failed!");
            radsqliteRowDescriptionDelete(rowDesc);
            return ERROR;
        }

        retVal = radsqliteRowDescriptionAddField(rowDesc, "meanTemp", SQLITE_FIELD_DOUBLE, 0);
        if (retVal == ERROR)
        {
            radsqliteClose(noaaDB);
            radMsgLog(PRI_HIGH, "dbsqliteNOAAInit: databaseRowDescriptionAddField failed!");
            radsqliteRowDescriptionDelete(rowDesc);
            return ERROR;
        }
        retVal = radsqliteRowDescriptionAddField(rowDesc, "highTemp", SQLITE_FIELD_DOUBLE, 0);
        if (retVal == ERROR)
        {
            radsqliteClose(noaaDB);
            radMsgLog(PRI_HIGH, "dbsqliteNOAAInit: databaseRowDescriptionAddField failed!");
            radsqliteRowDescriptionDelete(rowDesc);
            return ERROR;
        }
        retVal = radsqliteRowDescriptionAddField(rowDesc, "highTempTime", SQLITE_FIELD_BIGINT, 0);
        if (retVal == ERROR)
        {
            radsqliteClose(noaaDB);
            radMsgLog(PRI_HIGH, "dbsqliteNOAAInit: databaseRowDescriptionAddField failed!");
            radsqliteRowDescriptionDelete(rowDesc);
            return ERROR;
        }
        retVal = radsqliteRowDescriptionAddField(rowDesc, "lowTemp", SQLITE_FIELD_DOUBLE, 0);
        if (retVal == ERROR)
        {
            radsqliteClose(noaaDB);
            radMsgLog(PRI_HIGH, "dbsqliteNOAAInit: databaseRowDescriptionAddField failed!");
            radsqliteRowDescriptionDelete(rowDesc);
            return ERROR;
        }
        retVal = radsqliteRowDescriptionAddField(rowDesc, "lowTempTime", SQLITE_FIELD_BIGINT, 0);
        if (retVal == ERROR)
        {
            radsqliteClose(noaaDB);
            radMsgLog(PRI_HIGH, "dbsqliteNOAAInit: databaseRowDescriptionAddField failed!");
            radsqliteRowDescriptionDelete(rowDesc);
            return ERROR;
        }
        retVal = radsqliteRowDescriptionAddField(rowDesc, "heatDegDays", SQLITE_FIELD_DOUBLE, 0);
        if (retVal == ERROR)
        {
            radsqliteClose(noaaDB);
            radMsgLog(PRI_HIGH, "dbsqliteNOAAInit: databaseRowDescriptionAddField failed!");
            radsqliteRowDescriptionDelete(rowDesc);
            return ERROR;
        }
        retVal = radsqliteRowDescriptionAddField(rowDesc, "coolDegDays", SQLITE_FIELD_DOUBLE, 0);
        if (retVal == ERROR)
        {
            radsqliteClose(noaaDB);
            radMsgLog(PRI_HIGH, "dbsqliteNOAAInit: databaseRowDescriptionAddField failed!");
            radsqliteRowDescriptionDelete(rowDesc);
            return ERROR;
        }
        retVal = radsqliteRowDescriptionAddField(rowDesc, "rain", SQLITE_FIELD_DOUBLE, 0);
        if (retVal == ERROR)
        {
            radsqliteClose(noaaDB);
            radMsgLog(PRI_HIGH, "dbsqliteNOAAInit: databaseRowDescriptionAddField failed!");
            radsqliteRowDescriptionDelete(rowDesc);
            return ERROR;
        }
        retVal = radsqliteRowDescriptionAddField(rowDesc, "avgWind", SQLITE_FIELD_DOUBLE, 0);
        if (retVal == ERROR)
        {
            radsqliteClose(noaaDB);
            radMsgLog(PRI_HIGH, "dbsqliteNOAAInit: databaseRowDescriptionAddField failed!");
            radsqliteRowDescriptionDelete(rowDesc);
            return ERROR;
        }
        retVal = radsqliteRowDescriptionAddField(rowDesc, "highWind", SQLITE_FIELD_DOUBLE, 0);
        if (retVal == ERROR)
        {
            radsqliteClose(noaaDB);
            radMsgLog(PRI_HIGH, "dbsqliteNOAAInit: databaseRowDescriptionAddField failed!");
            radsqliteRowDescriptionDelete(rowDesc);
            return ERROR;
        }
        retVal = radsqliteRowDescriptionAddField(rowDesc, "highWindTime", SQLITE_FIELD_BIGINT, 0);
        if (retVal == ERROR)
        {
            radsqliteClose(noaaDB);
            radMsgLog(PRI_HIGH, "dbsqliteNOAAInit: databaseRowDescriptionAddField failed!");
            radsqliteRowDescriptionDelete(rowDesc);
            return ERROR;
        }
        retVal = radsqliteRowDescriptionAddField(rowDesc, "domWindDir", SQLITE_FIELD_BIGINT, 0);
        if (retVal == ERROR)
        {
            radsqliteClose(noaaDB);
            radMsgLog(PRI_HIGH, "dbsqliteNOAAInit: databaseRowDescriptionAddField failed!");
            radsqliteRowDescriptionDelete(rowDesc);
            return ERROR;
        }


        // Now create the table:
        if (radsqliteTableCreate(noaaDB, WVIEW_NOAA_TABLE, rowDesc) == ERROR)
        {
            radsqliteClose(noaaDB);
            radMsgLog(PRI_HIGH, "dbsqliteNOAAInit: radsqliteTableCreate failed!");
            radsqliteRowDescriptionDelete(rowDesc);
            return ERROR;
        }

        // We're done with this table:
        radsqliteRowDescriptionDelete(rowDesc);

        radMsgLog(PRI_STATUS, "NOAA DB: %s table created", WVIEW_NOAA_TABLE);


        // OK, if we had to create the table, assume it should be completely populated:
        startTime = dbsqliteArchiveGetNextRecord(0, &archiveRec);
        if (startTime == (time_t)ERROR)
        {
            radMsgLog(PRI_STATUS, "NOAA DB: Archive table is empty - starting with new station...");
        }
        else
        {
            // Back fill the table:
            localtime_r(&startTime, &bknTime);
            bknTime.tm_hour = 4;            // Avoid DST issues
            bknTime.tm_min = 0;
            bknTime.tm_sec = 0;
            bknTime.tm_isdst = -1;
            startTime = mktime(&bknTime);
    
            stopTime = time(NULL);
            localtime_r(&stopTime, &bknTime);
            bknTime.tm_hour = 0;
            bknTime.tm_min = 0;
            bknTime.tm_sec = 0;
            bknTime.tm_isdst = -1;
            stopTime = mktime(&bknTime);
    
            radMsgLog(PRI_STATUS, "NOAA DB: back filling tables with ALL HILOW data");
            radMsgLog(PRI_STATUS, "NOAA DB: syncing %4.4d%2.2d%2.2d => %4.4d%2.2d%2.2d",
                      wvutilsGetYear(startTime), 
                      wvutilsGetMonth(startTime), 
                      wvutilsGetDay(startTime),
                      wvutilsGetYear(stopTime-1), 
                      wvutilsGetMonth(stopTime-1),
                      wvutilsGetDay(stopTime-1));
            radMsgLog(PRI_STATUS, "NOAA DB: (this may take a while ...)");
    
            // Loop through all days:
            while (startTime < stopTime)
            {
                localtime_r(&startTime, &bknTime);
                bknTime.tm_hour = 0;
                bknTime.tm_min = 0;
                bknTime.tm_sec = 0;
                bknTime.tm_isdst = -1;
                hilowTime = mktime(&bknTime);
                
                windAverageReset(&sensorStore.wind[STF_DAY]);
                sensorClearSet(sensorStore.sensor[STF_DAY]);
    
                retVal = dbsqliteHiLowGetDay(hilowTime, &sensorStore, STF_DAY);
                if (retVal == ERROR)
                {
                    return ERROR;
                }
                else if (retVal == 0)
                {
                    // Continue in case there is a gap:
                    startTime += WV_SECONDS_IN_DAY;
                    continue;
                }
                else
                {
                    // Write a record:
                    numrecs += retVal;
    
                    if (noaaInsertData(startTime, &sensorStore) == OK)
                    {
                        numNOAARecs ++;
                    }
    
                    startTime += WV_SECONDS_IN_DAY;
                }
            }
    
            radMsgLog(PRI_STATUS, "NOAA DB: done: %d HILOW records => %d NOAA records",
                      numrecs, numNOAARecs);
        }
    }
    else
    {
        radMsgLog(PRI_STATUS, "NOAA DB: OK");
    }

    // Set normal syncing behavior:
    dbsqliteNOAAPragmaSet("synchronous", "normal");

    return OK;
}

void dbsqliteNOAAExit(void)
{
    if (noaaDB)
        radsqliteClose(noaaDB);
}

// set a PRAGMA to modify the operation of the SQLite library:
// Returns: OK or ERROR
int dbsqliteNOAAPragmaSet(char* pragma, char* setting)
{
    char        query[DB_SQLITE_QUERY_LENGTH_MAX];

    // Check SQLite version if a journalling pragma:
    if (!strcmp(pragma, "journal_mode"))
    {
        if (SQLITE_VERSION_NUMBER < 3005009)
        {
            // Not supported:
            return OK;
        }
    }

    sprintf (query, "PRAGMA %s = %s", pragma, setting);

    // Execute the query:
    if (radsqliteQuery(noaaDB, query, FALSE) == ERROR)
    {
        return ERROR;
    }

    return OK;
}

//  ... dbsqliteNOAAGetDay: summarize day records for NOAA reports;
//  ... this will zero out the NOAA_DAY_REC store before beginning;
//  ... returns OK or ERROR if day not found in archives
int dbsqliteNOAAGetDay
(
    NOAA_DAY_REC    *store,
    time_t          day
)
{
    int             retVal, year, month;
    struct tm       locTime;
    time_t          ntime;

    localtime_r (&day, &locTime);
    locTime.tm_hour  = 0;
    locTime.tm_sec   = 0;
    locTime.tm_isdst = -1;
    ntime = mktime(&locTime);

    // MUST be done here - noaaGetRecord expects it!
    memset (store, 0, sizeof (NOAA_DAY_REC));

    store->year     = locTime.tm_year + 1900;
    store->month    = locTime.tm_mon + 1;
    store->day      = locTime.tm_mday;

    retVal = noaaGetRecord(ntime, store);
    return retVal;
}

int dbsqliteNOAAComputeNorms (float *temps, float *rains, float *yearTemp, float *yearRain)
{
    NOAA_DAY_REC    record;
    int             i, numDays[13], numMonths[13];
    float           tempSum[13], rainSum[13];
    int             thismonth, lastmonth = -1;
    time_t          ntime, firstDay, lastDay;
    struct tm       locTime;

    memset (temps, 0, 13 * sizeof (float));
    memset (rains, 0, 13 * sizeof (float));
    memset (numDays, 0, 13 * sizeof (int));
    memset (numMonths, 0, 13 * sizeof (int));
    memset (tempSum, 0, 13 * sizeof (float));
    memset (rainSum, 0, 13 * sizeof (float));
    *yearTemp = *yearRain = 0.0;

    firstDay = noaaGetFirstUpdateDay();
    lastDay = noaaGetLastUpdateDay();
    if ((int)firstDay == ERROR || (int)lastDay == ERROR || 
        (int)firstDay == 0 || (int)lastDay == 0)
    {
        return ERROR;
    }

    for (ntime = firstDay; ntime <= lastDay; ntime += WV_SECONDS_IN_DAY)
    {
        if (dbsqliteNOAAGetDay(&record, ntime) == ERROR)
        {
            continue;
        }

        thismonth = wvutilsGetMonth(ntime);
        tempSum[thismonth] += record.meanTemp;
        rainSum[thismonth] += record.rain;
        numDays[thismonth] ++;
        if (thismonth != lastmonth)
        {
            numMonths[thismonth] ++;
        }
        lastmonth = thismonth;
    }

    // Now make sense of it all:
    for (i = 1; i < 13; i ++)
    {
        if (numDays[i] > 0)
        {
            temps[i] = tempSum[i]/numDays[i];
            rains[i] = rainSum[i]/numMonths[i];
        }
        else
        {
            temps[i] = rains[i] = 0;
        }

        *yearTemp += temps[i];
        *yearRain += rains[i];
    }

    *yearTemp /= 12;

    return OK;
}

void dbsqliteNOAAUpdate (void)
{
    time_t          ntime, lastNOAARecTime, nowDay, lastInsertTime = 0;
    struct tm       locTime;
    SENSOR_STORE    sensorStore;
    int             retVal, numrecs = 0, numNOAARecs = 0;
    char            fileName[128];
    ARCHIVE_PKT     archiveRec;

    nowDay = time(NULL);
    localtime_r(&nowDay, &locTime);
    locTime.tm_hour  = 0;
    locTime.tm_min   = 0;
    locTime.tm_sec   = 0;
    locTime.tm_isdst = -1;
    nowDay = mktime(&locTime);

    lastNOAARecTime = noaaGetLastUpdateDay();
    if ((int)lastNOAARecTime == 0)
    {
        // Use the first archive time:
        lastNOAARecTime = dbsqliteArchiveGetNextRecord(0, &archiveRec);
        if (lastNOAARecTime == (time_t)ERROR)
        {
            radMsgLog(PRI_HIGH, "dbsqliteNOAAUpdate: Archive database empty - nothing to do...");
            return;
        }
        
        lastNOAARecTime -= WV_SECONDS_IN_DAY;
        localtime_r(&lastNOAARecTime, &locTime);
        locTime.tm_hour  = 0;
        locTime.tm_min   = 0;
        locTime.tm_sec   = 0;
        locTime.tm_isdst = -1;
        lastNOAARecTime = mktime(&locTime);
    }

    ntime = lastNOAARecTime + WV_SECONDS_IN_DAY;

    // Is there any work to do?
    if ((ntime + WV_SECONDS_IN_DAY) <= nowDay)
    {
        // yes!
        radMsgLog(PRI_STATUS, "NOAA DB: syncing %4.4d%2.2d%2.2d => %4.4d%2.2d%2.2d",
                  wvutilsGetYear(ntime), 
                  wvutilsGetMonth(ntime), 
                  wvutilsGetDay(ntime),
                  wvutilsGetYear(nowDay-WV_SECONDS_IN_DAY), 
                  wvutilsGetMonth(nowDay-WV_SECONDS_IN_DAY),
                  wvutilsGetDay(nowDay-WV_SECONDS_IN_DAY));

        // Loop through all days:
        while (ntime < nowDay)
        {
            // Don't create a record if we have advanced to "today":
            if (! wvutilsTimeIsToday(ntime))
            {
                windAverageReset(&sensorStore.wind[STF_DAY]);
                sensorClearSet(sensorStore.sensor[STF_DAY]);
    
                retVal = dbsqliteHiLowGetDay(ntime, &sensorStore, STF_DAY);
                if (retVal > 0)
                {
                    // Write a record:
                    numrecs += retVal;
    
                    if (noaaInsertData(ntime, &sensorStore) == OK)
                    {
                        numNOAARecs ++;
                        lastInsertTime = ntime;
                    }
                }
            }

            ntime += WV_SECONDS_IN_DAY;
        }

        if (numrecs > 0)
        {
            radMsgLog(PRI_STATUS, "NOAA DB: done: %d HILOW records => %d NOAA records",
                      numrecs, numNOAARecs);
        }

        if (lastInsertTime != 0)
        {
            sprintf (fileName, "%s/export/%s", wvutilsGetConfigPath(), WVIEW_NOAA_MARKER_FILE);
            wvutilsWriteMarkerFile(fileName, lastInsertTime);
        }
    }
    else
    {
        radMsgLog(PRI_STATUS, "NOAA DB: nothing to do: last NOAA %u, today %u",
                  ntime-WV_SECONDS_IN_DAY, nowDay);
    }

    return;
}

#endif

