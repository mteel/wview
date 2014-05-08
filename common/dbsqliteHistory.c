//----------------------------------------------------------------------------
//
//  FILENAME:
//        dbsqliteHistory.c
//
//  PURPOSE:
//        Provide the weather station historical database utilities.
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


#ifdef BUILD_HTMLGEND


//  ... local memory:

static const char*  historyName[DATA_INDEX_MAX] = 
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



//  ... ----- static (local) methods -----

static const char* getHistoryDBFilename(void)
{
    static char     dbHistoryFileName[_MAX_PATH];

    if (strlen(DefaultArchivePath) > 0)
    {
        sprintf (dbHistoryFileName, "%s/%s", DefaultArchivePath, WVIEW_HISTORY_DATABASE);
    }
    else
    {
        sprintf (dbHistoryFileName, "%s/%s", wvutilsGetArchivePath(), WVIEW_HISTORY_DATABASE);
    }

    return dbHistoryFileName;
}

static int getHistoryRecord (SQLITE_DATABASE_ID historyDB, time_t date, HISTORY_DATA* store)
{
    char                    query[DB_SQLITE_QUERY_LENGTH_MAX];
    SQLITE_DIRECT_ROW       rowDescr;
    Data_Indices            index;
    SQLITE_FIELD_ID         field;
    struct tm               locTime;
    char                    dayString[64];
    int                     RowCount = 0;

    // First let's make sure there aren't 2 records for the same day;
    // if there are, delete them both and return ERROR.
    localtime_r (&date, &locTime);
    snprintf (dayString, 64, "%4.4d-%2.2d-%2.2d",
              locTime.tm_year + 1900,
              locTime.tm_mon + 1,
              locTime.tm_mday);

    // get a count of matching rows:
    sprintf (query,
             "select count(date) from %s where date(date, 'unixepoch', 'localtime') = '%s'",
             WVIEW_DAY_HISTORY_TABLE, dayString);

    // Execute the query:
    if (radsqlitedirectQuery(historyDB, query, TRUE) == ERROR)
    {
        radMsgLog (PRI_MEDIUM, "dbsqliteHistory: row count query failed.");
        return ERROR;
    }
    rowDescr = radsqlitedirectGetRow(historyDB);
    if (rowDescr == NULL)
    {
        radMsgLog (PRI_MEDIUM, "dbsqliteHistory: row count get row failed.");
        radsqlitedirectReleaseResults(historyDB);
        return ERROR;
    }
    field = radsqlitedirectFieldGet(rowDescr, "count(date)");
    if ((field == NULL) || ((radsqliteFieldGetType(field) & SQLITE_FIELD_VALUE_IS_NULL) != 0))
    {
        radMsgLog (PRI_MEDIUM, "dbsqliteHistory: row count get field failed.");
        radsqlitedirectReleaseResults(historyDB);
        return ERROR;
    }
    RowCount = (int)radsqliteFieldGetBigIntValue(field);
    radsqlitedirectReleaseResults(historyDB);

    if (RowCount > 1)
    {
        // Delete all rows, we'll regenerate from archive data:
        sprintf (query,
                 "delete from %s where date(date, 'unixepoch', 'localtime') = '%s'",
                 WVIEW_DAY_HISTORY_TABLE, dayString);

        // Execute the query:
        radsqliteQuery(historyDB, query, FALSE);

        // Return ERROR regardless...
        return ERROR;
    }

    // Proceed as normal if here.
    // grab the entire row:
    sprintf (query, "SELECT * FROM %s WHERE date = '%d'",
             WVIEW_DAY_HISTORY_TABLE, (int)date);

    // Execute the query:
    if (radsqlitedirectQuery(historyDB, query, TRUE) == ERROR)
    {
        return ERROR;
    }

    rowDescr = radsqlitedirectGetRow(historyDB);
    if (rowDescr == NULL)
    {
        radsqlitedirectReleaseResults(historyDB);
        return ERROR;
    }

    // finally copy it to the internal history:
    store->startTime = date;

    for (index = 0; index < DATA_INDEX_MAX; index ++)
    {
        field = radsqlitedirectFieldGet(rowDescr, historyName[index]);
        if ((field == NULL) || ((radsqliteFieldGetType(field) & SQLITE_FIELD_VALUE_IS_NULL) != 0))
        {
            store->values[index] = ARCHIVE_VALUE_NULL;
        }
        else
        {
            store->samples[index] = 1;
            store->values[index] = (float)radsqliteFieldGetDoubleValue(field);
        }
    }

    radsqlitedirectReleaseResults(historyDB);
    return OK;
}

