/*---------------------------------------------------------------------------
 
  FILENAME:
        dbdatabase.c
 
  PURPOSE:
        Provide the SQL database archiving API methods.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        4/18/2005       M.S. Teel       0               Original
 
  NOTES:
        
 
  LICENSE:
        Copyright (c) 2004, Mark S. Teel (mark@teel.ws)
  
        This source code is released for free distribution under the terms 
        of the GNU General Public License.
  
----------------------------------------------------------------------------*/

//  ... System header files
#include <radmsgLog.h>


//  ... Local header files
#include <dbdatabase.h>


//  ... local memory

static DBDATABASE_WORK          dbdbWork;
static int                      dbTableExists;
static char                     dupQuery[256];
static int                      archiveExtendedIndex = 16;  // where extended begins
static DB_ARCHIVE_DESCRIPTION   archiveDescription[] =
    {
        {"RecordTime",   FIELD_DATETIME | FIELD_PRI_KEY | FIELD_UNIQUE_INDEX | FIELD_NOT_NULL, 16},
        {"ArcInt",       FIELD_INT,                                           0},
        {"OutTemp",      FIELD_FLOAT,                                         0},
        {"InTemp",       FIELD_FLOAT,                                         0},
        {"Barometer",    FIELD_FLOAT,                                         0},
        {"OutHumid",     FIELD_FLOAT,                                         0},
        {"InHumid",      FIELD_FLOAT,                                         0},
        {"Rain",         FIELD_FLOAT,                                         0},
        {"HiRainRate",   FIELD_FLOAT,                                         0},
        {"WindSpeed",    FIELD_FLOAT,                                         0},
        {"HiWindSpeed",  FIELD_FLOAT,                                         0},
        {"WindDir",      FIELD_INT,                                           0},
        {"HiWindDir",    FIELD_INT,                                           0},
        {"Dewpoint",     FIELD_FLOAT,                                         0},
        {"WindChill",    FIELD_FLOAT,                                         0},
        {"HeatIndex",    FIELD_FLOAT,                                         0},
        {"solarRad",     FIELD_INT,                                           0},
        {"UV",           FIELD_FLOAT,                                         0},
        {"ET",           FIELD_FLOAT,                                         0},
        {"leafTemp1",    FIELD_INT,                                           0},
        {"leafTemp2",    FIELD_INT,                                           0},
        {"leafWet1",     FIELD_INT,                                           0},
        {"leafWet2",     FIELD_INT,                                           0},
        {"soilTemp1",    FIELD_INT,                                           0},
        {"soilTemp2",    FIELD_INT,                                           0},
        {"soilTemp3",    FIELD_INT,                                           0},
        {"soilTemp4",    FIELD_INT,                                           0},
        {"soilMoist1",   FIELD_INT,                                           0},
        {"soilMoist2",   FIELD_INT,                                           0},
        {"soilMoist3",   FIELD_INT,                                           0},
        {"soilMoist4",   FIELD_INT,                                           0},
        {"extraHumid1",  FIELD_INT,                                           0},
        {"extraHumid2",  FIELD_INT,                                           0},
        {"extraTemp1",   FIELD_INT,                                           0},
        {"extraTemp2",   FIELD_INT,                                           0},
        {"extraTemp3",   FIELD_INT,                                           0},
        {NULL, 0, 0}
    };


//  ... define methods here

int dbdatabaseInit
(
    int             isEnabled,
    int             useExtended,
    char            *host,
    char            *username,
    char            *password,
    char            *databaseName,
    char            *tableName,
    int             isMetricUnits
)
{
    dbTableExists = FALSE;

    memset (&dbdbWork, 0, sizeof (dbdbWork));

    dbdbWork.isEnabled = isEnabled;
    dbdbWork.isExtended = useExtended;

    if (!dbdbWork.isEnabled)
        return OK;

    if (host == NULL)
        strcpy (dbdbWork.host, "NA");
    else
        wvstrncpy (dbdbWork.host, host, sizeof(dbdbWork.host));

    wvstrncpy (dbdbWork.username, username, sizeof(dbdbWork.username));
    wvstrncpy (dbdbWork.password, password, sizeof(dbdbWork.password));
    wvstrncpy (dbdbWork.databaseName, databaseName, sizeof(dbdbWork.databaseName));
    wvstrncpy (dbdbWork.tableName, tableName, sizeof(dbdbWork.tableName));
    dbdbWork.isMetricUnits = isMetricUnits;

    return OK;
}

