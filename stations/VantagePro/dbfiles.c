//----------------------------------------------------------------------------
//
//  FILENAME:
//        dbfiles.c
//
//  PURPOSE:
//        Provide the weather station database files utilities.
//
//  REVISION HISTORY:
//        Date            Engineer        Revision        Remarks
//        08/16/03        M.S. Teel       0               Original
//        04/12/2008      W. Krenn        1               RainCollectorType, 
//                                                        metric ARC
//
//  NOTES:
//
//
//  LICENSE:
//        Copyright (c) 2004, Mark S. Teel (mark@teel.ws)
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
#include <dbfiles.h>

//  ... global memory declarations


//  ... local memory
static HeaderBlock          fileHdr;
static DailySummaryRecord   sumRecord;
static ArchiveRecord        arcRecord;
static char                 idCode[16] = {'W','D','A','T','5','.','0',0,0,0,0,0,0,0,5,0};
static char                 winddir[16][4] =
    {
        "N",
        "NNE",
        "NE",
        "ENE",
        "E",
        "ESE",
        "SE",
        "SSE",
        "S",
        "SSW",
        "SW",
        "WSW",
        "W",
        "WNW",
        "NW",
        "NNW"
    };


//  ... ----- static (local) methods -----

static int extractTimeValue (uint8_t *valarray, int index)
{
    int     fIndex;
    int     value;

    fIndex = (index/2) * 3;
    if ((index % 2) == 0)
    {
        value = valarray[fIndex] + ((valarray[fIndex+2] & 0x0F) << 8);
    }
    else
    {
        value = valarray[fIndex+1] + ((valarray[fIndex+2] & 0xF0) << 4);
    }

    if (value == 0xFFF || value == 0x7FF)
    {
        return -1;
    }
    else
    {
        return value;
    }
}

static int insertTimeValue (uint8_t *valarray, int index, uint16_t value)
{
    int     fIndex;

    fIndex = (index/2) * 3;
    if ((index % 2) == 0)
    {
        valarray[fIndex] = value & 0xFF;
        valarray[fIndex+2] = (valarray[fIndex+2] & 0xF0) | ((value >> 8) & 0x0F);
    }
    else
    {
        valarray[fIndex+1] = value & 0xFF;
        valarray[fIndex+2] = (valarray[fIndex+2] & 0x0F) | ((value >> 4) & 0xF0);
    }

    return value;
}

static char *findDominantDirection (uint8_t *dirarray)
{
    int         i;
    int         max = -1, index = -1, value;

    for (i = 0; i < 16; i ++)
    {
        value = extractTimeValue (dirarray, i);
        if (value == -1)
            continue;

        if (value > max)
        {
            max = value;
            index = i;
        }
    }

    if (index == -1)
        return "---";
    else
        return winddir[index];
}

static char *buildDayTimeFromIndex (uint8_t *valarray, int index, char *store)
{
    int         minutes;

    minutes = extractTimeValue (valarray, index);

    if (minutes == -1)
    {
        strcpy (store, "?????");
    }
    else
    {
        sprintf (store, "%2.2d:%2.2d", minutes/60, minutes%60);
    }

    return store;
}

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

    wvstrncpy (path, oldName, 256);
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

static void incrementDayHourMin (int *day, int *hour, int *min, int numMins)
{
    int     newDay, newHour, newMin;

    // figure out how much numMins "is"
    newDay = numMins/WV_MINUTES_IN_DAY;
    newHour = numMins%WV_MINUTES_IN_DAY;
    newMin = newHour % 60;
    newHour /= 60;

    *day += newDay;
    *hour += newHour;
    *min += newMin;

    return;
}

static int getRecordTimeDeltaInMinutes
(
    int     oldDay,
    int     oldHour,
    int     oldMin,
    int     newDay,
    int     newHour,
    int     newMin
)
{
    int     oldTotal, newTotal;

    oldTotal = oldDay*WV_MINUTES_IN_DAY + oldHour*60 + oldMin;
    newTotal = newDay*WV_MINUTES_IN_DAY + newHour*60 + newMin;

    return (newTotal - oldTotal);
}

static char *decrementArchiveFileName
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

    wvstrncpy (path, oldName, 256);
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

    if (month <= 1)
    {
        month = 12;
        year --;
    }
    else
    {
        month --;
    }

    sprintf (store, "%s/%4.4d-%2.2d.wlk", path, year, month);
    *yr = year;
    *mo = month;
    return store;
}

static void convertArchiveAtoB (ARCHIVE_RECORD *rec)
{
    ARCHIVE_RECORD_A    vpArcRecordA;

    memcpy (&vpArcRecordA, rec, sizeof (*rec));

    rec->highRadiation      = rec->radiation;
    rec->highUV             = rec->UV;
    rec->fcstRule           = 0;
    rec->leafTemp1          = 0xFF;
    rec->leafTemp2          = 0xFF;
    rec->leafWet1           = vpArcRecordA.leafWet1;
    rec->leafWet2           = vpArcRecordA.leafWet2;
    rec->soilTemp1          = vpArcRecordA.soilTemp1;
    rec->soilTemp2          = vpArcRecordA.soilTemp2;
    rec->soilTemp3          = vpArcRecordA.soilTemp3;
    rec->soilTemp4          = vpArcRecordA.soilTemp4;
    rec->recordType         = 0;
    rec->extraHumid1        = vpArcRecordA.extraHumid1;
    rec->extraHumid2        = vpArcRecordA.extraHumid2;
    rec->extraTemp1         = vpArcRecordA.extraTemp1;
    rec->extraTemp2         = vpArcRecordA.extraTemp2;
    rec->extraTemp3         = 0xFF;
    rec->soilMoist1         = vpArcRecordA.soilMoist1;
    rec->soilMoist2         = vpArcRecordA.soilMoist2;
    rec->soilMoist3         = vpArcRecordA.soilMoist3;
    rec->soilMoist4         = vpArcRecordA.soilMoist4;

    return;
}

static int convertFileToAscii (char *startName, int outfilefd, int yr, int mo)
{
    int         i, j;
    FILE        *infile;
    char        temp[512];
    float       click;

    infile = fopen (startName, "r");
    if (infile == NULL)
    {
        return ERROR;
    }

    if (fread (&fileHdr, sizeof(fileHdr), 1, infile) != 1)
    {
        fclose (infile);
        return ERROR;
    }

    for (i = 1; i < 32; i ++)
    {
        if (fileHdr.dayIndex[i].recordsInDay == 0)
        {
            // we're done with this day
            continue;
        }

        //  ... goto the beginning of the archive recs for this day
        if (fseek (infile,
                   sizeof(fileHdr) +
                   DBFILES_RECORD_SIZE*(fileHdr.dayIndex[i].startPos + 2),
                   SEEK_SET)
                == -1)
        {
            fclose (infile);
            return ERROR;
        }

        for (j = 0; j < fileHdr.dayIndex[i].recordsInDay - 2; j ++)
        {
            // read the archive record
            if (fread (&arcRecord, sizeof(arcRecord), 1, infile) != 1)
            {
                fclose (infile);
                return ERROR;
            }

            if (arcRecord.windSpeed/10 > 200)
                arcRecord.windSpeed = 0;

            if ((arcRecord.rain & 0xF000) == 0x0000)
                click = 10;
            else if ((arcRecord.rain & 0xF000) == 0x2000)   //0.2 mm 0x2000
                click = 127;
            else if ((arcRecord.rain & 0xF000) == 0x3000)   //1.0 mm 0x3000
                click = 25.4;
            else if ((arcRecord.rain & 0xF000) == 0x6000)   //0.1 mm 0x6000 (not fully supported)
                click = 254;
            else                                            //0.01 in
                click = 100;

            sprintf (temp,
                     "%2.2d/%2.2d/%4.4d\t%2.2d:%2.2d\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%d\t%2.1f\t%.1f\t%.1f\t%s\t%.2f\t%.3f\t%.1f\t%d\t%d\n",
                     mo,
                     i,
                     yr,
                     arcRecord.packedTime/60,
                     arcRecord.packedTime%60,
                     (float)arcRecord.outsideTemp/10,
                     wvutilsCalculateHeatIndex ((float)arcRecord.outsideTemp/10,
                                                (float)arcRecord.outsideHum/10),
                     wvutilsCalculateWindChill ((float)arcRecord.outsideTemp/10,
                                                (float)arcRecord.windSpeed/10),
                     (float)arcRecord.hiOutsideTemp/10,
                     (float)arcRecord.lowOutsideTemp/10,
                     arcRecord.outsideHum/10,
                     wvutilsCalculateDewpoint ((float)arcRecord.outsideTemp/10,
                                               (float)arcRecord.outsideHum/10),
                     (float)arcRecord.windSpeed/10,
                     (float)arcRecord.hiWindSpeed/10,
                     dbfBuildWindDirString ((int)arcRecord.windDirection),
                     (float)(arcRecord.rain & 0xFFF)/click,
                     (float)arcRecord.barometer/1000,
                     (float)arcRecord.insideTemp/10,
                     arcRecord.insideHum/10,
                     arcRecord.archiveInterval);

            if (write (outfilefd, temp, strlen(temp)) != strlen(temp))
            {
                fclose (infile);
                return ERROR;
            }
        }
    }

    fclose(infile);
    return OK;
}

