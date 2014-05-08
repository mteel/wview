/*---------------------------------------------------------------------------
 
  FILENAME:
        sensor.c
 
  PURPOSE:
        Provide the sensor API methods.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        1/02/2006       M.S. Teel       0               Original
 
  NOTES:
        
 
  LICENSE:
        Copyright (c) 2006, Mark S. Teel (mark@teel.ws)
  
        This source code is released for free distribution under the terms 
        of the GNU General Public License.
  
----------------------------------------------------------------------------*/

//  ... System header files
#include <errno.h>
#include <radmsgLog.h>


//  ... Local header files
#include <sensor.h>


//  ... methods

void sensorInit (WV_SENSOR *sensor)
{
    memset (sensor, 0, sizeof(*sensor));
    sensor->low = 1000000.0;
    sensor->high = -1000000.0;
    return;
}

void sensorUpdate (WV_SENSOR *sensor, float value)
{
    if (value <= ARCHIVE_VALUE_NULL)
    {
        return;
    }

    if (sensor->low > value)
    {
        sensor->low = value;
        sensor->time_low = time (NULL);
    }
    if (sensor->high < value)
    {
        sensor->high = value;
        sensor->time_high = time (NULL);
    }
    sensor->cumulative += value;
    sensor->samples ++;
    return;
}

void sensorUpdateWhen (WV_SENSOR *sensor, float value, float whenVal)
{
    if (value <= ARCHIVE_VALUE_NULL)
    {
        return;
    }

    if (sensor->low > value)
    {
        sensor->low = value;
        sensor->time_low = time (NULL);
    }
    if (sensor->high < value)
    {
        sensor->high = value;
        sensor->time_high = time (NULL);
        sensor->when_high = whenVal;
    }
    sensor->cumulative += value;
    sensor->samples ++;
    return;
}

// Initialize sensor with given values:
void sensorSetValues
(
    WV_SENSOR           *sensor,
    float               low,
    time_t              time_low,
    float               high,
    time_t              time_high,
    float               when_high,
    float               cumulative,
    int                 samples
)
{
    sensor->low         = low;
    sensor->time_low    = time_low;
    sensor->high        = high;
    sensor->time_high   = time_high;
    sensor->when_high   = when_high;
    sensor->cumulative  = cumulative;
    sensor->samples     = samples;
}

void sensorUpdateLowValue (WV_SENSOR *sensor, float value)
{
    if (value <= ARCHIVE_VALUE_NULL)
    {
        return;
    }

    sensor->low = value;
}

void sensorUpdateHighValue (WV_SENSOR *sensor, float value)
{
    if (value <= ARCHIVE_VALUE_NULL)
    {
        return;
    }

    sensor->high = value;
}

void sensorUpdateWhenHighValue (WV_SENSOR *sensor, float value, float whenVal)
{
    if (value <= ARCHIVE_VALUE_NULL)
    {
        return;
    }

    sensor->high = value;
    sensor->when_high = whenVal;
}

void sensorUpdateCumulative (WV_SENSOR *sensor, float value)
{
    if (value <= ARCHIVE_VALUE_NULL)
    {
        return;
    }

    sensor->cumulative = value;
}

void sensorAddCumulative (WV_SENSOR *sensor, float value)
{
    if (value <= ARCHIVE_VALUE_NULL)
    {
        return;
    }

    sensor->cumulative += value;
}

void sensorAddSample (WV_SENSOR *sensor, WV_SENSOR *sample, int DoSensorDbg)
{
    if (DoSensorDbg)
    {
        radMsgLog(PRI_HIGH, "sensorAddSample: Low %f@%2.2d:%2.2d  High %f@%2.2d:%2.2d",
                  sample->low, wvutilsGetHour(sample->time_low), wvutilsGetMin(sample->time_low),
                  sample->high, wvutilsGetHour(sample->time_high), wvutilsGetMin(sample->time_high));
    }
    if (sample->low <= ARCHIVE_VALUE_NULL || sample->high <= ARCHIVE_VALUE_NULL)
    {
        if (DoSensorDbg)
        {
            radMsgLog(PRI_HIGH, "sensorAddSample: invalid high or low!");
        }
        return;
    }

    if (sensor->low > sample->low)
    {
    if (DoSensorDbg)
        {
            radMsgLog(PRI_HIGH, "sensorAddSample: New Low: %f@%2.2d:%2.2d",
                      sample->low, wvutilsGetHour(sample->time_low), wvutilsGetMin(sample->time_low));
        }
        sensor->low = sample->low;
        sensor->time_low = sample->time_low;
    }
    if (sensor->high < sample->high)
    {
    if (DoSensorDbg)
    {
        radMsgLog(PRI_HIGH, "sensorAddSample: New High: %f@%2.2d:%2.2d",
                  sample->high, wvutilsGetHour(sample->time_high), wvutilsGetMin(sample->time_high));
    }
    sensor->high = sample->high;
        sensor->when_high = sample->when_high;
        sensor->time_high = sample->time_high;
    }

    sensor->cumulative += sample->cumulative;
    sensor->samples += sample->samples;

    return;
}