int dbdatabaseOpen (void)
{
    char            *host;
    int             i, retVal, maxLoop;
    ROW_ID          row;

    if (!dbdbWork.isEnabled)
        return ERROR;

    if (dbdbWork.dbId != NULL)
    {
        // try to close it
        raddatabaseClose (dbdbWork.dbId);
        dbdbWork.dbId = NULL;
    }

    if (!strcmp (dbdbWork.host, "NA"))
        host = NULL;
    else
        host = dbdbWork.host;

    // does the database exist?
    if ((dbdbWork.dbId = raddatabaseOpen (host,
                                          dbdbWork.username,
                                          dbdbWork.password,
                                          dbdbWork.databaseName))
            == NULL)
    {
        radMsgLog (PRI_MEDIUM, "dbdatabaseOpen: %s databaseOpen failed!",
                   dbdbWork.databaseName);
        return ERROR;
    }

    // open the table (or create it)
    if (!dbTableExists)
    {
        if (!raddatabaseTableIfExists (dbdbWork.dbId, dbdbWork.tableName))
        {
            row = raddatabaseRowDescriptionCreate ();
            if (row == NULL)
            {
                radMsgLog (PRI_MEDIUM, "dbdatabaseOpen: databaseRowDescriptionCreate failed!");
                raddatabaseClose (dbdbWork.dbId);
                dbdbWork.dbId = NULL;
                return ERROR;
            }

            if (dbdbWork.isExtended)
                maxLoop = 255;
            else
                maxLoop = archiveExtendedIndex;

            // loop through adding all defined fields
            for (i = 0; i < maxLoop && archiveDescription[i].name != NULL; i ++)
            {
                retVal = raddatabaseRowDescriptionAddField (row,
                         archiveDescription[i].name,
                         archiveDescription[i].type,
                         archiveDescription[i].maxLength);
                if (retVal == ERROR)
                {
                    radMsgLog (PRI_MEDIUM, "dbdatabaseOpen: "
                               "RowDescriptionAddField failed!");
                    raddatabaseRowDescriptionDelete (row);
                    raddatabaseClose (dbdbWork.dbId);
                    dbdbWork.dbId = NULL;
                    return ERROR;
                }
            }

            //  ... now try to create the table
            retVal = raddatabaseTableCreate (dbdbWork.dbId, dbdbWork.tableName, row);
            if (retVal == ERROR)
            {
                radMsgLog (PRI_MEDIUM, "dbdatabaseOpen: "
                           "raddatabaseTableCreate failed!");
                raddatabaseRowDescriptionDelete (row);
                raddatabaseClose (dbdbWork.dbId);
                dbdbWork.dbId = NULL;
                return ERROR;
            }

            raddatabaseRowDescriptionDelete (row);
            dbTableExists = TRUE;
        }
        else
        {
            dbTableExists = TRUE;
        }
    }

    return OK;
}

void dbdatabaseClose (void)
{
    raddatabaseClose (dbdbWork.dbId);
    dbdbWork.dbId = NULL;
    return;
}


