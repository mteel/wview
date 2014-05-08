//----------------------------------------------------------------------------
//
//  FILENAME:
//        dbsqlite.c
//
//  PURPOSE:
//        Provide the weather station archive database utilities.
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

//  ... global memory declarations


//  ... local memory:

static SQLITE_DATABASE_ID   archiveDB = NULL;
static const char*  ArchiveValueName[DATA_INDEX_MAX] = 
{
    "barometer",
    "pressure",
    "altimeter",
    "inTemp",
    "outTemp",
    "inHumidity",
    "outHumidity",
    "windSpeed",
    "windDir",
    "windGust",
    "windGustDir",
    "rainRate",
    "rain",
    "dewpoint",
    "windchill",
    "heatindex",
    "rxCheckPercent",
    "ET",
    "radiation",
    "UV",
    "extraTemp1",
    "extraTemp2",
    "extraTemp3",
    "soilTemp1",
    "soilTemp2",
    "soilTemp3",
    "soilTemp4",
    "leafTemp1",
    "leafTemp2",
    "extraHumid1",
    "extraHumid2",
    "soilMoist1",
    "soilMoist2",
    "soilMoist3",
    "soilMoist4",
    "leafWet1",
    "leafWet2",
    "txBatteryStatus",
    "consBatteryVoltage",
    "hail",
    "hailRate",
    "heatingTemp",
    "heatingVoltage",
    "supplyVoltage",
    "referenceVoltage",
    "windBatteryStatus",
    "rainBatteryStatus",
    "outTempBatteryStatus",
    "inTempBatteryStatus"
};

static char     DefaultArchivePath[_MAX_PATH] = { 0 };


static float noConversion (float value)
{
    return value;
}


// Define the jump tables for conversion utilities:

static float (*imperialToMetric_convertors[]) (float value) =
{
    wvutilsConvertINHGToHPA,
    wvutilsConvertINHGToHPA,
    wvutilsConvertINHGToHPA,
    wvutilsConvertFToC,
    wvutilsConvertFToC,
    noConversion,
    noConversion,
    wvutilsGetWindSpeed,
    noConversion,
    wvutilsGetWindSpeed,
    noConversion,
    wvutilsConvertRainINToMetric,
    wvutilsConvertRainINToMetric,
    wvutilsConvertFToC,
    wvutilsConvertFToC,
    wvutilsConvertFToC,
    noConversion,
    wvutilsConvertRainINToMetric,
    noConversion,
    noConversion,
    wvutilsConvertFToC,
    wvutilsConvertFToC,
    wvutilsConvertFToC,
    wvutilsConvertFToC,
    wvutilsConvertFToC,
    wvutilsConvertFToC,
    wvutilsConvertFToC,
    wvutilsConvertFToC,
    wvutilsConvertFToC,
    noConversion,
    noConversion,
    noConversion,
    noConversion,
    noConversion,
    noConversion,
    noConversion,
    noConversion,
    noConversion,
    noConversion,
    wvutilsConvertRainINToMetric,
    wvutilsConvertRainINToMetric,
    noConversion,
    noConversion,
    noConversion,
    noConversion,
    noConversion,
    noConversion,
    noConversion,
    noConversion
};

static float (*metricToImperial_convertors[]) (float value) =
{
    wvutilsConvertHPAToINHG,
    wvutilsConvertHPAToINHG,
    wvutilsConvertHPAToINHG,
    wvutilsConvertCToF,
    wvutilsConvertCToF,
    noConversion,
    noConversion,
    wvutilsGetWindSpeed,
    noConversion,
    wvutilsGetWindSpeed,
    noConversion,
    wvutilsConvertRainMetricToIN,
    wvutilsConvertRainMetricToIN,
    wvutilsConvertCToF,
    wvutilsConvertCToF,
    wvutilsConvertCToF,
    noConversion,
    wvutilsConvertRainMetricToIN,
    noConversion,
    noConversion,
    wvutilsConvertCToF,
    wvutilsConvertCToF,
    wvutilsConvertCToF,
    wvutilsConvertCToF,
    wvutilsConvertCToF,
    wvutilsConvertCToF,
    wvutilsConvertCToF,
    wvutilsConvertCToF,
    wvutilsConvertCToF,
    noConversion,
    noConversion,
    noConversion,
    noConversion,
    noConversion,
    noConversion,
    noConversion,
    noConversion,
    noConversion,
    noConversion,
    wvutilsConvertRainMetricToIN,
    wvutilsConvertRainMetricToIN,
    noConversion,
    noConversion,
    noConversion,
    noConversion,
    noConversion,
    noConversion,
    noConversion,
    noConversion
};