static int convertSummaryFileToAscii (char *startName, int outfilefd, int yr, int mo)
{
    int         i;
    FILE        *infile;
    char        temp[512], temp1[64];
    char        timestr1[8], timestr2[8], timestr3[8], timestr4[8];
    char        timestr5[8], timestr6[8], timestr7[8], rainRateTime[8];
    float       rainRate;

    infile = fopen (startName, "r");
    if (infile == NULL)
    {
        return ERROR;
    }

    if (fread (&fileHdr, sizeof(fileHdr), 1, infile) != 1)
    {
        fclose (infile);
        return ERROR;
    }


    for (i = 1; i < 32; i ++)
    {
        if (fileHdr.dayIndex[i].recordsInDay == 0)
        {
            // we're done with this day
            continue;
        }

        //  ... goto the beginning of the archive recs for this day
        if (fseek (infile,
                   sizeof(fileHdr) +
                   DBFILES_RECORD_SIZE * fileHdr.dayIndex[i].startPos,
                   SEEK_SET)
                == -1)
        {
            fclose (infile);
            return ERROR;
        }

        // read the summary record
        if (fread (&sumRecord, sizeof(sumRecord), 1, infile) != 1)
        {
            fclose (infile);
            return ERROR;
        }

        sprintf (temp,
                 "%2.2d/%2.2d/%4.4d\t"
                 "%.1f\t%.1f\t%s\t%.1f\t%s\t%.1f\t%s\t%.1f\t%s\t%d\t%d\t%.1f\t%s\t%.1f\t%s\t"
                 "%.1f\t%.1f\t%s\t%s\t"
                 "%.2f\t%.3f\t%.3f\n",
                 mo,
                 i,
                 yr,
                 (float)sumRecord.avgOutTemp/10,
                 (float)sumRecord.hiOutTemp/10,
                 buildDayTimeFromIndex (sumRecord.timeValues1, High_Outside_Temperature, timestr1),
                 (float)sumRecord.lowOutTemp/10,
                 buildDayTimeFromIndex (sumRecord.timeValues1, Low_Outside_Temperature, timestr2),
                 (float)sumRecord.hiHeat/10,
                 buildDayTimeFromIndex (sumRecord.timeValues2, High_Outside_Heat_Index, timestr3),
                 (float)sumRecord.lowChill/10,
                 buildDayTimeFromIndex (sumRecord.timeValues1, Low_Wind_Chill, timestr4),
                 sumRecord.hiOutHum/10,
                 sumRecord.lowOutHum/10,
                 (float)sumRecord.hiDew/10,
                 buildDayTimeFromIndex (sumRecord.timeValues1, High_Dew_Point, timestr5),
                 (float)sumRecord.lowDew/10,
                 buildDayTimeFromIndex (sumRecord.timeValues1, Low_Dew_Point, timestr6),
                 (float)sumRecord.avgSpeed/10,
                 (float)sumRecord.hiSpeed/10,
                 buildDayTimeFromIndex (sumRecord.timeValues1, High_Wind_Speed, timestr7),
                 findDominantDirection (sumRecord.dirBins),
                 (float)sumRecord.dailyRainTotal/1000,
                 (float)sumRecord.hiBar/1000,
                 (float)sumRecord.lowBar/1000);



        if (write (outfilefd, temp, strlen(temp)) != strlen(temp))
        {
            fclose (infile);
            return ERROR;
        }
    }

    fclose(infile);
    return OK;
}


static float    lastGoodTemp        = 680.0;
static float    lastGoodHumid       = 500.0;
static uint8_t  lastGoodExtraTemp   = 158;
static int      lastWDIR            = 0;

