/*---------------------------------------------------------------------------
 
  FILENAME:
        computedData.c
 
  PURPOSE:
        Provide utilities to compute and store HILOW values.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        08/07/2005      M.S. Teel       0               Original
 
  NOTES:
 
 
  LICENSE:
        Copyright (c) 2005, Mark S. Teel (mark@teel.ws)
 
        This source code is released for free distribution under the terms
        of the GNU General Public License.
 
----------------------------------------------------------------------------*/

/*  ... System include files
*/
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
#include <math.h>


/*  ... Library include files
*/
#include <radmsgLog.h>
#include <radsysutils.h>

/*  ... Local include files
*/
#include <services.h>
#include <computedData.h>

/*  ... global memory declarations
*/

/*  ... local memory
*/
static COMPDATA_WORK        cdWork;
static ARCHIVE_PKT          ArcRecStore;


// compute HILOW value deltas
static int computeDataChanges (WVIEWD_WORK *work)
{
    time_t              retVal;
    time_t              timenow, timeweekago, timedayago, timehourago;
    LOOP_PKT            *current = &work->loopPkt;
    float               temp;
    SENSOR_STORE        *store = &work->sensors;
    ARCHIVE_PKT         arcRecord;
    int                 sqlInterval = work->archiveInterval * 60;

    // get our three times of interest:
    timenow = time(NULL);
    timeweekago = timenow - (WV_SECONDS_IN_DAY * 7);
    timedayago  = timenow - WV_SECONDS_IN_DAY;
    timehourago = timenow - WV_SECONDS_IN_HOUR;


    // initialize the store
    store->hourchangetemp = 0;
    store->hourchangewind = 0;
    store->hourchangewinddir = 0;
    store->hourchangehumid = 0;
    store->hourchangedewpt = 0;
    store->hourchangebarom = 0;
    store->daychangetemp = 0;
    store->daychangewind = 0;
    store->daychangewinddir = 0;
    store->daychangehumid = 0;
    store->daychangedewpt = 0;
    store->daychangebarom = 0;
    store->weekchangetemp = 0;
    store->weekchangewind = 0;
    store->weekchangewinddir = 0;
    store->weekchangehumid = 0;
    store->weekchangedewpt = 0;
    store->weekchangebarom = 0;


    // now on to the record examination...
    retVal = dbsqliteArchiveGetFirstRecord(timeweekago-sqlInterval, 
                                           timeweekago+sqlInterval,
                                           &arcRecord);
    if (retVal != ERROR)
    {
        // Week Ago:
        if (current->outTemp > ARCHIVE_VALUE_NULL && 
            arcRecord.value[DATA_INDEX_outTemp] > ARCHIVE_VALUE_NULL)
            store->weekchangetemp = current->outTemp - (float)arcRecord.value[DATA_INDEX_outTemp];
        else
            store->weekchangetemp = 0;

        if (arcRecord.value[DATA_INDEX_windSpeed] > ARCHIVE_VALUE_NULL)
            store->weekchangewind = current->windSpeedF - (float)arcRecord.value[DATA_INDEX_windSpeed];
        else
            store->weekchangewind = 0;

        if (arcRecord.value[DATA_INDEX_windDir] >= 0)
            store->weekchangewinddir = (int16_t)((float)current->windDir - (float)arcRecord.value[DATA_INDEX_windDir]);
        else
            store->weekchangewinddir = 0;

        if (arcRecord.value[DATA_INDEX_outHumidity] > ARCHIVE_VALUE_NULL)
            store->weekchangehumid = current->outHumidity - (uint16_t)arcRecord.value[DATA_INDEX_outHumidity];
        else
            store->weekchangehumid = 0;

        if (current->dewpoint > ARCHIVE_VALUE_NULL && 
            arcRecord.value[DATA_INDEX_outTemp] > ARCHIVE_VALUE_NULL &&
            arcRecord.value[DATA_INDEX_outHumidity] > ARCHIVE_VALUE_NULL)
        {
            temp = wvutilsCalculateDewpoint ((float)arcRecord.value[DATA_INDEX_outTemp],
                                             (float)arcRecord.value[DATA_INDEX_outHumidity]);
            store->weekchangedewpt = current->dewpoint - temp;
        }
        else
        {
            store->weekchangedewpt = 0;
        }

        if (current->barometer > ARCHIVE_VALUE_NULL && 
            arcRecord.value[DATA_INDEX_barometer] > ARCHIVE_VALUE_NULL)
            store->weekchangebarom = current->barometer - (float)arcRecord.value[DATA_INDEX_barometer];
        else
            store->weekchangebarom = 0;
    }

    retVal = dbsqliteArchiveGetFirstRecord(timedayago-sqlInterval, 
                                           timedayago+sqlInterval,
                                           &arcRecord);
    if (retVal != ERROR)
    {
        // Day Ago:
        if (current->outTemp > ARCHIVE_VALUE_NULL && 
            arcRecord.value[DATA_INDEX_outTemp] > ARCHIVE_VALUE_NULL)
            store->daychangetemp = current->outTemp - (float)arcRecord.value[DATA_INDEX_outTemp];
        else
            store->daychangetemp = 0;

        if (arcRecord.value[DATA_INDEX_windSpeed] > ARCHIVE_VALUE_NULL)
            store->daychangewind = current->windSpeedF - (float)arcRecord.value[DATA_INDEX_windSpeed];
        else
            store->daychangewind = 0;

        if (arcRecord.value[DATA_INDEX_windDir] >= 0)
            store->daychangewinddir = (int16_t)((float)current->windDir - (float)arcRecord.value[DATA_INDEX_windDir]);
        else
            store->daychangewinddir = 0;

        if (arcRecord.value[DATA_INDEX_outHumidity] > ARCHIVE_VALUE_NULL)
            store->daychangehumid = current->outHumidity - (uint16_t)arcRecord.value[DATA_INDEX_outHumidity];
        else
            store->daychangehumid = 0;

        if (current->dewpoint > ARCHIVE_VALUE_NULL && 
            arcRecord.value[DATA_INDEX_outTemp] > ARCHIVE_VALUE_NULL &&
            arcRecord.value[DATA_INDEX_outHumidity] > ARCHIVE_VALUE_NULL)
        {
            temp = wvutilsCalculateDewpoint ((float)arcRecord.value[DATA_INDEX_outTemp],
                                             (float)arcRecord.value[DATA_INDEX_outHumidity]);
            store->daychangedewpt = current->dewpoint - temp;
        }
        else
        {
            store->daychangedewpt = 0;
        }

        if (current->barometer > ARCHIVE_VALUE_NULL && 
            arcRecord.value[DATA_INDEX_barometer] > ARCHIVE_VALUE_NULL)
            store->daychangebarom = current->barometer - (float)arcRecord.value[DATA_INDEX_barometer];
        else
            store->daychangebarom = 0;
    }

    retVal = dbsqliteArchiveGetFirstRecord(timehourago-sqlInterval, 
                                           timehourago+sqlInterval,
                                           &arcRecord);
    if (retVal != ERROR)
    {
        // Hour Ago:
        if (current->outTemp > ARCHIVE_VALUE_NULL && 
            arcRecord.value[DATA_INDEX_outTemp] > ARCHIVE_VALUE_NULL)
            store->hourchangetemp = current->outTemp - (float)arcRecord.value[DATA_INDEX_outTemp];
        else
            store->hourchangetemp = 0;

        if (arcRecord.value[DATA_INDEX_windSpeed] > ARCHIVE_VALUE_NULL)
            store->hourchangewind = current->windSpeedF - (float)arcRecord.value[DATA_INDEX_windSpeed];
        else
            store->hourchangewind = 0;

        if (arcRecord.value[DATA_INDEX_windDir] >= 0)
            store->hourchangewinddir = (int16_t)((float)current->windDir - (float)arcRecord.value[DATA_INDEX_windDir]);
        else
            store->hourchangewinddir = 0;

        if (arcRecord.value[DATA_INDEX_outHumidity] > ARCHIVE_VALUE_NULL)
            store->hourchangehumid = current->outHumidity - (uint16_t)arcRecord.value[DATA_INDEX_outHumidity];
        else
            store->hourchangehumid = 0;

        if (current->dewpoint > ARCHIVE_VALUE_NULL && 
            arcRecord.value[DATA_INDEX_outTemp] > ARCHIVE_VALUE_NULL &&
            arcRecord.value[DATA_INDEX_outHumidity] > ARCHIVE_VALUE_NULL)
        {
            temp = wvutilsCalculateDewpoint ((float)arcRecord.value[DATA_INDEX_outTemp],
                                             (float)arcRecord.value[DATA_INDEX_outHumidity]);
            store->hourchangedewpt = current->dewpoint - temp;
        }
        else
        {
            store->hourchangedewpt = 0;
        }

        if (current->barometer > ARCHIVE_VALUE_NULL && 
            arcRecord.value[DATA_INDEX_barometer] > ARCHIVE_VALUE_NULL)
            store->hourchangebarom = current->barometer - (float)arcRecord.value[DATA_INDEX_barometer];
        else
            store->hourchangebarom = 0;
    }

    return OK;
}