//  ... ----- static (local) methods -----

static const char* getArchiveDBFilename(void)
{
    static char     dbFileName[_MAX_PATH];

    if (strlen(DefaultArchivePath) > 0)
    {
        sprintf (dbFileName, "%s/%s", DefaultArchivePath, WVIEW_ARCHIVE_DATABASE);
    }
    else
    {
        sprintf (dbFileName, "%s/%s", wvutilsGetArchivePath(), WVIEW_ARCHIVE_DATABASE);
    }

    return dbFileName;
}

static int getDBData(SQLITE_DIRECT_ROW row, ARCHIVE_PKT* data)
{
    SQLITE_FIELD_ID field;
    Data_Indices index;

    field = radsqlitedirectFieldGet(row, "dateTime");
    if (field == NULL)
    {
        radMsgLog (PRI_MEDIUM, "dbsqlite: radsqlitedirectFieldGet failed!");
        return ERROR;
    }
    else
    {
        data->dateTime = (int32_t)radsqliteFieldGetBigIntValue(field);
    }

    field = radsqlitedirectFieldGet(row, "usUnits");
    if (field == NULL)
    {
        radMsgLog (PRI_MEDIUM, "dbsqlite: radsqlitedirectFieldGet failed!");
        return ERROR;
    }
    else
    {
        data->usUnits = (long)radsqliteFieldGetBigIntValue(field);
    }

    field = radsqlitedirectFieldGet(row, "interval");
    if (field == NULL)
    {
        radMsgLog (PRI_MEDIUM, "dbsqlite: radsqlitedirectFieldGet failed!");
        return ERROR;
    }
    else
    {
        data->interval = (long)radsqliteFieldGetBigIntValue(field);
    }

    for (index = DATA_INDEX_barometer; index < DATA_INDEX_MAX; index ++)
    {
        field = radsqlitedirectFieldGet(row, ArchiveValueName[index]);
        if (field == NULL || FIELD_IS_NULL(field))
        {
            data->value[index] = ARCHIVE_VALUE_NULL;
        }
        else
        {
            data->value[index] = (float)radsqliteFieldGetDoubleValue(field);
        }
    }

    return OK;
}

static int insertDBData(ARCHIVE_PKT* data)
{
    SQLITE_ROW_ID           row;
    SQLITE_FIELD_ID         field;
    Data_Indices            index;

    if (archiveDB == NULL)
    {
        radMsgLog (PRI_HIGH, "dbsqlite: failed to open %s!", getArchiveDBFilename());
        return ERROR;
    }

    row = radsqliteTableDescriptionGet (archiveDB, "archive");
    if (row == NULL)
    {
        printf ("dbsqlite: databaseTableDescriptionGet failed!");
        return ERROR;
    }

    field = radsqliteFieldGet(row, "dateTime");
    if (field == NULL)
    {
        radMsgLog (PRI_MEDIUM, "dbsqlite: radsqliteFieldGet failed!");
        radsqliteRowDescriptionDelete(row);
        return ERROR;
    }
    else
    {
        radsqliteFieldSetBigIntValue(field, (uint64_t)data->dateTime);
    }

    field = radsqliteFieldGet(row, "usUnits");
    if (field == NULL)
    {
        radMsgLog (PRI_MEDIUM, "dbsqlite: radsqliteFieldGet failed!");
        radsqliteRowDescriptionDelete(row);
        return ERROR;
    }
    else
    {
        radsqliteFieldSetBigIntValue(field, (uint64_t)data->usUnits);
    }

    field = radsqliteFieldGet(row, "interval");
    if (field == NULL)
    {
        radMsgLog (PRI_MEDIUM, "dbsqlite: radsqliteFieldGet failed!");
        radsqliteRowDescriptionDelete(row);
        return ERROR;
    }
    else
    {
        radsqliteFieldSetBigIntValue(field, (uint64_t)data->interval);
    }

    for (index = DATA_INDEX_barometer; index < DATA_INDEX_MAX; index ++)
    {
        field = radsqliteFieldGet(row, ArchiveValueName[index]);
        if (field == NULL)
        {
            radMsgLog (PRI_MEDIUM, "dbsqlite: radsqliteFieldGet %s failed!", ArchiveValueName[index]);
            radsqliteRowDescriptionDelete(row);
            return ERROR;
        }
        else
        {
            if (data->value[index] <= ARCHIVE_VALUE_NULL)
            {
                radsqliteFieldSetToNull(field);
            }
            else
            {
                radsqliteFieldSetDoubleValue(field, (double)data->value[index]);
            }
        }
    }

    // insert the row:
    if (radsqliteTableInsertRow(archiveDB, "archive", row) == ERROR)
    {
        radMsgLog (PRI_HIGH, "dbsqlite: radsqliteTableInsertRow failed!");
        radsqliteRowDescriptionDelete(row);
        return ERROR;
    }

    radsqliteRowDescriptionDelete(row);
    return OK;
}