#if defined(BUILD_HTMLGEND)
//  ... return num minutes processed or error
static int rollIntoAverages
(
    int             isMetricUnits,
    int             isMetricMM,
    WAVG_ID         windId,
    char            *startName,
    int             day,
    int             hour,
    int             minutes,
    HISTORY_DATA    *store,
    int             numMins
)
{
    int             skipMins, mins = 0;
    int             i, j, k, minsStarted = FALSE;
    float           sum, click;
    FILE            *infile;
    char            temp[512];

    infile = fopen (startName, "r");
    if (infile == NULL)
    {
        return ERROR;
    }

    if (fread (&fileHdr, sizeof(fileHdr), 1, infile) != 1)
    {
        fclose (infile);
        return ERROR;
    }

    for (i = day; i < 32; i ++)
    {
        if (mins >= numMins)
        {
            // we're done
            fclose (infile);
            return mins;
        }

        if (fileHdr.dayIndex[i].recordsInDay == 0)
        {
            // no record for this day, we are done
            fclose (infile);

            if (mins > 0)
                return mins;
            else
                return ERROR;
        }

        //  ... goto the beginning of the archive recs for this day
        if (fseek (infile,
                   sizeof(fileHdr) +
                   DBFILES_RECORD_SIZE*(fileHdr.dayIndex[i].startPos + 2),
                   SEEK_SET)
                == -1)
        {
            fclose (infile);
            return ERROR;
        }

        for (j = 0; (j < fileHdr.dayIndex[i].recordsInDay - 2) && (mins < numMins); j ++)
        {
            // read the archive record
            if (fread (&arcRecord, sizeof(arcRecord), 1, infile) != 1)
            {
                fclose (infile);
                return ERROR;
            }

            if ((arcRecord.rain & 0xF000) == 0x0000)
                click = 10;
            else if ((arcRecord.rain & 0xF000) == 0x2000) //0.2 mm 0x2000
                click = 127;
            else if ((arcRecord.rain & 0xF000) == 0x3000) //1.0 mm 0x3000
                click = 25.4;
            else if ((arcRecord.rain & 0xF000) == 0x6000) //0.1 mm 0x6000 (not fully supported)
                click = 254;
            else
                click = 100;

            if (!minsStarted)
            {
                // we haven't started yet
                if (arcRecord.packedTime/60 < hour)
                {
                    // not our hour yet
                    continue;
                }
                else if (arcRecord.packedTime/60 > hour)
                {
                    // our exact start hour is not there...
                    // increment our minute counter appropriately and see if
                    // there is more work to do - if so we can use this record
                    mins = (arcRecord.packedTime / 60) - hour;
                    mins *= 60;
                    mins += arcRecord.packedTime % 60;
                    if (mins < numMins)     // allow for one interval for this rec
                    {
                        // we can jump forward, making note of the jump time
                        minsStarted = TRUE;
                        mins += arcRecord.archiveInterval;      // for this rec
                        incrementDayHourMin (&day,
                                             &hour,
                                             &minutes,
                                             mins);
                    }
                    else
                    {
                        fclose (infile);
                        return ERROR;
                    }
                }
                else if (arcRecord.packedTime%60 < minutes)
                {
                    // not our mins yet
                    continue;
                }
                else if (arcRecord.packedTime%60 > minutes)
                {
                    // our exact start time minutes is not there...
                    // increment our minute counter appropriately and see if
                    // there is more work to do - if so we can use this record
                    mins = (arcRecord.packedTime % 60) - minutes;
                    if (mins < numMins)     // allow for one interval for this rec
                    {
                        // we can jump forward, making note of the jump time
                        minsStarted = TRUE;
                        mins += arcRecord.archiveInterval;      // for this rec
                        incrementDayHourMin (&day,
                                             &hour,
                                             &minutes,
                                             mins);
                    }
                    else
                    {
                        fclose (infile);
                        return ERROR;
                    }
                }
                else
                {
                    minsStarted = TRUE;
                    mins += arcRecord.archiveInterval;
                    incrementDayHourMin (&day,
                                         &hour,
                                         &minutes,
                                         arcRecord.archiveInterval);
                }
            }
            else
            {
                // we have started
                if (i != day ||
                        arcRecord.packedTime/60 != hour ||
                        arcRecord.packedTime%60 != minutes)
                {
                    // oops, record(s) has been skipped
                    skipMins = getRecordTimeDeltaInMinutes (day,
                                                            hour,
                                                            minutes,
                                                            i,
                                                            arcRecord.packedTime/60,
                                                            arcRecord.packedTime%60);

                    if (mins + skipMins >= numMins)
                    {
                        // we're done
                        fclose (infile);
                        return mins;
                    }
                    else
                    {
                        skipMins += arcRecord.archiveInterval;  // for this rec
                        mins += skipMins;
                        incrementDayHourMin (&day,
                                             &hour,
                                             &minutes,
                                             skipMins);
                    }
                }
                else
                {
                    mins += arcRecord.archiveInterval;
                    incrementDayHourMin (&day,
                                         &hour,
                                         &minutes,
                                         arcRecord.archiveInterval);
                }
            }

            if (arcRecord.windSpeed/10 > 200)
            {
                arcRecord.windSpeed = 0;
            }

            if ((uint16_t)arcRecord.outsideTemp != 0x8000 &&
                (uint16_t)arcRecord.outsideTemp != 0x7FFF)
            {
                lastGoodTemp = arcRecord.outsideTemp;
            }
            else
            {
                arcRecord.outsideTemp = lastGoodTemp;
            }

            if ((uint16_t)arcRecord.outsideHum != 0x8000 &&
                (uint16_t)arcRecord.outsideHum != 0x7FFF &&
                (uint16_t)arcRecord.outsideHum != 2550)
            {
                lastGoodHumid = arcRecord.outsideHum;
            }
            else
            {
                arcRecord.outsideHum = lastGoodHumid;
            }

            for (k = 0; k < 3; k ++)
            {
                if (arcRecord.extraTemp[k] == 0xff) 
                    arcRecord.extraTemp[k] = lastGoodExtraTemp;
                else
                    lastGoodExtraTemp[k] = arcRecord.extraTemp[k];
            }

            for (k = 0; k < DATA_INDEX_MAX; k ++)
            {
                store->samples[k] ++;
            }

            if (isMetricUnits)
            {
                store->values[DATA_INDEX_outTemp] +=
                    wvutilsConvertFToC ((float)arcRecord.outsideTemp/10.0);
                store->values[DATA_INDEX_outHumidity] += (float)arcRecord.outsideHum/10.0;
                store->values[DATA_INDEX_inTemp] +=
                    wvutilsConvertFToC ((float)arcRecord.insideTemp/10.0);
                store->values[DATA_INDEX_inHumidity] += (float)arcRecord.insideHum/10.0;
                store->values[DATA_INDEX_dewpoint] +=
                    wvutilsConvertFToC (wvutilsCalculateDewpoint ((float)arcRecord.outsideTemp/10.0,
                                                   (float)arcRecord.outsideHum/10.0));
                store->values[DATA_INDEX_windSpeed] +=
                    wvutilsConvertMPHToKPH ((float)arcRecord.windSpeed/10.0);
                if (arcRecord.windDirection < 16)
                {
                    lastWDIR = arcRecord.windDirection;
                }
                windAverageAddValue (windId, lastWDIR);

                store->values[DATA_INDEX_windGust] +=
                    wvutilsConvertMPHToKPH ((float)arcRecord.hiWindSpeed/10.0);
                store->values[DATA_INDEX_barometer] +=
                    wvutilsConvertINHGToHPA ((float)arcRecord.barometer/1000.0);

                if (isMetricMM)
                    store->values[DATA_INDEX_rain] +=
                        wvutilsConvertINToMM ((float)(arcRecord.rain & 0xFFF)/click);
                else
                    store->values[DATA_INDEX_rain] +=
                        wvutilsConvertINToCM ((float)(arcRecord.rain & 0xFFF)/click);

                store->values[DATA_INDEX_windchill] +=
                    wvutilsConvertFToC (wvutilsCalculateWindChill ((float)arcRecord.outsideTemp/10.0,
                                                   (float)arcRecord.windSpeed/10.0));
                store->values[DATA_INDEX_heatindex] +=
                    wvutilsConvertFToC (wvutilsCalculateHeatIndex ((float)arcRecord.outsideTemp/10.0,
                                                   (float)arcRecord.outsideHum/10.0));

                if ((uint16_t)arcRecord.solarRad != 0x7FFF && (uint16_t)arcRecord.solarRad != 0xFFFF && (float)arcRecord.solarRad >= 0 && (float)arcRecord.solarRad <= 1800)
                    store->values[DATA_INDEX_radiation] += (float)arcRecord.solarRad;
                if (arcRecord.UV != 0xFF)
                    store->values[DATA_INDEX_UV] += (float)arcRecord.UV/10.0;

                if (arcRecord.ET != 0xFF)
                {
                    if (isMetricMM)
                        store->values[DATA_INDEX_ET] +=
                            wvutilsConvertINToMM ((float)arcRecord.ET/1000.0);
                    else
                        store->values[DATA_INDEX_ET] +=
                            wvutilsConvertINToCM ((float)arcRecord.ET/1000.0);
                }

                if (arcRecord.leafTemp[0] != 0xFF)
                    store->values[DATA_INDEX_leafTemp1] +=
                        (int)wvutilsConvertFToC ((float)arcRecord.leafTemp[0] - 90.0);
                if (arcRecord.leafTemp[1] != 0xFF)
                    store->values[DATA_INDEX_leafTemp2] +=
                        (int)wvutilsConvertFToC ((float)arcRecord.leafTemp[1] - 90.0);
                if (arcRecord.leafWetness[0] != 0xFF)
                    store->values[DATA_INDEX_leafWet1] += (float)arcRecord.leafWetness[0];
                if (arcRecord.leafWetness[1] != 0xFF)
                    store->values[DATA_INDEX_leafWet2] += (float)arcRecord.leafWetness[1];
                if (arcRecord.soilTemp[0] != 0xFF)
                    store->values[DATA_INDEX_soilTemp1] +=
                        (int)wvutilsConvertFToC ((float)arcRecord.soilTemp[0] - 90.0);
                if (arcRecord.soilTemp[1] != 0xFF)
                    store->values[DATA_INDEX_soilTemp2] +=
                        (int)wvutilsConvertFToC ((float)arcRecord.soilTemp[1] - 90.0);
                if (arcRecord.soilTemp[2] != 0xFF)
                    store->values[DATA_INDEX_soilTemp3] +=
                        (int)wvutilsConvertFToC ((float)arcRecord.soilTemp[2] - 90.0);
                if (arcRecord.soilTemp[3] != 0xFF)
                    store->values[DATA_INDEX_soilTemp4] +=
                        (int)wvutilsConvertFToC ((float)arcRecord.soilTemp[3] - 90.0);
                if (arcRecord.extraHum[0] != 0xFF)
                    store->values[DATA_INDEX_extraHumid1] += (float)arcRecord.extraHum[0];
                if (arcRecord.extraHum[1] != 0xFF)
                    store->values[DATA_INDEX_extraHumid2] += (float)arcRecord.extraHum[1];
                if (arcRecord.extraTemp[0] != 0xFF)
                    store->values[DATA_INDEX_extraTemp1] +=
                        (int)wvutilsConvertFToC ((float)arcRecord.extraTemp[0] - 90);
                if (arcRecord.extraTemp[1] != 0xFF)
                    store->values[DATA_INDEX_extraTemp2] +=
                        (int)wvutilsConvertFToC ((float)arcRecord.extraTemp[1] - 90);
                if (arcRecord.extraTemp[2] != 0xFF)
                    store->values[DATA_INDEX_extraTemp3] +=
                        (int)wvutilsConvertFToC ((float)arcRecord.extraTemp[2] - 90);
                if (arcRecord.soilMoisture[0] != 0xFF)
                    store->values[DATA_INDEX_soilMoist1] += (float)arcRecord.soilMoisture[0];
                if (arcRecord.soilMoisture[1] != 0xFF)
                    store->values[DATA_INDEX_soilMoist2] += (float)arcRecord.soilMoisture[1];
                if (arcRecord.soilMoisture[2] != 0xFF)
                    store->values[DATA_INDEX_soilMoist3] += (float)arcRecord.soilMoisture[2];
                if (arcRecord.soilMoisture[3] != 0xFF)
                    store->values[DATA_INDEX_soilMoist4] += (float)arcRecord.soilMoisture[3];
            }
            else
            {
                store->values[DATA_INDEX_outTemp] += (float)arcRecord.outsideTemp/10.0;
                store->values[DATA_INDEX_outHumidity] += (float)arcRecord.outsideHum/10.0;
                store->values[DATA_INDEX_inTemp] += (float)arcRecord.insideTemp/10.0;
                store->values[DATA_INDEX_inHumidity] += (float)arcRecord.insideHum/10.0;
                store->values[DATA_INDEX_dewpoint] +=
                    wvutilsCalculateDewpoint ((float)arcRecord.outsideTemp/10.0,
                                              (float)arcRecord.outsideHum/10.0);
                store->values[DATA_INDEX_windSpeed] += (float)arcRecord.windSpeed/10.0;
                if (arcRecord.windDirection < 16)
                {
                    lastWDIR = arcRecord.windDirection;
                }
                windAverageAddValue (windId, lastWDIR);

                store->values[DATA_INDEX_windGust] += (float)arcRecord.hiWindSpeed/10.0;
                store->values[DATA_INDEX_barometer] += (float)arcRecord.barometer/1000.0;
                store->values[DATA_INDEX_rain] += (float)(arcRecord.rain & 0xFFF)/click;
                store->values[DATA_INDEX_windchill] +=
                    wvutilsCalculateWindChill ((float)arcRecord.outsideTemp/10.0,
                                               (float)arcRecord.windSpeed/10.0);
                store->values[DATA_INDEX_heatindex] +=
                    wvutilsCalculateHeatIndex ((float)arcRecord.outsideTemp/10.0,
                                               (float)arcRecord.outsideHum/10.0);


                if ((uint16_t)arcRecord.solarRad != 0x7FFF && (uint16_t)arcRecord.solarRad != 0xFFFF && (float)arcRecord.solarRad >= 0 && (float)arcRecord.solarRad <= 1800)
                    store->values[DATA_INDEX_radiation] += (float)arcRecord.solarRad;
                if (arcRecord.UV != 0xFF)
                    store->values[DATA_INDEX_UV] += (float)arcRecord.UV/10.0;
                if (arcRecord.ET != 0xFF)
                    store->values[DATA_INDEX_ET] += (float)arcRecord.ET/1000.0;
                if (arcRecord.leafTemp[0] != 0xFF)
                    store->values[DATA_INDEX_leafTemp1] += (float)arcRecord.leafTemp[0] - 90.0;
                if (arcRecord.leafTemp[1] != 0xFF)
                    store->values[DATA_INDEX_leafTemp2] += (float)arcRecord.leafTemp[1] - 90.0;
                if (arcRecord.leafWetness[0] != 0xFF)
                    store->values[DATA_INDEX_leafWet1] += (float)arcRecord.leafWetness[0];
                if (arcRecord.leafWetness[1] != 0xFF)
                    store->values[DATA_INDEX_leafWet2] += (float)arcRecord.leafWetness[1];
                if (arcRecord.soilTemp[0] != 0xFF)
                    store->values[DATA_INDEX_soilTemp1] += (float)arcRecord.soilTemp[0] - 90.0;
                if (arcRecord.soilTemp[1] != 0xFF)
                    store->values[DATA_INDEX_soilTemp2] += (float)arcRecord.soilTemp[1] - 90.0;
                if (arcRecord.soilTemp[2] != 0xFF)
                    store->values[DATA_INDEX_soilTemp3] += (float)arcRecord.soilTemp[2] - 90.0;
                if (arcRecord.soilTemp[3] != 0xFF)
                    store->values[DATA_INDEX_soilTemp4] += (float)arcRecord.soilTemp[3] - 90.0;
                if (arcRecord.extraHum[0] != 0xFF)
                    store->values[DATA_INDEX_extraHumid1] += (float)arcRecord.extraHum[0];
                if (arcRecord.extraHum[1] != 0xFF)
                    store->values[DATA_INDEX_extraHumid2] += (float)arcRecord.extraHum[1];
                if (arcRecord.extraTemp[0] != 0xFF)
                    store->values[DATA_INDEX_extraTemp1] += (float)arcRecord.extraTemp[0] - 90;
                if (arcRecord.extraTemp[1] != 0xFF)
                    store->values[DATA_INDEX_extraTemp2] += (float)arcRecord.extraTemp[1] - 90;
                if (arcRecord.extraTemp[2] != 0xFF)
                    store->values[DATA_INDEX_extraTemp3] += (float)arcRecord.extraTemp[2] - 90;
                if (arcRecord.soilMoisture[0] != 0xFF)
                    store->values[DATA_INDEX_soilMoist1] += (float)arcRecord.soilMoisture[0];
                if (arcRecord.soilMoisture[1] != 0xFF)
                    store->values[DATA_INDEX_soilMoist2] += (float)arcRecord.soilMoisture[1];
                if (arcRecord.soilMoisture[2] != 0xFF)
                    store->values[DATA_INDEX_soilMoist3] += (float)arcRecord.soilMoisture[2];
                if (arcRecord.soilMoisture[3] != 0xFF)
                    store->values[DATA_INDEX_soilMoist4] += (float)arcRecord.soilMoisture[3];
            }
        }
    }

    fclose (infile);
    return mins;
}