// compute HILOW values for the current hour
static int computeDataHour (WVIEWD_WORK *work, time_t lastTime)
{
    int                 numrecs = 0;
    time_t              timenow = time(NULL);
    struct tm           bkntimenow;
    SENSOR_STORE        *store = &work->sensors;

    // do this so we pick up the proper hour/day when mins < archiveInterval
    timenow -= (work->archiveInterval * 60);

    // build time for this hour:
    localtime_r(&timenow, &bkntimenow);
    bkntimenow.tm_min  = 0;
    bkntimenow.tm_sec  = 0;
    timenow = mktime(&bkntimenow);
    
    if (lastTime > timenow)
    {
        // Only add new records:
        timenow = lastTime + 1;
    }

    numrecs = dbsqliteHiLowGetHour(timenow, store, STF_HOUR);

    // were there any recs for this hour?
    if (numrecs <= 0)
    {
        return ERROR;
    }
    else
    {
        return OK;
    }
}

// compute HILOW values for the given day
// Returns: number of good records processed
static int computeDataForDay
(
    WVIEWD_WORK         *work,
    time_t              dayStart,
    SENSOR_TIMEFRAMES   frame
)
{
    int                 retVal;

    retVal = dbsqliteHiLowGetDay(dayStart, &work->sensors, frame);
    if (retVal >= 0)
    {
        return retVal;
    }
    else
    {
        return 0;
    }
}

// compute HILOW values for the current day
static int computeDataDay (WVIEWD_WORK *work, time_t lastTime)
{
    int                 retVal;
    time_t              timenow = time(NULL);
    struct tm           bkntimenow;
    SENSOR_STORE        *store = &work->sensors;

    // do this so we pick up the proper hour/day when mins < archiveInterval
    timenow -= (work->archiveInterval * 60);

    // build time for this day:
    localtime_r(&timenow, &bkntimenow);
    bkntimenow.tm_hour  = 0;
    bkntimenow.tm_min   = 0;
    bkntimenow.tm_sec   = 0;
    bkntimenow.tm_isdst = -1;
    timenow = mktime(&bkntimenow);
    localtime_r(&timenow, &bkntimenow);

    if (lastTime > timenow)
    {
        // Only add new records:
        timenow = lastTime + 1;
    }

    retVal = computeDataForDay(work, timenow, STF_DAY);
    if (retVal <= 0)
    {
        return ERROR;
    }
    else
    {
        radMsgLog(PRI_STATUS, "computeDataDay: %4.4d%2.2d%2.2d",
                  bkntimenow.tm_year + 1900,
                  bkntimenow.tm_mon + 1,
                  bkntimenow.tm_mday);
        return OK;
    }
}

