/*---------------------------------------------------------------------------
 
  FILENAME:
        noaaGenerate.c
 
  PURPOSE:
        Provide the wview NOAA generator utilities.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        01/25/04        M.S. Teel       0               Original
 
  NOTES:
        
 
  LICENSE:
        Copyright (c) 2004, Mark S. Teel (mark@teel.ws)
  
        This source code is released for free distribution under the terms 
        of the GNU General Public License.
  
----------------------------------------------------------------------------*/

/*  ... System include files
*/

/*  ... Library include files
*/

/*  ... Local include files
*/
#include <noaaGenerate.h>


/*  ... global memory declarations
*/

/*  ... global memory referenced
*/

/*  ... static (local) memory declarations
*/
static NOAA_WORK        noaaWork;

static char             *monthStr[13] =
    {
        "NAA",
        "JAN",
        "FEB",
        "MAR",
        "APR",
        "MAY",
        "JUN",
        "JUL",
        "AUG",
        "SEP",
        "OCT",
        "NOV",
        "DEC"
    };

static char             *windirStr[16] =
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


/*  ... local functions
*/


static void writeNoaaMonthFiles (NOAA_ID work, time_t startTime)
{
    char            *destPath = work->htmlPath;
    FILE            *outfile = NULL;
    char            temp[_MAX_PATH];
    NOAA_DAY_REC    record;
    int             month;
    int             numDays, totalDays, totalMonths, done;
    float           avgMean, avgHigh, avgLow, heatDays, coolDays;
    float           rain, avgWind, avgHighWind;
    float           highTemp, lowTemp, highWind;
    int             highTempDay, lowTempDay, highWindDay;
    int             daysMaxOver90, daysMaxUnder32, daysMinUnder32, daysMinUnder0;
    int             daysRainOver001, daysRainOver010, daysRainOver100;
    float           maxDayRain;
    char            maxDayRainDate[12];
    WAVG            windAvg;
    time_t          timeNow = time(NULL);
    struct tm       bknTime;

    localtime_r(&timeNow, &bknTime);
    bknTime.tm_hour = 0;
    bknTime.tm_min = 0;
    bknTime.tm_sec = 0;
    bknTime.tm_isdst = -1;
    timeNow = mktime(&bknTime);

    month = done = numDays = totalDays = totalMonths = 0;

    //  ... do the monthly reports
    startTime -= WV_SECONDS_IN_DAY;
    while (! done)
    {
        startTime += WV_SECONDS_IN_DAY;
        if (startTime >= timeNow)
        {
            done = TRUE;
            record.month = 0xFFFF;
        }
        else if (dbsqliteNOAAGetDay(&record, startTime) == ERROR)
        {
            // There may be gaps:
            continue;
        }

        if (month != record.month)
        {
            totalMonths ++;
            month = record.month;

            // time for a new file, close out the old one and open the new one
            if (outfile)
            {
                if (! work->isMetric)
                {
                    // write the imperial footer
                    fprintf (outfile, "---------------------------------------------------------------------------------------\n");
                    fprintf (outfile, "     %5.1f  %5.1f    %2d   %5.1f    %2d   %5.1f  %5.1f   %4.2f  %4.1f   %5.1f %5d    %s\n\n",
                             avgMean/numDays,
                             highTemp,
                             highTempDay,
                             lowTemp,
                             lowTempDay,
                             heatDays,
                             coolDays,
                             rain,
                             avgWind/numDays,
                             highWind,
                             highWindDay,
                             windirStr[(int)(windAverageCompute(&windAvg)/22.5)]);
    
                    fprintf (outfile, "Max >=  90.0:  %d\n", daysMaxOver90);
                    fprintf (outfile, "Max <=  32.0:  %d\n", daysMaxUnder32);
                    fprintf (outfile, "Min <=  32.0:  %d\n", daysMinUnder32);
                    fprintf (outfile, "Min <=   0.0:  %d\n", daysMinUnder0);
                    fprintf (outfile, "Max Rain    :  %2.2f in ON %s\n",
                             maxDayRain, maxDayRainDate);
                    fprintf (outfile, "Days of Rain: %d (> 0.01 in) %d (> 0.10 in) %d (> 1.00 in)\n",
                             daysRainOver001, daysRainOver010, daysRainOver100);
                }
                else
                {
                    // write the metric footer
                    fprintf (outfile, "---------------------------------------------------------------------------------------\n");
                    fprintf (outfile, "     %5.1f  %5.1f    %2d   %5.1f    %2d   %5.1f  %5.1f   %5.1f %4.1f   %5.1f %5d    %s\n\n",
                             wvutilsConvertFToC(avgMean/numDays),
                             wvutilsConvertFToC(highTemp),
                             highTempDay,
                             wvutilsConvertFToC(lowTemp),
                             lowTempDay,
                             heatDays,
                             coolDays,
                             wvutilsConvertRainINToMetric(rain),
                             wvutilsConvertMPHToKPH(avgWind/numDays),
                             wvutilsConvertMPHToKPH(highWind),
                             highWindDay,
                             windirStr[(int)(windAverageCompute(&windAvg)/22.5)]);
    
                    fprintf (outfile, "Max >=   %3.1f:  %d\n", wvutilsConvertFToC(90.0), daysMaxOver90);
                    fprintf (outfile, "Max <=    %3.1f:  %d\n", wvutilsConvertFToC(32.0), daysMaxUnder32);
                    fprintf (outfile, "Min <=    %3.1f:  %d\n", wvutilsConvertFToC(32.0), daysMinUnder32);
                    fprintf (outfile, "Min <=  %3.1f:  %d\n", wvutilsConvertFToC(0.0), daysMinUnder0);
                    fprintf (outfile, "Max Rain    :  %5.1f mm ON %s\n",
                             wvutilsConvertRainINToMetric(maxDayRain), maxDayRainDate);
                    fprintf (outfile, "Days of Rain: %d (> 0.254 mm) %d (> 2.54 mm) %d (> 25.4 mm)\n",
                             daysRainOver001, daysRainOver010, daysRainOver100);
                }

                fclose (outfile);
                outfile = NULL;
            }

            if (! done)
            {
                sprintf (temp, "%s/NOAA/NOAA-%4.4d-%2.2d.txt",
                         destPath, record.year, record.month);
                outfile = fopen (temp, "w");
                if (outfile == NULL)
                {
                    radMsgLog (PRI_HIGH, "noaaGenerate: %s open error!", temp);
                    return;
                }

                windAverageReset (&windAvg);
                totalDays += numDays;
                numDays = 0;
                avgMean = heatDays = coolDays = 0.0;
                rain = avgWind = 0.0;
                highTemp = -1000.0;
                lowTemp = 1000.0;
                highWind = -100.0;
                highTempDay = lowTempDay = highWindDay = 0;
                daysMaxOver90 = daysMaxUnder32 = daysMinUnder32 = daysMinUnder0 = 0;
                daysRainOver001 = daysRainOver010 = daysRainOver100 = 0;
                maxDayRain = 0.0;
                strcpy (maxDayRainDate, "--/--/----");

                // create the header
                fprintf (outfile, "                   MONTHLY CLIMATOLOGICAL SUMMARY for %s %4.4d\n\n\n",
                         monthStr[record.month], record.year);
                fprintf (outfile, "NAME: %-20s  CITY: %-20s  STATE: %-20s\n",
                         work->stationName, work->stationCity, work->stationState);
                if (! work->isMetric)
                {
                    fprintf (outfile, "ELEV: %d ft    LAT: %4.1f %c    LONG: %4.1f %c\n\n\n",
                             work->elevation,
                             (float)abs(work->latitude)/10.0,
                             ((work->latitude < 0) ? 'S' : 'N'),
                             (float)abs(work->longitude)/10.0,
                             ((work->longitude < 0) ? 'W' : 'E'));
    
                    fprintf (outfile, "                   TEMPERATURE (F), RAIN (in), WIND SPEED (mph)\n\n");
                    fprintf (outfile, "                                         HEAT   COOL         AVG\n");
                    fprintf (outfile, "      MEAN                               DEG    DEG          WIND                  DOM\n");
                    fprintf (outfile, "DAY   TEMP   HIGH  TIME    LOW   TIME    DAYS   DAYS   RAIN  SPEED   HIGH  TIME    DIR\n");
                    fprintf (outfile, "---------------------------------------------------------------------------------------\n");
                }
                else
                {
                    fprintf (outfile, "ELEV: %6.1f m    LAT: %4.1f %c    LONG: %4.1f %c\n\n\n",
                             wvutilsConvertFeetToMeters((float)work->elevation),
                             (float)abs(work->latitude)/10.0,
                             ((work->latitude < 0) ? 'S' : 'N'),
                             (float)abs(work->longitude)/10.0,
                             ((work->longitude < 0) ? 'W' : 'E'));
    
                    fprintf (outfile, "                   TEMPERATURE (C), RAIN (mm), WIND SPEED (kph)\n\n");
                    fprintf (outfile, "                                         HEAT   COOL         AVG\n");
                    fprintf (outfile, "      MEAN                               DEG    DEG          WIND                  DOM\n");
                    fprintf (outfile, "DAY   TEMP   HIGH  TIME    LOW   TIME    DAYS   DAYS   RAIN  SPEED   HIGH  TIME    DIR\n");
                    fprintf (outfile, "---------------------------------------------------------------------------------------\n");
                }
            }
        }

        if (! done)
        {
            numDays ++;

            // compute some stuff for the total line
            if (record.highTemp > highTemp)
            {
                highTemp = record.highTemp;
                highTempDay = record.day;
            }
            if (record.lowTemp < lowTemp)
            {
                lowTemp = record.lowTemp;
                lowTempDay = record.day;
            }
            if (record.highWind > highWind)
            {
                highWind = record.highWind;
                highWindDay = record.day;
            }
            avgMean     += record.meanTemp;
            heatDays    += record.heatDegDays;
            coolDays    += record.coolDegDays;
            rain        += record.rain;
            avgWind     += record.avgWind;
            windAverageAddValue (&windAvg, record.domWindDir);

            if (record.highTemp >= 90.0)
                daysMaxOver90 ++;
            if (record.highTemp <= 32.0)
                daysMaxUnder32 ++;
            if (record.lowTemp <= 32.0)
                daysMinUnder32 ++;
            if (record.lowTemp <= 0.0)
                daysMinUnder0 ++;
            if (record.rain > maxDayRain)
            {
                maxDayRain = record.rain;
                sprintf (maxDayRainDate, "%2.2d/%2.2d/%4.4d",
                         record.month, record.day, record.year);
            }
            if (record.rain > 0.01)
                daysRainOver001 ++;
            if (record.rain > 0.10)
                daysRainOver010 ++;
            if (record.rain > 1.00)
                daysRainOver100 ++;

            if (! work->isMetric)
            {
                // finally, write out the imperial record
                fprintf (outfile, "%2.2d   %5.1f  %5.1f  %5s  %5.1f  %5s  %5.1f  %5.1f   %4.2f  %4.1f   %5.1f  %5s   %s\n",
                         record.day,
                         record.meanTemp,
                         record.highTemp,
                         record.highTempTime,
                         record.lowTemp,
                         record.lowTempTime,
                         record.heatDegDays,
                         record.coolDegDays,
                         record.rain,
                         record.avgWind,
                         record.highWind,
                         record.highWindTime,
                         windirStr[(int)(record.domWindDir/22.5)]);
            }
            else
            {
                // finally, write out the metric record
                fprintf (outfile, "%2.2d   %5.1f  %5.1f  %5s  %5.1f  %5s  %5.1f  %5.1f   %5.1f %4.1f   %5.1f  %5s   %s\n",
                         record.day,
                         wvutilsConvertFToC(record.meanTemp),
                         wvutilsConvertFToC(record.highTemp),
                         record.highTempTime,
                         wvutilsConvertFToC(record.lowTemp),
                         record.lowTempTime,
                         record.heatDegDays,
                         record.coolDegDays,
                         wvutilsConvertRainINToMetric(record.rain),
                         wvutilsConvertMPHToKPH(record.avgWind),
                         wvutilsConvertMPHToKPH(record.highWind),
                         record.highWindTime,
                         windirStr[(int)(record.domWindDir/22.5)]);
            }
        }
    }

    if (outfile)
        fclose (outfile);

    if (numDays > 0 || totalDays > 0)
    {
        if (numDays > 0)
            totalDays += numDays;
        radMsgLog(PRI_STATUS, "NOAA: %d days, %d months", totalDays, totalMonths-1);
    }

    return;
}