static int computeNoaaDay
(
    char            *startName,
    int             day,
    NOAA_DAY_REC    *store
)
{
    int                 j, numrecs = 0;
    float               sum;
    FILE                *infile;
    char                temp[512];
    HeaderBlock         fileHdr;
    DailySummaryRecord  sumRecord;
    ArchiveRecord       arcRecord;
    WAVG                windAvg;
    float               click;

    infile = fopen (startName, "r");
    if (infile == NULL)
    {
        return ERROR;
    }

    if (fread (&fileHdr, sizeof(fileHdr), 1, infile) != 1)
    {
        fclose (infile);
        return ERROR;
    }

    if (fileHdr.dayIndex[day].recordsInDay == 0)
    {
        // no entry for this day
        fclose (infile);
        return ERROR;
    }

    //  ... goto the beginning of the archive recs for this day
    if (fseek (infile,
               sizeof(fileHdr) +
               DBFILES_RECORD_SIZE*(fileHdr.dayIndex[day].startPos + 2),
               SEEK_SET)
            == -1)
    {
        fclose (infile);
        return ERROR;
    }

    windAverageReset (&windAvg);

    //  ... set some values for first time through
    store->highTemp = -10000.0;
    store->lowTemp  = 10000.0;
    store->highWind = -1000.0;

    for (j = 0; j < fileHdr.dayIndex[day].recordsInDay - 2; j ++)
    {
        // read the archive record
        if (fread (&arcRecord, sizeof(arcRecord), 1, infile) != 1)
        {
            fclose (infile);
            return ERROR;
        }

        if (arcRecord.windSpeed/10 > 200)
        {
            continue;
        }
        if ((uint16_t)arcRecord.outsideTemp == 0x8000 ||
                (uint16_t)arcRecord.outsideTemp == 0x7FFF ||
                (uint16_t)arcRecord.hiOutsideTemp == 0x8000 ||
                (uint16_t)arcRecord.hiOutsideTemp == 0x7FFF ||
                (uint16_t)arcRecord.lowOutsideTemp == 0x8000 ||
                (uint16_t)arcRecord.lowOutsideTemp == 0x7FFF)
        {
            continue;
        }

        numrecs ++;


        if ((arcRecord.rain & 0xF000) == 0x0000)        // 0.1 in 0x0000
            click = 10;
        else if ((arcRecord.rain & 0xF000) == 0x2000)   // 0.2 mm 0x2000
            click = 127;
        else if ((arcRecord.rain & 0xF000) == 0x3000)   // 1.0 mm 0x3000
            click = 25.4;
        else if ((arcRecord.rain & 0xF000) == 0x6000)   // 0.1 mm 0x6000 (not fully supported)
            click = 254;
        else                                            //0.01 in 0x1000
            click = 100;

        store->meanTemp         += (float)arcRecord.outsideTemp;
        if (store->highTemp < (float)arcRecord.hiOutsideTemp)
        {
            store->highTemp     = (float)arcRecord.hiOutsideTemp;
            sprintf (store->highTempTime, "%2.2d:%2.2d",
                     arcRecord.packedTime/60,
                     arcRecord.packedTime%60);
        }
        if (store->lowTemp > (float)arcRecord.lowOutsideTemp)
        {
            store->lowTemp      = (float)arcRecord.lowOutsideTemp;
            sprintf (store->lowTempTime, "%2.2d:%2.2d",
                     arcRecord.packedTime/60,
                     arcRecord.packedTime%60);
        }

        sum = (store->highTemp + store->lowTemp)/20.0;
        store->heatDegDays      = ((sum < 65) ? 65 - sum : 0);
        store->coolDegDays      = ((sum > 65) ? sum - 65 : 0);

        store->rain             += (float)(arcRecord.rain & 0xFFF)/click;
        store->avgWind          += (float)arcRecord.windSpeed;
        if (store->highWind < (float)arcRecord.hiWindSpeed)
        {
            store->highWind     = (float)arcRecord.hiWindSpeed;
            sprintf (store->highWindTime, "%2.2d:%2.2d",
                     arcRecord.packedTime/60,
                     arcRecord.packedTime%60);
        }

        if (arcRecord.windDirection < 16)
        {
            windAverageAddValue (&windAvg, arcRecord.windDirection);
        }
    }

    //  ... compute the means necessary
    store->meanTemp     /= (numrecs * 10.0);
    store->avgWind      /= (numrecs * 10.0);
    store->lowTemp      /= 10.0;
    store->highTemp     /= 10.0;
    store->highWind     /= 10.0;
    store->domWindDir   = (int)(windAverageCompute(&windAvg)/22.5);

    fclose (infile);
    return OK;
}
#endif

static int getNewestDateTime
(
    char            *startName,
    int             year,
    int             month,
    uint16_t        *date,
    uint16_t        *time,
    ArchiveRecord   *newRec
)
{
    int             i, j;
    FILE            *infile;
    char            temp[512];

    infile = fopen (startName, "r");
    if (infile == NULL)
    {
        return ERROR;
    }

    if (fread (&fileHdr, sizeof(fileHdr), 1, infile) != 1)
    {
        fclose (infile);
        return ERROR;
    }

    for (i = 1; i < 32; i ++)
    {
        if (fileHdr.dayIndex[i].recordsInDay == 0)
        {
            // we're done with this day
            continue;
        }

        //  ... goto the beginning of the archive recs for this day
        if (fseek (infile,
                   sizeof(fileHdr) +
                   DBFILES_RECORD_SIZE*(fileHdr.dayIndex[i].startPos + 2),
                   SEEK_SET)
                == -1)
        {
            fclose (infile);
            return ERROR;
        }

        for (j = 0; j < fileHdr.dayIndex[i].recordsInDay - 2; j ++)
        {
            // read the archive record
            if (fread (&arcRecord, sizeof(arcRecord), 1, infile) != 1)
            {
                fclose (infile);
                return ERROR;
            }

            *date = i + (32*month) + ((year-2000)*512);
            *time = ((arcRecord.packedTime/60)*100) + (arcRecord.packedTime%60);
            *newRec = arcRecord;
        }
    }

    fclose (infile);
    return OK;
}

static int getNextRecord
(
    char            *startName,
    int             year,
    int             month,
    uint16_t        *date,
    uint16_t        *atime,
    ArchiveRecord   *newRec
)
{
    int             i, j, day, hour, minute;
    FILE            *infile;
    char            temp[512];
    time_t          ntime;
    struct tm       tmtime;

    infile = fopen (startName, "r");
    if (infile == NULL)
    {
        return ERROR;
    }

    if (fread (&fileHdr, sizeof(fileHdr), 1, infile) != 1)
    {
        fclose (infile);
        return ERROR;
    }

    day = EXTRACT_PACKED_DAY(*date);
    hour = EXTRACT_PACKED_HOUR(*atime);
    minute = EXTRACT_PACKED_MINUTE(*atime);
    *date = *atime = 0;

    for (i = day; i < 32; i ++)
    {
        if (fileHdr.dayIndex[i].recordsInDay == 0)
        {
            // we're done with this day
            continue;
        }

        //  ... goto the beginning of the archive recs for this day
        if (fseek (infile,
                   sizeof(fileHdr) +
                   DBFILES_RECORD_SIZE*(fileHdr.dayIndex[i].startPos + 2),
                   SEEK_SET)
                == -1)
        {
            fclose (infile);
            return ERROR;
        }

        for (j = 0; j < fileHdr.dayIndex[i].recordsInDay - 2; j ++)
        {
            // read the archive record
            if (fread (&arcRecord, sizeof(arcRecord), 1, infile) != 1)
            {
                fclose (infile);
                return ERROR;
            }

            if (i == day)
            {
                if ((arcRecord.packedTime/60) < hour)
                {
                    continue;
                }
                else if (((arcRecord.packedTime/60) == hour) &&
                         ((arcRecord.packedTime%60) <= minute))
                {
                    continue;
                }
            }

            // we have a winner - set the proper date and time
            if (arcRecord.packedTime >= (60 * 24))
            {
                // joy, 24:00 VP madness - bump the day forward
                memset (&tmtime, 0, sizeof (tmtime));
                tmtime.tm_year = year - 1900;
                tmtime.tm_mon = month - 1;
                tmtime.tm_mday = i;
                tmtime.tm_min = 5;              // avoid leap second worries
                tmtime.tm_isdst = -1;
                ntime = mktime (&tmtime);
                ntime += (WV_SECONDS_IN_DAY + 4*WV_SECONDS_IN_HOUR); // Avoid DST fallback
                localtime_r (&ntime, &tmtime);
                year = tmtime.tm_year + 1900;
                month = tmtime.tm_mon + 1;
                day = tmtime.tm_mday;
                hour = 0;
                minute = 0;
            }
            else
            {
                day = i;
                hour = arcRecord.packedTime/60;
                minute = arcRecord.packedTime%60;
            }

            *date = INSERT_PACKED_DATE(year,month,day);
            *atime = (hour*100) + minute;
            *newRec = arcRecord;
            fclose (infile);
            return OK;
        }
    }

    fclose (infile);
    return ERROR;
}


