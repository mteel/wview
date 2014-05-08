//----------------------------------------------------------------------------
//
//  FILENAME:
//        dbsqliteHiLow.c
//
//  PURPOSE:
//        Provide the weather station HILOW database utilities.
//
//  REVISION HISTORY:
//        Date            Engineer        Revision        Remarks
//        08/31/2008      M.S. Teel       0               Original
//
//  NOTES:
//
//
//  LICENSE:
//        Copyright (c) 2008, Mark S. Teel (mark@teel.ws)
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


//  ... local memory:

static SQLITE_DATABASE_ID   hilowDB = NULL;

static char*                sensorTables[SENSOR_MAX] =
{
    "inTemp",
    "outTemp",
    "inHumidity",
    "outHumidity",
    "baromPressure",
    "windSpeed",
    "windGust",
    "dewPoint",
    "rain",
    "rainRate",
    "windChill",
    "heatIndex",
    "ET",
    "UV",
    "solarRadiation",
    "hail",
    "hailRate"
};


//  ... ----- static (local) methods -----

static const char* hilowGetDBFilename(void)
{
    static char     dbHiLowFileName[_MAX_PATH];

    if (strlen(dbsqliteArchiveGetPath()) > 0)
    {
        sprintf (dbHiLowFileName, "%s/%s", dbsqliteArchiveGetPath(), WVIEW_HILOW_DATABASE);
    }
    else
    {
        sprintf (dbHiLowFileName, "%s/%s", wvutilsGetArchivePath(), WVIEW_HILOW_DATABASE);
    }

    return dbHiLowFileName;
}

static int hilowExtractValues (SQLITE_DIRECT_ROW row, WV_SENSOR* newRec)
{
    SQLITE_FIELD_ID         field;

    field = radsqlitedirectFieldGet(row, "low");
    if (field == NULL)
    {
        return ERROR;
    }
    else
    {
        newRec->low = (float)radsqliteFieldGetDoubleValue(field);
    }

    field = radsqlitedirectFieldGet(row, "timeLow");
    if (field == NULL)
    {
        return ERROR;
    }
    else
    {
        newRec->time_low = (time_t)radsqliteFieldGetBigIntValue(field);
    }

    field = radsqlitedirectFieldGet(row, "high");
    if (field == NULL)
    {
        return ERROR;
    }
    else
    {
        newRec->high = (float)radsqliteFieldGetDoubleValue(field);
    }

    field = radsqlitedirectFieldGet(row, "timeHigh");
    if (field == NULL)
    {
        return ERROR;
    }
    else
    {
        newRec->time_high = (time_t)radsqliteFieldGetBigIntValue(field);
    }

    field = radsqlitedirectFieldGet(row, "whenHigh");
    if (field == NULL)
    {
        return ERROR;
    }
    else
    {
        newRec->when_high = (float)radsqliteFieldGetDoubleValue(field);
    }

    field = radsqlitedirectFieldGet(row, "cumulative");
    if (field == NULL)
    {
        return ERROR;
    }
    else
    {
        newRec->cumulative = (float)radsqliteFieldGetDoubleValue(field);
    }

    field = radsqlitedirectFieldGet(row, "samples");
    if (field == NULL)
    {
        return ERROR;
    }
    else
    {
        newRec->samples = (int)radsqliteFieldGetBigIntValue(field);
    }

    return OK;
}

static int hilowGetRecord (time_t dateTime, SENSOR_TYPES type, WV_SENSOR* newRec)
{
    char                    query[DB_SQLITE_QUERY_LENGTH_MAX];
    SQLITE_DIRECT_ROW       row;

    if (hilowDB == NULL)
    {
        radMsgLog (PRI_HIGH, "hilowGetRecord: failed to open %s!", hilowGetDBFilename());
        return ERROR;
    }

    // grab the entire row:
    sprintf (query, "SELECT * FROM %s WHERE dateTime = '%d'", 
             sensorTables[type], (int)dateTime);

    // Execute the query:
    if (radsqlitedirectQuery(hilowDB, query, TRUE) == ERROR)
    {
        return ERROR;
    }

    row = radsqlitedirectGetRow(hilowDB);
    if (row == NULL)
    {
        radsqlitedirectReleaseResults(hilowDB);
        return ERROR;
    }

    // copy it to the internal sensor:
    if (hilowExtractValues(row, newRec) == ERROR)
    {
        radsqlitedirectReleaseResults(hilowDB);
        radMsgLog (PRI_MEDIUM, "hilowGetRecord: radsqlitedirectFieldGet failed!");
        return ERROR;
    }

    radsqlitedirectReleaseResults(hilowDB);
    return OK;
}

static int hilowGetWindRecord (time_t dateTime, int binIndex)
{
    char                    query[DB_SQLITE_QUERY_LENGTH_MAX];
    SQLITE_DIRECT_ROW       row;
    SQLITE_FIELD_ID         field;
    char                    binName[16];
    int                     retVal;

    if (hilowDB == NULL)
    {
        radMsgLog (PRI_HIGH, "hilowGetWindRecord: failed to open %s!", hilowGetDBFilename());
        return ERROR;
    }

    // grab the entire row:
    sprintf (query, "SELECT * FROM %s WHERE dateTime = '%d'", 
             WVIEW_HILOW_WINDDIR_TABLE, (int)dateTime);

    // Execute the query:
    if (radsqlitedirectQuery(hilowDB, query, TRUE) == ERROR)
    {
        return ERROR;
    }

    row = radsqlitedirectGetRow(hilowDB);
    if (row == NULL)
    {
        radsqlitedirectReleaseResults(hilowDB);
        return ERROR;
    }

    sprintf(binName, "bin%d", binIndex);

    // retrieve current value:
    field = radsqlitedirectFieldGet(row, binName);
    if (field == NULL)
    {
        radMsgLog (PRI_MEDIUM, "hilowGetWindRecord: radsqliteFieldGet failed!");
        radsqlitedirectReleaseResults(hilowDB);
        return ERROR;
    }
    else
    {
        retVal = radsqliteFieldGetBigIntValue(field);
    }

    radsqlitedirectReleaseResults(hilowDB);
    return retVal;
}

