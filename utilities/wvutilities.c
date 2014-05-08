/*---------------------------------------------------------------------
 
 FILE NAME:
        wvutilities.c
 
 PURPOSE:
        Provide the wview utility program common definitions.
 
 REVISION HISTORY:
    Date        Programmer  Revision    Function
    02/26/2006  M.S. Teel   0           Original
 
 ASSUMPTIONS:
 None.
 
------------------------------------------------------------------------*/
#include "wvutilities.h"

static int              DoLe2Be;
static HeaderBlock      hostHeader;

#define SHORT_SWAP(x)           (((x << 8) & 0xFF00) | ((x >> 8) & 0x00FF))
#define LONG_SWAP(x)            (((x << 24) & 0xFF000000) | \
                                 ((x << 8) & 0x00FF0000) | \
                                 ((x >> 8) & 0x0000FF00) | \
                                 ((x >> 24) & 0x000000FF))

#ifdef WORDS_BIGENDIAN

#define SHORT_READ(x)           (DoLe2Be ? SHORT_SWAP(x) : (x))
#define LONG_READ(x)            (DoLe2Be ? LONG_SWAP(x) : (x)) 
#define SHORT_WRITE(x)          (DoLe2Be ? (x) : SHORT_SWAP(x))
#define LONG_WRITE(x)           (DoLe2Be ? (x) : LONG_SWAP(x))

#else

#define SHORT_READ(x)           (DoLe2Be ? (x) : SHORT_SWAP(x))
#define LONG_READ(x)            (DoLe2Be ? (x) : LONG_SWAP(x))
#define SHORT_WRITE(x)          (DoLe2Be ? SHORT_SWAP(x) : (x))
#define LONG_WRITE(x)           (DoLe2Be ? LONG_SWAP(x) : (x))

#endif

//// Local methods
static char *buildArchiveFileName
(
    char        *path,
    uint16_t    date,
    char        *store,
    int         *yr,
    int         *mo
)
{
    *yr = (date/100) + 2000;
    *mo = date%100;

    sprintf (store, "%s/%4.4d-%2.2d.wlk", path, *yr, *mo);
    return store;
}

static char *incrementArchiveFileName
(
    char    *oldName,
    char    *store,
    int     *yr,
    int     *mo
)
{
    uint16_t    year, month;
    char        temp[32], path[256];
    int         i;

    wvstrncpy (path, oldName, sizeof(path));
    for (i = strlen (oldName) - 1; i >= 0; i --)
    {
        if (path[i] == '/')
        {
            path[i] = 0;
            break;
        }
    }
    if (i < 0)
    {
        path[0] = '.';
    }

    memset (temp, 0, sizeof(temp));
    strncpy (temp, &oldName[strlen(path)+1], 4);
    year = atoi (temp);

    memset (temp, 0, sizeof(temp));
    strncpy (temp, &oldName[strlen(path)+6], 2);
    month = atoi (temp);

    if (month >= 12)
    {
        month = 1;
        year ++;
    }
    else
    {
        month ++;
    }

    sprintf (store, "%s/%4.4d-%2.2d.wlk", path, year, month);
    *yr = year;
    *mo = month;
    return store;
}

