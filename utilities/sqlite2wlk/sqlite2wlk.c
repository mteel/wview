/*---------------------------------------------------------------------
 
 FILE NAME:
        sqlite2wlk.c
 
 PURPOSE:
        wview archive file convertor utility: SQLite3 database TO Davis WLK.
 
 REVISION HISTORY:
    Date        Programmer  Revision    Function
    09/13/2008  M.S. Teel   0           Original
 
 ASSUMPTIONS:
 None.
 
------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>

#include <sysdefs.h>
#include <datadefs.h>
#include <dbfiles.h>
#include "wvutilities.h"


static void USAGE (void)
{
    printf ("Usage: sqlite2wlk <source_directory> <dest_directory>\n\n");
    printf ("    Convert wview archive data in <source_directory>/wview-archive.sdb\n");
    printf ("    to Davis WLK files in <dest_directory>\n");
    printf ("Note: If Davis WLK files in <dest_directory>, they will be overwritten.\n");
    printf ("      If <source_directory> is the live wview archive, you will need root access.\n\n");
    return;
}

#define GETVALWITHOFFSET(x,y)       ((x <= ARCHIVE_VALUE_NULL) ? 0xff : (uint8_t)(x+y))
#define GETVALNOOFFSET(x)                   ((x <= ARCHIVE_VALUE_NULL) ? 0xff : (uint8_t)x)

static void convertArchivePktToWLK(ARCHIVE_PKT* archivePkt, ARCHIVE_RECORD* newRecord, time_t timeStamp)
{
    uint16_t        temp;
    time_t          tempTime;

    // create the silly packed time crap for the record:
    // First check for 24:00 madness:
    if (wvutilsGetHour(timeStamp) == 0 && wvutilsGetMin(timeStamp) == 0)
    {
        // Need to make it 24:00:
        tempTime = timeStamp - (3*WV_SECONDS_IN_HOUR);      // Avoid DST issues
        newRecord->date = INSERT_PACKED_DATE(wvutilsGetYear(tempTime),
                                             wvutilsGetMonth(tempTime),
                                             wvutilsGetDay(tempTime));
        newRecord->time = (100 * 24);
    }
    else
    {
        newRecord->date = INSERT_PACKED_DATE(wvutilsGetYear(timeStamp),
                                             wvutilsGetMonth(timeStamp),
                                             wvutilsGetDay(timeStamp));
        newRecord->time = (100 * wvutilsGetHour(timeStamp)) + wvutilsGetMin(timeStamp);
    }

    // Set the values we can:
    newRecord->outTemp          = (int16_t)(archivePkt->value[DATA_INDEX_outTemp]*10);
    newRecord->highOutTemp      = (int16_t)(archivePkt->value[DATA_INDEX_outTemp]*10);
    newRecord->lowOutTemp       = (int16_t)(archivePkt->value[DATA_INDEX_outTemp]*10);
    temp = (uint16_t)(archivePkt->value[DATA_INDEX_rain]*100);
    newRecord->rain             = temp;
    newRecord->highRainRate     = (uint16_t)(archivePkt->value[DATA_INDEX_rainRate]*100);
    newRecord->barometer        = (uint16_t)(archivePkt->value[DATA_INDEX_barometer]*1000);
    newRecord->radiation        = (uint16_t)(archivePkt->value[DATA_INDEX_radiation]);
    newRecord->windSamples      = 0xffff;
    newRecord->inTemp           = (int16_t)(archivePkt->value[DATA_INDEX_inTemp]*10);
    newRecord->inHumidity       = (uint8_t)archivePkt->value[DATA_INDEX_inHumidity];
    newRecord->outHumidity      = (uint8_t)archivePkt->value[DATA_INDEX_outHumidity];
    newRecord->avgWindSpeed     = (uint8_t)archivePkt->value[DATA_INDEX_windSpeed];
    newRecord->highWindSpeed    = (uint8_t)archivePkt->value[DATA_INDEX_windGust];
    newRecord->highWindDir      = (uint8_t)(archivePkt->value[DATA_INDEX_windGustDir] / 22.5);
    newRecord->prevWindDir      = (uint8_t)(archivePkt->value[DATA_INDEX_windDir] / 22.5);
    newRecord->UV               = (uint8_t)(archivePkt->value[DATA_INDEX_UV]*10);
    newRecord->ET               = (uint8_t)(archivePkt->value[DATA_INDEX_ET]*1000);
    newRecord->highRadiation    = (uint16_t)(archivePkt->value[DATA_INDEX_radiation]);
    newRecord->highUV           = (uint8_t)(archivePkt->value[DATA_INDEX_UV]*10);
    newRecord->fcstRule         = 0xff;
    newRecord->leafTemp1        = GETVALWITHOFFSET(archivePkt->value[DATA_INDEX_leafTemp1],90);
    newRecord->leafTemp2        = GETVALWITHOFFSET(archivePkt->value[DATA_INDEX_leafTemp2],90);
    newRecord->leafWet1         = GETVALNOOFFSET(archivePkt->value[DATA_INDEX_leafWet1]);
    newRecord->leafWet2         = GETVALNOOFFSET(archivePkt->value[DATA_INDEX_leafWet2]);
    newRecord->soilTemp1        = GETVALWITHOFFSET(archivePkt->value[DATA_INDEX_soilTemp1],90);
    newRecord->soilTemp2        = GETVALWITHOFFSET(archivePkt->value[DATA_INDEX_soilTemp2],90);
    newRecord->soilTemp3        = GETVALWITHOFFSET(archivePkt->value[DATA_INDEX_soilTemp3],90);
    newRecord->soilTemp4        = GETVALWITHOFFSET(archivePkt->value[DATA_INDEX_soilTemp4],90);
    newRecord->recordType       = 0;
    newRecord->extraHumid1      = GETVALNOOFFSET(archivePkt->value[DATA_INDEX_extraHumid1]);
    newRecord->extraHumid2      = GETVALNOOFFSET(archivePkt->value[DATA_INDEX_extraHumid2]);
    newRecord->extraTemp1       = GETVALWITHOFFSET(archivePkt->value[DATA_INDEX_extraTemp1],90);
    newRecord->extraTemp2       = GETVALWITHOFFSET(archivePkt->value[DATA_INDEX_extraTemp2],90);
    newRecord->extraTemp3       = GETVALWITHOFFSET(archivePkt->value[DATA_INDEX_extraTemp3],90);
    newRecord->soilMoist1       = GETVALNOOFFSET(archivePkt->value[DATA_INDEX_soilMoist1]);
    newRecord->soilMoist2       = GETVALNOOFFSET(archivePkt->value[DATA_INDEX_soilMoist2]);
    newRecord->soilMoist3       = GETVALNOOFFSET(archivePkt->value[DATA_INDEX_soilMoist3]);
    newRecord->soilMoist4       = GETVALNOOFFSET(archivePkt->value[DATA_INDEX_soilMoist4]);

    return;
}

// Convert SQLite3 database to WLK database:
static void ConvertSqliteToWlk (char *srcDir, char *destDir)
{
    time_t              timeStamp, diffTime, retVal, startTime = time(NULL);
    struct tm           buildTime;
    ARCHIVE_RECORD      wlkPkt;
    ARCHIVE_PKT         sqlitePkt;
    int                 inserts = 0, errors = 0;
    char                cmnd[_MAX_PATH];

    dbsqliteArchiveSetPath(srcDir);

    // ... Initialize the archive database interface:
    if (dbsqliteArchiveInit() == ERROR)
    {
        printf("dbsqliteArchiveInit failed");
        return;
    }

    sprintf(cmnd, "rm -rf %s", destDir);
    if (system(cmnd) != 0)
    {
        printf("system error:%s:%s\n", cmnd, strerror(errno));
    }
    if (mkdir(destDir, S_IFDIR | 00777) != 0)
    {
        printf("mkdir:%s:%s\n", destDir, strerror(errno));
    }

    // Set the start date to 1/1/2000:
    // Build the sqlite timestamp:
    memset (&buildTime, 0, sizeof(buildTime));
    buildTime.tm_min    = 0;
    buildTime.tm_hour   = 0;
    buildTime.tm_mday   = 1;
    buildTime.tm_mon    = 0;
    buildTime.tm_year   = 100;
    buildTime.tm_isdst  = -1;
    timeStamp = mktime(&buildTime);

    for ((retVal = dbsqliteArchiveGetNextRecord(timeStamp, &sqlitePkt)); 
         (int)retVal != ERROR;
         (retVal = dbsqliteArchiveGetNextRecord(retVal, &sqlitePkt)))
    {
        // OK, need to insert this puppy:
        convertArchivePktToWLK(&sqlitePkt, &wlkPkt, retVal);

        if (dbfStoreArchiveRecord(destDir, &wlkPkt, sqlitePkt.interval, 0x1000) == ERROR)
        {
            // Error:
            errors ++;
        }
        else
        {
            // Good stuff:
            inserts ++;
        }
    }

    diffTime = time(NULL) - startTime;

    dbsqliteArchiveExit();

    // Output results:
    printf("Conversion Stats:\n");
    printf("    Time      : %d:%d\n", (int)diffTime/60, (int)diffTime%60);
    printf("    Recs/sec  : %d\n", (inserts+errors)/(int)diffTime);
    printf("    Inserts   : %d\n", inserts);
    printf("    Errors    : %d\n", errors);
}

int main (int argc, char *argv[])
{
    char            *SourceDir, *DestDir;
    struct stat     fileData;
    
    if (argc < 3)
    {
        USAGE ();
        return ERROR;
    }

    SourceDir = argv[1];
    DestDir = argv[2];

    // sanity check the arguments
    if (stat(SourceDir, &fileData) != 0)
    {
        printf ("Source directory %s does not exist!\n", SourceDir);
        return ERROR;
    }
    else if (!(fileData.st_mode & S_IFDIR))
    {
        printf ("Source directory %s is not a directory!\n", SourceDir);
        return ERROR;
    }
    if (stat (DestDir, &fileData) != 0)
    {
        printf ("Destination directory %s does not exist, creating...\n", DestDir);
        if (mkdir(DestDir, 0755) != 0)
        {
            printf ("Destination directory %s could not be created: %s\n", 
                    DestDir, strerror(errno));
            return -1;
        }
    }

    // OK, args appear to be good, convert SQLite DB to WLK files:
    printf("Converting...(this takes ~25 minutes per year of data at 5 minute archive interval)...\n");
    ConvertSqliteToWlk (SourceDir, DestDir);

    exit (0);
}