static int hilowInsertData(time_t timestamp, SENSOR_TYPES type, float value, float whenHigh)
{
    SQLITE_ROW_ID           row;
    SQLITE_FIELD_ID         field;
    time_t                  hilowTime;
    struct tm               bknTime;
    WV_SENSOR               store;
    char                    query[DB_SQLITE_QUERY_LENGTH_MAX];

    if (value <= ARCHIVE_VALUE_NULL)
    {
        return ERROR;
    }

    hilowTime = timestamp;
    localtime_r(&hilowTime, &bknTime);
    bknTime.tm_min   = 0;
    bknTime.tm_sec   = 0;
    bknTime.tm_isdst = -1;
    hilowTime = mktime(&bknTime);

    // First see if the record exists:
    if (hilowGetRecord(hilowTime, type, &store) == OK)
    {
        // Found the guy, just update him:
        store.samples ++;
        store.cumulative += value;
        if (store.low > value)
        {
            // New low:
            store.low = value;
            store.time_low = timestamp;
        }
        if (store.high < value)
        {
            // New high:
            store.high = value;
            store.time_high = timestamp;
            store.when_high = whenHigh;
        }

        // Update the database:
        sprintf(query, 
                "UPDATE %s SET low = '%.3f', timeLow = '%d', "
                "high = '%.3f', timeHigh = '%d', whenHigh = '%.3f', "
                "cumulative = '%.6f', samples = '%d' "
                "WHERE dateTime = '%d'",
                sensorTables[type],
                store.low, (int)store.time_low,
                store.high, (int)store.time_high, store.when_high,
                store.cumulative, store.samples,
                (int)hilowTime);

        // Execute the query:
        if (radsqliteQuery(hilowDB, query, FALSE) == ERROR)
        {
            radMsgLog (PRI_HIGH, "dbsqliteHiLow: query failed");
            return ERROR;
        }
    }
    else
    {
        // Must create a new record:
        row = radsqliteTableDescriptionGet(hilowDB, sensorTables[type]);
        if (row == NULL)
        {
            radMsgLog (PRI_HIGH, "dbsqliteHiLow: databaseTableDescriptionGet failed!");
            return ERROR;
        }
    
        field = radsqliteFieldGet(row, "dateTime");
        if (field == NULL)
        {
            radMsgLog (PRI_HIGH, "dbsqliteHiLow: radsqliteFieldGet failed!");
            radsqliteRowDescriptionDelete(row);
            return ERROR;
        }
        else
        {
            radsqliteFieldSetBigIntValue(field, (uint64_t)hilowTime);
        }

        field = radsqliteFieldGet(row, "low");
        if (field == NULL)
        {
            radMsgLog (PRI_HIGH, "dbsqliteHiLow: radsqliteFieldGet failed!");
            radsqliteRowDescriptionDelete(row);
            return ERROR;
        }
        else
        {
            radsqliteFieldSetDoubleValue(field, (double)value);
        }

        field = radsqliteFieldGet(row, "timeLow");
        if (field == NULL)
        {
            radMsgLog (PRI_HIGH, "dbsqliteHiLow: radsqliteFieldGet failed!");
            radsqliteRowDescriptionDelete(row);
            return ERROR;
        }
        else
        {
            radsqliteFieldSetBigIntValue(field, (uint64_t)timestamp);
        }

        field = radsqliteFieldGet(row, "high");
        if (field == NULL)
        {
            radMsgLog (PRI_HIGH, "dbsqliteHiLow: radsqliteFieldGet failed!");
            radsqliteRowDescriptionDelete(row);
            return ERROR;
        }
        else
        {
            radsqliteFieldSetDoubleValue(field, (double)value);
        }

        field = radsqliteFieldGet(row, "timeHigh");
        if (field == NULL)
        {
            radMsgLog (PRI_HIGH, "dbsqliteHiLow: radsqliteFieldGet failed!");
            radsqliteRowDescriptionDelete(row);
            return ERROR;
        }
        else
        {
            radsqliteFieldSetBigIntValue(field, (uint64_t)timestamp);
        }

        field = radsqliteFieldGet(row, "whenHigh");
        if (field == NULL)
        {
            radMsgLog (PRI_HIGH, "dbsqliteHiLow: radsqliteFieldGet failed!");
            radsqliteRowDescriptionDelete(row);
            return ERROR;
        }
        else
        {
            radsqliteFieldSetDoubleValue(field, (double)whenHigh);
        }

        field = radsqliteFieldGet(row, "cumulative");
        if (field == NULL)
        {
            radMsgLog (PRI_HIGH, "dbsqliteHiLow: radsqliteFieldGet failed!");
            radsqliteRowDescriptionDelete(row);
            return ERROR;
        }
        else
        {
            radsqliteFieldSetDoubleValue(field, (double)value);
        }

        field = radsqliteFieldGet(row, "samples");
        if (field == NULL)
        {
            radMsgLog (PRI_HIGH, "dbsqliteHiLow: radsqliteFieldGet failed!");
            radsqliteRowDescriptionDelete(row);
            return ERROR;
        }
        else
        {
            radsqliteFieldSetBigIntValue(field, (uint64_t)1);
        }

        // insert the row:
        if (radsqliteTableInsertRow(hilowDB, sensorTables[type], row) == ERROR)
        {
            radMsgLog (PRI_HIGH, "dbsqlite: radsqliteTableInsertRow (hilow) failed!");
            radsqliteRowDescriptionDelete(row);
            return ERROR;
        }
    
        radsqliteRowDescriptionDelete(row);
    }

    return OK;
}

static int hilowInsertWindDir (time_t timestamp, int value)
{
    SQLITE_ROW_ID           row, newrow;
    SQLITE_RESULT_SET_ID    results;
    SQLITE_FIELD_ID         field;
    time_t                  hilowTime;
    struct tm               bknTime;
    char                    query[DB_SQLITE_QUERY_LENGTH_MAX];
    int                     i, binIndex, binValue;
    char                    binName[16];

    if (hilowDB == NULL)
    {
        radMsgLog (PRI_HIGH, "hilowInsertWindDir: failed to open %s!", hilowGetDBFilename());
        return ERROR;
    }

    hilowTime = timestamp;
    localtime_r(&hilowTime, &bknTime);
    bknTime.tm_min   = 0;
    bknTime.tm_sec   = 0;
    bknTime.tm_isdst = -1;
    hilowTime = mktime(&bknTime);

    if (value < 0)
        binIndex = 0;
    else
        binIndex = value;

    binIndex += (WAVG_BIN_SIZE/2);
    binIndex /= WAVG_BIN_SIZE;
    binIndex %= WAVG_NUM_BINS;

    // First see if the record exists:
    binValue = hilowGetWindRecord(hilowTime, binIndex);
    if (binValue == ERROR)
    {
        // Create the row:
        newrow = radsqliteTableDescriptionGet(hilowDB, WVIEW_HILOW_WINDDIR_TABLE);
        if (newrow == NULL)
        {
            printf ("hilowInsertWindDir: databaseTableDescriptionGet failed!");
            return ERROR;
        }
    
        field = radsqliteFieldGet(newrow, "dateTime");
        if (field == NULL)
        {
            radMsgLog (PRI_MEDIUM, "hilowInsertWindDir: radsqliteFieldGet failed!");
            radsqliteRowDescriptionDelete(newrow);
            return ERROR;
        }
        else
        {
            radsqliteFieldSetBigIntValue(field, (uint64_t)hilowTime);
        }

        for (i = 0; i < WAVG_NUM_BINS; i ++)
        {
            sprintf(binName, "bin%d", i);
            field = radsqliteFieldGet(newrow, binName);
            if (field == NULL)
            {
                radMsgLog (PRI_MEDIUM, "hilowInsertWindDir: radsqliteFieldGet failed!");
                radsqliteRowDescriptionDelete(newrow);
                return ERROR;
            }
            else
            {
                if (i == binIndex)
                {
                    // This is our guy:
                    radsqliteFieldSetBigIntValue(field, (uint64_t)1);
                }
                else
                {
                    radsqliteFieldSetBigIntValue(field, (uint64_t)0);
                }
            }
        }

        // insert the row:
        if (radsqliteTableInsertRow(hilowDB, WVIEW_HILOW_WINDDIR_TABLE, newrow) == ERROR)
        {
            radMsgLog (PRI_HIGH, "hilowInsertWindDir: radsqliteTableInsertRow (hilow) failed!");
            radsqliteRowDescriptionDelete(newrow);
            return ERROR;
        }
    
        radsqliteRowDescriptionDelete(newrow);
    }
    else
    {
        sprintf(binName, "bin%d", binIndex);
        binValue ++;

        sprintf(query, "UPDATE %s SET %s = '%d' WHERE dateTime = '%d'",
                WVIEW_HILOW_WINDDIR_TABLE,
                binName, binValue, (int)hilowTime);

        
        // Execute the query:
        if (radsqliteQuery(hilowDB, query, FALSE) == ERROR)
        {
            return ERROR;
        }
    }

    return OK;
}