//  ... #########  API Functions #########

char *dbfBuildWindDirString (int dir)
{
    if (dir < 0 || dir > 15)
    {
        return "---";
    }
    else
    {
        return winddir[dir];
    }
}


#if defined(BUILD_HTMLGEND)
//  ... calculate averages over a given period of time
//  ... (given in arcInterval minute samples);
//  ... this will zero out the HISTORY_DATA store before beginning
//  ... the processing;
//  ... returns OK or ERROR

int dbfGetAverages
(
    int             isMetricUnits,
    int             isMetricMM,
    int             arcInterval,
    char            *archivePath,
    HISTORY_DATA    *store,
    int             startmin,
    int             starthour,
    int             startday,
    int             startmonth,
    int             startyear,
    int             numSamples
)
{
    char            startName[256];
    uint16_t        fileDate;
    struct stat     fileStatus;
    int             retVal, year, month;
    int             curyear, curmonth;
    int             day = startday, hour = starthour, mins = startmin;
    int             numMins = numSamples * arcInterval;
    int             minsDone = 0, done = FALSE;
    time_t          ntime;
    struct tm       locTime;
    WAVG            windAvg;


    ntime = time (NULL);
    localtime_r (&ntime, &locTime);
    curmonth = locTime.tm_mon + 1;
    curyear = locTime.tm_year + 1900;

    //  ... build the file names
    fileDate = ((startyear - 2000) * 100) + startmonth;
    buildArchiveFileName (archivePath, fileDate, startName, &year, &month);


    memset (store, 0, sizeof (HISTORY_DATA));

    store->startYear    = startyear;
    store->startMonth   = startmonth;
    store->startDay     = startday;
    store->startHour    = starthour;
    store->startMin     = startmin;


    //  ... create the wind average object
    windAverageReset (&windAvg);

    while (!done)
    {
        if ((year > curyear) || (year == curyear && month > curmonth))
        {
            // we're done!
            done = TRUE;
            continue;
        }

        if (stat (startName, &fileStatus) == -1)
        {
            return ERROR;
        }

        //  ... if we are here, we have a file to work with
        retVal = rollIntoAverages (isMetricUnits,
                                   isMetricMM,
                                   &windAvg,
                                   startName,
                                   day,
                                   hour,
                                   mins,
                                   store,
                                   numMins - minsDone);
        if (retVal == ERROR)
        {
            return ERROR;
        }

        day = 1;
        hour = 0;
        mins = 0;
        minsDone += retVal;
        if (minsDone >= numMins)
        {
            // we're done!
            done = TRUE;
            continue;
        }

        incrementArchiveFileName (startName, startName, &year, &month);
    }

    store->values[DATA_INDEX_windDir] = windAverageCompute (&windAvg);

    return OK;
}

//  ... dbfGetNoaaDay: summarize day records for NOAA reports;
//  ... this will zero out the NOAA_DAY_REC store before beginning;
//  ... returns OK or ERROR if day not found in archives

int dbfGetNoaaDay
(
    char            *archivePath,
    NOAA_DAY_REC    *store,
    int             startyear,
    int             startmonth,
    int             startday
)
{
    char            startName[256];
    uint16_t        fileDate;
    struct stat     fileStatus;
    int             retVal, year, month;
    int             curyear, curmonth;
    time_t          ntime;
    struct tm       locTime;

    ntime = time (NULL);
    localtime_r (&ntime, &locTime);
    curmonth = locTime.tm_mon + 1;
    curyear = locTime.tm_year + 1900;

    if (startyear > curyear ||
            (startyear == curyear && startmonth > curmonth))
    {
        return ERROR;
    }

    //  ... build the file names
    fileDate = ((startyear - 2000) * 100) + startmonth;
    buildArchiveFileName (archivePath, fileDate, startName, &year, &month);

    //  ... MUST be done here - computeNoaaDay expects it!
    memset (store, 0, sizeof (NOAA_DAY_REC));

    if (stat (startName, &fileStatus) == -1)
    {
        return ERROR;
    }

    store->year     = startyear;
    store->month    = startmonth;
    store->day      = startday;

    //  ... if we are here, we have a file to work with
    return (computeNoaaDay (startName, startday, store));
}

// write out all ASCII archive records for the given day to 'filename'
int dbfWriteDailyArchiveReport
(
    char            *archivePath,
    char            *filename,
    time_t          timeval,
    int             isMetric,
    int             isMetricMM,
    int             arcInterval,
    void            (*writeHeader) (FILE *file)
)
{
    int             i, recsWritten = ERROR;
    char            temp[256];
    HISTORY_DATA    data;
    struct stat     fileStatus;
    int             retVal, year, month, day, hour, minute;
    int             dummyYear, dummyMonth;
    uint16_t        start;
    struct tm       locTime;

    localtime_r (&timeval, &locTime);
    year    = locTime.tm_year + 1900;
    month   = locTime.tm_mon + 1;
    day     = locTime.tm_mday;

    if (stat (filename, &fileStatus) != -1)
    {
        //  exists, delete it
        unlink (filename);
    }

    // is there even an archive file for this month?
    start = ((year-2000)*100) + month;
    buildArchiveFileName (archivePath,
                          start,
                          temp,
                          &dummyYear,
                          &dummyMonth);

    if (stat (temp, &fileStatus) == -1)
    {
        // no records - no month!
        return ERROR;
    }


    // loop through the day an archive interval at a time
    hour = minute = 0;
    for (i = arcInterval; i <= WV_MINUTES_IN_DAY; i += arcInterval)
    {
        hour   = i / 60;
        minute = i % 60;

        retVal = dbfGetAverages (isMetric,
                                 isMetricMM,
                                 arcInterval,
                                 archivePath,
                                 &data,
                                 minute,
                                 hour,
                                 day,
                                 month,
                                 year,
                                 1);

        if (retVal == ERROR)
        {
            continue;
        }
        else
        {
            recsWritten = OK;
        }

        // call our little updater
        dbfUpdateDailyArchiveReport (filename, &data, writeHeader, isMetric);
    }

    return recsWritten;
}