int dbdatabaseInsertRecord (ARCHIVE_PKT* newRecord)
{
    FIELD_ID        field;
    char            *host, dtime[64];
    RESULT_SET_ID   resultId;
    ROW_ID          row;

    if (!dbdbWork.isEnabled)
        return DBDB_INSERT_ERROR;

    // build the proper date/time field format
    sprintf (dtime,
             "%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d",
             wvutilsGetYear(newRecord->dateTime),
             wvutilsGetMonth(newRecord->dateTime),
             wvutilsGetDay(newRecord->dateTime),
             wvutilsGetHour(newRecord->dateTime),
             wvutilsGetMin(newRecord->dateTime),
             0);

    // first check for a duplicate
    sprintf (dupQuery, "SELECT RecordTime FROM %s WHERE RecordTime='%s'",
             dbdbWork.tableName, dtime);
    if (raddatabaseQuery (dbdbWork.dbId, dupQuery, TRUE) == OK)
    {
        resultId = raddatabaseGetResults (dbdbWork.dbId);
        if (resultId != NULL)
        {
            row = raddatabaseResultsGetFirst (resultId);
            if (row != NULL)
            {
                // looks like a duplicate...
                raddatabaseReleaseResults (dbdbWork.dbId, resultId);
                return DBDB_INSERT_DUPLICATE;
            }

            raddatabaseReleaseResults (dbdbWork.dbId, resultId);
        }
    }

    // let's add a row here
    row = raddatabaseTableDescriptionGet (dbdbWork.dbId, dbdbWork.tableName);
    if (row == NULL)
    {
        radMsgLog (PRI_MEDIUM,
                   "dbdatabaseInsertRecord: databaseTableDescriptionGet failed!");
        return DBDB_INSERT_ERROR;
    }

    field = raddatabaseFieldGet (row, archiveDescription[0].name);
    if (field == NULL)
    {
        radMsgLog (PRI_CATASTROPHIC, "dbdatabaseInsertRecord: FieldGet DateTime failed!");
        raddatabaseRowDescriptionDelete (row);
        return DBDB_INSERT_ERROR;
    }
    raddatabaseFieldSetDateTimeValue (field, dtime, strlen (dtime));

    field = raddatabaseFieldGet (row, archiveDescription[1].name);
    if (field == NULL)
    {
        radMsgLog (PRI_CATASTROPHIC, "dbdatabaseInsertRecord: FieldGet 1 failed!");
        raddatabaseRowDescriptionDelete (row);
        return DBDB_INSERT_ERROR;
    }
    raddatabaseFieldSetIntValue (field, (int)newRecord->interval);

    field = raddatabaseFieldGet (row, archiveDescription[2].name);
    if (field == NULL)
    {
        radMsgLog (PRI_CATASTROPHIC, "dbdatabaseInsertRecord: FieldGet 2 failed!");
        raddatabaseRowDescriptionDelete (row);
        return DBDB_INSERT_ERROR;
    }
    if (dbdbWork.isMetricUnits)
        raddatabaseFieldSetFloatValue (field,
                                       wvutilsConvertFToC ((float)newRecord->value[DATA_INDEX_outTemp]));
    else
        raddatabaseFieldSetFloatValue (field, (float)newRecord->value[DATA_INDEX_outTemp]);

    field = raddatabaseFieldGet (row, archiveDescription[3].name);
    if (field == NULL)
    {
        radMsgLog (PRI_CATASTROPHIC, "dbdatabaseInsertRecord: FieldGet 3 failed!");
        raddatabaseRowDescriptionDelete (row);
        return DBDB_INSERT_ERROR;
    }
    if (dbdbWork.isMetricUnits)
        raddatabaseFieldSetFloatValue (field,
                                       wvutilsConvertFToC ((float)newRecord->value[DATA_INDEX_inTemp]));
    else
        raddatabaseFieldSetFloatValue (field, (float)newRecord->value[DATA_INDEX_inTemp]);

    field = raddatabaseFieldGet (row, archiveDescription[4].name);
    if (field == NULL)
    {
        radMsgLog (PRI_CATASTROPHIC, "dbdatabaseInsertRecord: FieldGet 4 failed!");
        raddatabaseRowDescriptionDelete (row);
        return DBDB_INSERT_ERROR;
    }
    if (dbdbWork.isMetricUnits)
        raddatabaseFieldSetFloatValue (field,
                                       wvutilsConvertINHGToHPA ((float)newRecord->value[DATA_INDEX_barometer]));
    else
        raddatabaseFieldSetFloatValue (field, (float)newRecord->value[DATA_INDEX_barometer]);

    field = raddatabaseFieldGet (row, archiveDescription[5].name);
    if (field == NULL)
    {
        radMsgLog (PRI_CATASTROPHIC, "dbdatabaseInsertRecord: FieldGet 5 failed!");
        raddatabaseRowDescriptionDelete (row);
        return DBDB_INSERT_ERROR;
    }
    raddatabaseFieldSetFloatValue (field, (float)newRecord->value[DATA_INDEX_outHumidity]);

    field = raddatabaseFieldGet (row, archiveDescription[6].name);
    if (field == NULL)
    {
        radMsgLog (PRI_CATASTROPHIC, "dbdatabaseInsertRecord: FieldGet 6 failed!");
        raddatabaseRowDescriptionDelete (row);
        return DBDB_INSERT_ERROR;
    }
    raddatabaseFieldSetFloatValue (field, (float)newRecord->value[DATA_INDEX_inHumidity]);

    field = raddatabaseFieldGet (row, archiveDescription[7].name);
    if (field == NULL)
    {
        radMsgLog (PRI_CATASTROPHIC, "dbdatabaseInsertRecord: FieldGet 7 failed!");
        raddatabaseRowDescriptionDelete (row);
        return DBDB_INSERT_ERROR;
    }
    if (dbdbWork.isMetricUnits)
        raddatabaseFieldSetFloatValue (field,
                                       wvutilsConvertRainINToMetric (newRecord->value[DATA_INDEX_rain]));
    else
        raddatabaseFieldSetFloatValue (field, newRecord->value[DATA_INDEX_rain]);

    field = raddatabaseFieldGet (row, archiveDescription[8].name);
    if (field == NULL)
    {
        radMsgLog (PRI_CATASTROPHIC, "dbdatabaseInsertRecord: FieldGet 8 failed!");
        raddatabaseRowDescriptionDelete (row);
        return DBDB_INSERT_ERROR;
    }
    if (dbdbWork.isMetricUnits)
        raddatabaseFieldSetFloatValue (field,
                                       wvutilsConvertRainINToMetric (newRecord->value[DATA_INDEX_rainRate]));
    else
        raddatabaseFieldSetFloatValue (field, newRecord->value[DATA_INDEX_rainRate]);

    field = raddatabaseFieldGet (row, archiveDescription[9].name);
    if (field == NULL)
    {
        radMsgLog (PRI_CATASTROPHIC, "dbdatabaseInsertRecord: FieldGet 9 failed!");
        raddatabaseRowDescriptionDelete (row);
        return DBDB_INSERT_ERROR;
    }
    if (dbdbWork.isMetricUnits)
        raddatabaseFieldSetFloatValue (field,
                                       wvutilsConvertMPHToKPH ((float)newRecord->value[DATA_INDEX_windSpeed]));
    else
        raddatabaseFieldSetFloatValue (field, (float)newRecord->value[DATA_INDEX_windSpeed]);

    field = raddatabaseFieldGet (row, archiveDescription[10].name);
    if (field == NULL)
    {
        radMsgLog (PRI_CATASTROPHIC, "dbdatabaseInsertRecord: FieldGet 10 failed!");
        raddatabaseRowDescriptionDelete (row);
        return DBDB_INSERT_ERROR;
    }
    if (dbdbWork.isMetricUnits)
        raddatabaseFieldSetFloatValue (field,
                                       wvutilsConvertMPHToKPH ((float)newRecord->value[DATA_INDEX_windGust]));
    else
        raddatabaseFieldSetFloatValue (field, (float)newRecord->value[DATA_INDEX_windGust]);

    field = raddatabaseFieldGet (row, archiveDescription[11].name);
    if (field == NULL)
    {
        radMsgLog (PRI_CATASTROPHIC, "dbdatabaseInsertRecord: FieldGet 11 failed!");
        raddatabaseRowDescriptionDelete (row);
        return DBDB_INSERT_ERROR;
    }
    raddatabaseFieldSetIntValue (field, (int)newRecord->value[DATA_INDEX_windDir]);

    field = raddatabaseFieldGet (row, archiveDescription[12].name);
    if (field == NULL)
    {
        radMsgLog (PRI_CATASTROPHIC, "dbdatabaseInsertRecord: FieldGet 12 failed!");
        raddatabaseRowDescriptionDelete (row);
        return DBDB_INSERT_ERROR;
    }
    raddatabaseFieldSetIntValue (field, (int)newRecord->value[DATA_INDEX_windGustDir]);

    field = raddatabaseFieldGet (row, archiveDescription[13].name);
    if (field == NULL)
    {
        radMsgLog (PRI_CATASTROPHIC, "dbdatabaseInsertRecord: FieldGet 13 failed!");
        raddatabaseRowDescriptionDelete (row);
        return DBDB_INSERT_ERROR;
    }
    if (dbdbWork.isMetricUnits)
        raddatabaseFieldSetFloatValue (field,
                                       wvutilsConvertFToC (newRecord->value[DATA_INDEX_dewpoint]));
    else
        raddatabaseFieldSetFloatValue (field, newRecord->value[DATA_INDEX_dewpoint]);

    field = raddatabaseFieldGet (row, archiveDescription[14].name);
    if (field == NULL)
    {
        radMsgLog (PRI_CATASTROPHIC, "dbdatabaseInsertRecord: FieldGet 14 failed!");
        raddatabaseRowDescriptionDelete (row);
        return DBDB_INSERT_ERROR;
    }
    if (dbdbWork.isMetricUnits)
        raddatabaseFieldSetFloatValue (field,
                                       wvutilsConvertFToC (newRecord->value[DATA_INDEX_windchill]));
    else
        raddatabaseFieldSetFloatValue (field, newRecord->value[DATA_INDEX_windchill]);

    field = raddatabaseFieldGet (row, archiveDescription[15].name);
    if (field == NULL)
    {
        radMsgLog (PRI_CATASTROPHIC, "dbdatabaseInsertRecord: FieldGet 15 failed!");
        raddatabaseRowDescriptionDelete (row);
        return DBDB_INSERT_ERROR;
    }
    if (dbdbWork.isMetricUnits)
        raddatabaseFieldSetFloatValue (field,
                                       wvutilsConvertFToC (newRecord->value[DATA_INDEX_heatindex]));
    else
        raddatabaseFieldSetFloatValue (field, newRecord->value[DATA_INDEX_heatindex]);


    // Are we storing extended values too?
    if (dbdbWork.isExtended)
    {
        //  ... add the extended values
        field = raddatabaseFieldGet (row, archiveDescription[16].name);
        if (field == NULL)
        {
            radMsgLog (PRI_CATASTROPHIC, "dbdatabaseInsertRecord: FieldGet 16 failed!");
            raddatabaseRowDescriptionDelete (row);
            return DBDB_INSERT_ERROR;
        }
        raddatabaseFieldSetIntValue (field, (int)newRecord->value[DATA_INDEX_radiation]);

        field = raddatabaseFieldGet (row, archiveDescription[17].name);
        if (field == NULL)
        {
            radMsgLog (PRI_CATASTROPHIC, "dbdatabaseInsertRecord: FieldGet 17 failed!");
            raddatabaseRowDescriptionDelete (row);
            return DBDB_INSERT_ERROR;
        }
        if (newRecord->value[DATA_INDEX_UV] < 0)
        {
            //no value
            raddatabaseFieldSetFloatValue (field, -1.0);
        }
        else
        {
            raddatabaseFieldSetFloatValue (field, (float)newRecord->value[DATA_INDEX_UV]);
        }

        field = raddatabaseFieldGet (row, archiveDescription[18].name);
        if (field == NULL)
        {
            radMsgLog (PRI_CATASTROPHIC, "dbdatabaseInsertRecord: FieldGet 18 failed!");
            raddatabaseRowDescriptionDelete (row);
            return DBDB_INSERT_ERROR;
        }
        if (newRecord->value[DATA_INDEX_ET] < 0)
        {
            // no value
            raddatabaseFieldSetFloatValue (field, -1.0);
        }
        else
        {
            if (dbdbWork.isMetricUnits)
                raddatabaseFieldSetFloatValue (field,
                                               wvutilsConvertRainINToMetric ((float)newRecord->value[DATA_INDEX_ET]));
            else
                raddatabaseFieldSetFloatValue (field, (float)newRecord->value[DATA_INDEX_ET]);
        }

        field = raddatabaseFieldGet (row, archiveDescription[19].name);
        if (field == NULL)
        {
            radMsgLog (PRI_CATASTROPHIC, "dbdatabaseInsertRecord: FieldGet 19 failed!");
            raddatabaseRowDescriptionDelete (row);
            return DBDB_INSERT_ERROR;
        }
        if (newRecord->value[DATA_INDEX_leafTemp1] <= ARCHIVE_VALUE_NULL)
        {
            // no value
            raddatabaseFieldSetIntValue (field, -1);
        }
        else
        {
            if (dbdbWork.isMetricUnits)
                raddatabaseFieldSetIntValue (field,
                                             (int)wvutilsConvertFToC ((float)(newRecord->value[DATA_INDEX_leafTemp1])));
            else
                raddatabaseFieldSetIntValue (field, (int)newRecord->value[DATA_INDEX_leafTemp1]);
        }

        field = raddatabaseFieldGet (row, archiveDescription[20].name);
        if (field == NULL)
        {
            radMsgLog (PRI_CATASTROPHIC, "dbdatabaseInsertRecord: FieldGet 20 failed!");
            raddatabaseRowDescriptionDelete (row);
            return DBDB_INSERT_ERROR;
        }
        if (newRecord->value[DATA_INDEX_leafTemp2] <= ARCHIVE_VALUE_NULL)
        {
            // no value
            raddatabaseFieldSetIntValue (field, -1);
        }
        else
        {
            if (dbdbWork.isMetricUnits)
                raddatabaseFieldSetIntValue (field,
                                             (int)wvutilsConvertFToC ((float)(newRecord->value[DATA_INDEX_leafTemp2])));
            else
                raddatabaseFieldSetIntValue (field, (int)newRecord->value[DATA_INDEX_leafTemp2]);
        }

        field = raddatabaseFieldGet (row, archiveDescription[21].name);
        if (field == NULL)
        {
            radMsgLog (PRI_CATASTROPHIC, "dbdatabaseInsertRecord: FieldGet 21 failed!");
            raddatabaseRowDescriptionDelete (row);
            return DBDB_INSERT_ERROR;
        }
        if (newRecord->value[DATA_INDEX_leafWet1] <= ARCHIVE_VALUE_NULL)
        {
            // no value
            raddatabaseFieldSetIntValue (field, -1);
        }
        else
        {
            raddatabaseFieldSetIntValue (field, (int)newRecord->value[DATA_INDEX_leafWet1]);
        }

        field = raddatabaseFieldGet (row, archiveDescription[22].name);
        if (field == NULL)
        {
            radMsgLog (PRI_CATASTROPHIC, "dbdatabaseInsertRecord: FieldGet 22 failed!");
            raddatabaseRowDescriptionDelete (row);
            return DBDB_INSERT_ERROR;
        }
        if (newRecord->value[DATA_INDEX_leafWet2] <= ARCHIVE_VALUE_NULL)
        {
            // no value
            raddatabaseFieldSetIntValue (field, -1);
        }
        else
        {
            raddatabaseFieldSetIntValue (field, (int)newRecord->value[DATA_INDEX_leafWet2]);
        }

        field = raddatabaseFieldGet (row, archiveDescription[23].name);
        if (field == NULL)
        {
            radMsgLog (PRI_CATASTROPHIC, "dbdatabaseInsertRecord: FieldGet 23 failed!");
            raddatabaseRowDescriptionDelete (row);
            return DBDB_INSERT_ERROR;
        }
        if (newRecord->value[DATA_INDEX_soilTemp1] <= ARCHIVE_VALUE_NULL)
        {
            // no value
            raddatabaseFieldSetIntValue (field, -1);
        }
        else
        {
            if (dbdbWork.isMetricUnits)
                raddatabaseFieldSetIntValue (field,
                                             (int)wvutilsConvertFToC ((float)(newRecord->value[DATA_INDEX_soilTemp1])));
            else
                raddatabaseFieldSetIntValue (field, (int)newRecord->value[DATA_INDEX_soilTemp1]);
        }

        field = raddatabaseFieldGet (row, archiveDescription[24].name);
        if (field == NULL)
        {
            radMsgLog (PRI_CATASTROPHIC, "dbdatabaseInsertRecord: FieldGet 24 failed!");
            raddatabaseRowDescriptionDelete (row);
            return DBDB_INSERT_ERROR;
        }
        if (newRecord->value[DATA_INDEX_soilTemp2] <= ARCHIVE_VALUE_NULL)
        {
            // no value
            raddatabaseFieldSetIntValue (field, -1);
        }
        else
        {
            if (dbdbWork.isMetricUnits)
                raddatabaseFieldSetIntValue (field,
                                             (int)wvutilsConvertFToC ((float)(newRecord->value[DATA_INDEX_soilTemp2])));
            else
                raddatabaseFieldSetIntValue (field, (int)newRecord->value[DATA_INDEX_soilTemp2]);
        }

        field = raddatabaseFieldGet (row, archiveDescription[25].name);
        if (field == NULL)
        {
            radMsgLog (PRI_CATASTROPHIC, "dbdatabaseInsertRecord: FieldGet 25 failed!");
            raddatabaseRowDescriptionDelete (row);
            return DBDB_INSERT_ERROR;
        }
        if (newRecord->value[DATA_INDEX_soilTemp3] <= ARCHIVE_VALUE_NULL)
        {
            // no value
            raddatabaseFieldSetIntValue (field, -1);
        }
        else
        {
            if (dbdbWork.isMetricUnits)
                raddatabaseFieldSetIntValue (field,
                                             (int)wvutilsConvertFToC ((float)(newRecord->value[DATA_INDEX_soilTemp3])));
            else
                raddatabaseFieldSetIntValue (field, (int)newRecord->value[DATA_INDEX_soilTemp3]);
        }

        field = raddatabaseFieldGet (row, archiveDescription[26].name);
        if (field == NULL)
        {
            radMsgLog (PRI_CATASTROPHIC, "dbdatabaseInsertRecord: FieldGet 26 failed!");
            raddatabaseRowDescriptionDelete (row);
            return DBDB_INSERT_ERROR;
        }
        if (newRecord->value[DATA_INDEX_soilTemp4] <= ARCHIVE_VALUE_NULL)
        {
            // no value
            raddatabaseFieldSetIntValue (field, -1);
        }
        else
        {
            if (dbdbWork.isMetricUnits)
                raddatabaseFieldSetIntValue (field,
                                             (int)wvutilsConvertFToC ((float)(newRecord->value[DATA_INDEX_soilTemp4])));
            else
                raddatabaseFieldSetIntValue (field, (int)newRecord->value[DATA_INDEX_soilTemp4]);
        }

        field = raddatabaseFieldGet (row, archiveDescription[27].name);
        if (field == NULL)
        {
            radMsgLog (PRI_CATASTROPHIC, "dbdatabaseInsertRecord: FieldGet 27 failed!");
            raddatabaseRowDescriptionDelete (row);
            return DBDB_INSERT_ERROR;
        }
        if (newRecord->value[DATA_INDEX_soilMoist1] <= ARCHIVE_VALUE_NULL)
        {
            // no value
            raddatabaseFieldSetIntValue (field, -1);
        }
        else
        {
            raddatabaseFieldSetIntValue (field, (int)newRecord->value[DATA_INDEX_soilMoist1]);
        }

        field = raddatabaseFieldGet (row, archiveDescription[28].name);
        if (field == NULL)
        {
            radMsgLog (PRI_CATASTROPHIC, "dbdatabaseInsertRecord: FieldGet 28 failed!");
            raddatabaseRowDescriptionDelete (row);
            return DBDB_INSERT_ERROR;
        }
        if (newRecord->value[DATA_INDEX_soilMoist2] <= ARCHIVE_VALUE_NULL)
        {
            // no value
            raddatabaseFieldSetIntValue (field, -1);
        }
        else
        {
            raddatabaseFieldSetIntValue (field, (int)newRecord->value[DATA_INDEX_soilMoist2]);
        }

        field = raddatabaseFieldGet (row, archiveDescription[29].name);
        if (field == NULL)
        {
            radMsgLog (PRI_CATASTROPHIC, "dbdatabaseInsertRecord: FieldGet 29 failed!");
            raddatabaseRowDescriptionDelete (row);
            return DBDB_INSERT_ERROR;
        }
        if (newRecord->value[DATA_INDEX_soilMoist3] <= ARCHIVE_VALUE_NULL)
        {
            // no value
            raddatabaseFieldSetIntValue (field, -1);
        }
        else
        {
            raddatabaseFieldSetIntValue (field, (int)newRecord->value[DATA_INDEX_soilMoist3]);
        }

        field = raddatabaseFieldGet (row, archiveDescription[30].name);
        if (field == NULL)
        {
            radMsgLog (PRI_CATASTROPHIC, "dbdatabaseInsertRecord: FieldGet 30 failed!");
            raddatabaseRowDescriptionDelete (row);
            return DBDB_INSERT_ERROR;
        }
        if (newRecord->value[DATA_INDEX_soilMoist4] <= ARCHIVE_VALUE_NULL)
        {
            // no value
            raddatabaseFieldSetIntValue (field, -1);
        }
        else
        {
            raddatabaseFieldSetIntValue (field, (int)newRecord->value[DATA_INDEX_soilMoist4]);
        }

        field = raddatabaseFieldGet (row, archiveDescription[31].name);
        if (field == NULL)
        {
            radMsgLog (PRI_CATASTROPHIC, "dbdatabaseInsertRecord: FieldGet 31 failed!");
            raddatabaseRowDescriptionDelete (row);
            return DBDB_INSERT_ERROR;
        }
        if (newRecord->value[DATA_INDEX_extraHumid1] <= ARCHIVE_VALUE_NULL)
        {
            // no value
            raddatabaseFieldSetIntValue (field, -1);
        }
        else
        {
            raddatabaseFieldSetIntValue (field, (int)newRecord->value[DATA_INDEX_extraHumid1]);
        }

        field = raddatabaseFieldGet (row, archiveDescription[32].name);
        if (field == NULL)
        {
            radMsgLog (PRI_CATASTROPHIC, "dbdatabaseInsertRecord: FieldGet 32 failed!");
            raddatabaseRowDescriptionDelete (row);
            return DBDB_INSERT_ERROR;
        }
        if (newRecord->value[DATA_INDEX_extraHumid2] <= ARCHIVE_VALUE_NULL)
        {
            // no value
            raddatabaseFieldSetIntValue (field, -1);
        }
        else
        {
            raddatabaseFieldSetIntValue (field, (int)newRecord->value[DATA_INDEX_extraHumid2]);
        }

        field = raddatabaseFieldGet (row, archiveDescription[33].name);
        if (field == NULL)
        {
            radMsgLog (PRI_CATASTROPHIC, "dbdatabaseInsertRecord: FieldGet 33 failed!");
            raddatabaseRowDescriptionDelete (row);
            return DBDB_INSERT_ERROR;
        }
        if (newRecord->value[DATA_INDEX_extraTemp1] <= ARCHIVE_VALUE_NULL)
        {
            // no value
            raddatabaseFieldSetIntValue (field, -1);
        }
        else
        {
            if (dbdbWork.isMetricUnits)
                raddatabaseFieldSetIntValue (field,
                                             (int)wvutilsConvertFToC ((float)(newRecord->value[DATA_INDEX_extraTemp1])));
            else
                raddatabaseFieldSetIntValue (field, (int)newRecord->value[DATA_INDEX_extraTemp1]);
        }

        field = raddatabaseFieldGet (row, archiveDescription[34].name);
        if (field == NULL)
        {
            radMsgLog (PRI_CATASTROPHIC, "dbdatabaseInsertRecord: FieldGet 34 failed!");
            raddatabaseRowDescriptionDelete (row);
            return DBDB_INSERT_ERROR;
        }
        if (newRecord->value[DATA_INDEX_extraTemp2] <= ARCHIVE_VALUE_NULL)
        {
            // no value
            raddatabaseFieldSetIntValue (field, -1);
        }
        else
        {
            if (dbdbWork.isMetricUnits)
                raddatabaseFieldSetIntValue (field,
                                             (int)wvutilsConvertFToC ((float)(newRecord->value[DATA_INDEX_extraTemp2])));
            else
                raddatabaseFieldSetIntValue (field, (int)newRecord->value[DATA_INDEX_extraTemp2]);
        }

        field = raddatabaseFieldGet (row, archiveDescription[35].name);
        if (field == NULL)
        {
            radMsgLog (PRI_CATASTROPHIC, "dbdatabaseInsertRecord: FieldGet 35 failed!");
            raddatabaseRowDescriptionDelete (row);
            return DBDB_INSERT_ERROR;
        }
        if (newRecord->value[DATA_INDEX_extraTemp3] <= ARCHIVE_VALUE_NULL)
        {
            // no value
            raddatabaseFieldSetIntValue (field, -1);
        }
        else
        {
            if (dbdbWork.isMetricUnits)
                raddatabaseFieldSetIntValue (field,
                                             (int)wvutilsConvertFToC ((float)(newRecord->value[DATA_INDEX_extraTemp3])));
            else
                raddatabaseFieldSetIntValue (field, (int)newRecord->value[DATA_INDEX_extraTemp3]);
        }
    }


    //  ... insert the row
    if (raddatabaseTableInsertRow (dbdbWork.dbId, dbdbWork.tableName, row) == ERROR)
    {
        radMsgLog (PRI_MEDIUM, "dbdatabaseInsertRecord: TableInsertRow failed!");
        raddatabaseRowDescriptionDelete (row);
        return DBDB_INSERT_ERROR;
    }

    raddatabaseRowDescriptionDelete (row);
    return DBDB_INSERT_OK;
}