// compute HILOW values for the last 7 days
static int computeDataWeek (WVIEWD_WORK *work, time_t lastTime)
{
    int                 i, retVal, numrecs = 0;
    time_t              currentTime, timenow = time(NULL);
    struct tm           bkntimenow;
    SENSOR_STORE        *store = &work->sensors;

    // do this so we pick up the proper hour/day when mins < archiveInterval
    currentTime = timenow;
    timenow -= (work->archiveInterval * 60);
    timenow -= (WV_SECONDS_IN_DAY * 7);

    // build time:
    localtime_r(&timenow, &bkntimenow);
    bkntimenow.tm_hour  = 0;
    bkntimenow.tm_min   = 0;
    bkntimenow.tm_sec   = 0;
    bkntimenow.tm_isdst = -1;
    timenow = mktime(&bkntimenow);
    localtime_r(&timenow, &bkntimenow);

    if (lastTime > timenow)
    {
        // Only add new records:
        timenow = lastTime + 1;
    }

    // Note: timenow may not point to the beginning of a day, so we renormalize
    //       at the bottom of the for loop.
    for (i = 1; (timenow < currentTime) && (i <= 7); i ++)
    {
        retVal = computeDataForDay(work, timenow, STF_WEEK);
        if (retVal > 0)
        {
            numrecs += retVal;
            radMsgLog(PRI_STATUS, "computeDataWeek: %4.4d%2.2d%2.2d",
                      bkntimenow.tm_year + 1900,
                      bkntimenow.tm_mon + 1,
                      bkntimenow.tm_mday);
        }

        timenow += WV_SECONDS_IN_DAY;
        localtime_r(&timenow, &bkntimenow);
        bkntimenow.tm_hour  = 0;
        bkntimenow.tm_min   = 0;
        bkntimenow.tm_sec   = 0;
        bkntimenow.tm_isdst = -1;
        timenow = mktime(&bkntimenow);
        localtime_r(&timenow, &bkntimenow);
    }

    // were there any recs for this week?
    if (numrecs == 0)
    {
        return ERROR;
    }
    else
    {
        return OK;
    }
}

// compute HILOW values for the given month
// Returns: number of good records processed
static int computeDataForMonth
(
    WVIEWD_WORK         *work,
    int                 month,
    int                 year,
    SENSOR_TIMEFRAMES   frame,
    int                 yearRainFlag,
    time_t              lastTime
)
{
    int                 retVal;
    time_t              timenow;
    struct tm           bkntimenow;

    // build time for this month:
    bkntimenow.tm_year  = year - 1900;
    bkntimenow.tm_mon   =  month - 1;
    bkntimenow.tm_mday  = 1;
    bkntimenow.tm_hour  = 0;
    bkntimenow.tm_min   = 0;
    bkntimenow.tm_sec   = 0;
    bkntimenow.tm_isdst = -1;               // we don't know...
    timenow = mktime(&bkntimenow);

    if (lastTime > timenow)
    {
        // Only add new records:
        timenow = lastTime + 1;
    }


    retVal = dbsqliteHiLowGetMonth(timenow, &work->sensors, frame, yearRainFlag);
    if (retVal >= 0)
    {
        return retVal;
    }
    else
    {
        return 0;
    }
}

// compute HILOW values for the current month:
// Returns: 0 if OK or ERROR if an error occurs
static int computeDataMonth (WVIEWD_WORK *work, time_t lastTime)
{
    SENSOR_STORE        *store = &work->sensors;
    int                 retVal;
    time_t              timenow = time(NULL);
    struct tm           bkntimenow;

    // do this so we pick up the proper hour/day when mins < archiveInterval
    timenow -= (work->archiveInterval * 60);
    localtime_r (&timenow, &bkntimenow);

    // process the month
    retVal = computeDataForMonth (work,
                                  bkntimenow.tm_mon + 1,
                                  bkntimenow.tm_year + 1900,
                                  STF_MONTH,
                                  FALSE,
                                  lastTime);
    if (retVal == ERROR || retVal == 0)
    {
        return ERROR;
    }

    radMsgLog(PRI_STATUS, "computeDataMonth: %4.4d%2.2d", 
              bkntimenow.tm_year + 1900, bkntimenow.tm_mon + 1);
    return OK;
}

// compute HILOW values for the current year
static int computeDataYear (WVIEWD_WORK *work, time_t lastTime)
{
    SENSOR_STORE        *store = &work->sensors;
    int                 i, retVal, numrecs = 0;
    int                 nowmonth, rainmonth, nowyear, rainyear;
    time_t              startTime, timenow = time(NULL);
    struct tm           startMonth, bkntimenow;

    // do this so we pick up the proper hour/day when mins < archiveInterval
    timenow -= (work->archiveInterval * 60);

    localtime_r (&timenow, &bkntimenow);
    nowmonth = bkntimenow.tm_mon + 1;
    nowyear = bkntimenow.tm_year + 1900;

    bkntimenow.tm_mon   = 0;
    bkntimenow.tm_mday  = 1;
    bkntimenow.tm_hour  = 0;
    bkntimenow.tm_min   = 0;
    bkntimenow.tm_sec   = 0;
    bkntimenow.tm_isdst = -1;
    startTime = mktime(&bkntimenow);

    if (lastTime > startTime)
    {
        // Only add new records:
        startTime = lastTime + 1;
    }
    
    localtime_r(&startTime, &startMonth);

    // loop through each month this year (so far)
    for (i = startMonth.tm_mon+1; i <= nowmonth; i ++)
    {
        // process the month
        retVal = computeDataForMonth (work, i, nowyear, STF_YEAR, FALSE, startTime);
        if (retVal == ERROR || retVal == 0)
        {
            // just continue here
            continue;
        }

        radMsgLog(PRI_STATUS, "computeDataYear: %4.4d%2.2d", nowyear, i);

        // Clear this so it is only used the first time through:
        startTime = 0;

        numrecs += retVal;
    }

    // If we didn't have save data:
    if (lastTime == 0)
    {
        // Clear rain/ET sensors:
        sensorInit(&store->sensor[STF_YEAR][SENSOR_RAIN]);
        sensorInit(&store->sensor[STF_YEAR][SENSOR_RAINRATE]);
        sensorInit(&store->sensor[STF_YEAR][SENSOR_ET]);
        
        // Do the Year rain totals to account for rain season start:
        rainmonth = work->stationRainSeasonStart;
        rainyear = nowyear;
        if (rainmonth > nowmonth)
        {
            // we need to go back a year...
            rainyear --;
        }
    
        localtime_r (&timenow, &bkntimenow);
        bkntimenow.tm_year  = rainyear - 1900;
        bkntimenow.tm_mon   = rainmonth - 1;
        bkntimenow.tm_mday  = 1;
        bkntimenow.tm_hour  = 0;
        bkntimenow.tm_min   = 0;
        bkntimenow.tm_sec   = 0;
        bkntimenow.tm_isdst = -1;
        startTime = mktime(&bkntimenow);
    
        // now loop till we get to this month:
        while ((rainyear < nowyear) || ((rainmonth <= nowmonth) && (rainyear == nowyear)))
        {
            // process the month for yearly rain
            computeDataForMonth (work, rainmonth, rainyear, STF_YEAR, TRUE, startTime);
    
            if (++rainmonth > 12)
            {
                rainmonth = 1;
                rainyear ++;
            }
    
            // Only use this the first time through:
            startTime = 0;
        }
    }
    return OK;
}