static int lastWDIR;
#if defined(BUILD_HTMLGEND) || defined(BUILD_WVIEWD)

//  return num minutes processed or error
static int rollIntoAverages
(
    int                     isMetricUnits,
    WAVG_ID                 windId,
    time_t                  startTime,
    HISTORY_DATA*           store,
    int                     numMins
)
{
    char                    query[DB_SQLITE_QUERY_LENGTH_MAX];
    int                     recordIsUSUnits, mins = 0;
    float                   value;
    time_t                  endTime = startTime + (numMins * 60);
    SQLITE_DIRECT_ROW       rowDescr;
    SQLITE_FIELD_ID         field;
    Data_Indices            index;

    if (archiveDB == NULL)
    {
        radMsgLog (PRI_HIGH, "rollIntoAverages: failed to open %s!", getArchiveDBFilename());
        return ERROR;
    }

    // Build the query:
    sprintf (query, "SELECT * FROM archive WHERE dateTime >= '%d' AND dateTime < '%d' ORDER BY dateTime ASC",
             (int)startTime, (int)endTime);

    // Execute the query:
    if (radsqlitedirectQuery(archiveDB, query, TRUE) == ERROR)
    {
        return ERROR;
    }

    for (rowDescr = radsqlitedirectGetRow(archiveDB); 
         rowDescr != NULL; 
         rowDescr = radsqlitedirectGetRow(archiveDB))
    {
        field = radsqlitedirectFieldGet(rowDescr, "interval");
        if (field == NULL)
        {
            radMsgLog (PRI_MEDIUM, "rollIntoAverages: radsqlitedirectFieldGet failed!");
            radsqlitedirectReleaseResults(archiveDB);
            return ERROR;
        }
        else
        {
            mins += (int)radsqliteFieldGetBigIntValue(field);
        }

        field = radsqlitedirectFieldGet(rowDescr, "usUnits");
        if (field == NULL)
        {
            radMsgLog (PRI_MEDIUM, "rollIntoAverages: radsqlitedirectFieldGet failed!");
            radsqlitedirectReleaseResults(archiveDB);
            return ERROR;
        }
        else
        {
            recordIsUSUnits = (int)radsqliteFieldGetBigIntValue(field);
        }

        for (index = DATA_INDEX_barometer; index < DATA_INDEX_MAX; index ++)
        {
            field = radsqlitedirectFieldGet(rowDescr, ArchiveValueName[index]);
            if (field == NULL)
            {
                radMsgLog (PRI_MEDIUM, "rollIntoAverages: radsqlitedirectFieldGet %s failed!", ArchiveValueName[index]);
                radsqlitedirectReleaseResults(archiveDB);
                return ERROR;
            }
            else
            {
                if (! FIELD_IS_NULL(field))
                {
                    value = (float)radsqliteFieldGetDoubleValue(field);

                    // Check for wview NULL value:
                    if (value > ARCHIVE_VALUE_NULL)
                    {
                        store->samples[index] += 1;

                        // Handle WIND separately:
                        if (index == DATA_INDEX_windSpeed || index == DATA_INDEX_windGust)
                        {
                            if (recordIsUSUnits)
                            {
                                store->values[index] += wvutilsGetWindSpeed(value);
                            }
                            else
                            {
                                store->values[index] += wvutilsGetWindSpeedMetric(value);
                            }
                        }
                        else
                        {
                            if (isMetricUnits & recordIsUSUnits)
                            {
                                store->values[index] += (*imperialToMetric_convertors[index])(value);
                            }
                            else if (! isMetricUnits & ! recordIsUSUnits)
                            {
                                store->values[index] += (*metricToImperial_convertors[index])(value);
                            }
                            else
                            {
                                store->values[index] += value;
                            }
                        }
                    }
                }
            }
        }

        field = radsqlitedirectFieldGet(rowDescr, ArchiveValueName[DATA_INDEX_windDir]);
        if (field == NULL)
        {
            radMsgLog (PRI_MEDIUM, "rollIntoAverages: radsqlitedirectFieldGet failed!");
            radsqlitedirectReleaseResults(archiveDB);
            return ERROR;
        }
        else if (! FIELD_IS_NULL(field))
        {
            value = (float)radsqliteFieldGetDoubleValue(field);
            if (value >= 0 && value < 360)
            {
                lastWDIR = (int)value;
            }
            windAverageAddValue (windId, lastWDIR);
        }
    }

    radsqlitedirectReleaseResults(archiveDB);


    // Finally, check to be sure values were found, if not, set to ARCHIVE_VALUE_NULL:
    for (index = DATA_INDEX_barometer; index < DATA_INDEX_MAX; index ++)
    {
        if (store->samples[index] == 0)
        {
            // set value to NULL:
            store->values[index] = ARCHIVE_VALUE_NULL;
        }
    }
        
    return mins;
}
#endif