static int readArcHeader (FILE *inFile, HeaderBlock *header)
{
    int         i;
    int         isSourceNative = TRUE;

    if (fread (header, sizeof(*header), 1, inFile) != 1)
    {
        printf ("read error\n");
        return ERROR;
    }

    hostHeader = *header;

    // test to see if it is already the desired endianness
    if (DoLe2Be)
    {
#ifdef WORDS_BIGENDIAN
        if ((unsigned int)header->totalRecords < 0x10000)
        {
            printf ("File is already in Big Endian format\n");
            return ERROR;
        }
        else
        {
            isSourceNative = FALSE;
        }
#else
        if ((unsigned int)header->totalRecords >= 0x10000)
        {
            printf ("File is already in Big Endian format\n");
            return ERROR;
        }
#endif
    }
    else
    {
#ifdef WORDS_BIGENDIAN
        if ((unsigned int)header->totalRecords >= 0x10000)
        {
            printf ("File is already in Little Endian format\n");
            return ERROR;
        }
#else
        if ((unsigned int)header->totalRecords < 0x10000)
        {
            printf ("File is already in Little Endian format\n");
            return ERROR;
        }
        else
        {
            isSourceNative = FALSE;
        }
#endif
    }

    // Do header conversion (ID field is a string and does not require conversion)
    header->totalRecords = LONG_READ(header->totalRecords);

    for (i = 1; i <= 31; i ++)
    {
        header->dayIndex[i].recordsInDay = SHORT_READ(header->dayIndex[i].recordsInDay);
        header->dayIndex[i].startPos     = LONG_READ(header->dayIndex[i].startPos);
    }

    if (!isSourceNative)
    {
        hostHeader = *header;
    }

    return OK;
}

static int writeArcHeader (FILE *outFile, HeaderBlock *header)
{
    int         i;

    header->totalRecords = LONG_WRITE(header->totalRecords);

    for (i = 1; i <= 31; i ++)
    {
        header->dayIndex[i].recordsInDay = SHORT_WRITE(header->dayIndex[i].recordsInDay);
        header->dayIndex[i].startPos     = LONG_WRITE(header->dayIndex[i].startPos);
    }

    if (fwrite (header, sizeof(*header), 1, outFile) != 1)
    {
        printf ("write error\n");
        return ERROR;
    }

    return OK;
}

static int readSummaryRecord (FILE *inFile, DailySummaryRecord *record)
{
    if (fread (record, sizeof(*record), 1, inFile) != 1)
    {
        return ERROR;
    }

    record->dataSpan           = SHORT_READ(record->dataSpan);
    record->hiOutTemp          = SHORT_READ(record->hiOutTemp);
    record->lowOutTemp         = SHORT_READ(record->lowOutTemp);
    record->hiInTemp           = SHORT_READ(record->hiInTemp);
    record->lowInTemp          = SHORT_READ(record->lowInTemp);
    record->avgOutTemp         = SHORT_READ(record->avgOutTemp);
    record->avgInTemp          = SHORT_READ(record->avgInTemp);
    record->hiChill            = SHORT_READ(record->hiChill);
    record->lowChill           = SHORT_READ(record->lowChill);
    record->hiDew              = SHORT_READ(record->hiDew);
    record->lowDew             = SHORT_READ(record->lowDew);
    record->avgChill           = SHORT_READ(record->avgChill);
    record->avgDew             = SHORT_READ(record->avgDew);
    record->hiOutHum           = SHORT_READ(record->hiOutHum);
    record->lowOutHum          = SHORT_READ(record->lowOutHum);
    record->hiInHum            = SHORT_READ(record->hiInHum);
    record->lowInHum           = SHORT_READ(record->lowInHum);
    record->avgOutHum          = SHORT_READ(record->avgOutHum);
    record->hiBar              = SHORT_READ(record->hiBar);
    record->lowBar             = SHORT_READ(record->lowBar);
    record->avgBar             = SHORT_READ(record->avgBar);
    record->hiSpeed            = SHORT_READ(record->hiSpeed);
    record->avgSpeed           = SHORT_READ(record->avgSpeed);
    record->dailyWindRunTotal  = SHORT_READ(record->dailyWindRunTotal);
    record->hi10MinSpeed       = SHORT_READ(record->hi10MinSpeed);
    record->dailyRainTotal     = SHORT_READ(record->dailyRainTotal);
    record->hiRainRate         = SHORT_READ(record->hiRainRate);
    record->dailyUVDose        = SHORT_READ(record->dailyUVDose);
    record->todaysWeather      = SHORT_READ(record->todaysWeather);
    record->numWindPackets     = SHORT_READ(record->numWindPackets);
    record->hiSolar            = SHORT_READ(record->hiSolar);
    record->dailySolarEnergy   = SHORT_READ(record->dailySolarEnergy);
    record->minSunlight        = SHORT_READ(record->minSunlight);
    record->dailyETTotal       = SHORT_READ(record->dailyETTotal);
    record->hiHeat             = SHORT_READ(record->hiHeat);
    record->lowHeat            = SHORT_READ(record->lowHeat);
    record->avgHeat            = SHORT_READ(record->avgHeat);
    record->hiTHSW             = SHORT_READ(record->hiTHSW);
    record->lowTHSW            = SHORT_READ(record->lowTHSW);
    record->hiTHW              = SHORT_READ(record->hiTHW);
    record->lowTHW             = SHORT_READ(record->lowTHW);
    record->integratedHeatDD65 = SHORT_READ(record->integratedHeatDD65);
    record->hiWetBulb          = SHORT_READ(record->hiWetBulb);
    record->lowWetBulb         = SHORT_READ(record->lowWetBulb);
    record->avgWetBulb         = SHORT_READ(record->avgWetBulb);
    record->integratedCoolDD65 = SHORT_READ(record->integratedCoolDD65);
    
    return OK;
}