// compute HILOW values for all time:
static int computeDataAllTime (WVIEWD_WORK *work, time_t firstTime)
{
    SENSOR_STORE        *store = &work->sensors;
    int                 i, j, retVal, numrecs = 0;
    int                 nowmonth, nowyear;
    time_t              startTime, timenow = time(NULL);
    struct tm           startMonth, bkntimenow;

    // do this so we pick up the proper hour/day when mins < archiveInterval
    timenow -= (work->archiveInterval * 60);

    localtime_r (&timenow, &bkntimenow);
    nowmonth = bkntimenow.tm_mon + 1;
    nowyear = bkntimenow.tm_year + 1900;

    localtime_r(&firstTime, &bkntimenow);
    bkntimenow.tm_mday  = 1;
    bkntimenow.tm_hour  = 0;
    bkntimenow.tm_min   = 0;
    bkntimenow.tm_sec   = 0;
    bkntimenow.tm_isdst = -1;
    startTime = mktime(&bkntimenow);

    localtime_r(&startTime, &startMonth);

    // loop through each month until we reach now:
    for (i = startMonth.tm_mon+1, j = startMonth.tm_year+1900; 
         (j < nowyear) || (j == nowyear && i <= nowmonth); 
         i ++)
    {
        if (i > 12)
        {
            i = 1;
            j ++;
        }

        // process the month
        retVal = computeDataForMonth (work, i, j, STF_ALL, FALSE, 0);
        if (retVal <= 0)
        {
            // just continue here
            continue;
        }

        radMsgLog(PRI_STATUS, "computeDataAllTime: %4.4d%2.2d", j, i);

        numrecs += retVal;
    }

    return OK;
}

static void addRecordHour (WVIEWD_WORK *work, int clearFlag)
{
    if (clearFlag)
    {
        // initialize the store
        sensorClearSet (work->sensors.sensor[STF_HOUR]);
        windAverageReset (&work->sensors.wind[STF_HOUR]);
    }

    sensorPropogateSample (work->sensors.sensor[STF_HOUR],
                           work->sensors.sensor[STF_INTERVAL]);
    windAverageAddValue (&work->sensors.wind[STF_HOUR],
                         (int)windAverageCompute(&work->sensors.wind[STF_INTERVAL]));
    return;
}

static void addRecordDay (WVIEWD_WORK *work, int clearFlag)
{
    if (clearFlag)
    {
        // initialize the store
        sensorClearSet (work->sensors.sensor[STF_DAY]);
        windAverageReset (&work->sensors.wind[STF_DAY]);
    }

    sensorPropogateSample (work->sensors.sensor[STF_DAY],
                           work->sensors.sensor[STF_INTERVAL]);
    windAverageAddValue (&work->sensors.wind[STF_DAY],
                         (int)windAverageCompute(&work->sensors.wind[STF_INTERVAL]));
    return;
}

static void addRecordWeek (WVIEWD_WORK *work)
{
    sensorPropogateSample (work->sensors.sensor[STF_WEEK],
                           work->sensors.sensor[STF_INTERVAL]);
    windAverageAddValue (&work->sensors.wind[STF_WEEK],
                         (int)windAverageCompute(&work->sensors.wind[STF_INTERVAL]));
    return;
}

static void addRecordMonth (WVIEWD_WORK *work, int clearFlag)
{
    if (clearFlag)
    {
        // initialize the store
        sensorClearSet (work->sensors.sensor[STF_MONTH]);
        windAverageReset (&work->sensors.wind[STF_MONTH]);
    }

    sensorPropogateSample (work->sensors.sensor[STF_MONTH],
                           work->sensors.sensor[STF_INTERVAL]);
    windAverageAddValue (&work->sensors.wind[STF_MONTH],
                         (int)windAverageCompute(&work->sensors.wind[STF_INTERVAL]));
    return;
}

static void addRecordYear (WVIEWD_WORK *work, int clearFlag)
{
    WV_SENSOR       rainSensor, rainrateSensor, etSensor;

    if (clearFlag)
    {
        // initialize the store
        rainSensor = work->sensors.sensor[STF_YEAR][SENSOR_RAIN];
        rainrateSensor = work->sensors.sensor[STF_YEAR][SENSOR_RAINRATE];
        etSensor = work->sensors.sensor[STF_YEAR][SENSOR_ET];
        sensorClearSet (work->sensors.sensor[STF_YEAR]);
        work->sensors.sensor[STF_YEAR][SENSOR_RAIN] = rainSensor;
        work->sensors.sensor[STF_YEAR][SENSOR_RAINRATE] = rainrateSensor;
        work->sensors.sensor[STF_YEAR][SENSOR_ET] = etSensor;
        windAverageReset (&work->sensors.wind[STF_YEAR]);
    }

    sensorPropogateSample (work->sensors.sensor[STF_YEAR],
                           work->sensors.sensor[STF_INTERVAL]);
    windAverageAddValue (&work->sensors.wind[STF_YEAR],
                         (int)windAverageCompute(&work->sensors.wind[STF_INTERVAL]));
    return;
}