static time_t getNewestDateTime (ARCHIVE_PKT *newRec)
{
    char                    query[DB_SQLITE_QUERY_LENGTH_MAX];
    SQLITE_DIRECT_ROW       rowDescr;
    SQLITE_FIELD_ID         field;
    time_t                  retVal;

    if (archiveDB == NULL)
    {
        radMsgLog (PRI_HIGH, "getNewestDateTime: failed to open %s!", getArchiveDBFilename());
        return ERROR;
    }

    // Build the query:
    sprintf (query, "SELECT MAX(dateTime) AS 'max' FROM archive");

    // Execute the query:
    if (radsqlitedirectQuery(archiveDB, query, TRUE) == ERROR)
    {
        radMsgLog (PRI_HIGH, "getNewestDateTime: radsqlitedirectQuery failed!");
        return ERROR;
    }

    rowDescr = radsqlitedirectGetRow(archiveDB);
    if (rowDescr == NULL)
    {
        radMsgLog (PRI_MEDIUM, 
                   "getNewestDateTime: radsqlitedirectGetRow failed!");
        radsqlitedirectReleaseResults(archiveDB);
        return ERROR;
    }

    field = radsqlitedirectFieldGet(rowDescr, "max");
    if (field == NULL)
    {
        radMsgLog (PRI_MEDIUM, 
                   "getNewestDateTime: radsqlitedirectFieldGet failed!");
        radsqlitedirectReleaseResults(archiveDB);
        return ERROR;
    }

    retVal = (time_t)radsqliteFieldGetBigIntValue(field);

    // Clean up:
    radsqlitedirectReleaseResults(archiveDB);

    // Now grab the entire row:
    sprintf (query, "SELECT * FROM archive WHERE dateTime = '%d'", (int)retVal);

    // Execute the query:
    if (radsqlitedirectQuery(archiveDB, query, TRUE) == ERROR)
    {
        radMsgLog (PRI_HIGH, "getNewestDateTime: radsqlitedirectQuery failed!");
        return ERROR;
    }

    rowDescr = radsqlitedirectGetRow(archiveDB);
    if (rowDescr == NULL)
    {
        radMsgLog (PRI_MEDIUM, 
                   "getNewestDateTime: radsqlitedirectGetRow failed!");
        radsqlitedirectReleaseResults(archiveDB);
        return ERROR;
    }

    // finally copy it to the internal PKT:
    getDBData(rowDescr, newRec);

    radsqlitedirectReleaseResults(archiveDB);
    return retVal;
}