int dbdatabaseGetDateOfLastEntry (time_t* retTime)
{
    char            lastQuery[1024];
    int             dbRecordYear,dbRecordMonth,dbRecordDay,dbRecordMinute,dbRecordHour;

    ROW_ID          row;
    RESULT_SET_ID   resultId;
    FIELD_ID        field;
    struct tm       bknTime;

    // TODO: it should be possible to do most of this function (shifting, ANDing, casting etc)
    //  in this SQL statement...
    sprintf( lastQuery, "SELECT (EXTRACT(YEAR FROM recordTime)) AS year, (EXTRACT(MONTH FROM recordTime)) AS month, (EXTRACT(DAY FROM recordTime)) AS day, (EXTRACT(HOUR FROM recordTime)) AS hour, (EXTRACT(MINUTE FROM recordTime)) AS minute FROM %s ORDER BY recordTime DESC LIMIT 1", dbdbWork.tableName);
    if (raddatabaseQuery (dbdbWork.dbId, lastQuery, TRUE) == OK)
    {
        resultId = raddatabaseGetResults (dbdbWork.dbId);
        if (resultId != NULL)
        {
            row = raddatabaseResultsGetFirst (resultId);
            if (row != NULL)
            {
                // mysql and pgresql return different data types from the query, so...
                field = raddatabaseFieldGet (row, "year");
                if ( raddatabaseFieldGetType(field) == FIELD_DOUBLE )
                {
                    dbRecordYear = (int) raddatabaseFieldGetDoubleValue (field);
                }
                else
                {
                    dbRecordYear = raddatabaseFieldGetBigIntValue (field);
                }

                field = raddatabaseFieldGet (row, "month");
                if ( raddatabaseFieldGetType(field) == FIELD_DOUBLE )
                {
                    dbRecordMonth = (int) raddatabaseFieldGetDoubleValue (field);
                }
                else
                {
                    dbRecordMonth = raddatabaseFieldGetBigIntValue (field);
                }

                field = raddatabaseFieldGet (row, "day");
                if ( raddatabaseFieldGetType(field) == FIELD_DOUBLE )
                {
                    dbRecordDay = (int) raddatabaseFieldGetDoubleValue (field);
                }
                else
                {
                    dbRecordDay = raddatabaseFieldGetBigIntValue (field);
                }

                field = raddatabaseFieldGet (row, "hour");
                if ( raddatabaseFieldGetType(field) == FIELD_DOUBLE )
                {
                    dbRecordHour = (int) raddatabaseFieldGetDoubleValue (field);
                }
                else
                {
                    dbRecordHour = raddatabaseFieldGetBigIntValue (field);
                }

                field = raddatabaseFieldGet (row, "minute");
                if ( raddatabaseFieldGetType(field) == FIELD_DOUBLE )
                {
                    dbRecordMinute = (int) raddatabaseFieldGetDoubleValue (field);
                }
                else
                {
                    dbRecordMinute = raddatabaseFieldGetBigIntValue (field);
                }

                bknTime.tm_year  = dbRecordYear - 1900;
                bknTime.tm_mon   = dbRecordMonth - 1;
                bknTime.tm_mday  = dbRecordDay;
                bknTime.tm_hour  = dbRecordHour;
                bknTime.tm_min   = dbRecordMinute;
                bknTime.tm_sec   = 0;
                bknTime.tm_isdst = -1;
                *retTime = mktime(&bknTime);
                raddatabaseReleaseResults (dbdbWork.dbId, resultId);
                return OK;
            }

            raddatabaseReleaseResults (dbdbWork.dbId, resultId);
            return ERROR;
        }
    }
    return ERROR;
}