static int hilowUpdateTableWithArchive (SENSOR_TYPES type, ARCHIVE_PKT* pkt)
{
    switch(type)
    {
    case SENSOR_INTEMP:
        if (pkt->value[DATA_INDEX_inTemp] > -500 && pkt->value[DATA_INDEX_inTemp] < 500)
            hilowInsertData(pkt->dateTime - (60*pkt->interval), type, pkt->value[DATA_INDEX_inTemp], 0);
        break;
    case SENSOR_OUTTEMP:
        hilowInsertData(pkt->dateTime - (60*pkt->interval), type, pkt->value[DATA_INDEX_outTemp], 0);
        break;
    case SENSOR_INHUMID:
        if (pkt->value[DATA_INDEX_inHumidity] >= 0 && pkt->value[DATA_INDEX_inHumidity] <= 100)
            hilowInsertData(pkt->dateTime - (60*pkt->interval), type, pkt->value[DATA_INDEX_inHumidity], 0);
        break;
    case SENSOR_OUTHUMID:
        hilowInsertData(pkt->dateTime - (60*pkt->interval), type, pkt->value[DATA_INDEX_outHumidity], 0);
        break;
    case SENSOR_BP:
        hilowInsertData(pkt->dateTime - (60*pkt->interval), type, pkt->value[DATA_INDEX_barometer], 0);
        break;
    case SENSOR_WSPEED:
        hilowInsertData(pkt->dateTime - (60*pkt->interval), type, pkt->value[DATA_INDEX_windSpeed], 0);
        break;
    case SENSOR_WGUST:
        hilowInsertData(pkt->dateTime - (60*pkt->interval), type, pkt->value[DATA_INDEX_windGust], 
                        pkt->value[DATA_INDEX_windGustDir]);
        break;
    case SENSOR_DEWPOINT:
        hilowInsertData(pkt->dateTime - (60*pkt->interval), type, pkt->value[DATA_INDEX_dewpoint], 0);
        break;
    case SENSOR_RAIN:
        hilowInsertData(pkt->dateTime - (60*pkt->interval), type, pkt->value[DATA_INDEX_rain], 0);
        break;
    case SENSOR_RAINRATE:
        hilowInsertData(pkt->dateTime - (60*pkt->interval), type, pkt->value[DATA_INDEX_rainRate], 0);
        break;
    case SENSOR_WCHILL:
        hilowInsertData(pkt->dateTime - (60*pkt->interval), type, pkt->value[DATA_INDEX_windchill], 0);
        break;
    case SENSOR_HINDEX:
        hilowInsertData(pkt->dateTime - (60*pkt->interval), type, pkt->value[DATA_INDEX_heatindex], 0);
        break;
    case SENSOR_ET:
        if (pkt->value[DATA_INDEX_ET] >= 0 && pkt->value[DATA_INDEX_ET] < 100)
              hilowInsertData(pkt->dateTime - (60*pkt->interval), type, pkt->value[DATA_INDEX_ET], 0);
        break;
    case SENSOR_UV:
        if (pkt->value[DATA_INDEX_UV] >= 0 && pkt->value[DATA_INDEX_UV] < 100)
            hilowInsertData(pkt->dateTime - (60*pkt->interval), type, pkt->value[DATA_INDEX_UV], 0);
        break;
    case SENSOR_SOLRAD:
        if (pkt->value[DATA_INDEX_radiation] >= 0 && pkt->value[DATA_INDEX_radiation] < 10000)
            hilowInsertData(pkt->dateTime - (60*pkt->interval), type, pkt->value[DATA_INDEX_radiation], 0);
        break;
    case SENSOR_HAIL:
        if (pkt->value[DATA_INDEX_hail] >= 0 && pkt->value[DATA_INDEX_hail] < 100)
            hilowInsertData(pkt->dateTime - (60*pkt->interval), type, pkt->value[DATA_INDEX_hail], 0);
        break;
    case SENSOR_HAILRATE:
        if (pkt->value[DATA_INDEX_hailrate] >= 0 && pkt->value[DATA_INDEX_hailrate] < 100)
            hilowInsertData(pkt->dateTime - (60*pkt->interval), type, pkt->value[DATA_INDEX_hailrate], 0);
        break;
    default:
        break;
    }
    return OK;
}

static int hilowUpdateTableWithSample (SENSOR_TYPES type, time_t timestamp, LOOP_PKT* pkt)
{
    switch(type)
    {
    case SENSOR_INTEMP:
        if (pkt->inTemp > -500 && pkt->inTemp < 500)
            hilowInsertData(timestamp, type, pkt->inTemp, 0);
        break;
    case SENSOR_OUTTEMP:
        hilowInsertData(timestamp, type, pkt->outTemp, 0);
        break;
    case SENSOR_INHUMID:
        if (pkt->inHumidity <= 100)
            hilowInsertData(timestamp, type, pkt->inHumidity, 0);
        break;
    case SENSOR_OUTHUMID:
        hilowInsertData(timestamp, type, pkt->outHumidity, 0);
        break;
    case SENSOR_BP:
        hilowInsertData(timestamp, type, pkt->barometer, 0);
        break;
    case SENSOR_WSPEED:
        hilowInsertData(timestamp, type, pkt->windSpeedF, 0);
        break;
    case SENSOR_WGUST:
        hilowInsertData(timestamp, type, pkt->windGustF, pkt->windGustDir);
        break;
    case SENSOR_DEWPOINT:
        hilowInsertData(timestamp, type, pkt->dewpoint, 0);
        break;
    case SENSOR_RAIN:
        hilowInsertData(timestamp, type, pkt->sampleRain, 0);
        break;
    case SENSOR_RAINRATE:
        hilowInsertData(timestamp, type, pkt->rainRate, 0);
        break;
    case SENSOR_WCHILL:
        hilowInsertData(timestamp, type, pkt->windchill, 0);
        break;
    case SENSOR_HINDEX:
        hilowInsertData(timestamp, type, pkt->heatindex, 0);
        break;
    case SENSOR_ET:
        if (pkt->sampleET >= 0 && pkt->sampleET < 100)
            hilowInsertData(timestamp, type, pkt->sampleET, 0);
        break;
    case SENSOR_UV:
        if (pkt->UV < 100)
            hilowInsertData(timestamp, type, pkt->UV, 0);
        break;
    case SENSOR_SOLRAD:
        if (pkt->radiation < 10000)
            hilowInsertData(timestamp, type, pkt->radiation, 0);
        break;
    case SENSOR_HAIL:
        if (pkt->wxt510Hail >= 0 && pkt->wxt510Hail < 100)
            hilowInsertData(timestamp, type, pkt->wxt510Hail, 0);
        break;
    case SENSOR_HAILRATE:
        if (pkt->wxt510Hailrate >= 0 && pkt->wxt510Hailrate < 100)
            hilowInsertData(timestamp, type, pkt->wxt510Hailrate, 0);
        break;
    default:
        break;
    }
    return OK;
}