static char     tempMonths[13][256];
static char     rainMonths[13][256];
static char     windMonths[13][256];
static void writeNoaaYearFiles (NOAA_ID work, time_t startTime)
{
    char            *destPath = work->htmlPath;
    FILE            *outfile = NULL;
    char            temp[512];
    float           normalTemp[13], normalRain[13];
    float           normYearTemp, normYearRain;
    NOAA_DAY_REC    record;
    int             i, month, year, done = 0;
    int             numDays, numMonths, totalMonths = 0, totalYears = 0;


    float           monthAvgMaxTemp, monthAvgMinTemp, monthAvgTemp, monthDeparture;
    float           monthHeatDays, monthCoolDays;
    float           monthHighTemp, monthLowTemp;
    int             monthHighTempDate, monthLowTempDate;
    int             monthDaysMaxOver90, monthDaysMaxUnder32;
    int             monthDaysMinUnder32, monthDaysMinUnder0;

    float           monthRain, monthHighRainDay;
    int             monthHighRainDayDate;
    int             monthDaysRainOver001, monthDaysRainOver010, monthDaysRainOver100;

    float           monthAvgWind, monthHighWind;
    WAVG            windAvgMonth;
    int             monthHighWindDate;


    float           yearAvgMaxTemp, yearAvgMinTemp, yearAvgTemp;
    float           yearHeatDays, yearCoolDays;
    float           yearHighTemp, yearLowTemp;
    int             yearHighTempDate, yearLowTempDate;
    int             yearDaysMaxOver90, yearDaysMaxUnder32;
    int             yearDaysMinUnder32, yearDaysMinUnder0;

    float           yearRain, yearHighRainDay;
    int             yearHighRainDayDate;
    int             yearDaysRainOver001, yearDaysRainOver010, yearDaysRainOver100;

    float           yearAvgWind, yearHighWind;
    WAVG            windAvgYear;
    int             yearHighWindDate;
    time_t          timeNow = time(NULL);
    struct tm       bknTime;

    localtime_r(&timeNow, &bknTime);
    bknTime.tm_hour = 0;
    bknTime.tm_min = 0;
    bknTime.tm_sec = 0;
    bknTime.tm_isdst = -1;
    timeNow = mktime(&bknTime);

    windAverageReset (&windAvgMonth);
    windAverageReset (&windAvgYear);

    //  ... compute the "normal" values for each month
    dbsqliteNOAAComputeNorms(normalTemp, normalRain, &normYearTemp, &normYearRain);


    //  ... now, down to business
    month = year = 0;

    //  ... do the yearly reports
    startTime -= WV_SECONDS_IN_DAY;
    while (! done)
    {
        startTime += WV_SECONDS_IN_DAY;

        if (startTime >= timeNow)
        {
            done = TRUE;
            record.month = 0xFFFF;      // force a flush for the last file
            record.year = 0xFFFF;       // to avoid the initial match
        }
        else
        {
            if (dbsqliteNOAAGetDay(&record, startTime) == ERROR)
            {
                // Maybe a gap:
                continue;
            }
        }

        if (month != record.month)
        {
            if (outfile)
            {
                monthDeparture = ((normalTemp[month] <= -10000.0) ? 0.0 : ((monthAvgTemp/numDays) - normalTemp[month]));
                if (monthDeparture > -0.1 && monthDeparture < 0.1)
                    monthDeparture = 0.0;

                // save some month records
                if (! work->isMetric)
                {
                    sprintf (tempMonths[month], "%4.4d %2.2d  %5.1f %5.1f %5.1f %5.1f  %6.1f  %6.1f  %5.1f  %2d  %5.1f  %2d     %2d   %2d   %2d   %2d\n",
                             year,
                             month,
                             monthAvgMaxTemp/numDays,
                             monthAvgMinTemp/numDays,
                             monthAvgTemp/numDays,
                             monthDeparture,
                             monthHeatDays,
                             monthCoolDays,
                             monthHighTemp,
                             monthHighTempDate,
                             monthLowTemp,
                             monthLowTempDate,
                             monthDaysMaxOver90,
                             monthDaysMaxUnder32,
                             monthDaysMinUnder32,
                             monthDaysMinUnder0);
    
                    sprintf (rainMonths[month], "%4.4d %2.2d  %5.2f  %6.2f  %5.2f   %2d     %3d    %3d    %3d\n",
                             year,
                             month,
                             monthRain,
                             ((normalRain[month] <= -10000.0) ? 0.0 : (monthRain - normalRain[month])),
                             monthHighRainDay,
                             monthHighRainDayDate,
                             monthDaysRainOver001,
                             monthDaysRainOver010,
                             monthDaysRainOver100);
    
                    sprintf (windMonths[month], "%4.4d %2.2d  %5.1f  %5.1f    %2d   %3s\n",
                             year,
                             month,
                             monthAvgWind/numDays,
                             monthHighWind,
                             monthHighWindDate,
                             windirStr[(int)(windAverageCompute(&windAvgMonth)/22.5)]);
                }
                else
                {
                    sprintf (tempMonths[month], "%4.4d %2.2d  %5.1f %5.1f %5.1f %5.1f  %6.1f  %6.1f  %5.1f  %2d  %5.1f  %2d     %2d   %2d   %2d   %2d\n",
                             year,
                             month,
                             wvutilsConvertFToC(monthAvgMaxTemp/numDays),
                             wvutilsConvertFToC(monthAvgMinTemp/numDays),
                             wvutilsConvertFToC(monthAvgTemp/numDays),
                             wvutilsConvertDeltaFToC(monthDeparture),
                             monthHeatDays,
                             monthCoolDays,
                             wvutilsConvertFToC(monthHighTemp),
                             monthHighTempDate,
                             wvutilsConvertFToC(monthLowTemp),
                             monthLowTempDate,
                             monthDaysMaxOver90,
                             monthDaysMaxUnder32,
                             monthDaysMinUnder32,
                             monthDaysMinUnder0);
    
                    sprintf (rainMonths[month], "%4.4d %2.2d %6.1f  %6.1f %6.1f   %2d     %3d    %3d    %3d\n",
                             year,
                             month,
                             wvutilsConvertRainINToMetric(monthRain),
                             ((normalRain[month] <= -10000.0) ? 0.0 : 
                                 wvutilsConvertRainINToMetric((monthRain - normalRain[month]))),
                             wvutilsConvertRainINToMetric(monthHighRainDay),
                             monthHighRainDayDate,
                             monthDaysRainOver001,
                             monthDaysRainOver010,
                             monthDaysRainOver100);
    
                    sprintf (windMonths[month], "%4.4d %2.2d  %5.1f  %5.1f    %2d   %3s\n",
                             year,
                             month,
                             wvutilsConvertMPHToKPH(monthAvgWind/numDays),
                             wvutilsConvertMPHToKPH(monthHighWind),
                             monthHighWindDate,
                             windirStr[(int)(windAverageCompute(&windAvgMonth)/22.5)]);
                }

                numMonths ++;

                // compute some stuff for the year totals
                yearAvgMaxTemp      += monthAvgMaxTemp/numDays;
                yearAvgMinTemp      += monthAvgMinTemp/numDays;
                yearAvgTemp         += monthAvgTemp/numDays;
                yearHeatDays        += monthHeatDays;
                yearCoolDays        += monthCoolDays;
                yearRain            += monthRain;
                yearAvgWind         += monthAvgWind/numDays;
                windAverageAddValue (&windAvgYear,
                                     (int)(windAverageCompute(&windAvgMonth)));

                if (monthHighTemp > yearHighTemp)
                {
                    yearHighTemp = monthHighTemp;
                    yearHighTempDate = month;
                }
                if (monthLowTemp < yearLowTemp)
                {
                    yearLowTemp = monthLowTemp;
                    yearLowTempDate = month;
                }
                if (monthHighRainDay > yearHighRainDay)
                {
                    yearHighRainDay = monthHighRainDay;
                    yearHighRainDayDate = month;
                }
                if (monthHighWind > yearHighWind)
                {
                    yearHighWind = monthHighWind;
                    yearHighWindDate = month;
                }
                yearDaysMaxOver90 += monthDaysMaxOver90;
                yearDaysMaxUnder32 += monthDaysMaxUnder32;
                yearDaysMinUnder32 += monthDaysMinUnder32;
                yearDaysMinUnder0 += monthDaysMinUnder0;
                yearDaysRainOver001 += monthDaysRainOver001;
                yearDaysRainOver010 += monthDaysRainOver010;
                yearDaysRainOver100 += monthDaysRainOver100;
            }

            // reset some sums, etc.
            numDays = 0;
            totalMonths ++;
            month = record.month;

            windAverageReset (&windAvgMonth);

            monthAvgMaxTemp = monthAvgMinTemp = monthAvgTemp =
            monthHeatDays = monthCoolDays =
            monthHighTempDate = monthLowTempDate =
            monthRain = monthHighRainDay =
            monthAvgWind = monthHighWind = 0.0;

            monthHighTemp = -1000.0;
            monthLowTemp = 1000.0;

            monthDaysMaxOver90 = monthDaysMaxUnder32 =
            monthDaysMinUnder32 = monthDaysMinUnder0 =
            monthHighRainDayDate = monthDaysRainOver001 =
            monthDaysRainOver010 = monthDaysRainOver100 =
            monthHighWindDate = 0;
        }

        if (year != record.year)
        {
            // time for a new file, write the old one and open the new one:
            totalYears ++;

            if (outfile)
            {
                // write the temp header
                fprintf (outfile, "                   ANNUAL CLIMATOLOGICAL SUMMARY for %4.4d\n\n\n",
                         year);
                fprintf (outfile, "NAME: %-20s  CITY: %-20s  STATE: %-20s\n",
                         work->stationName, work->stationCity, work->stationState);
                if (! work->isMetric)
                {
                    // IMPERIAL:
                    fprintf (outfile, "ELEV: %d ft    LAT: %4.1f %c    LONG: %4.1f %c\n\n\n",
                             work->elevation,
                             (float)abs(work->latitude)/10.0,
                             ((work->latitude < 0) ? 'S' : 'N'),
                             (float)abs(work->longitude)/10.0,
                             ((work->longitude < 0) ? 'W' : 'E'));
    
                    fprintf (outfile, "                 TEMPERATURE (F), RAIN (in), WIND SPEED (mph)\n\n");
                    fprintf (outfile, "                            DEP.    HEAT    COOL\n");
                    fprintf (outfile, "          MEAN  MEAN        FROM    DEG     DEG                          MAX  MAX  MIN   MIN\n");
                    fprintf (outfile, " YR  MO   MAX   MIN   MEAN  NORM    DAYS    DAYS     HI DATE   LOW DATE  >=90 <=32 <=32  <=0\n");
                    fprintf (outfile, "---------------------------------------------------------------------------------------------\n");
    
                    for (i = 1; i < 13; i ++)
                    {
                        fputs (tempMonths[i], outfile);
                    }
    
                    // write the temp footer
                    fprintf (outfile, "---------------------------------------------------------------------------------------------\n");
                    fprintf (outfile, "         %5.1f %5.1f %5.1f %5.1f  %6.1f  %6.1f  %5.1f %3s  %5.1f %3s     %2d   %2d   %2d   %2d\n\n\n",
                             yearAvgMaxTemp/numMonths,
                             yearAvgMinTemp/numMonths,
                             yearAvgTemp/numMonths,
                             ((normYearTemp < 0.0) ? 0.0 : ((yearAvgTemp/numMonths) - normYearTemp)),
                             yearHeatDays,
                             yearCoolDays,
                             yearHighTemp,
                             monthStr[yearHighTempDate],
                             yearLowTemp,
                             monthStr[yearLowTempDate],
                             yearDaysMaxOver90,
                             yearDaysMaxUnder32,
                             yearDaysMinUnder32,
                             yearDaysMinUnder0);
    
                    // write the rain header
                    fprintf (outfile, "                            PRECIPITATION (in)\n\n");
                    fprintf (outfile, "                  DEP.    MAX         ---DAYS OF RAIN---\n");
                    fprintf (outfile, "                  FROM    OBS.                OVER\n");
                    fprintf (outfile, " YR  MO  TOTAL    NORM    DAY  DATE   0.01   0.10   1.00\n");
                    fprintf (outfile, "---------------------------------------------------------\n");
    
                    for (i = 1; i < 13; i ++)
                    {
                        fputs (rainMonths[i], outfile);
                    }
    
                    // write the rain footer
                    fprintf (outfile, "---------------------------------------------------------\n");
                    fprintf (outfile, "         %5.2f  %6.2f  %5.2f  %3s     %3d    %3d    %3d\n\n\n",
                             yearRain,
                             ((normYearRain < 0.0) ? 0.0 : (yearRain - normYearRain)),
                             yearHighRainDay,
                             monthStr[yearHighRainDayDate],
                             yearDaysRainOver001,
                             yearDaysRainOver010,
                             yearDaysRainOver100);
    
                    // write the wind header
                    fprintf (outfile, "                              WIND SPEED (mph)\n\n");
                    fprintf (outfile, "                              DOM\n");
                    fprintf (outfile, " YR  MO    AVG     HI   DATE  DIR\n");
                    fprintf (outfile, "----------------------------------\n");
    
                    for (i = 1; i < 13; i ++)
                    {
                        fputs (windMonths[i], outfile);
                    }
    
                    // write the wind footer
                    fprintf (outfile, "----------------------------------\n");
                    fprintf (outfile, "          %4.1f  %5.1f   %3s   %3s\n",
                             yearAvgWind/numMonths,
                             yearHighWind,
                             monthStr[yearHighWindDate],
                             windirStr[(int)(windAverageCompute(&windAvgYear)/22.5)]);
                }
                else
                {
                    // METRIC:
                    fprintf (outfile, "ELEV: %6.1f m    LAT: %4.1f %c    LONG: %4.1f %c\n\n\n",
                             wvutilsConvertFeetToMeters((float)work->elevation),
                             (float)abs(work->latitude)/10.0,
                             ((work->latitude < 0) ? 'S' : 'N'),
                             (float)abs(work->longitude)/10.0,
                             ((work->longitude < 0) ? 'W' : 'E'));
    
                    fprintf (outfile, "                 TEMPERATURE (C), RAIN (mm), WIND SPEED (kph)\n\n");
                    fprintf (outfile, "                            DEP.    HEAT    COOL\n");
                    fprintf (outfile, "          MEAN  MEAN        FROM    DEG     DEG                          MAX   MAX  MIN   MIN\n");
                    fprintf (outfile, " YR  MO   MAX   MIN   MEAN  NORM    DAYS    DAYS     HI DATE   LOW DATE >=32.2 <=0  <=0  <=-17.8\n");
                    fprintf (outfile, "---------------------------------------------------------------------------------------------\n");
    
                    for (i = 1; i < 13; i ++)
                    {
                        fputs (tempMonths[i], outfile);
                    }
    
                    // write the temp footer
                    fprintf (outfile, "---------------------------------------------------------------------------------------------\n");
                    fprintf (outfile, "         %5.1f %5.1f %5.1f %5.1f  %6.1f  %6.1f  %5.1f %3s  %5.1f %3s     %2d   %2d   %2d   %2d\n\n\n",
                             wvutilsConvertFToC(yearAvgMaxTemp/numMonths),
                             wvutilsConvertFToC(yearAvgMinTemp/numMonths),
                             wvutilsConvertFToC(yearAvgTemp/numMonths),
                             ((normYearTemp < 0.0) ? 0.0 : 
                                 wvutilsConvertDeltaFToC(((yearAvgTemp/numMonths) - normYearTemp))),
                             yearHeatDays,
                             yearCoolDays,
                             wvutilsConvertFToC(yearHighTemp),
                             monthStr[yearHighTempDate],
                             wvutilsConvertFToC(yearLowTemp),
                             monthStr[yearLowTempDate],
                             yearDaysMaxOver90,
                             yearDaysMaxUnder32,
                             yearDaysMinUnder32,
                             yearDaysMinUnder0);
    
                    // write the rain header
                    fprintf (outfile, "                            PRECIPITATION (mm)\n\n");
                    fprintf (outfile, "                  DEP.    MAX         ---DAYS OF RAIN---\n");
                    fprintf (outfile, "                  FROM    OBS.                OVER\n");
                    fprintf (outfile, " YR  MO  TOTAL    NORM    DAY  DATE   0.254  2.54   25.4\n");
                    fprintf (outfile, "---------------------------------------------------------\n");
    
                    for (i = 1; i < 13; i ++)
                    {
                        fputs (rainMonths[i], outfile);
                    }
    
                    // write the rain footer
                    fprintf (outfile, "---------------------------------------------------------\n");
                    fprintf (outfile, "        %6.1f  %6.1f  %6.1f %3s     %3d    %3d    %3d\n\n\n",
                             wvutilsConvertRainINToMetric(yearRain),
                             ((normYearRain < 0.0) ? 0.0 : 
                                 wvutilsConvertRainINToMetric((yearRain - normYearRain))),
                             wvutilsConvertRainINToMetric(yearHighRainDay),
                             monthStr[yearHighRainDayDate],
                             yearDaysRainOver001,
                             yearDaysRainOver010,
                             yearDaysRainOver100);
    
                    // write the wind header
                    fprintf (outfile, "                              WIND SPEED (kph)\n\n");
                    fprintf (outfile, "                              DOM\n");
                    fprintf (outfile, " YR  MO    AVG     HI   DATE  DIR\n");
                    fprintf (outfile, "----------------------------------\n");
    
                    for (i = 1; i < 13; i ++)
                    {
                        fputs (windMonths[i], outfile);
                    }
    
                    // write the wind footer
                    fprintf (outfile, "----------------------------------\n");
                    fprintf (outfile, "          %4.1f  %5.1f   %3s   %3s\n",
                             wvutilsConvertMPHToKPH(yearAvgWind/numMonths),
                             wvutilsConvertMPHToKPH(yearHighWind),
                             monthStr[yearHighWindDate],
                             windirStr[(int)(windAverageCompute(&windAvgYear)/22.5)]);
                }

                fclose (outfile);
                outfile = NULL;
            }

            if (! done)
            {
                year = record.year;
                numMonths = 0;
                sprintf (temp, "%s/NOAA/NOAA-%4.4d.txt", destPath, record.year);
                outfile = fopen (temp, "w");
                if (outfile == NULL)
                {
                    radMsgLog (PRI_HIGH, "noaaGenerate: %s open error!", temp);
                    return;
                }
    
                windAverageReset (&windAvgYear);
    
                // reset all the record strings
                for (i = 1; i < 13; i ++)
                {
                    sprintf (tempMonths[i], "%2.2d %2.2d\n", year, i);
                    sprintf (rainMonths[i], "%2.2d %2.2d\n", year, i);
                    sprintf (windMonths[i], "%2.2d %2.2d\n", year, i);
                }
    
                yearAvgMaxTemp = yearAvgMinTemp = yearAvgTemp =
                yearHeatDays = yearCoolDays =
                yearRain = yearHighRainDay =
                yearDaysRainOver001 = yearDaysRainOver010 =
                yearDaysRainOver100 = yearAvgWind = yearHighWind = 0.0;
    
                yearHighTemp = -1000.0;
                yearLowTemp = 1000.0;
    
                yearHighTempDate = yearLowTempDate =
                yearDaysMaxOver90 = yearDaysMaxUnder32 =
                yearDaysMinUnder32 = yearDaysMinUnder0 =
                yearHighRainDayDate = yearHighWindDate = 0;
            }
        }

        if (! done)
        {
            numDays ++;
    
            // compute some stuff for the current month
            monthAvgMaxTemp     += record.highTemp;
            monthAvgMinTemp     += record.lowTemp;
            monthAvgTemp        += record.meanTemp;
            monthHeatDays       += record.heatDegDays;
            monthCoolDays       += record.coolDegDays;
            monthRain           += record.rain;
            monthAvgWind        += record.avgWind;
            windAverageAddValue (&windAvgMonth, record.domWindDir);
    
            if (record.highTemp > monthHighTemp)
            {
                monthHighTemp = record.highTemp;
                monthHighTempDate = record.day;
            }
            if (record.lowTemp < monthLowTemp)
            {
                monthLowTemp = record.lowTemp;
                monthLowTempDate = record.day;
            }
            if (record.rain > monthHighRainDay)
            {
                monthHighRainDay = record.rain;
                monthHighRainDayDate = record.day;
            }
            if (record.highWind > monthHighWind)
            {
                monthHighWind = record.highWind;
                monthHighWindDate = record.day;
            }
            if (record.highTemp >= 90.0)
                monthDaysMaxOver90 ++;
            if (record.highTemp <= 32.0)
                monthDaysMaxUnder32 ++;
            if (record.lowTemp <= 32.0)
                monthDaysMinUnder32 ++;
            if (record.lowTemp <= 0.0)
                monthDaysMinUnder0 ++;
            if (record.rain > 0.01)
                monthDaysRainOver001 ++;
            if (record.rain > 0.1)
                monthDaysRainOver010 ++;
            if (record.rain > 1.0)
                monthDaysRainOver100 ++;
        }
    }

    if (outfile)
        fclose (outfile);
  
    radMsgLog(PRI_STATUS, "NOAA: %d months, %d years", totalMonths-1, totalYears-1);
    return;
}