static int writeSummaryRecord (FILE *outFile, DailySummaryRecord *record)
{
    record->dataSpan           = SHORT_WRITE(record->dataSpan);
    record->hiOutTemp          = SHORT_WRITE(record->hiOutTemp);
    record->lowOutTemp         = SHORT_WRITE(record->lowOutTemp);
    record->hiInTemp           = SHORT_WRITE(record->hiInTemp);
    record->lowInTemp          = SHORT_WRITE(record->lowInTemp);
    record->avgOutTemp         = SHORT_WRITE(record->avgOutTemp);
    record->avgInTemp          = SHORT_WRITE(record->avgInTemp);
    record->hiChill            = SHORT_WRITE(record->hiChill);
    record->lowChill           = SHORT_WRITE(record->lowChill);
    record->hiDew              = SHORT_WRITE(record->hiDew);
    record->lowDew             = SHORT_WRITE(record->lowDew);
    record->avgChill           = SHORT_WRITE(record->avgChill);
    record->avgDew             = SHORT_WRITE(record->avgDew);
    record->hiOutHum           = SHORT_WRITE(record->hiOutHum);
    record->lowOutHum          = SHORT_WRITE(record->lowOutHum);
    record->hiInHum            = SHORT_WRITE(record->hiInHum);
    record->lowInHum           = SHORT_WRITE(record->lowInHum);
    record->avgOutHum          = SHORT_WRITE(record->avgOutHum);
    record->hiBar              = SHORT_WRITE(record->hiBar);
    record->lowBar             = SHORT_WRITE(record->lowBar);
    record->avgBar             = SHORT_WRITE(record->avgBar);
    record->hiSpeed            = SHORT_WRITE(record->hiSpeed);
    record->avgSpeed           = SHORT_WRITE(record->avgSpeed);
    record->dailyWindRunTotal  = SHORT_WRITE(record->dailyWindRunTotal);
    record->hi10MinSpeed       = SHORT_WRITE(record->hi10MinSpeed);
    record->dailyRainTotal     = SHORT_WRITE(record->dailyRainTotal);
    record->hiRainRate         = SHORT_WRITE(record->hiRainRate);
    record->dailyUVDose        = SHORT_WRITE(record->dailyUVDose);
    record->todaysWeather      = SHORT_WRITE(record->todaysWeather);
    record->numWindPackets     = SHORT_WRITE(record->numWindPackets);
    record->hiSolar            = SHORT_WRITE(record->hiSolar);
    record->dailySolarEnergy   = SHORT_WRITE(record->dailySolarEnergy);
    record->minSunlight        = SHORT_WRITE(record->minSunlight);
    record->dailyETTotal       = SHORT_WRITE(record->dailyETTotal);
    record->hiHeat             = SHORT_WRITE(record->hiHeat);
    record->lowHeat            = SHORT_WRITE(record->lowHeat);
    record->avgHeat            = SHORT_WRITE(record->avgHeat);
    record->hiTHSW             = SHORT_WRITE(record->hiTHSW);
    record->lowTHSW            = SHORT_WRITE(record->lowTHSW);
    record->hiTHW              = SHORT_WRITE(record->hiTHW);
    record->lowTHW             = SHORT_WRITE(record->lowTHW);
    record->integratedHeatDD65 = SHORT_WRITE(record->integratedHeatDD65);
    record->hiWetBulb          = SHORT_WRITE(record->hiWetBulb);
    record->lowWetBulb         = SHORT_WRITE(record->lowWetBulb);
    record->avgWetBulb         = SHORT_WRITE(record->avgWetBulb);
    record->integratedCoolDD65 = SHORT_WRITE(record->integratedCoolDD65);
    
    if (fwrite (record, sizeof(*record), 1, outFile) != 1)
    {
        return ERROR;
    }
    
    return OK;
}

