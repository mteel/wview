/*---------------------------------------------------------------------------

  FILENAME:
        arcrecGenerate.c

  PURPOSE:
        Provide the wview archive record browser generator utilities.

  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        07/05/2005      M.S. Teel       0               Original
        04/12/2008      W. Krenn        1               metric adaptions

  NOTES:


  LICENSE:
        Copyright (c) 2005, Mark S. Teel (mark@teel.ws)

        This source code is released for free distribution under the terms
        of the GNU General Public License.

----------------------------------------------------------------------------*/

/*  ... System include files
*/

/*  ... Library include files
*/

/*  ... Local include files
*/
#include <arcrecGenerate.h>


/*  ... global memory declarations
*/

/*  ... global memory referenced
*/

/*  ... static (local) memory declarations
*/
static ARCREC_WORK      arWork;


/*  ... local functions
*/
static time_t decrementTime (time_t timeval, int days)
{
    time_t      retval = timeval;

    retval -= (days * WV_SECONDS_IN_DAY);

    return retval;
}

static time_t setTo2000 (void)
{
    time_t      retVal;
    struct tm   loctime;

    memset (&loctime, 0, sizeof (loctime));
    loctime.tm_year  = 100;
    loctime.tm_mday  = 1;

    retVal = mktime (&loctime);
    return retVal;
}


static char arcrecFileName[_MAX_PATH];
static char *buildArcFilename (time_t timeval)
{
    sprintf (arcrecFileName, "%s/Archive/ARC-%4.4d-%2.2d-%2.2d.txt",
             arWork.htmlPath,
             wvutilsGetYear(timeval),
             wvutilsGetMonth(timeval),
             wvutilsGetDay(timeval));

    return arcrecFileName;
}

static char *buildArcFilenameFromRecord (ARCHIVE_PKT* record)
{
    sprintf (arcrecFileName, "%s/Archive/ARC-%4.4d-%2.2d-%2.2d.txt",
             arWork.htmlPath,
             wvutilsGetYear(record->dateTime),
             wvutilsGetMonth(record->dateTime),
             wvutilsGetDay(record->dateTime));

    return arcrecFileName;
}

static char *buildArcStartTime (time_t timeval)
{
    static char     arcrecStart[64];

    sprintf (arcrecStart, "%4.4d%2.2d%2.2d",
             wvutilsGetYear(timeval),
             wvutilsGetMonth(timeval),
             wvutilsGetDay(timeval));

    return arcrecStart;
}