static int getNextRecord (time_t dateTime, ARCHIVE_PKT* newRec)
{
    char                    query[DB_SQLITE_QUERY_LENGTH_MAX];
    SQLITE_DIRECT_ROW       rowDescr;
    SQLITE_FIELD_ID         field;
    time_t                  retVal;

    if (archiveDB == NULL)
    {
        radMsgLog (PRI_HIGH, "getNextRecord: failed to open %s!", getArchiveDBFilename());
        return ERROR;
    }

    // Build the query:
    sprintf (query, "SELECT MIN(dateTime) AS 'min' FROM archive WHERE dateTime > '%d'",
             (int)dateTime);

    // Execute the query:
    if (radsqlitedirectQuery(archiveDB, query, TRUE) == ERROR)
    {
        return ERROR;
    }

    rowDescr = radsqlitedirectGetRow(archiveDB);
    if (rowDescr == NULL)
    {
        radsqlitedirectReleaseResults(archiveDB);
        return ERROR;
    }

    field = radsqlitedirectFieldGet(rowDescr, "min");
    if (field == NULL)
    {
        radMsgLog (PRI_MEDIUM, 
                   "getNextRecord: radsqlitedirectFieldGet failed!");
        radsqlitedirectReleaseResults(archiveDB);
        return ERROR;
    }

    retVal = (time_t)radsqliteFieldGetBigIntValue(field);

    // Clean up:
    radsqlitedirectReleaseResults(archiveDB);

    if ((int)retVal == 0)
    {
        return ERROR;
    }

    // Now grab the entire row:
    sprintf (query, "SELECT * FROM archive WHERE dateTime = '%d'", (int)retVal);

    // Execute the query:
    if (radsqlitedirectQuery(archiveDB, query, TRUE) == ERROR)
    {
        return ERROR;
    }

    rowDescr = radsqlitedirectGetRow(archiveDB);
    if (rowDescr == NULL)
    {
        radsqlitedirectReleaseResults(archiveDB);
        return ERROR;
    }

    // finally copy it to the internal PKT:
    getDBData(rowDescr, newRec);

    radsqlitedirectReleaseResults(archiveDB);
    return retVal;
}

static int getRecord (time_t dateTime, ARCHIVE_PKT* newRec)
{
    char                    query[DB_SQLITE_QUERY_LENGTH_MAX];
    SQLITE_DIRECT_ROW       rowDescr;

    if (archiveDB == NULL)
    {
        radMsgLog (PRI_HIGH, "getRecord: failed to open %s!", getArchiveDBFilename());
        return ERROR;
    }

    // grab the entire row:
    sprintf (query, "SELECT * FROM archive WHERE dateTime = '%d'", (int)dateTime);

    // Execute the query:
    if (radsqlitedirectQuery(archiveDB, query, TRUE) == ERROR)
    {
        return ERROR;
    }

    rowDescr = radsqlitedirectGetRow(archiveDB);
    if (rowDescr == NULL)
    {
        radsqlitedirectReleaseResults(archiveDB);
        return ERROR;
    }

    // finally copy it to the internal PKT:
    getDBData(rowDescr, newRec);

    radsqlitedirectReleaseResults(archiveDB);
    return OK;
}

static int getFirstRecord(time_t startTime, time_t stopTime, ARCHIVE_PKT* recordStore)
{
    char                    query[DB_SQLITE_QUERY_LENGTH_MAX];
    SQLITE_DIRECT_ROW       rowDescr;

    if (archiveDB == NULL)
    {
        radMsgLog (PRI_HIGH, "getFirstRecord: failed to open %s!", getArchiveDBFilename());
        return ERROR;
    }

    // grab the entire row:
    sprintf (query, "SELECT * FROM archive WHERE dateTime BETWEEN '%d' AND '%d' ORDER BY dateTime ASC", 
             (int)startTime, (int)stopTime);

    // Execute the query:
    if (radsqlitedirectQuery(archiveDB, query, TRUE) == ERROR)
    {
        return ERROR;
    }

    rowDescr = radsqlitedirectGetRow(archiveDB);
    if (rowDescr == NULL)
    {
        radsqlitedirectReleaseResults(archiveDB);
        return ERROR;
    }

    // finally copy it to the internal PKT:
    getDBData(rowDescr, recordStore);

    radsqlitedirectReleaseResults(archiveDB);
    return OK;
}

static int getCount (char* whereClause)
{
    char                    query[DB_SQLITE_QUERY_LENGTH_MAX];
    SQLITE_DIRECT_ROW       rowDescr;
    SQLITE_FIELD_ID         field;
    int                     retVal;

    if (archiveDB == NULL)
    {
        radMsgLog (PRI_HIGH, "getCount: failed to open %s!", getArchiveDBFilename());
        return ERROR;
    }

    // Build the query:
    sprintf (query, "SELECT COUNT(*) AS 'number' FROM archive where %s",
             whereClause);

    // Execute the query:
    if (radsqlitedirectQuery(archiveDB, query, TRUE) == ERROR)
    {
        radMsgLog (PRI_HIGH, "getCount: radsqlitedirectQuery failed!");
        return ERROR;
    }

    rowDescr = radsqlitedirectGetRow(archiveDB);
    if (rowDescr == NULL)
    {
        radMsgLog (PRI_MEDIUM, 
                   "getCount: radsqlitedirectGetRow failed!");
        radsqlitedirectReleaseResults(archiveDB);
        return ERROR;
    }

    field = radsqlitedirectFieldGet(rowDescr, "number");
    if (field == NULL)
    {
        radMsgLog (PRI_MEDIUM, 
                   "getCount: radsqlitedirectFieldGet failed!");
        radsqlitedirectReleaseResults(archiveDB);
        return ERROR;
    }

    retVal = (int)radsqliteFieldGetBigIntValue(field);

    // Clean up:
    radsqlitedirectReleaseResults(archiveDB);

    return retVal;
}