/*  ... API methods
*/
NOAA_ID noaaGenerateInit
(
    char            *htmlPath,
    int             isMetricUnits,
    int16_t         latitude,
    int16_t         longitude,
    int16_t         elevation,
    char            *name,
    char            *city,
    char            *state,
    time_t          arcTime
)
{
    NOAA_ID         newId;
    NOAA_DAY_REC    record;
    time_t          ntime;
    struct tm       locTime, startTime;
    ARCHIVE_PKT     newestRecord;
    int             retVal;

    newId = &noaaWork;
    memset (newId, 0, sizeof (*newId));

    wvstrncpy (newId->htmlPath, htmlPath, sizeof(newId->htmlPath));
    newId->isMetric = isMetricUnits;
    newId->latitude = latitude;
    newId->longitude = longitude;
    newId->elevation = elevation;
    wvstrncpy (newId->stationName, name, sizeof(newId->stationName));
    wvstrncpy (newId->stationCity, city, sizeof(newId->stationCity));
    wvstrncpy (newId->stationState, state, sizeof(newId->stationState));


    // Make sure there are archive records in the database:
    ntime = dbsqliteArchiveGetNextRecord(0, &newestRecord);
    if (ntime == (time_t)ERROR)
    {
        // There are not!
        radMsgLog (PRI_MEDIUM, "noaaGenerateInit: archive table is empty...");
        return newId;
    }        
    else
    {
        localtime_r(&ntime, &locTime);
    }

    // Update the database:
    newId->lastDay      = arcTime - WV_SECONDS_IN_DAY;
    dbsqliteNOAAUpdate();
    
    
#if __DEBUG_BUFFERS
    radMsgLog (PRI_STATUS, "DBG BFRS: NOAA BEGIN: %u of %u available",
               buffersGetAvailable (),
               buffersGetTotal ());
#endif

    // Normalize the start day:
    localtime_r(&ntime, &startTime);
    startTime.tm_hour = 4;              // Avoid DST mess
    startTime.tm_min = 0;
    startTime.tm_sec = 0;
    startTime.tm_isdst = -1;
    ntime = mktime(&startTime);

    //  ... write all NOAA reports
    radMsgLog (PRI_STATUS, "NOAA Generate: creating monthly NOAA reports starting %4.4d%2.2d",
               locTime.tm_year+1900, locTime.tm_mon+1);
    writeNoaaMonthFiles (newId, ntime);

    radMsgLog (PRI_STATUS, "NOAA Generate: creating yearly NOAA reports starting %d",
               locTime.tm_year+1900);
    writeNoaaYearFiles (newId, ntime);


#if __DEBUG_BUFFERS
    radMsgLog (PRI_STATUS, "DBG BFRS: NOAA END: %u of %u available",
               buffersGetAvailable (),
               buffersGetTotal ());
#endif


    return newId;
}