// update (or create) the current day's ASCII archive records file
int dbfUpdateDailyArchiveReport
(
    char            *file,
    HISTORY_DATA    *data,
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
                 data->startYear,
                 data->startMonth,
                 data->startDay,
                 data->startHour,
                 data->startMin,
                 data->values[DATA_INDEX_outTemp],
                 data->values[DATA_INDEX_windchill],
                 data->values[DATA_INDEX_heatindex],
                 data->values[DATA_INDEX_outHumidity],
                 data->values[DATA_INDEX_dewpoint],
                 data->values[DATA_INDEX_windSpeed],
                 data->values[DATA_INDEX_windGust],
                 data->values[DATA_INDEX_windDir],
                 data->values[DATA_INDEX_rain],
                 data->values[DATA_INDEX_barometer],
                 data->values[DATA_INDEX_radiation],
                 data->values[DATA_INDEX_ET],
                 data->values[DATA_INDEX_UV]);
    }
    else
    {
        //    "--Timestamp---\tTemp\tChill\tHIndex\tHumid\tDewpt\tWind\tHiWind\tWindDir\tRain\tBarom\tSolar\tET\tUV\n"
        sprintf (temp,
                 "%4.4d%2.2d%2.2d %2.2d:%2.2d\t%.1f\t%.1f\t%.1f\t%.0f\t%.1f\t%.0f\t%.0f\t%.0f\t%.1f\t%.1f\t%.0f\t%.3f\t%.1f\n",
                 data->startYear,
                 data->startMonth,
                 data->startDay,
                 data->startHour,
                 data->startMin,
                 data->values[DATA_INDEX_outTemp],
                 data->values[DATA_INDEX_windchill],
                 data->values[DATA_INDEX_heatindex],
                 data->values[DATA_INDEX_outHumidity],
                 data->values[DATA_INDEX_dewpoint],
                 data->values[DATA_INDEX_windSpeed],
                 data->values[DATA_INDEX_windGust],
                 data->values[DATA_INDEX_windDir],
                 data->values[DATA_INDEX_rain],
                 data->values[DATA_INDEX_barometer],
                 data->values[DATA_INDEX_radiation],
                 data->values[DATA_INDEX_ET],
                 data->values[DATA_INDEX_UV]);
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


//  ... append an archive record to the appropriate dbfile;
//  ... if the date extends into a new month, this will create the new dbfile
//  ... and insert this archive record;
//  ... returns OK or ERROR

int dbfStoreArchiveRecord
(
    char            *archivePath,
    ARCHIVE_RECORD  *record,
    int             archiveInterval,
    uint16_t        RainCollectorType
)
{
    int             year, month, day, hour, minute, dayRecs, dayMinutes;
    int             sqlyear, sqlmonth, sqlday;
    char            arcFileName[256];
    FILE            *arcFile;
    struct stat     fileStatus;
    HeaderBlock     hdrblk;
    int16_t         wind;
    float           chill, dew, heat;
    struct tm       bknTime;
    time_t          secTime;

    // check the archive record version
    if (record->recordType == 0xFF)
    {
        // we have an 'A' record here - convert to 'B' before proceeding
        convertArchiveAtoB (record);
    }

    year = sqlyear = (record->date >> 9) + 2000;
    month = sqlmonth = (record->date >> 5) & 0xf;
    day = sqlday = record->date & 0x1f;
    hour = record->time/100;
    minute = record->time%100;

    if (minute == 0 && hour == 0)
    {
        memset (&bknTime, 0, sizeof (bknTime));

        bknTime.tm_mday  = day;
        bknTime.tm_mon   = month-1;
        bknTime.tm_year  = year-1900;
        bknTime.tm_min   = 5;                // to avoid midnight confusion
        bknTime.tm_isdst = -1;
        secTime = mktime (&bknTime);
        secTime -= WV_SECONDS_IN_DAY;        // subtract a day
        localtime_r (&secTime, &bknTime);

        hour    = 24;
        year    = bknTime.tm_year + 1900;
        month   = bknTime.tm_mon + 1;
        day     = bknTime.tm_mday;
    }

    dayMinutes = minute + 60*hour;

    if (record->avgWindSpeed > 200 || record->prevWindDir > 15)
    {
        record->avgWindSpeed = 0;
        record->prevWindDir = 255;
    }

    if (record->highWindSpeed > 200 || record->highWindDir > 15)
    {
        record->highWindSpeed = 0;
        record->highWindDir = 255;
    }

    chill = wvutilsCalculateWindChill ((float)record->outTemp/10,
                                       (float)record->avgWindSpeed);
    dew = wvutilsCalculateDewpoint ((float)record->outTemp/10,
                                    (float)record->outHumidity);
    heat = wvutilsCalculateHeatIndex ((float)record->outTemp/10,
                                      (float)record->outHumidity);

    sprintf (arcFileName, "%s/%4.4d-%2.2d.wlk", archivePath, year, month);

    if (stat (arcFileName, &fileStatus) == -1)
    {
        //  ... archive does not exist, create it
        arcFile = fopen (arcFileName, "w");
        if (arcFile == NULL)
        {
            return ERROR;
        }

        memset (&hdrblk, 0, sizeof (hdrblk));
        memcpy (hdrblk.idCode, idCode, sizeof (idCode));

        if (fwrite (&hdrblk, sizeof (hdrblk), 1, arcFile) != 1)
        {
            fclose (arcFile);
            return ERROR;
        }

        fclose (arcFile);
    }

    //  ... now open up the file and read some hdrs
    arcFile = fopen (arcFileName, "r");
    if (arcFile == NULL)
    {
        return ERROR;
    }
    if (fseek (arcFile, 0, SEEK_SET) == -1)
    {
        fclose (arcFile);
        return ERROR;
    }

    if (fread (&hdrblk, sizeof (hdrblk), 1, arcFile) != 1)
    {
        fclose (arcFile);
        return ERROR;
    }

    if (hdrblk.dayIndex[day].recordsInDay != 0)
    {
        if (fseek (arcFile,
                   sizeof(hdrblk) +
                   DBFILES_RECORD_SIZE * hdrblk.dayIndex[day].startPos,
                   SEEK_SET)
                == -1)
        {
            fclose (arcFile);
            return ERROR;
        }

        if (fread (&sumRecord, sizeof (sumRecord), 1, arcFile) != 1)
        {
            fclose (arcFile);
            return ERROR;
        }
    }
    else
    {
        memset (&sumRecord, 0, sizeof (sumRecord));
        sumRecord.dataType1 = 0;
        sumRecord.dataSpan = archiveInterval;
        sumRecord.hiOutTemp = record->highOutTemp;
        insertTimeValue (sumRecord.timeValues1,
                         High_Outside_Temperature,
                         dayMinutes);
        sumRecord.lowOutTemp = record->lowOutTemp;
        insertTimeValue (sumRecord.timeValues1,
                         Low_Outside_Temperature,
                         dayMinutes);
        sumRecord.hiInTemp = record->inTemp;
        insertTimeValue (sumRecord.timeValues1,
                         High_Inside_Temperature,
                         dayMinutes);
        sumRecord.lowInTemp = record->inTemp;
        insertTimeValue (sumRecord.timeValues1,
                         Low_Inside_Temperature,
                         dayMinutes);
        sumRecord.avgOutTemp = record->outTemp;
        sumRecord.avgInTemp = record->inTemp;
        sumRecord.hiChill = (int)(chill*10);
        insertTimeValue (sumRecord.timeValues1,
                         High_Wind_Chill,
                         dayMinutes);
        sumRecord.lowChill = sumRecord.hiChill;
        insertTimeValue (sumRecord.timeValues1,
                         Low_Wind_Chill,
                         dayMinutes);
        sumRecord.hiDew = (int)(dew*10);
        insertTimeValue (sumRecord.timeValues1,
                         High_Dew_Point,
                         dayMinutes);
        sumRecord.lowDew = sumRecord.hiDew;
        insertTimeValue (sumRecord.timeValues1,
                         Low_Dew_Point,
                         dayMinutes);
        sumRecord.avgChill = sumRecord.hiChill;
        sumRecord.avgDew = sumRecord.hiDew;
        sumRecord.hiOutHum = record->outHumidity*10;
        insertTimeValue (sumRecord.timeValues1,
                         High_Outside_Humidity,
                         dayMinutes);
        sumRecord.lowOutHum = record->outHumidity*10;
        insertTimeValue (sumRecord.timeValues1,
                         Low_Outside_Humidity,
                         dayMinutes);
        sumRecord.hiInHum = record->inHumidity*10;
        insertTimeValue (sumRecord.timeValues1,
                         High_Inside_Humidity,
                         dayMinutes);
        sumRecord.lowInHum = record->inHumidity*10;
        insertTimeValue (sumRecord.timeValues1,
                         Low_Inside_Humidity,
                         dayMinutes);
        sumRecord.avgOutHum = record->outHumidity*10;
        sumRecord.hiBar = record->barometer;
        insertTimeValue (sumRecord.timeValues1,
                         High_Barometer,
                         dayMinutes);
        sumRecord.lowBar = record->barometer;
        insertTimeValue (sumRecord.timeValues1,
                         Low_Barometer,
                         dayMinutes);
        sumRecord.avgBar = record->barometer;
        sumRecord.hiSpeed = record->highWindSpeed;
        insertTimeValue (sumRecord.timeValues1,
                         High_Wind_Speed,
                         dayMinutes);
        insertTimeValue (sumRecord.dirBins,
                         record->prevWindDir,
                         archiveInterval);
        sumRecord.avgSpeed = record->avgWindSpeed*10;
        sumRecord.dailyWindRunTotal = 0x8000;
        sumRecord.hi10MinSpeed = 0x8000;
        sumRecord.dirHiSpeed = record->highWindDir;
        sumRecord.hi10MinDir = 0xff;
        sumRecord.dailyRainTotal = record->rain;
        sumRecord.hiRainRate = record->highRainRate;
        insertTimeValue (sumRecord.timeValues1,
                         High_Rain_Rate,
                         dayMinutes);
        sumRecord.dailyUVDose = 0x8000;
        sumRecord.hiUV = record->highUV;
        sumRecord.dataType2 = 3;
        sumRecord.todaysWeather = 0;
        sumRecord.numWindPackets = 0x8000;
        sumRecord.hiSolar = record->highRadiation;
        sumRecord.dailySolarEnergy = 0x8000;
        sumRecord.minSunlight = 0x8000;
        sumRecord.dailyETTotal = 0;
        sumRecord.hiHeat = (int)(heat*10);
        insertTimeValue (sumRecord.timeValues1,
                         High_Outside_Heat_Index,
                         dayMinutes);
        sumRecord.lowHeat = sumRecord.hiHeat;
        insertTimeValue (sumRecord.timeValues1,
                         Low_Outside_Heat_Index,
                         dayMinutes);
        sumRecord.avgHeat = sumRecord.hiHeat;
    }

    //  ... we're done reading, let's do some writin'
    fclose (arcFile);

    arcFile = fopen (arcFileName, "r+");
    if (arcFile == NULL)
    {
        return ERROR;
    }

    if (fseek (arcFile, 0, SEEK_SET) == -1)
    {
        fclose (arcFile);
        return ERROR;
    }

    if (sumRecord.dataType1 == 0)
    {
        hdrblk.dayIndex[day].startPos = hdrblk.totalRecords;
        hdrblk.totalRecords += 3;
        hdrblk.dayIndex[day].recordsInDay += 3;
        sumRecord.dataSpan = archiveInterval;
    }

    //  ... roll in this archive record's data
    if (sumRecord.dataType1 != 0)
    {
        hdrblk.totalRecords += 1;
        hdrblk.dayIndex[day].recordsInDay += 1;
        sumRecord.dataSpan += archiveInterval;
        dayRecs = hdrblk.dayIndex[day].recordsInDay - 1;    // add one for record we're adding

        if (record->highOutTemp > sumRecord.hiOutTemp)
        {
            sumRecord.hiOutTemp = record->highOutTemp;
            insertTimeValue (sumRecord.timeValues1,
                             High_Outside_Temperature,
                             dayMinutes);
        }
        if (record->lowOutTemp < sumRecord.lowOutTemp)
        {
            sumRecord.lowOutTemp = record->lowOutTemp;
            insertTimeValue (sumRecord.timeValues1,
                             Low_Outside_Temperature,
                             dayMinutes);
        }
        if (record->inTemp > sumRecord.hiInTemp)
        {
            sumRecord.hiInTemp = record->inTemp;
            insertTimeValue (sumRecord.timeValues1,
                             High_Inside_Temperature,
                             dayMinutes);
        }
        if (record->inTemp < sumRecord.lowInTemp)
        {
            sumRecord.lowInTemp = record->inTemp;
            insertTimeValue (sumRecord.timeValues1,
                             Low_Inside_Temperature,
                             dayMinutes);
        }
        sumRecord.avgOutTemp = (((dayRecs-1)*sumRecord.avgOutTemp)+record->outTemp)/dayRecs;
        sumRecord.avgInTemp = (((dayRecs-1)*sumRecord.avgInTemp)+record->inTemp)/dayRecs;
        if ((int)(chill*10) > sumRecord.hiChill)
        {
            sumRecord.hiChill = (int)(chill*10);
            insertTimeValue (sumRecord.timeValues1,
                             High_Wind_Chill,
                             dayMinutes);
        }
        if ((int)(chill*10) < sumRecord.lowChill)
        {
            sumRecord.lowChill = (int)(chill*10);
            insertTimeValue (sumRecord.timeValues1,
                             Low_Wind_Chill,
                             dayMinutes);
        }
        if ((int)(dew*10) > sumRecord.hiDew)
        {
            sumRecord.hiDew = (int)(dew*10);
            insertTimeValue (sumRecord.timeValues1,
                             High_Dew_Point,
                             dayMinutes);
        }
        if ((int)(dew*10) < sumRecord.lowDew)
        {
            sumRecord.lowDew = (int)(dew*10);
            insertTimeValue (sumRecord.timeValues1,
                             Low_Dew_Point,
                             dayMinutes);
        }
        sumRecord.avgChill = (((dayRecs-1)*sumRecord.avgChill)+(int)(chill*10))/dayRecs;
        sumRecord.avgDew = (((dayRecs-1)*sumRecord.avgDew)+(int)(dew*10))/dayRecs;
        if (record->outHumidity*10 > sumRecord.hiOutHum)
        {
            sumRecord.hiOutHum = record->outHumidity*10;
            insertTimeValue (sumRecord.timeValues1,
                             High_Outside_Humidity,
                             dayMinutes);
        }
        if (record->outHumidity*10 < sumRecord.lowOutHum)
        {
            sumRecord.lowOutHum = record->outHumidity*10;
            insertTimeValue (sumRecord.timeValues1,
                             Low_Outside_Humidity,
                             dayMinutes);
        }
        if (record->inHumidity*10 > sumRecord.hiInHum)
        {
            sumRecord.hiInHum = record->inHumidity*10;
            insertTimeValue (sumRecord.timeValues1,
                             High_Inside_Humidity,
                             dayMinutes);
        }
        if (record->inHumidity*10 < sumRecord.lowInHum)
        {
            sumRecord.lowInHum = record->inHumidity*10;
            insertTimeValue (sumRecord.timeValues1,
                             Low_Inside_Humidity,
                             dayMinutes);
        }
        sumRecord.avgOutHum = (((dayRecs-1)*sumRecord.avgOutHum)+record->outHumidity*10)/dayRecs;
        if (record->barometer > sumRecord.hiBar)
        {
            sumRecord.hiBar = record->barometer;
            insertTimeValue (sumRecord.timeValues1,
                             High_Barometer,
                             dayMinutes);
        }
        if (record->barometer < sumRecord.lowBar)
        {
            sumRecord.lowBar = record->barometer;
            insertTimeValue (sumRecord.timeValues1,
                             Low_Barometer,
                             dayMinutes);
        }
        sumRecord.avgBar = (((dayRecs-1)*sumRecord.avgBar)+record->barometer)/dayRecs;
        if (record->highUV > sumRecord.hiUV)
        {
            sumRecord.hiUV = record->highUV;
            insertTimeValue (sumRecord.timeValues1,
                             High_UV,
                             dayMinutes);
        }

        if (record->highRadiation > sumRecord.hiSolar && (float)record->highRadiation >=0 && (float)record->highRadiation <=1800)
        {
            sumRecord.hiSolar = record->highRadiation;
            insertTimeValue (sumRecord.timeValues2,
                             High_Solar_Rad,
                             dayMinutes);
        }
        if (record->highWindSpeed*10 > sumRecord.hiSpeed)
        {
            sumRecord.hiSpeed = record->highWindSpeed*10;
            insertTimeValue (sumRecord.timeValues1,
                             High_Wind_Speed,
                             dayMinutes);
        }

        sumRecord.avgSpeed = (((dayRecs-1)*sumRecord.avgSpeed)+record->avgWindSpeed*10)/dayRecs;
        sumRecord.dailyRainTotal += record->rain;
        if (record->highRainRate > sumRecord.hiRainRate)
        {
            sumRecord.hiRainRate = record->highRainRate;
            insertTimeValue (sumRecord.timeValues1,
                             High_Rain_Rate,
                             dayMinutes);
        }
        if ((int)(heat*10) > sumRecord.hiHeat)
        {
            sumRecord.hiHeat = (int)(heat*10);
            insertTimeValue (sumRecord.timeValues2,
                             High_Outside_Heat_Index,
                             dayMinutes);
        }
        if ((int)(heat*10) < sumRecord.lowHeat)
        {
            sumRecord.lowHeat = (int)(heat*10);
            insertTimeValue (sumRecord.timeValues2,
                             Low_Outside_Heat_Index,
                             dayMinutes);
        }
        sumRecord.avgHeat = (((dayRecs-1)*sumRecord.avgHeat)+(int)(heat*10))/dayRecs;

        wind = extractTimeValue (sumRecord.dirBins, record->prevWindDir);
        wind += archiveInterval;
        insertTimeValue (sumRecord.dirBins,
                         record->prevWindDir,
                         wind);
    }



    memset (&arcRecord, 0xFF, sizeof (arcRecord));
    arcRecord.dataType = 1;
    arcRecord.archiveInterval = archiveInterval;
    arcRecord.packedTime = dayMinutes;
    arcRecord.outsideTemp = record->outTemp;
    arcRecord.hiOutsideTemp = record->highOutTemp;
    arcRecord.lowOutsideTemp = record->lowOutTemp;
    arcRecord.insideTemp = record->inTemp;
    arcRecord.barometer = record->barometer;
    arcRecord.outsideHum = record->outHumidity*10;
    arcRecord.insideHum = record->inHumidity*10;
    arcRecord.rain = record->rain | RainCollectorType;
    arcRecord.hiRainRate = record->highRainRate;
    arcRecord.windSpeed = record->avgWindSpeed*10;
    arcRecord.hiWindSpeed = record->highWindSpeed*10;
    arcRecord.windDirection = record->prevWindDir;
    arcRecord.hiWindDirection = record->highWindDir;
    arcRecord.numWindSamples = record->windSamples;
    arcRecord.solarRad = record->radiation;
    arcRecord.hisolarRad = record->highRadiation;
    arcRecord.UV = record->UV;
    arcRecord.hiUV = record->highUV;
    arcRecord.leafTemp[0] = record->leafTemp1;
    arcRecord.leafTemp[1] = record->leafTemp2;
    arcRecord.forecast = record->fcstRule;
    arcRecord.ET = record->ET;
    arcRecord.soilTemp[0] = record->soilTemp1;
    arcRecord.soilTemp[1] = record->soilTemp2;
    arcRecord.soilTemp[2] = record->soilTemp3;
    arcRecord.soilTemp[3] = record->soilTemp4;
    arcRecord.soilMoisture[0] = record->soilMoist1;
    arcRecord.soilMoisture[1] = record->soilMoist2;
    arcRecord.soilMoisture[2] = record->soilMoist3;
    arcRecord.soilMoisture[3] = record->soilMoist4;
    arcRecord.leafWetness[0] = record->leafWet1;
    arcRecord.leafWetness[1] = record->leafWet2;
    arcRecord.extraTemp[0] = record->extraTemp1;
    arcRecord.extraTemp[1] = record->extraTemp2;
    arcRecord.extraTemp[2] = record->extraTemp3;
    arcRecord.extraHum[0] = record->extraHumid1;
    arcRecord.extraHum[1] = record->extraHumid2;


    //  ... write out the hdr and the new records
    if (fseek (arcFile, 0, SEEK_SET) == -1)
    {
        fclose (arcFile);
        return ERROR;
    }

    if (fwrite (&hdrblk, sizeof(hdrblk), 1, arcFile) != 1)
    {
        fclose (arcFile);
        return ERROR;
    }

    if (fseek (arcFile,
               sizeof(hdrblk) +
               DBFILES_RECORD_SIZE * hdrblk.dayIndex[day].startPos,
               SEEK_SET)
            == -1)
    {
        fclose (arcFile);
        return ERROR;
    }

    sumRecord.dataType1 = 2;
    if (fwrite (&sumRecord, sizeof(sumRecord), 1, arcFile) != 1)
    {
        fclose (arcFile);
        return ERROR;
    }

    if (fseek (arcFile, 0, SEEK_END) == -1)
    {
        fclose (arcFile);
        return ERROR;
    }
    if (fwrite (&arcRecord, sizeof(arcRecord), 1, arcFile) != 1)
    {
        fclose (arcFile);
        return ERROR;
    }

    fclose (arcFile);
    return OK;
}

#ifdef BUILD_WVIEWD

//  ... search the archive path for the most recent archive record date;
//  ... places the most recent date and time in 'date' and 'time' or all
//  ... zero's if no archive record found;
//  ... returns OK or ERROR if no archives found

int dbfGetNewestArchiveTime
(
    char            *archivePath,
    uint16_t        curMonth,
    uint16_t        curYear,
    uint16_t        *date,
    uint16_t        *time,
    ArchiveRecord   *newestRecord
)
{
    char            startName[256], stopName[256];
    struct stat     fileStatus;
    int             year, month, done = FALSE;
    uint16_t        start, stop;

    start = ((curYear-2000)*100) + curMonth;
    stop = ((curYear-2001)*100) + curMonth;             // go back one year

    *date = *time = 0;

    //  ... build the file names
    buildArchiveFileName (archivePath, stop, stopName, &year, &month);
    buildArchiveFileName (archivePath, start, startName, &year, &month);

    while (!done)
    {
        if (strcmp (startName, stopName) < 0)
        {
            // we're done!
            done = TRUE;
            continue;
        }

        if (stat (startName, &fileStatus) == -1)
        {
            decrementArchiveFileName (startName, startName, &year, &month);
            continue;
        }

        //  ... if we are here, we have a file to work with
        getNewestDateTime (startName, year, month, date, time, newestRecord);

        done = TRUE;
    }

    if (*date == 0)
        return ERROR;
    else
        return OK;
}


//  ... dbfCopyArchiveRecordToConsoleRecord - copy an archive file record to
//  ... the station message record format

int dbfCopyArchiveRecordToConsoleRecord
(
    ArchiveRecord       *src,
    ARCHIVE_RECORD      *dest,
    uint16_t            date,
    uint16_t            time
)
{
    memset (dest, 0, sizeof (*dest));

    dest->date = date;
    dest->time = time;
    dest->outTemp = src->outsideTemp;
    dest->highOutTemp = src->hiOutsideTemp;
    dest->lowOutTemp = src->lowOutsideTemp;
    dest->inTemp = src->insideTemp;
    dest->barometer = src->barometer;
    dest->outHumidity = src->outsideHum/10;
    dest->inHumidity = src->insideHum/10;
    dest->rain = src->rain;
    dest->highRainRate = src->hiRainRate;
    dest->avgWindSpeed = src->windSpeed/10;
    dest->highWindSpeed = src->hiWindSpeed/10;
    dest->prevWindDir = src->windDirection;
    dest->highWindDir = src->hiWindDirection;
    dest->windSamples = src->numWindSamples;

    return OK;
}


// dbfIncrementConsoleTime - increment the station time representation
// by 'numMinutes' minutes
int dbfIncrementConsoleTime
(
    ARCHIVE_RECORD      *record,
    int                 numMinutes
)
{
    struct tm           brokenTime, curTime;
    time_t              calTime;

    calTime = time (NULL);
    localtime_r (&calTime, &curTime);

    memset (&brokenTime, 0, sizeof (brokenTime));
    brokenTime.tm_year      = EXTRACT_PACKED_YEAR(record->date) - 1900;
    brokenTime.tm_mon       = EXTRACT_PACKED_MONTH(record->date) - 1;
    brokenTime.tm_mday      = EXTRACT_PACKED_DAY(record->date);
    brokenTime.tm_hour      = EXTRACT_PACKED_HOUR(record->time);
    brokenTime.tm_min       = EXTRACT_PACKED_MINUTE(record->time);
    brokenTime.tm_isdst     = -1;

    calTime = mktime (&brokenTime);

    calTime += (numMinutes * 60);

    localtime_r (&calTime, &brokenTime);

    record->date = brokenTime.tm_mday;
    record->date |= ((brokenTime.tm_mon + 1) << 5);
    record->date |= ((brokenTime.tm_year - 100) << 9);

    record->time = brokenTime.tm_hour * 100;
    record->time += brokenTime.tm_min;

    return OK;
}

#endif


//  ... search the archive path for the next archive record after 'date' and
//  ... 'time';
//  ... places the most recent date and time in 'date' and 'time' or all
//  ... zero's if no archive record found;
//  ... returns OK or ERROR if no archives found

int dbfGetNextArchiveRecord
(
    char            *archivePath,
    uint16_t        *date,
    uint16_t        *atime,
    ArchiveRecord   *recordStore
)
{
    char            startName[256], stopName[256];
    struct stat     fileStatus;
    int             year, month, done = FALSE;
    uint16_t        start, tempDate, tempTime, tempMonth, tempYear, stop;
    time_t          ntime;
    struct tm       tmtime;

    ntime = time (NULL);
    localtime_r (&ntime, &tmtime);
    stop = ((tmtime.tm_year-100)*100) + tmtime.tm_mon + 1;

    tempMonth = MAX(EXTRACT_PACKED_MONTH(*date),1);
    tempYear = MAX(EXTRACT_PACKED_YEAR(*date),2000);
    start = ((tempYear-2000)*100) + tempMonth;

    //  ... build the file names
    buildArchiveFileName (archivePath, stop, stopName, &year, &month);
    buildArchiveFileName (archivePath, start, startName, &year, &month);

    tempDate = *date;
    tempTime = *atime;

    while (!done)
    {
        if (strcmp (startName, stopName) > 0)
        {
            // we're done!
            tempDate = 0;
            done = TRUE;
            continue;
        }

        if (stat (startName, &fileStatus) == -1)
        {
            incrementArchiveFileName (startName, startName, &year, &month);
            tempDate = 1;
            tempTime = 0;
            continue;
        }

        //  ... if we are here, we have a file to work with
        if (getNextRecord (startName, year, month, &tempDate, &tempTime, recordStore)
                == ERROR)
        {
            incrementArchiveFileName (startName, startName, &year, &month);
            tempDate = 1;
            tempTime = 0;
            continue;
        }

        *date = tempDate;
        *atime = tempTime;
        done = TRUE;
    }

    if (tempDate == 0)
        return ERROR;
    else
        return OK;
}


//  ... provide a routine to generate an ASCII browser view file
//  ... 'outputFilename';
//  ... 'start' and 'stop' are of the form: (("year" - 2000) * 100) + "month"
//  ... and are inclusive;
//  ... returns OK or ERROR

int dbfExportArchiveFile
(
    char                *archivePath,
    uint16_t            start,
    uint16_t            stop,
    char                *outputFilename
)
{
    char                startName[256], stopName[256];
    int                 outfilefd;
    struct stat         fileStatus;
    int                 year, month, done = FALSE;

    //  ... create our output file

    outfilefd = open (outputFilename, O_WRONLY | O_CREAT | O_TRUNC, 00755);
    if (outfilefd == -1)
    {
        return ERROR;
    }

    //  ... build the file names
    buildArchiveFileName (archivePath, stop, stopName, &year, &month);
    buildArchiveFileName (archivePath, start, startName, &year, &month);

    while (!done)
    {
        if (strcmp (startName, stopName) > 0)
        {
            // we're done!
            done = TRUE;
            continue;
        }

        if (stat (startName, &fileStatus) == -1)
        {
            incrementArchiveFileName (startName, startName, &year, &month);
            continue;
        }

        //  ... if we are here, we have a file to work with
        if (convertFileToAscii (startName, outfilefd, year, month) == ERROR)
        {
            close (outfilefd);
            return ERROR;
        }

        incrementArchiveFileName (startName, startName, &year, &month);
    }

    close (outfilefd);
    return OK;
}

//  ... provide a routine to generate an ASCII browser view file
//  ... 'outputFilename' containing daily summary records;
//  ... 'start' and 'stop' are of the form: (("year" - 2000) * 100) + "month"
//  ... and are inclusive;
//  ... returns OK or ERROR

int dbfExportDailySummaryFile
(
    char            *archivePath,
    uint16_t        start,
    uint16_t        stop,
    char            *outputFilename
)
{
    char            startName[256], stopName[256];
    int             outfilefd;
    struct stat     fileStatus;
    int             year, month, done = FALSE;

    //  ... create our output file
    outfilefd = open (outputFilename, O_WRONLY | O_CREAT | O_TRUNC, 00755);
    if (outfilefd == -1)
    {
        return ERROR;
    }

    //  ... build the file names
    buildArchiveFileName (archivePath, stop, stopName, &year, &month);
    buildArchiveFileName (archivePath, start, startName, &year, &month);

    while (!done)
    {
        if (strcmp (startName, stopName) > 0)
        {
            // we're done!
            done = TRUE;
            continue;
        }

        if (stat (startName, &fileStatus) == -1)
        {
            incrementArchiveFileName (startName, startName, &year, &month);
            continue;
        }

        //  ... if we are here, we have a file to work with
        if (convertSummaryFileToAscii (startName, outfilefd, year, month) == ERROR)
        {
            close (outfilefd);
            return ERROR;
        }

        incrementArchiveFileName (startName, startName, &year, &month);
    }

    close (outfilefd);
    return OK;
}

// dbfComputePackedDelta - compute the delta in minutes between 2 packed times
int dbfComputePackedDelta
(
    uint16_t        oldDate,
    uint16_t        oldTime,
    uint16_t        newDate,
    uint16_t        newTime
)
{
    struct tm       oldTM, newTM;
    time_t          oldTIME_T, newTIME_T;
    int             retVal;

    memset (&oldTM, 0, sizeof (oldTM));
    oldTM.tm_year      = EXTRACT_PACKED_YEAR(oldDate) - 1900;
    oldTM.tm_mon       = EXTRACT_PACKED_MONTH(oldDate) - 1;
    oldTM.tm_mday      = EXTRACT_PACKED_DAY(oldDate);
    oldTM.tm_hour      = EXTRACT_PACKED_HOUR(oldTime);
    oldTM.tm_min       = EXTRACT_PACKED_MINUTE(oldTime);
    oldTM.tm_isdst     = -1;
    oldTIME_T = mktime (&oldTM);

    memset (&newTM, 0, sizeof (newTM));
    newTM.tm_year      = EXTRACT_PACKED_YEAR(newDate) - 1900;
    newTM.tm_mon       = EXTRACT_PACKED_MONTH(newDate) - 1;
    newTM.tm_mday      = EXTRACT_PACKED_DAY(newDate);
    newTM.tm_hour      = EXTRACT_PACKED_HOUR(newTime);
    newTM.tm_min       = EXTRACT_PACKED_MINUTE(newTime);
    newTM.tm_isdst     = -1;
    newTIME_T = mktime (&newTM);

    retVal = newTIME_T - oldTIME_T;
    retVal /= 60;
    return retVal;
}