static int hilowGetDataTimeFrame
(
    int32_t                 first,
    int32_t                 last,
    SENSOR_STORE*           sensors,
    SENSOR_TIMEFRAMES       timeFrame
)
{
    SENSOR_TYPES            index;
    int                     retVal = 0;
    char                    query[DB_SQLITE_QUERY_LENGTH_MAX];
    SQLITE_DIRECT_ROW       rowDescr;
    SQLITE_FIELD_ID         field;
    WV_SENSOR*              tempSensor;
    WV_SENSOR               store;
    int                     i, bins[WAVG_NUM_BINS];
    char                    binName[16];

    if (hilowDB == NULL)
    {
        radMsgLog (PRI_HIGH, "dbsqliteHiLowGet: failed to open %s!", hilowGetDBFilename());
        return ERROR;
    }

    // Loop through the sensor types:
    for (index = SENSOR_INTEMP; index < SENSOR_MAX; index ++)
    {
        // grab the rows:
        sprintf (query, "SELECT * FROM %s WHERE dateTime >= '%d' AND dateTime < '%d' ORDER BY dateTime ASC", 
                 sensorTables[index], first, last);
    
        // Execute the query:
        if (radsqlitedirectQuery(hilowDB, query, TRUE) == ERROR)
        {
            return ERROR;
        }

        for (rowDescr = radsqlitedirectGetRow(hilowDB);
             rowDescr != NULL;
             rowDescr = radsqlitedirectGetRow(hilowDB))
        {
            // copy it to the internal store:
            tempSensor = &sensors->sensor[timeFrame][index];

            if (hilowExtractValues(rowDescr, &store) == ERROR)
            {
                radsqlitedirectReleaseResults(hilowDB);
                radMsgLog (PRI_HIGH, "dbsqliteHiLowGetDay: failed to extract data");
                return ERROR;
            }

            tempSensor->cumulative      += store.cumulative;
            tempSensor->samples         += store.samples;
            if (tempSensor->low > store.low)
            {
                tempSensor->low         = store.low;
                tempSensor->time_low    = store.time_low;
            }
            if (tempSensor->high < store.high)
            {
                tempSensor->high        = store.high;
                tempSensor->time_high   = store.time_high;
                tempSensor->when_high   = store.when_high;
            }
        }
    
        radsqlitedirectReleaseResults(hilowDB);
    }

    // Do the wind dir here:
    sprintf (query, "SELECT * FROM %s WHERE dateTime >= '%d' AND dateTime < '%d' ORDER BY dateTime ASC", 
             WVIEW_HILOW_WINDDIR_TABLE, (int)first, (int)last);

    // Execute the query:
    if (radsqlitedirectQuery(hilowDB, query, TRUE) == ERROR)
    {
        return ERROR;
    }

    retVal = 0;
    for (rowDescr = radsqlitedirectGetRow(hilowDB);
         rowDescr != NULL;
         rowDescr = radsqlitedirectGetRow(hilowDB))
    {
        retVal ++;
        for (i = 0; i < WAVG_NUM_BINS; i ++)
        {
            bins[i] = 0;
            sprintf(binName, "bin%d", i);
            field = radsqlitedirectFieldGet(rowDescr, binName);
            if (field == NULL)
            {
                radsqlitedirectReleaseResults(hilowDB);
                return ERROR;
            }
            else
            {
                bins[i] = (int)radsqliteFieldGetBigIntValue(field);
            }
        }

        windAverageAddBins(&sensors->wind[timeFrame], bins);
    }

    radsqlitedirectReleaseResults(hilowDB);
    return retVal;
}

static time_t hilowGetLastUpdateTime (void)
{
    char                    query[DB_SQLITE_QUERY_LENGTH_MAX];
    SQLITE_DIRECT_ROW       rowDescr;
    SQLITE_FIELD_ID         field;
    time_t                  retVal;
    char                    tempstr[256];

    if (hilowDB == NULL)
    {
        radMsgLog (PRI_HIGH, "hilowGetLastUpdateTime: failed to open %s!", hilowGetDBFilename());
        return (time_t)(-1);
    }

    // grab the row:
    sprintf (query, "SELECT * FROM %s WHERE name = 'lastUpdate'", 
             WVIEW_HILOW_META_TABLE);
    
    // Execute the query:
    if (radsqlitedirectQuery(hilowDB, query, TRUE) == ERROR)
    {
        return (time_t)(-1);
    }

    rowDescr = radsqlitedirectGetRow(hilowDB);
    if (rowDescr == NULL)
    {
        radsqlitedirectReleaseResults(hilowDB);
        return (time_t)(-1);
    }

    field = radsqlitedirectFieldGet(rowDescr, "value");
    if (field == NULL)
    {
        radsqlitedirectReleaseResults(hilowDB);
        return (time_t)(-1);
    }

    strncpy(tempstr, radsqliteFieldGetCharValue(field), radsqliteFieldGetCharLength(field));
    retVal = (time_t)atoi(tempstr);
    radsqlitedirectReleaseResults(hilowDB);
    return retVal;
}

static int hilowSetLastUpdateTime (time_t newtime)
{
    char                    query[DB_SQLITE_QUERY_LENGTH_MAX];

    if (hilowDB == NULL)
    {
        radMsgLog (PRI_HIGH, "hilowSetLastUpdateTime: failed to open %s!", hilowGetDBFilename());
        return ERROR;
    }

    // update the row:
    sprintf (query, "UPDATE %s SET value = '%d' WHERE name = 'lastUpdate'", 
             WVIEW_HILOW_META_TABLE, (int)newtime);
    
    // Execute the query:
    if (radsqliteQuery(hilowDB, query, FALSE) == ERROR)
    {
        return ERROR;
    }

    return OK;
}

// Make these static so we can use the dbsqliteArchiveExecutePerRecord call to
// efficiently populate the HILOW tables when created:
static int      IfTablesCreated = FALSE, IfWindTableExists = FALSE, IfTableExists[SENSOR_MAX];
static time_t   LastHiLowUpdate, LastArchiveTime;