//  #####################  API Functions #####################

// Initialize the database interface (returns OK or ERROR):
int dbsqliteArchiveInit(void)
{
    archiveDB = radsqliteOpen (getArchiveDBFilename());
    if (archiveDB == NULL)
    {
        radMsgLog (PRI_HIGH, "dbsqliteArchiveInit: failed to open %s!", getArchiveDBFilename());
        return ERROR;
    }

    return OK;
}

// Clean up the database interface:
void dbsqliteArchiveExit(void)
{
    if (archiveDB)
        radsqliteClose(archiveDB);
}


// PRAGMA statement to modify the operation of the SQLite library
int dbsqliteArchivePragmaSet(char *pragma, char *setting)
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
    if (radsqliteQuery(archiveDB, query, FALSE) == ERROR)
    {
        return ERROR;
    }

    return OK;
}

//  ... Reset the default archive database location (used by conversion utils);
void dbsqliteArchiveSetPath (char* newPath)
{
    wvstrncpy(DefaultArchivePath, newPath, _MAX_PATH);
}

char* dbsqliteArchiveGetPath (void)
{
    return DefaultArchivePath;
}

#if defined(BUILD_HTMLGEND) || defined(BUILD_WVIEWD)

//  ... calculate averages over a given period of time
//  ... (given in arcInterval minute samples);
//  ... this will zero out the HISTORY_DATA store before beginning
//  ... the processing;
//  ... returns OK or ERROR

int dbsqliteArchiveGetAverages
(
    int             isMetricUnits,
    int             arcInterval,
    HISTORY_DATA    *store,
    time_t          startTime,
    int             numSamples
)
{
    int             retVal;
    int             numMins = numSamples * arcInterval;
    WAVG            windAvg;

    memset (store, 0, sizeof (HISTORY_DATA));
    store->startTime = startTime;

    //  create the wind average object
    windAverageReset (&windAvg);

    retVal = rollIntoAverages(isMetricUnits,
                              &windAvg,
                              startTime,
                              store,
                              numMins);

    if (retVal > 0)
    {
        store->values[DATA_INDEX_windDir] = windAverageCompute (&windAvg);
    }

    return retVal;
}
#endif

#if defined(BUILD_HTMLGEND)

// write out all ASCII archive records for the given day to 'filename'
int dbsqliteWriteDailyArchiveReport
(
    char                    *filename,
    time_t                  timeval,
    int                     isMetric,
    int                     arcInterval,
    void                    (*writeHeader) (FILE *file)
)
{
    struct tm               locTime;
    time_t                  startTime, stopTime;
    struct stat             fileStatus;
    char                    query[DB_SQLITE_QUERY_LENGTH_MAX];
    SQLITE_DIRECT_ROW       rowDescr;
    ARCHIVE_PKT             arcRecord;
    int                     numrecs = 0;

    localtime_r (&timeval, &locTime);
    locTime.tm_hour  = 0;
    locTime.tm_min   = 0;
    locTime.tm_sec   = 0;
    locTime.tm_isdst = -1;
    startTime = (time_t)mktime(&locTime);
    stopTime = startTime + WV_SECONDS_IN_DAY;

    if (stat (filename, &fileStatus) != -1)
    {
        //  exists, delete it
        unlink (filename);
    }

    if (archiveDB == NULL)
    {
        radMsgLog (PRI_HIGH, "dbsqliteWriteDailyArchiveReport: failed to open %s!", getArchiveDBFilename());
        return ERROR;
    }

    // grab the rows:
    sprintf (query, "SELECT * FROM archive WHERE dateTime >= '%d' AND dateTime < '%d' ORDER BY dateTime ASC", 
             (int)startTime, (int)stopTime);

    // Execute the query:
    if (radsqlitedirectQuery(archiveDB, query, TRUE) == ERROR)
    {
        return ERROR;
    }

    for (rowDescr = radsqlitedirectGetRow(archiveDB);
         rowDescr != NULL;
         rowDescr = radsqlitedirectGetRow(archiveDB))
    {
        // copy it to the internal PKT:
        if (getDBData(rowDescr, &arcRecord) == ERROR)
        {
            radMsgLog(PRI_MEDIUM, "dbsqliteWriteDailyArchiveReport: getDBData failed!");
            continue;
        }

        // call our little updater
        dbsqliteUpdateDailyArchiveReport(filename, &arcRecord, writeHeader, isMetric);

        numrecs ++;
    }

    radsqlitedirectReleaseResults(archiveDB);

    if (numrecs > 0)
    {
        return OK;
    }
    else
    {
        return ERROR;
    }
}