static int readArchiveRecord (FILE *inFile, ArchiveRecord *record)
{
    if (fread (record, sizeof(*record), 1, inFile) != 1)
    {
        return ERROR;
    }

    record->packedTime         = SHORT_READ(record->packedTime);
    record->outsideTemp        = SHORT_READ(record->outsideTemp);
    record->hiOutsideTemp      = SHORT_READ(record->hiOutsideTemp);
    record->lowOutsideTemp     = SHORT_READ(record->lowOutsideTemp);
    record->insideTemp         = SHORT_READ(record->insideTemp);
    record->barometer          = SHORT_READ(record->barometer);
    record->outsideHum         = SHORT_READ(record->outsideHum);
    record->insideHum          = SHORT_READ(record->insideHum);
    record->rain               = SHORT_READ(record->rain);
    record->hiRainRate         = SHORT_READ(record->hiRainRate);
    record->windSpeed          = SHORT_READ(record->windSpeed);
    record->hiWindSpeed        = SHORT_READ(record->hiWindSpeed);
    record->numWindSamples     = SHORT_READ(record->numWindSamples);
    record->solarRad           = SHORT_READ(record->solarRad);
    record->hisolarRad         = SHORT_READ(record->hisolarRad);
    
    return OK;
}

static int writeArchiveRecord (FILE *outFile, ArchiveRecord *record)
{
    record->packedTime         = SHORT_WRITE(record->packedTime);
    record->outsideTemp        = SHORT_WRITE(record->outsideTemp);
    record->hiOutsideTemp      = SHORT_WRITE(record->hiOutsideTemp);
    record->lowOutsideTemp     = SHORT_WRITE(record->lowOutsideTemp);
    record->insideTemp         = SHORT_WRITE(record->insideTemp);
    record->barometer          = SHORT_WRITE(record->barometer);
    record->outsideHum         = SHORT_WRITE(record->outsideHum);
    record->insideHum          = SHORT_WRITE(record->insideHum);
    record->rain               = SHORT_WRITE(record->rain);
    record->hiRainRate         = SHORT_WRITE(record->hiRainRate);
    record->windSpeed          = SHORT_WRITE(record->windSpeed);
    record->hiWindSpeed        = SHORT_WRITE(record->hiWindSpeed);
    record->numWindSamples     = SHORT_WRITE(record->numWindSamples);
    record->solarRad           = SHORT_WRITE(record->solarRad);
    record->hisolarRad         = SHORT_WRITE(record->hisolarRad);
    
    if (fwrite (record, sizeof(*record), 1, outFile) != 1)
    {
        return ERROR;
    }
    
    return OK;
}