// Callback method for dbsqliteArchiveExecutePerRecord:
static void hilowInitPerRecord (ARCHIVE_PKT* rec, void* data)
{
    struct tm           bknTime;
    SENSOR_TYPES        index;
    time_t              Time = (time_t)rec->dateTime;

    localtime_r(&Time, &bknTime);
    if (bknTime.tm_hour == 0 && 
        bknTime.tm_min == rec->interval)
    {
        // Beginning of day, log it:
        radMsgLog(PRI_STATUS, "HILOW: adding records for %4.4d%2.2d%2.2d",
                  bknTime.tm_year + 1900,
                  bknTime.tm_mon + 1,
                  bknTime.tm_mday);
    }

    // Did we just create the wind dir table -OR- is this record newer than 
    // our last update?
    if (IfWindTableExists == FALSE || LastHiLowUpdate < rec->dateTime)
    {
        // Insert wind direction data:
        hilowInsertWindDir(rec->dateTime - (60*rec->interval), 
                           (int)rec->value[DATA_INDEX_windDir]);
    }

    // Now loop through the sensor types, only updating ones we just created:
    for (index = SENSOR_INTEMP; index < SENSOR_MAX; index ++)
    {
        // If this table was not just created -AND- this is not a new record:
        if (IfTableExists[index] && LastHiLowUpdate > rec->dateTime)
        {
            // Nothing to do here:
            continue;
        }

        // Update table[index] with this record:
        hilowUpdateTableWithArchive(index, rec);
    }

    LastArchiveTime = rec->dateTime;
    hilowSetLastUpdateTime(LastArchiveTime);
}

////////////////////////////////////////////////////////////////////////////////