void sensorPropogateSample (WV_SENSOR *set, WV_SENSOR *sample)
{
    int         type;

    for (type = 0; type < SENSOR_MAX; type ++)
    {
        // Pass TRUE here to debug:
        sensorAddSample (&set[type], &sample[type], FALSE);
    }

    return;
}

void sensorClearSet (WV_SENSOR *set)
{
    int         type;

    for (type = 0; type < SENSOR_MAX; type ++)
    {
        sensorInit (&set[type]);
    }

    return;
}

float sensorGetLow (WV_SENSOR *sensor)
{
    return sensor->low;
}

char *sensorGetLowTime (WV_SENSOR *sensor, char *store)
{
    if (sensor->time_low == (time_t)0)
    {
        sprintf (store, "-----");
    }
    else
    {
        if (sensor->debug)
        {
            radMsgLog(PRI_MEDIUM, "SENSOR LOWTIME DBG: "
                      "low: %.2f, time_low: %u, high: %.2f, time_high: %u,"
                      "samples: %d",
                      sensor->low,
                      (unsigned int)sensor->time_low,
                      sensor->high,
                      (unsigned int)sensor->time_high,
                      sensor->samples);
        }

        sprintf (store, "%2.2d:%2.2d", wvutilsGetHour(sensor->time_low), wvutilsGetMin(sensor->time_low));
        if (sensor->debug)
        {
            radMsgLog(PRI_MEDIUM, "SENSOR LOWTIME RESULT DBG: %s", store);
        }
    }
    return store;
}

char *sensorGetLowDate (WV_SENSOR *sensor, char *store, char *dateFormat)
{
    struct tm       locTime;

    if (sensor->time_low == (time_t)0)
    {
        sprintf (store, "----------");
    }
    else
    {
        if (sensor->debug)
        {
            radMsgLog(PRI_MEDIUM, "SENSOR LOWDATE DBG: "
                      "low: %.2f, time_low: %u, high: %.2f, time_high: %u,"
                      "samples: %d",
                      sensor->low,
                      (unsigned int)sensor->time_low,
                      sensor->high,
                      (unsigned int)sensor->time_high,
                      sensor->samples);
        }

        localtime_r(&sensor->time_low, &locTime);
        strftime(store, WV_MAX_DATE_LENGTH-1, dateFormat, &locTime);
        if (sensor->debug)
        {
            radMsgLog(PRI_MEDIUM, "SENSOR LOWDATE RESULT DBG: %s", store);
        }
    }
    return store;
}

float sensorGetHigh (WV_SENSOR *sensor)
{
    return sensor->high;
}

float sensorGetWhenHigh (WV_SENSOR *sensor)
{
    return sensor->when_high;
}

char *sensorGetHighTime (WV_SENSOR *sensor, char *store)
{
    if (sensor->time_high == (time_t)0)
    {
        sprintf (store, "-----");
    }
    else
    {
        if (sensor->debug)
        {
            radMsgLog(PRI_MEDIUM, "SENSOR HIGHTIME DBG: "
                      "low: %.2f, time_low: %u, high: %.2f, time_high: %u,"
                      "samples: %d",
                      sensor->low,
                      (unsigned int)sensor->time_low,
                      sensor->high,
                      (unsigned int)sensor->time_high,
                      sensor->samples);
        }

        sprintf (store, "%2.2d:%2.2d", wvutilsGetHour(sensor->time_high), wvutilsGetMin(sensor->time_high));
        if (sensor->debug)
        {
            radMsgLog(PRI_MEDIUM, "SENSOR HIGHTIME RESULT DBG: %s", store);
        }
    }
    return store;
}

char *sensorGetHighDate (WV_SENSOR *sensor, char *store, char *dateFormat)
{
    struct tm       locTime;

    if (sensor->time_high == (time_t)0)
    {
        sprintf (store, "----------");
    }
    else
    {
        if (sensor->debug)
        {
            radMsgLog(PRI_MEDIUM, "SENSOR HIGHDATE DBG: "
                      "low: %.2f, time_low: %u, high: %.2f, time_high: %u,"
                      "samples: %d",
                      sensor->low,
                      (unsigned int)sensor->time_low,
                      sensor->high,
                      (unsigned int)sensor->time_high,
                      sensor->samples);
        }

        localtime_r(&sensor->time_high, &locTime);
        strftime(store, WV_MAX_DATE_LENGTH-1, dateFormat, &locTime);
        if (sensor->debug)
        {
            radMsgLog(PRI_MEDIUM, "SENSOR HIGHDATE RESULT DBG: %s", store);
        }
    }
    return store;
}