int noaaGenerate (NOAA_ID id, time_t day)
{
    NOAA_DAY_REC    record;
    struct tm       locTime, startTime;
    ARCHIVE_PKT     newestRecord;
    time_t          ntime;

    day -= WV_SECONDS_IN_DAY;
    dbsqliteNOAAUpdate();

    localtime_r(&day, &locTime);

    // Make sure there are archive records in the database:
    ntime = dbsqliteArchiveGetNextRecord(0, &newestRecord);
    if (ntime == (time_t)ERROR)
    {
        // There are not!
        radMsgLog (PRI_MEDIUM, "noaaGenerateInit: archive table is empty...");
        return ERROR;
    }        

#if __DEBUG_BUFFERS
    radMsgLog (PRI_STATUS, "DBG BFRS: NOAA BEGIN: %u of %u available",
               buffersGetAvailable (),
               buffersGetTotal ());
#endif


    // Normalize the start day:
    localtime_r(&ntime, &startTime);
    startTime.tm_hour = 4;              // Avoid DST mess
    startTime.tm_min = 0;
    startTime.tm_sec = 0;
    startTime.tm_isdst = -1;
    ntime = mktime(&startTime);

    //  ... write all NOAA reports
    radMsgLog (PRI_STATUS, "NOAA Generate: updating monthly NOAA report for %4.4d%2.2d",
               locTime.tm_year+1900, locTime.tm_mon+1);
    writeNoaaMonthFiles (id, ntime);

    radMsgLog (PRI_STATUS, "NOAA Generate: updating yearly NOAA report for %d",
               locTime.tm_year+1900);
    writeNoaaYearFiles (id, ntime);


#if __DEBUG_BUFFERS
    radMsgLog (PRI_STATUS, "DBG BFRS: NOAA END: %u of %u available",
               buffersGetAvailable (),
               buffersGetTotal ());
#endif


    id->lastDay = day;
    return OK;
}

