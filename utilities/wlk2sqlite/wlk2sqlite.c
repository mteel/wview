/*---------------------------------------------------------------------
 
 FILE NAME:
        wlk2sqlite.c
 
 PURPOSE:
        wview archive file convertor utility: Davis WLK TO SQLite3 database.
 
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
    printf ("Usage: wlk2sqlite <source_directory> [destination_directory]\n\n");
    printf ("    Convert WLK archive file data in <source_directory> to SQLite3 format in [destination_directory]/wview-archive.sdb\n");
    printf ("    or to $prefix/var/wview/archive/wview-archive.sdb if [destination_directory] is not given\n");
    printf ("Note: Must be run as root if [destination_directory] is not given. Only unique records will be inserted.\n");
    printf ("Note: <source_directory> can be the same as [destination_directory]\n\n");
    return;
}

static void convertWLKToArchivePkt(ArchiveRecord* newRecord, ARCHIVE_PKT* archivePkt, time_t timeStamp)
{
    Data_Indices index;
    float           click;

    // create the time_t time for the record:
    archivePkt->dateTime = (int32_t)timeStamp;

    archivePkt->usUnits  = 1;
    archivePkt->interval = newRecord->archiveInterval;

    // Set all values to NULL by default:
    for (index = DATA_INDEX_barometer; index < DATA_INDEX_MAX; index ++)
    {
        archivePkt->value[index] = ARCHIVE_VALUE_NULL;
    }

    // Set the values we can:
    archivePkt->value[DATA_INDEX_outTemp]        = (float)newRecord->outsideTemp/10.0;
    archivePkt->value[DATA_INDEX_barometer]      = (float)newRecord->barometer/1000.0;
    archivePkt->value[DATA_INDEX_inTemp]         = (float)newRecord->insideTemp/10.0;
    archivePkt->value[DATA_INDEX_inHumidity]     = (float)newRecord->insideHum/10.0;
    archivePkt->value[DATA_INDEX_outHumidity]    = (float)newRecord->outsideHum/10.0;
    archivePkt->value[DATA_INDEX_windSpeed]      = (float)newRecord->windSpeed/10.0;
    if (newRecord->windDirection < 16)
    {
        archivePkt->value[DATA_INDEX_windDir]    = (float)newRecord->windDirection * 22.5;
    }
    archivePkt->value[DATA_INDEX_windGust]       = (float)newRecord->hiWindSpeed/10.0;
    if (newRecord->hiWindDirection < 16)
    {
        archivePkt->value[DATA_INDEX_windGustDir] = (float)newRecord->hiWindDirection * 22.5;
    }

    if ((newRecord->rain & 0xF000) == 0x0000)
        click = 10;
    else if ((newRecord->rain & 0xF000) == 0x2000) //0.2 mm 0x2000
        click = 127;
    else if ((newRecord->rain & 0xF000) == 0x3000) //1.0 mm 0x3000
        click = 25.4;
    else if ((newRecord->rain & 0xF000) == 0x6000) //0.1 mm 0x6000 (not fully supported)
        click = 254;
    else
        click = 100;

    archivePkt->value[DATA_INDEX_rainRate]       
        = (float)newRecord->hiRainRate/click;
    archivePkt->value[DATA_INDEX_rain]           
        = ((float)(newRecord->rain & 0xFFF))/click;
    archivePkt->value[DATA_INDEX_dewpoint]       
        = wvutilsCalculateDewpoint ((float)archivePkt->value[DATA_INDEX_outTemp],
                                    (float)archivePkt->value[DATA_INDEX_outHumidity]);
    archivePkt->value[DATA_INDEX_windchill]      
        = wvutilsCalculateWindChill ((float)archivePkt->value[DATA_INDEX_outTemp],
                                     (float)archivePkt->value[DATA_INDEX_windSpeed]);
    archivePkt->value[DATA_INDEX_heatindex]      
        = wvutilsCalculateHeatIndex ((float)archivePkt->value[DATA_INDEX_outTemp],
                                     (float)archivePkt->value[DATA_INDEX_outHumidity]);
    archivePkt->value[DATA_INDEX_ET]             = (float)newRecord->ET/1000.0;
    if ((uint16_t)newRecord->solarRad != 0x7FFF && 
        (uint16_t)newRecord->solarRad != 0xFFFF && 
        (float)newRecord->solarRad >= 0 && 
        (float)newRecord->solarRad <= 1800)
    {
        archivePkt->value[DATA_INDEX_radiation]  = (float)newRecord->solarRad;
    }
    if (newRecord->UV != 0xFF)
    {
        archivePkt->value[DATA_INDEX_UV]         = (float)newRecord->UV/10.0;
    }
    if (newRecord->extraTemp[0] != 0xFF)
    {
        archivePkt->value[DATA_INDEX_extraTemp1] = (float)(newRecord->extraTemp[0] - 90);
    }
    if (newRecord->extraTemp[1] != 0xFF)
    {
        archivePkt->value[DATA_INDEX_extraTemp2] = (float)(newRecord->extraTemp[1] - 90);
    }
    if (newRecord->extraTemp[2] != 0xFF)
    {
        archivePkt->value[DATA_INDEX_extraTemp3] = (float)(newRecord->extraTemp[2] - 90);
    }
    if (newRecord->soilTemp[0] != 0xFF)
    {
        archivePkt->value[DATA_INDEX_soilTemp1]  = (float)(newRecord->soilTemp[0] - 90);
    }
    if (newRecord->soilTemp[1] != 0xFF)
    {
        archivePkt->value[DATA_INDEX_soilTemp2]  = (float)(newRecord->soilTemp[1] - 90);
    }
    if (newRecord->soilTemp[2] != 0xFF)
    {
        archivePkt->value[DATA_INDEX_soilTemp3]  = (float)(newRecord->soilTemp[2] - 90);
    }
    if (newRecord->soilTemp[3] != 0xFF)
    {
        archivePkt->value[DATA_INDEX_soilTemp4]  = (float)(newRecord->soilTemp[3] - 90);
    }
    if (newRecord->leafTemp[0] != 0xFF)
    {
        archivePkt->value[DATA_INDEX_leafTemp1]  = (float)(newRecord->leafTemp[0] - 90);
    }
    if (newRecord->leafTemp[1] != 0xFF)
    {
        archivePkt->value[DATA_INDEX_leafTemp2]  = (float)(newRecord->leafTemp[1] - 90);
    }
    if (newRecord->extraHum[0] != 0xFF)
    {
        archivePkt->value[DATA_INDEX_extraHumid1] = (float)newRecord->extraHum[0];
    }
    if (newRecord->extraHum[1] != 0xFF)
    {
        archivePkt->value[DATA_INDEX_extraHumid2] = (float)newRecord->extraHum[1];
    }
    if (newRecord->soilMoisture[0] != 0xFF)
    {
        archivePkt->value[DATA_INDEX_soilMoist1] = (float)newRecord->soilMoisture[0];
    }
    if (newRecord->soilMoisture[1] != 0xFF)
    {
        archivePkt->value[DATA_INDEX_soilMoist2] = (float)newRecord->soilMoisture[1];
    }
    if (newRecord->soilMoisture[2] != 0xFF)
    {
        archivePkt->value[DATA_INDEX_soilMoist3] = (float)newRecord->soilMoisture[2];
    }
    if (newRecord->soilMoisture[3] != 0xFF)
    {
        archivePkt->value[DATA_INDEX_soilMoist4] = (float)newRecord->soilMoisture[3];
    }
    if (newRecord->leafWetness[0] != 0xFF)
    {
        archivePkt->value[DATA_INDEX_leafWet1]   = (float)newRecord->leafWetness[0];
    }
    if (newRecord->leafWetness[1] != 0xFF)
    {
        archivePkt->value[DATA_INDEX_leafWet2]   = (float)newRecord->leafWetness[1];
    }

    return;
}

static void ConvertWlkToSqlite (char *srcDir, char *destDir)
{
    time_t              timeStamp, diffTime, startTime = time(NULL);
    struct tm           buildTime;
    uint16_t            recdate, rectime;
    ArchiveRecord       wlkPkt;
    ARCHIVE_PKT         sqlitePkt;
    int                 dups = 0, inserts = 0, errors = 0, lastminute = 0;

    if (destDir != NULL)
    {
        dbsqliteArchiveSetPath(destDir);
    }

    // ... Initialize the archive database interface:
    if (dbsqliteArchiveInit() == ERROR)
    {
        printf("dbsqliteArchiveInit failed\n");
        return;
    }
    
    // Avoid expensive fsyncs during conversion
    if (dbsqliteArchivePragmaSet("synchronous", "off") == ERROR)
    {
        printf("dbsqliteArchivePragmaSet synchronous = off failed\n");
        return;
    }

    // Avoid expensive journal operations during conversion
    if (dbsqliteArchivePragmaSet("journal_mode", "off") == ERROR)
    {
        printf("dbsqliteArchivePragmaSet journal_mode = off failed\n");
        return;
    }

    // Set the start date to 1/1/2000:
    recdate = INSERT_PACKED_DATE(2000,1,1);
    rectime = 0;
    while (dbfGetNextArchiveRecord(srcDir, &recdate, &rectime, &wlkPkt) != ERROR)
    {
        // Build the sqlite timestamp:
        memset (&buildTime, 0, sizeof(buildTime));
        buildTime.tm_min    = EXTRACT_PACKED_MINUTE(rectime);
        buildTime.tm_hour   = EXTRACT_PACKED_HOUR(rectime);
        buildTime.tm_mday   = EXTRACT_PACKED_DAY(recdate);
        buildTime.tm_mon    = EXTRACT_PACKED_MONTH(recdate) - 1;
        buildTime.tm_year   = EXTRACT_PACKED_YEAR(recdate) - 1900;
        buildTime.tm_isdst  = -1;
        timeStamp = mktime(&buildTime);

        // Check to see if this record exists in the sqlite DB:
        if (dbsqliteArchiveGetRecord(timeStamp, &sqlitePkt) != ERROR)
        {
            // duplicate, skip it:
            dups ++;
            continue;
        }

        // OK, need to insert this puppy:
        convertWLKToArchivePkt(&wlkPkt, &sqlitePkt, timeStamp);

        if (dbsqliteArchiveStoreRecord(&sqlitePkt) == ERROR)
        {
            // Error:
            errors ++;
        }
        else
        {
            // Good stuff:
            inserts ++;
        }

        diffTime = time(NULL) - startTime;

        // Output running stats
        if ( (int)diffTime%60 == 0 && (int)diffTime/60 > 0 && (int)diffTime/60 != lastminute )
        {
            printf("Conversion Stats:\n");
            printf("    Time      : %d:%d minutes\n", (int)diffTime/60, (int)diffTime%60);
            printf("    Recs/sec  : %d\n", (inserts+dups+errors)/(int)diffTime);
            printf("    Inserts   : %d\n", inserts);
            printf("    Duplicates: %d\n", dups);
            printf("    Errors    : %d\n", errors);
            lastminute = (int)diffTime/60;
        }
    }

    dbsqliteArchiveExit();

    diffTime = time(NULL) - startTime;

    // Output results:
    printf("Conversion Stats:\n");
    printf("    Time      : %d:%d\n", (int)diffTime/60, (int)diffTime%60);
    printf("    Recs/sec  : %d\n", (inserts+dups+errors)/(int)diffTime);
    printf("    Inserts   : %d\n", inserts);
    printf("    Duplicates: %d\n", dups);
    printf("    Errors    : %d\n", errors);
}

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char *argv[])
{
    char            *SourceDir, *DestDir;
    struct stat     fileData;
    
    if (argc < 2)
    {
        USAGE ();
        exit (-1);
    }
    if (argc < 3)
    {
        printf("Using default $prefix/var/wview/archive destination...\n");
        DestDir = NULL;
    }
    else
    {
        DestDir = argv[2];
    }

    SourceDir = argv[1];

    // sanity check the arguments
    if (stat(SourceDir, &fileData) != 0)
    {
        printf ("Source directory %s does not exist!\n", SourceDir);
        exit (-1);
    }
    else if (!(fileData.st_mode & S_IFDIR))
    {
        printf ("Source directory %s is not a directory!\n", SourceDir);
        exit (-1);
    }
    if (DestDir)
    {
        if (stat(DestDir, &fileData) != 0)
        {
            printf ("Destination directory %s does not exist!\n", DestDir);
            exit (-1);
        }
        else if (!(fileData.st_mode & S_IFDIR))
        {
            printf ("Destination directory %s is not a directory!\n", DestDir);
            exit (-1);
        }
    }

    // OK, args appear to be good, convert WLK files to our SQLite DB:
    printf("Converting...(this takes a while, longer on small devices)...\n");
    ConvertWlkToSqlite (SourceDir, DestDir);
    exit (0);
}