float sensorGetAvg (WV_SENSOR *sensor)
{
    if (sensor->samples == 0)
    {
        return 0;
    }

    return (sensor->cumulative/sensor->samples);
}

float sensorGetCumulative (WV_SENSOR *sensor)
{
    return sensor->cumulative;
}

int sensorGetSamples (WV_SENSOR *sensor)
{
    return sensor->samples;
}

// get a daily low value based on the RTF_INTERVAL and RTF_DAY time frames - 
// assumes input of type "set[STF_MAX][SENSOR_MAX]" and "SENSOR_TYPES"
float sensorGetDailyLow (WV_SENSOR set[STF_MAX][SENSOR_MAX], SENSOR_TYPES type)
{
    float       intVal, dayVal;

    intVal = sensorGetLow (&set[STF_INTERVAL][type]);
    dayVal = sensorGetLow (&set[STF_DAY][type]);
    if (intVal < dayVal)
        return intVal;
    else
        return dayVal;
}

char *sensorGetDailyLowTime (WV_SENSOR set[STF_MAX][SENSOR_MAX], SENSOR_TYPES type, char *store)
{
    float       intVal, dayVal;

    intVal = sensorGetLow (&set[STF_INTERVAL][type]);
    dayVal = sensorGetLow (&set[STF_DAY][type]);
    if (intVal < dayVal)
        return sensorGetLowTime (&set[STF_INTERVAL][type], store);
    else
        return sensorGetLowTime (&set[STF_DAY][type], store);
}

// get a daily high value based on the RTF_INTERVAL and RTF_DAY time frames - 
// assumes input of type "set[STF_MAX][SENSOR_MAX]" and "SENSOR_TYPES"
float sensorGetDailyHigh (WV_SENSOR set[STF_MAX][SENSOR_MAX], SENSOR_TYPES type)
{
    float       intVal, dayVal;

    intVal = sensorGetHigh (&set[STF_INTERVAL][type]);
    dayVal = sensorGetHigh (&set[STF_DAY][type]);
    if (intVal > dayVal)
        return intVal;
    else
        return dayVal;
}

char *sensorGetDailyHighTime (WV_SENSOR set[STF_MAX][SENSOR_MAX], SENSOR_TYPES type, char *store)
{
    float       intVal, dayVal;

    intVal = sensorGetHigh (&set[STF_INTERVAL][type]);
    dayVal = sensorGetHigh (&set[STF_DAY][type]);
    if (intVal > dayVal)
        return sensorGetHighTime (&set[STF_INTERVAL][type], store);
    else
        return sensorGetHighTime (&set[STF_DAY][type], store);
}

float sensorGetDailyWhenHigh (WV_SENSOR set[STF_MAX][SENSOR_MAX], SENSOR_TYPES type)
{
    float       intVal, dayVal;

    intVal = sensorGetHigh (&set[STF_INTERVAL][type]);
    dayVal = sensorGetHigh (&set[STF_DAY][type]);
    if (intVal > dayVal)
        return sensorGetWhenHigh (&set[STF_INTERVAL][type]);
    else
        return sensorGetWhenHigh (&set[STF_DAY][type]);
}

// get a daily cumulative value based on the RTF_INTERVAL and RTF_DAY time frames - 
float sensorGetDailyCumulative (WV_SENSOR set[STF_MAX][SENSOR_MAX], SENSOR_TYPES type)
{
    return (sensorGetCumulative(&set[STF_INTERVAL][type]) + sensorGetCumulative(&set[STF_DAY][type]));
}