static void addRecordAllTime (WVIEWD_WORK *work, int clearFlag)
{
    WV_SENSOR       rainSensor, rainrateSensor, etSensor;

    if (clearFlag)
    {
        // initialize the store
        rainSensor = work->sensors.sensor[STF_ALL][SENSOR_RAIN];
        rainrateSensor = work->sensors.sensor[STF_ALL][SENSOR_RAINRATE];
        etSensor = work->sensors.sensor[STF_ALL][SENSOR_ET];
        sensorClearSet (work->sensors.sensor[STF_ALL]);
        work->sensors.sensor[STF_ALL][SENSOR_RAIN] = rainSensor;
        work->sensors.sensor[STF_ALL][SENSOR_RAINRATE] = rainrateSensor;
        work->sensors.sensor[STF_ALL][SENSOR_ET] = etSensor;
        windAverageReset (&work->sensors.wind[STF_ALL]);
    }

    sensorPropogateSample (work->sensors.sensor[STF_ALL],
                           work->sensors.sensor[STF_INTERVAL]);
    windAverageAddValue (&work->sensors.wind[STF_ALL],
                         (int)windAverageCompute(&work->sensors.wind[STF_INTERVAL]));
    return;
}

static void resetRainRecordYear (WVIEWD_WORK *work)
{
    // initialize the store
    sensorInit (&work->sensors.sensor[STF_YEAR][SENSOR_RAIN]);
    sensorInit (&work->sensors.sensor[STF_YEAR][SENSOR_RAINRATE]);
    sensorInit (&work->sensors.sensor[STF_YEAR][SENSOR_ET]);
    sensorAddSample (&work->sensors.sensor[STF_YEAR][SENSOR_RAIN],
                     &work->sensors.sensor[STF_INTERVAL][SENSOR_RAIN],
                     FALSE);
    sensorAddSample (&work->sensors.sensor[STF_YEAR][SENSOR_RAINRATE],
                     &work->sensors.sensor[STF_INTERVAL][SENSOR_RAINRATE],
                     FALSE);
    sensorAddSample (&work->sensors.sensor[STF_YEAR][SENSOR_ET],
                     &work->sensors.sensor[STF_INTERVAL][SENSOR_ET],
                     FALSE);
    return;
}

static void intervalHousekeepingInit (WVIEWD_WORK *work)
{
    float       windAvg, tempAvg;

    // Note: executed once during init

    // Add the interval average values
    windAvg = sensorGetAvg (&work->sensors.sensor[STF_HOUR][SENSOR_WSPEED]);
    tempAvg = sensorGetAvg (&work->sensors.sensor[STF_HOUR][SENSOR_OUTTEMP]);
    work->loopPkt.intervalAvgWCHILL = wvutilsCalculateWindChill(tempAvg, windAvg);
    work->loopPkt.intervalAvgWSPEEDF = windAvg;

    return;
}

static void intervalHousekeeping (WVIEWD_WORK *work)
{
    float       windAvg, tempAvg;

    // Note: do any processing here that needs to be done every archive interval

    // Add the interval average values
    windAvg = sensorGetAvg (&work->sensors.sensor[STF_INTERVAL][SENSOR_WSPEED]);
    tempAvg = sensorGetAvg (&work->sensors.sensor[STF_INTERVAL][SENSOR_OUTTEMP]);
    work->loopPkt.intervalAvgWCHILL = wvutilsCalculateWindChill(tempAvg, windAvg);
    work->loopPkt.intervalAvgWSPEEDF = windAvg;

    return;
}

////////////////////////////////////////////////////////////////////////////
/////////////////////////////////   A P I  /////////////////////////////////
////////////////////////////////////////////////////////////////////////////

// store a new data sample for all sensors to the current archive interval:
int computedDataStoreSample (WVIEWD_WORK *work)
{
    WV_SENSOR       sample[SENSOR_MAX];

    sensorClearSet (sample);

    // convert from LOOP_PKT to a sensor sample
    sensorUpdate (&sample[SENSOR_INTEMP], work->loopPkt.inTemp);
    sensorUpdate (&sample[SENSOR_OUTTEMP], work->loopPkt.outTemp);
    sensorUpdate (&sample[SENSOR_INHUMID], (float)work->loopPkt.inHumidity);
    sensorUpdate (&sample[SENSOR_OUTHUMID], (float)work->loopPkt.outHumidity);
    sensorUpdate (&sample[SENSOR_BP], work->loopPkt.barometer);
    sensorUpdate (&sample[SENSOR_WSPEED], work->loopPkt.windSpeedF);
    sensorUpdateWhen (&sample[SENSOR_WGUST],
                      work->loopPkt.windGustF,
                      (float)work->loopPkt.windGustDir);
    sensorUpdate (&sample[SENSOR_DEWPOINT], work->loopPkt.dewpoint);
    sensorUpdate (&sample[SENSOR_RAIN], work->loopPkt.sampleRain);
    sensorUpdate (&sample[SENSOR_RAINRATE], work->loopPkt.rainRate);
    sensorUpdate (&sample[SENSOR_WCHILL], work->loopPkt.windchill);
    sensorUpdate (&sample[SENSOR_HINDEX], work->loopPkt.heatindex);

    if (work->loopPkt.sampleET > ARCHIVE_VALUE_NULL)
        sensorUpdate (&sample[SENSOR_ET], work->loopPkt.sampleET);
    if (work->loopPkt.UV != 0xFFFF)
        sensorUpdate (&sample[SENSOR_UV], (float)work->loopPkt.UV);
    if (work->loopPkt.radiation != 0xFFFF)
        sensorUpdate (&sample[SENSOR_SOLRAD], (float)work->loopPkt.radiation);
    if (work->loopPkt.wxt510Hail > ARCHIVE_VALUE_NULL)
        sensorUpdate (&sample[SENSOR_HAIL], work->loopPkt.wxt510Hail);
    if (work->loopPkt.wxt510Hailrate > ARCHIVE_VALUE_NULL)
        sensorUpdate (&sample[SENSOR_HAILRATE], work->loopPkt.wxt510Hailrate);

    // store it
    sensorPropogateSample (work->sensors.sensor[STF_INTERVAL], sample);

    // store the wind direction
    windAverageAddValue (&work->sensors.wind[STF_INTERVAL], work->loopPkt.windDir);

    // Store to the HILOW database:
    dbsqliteHiLowStoreSample(time(NULL), &work->loopPkt);

    return OK;
}