// Initialize the HILOW database (returns OK or ERROR):
int dbsqliteHiLowInit(int update)
{
    SQLITE_ROW_ID       rowDesc, newrow;
    SQLITE_FIELD_ID     field;
    SENSOR_TYPES        index;
    int                 retVal;
    int                 i, done = FALSE;
    int                 weeks=0;
    char                tableName[64];
    ARCHIVE_PKT         archiveRec;
    time_t              archiveTime, startTime, stopTime, runStartTime = time(NULL), diffTime;
    char                binName[16];
    struct tm           bknTime;

    hilowDB = radsqliteOpen(hilowGetDBFilename());
    if (hilowDB == NULL)
    {
        radMsgLog (PRI_HIGH, "dbsqliteHiLowInit: failed to open %s!", hilowGetDBFilename());
        return ERROR;
    }

    if (! update)
    {
        //dbsqliteHiLowPragmaSet("journal_mode", "DELETE");
        dbsqliteHiLowPragmaSet("synchronous", "NORMAL");
    
        radMsgLog(PRI_STATUS, "HILOW: OK");
        return OK;
    }

    // Make writes faster (and less safe) by avoiding fsyncs:
    dbsqliteHiLowPragmaSet("synchronous", "OFF");

    // Make things quicker but less safe by turning off journalling:
    //dbsqliteHiLowPragmaSet("journal_mode", "OFF");


    // Do the meta table first:
    if (! radsqliteTableIfExists(hilowDB, WVIEW_HILOW_META_TABLE))
    {
        // Create it:
        rowDesc = radsqliteRowDescriptionCreate();
        if (rowDesc == NULL)
        {
            radsqliteClose(hilowDB);
            radMsgLog(PRI_HIGH, "dbsqliteHiLowInit: radsqliteRowDescriptionCreate failed!");
            return ERROR;
        }
    
        // Populate the table:
        retVal = radsqliteRowDescriptionAddField(rowDesc, 
                                                 "name", 
                                                 SQLITE_FIELD_STRING | SQLITE_FIELD_PRI_KEY, 
                                                 64);
        if (retVal == ERROR)
        {
            radsqliteClose(hilowDB);
            radMsgLog(PRI_HIGH, "dbsqliteHiLowInit: databaseRowDescriptionAddField failed!");
            radsqliteRowDescriptionDelete(rowDesc);
            return ERROR;
        }
        retVal = radsqliteRowDescriptionAddField(rowDesc, 
                                                 "value", 
                                                 SQLITE_FIELD_STRING, 
                                                 64);
        if (retVal == ERROR)
        {
            radsqliteClose(hilowDB);
            radMsgLog(PRI_HIGH, "dbsqliteHiLowInit: databaseRowDescriptionAddField failed!");
            radsqliteRowDescriptionDelete(rowDesc);
            return ERROR;
        }
        
        // Now create the table:
        if (radsqliteTableCreate(hilowDB, WVIEW_HILOW_META_TABLE, rowDesc) == ERROR)
        {
            radsqliteClose(hilowDB);
            radMsgLog(PRI_HIGH, "dbsqliteHiLowInit: radsqliteTableCreate failed!");
            radsqliteRowDescriptionDelete(rowDesc);
            return ERROR;
        }
        radsqliteRowDescriptionDelete(rowDesc);

        // Create the row:
        newrow = radsqliteTableDescriptionGet(hilowDB, WVIEW_HILOW_META_TABLE);
        if (newrow == NULL)
        {
            printf ("hilowInsertWindDir: databaseTableDescriptionGet failed!");
            return ERROR;
        }
    
        field = radsqliteFieldGet(newrow, "name");
        if (field == NULL)
        {
            radMsgLog (PRI_MEDIUM, "dbsqliteHiLowInit: radsqliteFieldGet failed!");
            radsqliteRowDescriptionDelete(newrow);
            return ERROR;
        }
        else
        {
            radsqliteFieldSetCharValue(field, "lastUpdate", strlen("lastUpdate"));
        }
        field = radsqliteFieldGet(newrow, "value");
        if (field == NULL)
        {
            radMsgLog (PRI_MEDIUM, "dbsqliteHiLowInit: radsqliteFieldGet failed!");
            radsqliteRowDescriptionDelete(newrow);
            return ERROR;
        }
        else
        {
            radsqliteFieldSetCharValue(field, "0", 1);
        }

        // insert the row:
        if (radsqliteTableInsertRow(hilowDB, WVIEW_HILOW_META_TABLE, newrow) == ERROR)
        {
            radMsgLog (PRI_HIGH, "dbsqliteHiLowInit: radsqliteTableInsertRow (hilow) failed!");
            radsqliteRowDescriptionDelete(newrow);
            return ERROR;
        }
        radsqliteRowDescriptionDelete(newrow);
        radMsgLog(PRI_STATUS, "HILOW %s table created", WVIEW_HILOW_META_TABLE);
    }

    // Do the wind direction bins:
    if (radsqliteTableIfExists(hilowDB, WVIEW_HILOW_WINDDIR_TABLE))
    {
        IfWindTableExists = TRUE;
    }
    else
    {
        IfTablesCreated = TRUE;

        // Create it:
        rowDesc = radsqliteRowDescriptionCreate();
        if (rowDesc == NULL)
        {
            radsqliteClose(hilowDB);
            radMsgLog(PRI_HIGH, "dbsqliteHiLowInit: radsqliteRowDescriptionCreate failed!");
            return ERROR;
        }
    
        // Populate the table:
        retVal = radsqliteRowDescriptionAddField(rowDesc, 
                                                 "dateTime", 
                                                 SQLITE_FIELD_BIGINT | SQLITE_FIELD_PRI_KEY, 
                                                 0);
        if (retVal == ERROR)
        {
            radsqliteClose(hilowDB);
            radMsgLog(PRI_HIGH, "dbsqliteHiLowInit: databaseRowDescriptionAddField failed!");
            radsqliteRowDescriptionDelete(rowDesc);
            return ERROR;
        }
        
        for (i = 0; i < WAVG_NUM_BINS; i ++)
        {
            sprintf(binName, "bin%d", i);
            retVal = radsqliteRowDescriptionAddField(rowDesc, binName, SQLITE_FIELD_BIGINT, 0);
            if (retVal == ERROR)
            {
                radsqliteClose(hilowDB);
                radMsgLog(PRI_HIGH, "dbsqliteHiLowInit: databaseRowDescriptionAddField failed!");
                radsqliteRowDescriptionDelete(rowDesc);
                return ERROR;
            }
        }

        // Now create the table:
        if (radsqliteTableCreate(hilowDB, WVIEW_HILOW_WINDDIR_TABLE, rowDesc) == ERROR)
        {
            radsqliteClose(hilowDB);
            radMsgLog(PRI_HIGH, "dbsqliteHiLowInit: radsqliteTableCreate failed!");
            radsqliteRowDescriptionDelete(rowDesc);
            return ERROR;
        }
        radsqliteRowDescriptionDelete(rowDesc);
        radMsgLog(PRI_STATUS, "HILOW %s table created", WVIEW_HILOW_WINDDIR_TABLE);
    }


    // Loop through all sensor types, creating tables as needed:
    for (index = SENSOR_INTEMP; index < SENSOR_MAX; index ++)
    {
        IfTableExists[index] = FALSE;

        // Does the sensor HILOW table exist?
        if (radsqliteTableIfExists(hilowDB, sensorTables[index]))
        {
            IfTableExists[index] = TRUE;
            continue;
        }

        IfTablesCreated = TRUE;

        // We need to create the table:
        // Define the row first:
        rowDesc = radsqliteRowDescriptionCreate();
        if (rowDesc == NULL)
        {
            radsqliteClose(hilowDB);
            radMsgLog(PRI_HIGH, "dbsqliteHiLowInit: radsqliteRowDescriptionCreate failed!");
            return ERROR;
        }
    
        // Populate the table:
        retVal = radsqliteRowDescriptionAddField(rowDesc, 
                                                 "dateTime", 
                                                 SQLITE_FIELD_BIGINT | SQLITE_FIELD_PRI_KEY, 
                                                 0);
        if (retVal == ERROR)
        {
            radsqliteClose(hilowDB);
            radMsgLog(PRI_HIGH, "dbsqliteHiLowInit: databaseRowDescriptionAddField failed!");
            radsqliteRowDescriptionDelete(rowDesc);
            return ERROR;
        }

        retVal = radsqliteRowDescriptionAddField(rowDesc, "low", SQLITE_FIELD_DOUBLE, 0);
        if (retVal == ERROR)
        {
            radsqliteClose(hilowDB);
            radMsgLog(PRI_HIGH, "dbsqliteHiLowInit: databaseRowDescriptionAddField failed!");
            radsqliteRowDescriptionDelete(rowDesc);
            return ERROR;
        }
        retVal = radsqliteRowDescriptionAddField(rowDesc, "timeLow", SQLITE_FIELD_BIGINT, 0);
        if (retVal == ERROR)
        {
            radsqliteClose(hilowDB);
            radMsgLog(PRI_HIGH, "dbsqliteHiLowInit: databaseRowDescriptionAddField failed!");
            radsqliteRowDescriptionDelete(rowDesc);
            return ERROR;
        }
        retVal = radsqliteRowDescriptionAddField(rowDesc, "high", SQLITE_FIELD_DOUBLE, 0);
        if (retVal == ERROR)
        {
            radsqliteClose(hilowDB);
            radMsgLog(PRI_HIGH, "dbsqliteHiLowInit: databaseRowDescriptionAddField failed!");
            radsqliteRowDescriptionDelete(rowDesc);
            return ERROR;
        }
        retVal = radsqliteRowDescriptionAddField(rowDesc, "timeHigh", SQLITE_FIELD_BIGINT, 0);
        if (retVal == ERROR)
        {
            radsqliteClose(hilowDB);
            radMsgLog(PRI_HIGH, "dbsqliteHiLowInit: databaseRowDescriptionAddField failed!");
            radsqliteRowDescriptionDelete(rowDesc);
            return ERROR;
        }
        retVal = radsqliteRowDescriptionAddField(rowDesc, "whenHigh", SQLITE_FIELD_DOUBLE, 0);
        if (retVal == ERROR)
        {
            radsqliteClose(hilowDB);
            radMsgLog(PRI_HIGH, "dbsqliteHiLowInit: databaseRowDescriptionAddField failed!");
            radsqliteRowDescriptionDelete(rowDesc);
            return ERROR;
        }
        retVal = radsqliteRowDescriptionAddField(rowDesc, "cumulative", SQLITE_FIELD_DOUBLE, 0);
        if (retVal == ERROR)
        {
            radsqliteClose(hilowDB);
            radMsgLog(PRI_HIGH, "dbsqliteHiLowInit: databaseRowDescriptionAddField failed!");
            radsqliteRowDescriptionDelete(rowDesc);
            return ERROR;
        }
        retVal = radsqliteRowDescriptionAddField(rowDesc, "samples", SQLITE_FIELD_BIGINT, 0);
        if (retVal == ERROR)
        {
            radsqliteClose(hilowDB);
            radMsgLog(PRI_HIGH, "dbsqliteHiLowInit: databaseRowDescriptionAddField failed!");
            radsqliteRowDescriptionDelete(rowDesc);
            return ERROR;
        }


        // Now create the table:
        if (radsqliteTableCreate(hilowDB, sensorTables[index], rowDesc) == ERROR)
        {
            radsqliteClose(hilowDB);
            radMsgLog(PRI_HIGH, "dbsqliteHiLowInit: radsqliteTableCreate failed!");
            radsqliteRowDescriptionDelete(rowDesc);
            return ERROR;
        }

        // We're done with this table:
        radsqliteRowDescriptionDelete(rowDesc);

        radMsgLog(PRI_STATUS, "HILOW: %s table created", sensorTables[index]);
    }

    // OK, if we had to create one or more sensor tables, assume they should 
    // be completely populated:
    if (IfTablesCreated)
    {
        LastHiLowUpdate = hilowGetLastUpdateTime();

        radMsgLog(PRI_STATUS, "HILOW: back filling tables with ALL archive data");
        radMsgLog(PRI_STATUS, "HILOW: this is a one time process when tables are created");
        radMsgLog(PRI_STATUS, "HILOW: (this may take a while ...)");

        // Loop through all archive records, one week at a time:
        startTime = dbsqliteArchiveGetNextRecord(0, &archiveRec);
        stopTime = startTime + WV_SECONDS_IN_WEEK;
        while (! done)
        {
            // execute per row:
            retVal = dbsqliteArchiveExecutePerRecord(hilowInitPerRecord, 
                                                     NULL, 
                                                     startTime, 
                                                     stopTime, 
                                                     NULL);
            if (retVal == ERROR)
            {
                done = TRUE;
            }
            else if (retVal == 0)
            {
                // Make sure we don't have a gap:
                startTime = dbsqliteArchiveGetNextRecord(stopTime, &archiveRec);
                if ((int)startTime == ERROR)
                {
                    done = TRUE;
                }
                else
                {
                    stopTime = startTime + WV_SECONDS_IN_WEEK;
                }
            }
            else
            {
                startTime = stopTime;
                stopTime = startTime + WV_SECONDS_IN_WEEK;

                // Output running stats if we had data for the week:
                diffTime = time(NULL) - runStartTime;
                weeks++;
                radMsgLog(PRI_STATUS, "HILOW: Stats:");
                radMsgLog(PRI_STATUS, "HILOW:     Time               : %2.2d:%2.2d", 
                          (int)diffTime/60, (int)diffTime%60);
                radMsgLog(PRI_STATUS, "HILOW:     Weeks converted    : %d", weeks);
                radMsgLog(PRI_STATUS, "HILOW:     secs/week          : %.2f", 
                          (float)diffTime/(float)weeks);
            }
        }

        hilowSetLastUpdateTime(LastArchiveTime);
    }
    else
    {
        // See if we need to grab any recent archive records since we last ran:
        LastHiLowUpdate = hilowGetLastUpdateTime();
        LastArchiveTime = dbsqliteArchiveGetNewestTime(&archiveRec);

        if (LastHiLowUpdate < LastArchiveTime)
        {
            // We need to grab all records after LastHiLowUpdate:
            radMsgLog(PRI_STATUS, "HILOW: adding gap records since last update");

            // Loop through all archive records:
            for (archiveTime = dbsqliteArchiveGetNextRecord(LastHiLowUpdate, &archiveRec);
                 (int)archiveTime != ERROR;
                 archiveTime = dbsqliteArchiveGetNextRecord(archiveTime, &archiveRec))
            {
                LastArchiveTime = archiveTime;

                time_t Time = (time_t)archiveRec.dateTime;
                localtime_r(&Time, &bknTime);
                if (bknTime.tm_hour == 0 && 
                    bknTime.tm_min == archiveRec.interval)
                {
                    // Beginning of day, log it:
                    radMsgLog(PRI_STATUS, "HILOW: adding records for %4.4d%2.2d%2.2d",
                              bknTime.tm_year + 1900,
                              bknTime.tm_mon + 1,
                              bknTime.tm_mday);
                }
    
                // Insert wind direction data:
                hilowInsertWindDir(archiveRec.dateTime - (60*archiveRec.interval), 
                                   (int)archiveRec.value[DATA_INDEX_windDir]);
    
                // Now loop through the sensor types:
                for (index = SENSOR_INTEMP; index < SENSOR_MAX; index ++)
                {
                    // Update this table with this record:
                    hilowUpdateTableWithArchive(index, &archiveRec);
                }
            }
    
            hilowSetLastUpdateTime(LastArchiveTime);
        }
        radMsgLog(PRI_STATUS, "HILOW: database OK");
    }


    // Reinstate journals:
    //dbsqliteHiLowPragmaSet("journal_mode", "DELETE");

    // Restore normal syncing behavior:
    dbsqliteHiLowPragmaSet("synchronous", "NORMAL");

    radMsgLog(PRI_STATUS, "HILOW: beginning normal LOOP operation");
    return OK;
}