static void convertDayRecords (FILE *inFile, FILE *outFile, HeaderBlock *header, int dayNumber)
{
    int                 i;
    DailySummaryRecord  summary;
    ArchiveRecord       record;
    

    if (header->dayIndex[dayNumber].recordsInDay == 0)
    {
        // nothing for us to do
        return;
    }

    // goto the beginning of the archive recs for this day
    if (fseek (inFile, 
               sizeof(*header) + (DBFILES_RECORD_SIZE * header->dayIndex[dayNumber].startPos),
               SEEK_SET)
        == -1)
    {
        return;
    }

    if (readSummaryRecord (inFile, &summary) == OK)
    {
        fseek (outFile, 0, SEEK_END);
        writeSummaryRecord (outFile, &summary);

        for (i = 2; i < header->dayIndex[dayNumber].recordsInDay; i ++)
        {
            if (readArchiveRecord (inFile, &record) == OK)
            {
                writeArchiveRecord (outFile, &record);
            }
        }
    }

    return;
}


//// API methods

int wvuConvertWLKFiles (char *srcDir, char *destDir, int doLE2BE)
{
    char            sourceName[512], destName[512];
    uint16_t        fileDate;
    struct stat     fileStatus;
    FILE            *inFile, *outFile;
    int             retVal, year, month, dummyYear, dummyMonth;
    int             curyear, curmonth;
    int             i, done = FALSE;
    time_t          ntime;
    struct tm       locTime;
    HeaderBlock     Header;

    DoLe2Be = doLE2BE;
    ntime = time (NULL);
    localtime_r (&ntime, &locTime);
    curmonth = locTime.tm_mon + 1;
    curyear = locTime.tm_year + 1900;

    // start at 1/1/2000
    fileDate = 0;
    buildArchiveFileName (srcDir, fileDate, sourceName, &year, &month);
    buildArchiveFileName (destDir, fileDate, destName, &dummyYear, &dummyMonth);

    while (!done)
    {
        if ((year > curyear) || (year == curyear && month > curmonth))
        {
            // we're done!
            done = TRUE;
            continue;
        }

        if (stat (sourceName, &fileStatus) == -1)
        {
            // move on to the next month
            incrementArchiveFileName (sourceName, sourceName, &year, &month);
            incrementArchiveFileName (destName, destName, &dummyYear, &dummyMonth);
            continue;
        }

        // if we are here, we have a source file to work with
        inFile = fopen (sourceName, "r");
        if (inFile == NULL)
        {
            printf ("Problem opening %s\n", sourceName);
            incrementArchiveFileName (sourceName, sourceName, &year, &month);
            incrementArchiveFileName (destName, destName, &dummyYear, &dummyMonth);
            continue;
        }
        outFile = fopen (destName, "w");
        if (outFile == NULL)
        {
            printf ("Problem opening %s\n", destName);
            fclose (inFile);
            incrementArchiveFileName (sourceName, sourceName, &year, &month);
            incrementArchiveFileName (destName, destName, &dummyYear, &dummyMonth);
            continue;
        }

        // now do the dirty work
        if (readArcHeader (inFile, &Header) == ERROR)
        {
            printf ("Source File %s skipped\n", sourceName);
            fclose (outFile);
            unlink (destName);
            fclose (inFile);
            incrementArchiveFileName (sourceName, sourceName, &year, &month);
            incrementArchiveFileName (destName, destName, &dummyYear, &dummyMonth);
            continue;
        }

        if (writeArcHeader (outFile, &Header) == ERROR)
        {
            printf ("Write error on %s - %s skipped\n", destName, sourceName);
            fclose (outFile);
            unlink (destName);
            fclose (inFile);
            incrementArchiveFileName (sourceName, sourceName, &year, &month);
            incrementArchiveFileName (destName, destName, &dummyYear, &dummyMonth);
            continue;
        }

        // process each day
        for (i = 1; i <= 31; i ++)
        {
            convertDayRecords (inFile, outFile, &hostHeader, i);
        }

        fclose (outFile);
        fclose (inFile);
        incrementArchiveFileName (sourceName, sourceName, &year, &month);
        incrementArchiveFileName (destName, destName, &dummyYear, &dummyMonth);
    }

    return OK;
}