ARCHIVE_PKT *computedDataGenerateArchive (WVIEWD_WORK *work)
{
    WV_SENSOR       *sample = work->sensors.sensor[STF_INTERVAL];
    time_t          nowtime = time (NULL);
    struct tm       bknTime;
    Data_Indices    index;

    // create the time_t time for the record:
    localtime_r (&nowtime, &bknTime);
    bknTime.tm_sec  = 0;
    ArcRecStore.dateTime = (int32_t)mktime(&bknTime);

    ArcRecStore.usUnits  = 1;
    ArcRecStore.interval = work->archiveInterval;

    // Set all values to NULL by default:
    for (index = DATA_INDEX_barometer; index < DATA_INDEX_MAX; index ++)
    {
        ArcRecStore.value[index] = ARCHIVE_VALUE_NULL;
    }

    // have we received any LOOP updates this interval?
    if (sensorGetSamples(&sample[SENSOR_OUTTEMP]) == 0)
    {
        // we have not - squawk about it and exit
        radMsgLog (PRI_MEDIUM, "computedDataGenerateArchive: no samples for this interval!");
        emailAlertSend(ALERT_TYPE_STATION_LOOP);
        return NULL;
    }

    // Set the values we can:
    ArcRecStore.value[DATA_INDEX_outTemp]        = (float)sensorGetAvg (&sample[SENSOR_OUTTEMP]);
    ArcRecStore.value[DATA_INDEX_rain]           = (float)sensorGetCumulative (&sample[SENSOR_RAIN]);
    ArcRecStore.value[DATA_INDEX_rainRate]       = (float)sensorGetHigh (&sample[SENSOR_RAINRATE]);
    ArcRecStore.value[DATA_INDEX_barometer]      = (float)sensorGetAvg (&sample[SENSOR_BP]);
    ArcRecStore.value[DATA_INDEX_pressure]       = 
        wvutilsConvertSLPToSP((float)ArcRecStore.value[DATA_INDEX_barometer],
                              (float)ArcRecStore.value[DATA_INDEX_outTemp],
                              (float)work->elevation);
    ArcRecStore.value[DATA_INDEX_altimeter]      = 
        wvutilsConvertSPToAltimeter((float)ArcRecStore.value[DATA_INDEX_pressure],
                                    (float)work->elevation);
    ArcRecStore.value[DATA_INDEX_inTemp]         = (float)sensorGetAvg (&sample[SENSOR_INTEMP]);
    ArcRecStore.value[DATA_INDEX_inHumidity]     = (float)sensorGetAvg (&sample[SENSOR_INHUMID]);
    ArcRecStore.value[DATA_INDEX_outHumidity]    = (float)sensorGetAvg (&sample[SENSOR_OUTHUMID]);
    ArcRecStore.value[DATA_INDEX_windSpeed]      = (float)sensorGetAvg (&sample[SENSOR_WSPEED]);
    if (sensorGetHigh (&sample[SENSOR_WGUST]) >= 0)
    {
        ArcRecStore.value[DATA_INDEX_windGust]   = (float)sensorGetHigh (&sample[SENSOR_WGUST]);
    }
    else
    {
        ArcRecStore.value[DATA_INDEX_windGust]   = 0;
    }

    // save the high wind speed in the loop packet
    work->loopPkt.windGustF = ArcRecStore.value[DATA_INDEX_windGust];

    ArcRecStore.value[DATA_INDEX_dewpoint]       =
        wvutilsCalculateDewpoint ((float)ArcRecStore.value[DATA_INDEX_outTemp],
                                  (float)ArcRecStore.value[DATA_INDEX_outHumidity]);
    ArcRecStore.value[DATA_INDEX_windchill]      =
        wvutilsCalculateWindChill ((float)ArcRecStore.value[DATA_INDEX_outTemp],
                                   (float)ArcRecStore.value[DATA_INDEX_windSpeed]);
    ArcRecStore.value[DATA_INDEX_heatindex]      =
        wvutilsCalculateHeatIndex ((float)ArcRecStore.value[DATA_INDEX_outTemp],
                                   (float)ArcRecStore.value[DATA_INDEX_outHumidity]);

    if (ArcRecStore.value[DATA_INDEX_windSpeed] > 0 || 
        ArcRecStore.value[DATA_INDEX_windGust] > 0)
    {
        ArcRecStore.value[DATA_INDEX_windDir]        = (float)windAverageCompute(&work->sensors.wind[STF_INTERVAL]);
        ArcRecStore.value[DATA_INDEX_windGustDir]    = (float)sensorGetWhenHigh(&sample[SENSOR_WGUST]);
    }

    // These are conditional based on loop data being populated:
    if (work->loopPkt.radiation != 0xFFFF)
        ArcRecStore.value[DATA_INDEX_radiation]      = (float)sensorGetAvg (&sample[SENSOR_SOLRAD]);
    if (work->loopPkt.UV != 0xFFFF)
        ArcRecStore.value[DATA_INDEX_UV]             = (float)sensorGetAvg (&sample[SENSOR_UV]);
    if (work->loopPkt.sampleET != ARCHIVE_VALUE_NULL)
        ArcRecStore.value[DATA_INDEX_ET]             = (float)sensorGetCumulative (&sample[SENSOR_ET]);
    if (work->loopPkt.wxt510Hail != ARCHIVE_VALUE_NULL)
        ArcRecStore.value[DATA_INDEX_hail]           = (float)sensorGetCumulative (&sample[SENSOR_HAIL]);
    if (work->loopPkt.wxt510Hailrate != ARCHIVE_VALUE_NULL)
        ArcRecStore.value[DATA_INDEX_hailrate]       = (float)sensorGetHigh (&sample[SENSOR_HAILRATE]);


    // Get a few directly from the last LOOP_PKT:
    if (work->loopPkt.rxCheckPercent != 0xFFFF)
        ArcRecStore.value[DATA_INDEX_rxCheckPercent] = (float)work->loopPkt.rxCheckPercent;
    if (work->loopPkt.wxt510HeatingTemp != ARCHIVE_VALUE_NULL)
        ArcRecStore.value[DATA_INDEX_heatingTemp]    = (float)work->loopPkt.wxt510HeatingTemp;
    if (work->loopPkt.wxt510HeatingVoltage != ARCHIVE_VALUE_NULL)
        ArcRecStore.value[DATA_INDEX_heatingVoltage] = (float)work->loopPkt.wxt510HeatingVoltage;
    if (work->loopPkt.wxt510SupplyVoltage != ARCHIVE_VALUE_NULL)
        ArcRecStore.value[DATA_INDEX_supplyVoltage]  = (float)work->loopPkt.wxt510SupplyVoltage;
    if (work->loopPkt.wxt510ReferenceVoltage != ARCHIVE_VALUE_NULL)
        ArcRecStore.value[DATA_INDEX_referenceVoltage] = (float)work->loopPkt.wxt510ReferenceVoltage;
    if (work->loopPkt.wmr918WindBatteryStatus != 0xFF)
        ArcRecStore.value[DATA_INDEX_windBatteryStatus] = (float)work->loopPkt.wmr918WindBatteryStatus;
    if (work->loopPkt.wmr918RainBatteryStatus != 0xFF)
        ArcRecStore.value[DATA_INDEX_rainBatteryStatus] = (float)work->loopPkt.wmr918RainBatteryStatus;
    if (work->loopPkt.wmr918OutTempBatteryStatus != 0xFF)
        ArcRecStore.value[DATA_INDEX_outTempBatteryStatus] = (float)work->loopPkt.wmr918OutTempBatteryStatus;
    if (work->loopPkt.wmr918InTempBatteryStatus != 0xFF)
        ArcRecStore.value[DATA_INDEX_inTempBatteryStatus] = (float)work->loopPkt.wmr918InTempBatteryStatus;

    if (work->loopPkt.extraTemp[0] != ARCHIVE_VALUE_NULL)
        ArcRecStore.value[DATA_INDEX_extraTemp1] = (float)work->loopPkt.extraTemp[0];
    if (work->loopPkt.extraTemp[1] != ARCHIVE_VALUE_NULL)
        ArcRecStore.value[DATA_INDEX_extraTemp2] = (float)work->loopPkt.extraTemp[1];
    if (work->loopPkt.extraTemp[2] != ARCHIVE_VALUE_NULL)
        ArcRecStore.value[DATA_INDEX_extraTemp3]= (float)work->loopPkt.extraTemp[2];
    if (work->loopPkt.extraHumidity[0] != ARCHIVE_VALUE_NULL)
        ArcRecStore.value[DATA_INDEX_extraHumid1] = (float)work->loopPkt.extraHumidity[0];
    if (work->loopPkt.extraHumidity[1] != ARCHIVE_VALUE_NULL)
        ArcRecStore.value[DATA_INDEX_extraHumid2] = (float)work->loopPkt.extraHumidity[1];

    return &ArcRecStore;
}