static int arFileExists (char *store, time_t val)
{
    char            *arFilename = buildArcFilename (val);
    struct stat     fileStatus;

    wvstrncpy (store, arFilename, _MAX_PATH);
    if (stat(arFilename, &fileStatus) == 0)
    {
        //  does exist
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static int arSameDay (time_t temptime, time_t timenow)
{
    struct tm       temploc, nowloc;

    localtime_r (&temptime, &temploc);
    localtime_r (&timenow, &nowloc);

    if (temploc.tm_year != nowloc.tm_year ||
        temploc.tm_mon != nowloc.tm_mon ||
        temploc.tm_mday != nowloc.tm_mday)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

static void arcrecWriteHeader (FILE *file)
{
    int     retVal;

    // write out a human readable header for the data
    retVal = fwrite(arWork.header, 1, strlen(arWork.header), file);

    return;
}


int arcrecGenerateInit
(
    char        *htmlPath,
    int         daysToKeep,
    int         isMetric,
    int         arcInterval
)
{
    time_t      timenow = time (NULL);
    time_t      startTime, temptime;
    char        *filename, *fgetsRetVal, temp[_MAX_PATH];
    struct stat fileData;
    int         count, length = 0, daysWritten = 0;
    FILE        *hdrFile;

    memset (&arWork, 0, sizeof (arWork));

    wvstrncpy (arWork.htmlPath, htmlPath, _MAX_PATH);
    arWork.daysToKeep   = daysToKeep;

    if (daysToKeep == -1)
    {
        // we are disabled...
        return OK;
    }

    sprintf (temp, "%s/%s", WVIEW_CONFIG_DIR, ARCREC_HEADER_FILENAME);
    if (stat (temp, &fileData) != 0)
    {
        radMsgLog (PRI_STATUS, "ARCREC: cannot locate header file %s - using English default...",
                   temp);

        // write out the English default to our buffer
        memset (arWork.header, 0, ARCREC_HEADER_MAX_LENGTH);
        count = sprintf (arWork.header, "--Timestamp---\tTemp\tChill\tHIndex\tHumid\tDewpt\tWind\tHiWind\tWindDir\tRain\tBarom\tSolar\tET\tUV\n");
        sprintf (&arWork.header[count], "--------------\t----\t-----\t------\t-----\t-----\t----\t------\t-------\t----\t-----\t-----\t--\t--\n");
    }
    else
    {
        radMsgLog (PRI_STATUS, "ARCREC: using header file %s ...",
                   temp);

        // read the file into our header buffer
        hdrFile = fopen (temp, "r");
        if (hdrFile == NULL)
        {
            radMsgLog (PRI_STATUS, "ARCREC: failed to open header file %s - using English default...",
                       temp);
            memset (arWork.header, 0, ARCREC_HEADER_MAX_LENGTH);
            count = sprintf (arWork.header, "--Timestamp---\tTemp\tChill\tHIndex\tHumid\tDewpt\tWind\tHiWind\tWindDir\tRain\tBarom\tSolar\tET\tUV\n");
            sprintf (&arWork.header[count], "--------------\t----\t-----\t------\t-----\t-----\t----\t------\t-------\t----\t-----\t-----\t--\t--\n");
        }
        else
        {
            while ((length < ARCREC_HEADER_MAX_LENGTH - 1) && !feof (hdrFile))
            {
                fgetsRetVal = fgets(&arWork.header[length], ARCREC_HEADER_MAX_LENGTH - (length+1), hdrFile);
                length = strlen (arWork.header);
            }

            fclose (hdrFile);
        }
    }

    // do we need to purge any files?
    if (daysToKeep > 0)
    {
        // perhaps, get our starting point
        startTime = temptime = decrementTime (timenow, daysToKeep);
        startTime += WV_SECONDS_IN_DAY;

        // walk backwards in time until the file does not exist, deleting
        // the old files
        while (arFileExists (temp, temptime))
        {
            // delete that bad boy!
            unlink (temp);

            temptime -= WV_SECONDS_IN_DAY;
        }
    }
    else
    {
        startTime = setTo2000();
    }

    // now, let's generate files for the last "daysToKeep" days (if needed),
    // including today - if daysToKeep is 0, we must generate all of them
    if (daysToKeep == 0)
    {
        radMsgLog (PRI_STATUS, "ARCREC: saving ALL daily archive reports ...");
    }
    else
    {
        radMsgLog (PRI_STATUS, "ARCREC: saving %d daily archive reports ...",
                   daysToKeep);
    }

    // we include the current time in the loop to do the partial for today
    for (temptime = startTime; temptime <= timenow; temptime += WV_SECONDS_IN_DAY)
    {
        if (arFileExists(temp, temptime) && !arSameDay(temptime, timenow))
        {
            daysWritten ++;
            continue;
        }

        filename = temp;

        // call the dbf utility to write out a day's records
        if (dbsqliteWriteDailyArchiveReport (filename,
                                             temptime,
                                             isMetric,
                                             arcInterval,
                                             arcrecWriteHeader)
            == OK)
        {
            daysWritten ++;
        }
    }

    radMsgLog (PRI_STATUS, "ARCREC: %d daily archive reports available",
               daysWritten);

    return OK;
}

int arcrecGenerate (ARCHIVE_PKT* record, int isMetric)
{
    char        *temp;
    int         retVal;
    time_t      timenow;
    char        tempname[_MAX_PATH];

    if (arWork.daysToKeep == -1)
    {
        // we are disabled
        return OK;
    }

    // build the filename
    temp = buildArcFilenameFromRecord (record);

    retVal = dbsqliteUpdateDailyArchiveReport(temp, record, arcrecWriteHeader, isMetric);

    // we need to do some purging?
    if (retVal == 1 && arWork.daysToKeep > 0)
    {
        // yes!
        timenow = time (NULL);
        timenow -= (arWork.daysToKeep * WV_SECONDS_IN_DAY);
        if (arFileExists (tempname, timenow))
        {
            // delete that bad boy!
            unlink (tempname);
        }

        return OK;
    }
    else
    {
        return retVal;
    }
}