void dbsqliteHiLowExit(void)
{
    if (hilowDB)
        radsqliteClose(hilowDB);
}

// set a PRAGMA to modify the operation of the SQLite library:
// Returns: OK or ERROR
int dbsqliteHiLowPragmaSet(char* pragma, char* setting)
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
    if (radsqliteQuery(hilowDB, query, FALSE) == ERROR)
    {
        return ERROR;
    }

    return OK;
}

//  ... Update database with a timestamped LOOP sample:
//  ... No duplicate checking is done;
//  ... Returns OK or ERROR
int dbsqliteHiLowStoreSample (time_t timestamp, LOOP_PKT* sample)
{
    SENSOR_TYPES        index;
    char                fileName[128];

    // Update the update time:
    hilowSetLastUpdateTime(timestamp);

    // Loop through the sensor types:
    for (index = SENSOR_INTEMP; index < SENSOR_MAX; index ++)
    {
        // Update this table with this record:
        hilowUpdateTableWithSample(index, timestamp, sample);
    }

    // Update wind direction:
    hilowInsertWindDir(timestamp, sample->windDir);

    sprintf (fileName, "%s/export/%s", wvutilsGetConfigPath(), WVIEW_HILOW_MARKER_FILE);
    wvutilsWriteMarkerFile(fileName, timestamp);

    return OK;
}

//  ... Update database with an archive record in lieu of LOOP samples:
//  ... No duplicate checking is done;
//  ... Returns OK or ERROR
int dbsqliteHiLowStoreArchive (ARCHIVE_PKT* record)
{
    SENSOR_TYPES        index;
    char                fileName[128];

    // Update the update time:
    hilowSetLastUpdateTime(record->dateTime);

    // Loop through the sensor types:
    for (index = SENSOR_INTEMP; index < SENSOR_MAX; index ++)
    {
        // Update this table with this record:
        hilowUpdateTableWithArchive(index, record);
    }

    // Update wind direction:
    hilowInsertWindDir(record->dateTime, (int)record->value[DATA_INDEX_windDir]);

    sprintf (fileName, "%s/export/%s", wvutilsGetConfigPath(), WVIEW_HILOW_MARKER_FILE);
    wvutilsWriteMarkerFile(fileName, record->dateTime);

    return OK;
}

//  ... Update database with an archive record, ignoring cumulative values:
//  ... No duplicate checking is done;
//  ... Returns OK or ERROR
int dbsqliteHiLowUpdateArchive (ARCHIVE_PKT* record)
{
    SENSOR_TYPES        index;

    // Update the update time:
    hilowSetLastUpdateTime(record->dateTime);

    // Loop through the sensor types:
    for (index = SENSOR_INTEMP; index < SENSOR_MAX; index ++)
    {
        // If it is a cumulative sensor, don't store it:
        if (index == SENSOR_RAIN || index == SENSOR_ET || index == SENSOR_HAIL)
        {
            continue;
        }

        // Update this table with this record:
        hilowUpdateTableWithArchive(index, record);
    }

    // Update wind direction:
    hilowInsertWindDir(record->dateTime, (int)record->value[DATA_INDEX_windDir]);

    return OK;
}