void computedDataClearInterval (WVIEWD_WORK *work)
{
    // clear our interval store to start the next cycle
    sensorClearSet (work->sensors.sensor[STF_INTERVAL]);

    windAverageReset (&work->sensors.wind[STF_INTERVAL]);
    return;
}


// initialize the computed values from the archive records
int computedDataInit (WVIEWD_WORK *work)
{
    time_t          lastArchiveTime, firstArchiveTime, nowtime = time(NULL);
    struct tm       bknnowtime;
    int             rainyear;
    ARCHIVE_PKT     recordStore;

    localtime_r(&nowtime, &bknnowtime);
    rainyear = bknnowtime.tm_year + 1900;
    if (work->stationRainSeasonStart > (bknnowtime.tm_mon + 1))
    {
        // we need to go back a year...
        rainyear --;
    }

    radMsgLog (PRI_STATUS, "initializing computed data values...");

    memset (&cdWork, 0, sizeof (cdWork));

    // initialize the stores:
    windAverageReset (&work->sensors.wind[STF_ALL]);
    windAverageReset (&work->sensors.wind[STF_YEAR]);
    windAverageReset (&work->sensors.wind[STF_MONTH]);
    windAverageReset (&work->sensors.wind[STF_WEEK]);
    windAverageReset (&work->sensors.wind[STF_DAY]);
    windAverageReset (&work->sensors.wind[STF_HOUR]);

    sensorClearSet (work->sensors.sensor[STF_ALL]);
    sensorClearSet (work->sensors.sensor[STF_YEAR]);
    sensorClearSet (work->sensors.sensor[STF_MONTH]);
    sensorClearSet (work->sensors.sensor[STF_WEEK]);
    sensorClearSet (work->sensors.sensor[STF_DAY]);
    sensorClearSet (work->sensors.sensor[STF_HOUR]);

    lastArchiveTime = 0;
    firstArchiveTime = dbsqliteArchiveGetNextRecord(0, &recordStore);

    // Only compute if there are archive records:
    if ((int)firstArchiveTime != ERROR)
    {
        // update ALL data types to get started (ignore failures)
        computeDataAllTime(work, firstArchiveTime);
        computeDataYear(work, lastArchiveTime);
    
        // Add in any preset values if the year is right:
        if (work->stationRainETPresetYear == rainyear)
        {
            sensorAddCumulative (&work->sensors.sensor[STF_YEAR][SENSOR_RAIN],
                                 work->stationRainYTDPreset);
            sensorAddCumulative (&work->sensors.sensor[STF_YEAR][SENSOR_ET],
                                 work->stationETYTDPreset);
        }
    
        computeDataMonth (work, lastArchiveTime);
        computeDataWeek (work, lastArchiveTime);
        computeDataDay (work, lastArchiveTime);
        computeDataHour (work, lastArchiveTime);
    }

    sensorClearSet (work->sensors.sensor[STF_INTERVAL]);

    // initialize per-interval housekeeping
    intervalHousekeepingInit (work);

    // save the current time so we will know when things need to be updated
    nowtime -= (work->archiveInterval * 60);
    localtime_r (&nowtime, &bknnowtime);
    cdWork.currentHour      = bknnowtime.tm_hour;
    cdWork.currentDay       = bknnowtime.tm_mday;
    cdWork.currentMonth     = bknnowtime.tm_mon;
    cdWork.currentYear      = bknnowtime.tm_year;

    return OK;
}

