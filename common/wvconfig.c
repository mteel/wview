/*---------------------------------------------------------------------------
 
  FILENAME:
        wvconfig.c
 
  PURPOSE:
        Provide the configuration API methods.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        7/05/2008       M.S. Teel       0               Original
 
  NOTES:
        
 
  LICENSE:
        Copyright (c) 2008, Mark S. Teel (mark@teel.ws)
  
        This source code is released for free distribution under the terms 
        of the GNU General Public License.
  
----------------------------------------------------------------------------*/

//  ... System header files
#include <errno.h>
#include <radmsgLog.h>


//  ... Local header files
#include <wvconfig.h>


//  ... Local memory:
static SQLITE_DATABASE_ID   sqliteID;

//  ... Define a semaphore for access control:
static SEM_ID               wvconfigMutex;


//  ... Local methods

// Query for a parameter value - it is converted to the proper format later:
static int queryParmValue (const char* configItem, char* valueStore)
{
    char                    query[DB_SQLITE_QUERY_LENGTH_MAX];
    SQLITE_RESULT_SET_ID    result;
    SQLITE_ROW_ID           rowDescr;
    SQLITE_FIELD_ID         field;

    if (sqliteID == NULL)
    {
        radMsgLog (PRI_HIGH, "queryParmValue: sqliteID is NULL!");
        return ERROR;
    }

    sprintf (query, "SELECT value FROM config WHERE name = '%s'", configItem);

    if (radsqliteQuery(sqliteID, query, TRUE) == ERROR)
    {
        radMsgLog (PRI_MEDIUM, 
                   "queryParmValue: radsqliteQuery %s failed!", configItem);
        return ERROR;
    }

    result = radsqliteGetResults (sqliteID);
    if (result == NULL)
    {
        radMsgLog (PRI_MEDIUM, 
                   "queryParmValue: radsqliteGetResults failed!");
        return ERROR;
    }

    // We have a result, return it to the caller:
    rowDescr = radsqliteResultsGetFirst (result);
    if (rowDescr == NULL)
    {
        radMsgLog (PRI_MEDIUM, 
                   "queryParmValue: radsqliteResultsGetFirst failed!");
        radsqliteReleaseResults (sqliteID, result);
        return ERROR;
    }

    field = radsqliteFieldGet (rowDescr, configCOLUMN_VALUE);
    if (field == NULL)
    {
        radMsgLog (PRI_MEDIUM, 
                   "queryParmValue: radsqliteFieldGet failed!");
        radsqliteReleaseResults (sqliteID, result);
        return ERROR;
    }

    memcpy (valueStore, 
            radsqliteFieldGetCharValue(field), 
            radsqliteFieldGetCharLength(field));
    valueStore[radsqliteFieldGetCharLength(field)] = 0;

    // Clean up:
    radsqliteReleaseResults (sqliteID, result);

    return OK;
}


//  ... API (public) methods

//  wvconfigInit: Open the configuration database for this process:
int wvconfigInit (int firstProcess)
{
    char            buffer[_MAX_PATH];
    struct stat     fileData;

    // Make sure our config db is there:
    sprintf (buffer, "%s/%s", WVIEW_CONFIG_DIR, WVIEW_CONFIG_DATABASE);
    if (stat (buffer, &fileData) != 0)
    {
        radMsgLog (PRI_CATASTROPHIC,
                   "Cannot locate config database %s - aborting!", 
                   buffer);
        return ERROR;
    }

    if (firstProcess == TRUE)
    {
        wvconfigMutex = radSemCreate(WVIEW_CONFIG_SEM_INDEX, 1);
    }
    else
    {
        wvconfigMutex = radSemCreate(WVIEW_CONFIG_SEM_INDEX, -1);
    }

    if (wvconfigMutex == NULL)
    {
        radMsgLog (PRI_CATASTROPHIC, 
                   "Cannot create/attach config database semaphore - aborting!");
        return ERROR;
    }

    // Lock for serial access:
    radSemTake(wvconfigMutex);

    sqliteID = radsqliteOpen ((const char*)buffer);
    if (sqliteID == NULL)
    {
        radMsgLog (PRI_CATASTROPHIC, "wvconfigInit: radsqliteOpen %s failed!",
                   buffer);
        radSemGive(wvconfigMutex);
        radSemDelete(wvconfigMutex);
        return ERROR;
    }

    return OK;
}

//  wvconfigExit: clean up and detach from the wview configuration API
void wvconfigExit (void)
{
    if (sqliteID != NULL)
    {
        radsqliteClose (sqliteID);
        sqliteID = NULL;
    }

    radSemGive(wvconfigMutex);
    radSemDelete(wvconfigMutex);
}

//  wvconfigGetINTValue: retrieve the integer value for this parameter;
//  Returns: integer value or 0
int wvconfigGetINTValue (const char* configItem)
{
    char        buffer[_MAX_PATH];

    if (queryParmValue (configItem, buffer) == ERROR)
    {
        return 0;
    }

    return (atoi(buffer));
}

//  wvconfigGetDOUBLEValue: retrieve the double value for this parameter;
//  Returns: double value
double wvconfigGetDOUBLEValue (const char* configItem)
{
    char        buffer[_MAX_PATH];

    if (queryParmValue (configItem, buffer) == ERROR)
    {
        return 0.0;
    }

    return ((double)atof(buffer));
}

//  wvconfigGetStringValue: retrieve the string value for this parameter 
//  Returns: const static string reference
const char* wvconfigGetStringValue (const char* configItem)
{
    static char     buffer[_MAX_PATH];

    if (queryParmValue (configItem, buffer) == ERROR)
    {
        return NULL;
    }

    return buffer;
}

//  wvconfigGetBooleanValue: retrieve the bool value for this parameter 
//  Returns: TRUE or FALSE
int wvconfigGetBooleanValue (const char* configItem)
{
    char        buffer[_MAX_PATH];

    if (queryParmValue (configItem, buffer) == ERROR)
    {
        return ERROR;
    }

    if (!strcmp(buffer, "yes"))
    {
        return TRUE;
    }
    else if (!strcmp(buffer, "1"))
    {
        return TRUE;
    }
    else if (!strcmp(buffer, "TRUE"))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