// some special purpose utilities
float sensorGetWindRun (SENSOR_TIMEFRAMES frame, WV_SENSOR *sensor)
{
    float       retVal, temp;
    time_t      nowtime, jan1time;
    struct tm   bknTime, tempTime;

    if (sensor->samples == 0)
    {
        return 0;
    }

    nowtime = time(NULL);
    localtime_r(&nowtime, &bknTime);
    tempTime = bknTime;
    tempTime.tm_sec = 0;
    tempTime.tm_min = 0;
    tempTime.tm_hour = 0;
    tempTime.tm_mday = 0;
    tempTime.tm_mon = 0;
    jan1time = mktime(&tempTime);

    retVal = sensor->cumulative/sensor->samples;

    switch (frame)
    {
        case STF_HOUR:
            retVal *= (60*bknTime.tm_min + bknTime.tm_sec);
            break;
        case STF_DAY:
            retVal *= (3600*bknTime.tm_hour + 60*bknTime.tm_min + bknTime.tm_sec);
            break;
        case STF_WEEK:
            temp = WV_SECONDS_IN_DAY * 6;
            temp += (3600*bknTime.tm_hour + 60*bknTime.tm_min + bknTime.tm_sec);
            retVal *= temp;
            break;
        case STF_MONTH:
            retVal *= (86400*bknTime.tm_mday + 3600*bknTime.tm_hour + 60*bknTime.tm_min + bknTime.tm_sec);
            break;
        case STF_YEAR:
            retVal *= (nowtime - jan1time);
            break;
        default:
            break;
    }

    retVal /= 3600;
    return retVal;
}

static void AgeAccumulator (WV_ACCUM_ID id)
{
    WV_ACCUM_SAMPLE     *nodePtr, *oldPtr;
    time_t              nowTime = time(NULL);

    for (nodePtr = (WV_ACCUM_SAMPLE*)radListGetLast (&id->samples);
         nodePtr != NULL;
         nodePtr = (WV_ACCUM_SAMPLE*)radListGetLast (&id->samples))
    {
        if ((nowTime - id->secondsInAccumulator) >= nodePtr->sampleTime)
        {
            // remove and free him
            oldPtr = (WV_ACCUM_SAMPLE*)radListRemoveLast (&id->samples);
            radBufferRls (oldPtr);
        }
        else
        {
            break;
        }
    }
}

WV_ACCUM_ID sensorAccumInit (int minutesInAccumulator)
{
    WV_ACCUM_ID         newId;

    newId = (WV_ACCUM_ID)radBufferGet (sizeof (WV_ACCUM));
    if (newId == NULL)
    {
        return NULL;
    }

    radListReset (&newId->samples);
    newId->secondsInAccumulator = minutesInAccumulator * 60;
    newId->sum = 0;
    return newId;
}

void sensorAccumExit (WV_ACCUM_ID id)
{
    WV_ACCUM_SAMPLE*    nodePtr;

    for (nodePtr = (WV_ACCUM_SAMPLE*)radListRemoveFirst (&id->samples);
         nodePtr != NULL;
         nodePtr = (WV_ACCUM_SAMPLE*)radListRemoveFirst (&id->samples))
    {
        radBufferRls (nodePtr);
    }
    radBufferRls (id);
}

void sensorAccumAddSample (WV_ACCUM_ID id, time_t timeStamp, float value)
{
    WV_ACCUM_SAMPLE*    newNode;

    newNode = (WV_ACCUM_SAMPLE*)radBufferGet (sizeof(WV_ACCUM_SAMPLE));
    if (newNode == NULL)
    {
        return;
    }

    newNode->value      = value;
    newNode->sampleTime = timeStamp;
    radListAddToFront (&id->samples, (NODE_PTR)newNode);

    // Do we need to age off any nodes?
    AgeAccumulator (id);
}

float sensorAccumGetTotal (WV_ACCUM_ID id)
{
    WV_ACCUM_SAMPLE*    nodePtr;
    float               sum = 0;

    // Do we need to age off any nodes?
    AgeAccumulator (id);

    for (nodePtr = (WV_ACCUM_SAMPLE*)radListGetFirst (&id->samples);
         nodePtr != NULL;
         nodePtr = (WV_ACCUM_SAMPLE*)radListGetNext (&id->samples, (NODE_PTR)nodePtr))
    {
        sum += nodePtr->value;
    }

    return sum;
}

float sensorAccumGetAverage (WV_ACCUM_ID id)
{
    float               samples, sum;

    sum = sensorAccumGetTotal(id);
    samples = (float)radListGetNumberOfNodes(&id->samples);
    if (samples > 0)
    {
        return (sum/samples);
    }
    else
    {
        return 0;
    }
}

static char sensorDebugString[256];
char* sensorGetString (WV_SENSOR *sensor)
{
    snprintf (sensorDebugString, 256, "Low:%.3f@%2.2d:%2.2d High:%.3f@%2.2d:%2.2d Cum:%.3f Samples:%d",
              sensor->low, wvutilsGetHour(sensor->time_low), wvutilsGetMin(sensor->time_low),
              sensor->high, wvutilsGetHour(sensor->time_high), wvutilsGetMin(sensor->time_high),
              sensor->cumulative, sensor->samples);
    return sensorDebugString;
}