void computedDataExit (WVIEWD_WORK *work)
{
    int         rainyear;
    time_t      nowtime = time(NULL);
    struct tm   bkntimenow;

    localtime_r(&nowtime, &bkntimenow);
    rainyear = bkntimenow.tm_year + 1900;
    if (work->stationRainSeasonStart > (bkntimenow.tm_mon + 1))
    {
        // we need to go back a year...
        rainyear --;
    }

    return;
}

// update the computed values based on a new archive interval:
int computedDataUpdate (WVIEWD_WORK *work)
{
    time_t          timenow = time(NULL);
    struct tm       bkntimenow;

    // do some per-interval housekeeping
    intervalHousekeeping (work);

    // subtract an archive interval because the period 0 - (archiveInterval - 1)
    // minutes of each hour is still really the previous hour's data
    timenow -= (work->archiveInterval * 60);
    localtime_r (&timenow, &bkntimenow);

    // always update the "change" values
    computeDataChanges (work);

    // start at the lowest timeframe, that way we can bail out early

    // has the hour changed?
    if (bkntimenow.tm_hour != cdWork.currentHour)
    {
        // save the new hour
        cdWork.currentHour = bkntimenow.tm_hour;

        // update the new hour
        addRecordHour (work, TRUE);

        // has the day changed?
        if (bkntimenow.tm_mday != cdWork.currentDay)
        {
            // save the new day
            cdWork.currentDay = bkntimenow.tm_mday;

            // update the last week
            computeDataWeek (work, 0);

            // update the last day
            addRecordDay (work, TRUE);

            // has the month changed?
            if (bkntimenow.tm_mon != cdWork.currentMonth)
            {
                // save the new month
                cdWork.currentMonth = bkntimenow.tm_mon;

                // update the month
                addRecordMonth (work, TRUE);

                // has the year changed?
                if (bkntimenow.tm_year != cdWork.currentYear)
                {
                    // save the new year
                    cdWork.currentYear = bkntimenow.tm_year;

                    // update the year
                    addRecordYear (work, TRUE);
                    addRecordAllTime(work, FALSE);
                }
                else
                {
                    // just add record to year
                    addRecordYear (work, FALSE);
                    addRecordAllTime(work, FALSE);
                }

                // has the rain season rolled over?
                if ((cdWork.currentMonth+1) == work->stationRainSeasonStart)
                {
                    // start the new rain season year!
                    resetRainRecordYear (work);
                }
            }
            else
            {
                // just add record to month and year
                addRecordMonth (work, FALSE);
                addRecordYear (work, FALSE);
                addRecordAllTime(work, FALSE);
            }
        }
        else
        {
            // just add record to day, week, month and year
            addRecordDay (work, FALSE);
            addRecordWeek (work);
            addRecordMonth (work, FALSE);
            addRecordYear (work, FALSE);
            addRecordAllTime(work, FALSE);
        }
    }
    else
    {
        // just add record to hour, day, week, month and year
        addRecordHour (work, FALSE);
        addRecordDay (work, FALSE);
        addRecordWeek (work);
        addRecordMonth (work, FALSE);
        addRecordYear (work, FALSE);
        addRecordAllTime(work, FALSE);
    }

    return OK;
}

// update the computed values based on a new archive record:
int computedDataNewArchive (WVIEWD_WORK *work, ARCHIVE_PKT *newRecord)
{
#if 0
    float           carryOverRain = 0, carryOverET = 0;

    if (newRecord == NULL)
    {
        return ERROR;
    }

    // save trace accumulator amounts:
    if (newRecord->value[DATA_INDEX_rain] > ARCHIVE_VALUE_NULL)
    {
        carryOverRain = sensorGetCumulative(&work->sensors.sensor[STF_INTERVAL][SENSOR_RAIN]);
        carryOverRain -= (float)newRecord->value[DATA_INDEX_rain];
        if (carryOverRain < 0)
        {
            carryOverRain = 0;
        }
    }

    if (newRecord->value[DATA_INDEX_ET] > ARCHIVE_VALUE_NULL)
    {
        carryOverET = sensorGetCumulative(&work->sensors.sensor[STF_INTERVAL][SENSOR_ET]);
        carryOverET -= (float)newRecord->value[DATA_INDEX_ET];
        if (carryOverET < 0)
        {
            carryOverET = 0;
        }
    }

    // normalize the accumulator values to the archive record
    sensorUpdateCumulative (&work->sensors.sensor[STF_INTERVAL][SENSOR_RAIN],
                            (float)newRecord->value[DATA_INDEX_rain]);
    sensorUpdateCumulative (&work->sensors.sensor[STF_INTERVAL][SENSOR_ET],
                            (float)newRecord->value[DATA_INDEX_ET]);

    // store the accumulator trace amounts for the next clearInterval:
    work->carryOverRain = carryOverRain;
    work->carryOverET = carryOverET;
#endif

    return OK;
}