// update (or create) the current day's ASCII archive records file
int dbsqliteUpdateDailyArchiveReport
(
    char            *file,
    ARCHIVE_PKT     *data,
    void            (*writeHeaderFcn) (FILE *file),
    int             isMetric
)

{
    FILE            *outfile;
    char            temp[256];
    struct stat     fileStatus;
    int             writeHeader = FALSE;

    if (stat (file, &fileStatus) == -1)
    {
        //  new file, write a header
        writeHeader = TRUE;
    }

    // first, take care of creating/opening the file
    outfile = fopen (file, "a");
    if (outfile == NULL)
    {
        return ERROR;
    }

    if (writeHeader)
    {
        // callback the user with the supplied routine
        (*writeHeaderFcn)(outfile);
    }

    // append the new record

    if (!isMetric)
    {
        sprintf (temp,
                 "%4.4d%2.2d%2.2d %2.2d:%2.2d\t%.1f\t%.1f\t%.1f\t%.0f\t%.1f\t%.0f\t%.0f\t%.0f\t%.2f\t%.3f\t%.0f\t%.3f\t%.1f\n",
                 wvutilsGetYear(data->dateTime),
                 wvutilsGetMonth(data->dateTime),
                 wvutilsGetDay(data->dateTime),
                 wvutilsGetHour(data->dateTime),
                 wvutilsGetMin(data->dateTime),
                 data->value[DATA_INDEX_outTemp],
                 data->value[DATA_INDEX_windchill],
                 data->value[DATA_INDEX_heatindex],
                 data->value[DATA_INDEX_outHumidity],
                 data->value[DATA_INDEX_dewpoint],
                 data->value[DATA_INDEX_windSpeed],
                 data->value[DATA_INDEX_windGust],
                 data->value[DATA_INDEX_windDir],
                 data->value[DATA_INDEX_rain],
                 data->value[DATA_INDEX_barometer],
                 data->value[DATA_INDEX_radiation],
                 data->value[DATA_INDEX_ET],
                 data->value[DATA_INDEX_UV]);
    }
    else
    {
        //    "--Timestamp---\tTemp\tChill\tHIndex\tHumid\tDewpt\tWind\tHiWind\tWindDir\tRain\tBarom\tSolar\tET\tUV\n"
        sprintf (temp,
                 "%4.4d%2.2d%2.2d %2.2d:%2.2d\t%.1f\t%.1f\t%.1f\t%.0f\t%.1f\t%.0f\t%.0f\t%.0f\t%.1f\t%.1f\t%.0f\t%.3f\t%.1f\n",
                 wvutilsGetYear(data->dateTime),
                 wvutilsGetMonth(data->dateTime),
                 wvutilsGetDay(data->dateTime),
                 wvutilsGetHour(data->dateTime),
                 wvutilsGetMin(data->dateTime),
                 wvutilsConvertFToC(data->value[DATA_INDEX_outTemp]),
                 wvutilsConvertFToC(data->value[DATA_INDEX_windchill]),
                 wvutilsConvertFToC(data->value[DATA_INDEX_heatindex]),
                 data->value[DATA_INDEX_outHumidity],
                 wvutilsConvertFToC(data->value[DATA_INDEX_dewpoint]),
                 wvutilsConvertMPHToKPH(data->value[DATA_INDEX_windSpeed]),
                 wvutilsConvertMPHToKPH(data->value[DATA_INDEX_windGust]),
                 data->value[DATA_INDEX_windDir],
                 wvutilsConvertRainINToMetric(data->value[DATA_INDEX_rain]),
                 wvutilsConvertINHGToHPA(data->value[DATA_INDEX_barometer]),
                 data->value[DATA_INDEX_radiation],
                 wvutilsConvertRainINToMetric(data->value[DATA_INDEX_ET]),
                 data->value[DATA_INDEX_UV]);
    }

    if (fwrite (temp, 1, strlen(temp), outfile) != strlen(temp))
    {
        fclose (outfile);
        return ERROR;
    }


    // cleanup and bail out
    fclose (outfile);

    if (writeHeader)
        return 1;
    else
        return OK;
}
#endif