static int insertDBHistoryData(SQLITE_DATABASE_ID historyDB, HISTORY_DATA* data)
{
    SQLITE_ROW_ID           row;
    SQLITE_FIELD_ID         field;
    Data_Indices            index;
    HISTORY_DATA            store;

    // First see if the record exists:
    if (getHistoryRecord(historyDB, (time_t)data->startTime, &store) == OK)
    {
        // Found the guy, delete the old row:
        row = radsqliteTableDescriptionGet(historyDB, WVIEW_DAY_HISTORY_TABLE);
        if (row == NULL)
        {
            printf ("dbsqlite: databaseTableDescriptionGet (history) failed!");
            return ERROR;
        }
    
        field = radsqliteFieldGet(row, "date");
        if (field == NULL)
        {
            radMsgLog (PRI_MEDIUM, "dbsqlite: radsqliteFieldGet failed!");
            radsqliteRowDescriptionDelete(row);
            return ERROR;
        }
        else
        {
            radsqliteFieldSetBigIntValue(field, (uint64_t)data->startTime);
        }

        // Delete him:
        if (radsqliteTableDeleteRows(historyDB, WVIEW_DAY_HISTORY_TABLE, row)
            == ERROR)
        {
            radMsgLog (PRI_MEDIUM, "dbsqlite: radsqliteTableDeleteRows failed!");
            radsqliteRowDescriptionDelete(row);
            return ERROR;
        }

        radsqliteRowDescriptionDelete(row);
    }

    // Create the new record:
    row = radsqliteTableDescriptionGet(historyDB, WVIEW_DAY_HISTORY_TABLE);
    if (row == NULL)
    {
        printf ("dbsqlite: databaseTableDescriptionGet (history) failed!");
        return ERROR;
    }

    field = radsqliteFieldGet(row, "date");
    if (field == NULL)
    {
        radMsgLog (PRI_MEDIUM, "dbsqlite: radsqliteFieldGet failed!");
        radsqliteRowDescriptionDelete(row);
        return ERROR;
    }
    else
    {
        radsqliteFieldSetBigIntValue(field, (uint64_t)data->startTime);
    }

    for (index = 0; index < DATA_INDEX_MAX; index ++)
    {
        field = radsqliteFieldGet(row, historyName[index]);
        if (field == NULL)
        {
            radMsgLog (PRI_MEDIUM, "dbsqlite: radsqliteFieldGet %s failed!", historyName[index]);
            radsqliteRowDescriptionDelete(row);
            return ERROR;
        }
        else
        {
            if (data->values[index] <= ARCHIVE_VALUE_NULL || data->samples[index] == 0)
            {
                radsqliteFieldSetToNull(field);
            }
            else
            {
                if (index == DATA_INDEX_rain || index == DATA_INDEX_ET || index == DATA_INDEX_windDir)
                {
                    radsqliteFieldSetDoubleValue(field, (double)data->values[index]);
                }
                else
                {
                    radsqliteFieldSetDoubleValue(field, (double)data->values[index]/(double)data->samples[index]);
                }
            }
        }
    }

    // insert the row:
    if (radsqliteTableInsertRow(historyDB, WVIEW_DAY_HISTORY_TABLE, row) == ERROR)
    {
        radMsgLog (PRI_HIGH, "dbsqlite: radsqliteTableInsertRow (history) failed!");
        radsqliteRowDescriptionDelete(row);
        return ERROR;
    }

    radsqliteRowDescriptionDelete(row);
    return OK;
}


//  #####################  API Functions #####################