//  ... Update sensors for the given hour and time frame:
//  ... Returns number of records processed or ERROR
int dbsqliteHiLowGetHour
(
    time_t                  hour,
    SENSOR_STORE*           sensors,
    SENSOR_TIMEFRAMES       timeFrame
)
{
    time_t                  first, last;
    struct tm               bknTime;

    localtime_r(&hour, &bknTime);
    first = hour;
    bknTime.tm_hour ++;
    bknTime.tm_min   = 0;
    bknTime.tm_sec   = 0;
    bknTime.tm_isdst = -1;
    last = mktime(&bknTime);

    return (hilowGetDataTimeFrame(first, last, sensors, timeFrame));
}

int dbsqliteHiLowGetDay
(
    time_t                  day,
    SENSOR_STORE*           sensors,
    SENSOR_TIMEFRAMES       timeFrame
)
{
    time_t                  first, last;
    struct tm               bknTime;

    localtime_r(&day, &bknTime);
    first = day;
    bknTime.tm_mday ++;
    bknTime.tm_hour  = 0;
    bknTime.tm_min   = 0;
    bknTime.tm_sec   = 0;
    bknTime.tm_isdst = -1;
    last = mktime(&bknTime);

    return (hilowGetDataTimeFrame(first, last, sensors, timeFrame));
}

int dbsqliteHiLowGetMonth
(
    time_t                  month,
    SENSOR_STORE*           sensors,
    SENSOR_TIMEFRAMES       timeFrame,
    int                     yearRainFlag
)
{
    int                     retVal = 0;
    char                    query[DB_SQLITE_QUERY_LENGTH_MAX];
    SQLITE_DIRECT_ROW       rowDescr;
    SQLITE_FIELD_ID         field;
    time_t                  first, last;
    struct tm               bknTime;
    WV_SENSOR*              tempSensor;
    WV_SENSOR               store;
    int                     i, bins[WAVG_NUM_BINS];
    char                    binName[16];
    SENSOR_TYPES            index;

    localtime_r(&month, &bknTime);
    bknTime.tm_mday  = 1;
    bknTime.tm_hour  = 0;
    bknTime.tm_min   = 0;
    bknTime.tm_sec   = 0;
    bknTime.tm_mon   += 1;
    bknTime.tm_isdst = -1;
    first = month;
    last = mktime(&bknTime);

    if (hilowDB == NULL)
    {
        radMsgLog (PRI_HIGH, "dbsqliteHiLowGetMonth: failed to open %s!", hilowGetDBFilename());
        return ERROR;
    }

    // Loop through the sensor types:
    for (index = SENSOR_INTEMP; index < SENSOR_MAX; index ++)
    {
        // grab the rows:
        sprintf (query, "SELECT * FROM %s WHERE dateTime >= '%d' AND dateTime < '%d' ORDER BY dateTime ASC", 
                 sensorTables[index], (int)first, (int)last);
    
        // Execute the query:
        if (radsqlitedirectQuery(hilowDB, query, TRUE) == ERROR)
        {
            return ERROR;
        }

        retVal = 0;
        for (rowDescr = radsqlitedirectGetRow(hilowDB);
             rowDescr != NULL;
             rowDescr = radsqlitedirectGetRow(hilowDB))
        {
            if (yearRainFlag)
            {
                tempSensor = &sensors->sensor[STF_YEAR][index];
                if (index == SENSOR_RAIN)
                {
                    retVal ++;
                    if (hilowExtractValues(rowDescr, &store) == ERROR)
                    {
                        radsqlitedirectReleaseResults(hilowDB);
                        radMsgLog (PRI_HIGH, "dbsqliteHiLowGetMonth: failed to extract data");
                        return ERROR;
                    }
                    
                    tempSensor->cumulative      += store.cumulative;
                    tempSensor->samples         += store.samples;
                }
                else if (index == SENSOR_RAINRATE)
                {
                    if (hilowExtractValues(rowDescr, &store) == ERROR)
                    {
                        radsqlitedirectReleaseResults(hilowDB);
                        radMsgLog (PRI_HIGH, "dbsqliteHiLowGetMonth: failed to extract data");
                        return ERROR;
                    }
                    
                    tempSensor->cumulative      += store.cumulative;
                    tempSensor->samples         += store.samples;
                    if (tempSensor->high < store.high)
                    {
                        tempSensor->high        = store.high;
                        tempSensor->time_high   = store.time_high;
                        tempSensor->when_high   = store.when_high;
                    }
                }
                else if (index == SENSOR_ET)
                {
                    if (hilowExtractValues(rowDescr, &store) == ERROR)
                    {
                        radsqlitedirectReleaseResults(hilowDB);
                        radMsgLog (PRI_HIGH, "dbsqliteHiLowGetMonth: failed to extract data");
                        return ERROR;
                    }
                    
                    tempSensor->cumulative      += store.cumulative;
                    tempSensor->samples         += store.samples;
                }
                continue;
            }

            // copy it to the internal store:
            tempSensor = &sensors->sensor[timeFrame][index];

            if (hilowExtractValues(rowDescr, &store) == ERROR)
            {
                radsqlitedirectReleaseResults(hilowDB);
                radMsgLog (PRI_HIGH, "dbsqliteHiLowGetMonth: failed to extract data");
                return ERROR;
            }

            tempSensor->cumulative      += store.cumulative;
            tempSensor->samples         += store.samples;
            if (tempSensor->low > store.low)
            {
                tempSensor->low         = store.low;
                tempSensor->time_low    = store.time_low;
            }
            if (tempSensor->high < store.high)
            {
                tempSensor->high        = store.high;
                tempSensor->time_high   = store.time_high;
                tempSensor->when_high   = store.when_high;
            }
        }
    
        radsqlitedirectReleaseResults(hilowDB);
    }

    if (yearRainFlag)
    {
        return retVal;
    }

    // Do the wind dir here:
    sprintf (query, "SELECT * FROM %s WHERE dateTime >= '%d' AND dateTime < '%d' ORDER BY dateTime ASC", 
             WVIEW_HILOW_WINDDIR_TABLE, (int)first, (int)last);

    // Execute the query:
    if (radsqlitedirectQuery(hilowDB, query, TRUE) == ERROR)
    {
        return ERROR;
    }

    retVal = 0;
    for (rowDescr = radsqlitedirectGetRow(hilowDB);
         rowDescr != NULL;
         rowDescr = radsqlitedirectGetRow(hilowDB))
    {
        retVal ++;
        for (i = 0; i < WAVG_NUM_BINS; i ++)
        {
            bins[i] = 0;
            sprintf(binName, "bin%d", i);
            field = radsqlitedirectFieldGet(rowDescr, binName);
            if (field == NULL)
            {
                radsqlitedirectReleaseResults(hilowDB);
                return ERROR;
            }
            else
            {
                bins[i] = (int)radsqliteFieldGetBigIntValue(field);
            }
        }

        windAverageAddBins(&sensors->wind[timeFrame], bins);
    }

    radsqlitedirectReleaseResults(hilowDB);
    return retVal;
}

//  ... Retrieve the last HILOW hour in the database;
//  ... Returns time_t or ERROR
time_t dbsqliteHiLowGetLastUpdate(void)
{
    return (hilowGetLastUpdateTime());
}