//  ... append an archive record to the archive database;
//  ... returns OK or ERROR

int dbsqliteArchiveStoreRecord (ARCHIVE_PKT* record)
{
    char        fileName[128];

    insertDBData(record);
    sprintf (fileName, "%s/export/%s", wvutilsGetConfigPath(), WVIEW_ARCHIVE_MARKER_FILE);
    wvutilsWriteMarkerFile(fileName, record->dateTime);
    return OK;
}


//  ... search the archive path for the most recent archive record date;
//  ... returns OK or ERROR if no archives found

time_t dbsqliteArchiveGetNewestTime (ARCHIVE_PKT* newestRecord)
{
    time_t      retVal;

    retVal = getNewestDateTime(newestRecord);
    return retVal;
}


//  ... search the archive database for the next archive record after 'dateTime';
//  ... returns time of next record or ERROR if no archives found

time_t dbsqliteArchiveGetNextRecord (time_t dateTime, ARCHIVE_PKT* recordStore)
{
    time_t      retVal;

    retVal = getNextRecord(dateTime, recordStore);
    return retVal;
}


//  ... search the archive database for the archive record with timestamp 'dateTime';
//  ... returns OK or ERROR if no archive record found

int dbsqliteArchiveGetRecord (time_t dateTime, ARCHIVE_PKT* recordStore)
{
    int     retVal;

    retVal = getRecord(dateTime, recordStore);
    return retVal;
}

//  ... search the archive database for the first archive record in the given 
//  ... timestamp range;
//  ... returns OK or ERROR if no archive record found

int dbsqliteArchiveGetFirstRecord
(
    time_t          startTime, 
    time_t          stopTime, 
    ARCHIVE_PKT*    recordStore
)
{
    int             retVal;

    retVal = getFirstRecord(startTime, stopTime, recordStore);
    return retVal;
}

int dbsqliteArchiveExecutePerRecord
(
    void                    (*function)(ARCHIVE_PKT* rec, void* data),
    void*                   userData,
    time_t                  startTime, 
    time_t                  stopTime,
    char*                   selectClause
)
{
    char                    query[DB_SQLITE_QUERY_LENGTH_MAX];
    SQLITE_DIRECT_ROW       rowDescr;
    ARCHIVE_PKT             rowData;
    int                     numrecs = 0;

    if (archiveDB == NULL)
    {
        radMsgLog (PRI_HIGH, "dbsqliteArchiveExecutePerRecord: failed to open %s!", getArchiveDBFilename());
        return ERROR;
    }

    if (selectClause)
    {
        sprintf (query, "SELECT dateTime,usUnits,interval,%s FROM archive WHERE dateTime >= '%d' AND dateTime < '%d' ORDER BY dateTime ASC", 
                 selectClause, (int)startTime, (int)stopTime);
    }
    else
    {
        // grab the entire row:
        sprintf (query, "SELECT * FROM archive WHERE dateTime >= '%d' AND dateTime < '%d' ORDER BY dateTime ASC", 
                 (int)startTime, (int)stopTime);
    }

    // Execute the query:
    if (radsqlitedirectQuery(archiveDB, query, TRUE) == ERROR)
    {
        return ERROR;
    }

    for (rowDescr = radsqlitedirectGetRow(archiveDB); 
         rowDescr != NULL; 
         rowDescr = radsqlitedirectGetRow(archiveDB))
    {
        if (getDBData(rowDescr, &rowData) == ERROR)
        {
            radMsgLog (PRI_HIGH, "dbsqliteArchiveExecutePerRecord: getDBData failed!");
            radsqlitedirectReleaseResults(archiveDB);
            return ERROR;
        }

        numrecs ++;

        // Call the supplied function:
        (*function)(&rowData, userData);
    }

    radsqlitedirectReleaseResults(archiveDB);
    return numrecs;
}

int dbsqliteArchiveGetCount(char* whereClause)
{
    return (getCount(whereClause));
}