void dbsqliteHistoryInit (void)
{
    SQLITE_ROW_ID       rowDesc;
    Data_Indices        index;
    int                 retVal;
    SQLITE_DATABASE_ID  historyDB = NULL;

    historyDB = radsqliteOpen(getHistoryDBFilename());
    if (historyDB == NULL)
    {
        radMsgLog (PRI_HIGH, "dbsqliteHistoryInit: failed to open %s!", getHistoryDBFilename());
        return;
    }

    // Does the day history table exist?
    if (radsqliteTableIfExists(historyDB, WVIEW_DAY_HISTORY_TABLE))
    {
        radsqliteClose(historyDB);
        return;
    }

    // We need to create the table:
    // Define the row first:
    rowDesc = radsqliteRowDescriptionCreate();
    if (rowDesc == NULL)
    {
        radsqliteClose(historyDB);
        radMsgLog(PRI_HIGH, "dbsqliteHistoryInit: radsqliteRowDescriptionCreate failed!");
        return;
    }

    // Populate the table:
    retVal = radsqliteRowDescriptionAddField(rowDesc, 
                                             "date", 
                                             SQLITE_FIELD_BIGINT | SQLITE_FIELD_PRI_KEY, 
                                             0);
    if (retVal == ERROR)
    {
        radsqliteClose(historyDB);
        radMsgLog(PRI_HIGH, "dbsqliteHistoryInit: databaseRowDescriptionAddField failed!");
        radsqliteRowDescriptionDelete(rowDesc);
        return;
    }

    for (index = 0; index < DATA_INDEX_MAX; index ++)
    {
        retVal = radsqliteRowDescriptionAddField(rowDesc, 
                                                 historyName[index], 
                                                 SQLITE_FIELD_DOUBLE, 
                                                 0);
        if (retVal == ERROR)
        {
            radsqliteClose(historyDB);
            radMsgLog(PRI_HIGH, "dbsqliteHistoryInit: databaseRowDescriptionAddField failed!");
            radsqliteRowDescriptionDelete(rowDesc);
            return;
        }
    }

    // Now create the table:
    if (radsqliteTableCreate(historyDB, WVIEW_DAY_HISTORY_TABLE, rowDesc) == ERROR)
    {
        radsqliteClose(historyDB);
        radMsgLog(PRI_HIGH, "dbsqliteHistoryInit: radsqliteTableCreate failed!");
        radsqliteRowDescriptionDelete(rowDesc);
        return;
    }

    // We're done:
    radsqliteRowDescriptionDelete(rowDesc);

    radsqliteClose(historyDB);
    return;
}

// PRAGMA statement to modify the operation of the SQLite library
int dbsqliteHistoryPragmaSet(char *pragma, char *setting)
{
    char                query[DB_SQLITE_QUERY_LENGTH_MAX];
    SQLITE_DATABASE_ID  historyDB = NULL;

    historyDB = radsqliteOpen(getHistoryDBFilename());
    if (historyDB == NULL)
    {
        radMsgLog (PRI_HIGH, "dbsqliteHistoryPragmaSet: failed to open %s!", getHistoryDBFilename());
        return ERROR;
    }

    // Check SQLite version if a journalling pragma:
    if (!strcmp(pragma, "journal_mode"))
    {
        if (SQLITE_VERSION_NUMBER < 3005009)
        {
            // Not supported:
            radsqliteClose(historyDB);
            return OK;
        }
    }

    sprintf (query, "PRAGMA %s = %s", pragma, setting);

    // Execute the query:
    if (radsqliteQuery(historyDB, query, FALSE) == ERROR)
    {
        return ERROR;
    }

    radsqliteClose(historyDB);
    return OK;
}

int dbsqliteHistoryInsertDay (HISTORY_DATA* data)
{
    SQLITE_DATABASE_ID  historyDB = NULL;

    historyDB = radsqliteOpen(getHistoryDBFilename());
    if (historyDB == NULL)
    {
        radMsgLog (PRI_HIGH, "dbsqliteHistoryInsertDay: failed to open %s!", getHistoryDBFilename());
        return ERROR;
    }

    // Now do some inserting:
    if (insertDBHistoryData(historyDB, data) == ERROR)
    {
        radsqliteClose(historyDB);
        radMsgLog(PRI_HIGH, "dbsqliteHistoryInsertDay: insertDBHistoryData failed!");
        return ERROR;
    }

    radsqliteClose(historyDB);
    return OK;
}

int dbsqliteHistoryGetDay (time_t date, HISTORY_DATA* store)
{
    SQLITE_DATABASE_ID  historyDB = NULL;

    historyDB = radsqliteOpen(getHistoryDBFilename());
    if (historyDB == NULL)
    {
        radMsgLog (PRI_HIGH, "dbsqliteHistoryInsertDay: failed to open %s!", getHistoryDBFilename());
        return ERROR;
    }

    // First make sure the day history table exists:
    if (! radsqliteTableIfExists(historyDB, WVIEW_DAY_HISTORY_TABLE))
    {
        radsqliteClose(historyDB);
        return ERROR;
    }

    // Try to get the day requested:
    if (getHistoryRecord(historyDB, date, store) == ERROR)
    {
        radsqliteClose(historyDB);
        return ERROR;
    }

    radsqliteClose(historyDB);
    return OK;
}

#endif

