/*---------------------------------------------------------------------------
 
  FILENAME:
        images.c
 
  PURPOSE:
        Provide the built-in wview image generator utilities.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        01/20/04        M.S. Teel       0               Original
 
  NOTES:
        
 
  LICENSE:
        Copyright (c) 2004, Mark S. Teel (mark@teel.ws)
  
        This source code is released for free distribution under the terms 
        of the GNU General Public License.
  
----------------------------------------------------------------------------*/

/*  ... System include files
*/
#include <termios.h>

/*  ... Library include files
*/
#include <radmsgLog.h>
#include <radtimeUtils.h>

/*  ... Local include files
*/
#include <services.h>
#include <html.h>
#include <htmlMgr.h>
#include <htmlGenerate.h>
#include <glbucket.h>
#include <glchart.h>
#include <glmultichart.h>
#include <images.h>


/*  ... global memory declarations
*/

/*  ... global memory referenced
*/
extern char sampleLabels[MAX_DAILY_NUM_VALUES][8];
extern char sampleHourLabels[MAX_DAILY_NUM_VALUES][8];

/*  ... (local) memory declarations
*/
static char *labels[MONTHLY_NUM_VALUES];

static char *monthLabels[12] =
    {
        "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"
    };


//  ... !!!  Define the image generator engines  !!! ...
//  ....................................................

static int generateTemp (HTML_IMG *img)
{
    char        temp[256];

    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    if (img->mgrWork->isMetricUnits)
    {
        return (htmlGenPngBucket (temp,
                          1,
                          wvutilsConvertFToC (img->mgrWork->loopStore.outTemp),
                          wvutilsConvertFToC (sensorGetDailyLow(img->mgrWork->hilowStore.sensor, SENSOR_OUTTEMP)),
                          wvutilsConvertFToC (sensorGetDailyHigh(img->mgrWork->hilowStore.sensor, SENSOR_OUTTEMP)),
                          img->title,
                          img->units,
                          img->decimalPlaces,
                          -20.0,
                          40.0,
                          img->mgrWork->dateFormat,
                          img->mgrWork->isDualUnits));
    }
    else
    {
        return (htmlGenPngBucket (temp,
                          0,
                          img->mgrWork->loopStore.outTemp,
                          sensorGetDailyLow(img->mgrWork->hilowStore.sensor, SENSOR_OUTTEMP),
                          sensorGetDailyHigh(img->mgrWork->hilowStore.sensor, SENSOR_OUTTEMP),
                          img->title,
                          img->units,
                          img->decimalPlaces,
                          0.0,
                          99.0,
                          img->mgrWork->dateFormat,
                          img->mgrWork->isDualUnits));
    }
}

static int generateHumidity (HTML_IMG *img)
{
    char        temp[256];

    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    return (htmlGenPngPercentBucket (temp,
                             img->mgrWork->isMetricUnits,
                             (float)img->mgrWork->loopStore.outHumidity,
                             sensorGetDailyLow(img->mgrWork->hilowStore.sensor, SENSOR_OUTHUMID),
                             sensorGetDailyHigh(img->mgrWork->hilowStore.sensor, SENSOR_OUTHUMID),
                             img->title,
                             img->units,
                             img->decimalPlaces,
                             img->mgrWork->dateFormat,
                             img->mgrWork->isDualUnits));
}

static int generateDewpoint (HTML_IMG *img)
{
    char        temp[256];

    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    if (img->mgrWork->isMetricUnits)
    {
        return (htmlGenPngBucket (temp,
                          1,
                          wvutilsConvertFToC (img->mgrWork->loopStore.dewpoint),
                          wvutilsConvertFToC (sensorGetDailyLow(img->mgrWork->hilowStore.sensor, SENSOR_DEWPOINT)),
                          wvutilsConvertFToC (sensorGetDailyHigh(img->mgrWork->hilowStore.sensor, SENSOR_DEWPOINT)),
                          img->title,
                          img->units,
                          img->decimalPlaces,
                          -20.0,
                          40.0,
                          img->mgrWork->dateFormat,
                          img->mgrWork->isDualUnits));
    }
    else
    {
        return (htmlGenPngBucket (temp,
                          0,
                          img->mgrWork->loopStore.dewpoint,
                          sensorGetDailyLow(img->mgrWork->hilowStore.sensor, SENSOR_DEWPOINT),
                          sensorGetDailyHigh(img->mgrWork->hilowStore.sensor, SENSOR_DEWPOINT),
                          img->title,
                          img->units,
                          img->decimalPlaces,
                          0.0,
                          99.0,
                          img->mgrWork->dateFormat,
                          img->mgrWork->isDualUnits));
    }
}

static int generateWindChill (HTML_IMG *img)
{
    char        temp[256];
    float       low = MIN(img->mgrWork->loopStore.windchill,
                          sensorGetDailyLow(img->mgrWork->hilowStore.sensor, SENSOR_WCHILL));

    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    if (img->mgrWork->isMetricUnits)
    {
        return (htmlGenPngBucket (temp,
                          1,
                          wvutilsConvertFToC (img->mgrWork->loopStore.windchill),
                          wvutilsConvertFToC (low),
                          (float)GLB_HILOW_NONE,
                          img->title,
                          img->units,
                          img->decimalPlaces,
                          -20.0,
                          40.0,
                          img->mgrWork->dateFormat,
                          img->mgrWork->isDualUnits));
    }
    else
    {
        return (htmlGenPngBucket (temp,
                          0,
                          img->mgrWork->loopStore.windchill,
                          low,
                          (float)GLB_HILOW_NONE,
                          img->title,
                          img->units,
                          img->decimalPlaces,
                          0.0,
                          99.0,
                          img->mgrWork->dateFormat,
                          img->mgrWork->isDualUnits));
    }
}

static int generateHeatIndex (HTML_IMG *img)
{
    char        temp[256];
    float       hi = MAX(img->mgrWork->loopStore.heatindex,
                         sensorGetDailyHigh(img->mgrWork->hilowStore.sensor, SENSOR_HINDEX));

    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    if (img->mgrWork->isMetricUnits)
    {
        return (htmlGenPngBucket (temp,
                          1,
                          wvutilsConvertFToC(img->mgrWork->loopStore.heatindex),
                          (float)GLB_HILOW_NONE,
                          wvutilsConvertFToC (hi),
                          img->title,
                          img->units,
                          img->decimalPlaces,
                          -20.0,
                          40.0,
                          img->mgrWork->dateFormat,
                          img->mgrWork->isDualUnits));
    }
    else
    {
        return (htmlGenPngBucket (temp,
                          0,
                          img->mgrWork->loopStore.heatindex,
                          (float)GLB_HILOW_NONE,
                          hi,
                          img->title,
                          img->units,
                          img->decimalPlaces,
                          0.0,
                          99.0,
                          img->mgrWork->dateFormat,
                          img->mgrWork->isDualUnits));
    }
}

static int generateBarometer (HTML_IMG *img)
{
    char        temp[256];

    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    if (img->mgrWork->isMetricUnits)
    {
        return (htmlGenMetricPngBaromBucket (temp,
                                     wvutilsConvertINHGToHPA (img->mgrWork->loopStore.barometer),
                                     wvutilsConvertINHGToHPA (sensorGetDailyLow(img->mgrWork->hilowStore.sensor, SENSOR_BP)),
                                     wvutilsConvertINHGToHPA (sensorGetDailyHigh(img->mgrWork->hilowStore.sensor, SENSOR_BP)),
                                     img->title,
                                     img->units,
                                     img->decimalPlaces,
                                     img->mgrWork->dateFormat,
                                     img->mgrWork->isDualUnits));
    }
    else
    {
        return (htmlGenPngBaromBucket (temp,
                               img->mgrWork->loopStore.barometer,
                               sensorGetDailyLow(img->mgrWork->hilowStore.sensor, SENSOR_BP),
                               sensorGetDailyHigh(img->mgrWork->hilowStore.sensor, SENSOR_BP),
                               img->title,
                               img->units,
                               img->decimalPlaces,
                               img->mgrWork->dateFormat,
                               img->mgrWork->isDualUnits));
    }
}

static int generateDayRain (HTML_IMG *img)
{
    char        temp[256];
    float       tempfloat;

    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    if (img->mgrWork->isMetricUnits)
    {
        tempfloat = wvutilsConvertRainINToMetric (sensorGetCumulative(&img->mgrWork->hilowStore.sensor[STF_DAY][SENSOR_RAIN]));

        return (htmlGenPngRainBucket (temp,
                              1,
                              tempfloat,
                              (float)GLB_HILOW_NONE,
                              (float)GLB_HILOW_NONE,
                              img->title,
                              img->units,
                              img->decimalPlaces,
                              0.0,
                              0.1,
                              img->mgrWork->dateFormat,
                              img->mgrWork->isDualUnits));
    }
    else
    {
        return (htmlGenPngRainBucket (temp,
                              0,
                              sensorGetCumulative(&img->mgrWork->hilowStore.sensor[STF_DAY][SENSOR_RAIN]),
                              (float)GLB_HILOW_NONE,
                              (float)GLB_HILOW_NONE,
                              img->title,
                              img->units,
                              img->decimalPlaces,
                              0.0,
                              0.1,
                              img->mgrWork->dateFormat,
                              img->mgrWork->isDualUnits));
    }
}

static int generateStormRain (HTML_IMG *img)
{
    char        temp[256];
    float       tempfloat;

    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    if (img->mgrWork->isMetricUnits)
    {
        tempfloat = wvutilsConvertRainINToMetric (img->mgrWork->loopStore.stormRain);

        return (htmlGenPngRainBucket (temp,
                              1,
                              tempfloat,
                              (float)GLB_HILOW_NONE,
                              (float)GLB_HILOW_NONE,
                              img->title,
                              img->units,
                              img->decimalPlaces,
                              0.0,
                              0.1,
                              img->mgrWork->dateFormat,
                              img->mgrWork->isDualUnits));
    }
    else
    {
        return (htmlGenPngRainBucket (temp,
                              0,
                              img->mgrWork->loopStore.stormRain,
                              (float)GLB_HILOW_NONE,
                              (float)GLB_HILOW_NONE,
                              img->title,
                              img->units,
                              img->decimalPlaces,
                              0.0,
                              0.1,
                              img->mgrWork->dateFormat,
                              img->mgrWork->isDualUnits));
    }
}

static int generateRainRate (HTML_IMG *img)
{
    char        temp[256];
    float       tempfloat, tempfloat2;

    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    if (img->mgrWork->isMetricUnits)
    {
        tempfloat = wvutilsConvertRainINToMetric (img->mgrWork->loopStore.rainRate);
        tempfloat2 = wvutilsConvertRainINToMetric (sensorGetDailyHigh(img->mgrWork->hilowStore.sensor, SENSOR_RAINRATE));

        return (htmlGenPngRainBucket (temp,
                              1,
                              tempfloat,
                              (float)GLB_HILOW_NONE,
                              tempfloat2,
                              img->title,
                              img->units,
                              img->decimalPlaces,
                              0.0,
                              0.1,
                              img->mgrWork->dateFormat,
                              img->mgrWork->isDualUnits));
    }
    else
    {
        return (htmlGenPngRainBucket (temp,
                              0,
                              img->mgrWork->loopStore.rainRate,
                              (float)GLB_HILOW_NONE,
                              sensorGetDailyHigh(img->mgrWork->hilowStore.sensor, SENSOR_RAINRATE),
                              img->title,
                              img->units,
                              img->decimalPlaces,
                              0.0,
                              0.1,
                              img->mgrWork->dateFormat,
                              img->mgrWork->isDualUnits));
    }
}

static int generateMonthRain (HTML_IMG *img)
{
    char        temp[256];
    float       tempfloat;

    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    if (img->mgrWork->isMetricUnits)
    {
        tempfloat = wvutilsConvertRainINToMetric (sensorGetCumulative(&img->mgrWork->hilowStore.sensor[STF_MONTH][SENSOR_RAIN]));

        return (htmlGenPngRainBucket (temp,
                              1,
                              tempfloat,
                              (float)GLB_HILOW_NONE,
                              (float)GLB_HILOW_NONE,
                              img->title,
                              img->units,
                              img->decimalPlaces,
                              0.0,
                              0.1,
                              img->mgrWork->dateFormat,
                              img->mgrWork->isDualUnits));
    }
    else
    {
        return (htmlGenPngRainBucket (temp,
                              0,
                              sensorGetCumulative(&img->mgrWork->hilowStore.sensor[STF_MONTH][SENSOR_RAIN]),
                              (float)GLB_HILOW_NONE,
                              (float)GLB_HILOW_NONE,
                              img->title,
                              img->units,
                              img->decimalPlaces,
                              0.0,
                              0.1,
                              img->mgrWork->dateFormat,
                              img->mgrWork->isDualUnits));
    }
}

static int generateYearRain (HTML_IMG *img)
{
    char        temp[256];
    float       tempfloat;

    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    if (img->mgrWork->isMetricUnits)
    {
        tempfloat = wvutilsConvertRainINToMetric (sensorGetCumulative(&img->mgrWork->hilowStore.sensor[STF_YEAR][SENSOR_RAIN]));

        return (htmlGenPngBucket (temp,
                          1,
                          tempfloat,
                          (float)GLB_HILOW_NONE,
                          (float)GLB_HILOW_NONE,
                          img->title,
                          img->units,
                          img->decimalPlaces,
                          0.0,
                          0.5,
                          img->mgrWork->dateFormat,
                          img->mgrWork->isDualUnits));
    }
    else
    {
        return (htmlGenPngBucket (temp,
                          0,
                          sensorGetCumulative(&img->mgrWork->hilowStore.sensor[STF_YEAR][SENSOR_RAIN]),
                          (float)GLB_HILOW_NONE,
                          (float)GLB_HILOW_NONE,
                          img->title,
                          img->units,
                          img->decimalPlaces,
                          0.0,
                          0.5,
                          img->mgrWork->dateFormat,
                          img->mgrWork->isDualUnits));
    }
}

static int generateWind (HTML_IMG *img)
{
    char        temp[256];
    int         retVal;

    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    retVal = htmlGenPngDialWind (temp,
                    (int)img->mgrWork->loopStore.windDir,
                    (int)sensorGetDailyWhenHigh(img->mgrWork->hilowStore.sensor, SENSOR_WGUST),
                    img->mgrWork->loopStore.windSpeedF,
                    sensorGetDailyHigh(img->mgrWork->hilowStore.sensor, SENSOR_WGUST),
                    img->title);

    return retVal;
}

static int generateTempDay (HTML_IMG *img)
{
    int         i, j;
    char        temp[256];
            
    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_SAMPLE))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, j = img->mgrWork->dayStart; i < DAILY_NUM_VALUES(img->mgrWork) - 1; i ++, j ++)
    {
        if (j >= DAILY_NUM_VALUES(img->mgrWork) - 1)
            j = 0;

        labels[i] = sampleLabels[j];
    }
    labels[i] = sampleLabels[DAILY_NUM_VALUES(img->mgrWork) - 1];

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->dayValues[DATA_INDEX_outTemp],
                     6,
                     CHART_MIN_TEMP(img->mgrWork->isMetricUnits),
                     CHART_MAX_TEMP(img->mgrWork->isMetricUnits),
                     1.0,
                     labels,
                     DAILY_NUM_VALUES(img->mgrWork),
                     DAILY_NUM_VALUES(img->mgrWork),
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     img->mgrWork->isDualUnits));
}

static int generateTempWeek (HTML_IMG *img)
{
    int         i, skipNo;
    char        lbls[WEEKLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->weekStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, skipNo = 0; i < WEEKLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if (loctime.tm_hour == 0)
        {
            break;
        }
        else
        {
            skipNo ++;
            ntime += WV_SECONDS_IN_HOUR;
        }
    }

    for (i = 0; i < WEEKLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if ((i % 24) == 0)
        {
            if (loctime.tm_hour != 0)
            {
                // DST fall back nonsense:
                ntime += WV_SECONDS_IN_HOUR;
                localtime_r (&ntime, &loctime);
            }
        }
        sprintf (lbls[i], "%d", loctime.tm_mday);
        labels[i] = lbls[i];

        ntime += WV_SECONDS_IN_HOUR;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     &img->mgrWork->weekValues[DATA_INDEX_outTemp][skipNo],
                     7,
                     CHART_MIN_TEMP(img->mgrWork->isMetricUnits),
                     CHART_MAX_TEMP(img->mgrWork->isMetricUnits),
                     1.0,
                     labels,
                     WEEKLY_NUM_VALUES-skipNo,
                     WEEKLY_NUM_VALUES,
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     img->mgrWork->isDualUnits));
}

static int generateTempMonth (HTML_IMG *img)
{
    int         i, skipNo;
    char        lbls[MONTHLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->monthStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, skipNo = 0; i < MONTHLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if (loctime.tm_hour == 0)
        {
            break;
        }
        else
        {
            skipNo ++;
            ntime += WV_SECONDS_IN_HOUR;
        }
    }

    for (i = 0; i < MONTHLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if ((i % 24) == 0)
        {
            if (loctime.tm_hour != 0)
            {
                // DST fall back nonsense:
                ntime += WV_SECONDS_IN_HOUR;
                localtime_r (&ntime, &loctime);
            }
        }
        sprintf (lbls[i], "%d", loctime.tm_mday);
        labels[i] = lbls[i];

        ntime += WV_SECONDS_IN_HOUR;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     &img->mgrWork->monthValues[DATA_INDEX_outTemp][skipNo],
                     14,
                     CHART_MIN_TEMP(img->mgrWork->isMetricUnits),
                     CHART_MAX_TEMP(img->mgrWork->isMetricUnits),
                     1.0,
                     labels,
                     MONTHLY_NUM_VALUES-skipNo,
                     MONTHLY_NUM_VALUES,
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     img->mgrWork->isDualUnits));
}

static int generateTempYear (HTML_IMG *img)
{
    int         i;
    char        lbls[YEARLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->yearStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_DAY))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    ntime += 4*WV_SECONDS_IN_HOUR;
    for (i = 0; i < YEARLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%s %d", monthLabels[loctime.tm_mon], loctime.tm_mday);
        labels[i] = lbls[i];
        ntime += WV_SECONDS_IN_DAY;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->yearValues[DATA_INDEX_outTemp],
                     4,
                     CHART_MIN_TEMP(img->mgrWork->isMetricUnits),
                     CHART_MAX_TEMP(img->mgrWork->isMetricUnits),
                     1.0,
                     labels,
                     YEARLY_NUM_VALUES,
                     YEARLY_NUM_VALUES,
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     img->mgrWork->isDualUnits));
}

static int generateHumidDay (HTML_IMG *img)
{
    int         i, j;
    char        temp[256];

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_SAMPLE))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, j = img->mgrWork->dayStart; i < DAILY_NUM_VALUES(img->mgrWork) - 1; i ++, j ++)
    {
        if (j >= DAILY_NUM_VALUES(img->mgrWork) - 1)
            j = 0;

        labels[i] = sampleLabels[j];
    }
    labels[i] = sampleLabels[DAILY_NUM_VALUES(img->mgrWork) - 1];

    return (htmlGenPngPercentChart (temp,
                            img->mgrWork->isMetricUnits,
                            img->mgrWork->dayValues[DATA_INDEX_outHumidity],
                            6,
                            labels,
                            DAILY_NUM_VALUES(img->mgrWork),
                            DAILY_NUM_VALUES(img->mgrWork),
                            img->title,
                            img->mgrWork->dateFormat));
}

static int generateHumidWeek (HTML_IMG *img)
{
    int         i,skipNo;
    char        lbls[WEEKLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->weekStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, skipNo = 0; i < WEEKLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if (loctime.tm_hour == 0)
        {
            break;
        }
        else
        {
            skipNo ++;
            ntime += WV_SECONDS_IN_HOUR;
        }
    }

    for (i = 0; i < WEEKLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if ((i % 24) == 0)
        {
            if (loctime.tm_hour != 0)
            {
                // DST fall back nonsense:
                ntime += WV_SECONDS_IN_HOUR;
                localtime_r (&ntime, &loctime);
            }
        }
        sprintf (lbls[i], "%d", loctime.tm_mday);
        labels[i] = lbls[i];

        ntime += WV_SECONDS_IN_HOUR;
    }

    return (htmlGenPngPercentChart (temp,
                            img->mgrWork->isMetricUnits,
                            &img->mgrWork->weekValues[DATA_INDEX_outHumidity][skipNo],
                            7,
                            labels,
                            WEEKLY_NUM_VALUES-skipNo,
                            WEEKLY_NUM_VALUES,
                            img->title,
                            img->mgrWork->dateFormat));
}

static int generateHumidMonth (HTML_IMG *img)
{
    int         i, skipNo;
    char        lbls[MONTHLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->monthStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, skipNo = 0; i < MONTHLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if (loctime.tm_hour == 0)
        {
            break;
        }
        else
        {
            skipNo ++;
            ntime += WV_SECONDS_IN_HOUR;
        }
    }

    for (i = 0; i < MONTHLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if ((i % 24) == 0)
        {
            if (loctime.tm_hour != 0)
            {
                // DST fall back nonsense:
                ntime += WV_SECONDS_IN_HOUR;
                localtime_r (&ntime, &loctime);
            }
        }
        sprintf (lbls[i], "%d", loctime.tm_mday);
        labels[i] = lbls[i];

        ntime += WV_SECONDS_IN_HOUR;
    }

    return (htmlGenPngPercentChart (temp,
                            img->mgrWork->isMetricUnits,
                            &img->mgrWork->monthValues[DATA_INDEX_outHumidity][skipNo],
                            14,
                            labels,
                            MONTHLY_NUM_VALUES-skipNo,
                            MONTHLY_NUM_VALUES,
                            img->title,
                            img->mgrWork->dateFormat));
}

static int generateHumidYear (HTML_IMG *img)
{
    int         i;
    char        lbls[YEARLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->yearStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_DAY))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    ntime += 4*WV_SECONDS_IN_HOUR;
    for (i = 0; i < YEARLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%s %d", monthLabels[loctime.tm_mon], loctime.tm_mday);
        labels[i] = lbls[i];
        ntime += WV_SECONDS_IN_DAY;
    }

    return (htmlGenPngPercentChart (temp,
                            img->mgrWork->isMetricUnits,
                            img->mgrWork->yearValues[DATA_INDEX_outHumidity],
                            4,
                            labels,
                            YEARLY_NUM_VALUES,
                            YEARLY_NUM_VALUES,
                            img->title,
                            img->mgrWork->dateFormat));
}

static int generateDewDay (HTML_IMG *img)
{
    int         i, j;
    char        temp[256];

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_SAMPLE))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, j = img->mgrWork->dayStart; i < DAILY_NUM_VALUES(img->mgrWork) - 1; i ++, j ++)
    {
        if (j >= DAILY_NUM_VALUES(img->mgrWork) - 1)
            j = 0;

        labels[i] = sampleLabels[j];
    }
    labels[i] = sampleLabels[DAILY_NUM_VALUES(img->mgrWork) - 1];

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->dayValues[DATA_INDEX_dewpoint],
                     6,
                     CHART_MIN_TEMP(img->mgrWork->isMetricUnits),
                     CHART_MAX_TEMP(img->mgrWork->isMetricUnits),
                     1.0,
                     labels,
                     DAILY_NUM_VALUES(img->mgrWork),
                     DAILY_NUM_VALUES(img->mgrWork),
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     img->mgrWork->isDualUnits));
}

static int generateDewWeek (HTML_IMG *img)
{
    int         i, skipNo;
    char        lbls[WEEKLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->weekStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, skipNo = 0; i < WEEKLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if (loctime.tm_hour == 0)
        {
            break;
        }
        else
        {
            skipNo ++;
            ntime += WV_SECONDS_IN_HOUR;
        }
    }

    for (i = 0; i < WEEKLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if ((i % 24) == 0)
        {
            if (loctime.tm_hour != 0)
            {
                // DST fall back nonsense:
                ntime += WV_SECONDS_IN_HOUR;
                localtime_r (&ntime, &loctime);
            }
        }
        sprintf (lbls[i], "%d", loctime.tm_mday);
        labels[i] = lbls[i];

        ntime += WV_SECONDS_IN_HOUR;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     &img->mgrWork->weekValues[DATA_INDEX_dewpoint][skipNo],
                     7,
                     CHART_MIN_TEMP(img->mgrWork->isMetricUnits),
                     CHART_MAX_TEMP(img->mgrWork->isMetricUnits),
                     1.0,
                     labels,
                     WEEKLY_NUM_VALUES-skipNo,
                     WEEKLY_NUM_VALUES,
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     img->mgrWork->isDualUnits));
}

static int generateDewMonth (HTML_IMG *img)
{
    int         i, skipNo;
    char        lbls[MONTHLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->monthStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, skipNo = 0; i < MONTHLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if (loctime.tm_hour == 0)
        {
            break;
        }
        else
        {
            skipNo ++;
            ntime += WV_SECONDS_IN_HOUR;
        }
    }

    for (i = 0; i < MONTHLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if ((i % 24) == 0)
        {
            if (loctime.tm_hour != 0)
            {
                // DST fall back nonsense:
                ntime += WV_SECONDS_IN_HOUR;
                localtime_r (&ntime, &loctime);
            }
        }
        sprintf (lbls[i], "%d", loctime.tm_mday);
        labels[i] = lbls[i];

        ntime += WV_SECONDS_IN_HOUR;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     &img->mgrWork->monthValues[DATA_INDEX_dewpoint][skipNo],
                     14,
                     CHART_MIN_TEMP(img->mgrWork->isMetricUnits),
                     CHART_MAX_TEMP(img->mgrWork->isMetricUnits),
                     1.0,
                     labels,
                     MONTHLY_NUM_VALUES-skipNo,
                     MONTHLY_NUM_VALUES,
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     img->mgrWork->isDualUnits));
}

static int generateDewYear (HTML_IMG *img)
{
    int         i;
    char        lbls[YEARLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->yearStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_DAY))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    ntime += 4*WV_SECONDS_IN_HOUR;
    for (i = 0; i < YEARLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%s %d", monthLabels[loctime.tm_mon], loctime.tm_mday);
        labels[i] = lbls[i];
        ntime += WV_SECONDS_IN_DAY;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->yearValues[DATA_INDEX_dewpoint],
                     4,
                     CHART_MIN_TEMP(img->mgrWork->isMetricUnits),
                     CHART_MAX_TEMP(img->mgrWork->isMetricUnits),
                     1.0,
                     labels,
                     YEARLY_NUM_VALUES,
                     YEARLY_NUM_VALUES,
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     img->mgrWork->isDualUnits));
}

static int generateWSpeedDay (HTML_IMG *img)
{
    int         i, j;
    char        temp[256];
    char        units[16];

    // Make units consistent with global preference:
    strncpy(units, wvutilsGetWindUnitLabel(), 15);
    strncpy(img->units, units, 15);

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_SAMPLE))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, j = img->mgrWork->dayStart; i < DAILY_NUM_VALUES(img->mgrWork) - 1; i ++, j ++)
    {
        if (j >= DAILY_NUM_VALUES(img->mgrWork) - 1)
            j = 0;

        labels[i] = sampleLabels[j];
    }
    labels[i] = sampleLabels[DAILY_NUM_VALUES(img->mgrWork) - 1];

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->dayValues[DATA_INDEX_windSpeed],
                     6,
                     0,
                     5,
                     1.0,
                     labels,
                     DAILY_NUM_VALUES(img->mgrWork),
                     DAILY_NUM_VALUES(img->mgrWork),
                     img->title,
                     units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     img->mgrWork->isDualUnits));
}

static int generateWSpeedWeek (HTML_IMG *img)
{
    int         i, skipNo;
    char        lbls[WEEKLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->weekStartTime_T;
    struct tm   loctime;
    char        units[16];

    // Make units consistent with global preference:
    strncpy(units, wvutilsGetWindUnitLabel(), 15);
    strncpy(img->units, units, 15);

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, skipNo = 0; i < WEEKLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if (loctime.tm_hour == 0)
        {
            break;
        }
        else
        {
            skipNo ++;
            ntime += WV_SECONDS_IN_HOUR;
        }
    }

    for (i = 0; i < WEEKLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if ((i % 24) == 0)
        {
            if (loctime.tm_hour != 0)
            {
                // DST fall back nonsense:
                ntime += WV_SECONDS_IN_HOUR;
                localtime_r (&ntime, &loctime);
            }
        }
        sprintf (lbls[i], "%d", loctime.tm_mday);
        labels[i] = lbls[i];

        ntime += WV_SECONDS_IN_HOUR;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     &img->mgrWork->weekValues[DATA_INDEX_windSpeed][skipNo],
                     7,
                     0,
                     5,
                     1.0,
                     labels,
                     WEEKLY_NUM_VALUES-skipNo,
                     WEEKLY_NUM_VALUES,
                     img->title,
                     units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     img->mgrWork->isDualUnits));
}

static int generateWSpeedMonth (HTML_IMG *img)
{
    int         i, skipNo;
    char        lbls[MONTHLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->monthStartTime_T;
    struct tm   loctime;
    char        units[16];

    // Make units consistent with global preference:
    strncpy(units, wvutilsGetWindUnitLabel(), 15);
    strncpy(img->units, units, 15);

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, skipNo = 0; i < MONTHLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if (loctime.tm_hour == 0)
        {
            break;
        }
        else
        {
            skipNo ++;
            ntime += WV_SECONDS_IN_HOUR;
        }
    }

    for (i = 0; i < MONTHLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if ((i % 24) == 0)
        {
            if (loctime.tm_hour != 0)
            {
                // DST fall back nonsense:
                ntime += WV_SECONDS_IN_HOUR;
                localtime_r (&ntime, &loctime);
            }
        }
        sprintf (lbls[i], "%d", loctime.tm_mday);
        labels[i] = lbls[i];

        ntime += WV_SECONDS_IN_HOUR;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     &img->mgrWork->monthValues[DATA_INDEX_windSpeed][skipNo],
                     14,
                     0,
                     5,
                     1.0,
                     labels,
                     MONTHLY_NUM_VALUES-skipNo,
                     MONTHLY_NUM_VALUES,
                     img->title,
                     units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     img->mgrWork->isDualUnits));
}

static int generateWSpeedYear (HTML_IMG *img)
{
    int         i;
    char        lbls[YEARLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->yearStartTime_T;
    struct tm   loctime;
    char        units[16];

    // Make units consistent with global preference:
    strncpy(units, wvutilsGetWindUnitLabel(), 15);
    strncpy(img->units, units, 15);

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_DAY))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    ntime += 4*WV_SECONDS_IN_HOUR;
    for (i = 0; i < YEARLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%s %d", monthLabels[loctime.tm_mon], loctime.tm_mday);
        labels[i] = lbls[i];
        ntime += WV_SECONDS_IN_DAY;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->yearValues[DATA_INDEX_windSpeed],
                     4,
                     0,
                     5,
                     1.0,
                     labels,
                     YEARLY_NUM_VALUES,
                     YEARLY_NUM_VALUES,
                     img->title,
                     units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     img->mgrWork->isDualUnits));
}

static int generateWDirDay (HTML_IMG *img)
{
    int         i, j;
    char        temp[256];

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_SAMPLE))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, j = img->mgrWork->dayStart; i < DAILY_NUM_VALUES(img->mgrWork) - 1; i ++, j ++)
    {
        if (j >= DAILY_NUM_VALUES(img->mgrWork) - 1)
            j = 0;

        labels[i] = sampleLabels[j];
    }
    labels[i] = sampleLabels[DAILY_NUM_VALUES(img->mgrWork) - 1];

    return (htmlGenPngWDIRChart (temp,
                         img->mgrWork->isMetricUnits,
                         img->mgrWork->dayValues[DATA_INDEX_windDir],
                         6,
                         labels,
                         DAILY_NUM_VALUES(img->mgrWork),
                         DAILY_NUM_VALUES(img->mgrWork),
                         img->title,
                         img->units,
                         img->decimalPlaces,
                         img->mgrWork->dateFormat));
}

static int generateWDirWeek (HTML_IMG *img)
{
    int         i, skipNo;
    char        lbls[WEEKLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->weekStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, skipNo = 0; i < WEEKLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if (loctime.tm_hour == 0)
        {
            break;
        }
        else
        {
            skipNo ++;
            ntime += WV_SECONDS_IN_HOUR;
        }
    }

    for (i = 0; i < WEEKLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if ((i % 24) == 0)
        {
            if (loctime.tm_hour != 0)
            {
                // DST fall back nonsense:
                ntime += WV_SECONDS_IN_HOUR;
                localtime_r (&ntime, &loctime);
            }
        }
        sprintf (lbls[i], "%d", loctime.tm_mday);
        labels[i] = lbls[i];

        ntime += WV_SECONDS_IN_HOUR;
    }

    return (htmlGenPngWDIRChart (temp,
                         img->mgrWork->isMetricUnits,
                         &img->mgrWork->weekValues[DATA_INDEX_windDir][skipNo],
                         7,
                         labels,
                         WEEKLY_NUM_VALUES-skipNo,
                         WEEKLY_NUM_VALUES,
                         img->title,
                         img->units,
                         img->decimalPlaces,
                         img->mgrWork->dateFormat));
}

static int generateWDirMonth (HTML_IMG *img)
{
    int         i, skipNo;
    char        lbls[MONTHLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->monthStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, skipNo = 0; i < MONTHLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if (loctime.tm_hour == 0)
        {
            break;
        }
        else
        {
            skipNo ++;
            ntime += WV_SECONDS_IN_HOUR;
        }
    }

    for (i = 0; i < MONTHLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if ((i % 24) == 0)
        {
            if (loctime.tm_hour != 0)
            {
                // DST fall back nonsense:
                ntime += WV_SECONDS_IN_HOUR;
                localtime_r (&ntime, &loctime);
            }
        }
        sprintf (lbls[i], "%d", loctime.tm_mday);
        labels[i] = lbls[i];

        ntime += WV_SECONDS_IN_HOUR;
    }

    return (htmlGenPngWDIRChart (temp,
                         img->mgrWork->isMetricUnits,
                         &img->mgrWork->monthValues[DATA_INDEX_windDir][skipNo],
                         14,
                         labels,
                         MONTHLY_NUM_VALUES-skipNo,
                         MONTHLY_NUM_VALUES,
                         img->title,
                         img->units,
                         img->decimalPlaces,
                         img->mgrWork->dateFormat));
}

static int generateWDirYear (HTML_IMG *img)
{
    int         i;
    char        lbls[YEARLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->yearStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_DAY))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    ntime += 4*WV_SECONDS_IN_HOUR;
    for (i = 0; i < YEARLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%s %d", monthLabels[loctime.tm_mon], loctime.tm_mday);
        labels[i] = lbls[i];
        ntime += WV_SECONDS_IN_DAY;
    }

    return (htmlGenPngWDIRChart (temp,
                         img->mgrWork->isMetricUnits,
                         img->mgrWork->yearValues[DATA_INDEX_windDir],
                         4,
                         labels,
                         YEARLY_NUM_VALUES,
                         YEARLY_NUM_VALUES,
                         img->title,
                         img->units,
                         img->decimalPlaces,
                         img->mgrWork->dateFormat));
}

static int generateHiWSpeedDay (HTML_IMG *img)
{
    int         i, j;
    char        temp[256];
    char        units[16];

    // Make units consistent with global preference:
    strncpy(units, wvutilsGetWindUnitLabel(), 15);
    strncpy(img->units, units, 15);

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_SAMPLE))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, j = img->mgrWork->dayStart; i < DAILY_NUM_VALUES(img->mgrWork) - 1; i ++, j ++)
    {
        if (j >= DAILY_NUM_VALUES(img->mgrWork) - 1)
            j = 0;

        labels[i] = sampleLabels[j];
    }
    labels[i] = sampleLabels[DAILY_NUM_VALUES(img->mgrWork) - 1];

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->dayValues[DATA_INDEX_windGust],
                     6,
                     0,
                     5,
                     1.0,
                     labels,
                     DAILY_NUM_VALUES(img->mgrWork),
                     DAILY_NUM_VALUES(img->mgrWork),
                     img->title,
                     units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     img->mgrWork->isDualUnits));
}

static int generateHiWSpeedWeek (HTML_IMG *img)
{
    int         i, skipNo;
    char        lbls[WEEKLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->weekStartTime_T;
    struct tm   loctime;
    char        units[16];

    // Make units consistent with global preference:
    strncpy(units, wvutilsGetWindUnitLabel(), 15);
    strncpy(img->units, units, 15);

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, skipNo = 0; i < WEEKLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if (loctime.tm_hour == 0)
        {
            break;
        }
        else
        {
            skipNo ++;
            ntime += WV_SECONDS_IN_HOUR;
        }
    }

    for (i = 0; i < WEEKLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if ((i % 24) == 0)
        {
            if (loctime.tm_hour != 0)
            {
                // DST fall back nonsense:
                ntime += WV_SECONDS_IN_HOUR;
                localtime_r (&ntime, &loctime);
            }
        }
        sprintf (lbls[i], "%d", loctime.tm_mday);
        labels[i] = lbls[i];

        ntime += WV_SECONDS_IN_HOUR;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     &img->mgrWork->weekValues[DATA_INDEX_windGust][skipNo],
                     7,
                     0,
                     5,
                     1.0,
                     labels,
                     WEEKLY_NUM_VALUES-skipNo,
                     WEEKLY_NUM_VALUES,
                     img->title,
                     units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     img->mgrWork->isDualUnits));
}

static int generateHiWSpeedMonth (HTML_IMG *img)
{
    int         i, skipNo;
    char        lbls[MONTHLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->monthStartTime_T;
    struct tm   loctime;
    char        units[16];

    // Make units consistent with global preference:
    strncpy(units, wvutilsGetWindUnitLabel(), 15);
    strncpy(img->units, units, 15);

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, skipNo = 0; i < MONTHLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if (loctime.tm_hour == 0)
        {
            break;
        }
        else
        {
            skipNo ++;
            ntime += WV_SECONDS_IN_HOUR;
        }
    }

    for (i = 0; i < MONTHLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if ((i % 24) == 0)
        {
            if (loctime.tm_hour != 0)
            {
                // DST fall back nonsense:
                ntime += WV_SECONDS_IN_HOUR;
                localtime_r (&ntime, &loctime);
            }
        }
        sprintf (lbls[i], "%d", loctime.tm_mday);
        labels[i] = lbls[i];

        ntime += WV_SECONDS_IN_HOUR;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     &img->mgrWork->monthValues[DATA_INDEX_windGust][skipNo],
                     14,
                     0,
                     5,
                     1.0,
                     labels,
                     MONTHLY_NUM_VALUES-skipNo,
                     MONTHLY_NUM_VALUES,
                     img->title,
                     units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     img->mgrWork->isDualUnits));
}

static int generateHiWSpeedYear (HTML_IMG *img)
{
    int         i;
    char        lbls[YEARLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->yearStartTime_T;
    struct tm   loctime;
    char        units[16];

    // Make units consistent with global preference:
    strncpy(units, wvutilsGetWindUnitLabel(), 15);
    strncpy(img->units, units, 15);

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_DAY))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    ntime += 4*WV_SECONDS_IN_HOUR;
    for (i = 0; i < YEARLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%s %d", monthLabels[loctime.tm_mon], loctime.tm_mday);
        labels[i] = lbls[i];
        ntime += WV_SECONDS_IN_DAY;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->yearValues[DATA_INDEX_windGust],
                     4,
                     0,
                     5,
                     1.0,
                     labels,
                     YEARLY_NUM_VALUES,
                     YEARLY_NUM_VALUES,
                     img->title,
                     units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     img->mgrWork->isDualUnits));
}

static int generateBaromDay (HTML_IMG *img)
{
    int         i, j;
    char        temp[256];

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_SAMPLE))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, j = img->mgrWork->dayStart; i < DAILY_NUM_VALUES(img->mgrWork) - 1; i ++, j ++)
    {
        if (j >= DAILY_NUM_VALUES(img->mgrWork) - 1)
            j = 0;

        labels[i] = sampleLabels[j];
    }
    labels[i] = sampleLabels[DAILY_NUM_VALUES(img->mgrWork) - 1];

    if (img->mgrWork->isMetricUnits)
    {
        return (htmlGenPngChart (temp,
                         1,
                         img->mgrWork->dayValues[DATA_INDEX_barometer],
                         6,
                         1000.0,
                         1039.9,
                         10.0,
                         labels,
                         DAILY_NUM_VALUES(img->mgrWork),
                         DAILY_NUM_VALUES(img->mgrWork),
                         img->title,
                         img->units,
                         img->decimalPlaces,
                         img->mgrWork->dateFormat,
                         img->mgrWork->isDualUnits));
    }
    else
    {
        return (htmlGenPngChart (temp,
                         0,
                         img->mgrWork->dayValues[DATA_INDEX_barometer],
                         6,
                         29.7,
                         30.3,
                         0.1,
                         labels,
                         DAILY_NUM_VALUES(img->mgrWork),
                         DAILY_NUM_VALUES(img->mgrWork),
                         img->title,
                         img->units,
                         img->decimalPlaces,
                         img->mgrWork->dateFormat,
                         img->mgrWork->isDualUnits));
    }
}

static int generateBaromWeek (HTML_IMG *img)
{
    int         i, skipNo;
    char        lbls[WEEKLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->weekStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, skipNo = 0; i < WEEKLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if (loctime.tm_hour == 0)
        {
            break;
        }
        else
        {
            skipNo ++;
            ntime += WV_SECONDS_IN_HOUR;
        }
    }

    for (i = 0; i < WEEKLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if ((i % 24) == 0)
        {
            if (loctime.tm_hour != 0)
            {
                // DST fall back nonsense:
                ntime += WV_SECONDS_IN_HOUR;
                localtime_r (&ntime, &loctime);
            }
        }
        sprintf (lbls[i], "%d", loctime.tm_mday);
        labels[i] = lbls[i];

        ntime += WV_SECONDS_IN_HOUR;
    }

    if (img->mgrWork->isMetricUnits)
    {
        return (htmlGenPngChart (temp,
                         1,
                         &img->mgrWork->weekValues[DATA_INDEX_barometer][skipNo],
                         7,
                         1000.0,
                         1039.9,
                         10.0,
                         labels,
                         WEEKLY_NUM_VALUES-skipNo,
                         WEEKLY_NUM_VALUES,
                         img->title,
                         img->units,
                         img->decimalPlaces,
                         img->mgrWork->dateFormat,
                         img->mgrWork->isDualUnits));
    }
    else
    {
        return (htmlGenPngChart (temp,
                         0,
                         img->mgrWork->weekValues[DATA_INDEX_barometer],
                         7,
                         29.7,
                         30.3,
                         0.1,
                         labels,
                         WEEKLY_NUM_VALUES-skipNo,
                         WEEKLY_NUM_VALUES,
                         img->title,
                         img->units,
                         img->decimalPlaces,
                         img->mgrWork->dateFormat,
                         img->mgrWork->isDualUnits));
    }
}

static int generateBaromMonth (HTML_IMG *img)
{
    int         i, skipNo;
    char        lbls[MONTHLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->monthStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, skipNo = 0; i < MONTHLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if (loctime.tm_hour == 0)
        {
            break;
        }
        else
        {
            skipNo ++;
            ntime += WV_SECONDS_IN_HOUR;
        }
    }

    for (i = 0; i < MONTHLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if ((i % 24) == 0)
        {
            if (loctime.tm_hour != 0)
            {
                // DST fall back nonsense:
                ntime += WV_SECONDS_IN_HOUR;
                localtime_r (&ntime, &loctime);
            }
        }
        sprintf (lbls[i], "%d", loctime.tm_mday);
        labels[i] = lbls[i];

        ntime += WV_SECONDS_IN_HOUR;
    }

    if (img->mgrWork->isMetricUnits)
    {
        return (htmlGenPngChart (temp,
                         1,
                         &img->mgrWork->monthValues[DATA_INDEX_barometer][skipNo],
                         14,
                         1000.0,
                         1039.9,
                         10.0,
                         labels,
                         MONTHLY_NUM_VALUES-skipNo,
                         MONTHLY_NUM_VALUES,
                         img->title,
                         img->units,
                         img->decimalPlaces,
                         img->mgrWork->dateFormat,
                         img->mgrWork->isDualUnits));
    }
    else
    {
        return (htmlGenPngChart (temp,
                         0,
                         &img->mgrWork->monthValues[DATA_INDEX_barometer][skipNo],
                         14,
                         29.7,
                         30.3,
                         0.1,
                         labels,
                         MONTHLY_NUM_VALUES-skipNo,
                         MONTHLY_NUM_VALUES,
                         img->title,
                         img->units,
                         img->decimalPlaces,
                         img->mgrWork->dateFormat,
                         img->mgrWork->isDualUnits));
    }
}

static int generateBaromYear (HTML_IMG *img)
{
    int         i;
    char        lbls[YEARLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->yearStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_DAY))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);
    ntime += 4*WV_SECONDS_IN_HOUR;
    for (i = 0; i < YEARLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%s %d", monthLabels[loctime.tm_mon], loctime.tm_mday);
        labels[i] = lbls[i];
        ntime += WV_SECONDS_IN_DAY;
    }

    if (img->mgrWork->isMetricUnits)
    {
        return (htmlGenPngChart (temp,
                         1,
                         img->mgrWork->yearValues[DATA_INDEX_barometer],
                         4,
                         1000.0,
                         1039.9,
                         10.0,
                         labels,
                         YEARLY_NUM_VALUES,
                         YEARLY_NUM_VALUES,
                         img->title,
                         img->units,
                         img->decimalPlaces,
                         img->mgrWork->dateFormat,
                         img->mgrWork->isDualUnits));
    }
    else
    {
        return (htmlGenPngChart (temp,
                         0,
                         img->mgrWork->yearValues[DATA_INDEX_barometer],
                         4,
                         29.7,
                         30.3,
                         0.1,
                         labels,
                         YEARLY_NUM_VALUES,
                         YEARLY_NUM_VALUES,
                         img->title,
                         img->units,
                         img->decimalPlaces,
                         img->mgrWork->dateFormat,
                         img->mgrWork->isDualUnits));
    }
}

static int generateRainDay (HTML_IMG *img)
{
    int         i, j, skipNo = 0;
    int         samplesInHour = (60/img->mgrWork->archiveInterval);
    char        temp[256];
    float       max = 0;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_SAMPLE))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    j = img->mgrWork->dayStart;
    while ((j % samplesInHour) != 0)
    {
        skipNo ++;
        if (++ j >= DAILY_NUM_VALUES(img->mgrWork) - 1)
            j = 0;
    }

    for (i = 0; i < DAILY_NUM_VALUES(img->mgrWork)-skipNo; i ++)
    {
        labels[i] = sampleHourLabels[j];

        if (img->mgrWork->dayValues[DATA_INDEX_rain][i+skipNo] > max)
            max = img->mgrWork->dayValues[DATA_INDEX_rain][i+skipNo];

        if (++ j >= DAILY_NUM_VALUES(img->mgrWork) - 1)
            j = 0;
    }
    
    // tack on the extra label
    labels[(DAILY_NUM_VALUES(img->mgrWork)-skipNo)] = sampleHourLabels[(DAILY_NUM_VALUES(img->mgrWork)-1)];
    
    return (htmlGenPngBarChart (temp,
                        img->mgrWork->isMetricUnits,
                        &img->mgrWork->dayValues[DATA_INDEX_rain][skipNo+1],
                        6,
                        0,
                        MAX(0.1,max),
                        0.01,
                        labels,
                        DAILY_NUM_VALUES(img->mgrWork)-skipNo,
                        img->title,
                        img->units,
                        img->decimalPlaces,
                        60/img->mgrWork->archiveInterval,
                        24,
                        60/img->mgrWork->archiveInterval,
                        img->mgrWork->dateFormat,
                        img->mgrWork->isDualUnits));
}

static int generateRainWeek (HTML_IMG *img)
{
    int         i, skipNo;
    char        lbls[WEEKLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->weekStartTime_T;
    struct tm   loctime;
    float       max = 0;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, skipNo = 0; i < WEEKLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if (loctime.tm_hour == 0)
        {
            break;
        }
        else
        {
            skipNo ++;
            ntime += WV_SECONDS_IN_HOUR;
        }
    }

    // kludge this for aggregate values
    if (skipNo == 0)
    {
        // push forward one day
        skipNo = 24;
        ntime += WV_SECONDS_IN_DAY;
    }

    for (i = 0; i < WEEKLY_NUM_VALUES-skipNo; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%d", loctime.tm_mday);
        labels[i] = lbls[i];

        ntime += WV_SECONDS_IN_HOUR;

        if (img->mgrWork->weekValues[DATA_INDEX_rain][i+skipNo] > max)
            max = img->mgrWork->weekValues[DATA_INDEX_rain][i+skipNo];
    }

    // tack on the extra label
    if (skipNo > 1)
    {
        ntime += WV_SECONDS_IN_DAY;
    }

    localtime_r (&ntime, &loctime);
    sprintf (lbls[WEEKLY_NUM_VALUES-skipNo], "%d", loctime.tm_mday);
    labels[WEEKLY_NUM_VALUES-skipNo] = lbls[WEEKLY_NUM_VALUES-skipNo];

    return (htmlGenPngBarChart (temp,
                        img->mgrWork->isMetricUnits,
                        &img->mgrWork->weekValues[DATA_INDEX_rain][skipNo],
                        7,
                        0,
                        MAX(0.1,max),
                        0.1,
                        labels,
                        WEEKLY_NUM_VALUES-skipNo,
                        img->title,
                        img->units,
                        img->decimalPlaces,
                        24,
                        7,
                        24,
                        img->mgrWork->dateFormat,
                        img->mgrWork->isDualUnits));
}

static int generateRainMonth (HTML_IMG *img)
{
    int         i, skipNo;
    char        lbls[MONTHLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->monthStartTime_T;
    struct tm   loctime;
    float       max = 0;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, skipNo = 0; i < MONTHLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if (loctime.tm_hour == 0)
        {
            break;
        }
        else
        {
            skipNo ++;
            ntime += WV_SECONDS_IN_HOUR;
        }
    }

    // kludge this for aggregate values
    if (skipNo == 0)
    {
        // push forward one day
        skipNo = 24;
        ntime += WV_SECONDS_IN_DAY;
    }

    for (i = 0; i < MONTHLY_NUM_VALUES-skipNo; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%d", loctime.tm_mday);
        labels[i] = lbls[i];

        ntime += WV_SECONDS_IN_HOUR;
        if (img->mgrWork->monthValues[DATA_INDEX_rain][i+skipNo] > max)
            max = img->mgrWork->monthValues[DATA_INDEX_rain][i+skipNo];
    }

    // tack on the extra label
    if (skipNo > 1)
    {
        ntime += WV_SECONDS_IN_DAY;
    }

    localtime_r (&ntime, &loctime);
    sprintf (lbls[MONTHLY_NUM_VALUES-skipNo], "%d", loctime.tm_mday);
    labels[MONTHLY_NUM_VALUES-skipNo] = lbls[MONTHLY_NUM_VALUES-skipNo];

    return (htmlGenPngBarChart (temp,
                        img->mgrWork->isMetricUnits,
                        &img->mgrWork->monthValues[DATA_INDEX_rain][skipNo],
                        14,
                        0,
                        MAX(0.1,max),
                        0.1,
                        labels,
                        MONTHLY_NUM_VALUES-skipNo,
                        img->title,
                        img->units,
                        img->decimalPlaces,
                        24,
                        28,
                        24,
                        img->mgrWork->dateFormat,
                        img->mgrWork->isDualUnits));
}

static int generateRainYear (HTML_IMG *img)
{
    int         i, j, skipNo;
    char        lbls[YEARLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->yearStartTime_T;
    struct tm   loctime;
    float       max = 0;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_DAY))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    // move forward 1 day to land on 52 weeks ago
    ntime += WV_SECONDS_IN_DAY;
    skipNo = 1;
    
    // if we are sitting on Sunday, bump it another day
    localtime_r (&ntime, &loctime);
    if (loctime.tm_wday == 0)
    {
        ntime += WV_SECONDS_IN_DAY;
        skipNo ++;
    }
    
    // move to the next Sunday
    for (i = 0; i < 7; i ++)
    {
        localtime_r (&ntime, &loctime);
        if (loctime.tm_wday == 0)
        {
            break;
        }
        else
        {
            skipNo ++;
            ntime += WV_SECONDS_IN_DAY;
        }
    }

    for (i = 0; i < YEARLY_NUM_VALUES-skipNo; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%s %d", monthLabels[loctime.tm_mon], loctime.tm_mday);
        labels[i] = lbls[i];

        ntime += WV_SECONDS_IN_DAY;
        if (img->mgrWork->yearValues[DATA_INDEX_rain][i+skipNo] > max)
            max = img->mgrWork->yearValues[DATA_INDEX_rain][i+skipNo];
    }

    for (j = 0; j < 7; j ++)
    {
        localtime_r (&ntime, &loctime);
        if (loctime.tm_wday == 0)
        {
            break;
        }
        else
        {
            ntime += WV_SECONDS_IN_DAY;
        }
    }

    localtime_r (&ntime, &loctime);
    sprintf (lbls[YEARLY_NUM_VALUES-skipNo], "%s %d", monthLabels[loctime.tm_mon], loctime.tm_mday);
    labels[YEARLY_NUM_VALUES-skipNo] = lbls[YEARLY_NUM_VALUES-skipNo];

    return (htmlGenPngBarChart (temp,
                        img->mgrWork->isMetricUnits,
                        &img->mgrWork->yearValues[DATA_INDEX_rain][skipNo],
                        4,
                        0,
                        MAX(0.1,max),
                        0.1,
                        labels,
                        YEARLY_NUM_VALUES-skipNo,
                        img->title,
                        img->units,
                        img->decimalPlaces,
                        7,
                        52,
                        7,
                        img->mgrWork->dateFormat,
                        img->mgrWork->isDualUnits));
}

static int generateWChillDay (HTML_IMG *img)
{
    int         i, j;
    char        temp[256];

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_SAMPLE))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, j = img->mgrWork->dayStart; i < DAILY_NUM_VALUES(img->mgrWork) - 1; i ++, j ++)
    {
        if (j >= DAILY_NUM_VALUES(img->mgrWork) - 1)
            j = 0;

        labels[i] = sampleLabels[j];
    }
    labels[i] = sampleLabels[DAILY_NUM_VALUES(img->mgrWork) - 1];

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->dayValues[DATA_INDEX_windchill],
                     6,
                     CHART_MIN_TEMP(img->mgrWork->isMetricUnits),
                     CHART_MAX_TEMP(img->mgrWork->isMetricUnits),
                     1.0,
                     labels,
                     DAILY_NUM_VALUES(img->mgrWork),
                     DAILY_NUM_VALUES(img->mgrWork),
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     img->mgrWork->isDualUnits));
}

static int generateWChillWeek (HTML_IMG *img)
{
    int         i, skipNo;
    char        lbls[WEEKLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->weekStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, skipNo = 0; i < WEEKLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if (loctime.tm_hour == 0)
        {
            break;
        }
        else
        {
            skipNo ++;
            ntime += WV_SECONDS_IN_HOUR;
        }
    }

    for (i = 0; i < WEEKLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if ((i % 24) == 0)
        {
            if (loctime.tm_hour != 0)
            {
                // DST fall back nonsense:
                ntime += WV_SECONDS_IN_HOUR;
                localtime_r (&ntime, &loctime);
            }
        }
        sprintf (lbls[i], "%d", loctime.tm_mday);
        labels[i] = lbls[i];

        ntime += WV_SECONDS_IN_HOUR;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     &img->mgrWork->weekValues[DATA_INDEX_windchill][skipNo],
                     7,
                     CHART_MIN_TEMP(img->mgrWork->isMetricUnits),
                     CHART_MAX_TEMP(img->mgrWork->isMetricUnits),
                     1.0,
                     labels,
                     WEEKLY_NUM_VALUES-skipNo,
                     WEEKLY_NUM_VALUES,
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     img->mgrWork->isDualUnits));
}

static int generateWChillMonth (HTML_IMG *img)
{
    int         i, skipNo;
    char        lbls[MONTHLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->monthStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, skipNo = 0; i < MONTHLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if (loctime.tm_hour == 0)
        {
            break;
        }
        else
        {
            skipNo ++;
            ntime += WV_SECONDS_IN_HOUR;
        }
    }

    for (i = 0; i < MONTHLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if ((i % 24) == 0)
        {
            if (loctime.tm_hour != 0)
            {
                // DST fall back nonsense:
                ntime += WV_SECONDS_IN_HOUR;
                localtime_r (&ntime, &loctime);
            }
        }
        sprintf (lbls[i], "%d", loctime.tm_mday);
        labels[i] = lbls[i];

        ntime += WV_SECONDS_IN_HOUR;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     &img->mgrWork->monthValues[DATA_INDEX_windchill][skipNo],
                     14,
                     CHART_MIN_TEMP(img->mgrWork->isMetricUnits),
                     CHART_MAX_TEMP(img->mgrWork->isMetricUnits),
                     1.0,
                     labels,
                     MONTHLY_NUM_VALUES-skipNo,
                     MONTHLY_NUM_VALUES,
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     img->mgrWork->isDualUnits));
}

static int generateWChillYear (HTML_IMG *img)
{
    int         i;
    char        lbls[YEARLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->yearStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_DAY))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);
    ntime += 4*WV_SECONDS_IN_HOUR;
    for (i = 0; i < YEARLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%s %d", monthLabels[loctime.tm_mon], loctime.tm_mday);
        labels[i] = lbls[i];
        ntime += WV_SECONDS_IN_DAY;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->yearValues[DATA_INDEX_windchill],
                     4,
                     CHART_MIN_TEMP(img->mgrWork->isMetricUnits),
                     CHART_MAX_TEMP(img->mgrWork->isMetricUnits),
                     1.0,
                     labels,
                     YEARLY_NUM_VALUES,
                     YEARLY_NUM_VALUES,
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     img->mgrWork->isDualUnits));
}

static int generateHIndexDay (HTML_IMG *img)
{
    int         i, j;
    char        temp[256];

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_SAMPLE))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, j = img->mgrWork->dayStart; i < DAILY_NUM_VALUES(img->mgrWork) - 1; i ++, j ++)
    {
        if (j >= DAILY_NUM_VALUES(img->mgrWork) - 1)
            j = 0;

        labels[i] = sampleLabels[j];
    }
    labels[i] = sampleLabels[DAILY_NUM_VALUES(img->mgrWork) - 1];

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->dayValues[DATA_INDEX_heatindex],
                     6,
                     CHART_MIN_TEMP(img->mgrWork->isMetricUnits),
                     CHART_MAX_TEMP(img->mgrWork->isMetricUnits),
                     1.0,
                     labels,
                     DAILY_NUM_VALUES(img->mgrWork),
                     DAILY_NUM_VALUES(img->mgrWork),
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     img->mgrWork->isDualUnits));
}

static int generateHIndexWeek (HTML_IMG *img)
{
    int         i, skipNo;
    char        lbls[WEEKLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->weekStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, skipNo = 0; i < WEEKLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if (loctime.tm_hour == 0)
        {
            break;
        }
        else
        {
            skipNo ++;
            ntime += WV_SECONDS_IN_HOUR;
        }
    }

    for (i = 0; i < WEEKLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if ((i % 24) == 0)
        {
            if (loctime.tm_hour != 0)
            {
                // DST fall back nonsense:
                ntime += WV_SECONDS_IN_HOUR;
                localtime_r (&ntime, &loctime);
            }
        }
        sprintf (lbls[i], "%d", loctime.tm_mday);
        labels[i] = lbls[i];

        ntime += WV_SECONDS_IN_HOUR;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     &img->mgrWork->weekValues[DATA_INDEX_heatindex][skipNo],
                     7,
                     CHART_MIN_TEMP(img->mgrWork->isMetricUnits),
                     CHART_MAX_TEMP(img->mgrWork->isMetricUnits),
                     1.0,
                     labels,
                     WEEKLY_NUM_VALUES-skipNo,
                     WEEKLY_NUM_VALUES,
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     img->mgrWork->isDualUnits));
}

static int generateHIndexMonth (HTML_IMG *img)
{
    int         i, skipNo;
    char        lbls[MONTHLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->monthStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, skipNo = 0; i < MONTHLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if (loctime.tm_hour == 0)
        {
            break;
        }
        else
        {
            skipNo ++;
            ntime += WV_SECONDS_IN_HOUR;
        }
    }

    for (i = 0; i < MONTHLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if ((i % 24) == 0)
        {
            if (loctime.tm_hour != 0)
            {
                // DST fall back nonsense:
                ntime += WV_SECONDS_IN_HOUR;
                localtime_r (&ntime, &loctime);
            }
        }
        sprintf (lbls[i], "%d", loctime.tm_mday);
        labels[i] = lbls[i];

        ntime += WV_SECONDS_IN_HOUR;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     &img->mgrWork->monthValues[DATA_INDEX_heatindex][skipNo],
                     14,
                     CHART_MIN_TEMP(img->mgrWork->isMetricUnits),
                     CHART_MAX_TEMP(img->mgrWork->isMetricUnits),
                     1.0,
                     labels,
                     MONTHLY_NUM_VALUES-skipNo,
                     MONTHLY_NUM_VALUES,
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     img->mgrWork->isDualUnits));
}

static int generateHIndexYear (HTML_IMG *img)
{
    int         i;
    char        lbls[YEARLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->yearStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_DAY))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);
    ntime += 4*WV_SECONDS_IN_HOUR;
    for (i = 0; i < YEARLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%s %d", monthLabels[loctime.tm_mon], loctime.tm_mday);
        labels[i] = lbls[i];
        ntime += WV_SECONDS_IN_DAY;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->yearValues[DATA_INDEX_heatindex],
                     4,
                     CHART_MIN_TEMP(img->mgrWork->isMetricUnits),
                     CHART_MAX_TEMP(img->mgrWork->isMetricUnits),
                     1.0,
                     labels,
                     YEARLY_NUM_VALUES,
                     YEARLY_NUM_VALUES,
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     img->mgrWork->isDualUnits));
}


////////////////////////////////////////////////////////////////////////////
//  Composite Charts

// utility to determine max of 4 values
static void setMax(float *maxStore, float v1, float v2, float v3, float v4)
{
    if (v1 > *maxStore)
        *maxStore = v1;
    if (v2 > *maxStore)
        *maxStore = v2;
    if (v3 > *maxStore)
        *maxStore = v3;
    if (v4 > *maxStore)
        *maxStore = v4;
}

// utility to determine min of 4 values
static void setMin(float *minStore, float v1, float v2, float v3, float v4)
{
    if (v1 < *minStore)
        *minStore = v1;
    if (v2 < *minStore)
        *minStore = v2;
    if (v3 < *minStore)
        *minStore = v3;
    if (v4 < *minStore)
        *minStore = v4;
}


static int generateCompositeTempDay (HTML_IMG *img)
{
    int         i, j;
    char        temp[256];
    MC_DATASET  datasets[2];
    float       min = 10000.0, max = -10000.0;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_SAMPLE))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, j = img->mgrWork->dayStart; i < DAILY_NUM_VALUES(img->mgrWork) - 1; i ++, j ++)
    {
        if (j >= DAILY_NUM_VALUES(img->mgrWork) - 1)
            j = 0;

        labels[i] = sampleLabels[j];
    }
    labels[i] = sampleLabels[DAILY_NUM_VALUES(img->mgrWork) - 1];
    
    datasets[0].lineColor = htmlgenGetChartDefaultLine();
    datasets[1].lineColor = htmlgenGetChartDefaultSecondLine();
    
    memset (datasets[0].legend, 0, sizeof(datasets[0].legend));
    memset (datasets[1].legend, 0, sizeof(datasets[1].legend));
    for (i = 0; i < 15 && i < strlen (img->title); i ++)
    {
        if (img->title[i] == '/')
            break;
    
        datasets[0].legend[i] = img->title[i];
    }
    if (i < strlen (img->title) - 1)
    {
        wvstrncpy (datasets[1].legend, &img->title[i+1], sizeof(datasets[1].legend));
    }
    
    for (i = 0; i < DAILY_NUM_VALUES(img->mgrWork); i ++)
    {
        datasets[0].valueset[i] = img->mgrWork->dayValues[DATA_INDEX_outTemp][i];
        datasets[1].valueset[i] = img->mgrWork->dayValues[DATA_INDEX_dewpoint][i];

        if (img->mgrWork->dayValues[DATA_INDEX_outTemp][i] <= ARCHIVE_VALUE_NULL ||
            img->mgrWork->dayValues[DATA_INDEX_dewpoint][i] <= ARCHIVE_VALUE_NULL ||
            img->mgrWork->dayValues[DATA_INDEX_windchill][i] <= ARCHIVE_VALUE_NULL ||
            img->mgrWork->dayValues[DATA_INDEX_heatindex][i] <= ARCHIVE_VALUE_NULL)
        {
            continue;
        }
                
        setMin (&min, 
                img->mgrWork->dayValues[DATA_INDEX_outTemp][i],
                img->mgrWork->dayValues[DATA_INDEX_dewpoint][i],
                img->mgrWork->dayValues[DATA_INDEX_windchill][i],
                img->mgrWork->dayValues[DATA_INDEX_heatindex][i]);
                
        setMax (&max, 
                img->mgrWork->dayValues[DATA_INDEX_outTemp][i],
                img->mgrWork->dayValues[DATA_INDEX_dewpoint][i],
                img->mgrWork->dayValues[DATA_INDEX_windchill][i],
                img->mgrWork->dayValues[DATA_INDEX_heatindex][i]);
    }

    // if there is no data yet, pick default min/max
    if (min > max)
    {
        min = CHART_MIN_TEMP(img->mgrWork->isMetricUnits);
        max = CHART_MAX_TEMP(img->mgrWork->isMetricUnits);
    }

    return (htmlGenPngMultiChart (temp,
                          img->mgrWork->isMetricUnits,
                          datasets,
                          2,
                          6,
                          min,
                          max,
                          1.0,
                          labels,
                          DAILY_NUM_VALUES(img->mgrWork),
                          DAILY_NUM_VALUES(img->mgrWork),
                          img->units,
                          img->decimalPlaces,
                          img->mgrWork->dateFormat,
                          img->mgrWork->isDualUnits));
}

static int generateCompositeHIndexWChillDay (HTML_IMG *img)
{
    int         i, j;
    char        temp[256];
    MC_DATASET  datasets[2];
    float       min = 10000.0, max = -10000.0;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_SAMPLE))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, j = img->mgrWork->dayStart; i < DAILY_NUM_VALUES(img->mgrWork) - 1; i ++, j ++)
    {
        if (j >= DAILY_NUM_VALUES(img->mgrWork) - 1)
            j = 0;

        labels[i] = sampleLabels[j];
    }
    labels[i] = sampleLabels[DAILY_NUM_VALUES(img->mgrWork) - 1];
    
    datasets[0].lineColor = htmlgenGetChartDefaultLine();
    datasets[1].lineColor = htmlgenGetChartDefaultSecondLine();
    
    memset (datasets[0].legend, 0, sizeof(datasets[0].legend));
    memset (datasets[1].legend, 0, sizeof(datasets[1].legend));
    for (i = 0; i < 15 && i < strlen (img->title); i ++)
    {
        if (img->title[i] == '/')
            break;
    
        datasets[0].legend[i] = img->title[i];
    }
    if (i < strlen (img->title) - 1)
    {
        wvstrncpy (datasets[1].legend, &img->title[i+1], sizeof(datasets[1].legend));
    }
    
    for (i = 0; i < DAILY_NUM_VALUES(img->mgrWork); i ++)
    {
        datasets[0].valueset[i] = img->mgrWork->dayValues[DATA_INDEX_windchill][i];
        datasets[1].valueset[i] = img->mgrWork->dayValues[DATA_INDEX_heatindex][i];
        
        if (img->mgrWork->dayValues[DATA_INDEX_outTemp][i] <= ARCHIVE_VALUE_NULL ||
            img->mgrWork->dayValues[DATA_INDEX_dewpoint][i] <= ARCHIVE_VALUE_NULL ||
            img->mgrWork->dayValues[DATA_INDEX_windchill][i] <= ARCHIVE_VALUE_NULL ||
            img->mgrWork->dayValues[DATA_INDEX_heatindex][i] <= ARCHIVE_VALUE_NULL)
        {
            continue;
        }
        
        setMin (&min, 
                img->mgrWork->dayValues[DATA_INDEX_outTemp][i],
                img->mgrWork->dayValues[DATA_INDEX_dewpoint][i],
                img->mgrWork->dayValues[DATA_INDEX_windchill][i],
                img->mgrWork->dayValues[DATA_INDEX_heatindex][i]);
                
        setMax (&max, 
                img->mgrWork->dayValues[DATA_INDEX_outTemp][i],
                img->mgrWork->dayValues[DATA_INDEX_dewpoint][i],
                img->mgrWork->dayValues[DATA_INDEX_windchill][i],
                img->mgrWork->dayValues[DATA_INDEX_heatindex][i]);
    }

    // if there is no data yet, pick default min/max
    if (min > max)
    {
        min = CHART_MIN_TEMP(img->mgrWork->isMetricUnits);
        max = CHART_MAX_TEMP(img->mgrWork->isMetricUnits);
    }

    return (htmlGenPngMultiChart (temp,
                          img->mgrWork->isMetricUnits,
                          datasets,
                          2,
                          6,
                          min,
                          max,
                          1.0,
                          labels,
                          DAILY_NUM_VALUES(img->mgrWork),
                          DAILY_NUM_VALUES(img->mgrWork),
                          img->units,
                          img->decimalPlaces,
                          img->mgrWork->dateFormat,
                          img->mgrWork->isDualUnits));
}

static int generateCompositeInTempHumidDay (HTML_IMG *img)
{
    int         i, j;
    char        temp[256];
    MC_DATASET  datasets[2];

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_SAMPLE))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, j = img->mgrWork->dayStart; i < DAILY_NUM_VALUES(img->mgrWork) - 1; i ++, j ++)
    {
        if (j >= DAILY_NUM_VALUES(img->mgrWork) - 1)
            j = 0;

        labels[i] = sampleLabels[j];
    }
    labels[i] = sampleLabels[DAILY_NUM_VALUES(img->mgrWork) - 1];
    
    datasets[0].lineColor = htmlgenGetChartDefaultLine();
    datasets[1].lineColor = htmlgenGetChartDefaultSecondLine();
    
    memset (datasets[0].legend, 0, sizeof(datasets[0].legend));
    memset (datasets[1].legend, 0, sizeof(datasets[1].legend));
    for (i = 0; i < 15 && i < strlen (img->title); i ++)
    {
        if (img->title[i] == '/')
            break;
    
        datasets[0].legend[i] = img->title[i];
    }
    if (i < strlen (img->title) - 1)
    {
        wvstrncpy (datasets[1].legend, &img->title[i+1], sizeof(datasets[1].legend));
    }
    
    for (i = 0; i < DAILY_NUM_VALUES(img->mgrWork); i ++)
    {
        datasets[0].valueset[i] = img->mgrWork->dayValues[DATA_INDEX_inTemp][i];
        datasets[1].valueset[i] = img->mgrWork->dayValues[DATA_INDEX_inHumidity][i];
    }

    return (htmlGenPngMultiChart (temp,
                          img->mgrWork->isMetricUnits,
                          datasets,
                          2,
                          6,
                          CHART_MIN_TEMP(img->mgrWork->isMetricUnits),
                          CHART_MAX_TEMP(img->mgrWork->isMetricUnits),
                          1.0,
                          labels,
                          DAILY_NUM_VALUES(img->mgrWork),
                          DAILY_NUM_VALUES(img->mgrWork),
                          img->units,
                          img->decimalPlaces,
                          img->mgrWork->dateFormat,
                          img->mgrWork->isDualUnits));
}

static int generateCompositeTempWeek (HTML_IMG *img)
{
    int         i, j, skipNo;
    char        temp[256];
    char        lbls[WEEKLY_NUM_VALUES][8];
    time_t      ntime = img->mgrWork->weekStartTime_T;
    struct tm   loctime;
    float       min = 10000.0, max = -10000.0;
    MC_DATASET  datasets[2];

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, skipNo = 0; i < WEEKLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if (loctime.tm_hour == 0)
        {
            break;
        }
        else
        {
            skipNo ++;
            ntime += WV_SECONDS_IN_HOUR;
        }
    }

    for (i = 0; i < WEEKLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if ((i % 24) == 0)
        {
            if (loctime.tm_hour != 0)
            {
                // DST fall back nonsense:
                ntime += WV_SECONDS_IN_HOUR;
                localtime_r (&ntime, &loctime);
            }
        }
        sprintf (lbls[i], "%d", loctime.tm_mday);
        labels[i] = lbls[i];

        ntime += WV_SECONDS_IN_HOUR;
    }

    datasets[0].lineColor = htmlgenGetChartDefaultLine();
    datasets[1].lineColor = htmlgenGetChartDefaultSecondLine();
    
    memset (datasets[0].legend, 0, sizeof(datasets[0].legend));
    memset (datasets[1].legend, 0, sizeof(datasets[1].legend));
    for (i = 0; i < 15 && i < strlen (img->title); i ++)
    {
        if (img->title[i] == '/')
            break;
    
        datasets[0].legend[i] = img->title[i];
    }
    if (i < strlen (img->title) - 1)
    {
        wvstrncpy (datasets[1].legend, &img->title[i+1], sizeof(datasets[1].legend));
    }
    
    for (i = skipNo; i < WEEKLY_NUM_VALUES; i ++)
    {
        datasets[0].valueset[i-skipNo] = img->mgrWork->weekValues[DATA_INDEX_outTemp][i];
        datasets[1].valueset[i-skipNo] = img->mgrWork->weekValues[DATA_INDEX_dewpoint][i];

        if (img->mgrWork->weekValues[DATA_INDEX_outTemp][i] <= ARCHIVE_VALUE_NULL ||
            img->mgrWork->weekValues[DATA_INDEX_dewpoint][i] <= ARCHIVE_VALUE_NULL ||
            img->mgrWork->weekValues[DATA_INDEX_windchill][i] <= ARCHIVE_VALUE_NULL ||
            img->mgrWork->weekValues[DATA_INDEX_heatindex][i] <= ARCHIVE_VALUE_NULL)
        {
            continue;
        }
        
        setMin (&min, 
                img->mgrWork->weekValues[DATA_INDEX_outTemp][i],
                img->mgrWork->weekValues[DATA_INDEX_dewpoint][i],
                img->mgrWork->weekValues[DATA_INDEX_windchill][i],
                img->mgrWork->weekValues[DATA_INDEX_heatindex][i]);
                
        setMax (&max, 
                img->mgrWork->weekValues[DATA_INDEX_outTemp][i],
                img->mgrWork->weekValues[DATA_INDEX_dewpoint][i],
                img->mgrWork->weekValues[DATA_INDEX_windchill][i],
                img->mgrWork->weekValues[DATA_INDEX_heatindex][i]);
    }

    // if there is no data yet, pick default min/max
    if (min > max)
    {
        min = CHART_MIN_TEMP(img->mgrWork->isMetricUnits);
        max = CHART_MAX_TEMP(img->mgrWork->isMetricUnits);
    }

    return (htmlGenPngMultiChart (temp,
                          img->mgrWork->isMetricUnits,
                          datasets,
                          2,
                          7,
                          min,
                          max,
                          1.0,
                          labels,
                          WEEKLY_NUM_VALUES-skipNo,
                          WEEKLY_NUM_VALUES,
                          img->units,
                          img->decimalPlaces,
                          img->mgrWork->dateFormat,
                          img->mgrWork->isDualUnits));
}

static int generateCompositeHIndexWChillWeek (HTML_IMG *img)
{
    int         i, j, skipNo;
    char        temp[256];
    char        lbls[WEEKLY_NUM_VALUES][8];
    time_t      ntime = img->mgrWork->weekStartTime_T;
    struct tm   loctime;
    float       min = 10000.0, max = -10000.0;
    MC_DATASET  datasets[2];

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, skipNo = 0; i < WEEKLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if (loctime.tm_hour == 0)
        {
            break;
        }
        else
        {
            skipNo ++;
            ntime += WV_SECONDS_IN_HOUR;
        }
    }

    for (i = 0; i < WEEKLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if ((i % 24) == 0)
        {
            if (loctime.tm_hour != 0)
            {
                // DST fall back nonsense:
                ntime += WV_SECONDS_IN_HOUR;
                localtime_r (&ntime, &loctime);
            }
        }
        sprintf (lbls[i], "%d", loctime.tm_mday);
        labels[i] = lbls[i];

        ntime += WV_SECONDS_IN_HOUR;
    }
    
    datasets[0].lineColor = htmlgenGetChartDefaultLine();
    datasets[1].lineColor = htmlgenGetChartDefaultSecondLine();
    
    memset (datasets[0].legend, 0, sizeof(datasets[0].legend));
    memset (datasets[1].legend, 0, sizeof(datasets[1].legend));
    for (i = 0; i < 15 && i < strlen (img->title); i ++)
    {
        if (img->title[i] == '/')
            break;
    
        datasets[0].legend[i] = img->title[i];
    }
    if (i < strlen (img->title) - 1)
    {
        wvstrncpy (datasets[1].legend, &img->title[i+1], sizeof(datasets[1].legend));
    }
    
    for (i = skipNo; i < WEEKLY_NUM_VALUES; i ++)
    {
        datasets[0].valueset[i-skipNo] = img->mgrWork->weekValues[DATA_INDEX_windchill][i];
        datasets[1].valueset[i-skipNo] = img->mgrWork->weekValues[DATA_INDEX_heatindex][i];
        
        if (img->mgrWork->weekValues[DATA_INDEX_outTemp][i] <= ARCHIVE_VALUE_NULL ||
            img->mgrWork->weekValues[DATA_INDEX_dewpoint][i] <= ARCHIVE_VALUE_NULL ||
            img->mgrWork->weekValues[DATA_INDEX_windchill][i] <= ARCHIVE_VALUE_NULL ||
            img->mgrWork->weekValues[DATA_INDEX_heatindex][i] <= ARCHIVE_VALUE_NULL)
        {
            continue;
        }
        
        setMin (&min, 
                img->mgrWork->weekValues[DATA_INDEX_outTemp][i],
                img->mgrWork->weekValues[DATA_INDEX_dewpoint][i],
                img->mgrWork->weekValues[DATA_INDEX_windchill][i],
                img->mgrWork->weekValues[DATA_INDEX_heatindex][i]);
                
        setMax (&max, 
                img->mgrWork->weekValues[DATA_INDEX_outTemp][i],
                img->mgrWork->weekValues[DATA_INDEX_dewpoint][i],
                img->mgrWork->weekValues[DATA_INDEX_windchill][i],
                img->mgrWork->weekValues[DATA_INDEX_heatindex][i]);
    }

    // if there is no data yet, pick default min/max
    if (min > max)
    {
        min = CHART_MIN_TEMP(img->mgrWork->isMetricUnits);
        max = CHART_MAX_TEMP(img->mgrWork->isMetricUnits);
    }

    return (htmlGenPngMultiChart (temp,
                          img->mgrWork->isMetricUnits,
                          datasets,
                          2,
                          7,
                          min,
                          max,
                          1.0,
                          labels,
                          WEEKLY_NUM_VALUES-skipNo,
                          WEEKLY_NUM_VALUES,
                          img->units,
                          img->decimalPlaces,
                          img->mgrWork->dateFormat,
                          img->mgrWork->isDualUnits));
}

static int generateCompositeInTempHumidWeek (HTML_IMG *img)
{
    int         i, j, skipNo;
    char        temp[256];
    char        lbls[WEEKLY_NUM_VALUES][8];
    time_t      ntime = img->mgrWork->weekStartTime_T;
    struct tm   loctime;
    MC_DATASET  datasets[2];

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, skipNo = 0; i < WEEKLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if (loctime.tm_hour == 0)
        {
            break;
        }
        else
        {
            skipNo ++;
            ntime += WV_SECONDS_IN_HOUR;
        }
    }

    for (i = 0; i < WEEKLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if ((i % 24) == 0)
        {
            if (loctime.tm_hour != 0)
            {
                // DST fall back nonsense:
                ntime += WV_SECONDS_IN_HOUR;
                localtime_r (&ntime, &loctime);
            }
        }
        sprintf (lbls[i], "%d", loctime.tm_mday);
        labels[i] = lbls[i];

        ntime += WV_SECONDS_IN_HOUR;
    }
    
    datasets[0].lineColor = htmlgenGetChartDefaultLine();
    datasets[1].lineColor = htmlgenGetChartDefaultSecondLine();
    
    memset (datasets[0].legend, 0, sizeof(datasets[0].legend));
    memset (datasets[1].legend, 0, sizeof(datasets[1].legend));
    for (i = 0; i < 15 && i < strlen (img->title); i ++)
    {
        if (img->title[i] == '/')
            break;
    
        datasets[0].legend[i] = img->title[i];
    }
    if (i < strlen (img->title) - 1)
    {
        wvstrncpy (datasets[1].legend, &img->title[i+1], sizeof(datasets[1].legend));
    }
    
    for (i = skipNo; i < WEEKLY_NUM_VALUES; i ++)
    {
        datasets[0].valueset[i-skipNo] = img->mgrWork->weekValues[DATA_INDEX_inTemp][i];
        datasets[1].valueset[i-skipNo] = img->mgrWork->weekValues[DATA_INDEX_inHumidity][i];
    }

    return (htmlGenPngMultiChart (temp,
                          img->mgrWork->isMetricUnits,
                          datasets,
                          2,
                          7,
                          CHART_MIN_TEMP(img->mgrWork->isMetricUnits),
                          CHART_MAX_TEMP(img->mgrWork->isMetricUnits),
                          1.0,
                          labels,
                          WEEKLY_NUM_VALUES-skipNo,
                          WEEKLY_NUM_VALUES,
                          img->units,
                          img->decimalPlaces,
                          img->mgrWork->dateFormat,
                          img->mgrWork->isDualUnits));
}

static int generateCompositeTempMonth (HTML_IMG *img)
{
    int         i, j, skipNo;
    char        temp[256];
    char        lbls[MONTHLY_NUM_VALUES][8];
    time_t      ntime = img->mgrWork->monthStartTime_T;
    struct tm   loctime;
    float       min = 10000.0, max = -10000.0;
    MC_DATASET  datasets[2];

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, skipNo = 0; i < MONTHLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if (loctime.tm_hour == 0)
        {
            break;
        }
        else
        {
            skipNo ++;
            ntime += WV_SECONDS_IN_HOUR;
        }
    }

    for (i = 0; i < MONTHLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if ((i % 24) == 0)
        {
            if (loctime.tm_hour != 0)
            {
                // DST fall back nonsense:
                ntime += WV_SECONDS_IN_HOUR;
                localtime_r (&ntime, &loctime);
            }
        }
        sprintf (lbls[i], "%d", loctime.tm_mday);
        labels[i] = lbls[i];

        ntime += WV_SECONDS_IN_HOUR;
    }

    datasets[0].lineColor = htmlgenGetChartDefaultLine();
    datasets[1].lineColor = htmlgenGetChartDefaultSecondLine();
    
    memset (datasets[0].legend, 0, sizeof(datasets[0].legend));
    memset (datasets[1].legend, 0, sizeof(datasets[1].legend));
    for (i = 0; i < 15 && i < strlen (img->title); i ++)
    {
        if (img->title[i] == '/')
            break;
    
        datasets[0].legend[i] = img->title[i];
    }
    if (i < strlen (img->title) - 1)
    {
        wvstrncpy (datasets[1].legend, &img->title[i+1], sizeof(datasets[1].legend));
    }
    
    for (i = skipNo; i < MONTHLY_NUM_VALUES; i ++)
    {
        datasets[0].valueset[i-skipNo] = img->mgrWork->monthValues[DATA_INDEX_outTemp][i];
        datasets[1].valueset[i-skipNo] = img->mgrWork->monthValues[DATA_INDEX_dewpoint][i];

        if (img->mgrWork->monthValues[DATA_INDEX_outTemp][i] <= ARCHIVE_VALUE_NULL ||
            img->mgrWork->monthValues[DATA_INDEX_dewpoint][i] <= ARCHIVE_VALUE_NULL ||
            img->mgrWork->monthValues[DATA_INDEX_windchill][i] <= ARCHIVE_VALUE_NULL ||
            img->mgrWork->monthValues[DATA_INDEX_heatindex][i] <= ARCHIVE_VALUE_NULL)
        {
            continue;
        }
        
        setMin (&min, 
                img->mgrWork->monthValues[DATA_INDEX_outTemp][i],
                img->mgrWork->monthValues[DATA_INDEX_dewpoint][i],
                img->mgrWork->monthValues[DATA_INDEX_windchill][i],
                img->mgrWork->monthValues[DATA_INDEX_heatindex][i]);
                
        setMax (&max, 
                img->mgrWork->monthValues[DATA_INDEX_outTemp][i],
                img->mgrWork->monthValues[DATA_INDEX_dewpoint][i],
                img->mgrWork->monthValues[DATA_INDEX_windchill][i],
                img->mgrWork->monthValues[DATA_INDEX_heatindex][i]);
    }

    // if there is no data yet, pick default min/max
    if (min > max)
    {
        min = CHART_MIN_TEMP(img->mgrWork->isMetricUnits);
        max = CHART_MAX_TEMP(img->mgrWork->isMetricUnits);
    }

    return (htmlGenPngMultiChart (temp,
                          img->mgrWork->isMetricUnits,
                          datasets,
                          2,
                          14,
                          min,
                          max,
                          1.0,
                          labels,
                          MONTHLY_NUM_VALUES-skipNo,
                          MONTHLY_NUM_VALUES,
                          img->units,
                          img->decimalPlaces,
                          img->mgrWork->dateFormat,
                          img->mgrWork->isDualUnits));
}

static int generateCompositeHIndexWChillMonth (HTML_IMG *img)
{
    int         i, j, skipNo;
    char        temp[256];
    char        lbls[MONTHLY_NUM_VALUES][8];
    time_t      ntime = img->mgrWork->monthStartTime_T;
    struct tm   loctime;
    float       min = 10000.0, max = -10000.0;
    MC_DATASET  datasets[2];

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, skipNo = 0; i < MONTHLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if (loctime.tm_hour == 0)
        {
            break;
        }
        else
        {
            skipNo ++;
            ntime += WV_SECONDS_IN_HOUR;
        }
    }

    for (i = 0; i < MONTHLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if ((i % 24) == 0)
        {
            if (loctime.tm_hour != 0)
            {
                // DST fall back nonsense:
                ntime += WV_SECONDS_IN_HOUR;
                localtime_r (&ntime, &loctime);
            }
        }
        sprintf (lbls[i], "%d", loctime.tm_mday);
        labels[i] = lbls[i];

        ntime += WV_SECONDS_IN_HOUR;
    }

    datasets[0].lineColor = htmlgenGetChartDefaultLine();
    datasets[1].lineColor = htmlgenGetChartDefaultSecondLine();
    
    memset (datasets[0].legend, 0, sizeof(datasets[0].legend));
    memset (datasets[1].legend, 0, sizeof(datasets[1].legend));
    for (i = 0; i < 15 && i < strlen (img->title); i ++)
    {
        if (img->title[i] == '/')
            break;
    
        datasets[0].legend[i] = img->title[i];
    }
    if (i < strlen (img->title) - 1)
    {
        wvstrncpy (datasets[1].legend, &img->title[i+1], sizeof(datasets[1].legend));
    }
    
    for (i = skipNo; i < MONTHLY_NUM_VALUES; i ++)
    {
        datasets[0].valueset[i-skipNo] = img->mgrWork->monthValues[DATA_INDEX_windchill][i];
        datasets[1].valueset[i-skipNo] = img->mgrWork->monthValues[DATA_INDEX_heatindex][i];
        
        if (img->mgrWork->monthValues[DATA_INDEX_outTemp][i] <= ARCHIVE_VALUE_NULL ||
            img->mgrWork->monthValues[DATA_INDEX_dewpoint][i] <= ARCHIVE_VALUE_NULL ||
            img->mgrWork->monthValues[DATA_INDEX_windchill][i] <= ARCHIVE_VALUE_NULL ||
            img->mgrWork->monthValues[DATA_INDEX_heatindex][i] <= ARCHIVE_VALUE_NULL)
        {
            continue;
        }
        
        setMin (&min, 
                img->mgrWork->monthValues[DATA_INDEX_outTemp][i],
                img->mgrWork->monthValues[DATA_INDEX_dewpoint][i],
                img->mgrWork->monthValues[DATA_INDEX_windchill][i],
                img->mgrWork->monthValues[DATA_INDEX_heatindex][i]);
                
        setMax (&max, 
                img->mgrWork->monthValues[DATA_INDEX_outTemp][i],
                img->mgrWork->monthValues[DATA_INDEX_dewpoint][i],
                img->mgrWork->monthValues[DATA_INDEX_windchill][i],
                img->mgrWork->monthValues[DATA_INDEX_heatindex][i]);
    }

    // if there is no data yet, pick default min/max
    if (min > max)
    {
        min = CHART_MIN_TEMP(img->mgrWork->isMetricUnits);
        max = CHART_MAX_TEMP(img->mgrWork->isMetricUnits);
    }

    return (htmlGenPngMultiChart (temp,
                          img->mgrWork->isMetricUnits,
                          datasets,
                          2,
                          14,
                          min,
                          max,
                          1.0,
                          labels,
                          MONTHLY_NUM_VALUES-skipNo,
                          MONTHLY_NUM_VALUES,
                          img->units,
                          img->decimalPlaces,
                          img->mgrWork->dateFormat,
                          img->mgrWork->isDualUnits));
}

static int generateCompositeInTempHumidMonth (HTML_IMG *img)
{
    int         i, j, skipNo;
    char        temp[256];
    char        lbls[MONTHLY_NUM_VALUES][8];
    time_t      ntime = img->mgrWork->monthStartTime_T;
    struct tm   loctime;
    MC_DATASET  datasets[2];

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, skipNo = 0; i < MONTHLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if (loctime.tm_hour == 0)
        {
            break;
        }
        else
        {
            skipNo ++;
            ntime += WV_SECONDS_IN_HOUR;
        }
    }

    for (i = 0; i < MONTHLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if ((i % 24) == 0)
        {
            if (loctime.tm_hour != 0)
            {
                // DST fall back nonsense:
                ntime += WV_SECONDS_IN_HOUR;
                localtime_r (&ntime, &loctime);
            }
        }
        sprintf (lbls[i], "%d", loctime.tm_mday);
        labels[i] = lbls[i];

        ntime += WV_SECONDS_IN_HOUR;
    }

    datasets[0].lineColor = htmlgenGetChartDefaultLine();
    datasets[1].lineColor = htmlgenGetChartDefaultSecondLine();
    
    memset (datasets[0].legend, 0, sizeof(datasets[0].legend));
    memset (datasets[1].legend, 0, sizeof(datasets[1].legend));
    for (i = 0; i < 15 && i < strlen (img->title); i ++)
    {
        if (img->title[i] == '/')
            break;
    
        datasets[0].legend[i] = img->title[i];
    }
    if (i < strlen (img->title) - 1)
    {
        wvstrncpy (datasets[1].legend, &img->title[i+1], sizeof(datasets[1].legend));
    }
    
    for (i = skipNo; i < MONTHLY_NUM_VALUES; i ++)
    {
        datasets[0].valueset[i-skipNo] = img->mgrWork->monthValues[DATA_INDEX_inTemp][i];
        datasets[1].valueset[i-skipNo] = img->mgrWork->monthValues[DATA_INDEX_inHumidity][i];
    }

    return (htmlGenPngMultiChart (temp,
                          img->mgrWork->isMetricUnits,
                          datasets,
                          2,
                          14,
                          CHART_MIN_TEMP(img->mgrWork->isMetricUnits),
                          CHART_MAX_TEMP(img->mgrWork->isMetricUnits),
                          1.0,
                          labels,
                          MONTHLY_NUM_VALUES-skipNo,
                          MONTHLY_NUM_VALUES,
                          img->units,
                          img->decimalPlaces,
                          img->mgrWork->dateFormat,
                          img->mgrWork->isDualUnits));
}

static int generateCompositeWindDay (HTML_IMG *img)
{
    int         i, j;
    char        temp[256];
    MC_DATASET  datasets[2];
    float       min = 10000.0, max = -10000.0;
    char        units[16];

    // Make units consistent with global preference:
    strncpy(units, wvutilsGetWindUnitLabel(), 15);
    strncpy(img->units, units, 15);

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_SAMPLE))
    {
        // nothing for us here
        return ERROR_ABORT;
    }

    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, j = img->mgrWork->dayStart; i < DAILY_NUM_VALUES(img->mgrWork); i ++, j ++)
    {
        if (j >= DAILY_NUM_VALUES(img->mgrWork)-1)
            j = 0;

        labels[i] = sampleLabels[j];
    }

    datasets[0].lineColor = htmlgenGetChartDefaultLine();
    datasets[1].lineColor = htmlgenGetChartDefaultSecondLine();

    memset (datasets[0].legend, 0, 16);
    memset (datasets[1].legend, 0, 16);
    for (i = 0; i < 15 && i < strlen (img->title); i ++)
    {
        if (img->title[i] == '/')
            break;

        datasets[0].legend[i] = img->title[i];
    }
    if (i < strlen (img->title) - 1)
    {
        strncpy (datasets[1].legend, &img->title[i+1], 15);
    }

    for (i = 0; i < DAILY_NUM_VALUES(img->mgrWork); i ++)
    {
        datasets[0].valueset[i] = img->mgrWork->dayValues[DATA_INDEX_windSpeed][i];
        datasets[1].valueset[i] = img->mgrWork->dayValues[DATA_INDEX_windGust][i];

        if (img->mgrWork->dayValues[DATA_INDEX_windSpeed][i] == ARCHIVE_VALUE_NULL ||
            img->mgrWork->dayValues[DATA_INDEX_windGust][i] == ARCHIVE_VALUE_NULL)
        {
            continue;
        }

        if (img->mgrWork->dayValues[DATA_INDEX_windSpeed][i] < min)
            min = img->mgrWork->dayValues[DATA_INDEX_windSpeed][i];
        if (img->mgrWork->dayValues[DATA_INDEX_windGust][i] < min)
            min = img->mgrWork->dayValues[DATA_INDEX_windGust][i];

        if (img->mgrWork->dayValues[DATA_INDEX_windSpeed][i] > max)
            max = img->mgrWork->dayValues[DATA_INDEX_windSpeed][i];
        if (img->mgrWork->dayValues[DATA_INDEX_windGust][i] > max)
            max = img->mgrWork->dayValues[DATA_INDEX_windGust][i];
    }

    // if there is no data yet, pick default min/max
    if (min > max)
    {
        min = CHART_MIN_WSPEED(img->mgrWork->isMetricUnits);
        max = CHART_MAX_WSPEED(img->mgrWork->isMetricUnits);
    }

    return (htmlGenPngMultiChart (temp,
                          img->mgrWork->isMetricUnits,
                          datasets,
                          2,
                          6,
                          min,
                          max,
                          1.0,
                          labels,
                          DAILY_NUM_VALUES(img->mgrWork),
                          DAILY_NUM_VALUES(img->mgrWork),
                          units,
                          img->decimalPlaces,
                          img->mgrWork->dateFormat,
                          img->mgrWork->isDualUnits));
}

static int generateCompositeWindWeek (HTML_IMG *img)
{
    int         i, j, skipNo;
    char        temp[256];
    char        lbls[WEEKLY_NUM_VALUES][8];
    time_t      ntime = img->mgrWork->weekStartTime_T;
    struct tm   loctime;
    float       min = 10000.0, max = -10000.0;
    MC_DATASET  datasets[2];
    char        units[16];

    // Make units consistent with global preference:
    strncpy(units, wvutilsGetWindUnitLabel(), 15);
    strncpy(img->units, units, 15);

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }

    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, skipNo = 0; i < WEEKLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if (loctime.tm_hour == 0)
        {
            break;
        }
        else
        {
            skipNo ++;
            ntime += WV_SECONDS_IN_HOUR;
        }
    }

    for (i = 0; i < WEEKLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%d", loctime.tm_mday);
        labels[i] = lbls[i];

        ntime += WV_SECONDS_IN_HOUR;
    }

    datasets[0].lineColor = htmlgenGetChartDefaultLine();
    datasets[1].lineColor = htmlgenGetChartDefaultSecondLine();

    memset (datasets[0].legend, 0, 16);
    memset (datasets[1].legend, 0, 16);
    for (i = 0; i < 15 && i < strlen (img->title); i ++)
    {
        if (img->title[i] == '/')
            break;

        datasets[0].legend[i] = img->title[i];
    }
    if (i < strlen (img->title) - 1)
    {
        strncpy (datasets[1].legend, &img->title[i+1], 15);
    }

    for (i = skipNo; i < WEEKLY_NUM_VALUES; i ++)
    {
        datasets[0].valueset[i-skipNo] = img->mgrWork->weekValues[DATA_INDEX_windSpeed][i];
        datasets[1].valueset[i-skipNo] = img->mgrWork->weekValues[DATA_INDEX_windGust][i];

        if (img->mgrWork->weekValues[DATA_INDEX_windSpeed][i] == ARCHIVE_VALUE_NULL ||
            img->mgrWork->weekValues[DATA_INDEX_windGust][i] == ARCHIVE_VALUE_NULL)
        {
            continue;
        }

        if (img->mgrWork->weekValues[DATA_INDEX_windSpeed][i] < min)
            min = img->mgrWork->weekValues[DATA_INDEX_windSpeed][i];
        if (img->mgrWork->weekValues[DATA_INDEX_windGust][i] < min)
            img->mgrWork->weekValues[DATA_INDEX_windGust][i];

        if (img->mgrWork->weekValues[DATA_INDEX_windSpeed][i] > max)
            max = img->mgrWork->weekValues[DATA_INDEX_windSpeed][i];
        if (img->mgrWork->weekValues[DATA_INDEX_windGust][i] > max)
            max = img->mgrWork->weekValues[DATA_INDEX_windGust][i];
    }

    // if there is no data yet, pick default min/max
    if (min > max)
    {
        min = CHART_MIN_WSPEED(img->mgrWork->isMetricUnits);
        max = CHART_MAX_WSPEED(img->mgrWork->isMetricUnits);
    }

    return (htmlGenPngMultiChart (temp,
                          img->mgrWork->isMetricUnits,
                          datasets,
                          2,
                          7,
                          min,
                          max,
                          1.0,
                          labels,
                          WEEKLY_NUM_VALUES-skipNo,
                          WEEKLY_NUM_VALUES,
                          units,
                          img->decimalPlaces,
                          img->mgrWork->dateFormat,
                          img->mgrWork->isDualUnits));
}

static int generateCompositeWindMonth (HTML_IMG *img)
{
    int         i, j, skipNo;
    char        temp[256];
    char        lbls[MONTHLY_NUM_VALUES][8];
    time_t      ntime = img->mgrWork->monthStartTime_T;
    struct tm   loctime;
    float       min = 10000.0, max = -10000.0;
    MC_DATASET  datasets[2];
    char        units[16];

    // Make units consistent with global preference:
    strncpy(units, wvutilsGetWindUnitLabel(), 15);
    strncpy(img->units, units, 15);

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }

    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, skipNo = 0; i < MONTHLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if (loctime.tm_hour == 0)
        {
            break;
        }
        else
        {
            skipNo ++;
            ntime += WV_SECONDS_IN_HOUR;
        }
    }

    for (i = 0; i < MONTHLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%d", loctime.tm_mday);
        labels[i] = lbls[i];

        ntime += WV_SECONDS_IN_HOUR;
    }

    datasets[0].lineColor = htmlgenGetChartDefaultLine();
    datasets[1].lineColor = htmlgenGetChartDefaultSecondLine();

    memset (datasets[0].legend, 0, 16);
    memset (datasets[1].legend, 0, 16);
    for (i = 0; i < 15 && i < strlen (img->title); i ++)
    {
        if (img->title[i] == '/')
            break;

        datasets[0].legend[i] = img->title[i];
    }
    if (i < strlen (img->title) - 1)
    {
        strncpy (datasets[1].legend, &img->title[i+1], 15);
    }

    for (i = skipNo; i < MONTHLY_NUM_VALUES; i ++)
    {
        datasets[0].valueset[i-skipNo] = img->mgrWork->monthValues[DATA_INDEX_windSpeed][i];
        datasets[1].valueset[i-skipNo] = img->mgrWork->monthValues[DATA_INDEX_windGust][i];

        if (img->mgrWork->monthValues[DATA_INDEX_windSpeed][i] == ARCHIVE_VALUE_NULL ||
            img->mgrWork->monthValues[DATA_INDEX_windGust][i] == ARCHIVE_VALUE_NULL)
        {
            continue;
        }

        if (img->mgrWork->monthValues[DATA_INDEX_windSpeed][i] < min)
            min = img->mgrWork->monthValues[DATA_INDEX_windSpeed][i];
        if (img->mgrWork->monthValues[DATA_INDEX_windGust][i] < min)
            min = img->mgrWork->monthValues[DATA_INDEX_windGust][i];

        if (img->mgrWork->monthValues[DATA_INDEX_windSpeed][i] > max)
            max = img->mgrWork->monthValues[DATA_INDEX_windSpeed][i];
        if (img->mgrWork->monthValues[DATA_INDEX_windGust][i] > max)
            max = img->mgrWork->monthValues[DATA_INDEX_windGust][i];
    }

    // if there is no data yet, pick default min/max
    if (min > max)
    {
        min = CHART_MIN_WSPEED(img->mgrWork->isMetricUnits);
        max = CHART_MAX_WSPEED(img->mgrWork->isMetricUnits);
    }

    return (htmlGenPngMultiChart (temp,
                          img->mgrWork->isMetricUnits,
                          datasets,
                          2,
                          14,
                          min,
                          max,
                          1.0,
                          labels,
                          MONTHLY_NUM_VALUES-skipNo,
                          MONTHLY_NUM_VALUES,
                          units,
                          img->decimalPlaces,
                          img->mgrWork->dateFormat,
                          img->mgrWork->isDualUnits));
}

static int generateCompositeWindYear (HTML_IMG *img)
{
    int         i;
    char        lbls[YEARLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->yearStartTime_T;
    struct tm   loctime;
    MC_DATASET  datasets[2];
    float       min = 10000.0, max = -10000.0;
    char        units[16];

    // Make units consistent with global preference:
    strncpy(units, wvutilsGetWindUnitLabel(), 15);
    strncpy(img->units, units, 15);

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_DAY))
    {
        // nothing for us here
        return ERROR_ABORT;
    }

    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0; i < YEARLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%s %d", monthLabels[loctime.tm_mon], loctime.tm_mday);
        labels[i] = lbls[i];
        ntime += WV_SECONDS_IN_DAY;
    }

    datasets[0].lineColor = htmlgenGetChartDefaultLine();
    datasets[1].lineColor = htmlgenGetChartDefaultSecondLine();

    memset (datasets[0].legend, 0, 16);
    memset (datasets[1].legend, 0, 16);
    for (i = 0; i < 15 && i < strlen (img->title); i ++)
    {
        if (img->title[i] == '/')
            break;

        datasets[0].legend[i] = img->title[i];
    }
    if (i < strlen (img->title) - 1)
    {
        strncpy (datasets[1].legend, &img->title[i+1], 15);
    }

    for (i = 0; i < YEARLY_NUM_VALUES; i ++)
    {
        datasets[0].valueset[i] = img->mgrWork->yearValues[DATA_INDEX_windSpeed][i];
        datasets[1].valueset[i] = img->mgrWork->yearValues[DATA_INDEX_windGust][i];

        if (img->mgrWork->yearValues[DATA_INDEX_windSpeed][i] == ARCHIVE_VALUE_NULL ||
            img->mgrWork->yearValues[DATA_INDEX_windGust][i] == ARCHIVE_VALUE_NULL)
        {
            continue;
        }

        if (img->mgrWork->yearValues[DATA_INDEX_windSpeed][i] < min)
            min = img->mgrWork->yearValues[DATA_INDEX_windSpeed][i];
        if (img->mgrWork->yearValues[DATA_INDEX_windGust][i] < min)
            min = img->mgrWork->yearValues[DATA_INDEX_windGust][i];

        if (img->mgrWork->yearValues[DATA_INDEX_windSpeed][i] > max)
            max = img->mgrWork->yearValues[DATA_INDEX_windSpeed][i];
        if (img->mgrWork->yearValues[DATA_INDEX_windGust][i] > max)
            max = img->mgrWork->yearValues[DATA_INDEX_windGust][i];
    }

    // if there is no data yet, pick default min/max
    if (min > max)
    {
        min = CHART_MIN_WSPEED(img->mgrWork->isMetricUnits);
        max = CHART_MAX_WSPEED(img->mgrWork->isMetricUnits);
    }

    return (htmlGenPngMultiChart (temp,
                          img->mgrWork->isMetricUnits,
                          datasets,
                          2,
                          4,
                          min,
                          max,
                          1.0,
                          labels,
                          YEARLY_NUM_VALUES,
                          YEARLY_NUM_VALUES,
                          units,
                          img->decimalPlaces,
                          img->mgrWork->dateFormat,
                          img->mgrWork->isDualUnits));
}

//////////////////////////////////////////////////////////////////////////////
//////////////  E X T E N D E D   D A T A   B E G I N  ///////////////////////
//////////////////////////////////////////////////////////////////////////////

// first a few buckets for common extended items

static int generateUV (HTML_IMG *img)
{
    char        temp[256];

    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    return (htmlGenPngBucket (temp,
                      0,
                      (float)img->mgrWork->loopStore.UV,
                      (float)GLB_HILOW_NONE,
                      (float)GLB_HILOW_NONE,
                      img->title,
                      img->units,
                      img->decimalPlaces,
                      0.0,
                      4.0,
                      img->mgrWork->dateFormat,
                      img->mgrWork->isDualUnits));
}

static int generateRadiation (HTML_IMG *img)
{
    char        temp[256];

    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    return (htmlGenPngBucket (temp,
                      0,
                      (float)img->mgrWork->loopStore.radiation,
                      (float)GLB_HILOW_NONE,
                      (float)GLB_HILOW_NONE,
                      img->title,
                      img->units,
                      img->decimalPlaces,
                      0.0,
                      4.0,
                      img->mgrWork->dateFormat,
                      img->mgrWork->isDualUnits));
}

static int generateET (HTML_IMG *img)
{
    char        temp[256];
    float       tempfloat;

    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    if (img->mgrWork->isMetricUnits)
    {
        tempfloat = wvutilsConvertRainINToMetric (img->mgrWork->loopStore.dayET);

        return (htmlGenPngBucket (temp,
                          1,
                          tempfloat,
                          (float)GLB_HILOW_NONE,
                          (float)GLB_HILOW_NONE,
                          img->title,
                          img->units,
                          img->decimalPlaces,
                          0.0,
                          0.01,
                          img->mgrWork->dateFormat,
                          img->mgrWork->isDualUnits));
    }
    else
    {
        return (htmlGenPngBucket (temp,
                          0,
                          img->mgrWork->loopStore.dayET,
                          (float)GLB_HILOW_NONE,
                          (float)GLB_HILOW_NONE,
                          img->title,
                          img->units,
                          img->decimalPlaces,
                          0.0,
                          0.01,
                          img->mgrWork->dateFormat,
                          img->mgrWork->isDualUnits));
    }
}

static int generateRadiationDay (HTML_IMG *img)
{
    int         i, j;
    char        temp[256];

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_SAMPLE))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, j = img->mgrWork->dayStart; i < DAILY_NUM_VALUES(img->mgrWork) - 1; i ++, j ++)
    {
        if (j >= DAILY_NUM_VALUES(img->mgrWork) - 1)
            j = 0;

        labels[i] = sampleLabels[j];
    }
    labels[i] = sampleLabels[DAILY_NUM_VALUES(img->mgrWork) - 1];
    
    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->dayValues[DATA_INDEX_radiation],
                     6,
                     0,
                     10,
                     10,
                     labels,
                     DAILY_NUM_VALUES(img->mgrWork),
                     DAILY_NUM_VALUES(img->mgrWork),
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     0));
}

static int generateRadiationWeek (HTML_IMG *img)
{
    int         i, skipNo;
    char        lbls[WEEKLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->weekStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, skipNo = 0; i < WEEKLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if (loctime.tm_hour == 0)
        {
            break;
        }
        else
        {
            skipNo ++;
            ntime += WV_SECONDS_IN_HOUR;
        }
    }

    for (i = 0; i < WEEKLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if ((i % 24) == 0)
        {
            if (loctime.tm_hour != 0)
            {
                // DST fall back nonsense:
                ntime += WV_SECONDS_IN_HOUR;
                localtime_r (&ntime, &loctime);
            }
        }
        sprintf (lbls[i], "%d", loctime.tm_mday);
        labels[i] = lbls[i];

        ntime += WV_SECONDS_IN_HOUR;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     &img->mgrWork->weekValues[DATA_INDEX_radiation][skipNo],
                     7,
                     0,
                     100,
                     100,
                     labels,
                     WEEKLY_NUM_VALUES-skipNo,
                     WEEKLY_NUM_VALUES,
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     0));
}

static int generateRadiationMonth (HTML_IMG *img)
{
    int         i, skipNo;
    char        lbls[MONTHLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->monthStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, skipNo = 0; i < MONTHLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if (loctime.tm_hour == 0)
        {
            break;
        }
        else
        {
            skipNo ++;
            ntime += WV_SECONDS_IN_HOUR;
        }
    }

    for (i = 0; i < MONTHLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if ((i % 24) == 0)
        {
            if (loctime.tm_hour != 0)
            {
                // DST fall back nonsense:
                ntime += WV_SECONDS_IN_HOUR;
                localtime_r (&ntime, &loctime);
            }
        }
        sprintf (lbls[i], "%d", loctime.tm_mday);
        labels[i] = lbls[i];

        ntime += WV_SECONDS_IN_HOUR;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     &img->mgrWork->monthValues[DATA_INDEX_radiation][skipNo],
                     14,
                     0,
                     100,
                     100,
                     labels,
                     MONTHLY_NUM_VALUES-skipNo,
                     MONTHLY_NUM_VALUES,
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     0));
}

static int generateRadiationYear (HTML_IMG *img)
{
    int         i;
    char        lbls[YEARLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->yearStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_DAY))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);
    ntime += 4*WV_SECONDS_IN_HOUR;
    for (i = 0; i < YEARLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%s %d", monthLabels[loctime.tm_mon], loctime.tm_mday);
        labels[i] = lbls[i];
        ntime += WV_SECONDS_IN_DAY;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->yearValues[DATA_INDEX_radiation],
                     4,
                     0,
                     100,
                     100,
                     labels,
                     YEARLY_NUM_VALUES,
                     YEARLY_NUM_VALUES,
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     0));
}


static int generateUVDay (HTML_IMG *img)
{
    int         i, j;
    char        temp[256];

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_SAMPLE))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, j = img->mgrWork->dayStart; i < DAILY_NUM_VALUES(img->mgrWork) - 1; i ++, j ++)
    {
        if (j >= DAILY_NUM_VALUES(img->mgrWork) - 1)
            j = 0;

        labels[i] = sampleLabels[j];
    }
    labels[i] = sampleLabels[DAILY_NUM_VALUES(img->mgrWork) - 1];
    
    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->dayValues[DATA_INDEX_UV],
                     6,
                     0,
                     5,
                     1.0,
                     labels,
                     DAILY_NUM_VALUES(img->mgrWork),
                     DAILY_NUM_VALUES(img->mgrWork),
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     0));
}

static int generateUVWeek (HTML_IMG *img)
{
    int         i, skipNo;
    char        lbls[WEEKLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->weekStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, skipNo = 0; i < WEEKLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if (loctime.tm_hour == 0)
        {
            break;
        }
        else
        {
            skipNo ++;
            ntime += WV_SECONDS_IN_HOUR;
        }
    }

    for (i = 0; i < WEEKLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if ((i % 24) == 0)
        {
            if (loctime.tm_hour != 0)
            {
                // DST fall back nonsense:
                ntime += WV_SECONDS_IN_HOUR;
                localtime_r (&ntime, &loctime);
            }
        }
        sprintf (lbls[i], "%d", loctime.tm_mday);
        labels[i] = lbls[i];

        ntime += WV_SECONDS_IN_HOUR;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     &img->mgrWork->weekValues[DATA_INDEX_UV][skipNo],
                     7,
                     0,
                     5,
                     1.0,
                     labels,
                     WEEKLY_NUM_VALUES-skipNo,
                     WEEKLY_NUM_VALUES,
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     0));
}

static int generateUVMonth (HTML_IMG *img)
{
    int         i, skipNo;
    char        lbls[MONTHLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->monthStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, skipNo = 0; i < MONTHLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if (loctime.tm_hour == 0)
        {
            break;
        }
        else
        {
            skipNo ++;
            ntime += WV_SECONDS_IN_HOUR;
        }
    }

    for (i = 0; i < MONTHLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if ((i % 24) == 0)
        {
            if (loctime.tm_hour != 0)
            {
                // DST fall back nonsense:
                ntime += WV_SECONDS_IN_HOUR;
                localtime_r (&ntime, &loctime);
            }
        }
        sprintf (lbls[i], "%d", loctime.tm_mday);
        labels[i] = lbls[i];

        ntime += WV_SECONDS_IN_HOUR;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     &img->mgrWork->monthValues[DATA_INDEX_UV][skipNo],
                     14,
                     0,
                     5,
                     1.0,
                     labels,
                     MONTHLY_NUM_VALUES-skipNo,
                     MONTHLY_NUM_VALUES,
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     0));
}

static int generateUVYear (HTML_IMG *img)
{
    int         i;
    char        lbls[YEARLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->yearStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_DAY))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);
    ntime += 4*WV_SECONDS_IN_HOUR;
    for (i = 0; i < YEARLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%s %d", monthLabels[loctime.tm_mon], loctime.tm_mday);
        labels[i] = lbls[i];
        ntime += WV_SECONDS_IN_DAY;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->yearValues[DATA_INDEX_UV],
                     4,
                     0,
                     5,
                     1.0,
                     labels,
                     YEARLY_NUM_VALUES,
                     YEARLY_NUM_VALUES,
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     0));
}


static int generateETDay (HTML_IMG *img)
{
    int         i, j, skipNo = 0;
    int         samplesInHour = (60/img->mgrWork->archiveInterval);
    char        temp[256];
    float       max = 0;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_SAMPLE))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    j = img->mgrWork->dayStart;
    while ((j % samplesInHour) != 0)
    {
        skipNo ++;
        if (++ j >= DAILY_NUM_VALUES(img->mgrWork) - 1)
            j = 0;
    }

    for (i = 0; i < DAILY_NUM_VALUES(img->mgrWork)-skipNo; i ++)
    {
        labels[i] = sampleHourLabels[j];

        if (img->mgrWork->dayValues[DATA_INDEX_ET][i+skipNo] > max)
            max = img->mgrWork->dayValues[DATA_INDEX_ET][i+skipNo];

        if (++ j >= DAILY_NUM_VALUES(img->mgrWork) - 1)
            j = 0;
    }
    
    // tack on the extra label
    labels[(DAILY_NUM_VALUES(img->mgrWork)-skipNo)] = sampleHourLabels[(DAILY_NUM_VALUES(img->mgrWork)-1)];

    // add 2 to the skipNo since the VP console burps out ET at :00 -OR- :05 !!!
    return (htmlGenPngBarChart (temp,
                        img->mgrWork->isMetricUnits,
                        &img->mgrWork->dayValues[DATA_INDEX_ET][skipNo+2],
                        6,
                        0,
                        MAX(0.03,max),
                        0.01,
                        labels,
                        DAILY_NUM_VALUES(img->mgrWork)-skipNo,
                        img->title,
                        img->units,
                        img->decimalPlaces,
                        60/img->mgrWork->archiveInterval,
                        24,
                        60/img->mgrWork->archiveInterval,
                        img->mgrWork->dateFormat,
                        img->mgrWork->isDualUnits));
}

static int generateETWeek (HTML_IMG *img)
{
    int         i, skipNo;
    char        lbls[WEEKLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->weekStartTime_T;
    struct tm   loctime;
    float       max = 0;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, skipNo = 0; i < WEEKLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if (loctime.tm_hour == 0)
        {
            break;
        }
        else
        {
            skipNo ++;
            ntime += WV_SECONDS_IN_HOUR;
        }
    }

    // kludge this for aggregate values
    if (skipNo == 0)
    {
        // push forward one day
        skipNo = 24;
        ntime += WV_SECONDS_IN_DAY;
    }

    for (i = 0; i < WEEKLY_NUM_VALUES-skipNo; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%d", loctime.tm_mday);
        labels[i] = lbls[i];

        ntime += WV_SECONDS_IN_HOUR;
        if (img->mgrWork->weekValues[DATA_INDEX_ET][i] > max)
            max = img->mgrWork->weekValues[DATA_INDEX_ET][i];
    }

    // tack on the extra label
    if (skipNo > 1)
    {
        ntime += WV_SECONDS_IN_DAY;
    }

    localtime_r (&ntime, &loctime);
    sprintf (lbls[WEEKLY_NUM_VALUES-skipNo], "%d", loctime.tm_mday);
    labels[WEEKLY_NUM_VALUES-skipNo] = lbls[WEEKLY_NUM_VALUES-skipNo];

    return (htmlGenPngBarChart (temp,
                        img->mgrWork->isMetricUnits,
                        &img->mgrWork->weekValues[DATA_INDEX_ET][skipNo],
                        7,
                        0,
                        MAX(0.03,max),
                        0.01,
                        labels,
                        WEEKLY_NUM_VALUES-skipNo,
                        img->title,
                        img->units,
                        img->decimalPlaces,
                        24,
                        7,
                        24,
                        img->mgrWork->dateFormat,
                        img->mgrWork->isDualUnits));
}

static int generateETMonth (HTML_IMG *img)
{
    int         i, skipNo;
    char        lbls[MONTHLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->monthStartTime_T;
    struct tm   loctime;
    float       max = 0;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, skipNo = 0; i < MONTHLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if (loctime.tm_hour == 0)
        {
            break;
        }
        else
        {
            skipNo ++;
            ntime += WV_SECONDS_IN_HOUR;
        }
    }

    // kludge this for aggregate values
    if (skipNo == 0)
    {
        // push forward one day
        skipNo = 24;
        ntime += WV_SECONDS_IN_DAY;
    }

    for (i = 0; i < MONTHLY_NUM_VALUES-skipNo; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%d", loctime.tm_mday);
        labels[i] = lbls[i];

        ntime += WV_SECONDS_IN_HOUR;
        if (img->mgrWork->monthValues[DATA_INDEX_ET][i] > max)
            max = img->mgrWork->monthValues[DATA_INDEX_ET][i];
    }

    // tack on the extra label
    if (skipNo > 1)
    {
        ntime += WV_SECONDS_IN_DAY;
    }

    localtime_r (&ntime, &loctime);
    sprintf (lbls[MONTHLY_NUM_VALUES-skipNo], "%d", loctime.tm_mday);
    labels[MONTHLY_NUM_VALUES-skipNo] = lbls[MONTHLY_NUM_VALUES-skipNo];

    return (htmlGenPngBarChart (temp,
                        img->mgrWork->isMetricUnits,
                        &img->mgrWork->monthValues[DATA_INDEX_ET][skipNo],
                        14,
                        0,
                        MAX(0.03,max),
                        0.01,
                        labels,
                        MONTHLY_NUM_VALUES-skipNo,
                        img->title,
                        img->units,
                        img->decimalPlaces,
                        24,
                        28,
                        24,
                        img->mgrWork->dateFormat,
                        img->mgrWork->isDualUnits));
}

static int generateETYear (HTML_IMG *img)
{
    int         i, j, skipNo;
    char        lbls[YEARLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->yearStartTime_T;
    struct tm   loctime;
    float       max = 0;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_DAY))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    // move forward 1 day to land on 52 weeks ago
    ntime += WV_SECONDS_IN_DAY;
    skipNo = 1;
    
    // if we are sitting on Sunday, bump it another day
    localtime_r (&ntime, &loctime);
    if (loctime.tm_wday == 0)
    {
        ntime += WV_SECONDS_IN_DAY;
        skipNo ++;
    }
    
    // move to the next Sunday
    for (i = 0; i < 7; i ++)
    {
        localtime_r (&ntime, &loctime);
        if (loctime.tm_wday == 0)
        {
            break;
        }
        else
        {
            skipNo ++;
            ntime += WV_SECONDS_IN_DAY;
        }
    }

    for (i = 0; i < YEARLY_NUM_VALUES-skipNo; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%s %d", monthLabels[loctime.tm_mon], loctime.tm_mday);
        labels[i] = lbls[i];

        ntime += WV_SECONDS_IN_DAY;
        if (img->mgrWork->yearValues[DATA_INDEX_ET][i] > max)
            max = img->mgrWork->yearValues[DATA_INDEX_ET][i];
    }

    for (j = 0; j < 7; j ++)
    {
        localtime_r (&ntime, &loctime);
        if (loctime.tm_wday == 0)
        {
            break;
        }
        else
        {
            ntime += WV_SECONDS_IN_DAY;
        }
    }
    
    localtime_r (&ntime, &loctime);
    sprintf (lbls[YEARLY_NUM_VALUES-skipNo], "%s %d", monthLabels[loctime.tm_mon], loctime.tm_mday);
    labels[YEARLY_NUM_VALUES-skipNo] = lbls[YEARLY_NUM_VALUES-skipNo];

    return (htmlGenPngBarChart (temp,
                        img->mgrWork->isMetricUnits,
                        &img->mgrWork->yearValues[DATA_INDEX_ET][skipNo],
                        4,
                        0,
                        MAX(0.3,max),
                        0.1,
                        labels,
                        YEARLY_NUM_VALUES-skipNo,
                        img->title,
                        img->units,
                        img->decimalPlaces,
                        7,
                        52,
                        7,
                        img->mgrWork->dateFormat,
                        img->mgrWork->isDualUnits));
}


static int generateLeafTemp1Day (HTML_IMG *img)
{
    int         i, j;
    char        temp[256];

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_SAMPLE))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, j = img->mgrWork->dayStart; i < DAILY_NUM_VALUES(img->mgrWork) - 1; i ++, j ++)
    {
        if (j >= DAILY_NUM_VALUES(img->mgrWork) - 1)
            j = 0;

        labels[i] = sampleLabels[j];
    }
    labels[i] = sampleLabels[DAILY_NUM_VALUES(img->mgrWork) - 1];
    
    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->dayValues[DATA_INDEX_leafTemp1],
                     6,
                     CHART_MIN_TEMP(img->mgrWork->isMetricUnits),
                     CHART_MAX_TEMP(img->mgrWork->isMetricUnits),
                     1.0,
                     labels,
                     DAILY_NUM_VALUES(img->mgrWork),
                     DAILY_NUM_VALUES(img->mgrWork),
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     img->mgrWork->isDualUnits));
}

static int generateLeafTemp1Month (HTML_IMG *img)
{
    int         i;
    char        lbls[MONTHLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->monthStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0; i < MONTHLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%d", loctime.tm_mday);
        labels[i] = lbls[i];
        ntime += WV_SECONDS_IN_HOUR;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->monthValues[DATA_INDEX_leafTemp1],
                     14,
                     CHART_MIN_TEMP(img->mgrWork->isMetricUnits),
                     CHART_MAX_TEMP(img->mgrWork->isMetricUnits),
                     1.0,
                     labels,
                     MONTHLY_NUM_VALUES,
                     MONTHLY_NUM_VALUES,
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     img->mgrWork->isDualUnits));
}

static int generateLeafTemp1Year (HTML_IMG *img)
{
    int         i;
    char        lbls[YEARLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->yearStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_DAY))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);
    ntime += 4*WV_SECONDS_IN_HOUR;
    for (i = 0; i < YEARLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%s %d", monthLabels[loctime.tm_mon], loctime.tm_mday);
        labels[i] = lbls[i];
        ntime += WV_SECONDS_IN_DAY;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->yearValues[DATA_INDEX_leafTemp1],
                     4,
                     CHART_MIN_TEMP(img->mgrWork->isMetricUnits),
                     CHART_MAX_TEMP(img->mgrWork->isMetricUnits),
                     1.0,
                     labels,
                     YEARLY_NUM_VALUES,
                     YEARLY_NUM_VALUES,
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     img->mgrWork->isDualUnits));
}


static int generateLeafTemp2Day (HTML_IMG *img)
{
    int         i, j;
    char        temp[256];

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_SAMPLE))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, j = img->mgrWork->dayStart; i < DAILY_NUM_VALUES(img->mgrWork) - 1; i ++, j ++)
    {
        if (j >= DAILY_NUM_VALUES(img->mgrWork) - 1)
            j = 0;

        labels[i] = sampleLabels[j];
    }
    labels[i] = sampleLabels[DAILY_NUM_VALUES(img->mgrWork) - 1];
    
    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->dayValues[DATA_INDEX_leafTemp2],
                     6,
                     CHART_MIN_TEMP(img->mgrWork->isMetricUnits),
                     CHART_MAX_TEMP(img->mgrWork->isMetricUnits),
                     1.0,
                     labels,
                     DAILY_NUM_VALUES(img->mgrWork),
                     DAILY_NUM_VALUES(img->mgrWork),
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     img->mgrWork->isDualUnits));
}

static int generateLeafTemp2Month (HTML_IMG *img)
{
    int         i;
    char        lbls[MONTHLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->monthStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0; i < MONTHLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%d", loctime.tm_mday);
        labels[i] = lbls[i];
        ntime += WV_SECONDS_IN_HOUR;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->monthValues[DATA_INDEX_leafTemp2],
                     14,
                     CHART_MIN_TEMP(img->mgrWork->isMetricUnits),
                     CHART_MAX_TEMP(img->mgrWork->isMetricUnits),
                     1.0,
                     labels,
                     MONTHLY_NUM_VALUES,
                     MONTHLY_NUM_VALUES,
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     img->mgrWork->isDualUnits));
}

static int generateLeafTemp2Year (HTML_IMG *img)
{
    int         i;
    char        lbls[YEARLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->yearStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_DAY))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    ntime += 4*WV_SECONDS_IN_HOUR;
    for (i = 0; i < YEARLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%s %d", monthLabels[loctime.tm_mon], loctime.tm_mday);
        labels[i] = lbls[i];
        ntime += WV_SECONDS_IN_DAY;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->yearValues[DATA_INDEX_leafTemp2],
                     4,
                     CHART_MIN_TEMP(img->mgrWork->isMetricUnits),
                     CHART_MAX_TEMP(img->mgrWork->isMetricUnits),
                     1.0,
                     labels,
                     YEARLY_NUM_VALUES,
                     YEARLY_NUM_VALUES,
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     img->mgrWork->isDualUnits));
}


static int generateLeafWetness1Day (HTML_IMG *img)
{
    int         i, j;
    char        temp[256];

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_SAMPLE))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, j = img->mgrWork->dayStart; i < DAILY_NUM_VALUES(img->mgrWork) - 1; i ++, j ++)
    {
        if (j >= DAILY_NUM_VALUES(img->mgrWork) - 1)
            j = 0;

        labels[i] = sampleLabels[j];
    }
    labels[i] = sampleLabels[DAILY_NUM_VALUES(img->mgrWork) - 1];
    
    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->dayValues[DATA_INDEX_leafWet1],
                     6,
                     0,
                     5,
                     1.0,
                     labels,
                     DAILY_NUM_VALUES(img->mgrWork),
                     DAILY_NUM_VALUES(img->mgrWork),
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     0));
}

static int generateLeafWetness1Month (HTML_IMG *img)
{
    int         i;
    char        lbls[MONTHLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->monthStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0; i < MONTHLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%d", loctime.tm_mday);
        labels[i] = lbls[i];
        ntime += WV_SECONDS_IN_HOUR;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->monthValues[DATA_INDEX_leafWet1],
                     14,
                     0,
                     5,
                     1.0,
                     labels,
                     MONTHLY_NUM_VALUES,
                     MONTHLY_NUM_VALUES,
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     0));

    return OK;
}

static int generateLeafWetness1Year (HTML_IMG *img)
{
    int         i;
    char        lbls[YEARLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->yearStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_DAY))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    ntime += 4*WV_SECONDS_IN_HOUR;
    for (i = 0; i < YEARLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%s %d", monthLabels[loctime.tm_mon], loctime.tm_mday);
        labels[i] = lbls[i];
        ntime += WV_SECONDS_IN_DAY;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->yearValues[DATA_INDEX_leafWet1],
                     4,
                     0,
                     5,
                     1.0,
                     labels,
                     YEARLY_NUM_VALUES,
                     YEARLY_NUM_VALUES,
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     0));
}


static int generateLeafWetness2Day (HTML_IMG *img)
{
    int         i, j;
    char        temp[256];

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_SAMPLE))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, j = img->mgrWork->dayStart; i < DAILY_NUM_VALUES(img->mgrWork) - 1; i ++, j ++)
    {
        if (j >= DAILY_NUM_VALUES(img->mgrWork) - 1)
            j = 0;

        labels[i] = sampleLabels[j];
    }
    labels[i] = sampleLabels[DAILY_NUM_VALUES(img->mgrWork) - 1];
    
    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->dayValues[DATA_INDEX_leafWet2],
                     6,
                     0,
                     5,
                     1.0,
                     labels,
                     DAILY_NUM_VALUES(img->mgrWork),
                     DAILY_NUM_VALUES(img->mgrWork),
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     0));
}

static int generateLeafWetness2Month (HTML_IMG *img)
{
    int         i;
    char        lbls[MONTHLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->monthStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0; i < MONTHLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%d", loctime.tm_mday);
        labels[i] = lbls[i];
        ntime += WV_SECONDS_IN_HOUR;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->monthValues[DATA_INDEX_leafWet2],
                     14,
                     0,
                     5,
                     1.0,
                     labels,
                     MONTHLY_NUM_VALUES,
                     MONTHLY_NUM_VALUES,
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     0));
}

static int generateLeafWetness2Year (HTML_IMG *img)
{
    int         i;
    char        lbls[YEARLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->yearStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_DAY))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    ntime += 4*WV_SECONDS_IN_HOUR;
    for (i = 0; i < YEARLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%s %d", monthLabels[loctime.tm_mon], loctime.tm_mday);
        labels[i] = lbls[i];
        ntime += WV_SECONDS_IN_DAY;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->yearValues[DATA_INDEX_leafWet2],
                     4,
                     0,
                     5,
                     1.0,
                     labels,
                     YEARLY_NUM_VALUES,
                     YEARLY_NUM_VALUES,
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     0));
}


static int generateSoilTemp1Day (HTML_IMG *img)
{
    int         i, j;
    char        temp[256];

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_SAMPLE))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, j = img->mgrWork->dayStart; i < DAILY_NUM_VALUES(img->mgrWork) - 1; i ++, j ++)
    {
        if (j >= DAILY_NUM_VALUES(img->mgrWork) - 1)
            j = 0;

        labels[i] = sampleLabels[j];
    }
    labels[i] = sampleLabels[DAILY_NUM_VALUES(img->mgrWork) - 1];
    
    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->dayValues[DATA_INDEX_soilTemp1],
                     6,
                     CHART_MIN_TEMP(img->mgrWork->isMetricUnits),
                     CHART_MAX_TEMP(img->mgrWork->isMetricUnits),
                     1.0,
                     labels,
                     DAILY_NUM_VALUES(img->mgrWork),
                     DAILY_NUM_VALUES(img->mgrWork),
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     img->mgrWork->isDualUnits));
}

static int generateSoilTemp1Month (HTML_IMG *img)
{
    int         i;
    char        lbls[MONTHLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->monthStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0; i < MONTHLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%d", loctime.tm_mday);
        labels[i] = lbls[i];
        ntime += WV_SECONDS_IN_HOUR;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->monthValues[DATA_INDEX_soilTemp1],
                     14,
                     CHART_MIN_TEMP(img->mgrWork->isMetricUnits),
                     CHART_MAX_TEMP(img->mgrWork->isMetricUnits),
                     1.0,
                     labels,
                     MONTHLY_NUM_VALUES,
                     MONTHLY_NUM_VALUES,
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     img->mgrWork->isDualUnits));
}

static int generateSoilTemp1Year (HTML_IMG *img)
{
    int         i;
    char        lbls[YEARLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->yearStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_DAY))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    ntime += 4*WV_SECONDS_IN_HOUR;
    for (i = 0; i < YEARLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%s %d", monthLabels[loctime.tm_mon], loctime.tm_mday);
        labels[i] = lbls[i];
        ntime += WV_SECONDS_IN_DAY;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->yearValues[DATA_INDEX_soilTemp1],
                     4,
                     CHART_MIN_TEMP(img->mgrWork->isMetricUnits),
                     CHART_MAX_TEMP(img->mgrWork->isMetricUnits),
                     1.0,
                     labels,
                     YEARLY_NUM_VALUES,
                     YEARLY_NUM_VALUES,
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     img->mgrWork->isDualUnits));
}


static int generateSoilTemp2Day (HTML_IMG *img)
{
    int         i, j;
    char        temp[256];

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_SAMPLE))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, j = img->mgrWork->dayStart; i < DAILY_NUM_VALUES(img->mgrWork) - 1; i ++, j ++)
    {
        if (j >= DAILY_NUM_VALUES(img->mgrWork) - 1)
            j = 0;

        labels[i] = sampleLabels[j];
    }
    labels[i] = sampleLabels[DAILY_NUM_VALUES(img->mgrWork) - 1];
    
    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->dayValues[DATA_INDEX_soilTemp2],
                     6,
                     CHART_MIN_TEMP(img->mgrWork->isMetricUnits),
                     CHART_MAX_TEMP(img->mgrWork->isMetricUnits),
                     1.0,
                     labels,
                     DAILY_NUM_VALUES(img->mgrWork),
                     DAILY_NUM_VALUES(img->mgrWork),
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     img->mgrWork->isDualUnits));
}

static int generateSoilTemp2Month (HTML_IMG *img)
{
    int         i;
    char        lbls[MONTHLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->monthStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0; i < MONTHLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%d", loctime.tm_mday);
        labels[i] = lbls[i];
        ntime += WV_SECONDS_IN_HOUR;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->monthValues[DATA_INDEX_soilTemp2],
                     14,
                     CHART_MIN_TEMP(img->mgrWork->isMetricUnits),
                     CHART_MAX_TEMP(img->mgrWork->isMetricUnits),
                     1.0,
                     labels,
                     MONTHLY_NUM_VALUES,
                     MONTHLY_NUM_VALUES,
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     img->mgrWork->isDualUnits));
}

static int generateSoilTemp2Year (HTML_IMG *img)
{
    int         i;
    char        lbls[YEARLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->yearStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_DAY))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    ntime += 4*WV_SECONDS_IN_HOUR;
    for (i = 0; i < YEARLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%s %d", monthLabels[loctime.tm_mon], loctime.tm_mday);
        labels[i] = lbls[i];
        ntime += WV_SECONDS_IN_DAY;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->yearValues[DATA_INDEX_soilTemp2],
                     4,
                     CHART_MIN_TEMP(img->mgrWork->isMetricUnits),
                     CHART_MAX_TEMP(img->mgrWork->isMetricUnits),
                     1.0,
                     labels,
                     YEARLY_NUM_VALUES,
                     YEARLY_NUM_VALUES,
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     img->mgrWork->isDualUnits));
}


static int generateSoilTemp3Day (HTML_IMG *img)
{
    int         i, j;
    char        temp[256];

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_SAMPLE))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, j = img->mgrWork->dayStart; i < DAILY_NUM_VALUES(img->mgrWork) - 1; i ++, j ++)
    {
        if (j >= DAILY_NUM_VALUES(img->mgrWork) - 1)
            j = 0;

        labels[i] = sampleLabels[j];
    }
    labels[i] = sampleLabels[DAILY_NUM_VALUES(img->mgrWork) - 1];
    
    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->dayValues[DATA_INDEX_soilTemp3],
                     6,
                     CHART_MIN_TEMP(img->mgrWork->isMetricUnits),
                     CHART_MAX_TEMP(img->mgrWork->isMetricUnits),
                     1.0,
                     labels,
                     DAILY_NUM_VALUES(img->mgrWork),
                     DAILY_NUM_VALUES(img->mgrWork),
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     img->mgrWork->isDualUnits));
}

static int generateSoilTemp3Month (HTML_IMG *img)
{
    int         i;
    char        lbls[MONTHLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->monthStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0; i < MONTHLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%d", loctime.tm_mday);
        labels[i] = lbls[i];
        ntime += WV_SECONDS_IN_HOUR;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->monthValues[DATA_INDEX_soilTemp3],
                     14,
                     CHART_MIN_TEMP(img->mgrWork->isMetricUnits),
                     CHART_MAX_TEMP(img->mgrWork->isMetricUnits),
                     1.0,
                     labels,
                     MONTHLY_NUM_VALUES,
                     MONTHLY_NUM_VALUES,
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     img->mgrWork->isDualUnits));
}

static int generateSoilTemp3Year (HTML_IMG *img)
{
    int         i;
    char        lbls[YEARLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->yearStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_DAY))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    ntime += 4*WV_SECONDS_IN_HOUR;
    for (i = 0; i < YEARLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%s %d", monthLabels[loctime.tm_mon], loctime.tm_mday);
        labels[i] = lbls[i];
        ntime += WV_SECONDS_IN_DAY;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->yearValues[DATA_INDEX_soilTemp3],
                     4,
                     CHART_MIN_TEMP(img->mgrWork->isMetricUnits),
                     CHART_MAX_TEMP(img->mgrWork->isMetricUnits),
                     1.0,
                     labels,
                     YEARLY_NUM_VALUES,
                     YEARLY_NUM_VALUES,
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     img->mgrWork->isDualUnits));
}


static int generateSoilTemp4Day (HTML_IMG *img)
{
    int         i, j;
    char        temp[256];

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_SAMPLE))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, j = img->mgrWork->dayStart; i < DAILY_NUM_VALUES(img->mgrWork) - 1; i ++, j ++)
    {
        if (j >= DAILY_NUM_VALUES(img->mgrWork) - 1)
            j = 0;

        labels[i] = sampleLabels[j];
    }
    labels[i] = sampleLabels[DAILY_NUM_VALUES(img->mgrWork) - 1];
    
    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->dayValues[DATA_INDEX_soilTemp4],
                     6,
                     CHART_MIN_TEMP(img->mgrWork->isMetricUnits),
                     CHART_MAX_TEMP(img->mgrWork->isMetricUnits),
                     1.0,
                     labels,
                     DAILY_NUM_VALUES(img->mgrWork),
                     DAILY_NUM_VALUES(img->mgrWork),
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     img->mgrWork->isDualUnits));
}

static int generateSoilTemp4Month (HTML_IMG *img)
{
    int         i;
    char        lbls[MONTHLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->monthStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0; i < MONTHLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%d", loctime.tm_mday);
        labels[i] = lbls[i];
        ntime += WV_SECONDS_IN_HOUR;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->monthValues[DATA_INDEX_soilTemp4],
                     14,
                     CHART_MIN_TEMP(img->mgrWork->isMetricUnits),
                     CHART_MAX_TEMP(img->mgrWork->isMetricUnits),
                     1.0,
                     labels,
                     MONTHLY_NUM_VALUES,
                     MONTHLY_NUM_VALUES,
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     img->mgrWork->isDualUnits));
}

static int generateSoilTemp4Year (HTML_IMG *img)
{
    int         i;
    char        lbls[YEARLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->yearStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_DAY))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    ntime += 4*WV_SECONDS_IN_HOUR;
    for (i = 0; i < YEARLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%s %d", monthLabels[loctime.tm_mon], loctime.tm_mday);
        labels[i] = lbls[i];
        ntime += WV_SECONDS_IN_DAY;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->yearValues[DATA_INDEX_soilTemp4],
                     4,
                     CHART_MIN_TEMP(img->mgrWork->isMetricUnits),
                     CHART_MAX_TEMP(img->mgrWork->isMetricUnits),
                     1.0,
                     labels,
                     YEARLY_NUM_VALUES,
                     YEARLY_NUM_VALUES,
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     img->mgrWork->isDualUnits));
}


static int generateExtraHumid1Day (HTML_IMG *img)
{
    int         i, j;
    char        temp[256];

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_SAMPLE))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, j = img->mgrWork->dayStart; i < DAILY_NUM_VALUES(img->mgrWork) - 1; i ++, j ++)
    {
        if (j >= DAILY_NUM_VALUES(img->mgrWork) - 1)
            j = 0;

        labels[i] = sampleLabels[j];
    }
    labels[i] = sampleLabels[DAILY_NUM_VALUES(img->mgrWork) - 1];

    return (htmlGenPngPercentChart (temp,
                            img->mgrWork->isMetricUnits,
                            img->mgrWork->dayValues[DATA_INDEX_extraHumid1],
                            6,
                            labels,
                            DAILY_NUM_VALUES(img->mgrWork),
                            DAILY_NUM_VALUES(img->mgrWork),
                            img->title,
                            img->mgrWork->dateFormat));
}

static int generateExtraHumid1Month (HTML_IMG *img)
{
    int         i;
    char        lbls[MONTHLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->monthStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0; i < MONTHLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%d", loctime.tm_mday);
        labels[i] = lbls[i];
        ntime += WV_SECONDS_IN_HOUR;
    }

    return (htmlGenPngPercentChart (temp,
                            img->mgrWork->isMetricUnits,
                            img->mgrWork->monthValues[DATA_INDEX_extraHumid1],
                            14,
                            labels,
                            MONTHLY_NUM_VALUES,
                            MONTHLY_NUM_VALUES,
                            img->title,
                            img->mgrWork->dateFormat));
}

static int generateExtraHumid1Year (HTML_IMG *img)
{
    int         i;
    char        lbls[YEARLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->yearStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_DAY))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    ntime += 4*WV_SECONDS_IN_HOUR;
    for (i = 0; i < YEARLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%s %d", monthLabels[loctime.tm_mon], loctime.tm_mday);
        labels[i] = lbls[i];
        ntime += WV_SECONDS_IN_DAY;
    }

    return (htmlGenPngPercentChart (temp,
                            img->mgrWork->isMetricUnits,
                            img->mgrWork->yearValues[DATA_INDEX_extraHumid1],
                            4,
                            labels,
                            YEARLY_NUM_VALUES,
                            YEARLY_NUM_VALUES,
                            img->title,
                            img->mgrWork->dateFormat));
}


static int generateExtraHumid2Day (HTML_IMG *img)
{
    int         i, j;
    char        temp[256];

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_SAMPLE))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, j = img->mgrWork->dayStart; i < DAILY_NUM_VALUES(img->mgrWork) - 1; i ++, j ++)
    {
        if (j >= DAILY_NUM_VALUES(img->mgrWork) - 1)
            j = 0;

        labels[i] = sampleLabels[j];
    }
    labels[i] = sampleLabels[DAILY_NUM_VALUES(img->mgrWork) - 1];

    return (htmlGenPngPercentChart (temp,
                            img->mgrWork->isMetricUnits,
                            img->mgrWork->dayValues[DATA_INDEX_extraHumid2],
                            6,
                            labels,
                            DAILY_NUM_VALUES(img->mgrWork),
                            DAILY_NUM_VALUES(img->mgrWork),
                            img->title,
                            img->mgrWork->dateFormat));
}

static int generateExtraHumid2Month (HTML_IMG *img)
{
    int         i;
    char        lbls[MONTHLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->monthStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0; i < MONTHLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%d", loctime.tm_mday);
        labels[i] = lbls[i];
        ntime += WV_SECONDS_IN_HOUR;
    }

    return (htmlGenPngPercentChart (temp,
                            img->mgrWork->isMetricUnits,
                            img->mgrWork->monthValues[DATA_INDEX_extraHumid2],
                            14,
                            labels,
                            MONTHLY_NUM_VALUES,
                            MONTHLY_NUM_VALUES,
                            img->title,
                            img->mgrWork->dateFormat));
}

static int generateExtraHumid2Year (HTML_IMG *img)
{
    int         i;
    char        lbls[YEARLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->yearStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_DAY))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    ntime += 4*WV_SECONDS_IN_HOUR;
    for (i = 0; i < YEARLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%s %d", monthLabels[loctime.tm_mon], loctime.tm_mday);
        labels[i] = lbls[i];
        ntime += WV_SECONDS_IN_DAY;
    }

    return (htmlGenPngPercentChart (temp,
                            img->mgrWork->isMetricUnits,
                            img->mgrWork->yearValues[DATA_INDEX_extraHumid2],
                            4,
                            labels,
                            YEARLY_NUM_VALUES,
                            YEARLY_NUM_VALUES,
                            img->title,
                            img->mgrWork->dateFormat));
}


static int generateExtraTemp1Day (HTML_IMG *img)
{
    int         i, j;
    char        temp[256];

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_SAMPLE))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, j = img->mgrWork->dayStart; i < DAILY_NUM_VALUES(img->mgrWork) - 1; i ++, j ++)
    {
        if (j >= DAILY_NUM_VALUES(img->mgrWork) - 1)
            j = 0;

        labels[i] = sampleLabels[j];
    }
    labels[i] = sampleLabels[DAILY_NUM_VALUES(img->mgrWork) - 1];
    
    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->dayValues[DATA_INDEX_extraTemp1],
                     6,
                     CHART_MIN_TEMP(img->mgrWork->isMetricUnits),
                     CHART_MAX_TEMP(img->mgrWork->isMetricUnits),
                     1.0,
                     labels,
                     DAILY_NUM_VALUES(img->mgrWork),
                     DAILY_NUM_VALUES(img->mgrWork),
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     img->mgrWork->isDualUnits));
}

static int generateExtraTemp1Month (HTML_IMG *img)
{
    int         i;
    char        lbls[MONTHLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->monthStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0; i < MONTHLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%d", loctime.tm_mday);
        labels[i] = lbls[i];
        ntime += WV_SECONDS_IN_HOUR;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->monthValues[DATA_INDEX_extraTemp1],
                     14,
                     CHART_MIN_TEMP(img->mgrWork->isMetricUnits),
                     CHART_MAX_TEMP(img->mgrWork->isMetricUnits),
                     1.0,
                     labels,
                     MONTHLY_NUM_VALUES,
                     MONTHLY_NUM_VALUES,
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     img->mgrWork->isDualUnits));
}

static int generateExtraTemp1Year (HTML_IMG *img)
{
    int         i;
    char        lbls[YEARLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->yearStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_DAY))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    ntime += 4*WV_SECONDS_IN_HOUR;
    for (i = 0; i < YEARLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%s %d", monthLabels[loctime.tm_mon], loctime.tm_mday);
        labels[i] = lbls[i];
        ntime += WV_SECONDS_IN_DAY;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->yearValues[DATA_INDEX_extraTemp1],
                     4,
                     CHART_MIN_TEMP(img->mgrWork->isMetricUnits),
                     CHART_MAX_TEMP(img->mgrWork->isMetricUnits),
                     1.0,
                     labels,
                     YEARLY_NUM_VALUES,
                     YEARLY_NUM_VALUES,
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     img->mgrWork->isDualUnits));
}


static int generateExtraTemp2Day (HTML_IMG *img)
{
    int         i, j;
    char        temp[256];

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_SAMPLE))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, j = img->mgrWork->dayStart; i < DAILY_NUM_VALUES(img->mgrWork) - 1; i ++, j ++)
    {
        if (j >= DAILY_NUM_VALUES(img->mgrWork) - 1)
            j = 0;

        labels[i] = sampleLabels[j];
    }
    labels[i] = sampleLabels[DAILY_NUM_VALUES(img->mgrWork) - 1];
    
    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->dayValues[DATA_INDEX_extraTemp2],
                     6,
                     CHART_MIN_TEMP(img->mgrWork->isMetricUnits),
                     CHART_MAX_TEMP(img->mgrWork->isMetricUnits),
                     1.0,
                     labels,
                     DAILY_NUM_VALUES(img->mgrWork),
                     DAILY_NUM_VALUES(img->mgrWork),
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     img->mgrWork->isDualUnits));
}

static int generateExtraTemp2Month (HTML_IMG *img)
{
    int         i;
    char        lbls[MONTHLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->monthStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0; i < MONTHLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%d", loctime.tm_mday);
        labels[i] = lbls[i];
        ntime += WV_SECONDS_IN_HOUR;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->monthValues[DATA_INDEX_extraTemp2],
                     14,
                     CHART_MIN_TEMP(img->mgrWork->isMetricUnits),
                     CHART_MAX_TEMP(img->mgrWork->isMetricUnits),
                     1.0,
                     labels,
                     MONTHLY_NUM_VALUES,
                     MONTHLY_NUM_VALUES,
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     img->mgrWork->isDualUnits));
}

static int generateExtraTemp2Year (HTML_IMG *img)
{
    int         i;
    char        lbls[YEARLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->yearStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_DAY))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    ntime += 4*WV_SECONDS_IN_HOUR;
    for (i = 0; i < YEARLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%s %d", monthLabels[loctime.tm_mon], loctime.tm_mday);
        labels[i] = lbls[i];
        ntime += WV_SECONDS_IN_DAY;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->yearValues[DATA_INDEX_extraTemp2],
                     4,
                     CHART_MIN_TEMP(img->mgrWork->isMetricUnits),
                     CHART_MAX_TEMP(img->mgrWork->isMetricUnits),
                     1.0,
                     labels,
                     YEARLY_NUM_VALUES,
                     YEARLY_NUM_VALUES,
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     img->mgrWork->isDualUnits));
}


static int generateExtraTemp3Day (HTML_IMG *img)
{
    int         i, j;
    char        temp[256];

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_SAMPLE))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, j = img->mgrWork->dayStart; i < DAILY_NUM_VALUES(img->mgrWork) - 1; i ++, j ++)
    {
        if (j >= DAILY_NUM_VALUES(img->mgrWork) - 1)
            j = 0;

        labels[i] = sampleLabels[j];
    }
    labels[i] = sampleLabels[DAILY_NUM_VALUES(img->mgrWork) - 1];
    
    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->dayValues[DATA_INDEX_extraTemp3],
                     6,
                     CHART_MIN_TEMP(img->mgrWork->isMetricUnits),
                     CHART_MAX_TEMP(img->mgrWork->isMetricUnits),
                     1.0,
                     labels,
                     DAILY_NUM_VALUES(img->mgrWork),
                     DAILY_NUM_VALUES(img->mgrWork),
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     img->mgrWork->isDualUnits));
}

static int generateExtraTemp3Month (HTML_IMG *img)
{
    int         i;
    char        lbls[MONTHLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->monthStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0; i < MONTHLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%d", loctime.tm_mday);
        labels[i] = lbls[i];
        ntime += WV_SECONDS_IN_HOUR;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->monthValues[DATA_INDEX_extraTemp3],
                     14,
                     CHART_MIN_TEMP(img->mgrWork->isMetricUnits),
                     CHART_MAX_TEMP(img->mgrWork->isMetricUnits),
                     1.0,
                     labels,
                     MONTHLY_NUM_VALUES,
                     MONTHLY_NUM_VALUES,
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     img->mgrWork->isDualUnits));
}

static int generateExtraTemp3Year (HTML_IMG *img)
{
    int         i;
    char        lbls[YEARLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->yearStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_DAY))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    ntime += 4*WV_SECONDS_IN_HOUR;
    for (i = 0; i < YEARLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%s %d", monthLabels[loctime.tm_mon], loctime.tm_mday);
        labels[i] = lbls[i];
        ntime += WV_SECONDS_IN_DAY;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->yearValues[DATA_INDEX_extraTemp3],
                     4,
                     CHART_MIN_TEMP(img->mgrWork->isMetricUnits),
                     CHART_MAX_TEMP(img->mgrWork->isMetricUnits),
                     1.0,
                     labels,
                     YEARLY_NUM_VALUES,
                     YEARLY_NUM_VALUES,
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     img->mgrWork->isDualUnits));
}


static int generateSoilMoisture1Day (HTML_IMG *img)
{
    int         i, j;
    char        temp[256];

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_SAMPLE))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, j = img->mgrWork->dayStart; i < DAILY_NUM_VALUES(img->mgrWork) - 1; i ++, j ++)
    {
        if (j >= DAILY_NUM_VALUES(img->mgrWork) - 1)
            j = 0;

        labels[i] = sampleLabels[j];
    }
    labels[i] = sampleLabels[DAILY_NUM_VALUES(img->mgrWork) - 1];
    
    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->dayValues[DATA_INDEX_soilMoist1],
                     6,
                     0,
                     5,
                     1.0,
                     labels,
                     DAILY_NUM_VALUES(img->mgrWork),
                     DAILY_NUM_VALUES(img->mgrWork),
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     0));
}

static int generateSoilMoisture1Month (HTML_IMG *img)
{
    int         i;
    char        lbls[MONTHLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->monthStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0; i < MONTHLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%d", loctime.tm_mday);
        labels[i] = lbls[i];
        ntime += WV_SECONDS_IN_HOUR;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->monthValues[DATA_INDEX_soilMoist1],
                     14,
                     0,
                     5,
                     1.0,
                     labels,
                     MONTHLY_NUM_VALUES,
                     MONTHLY_NUM_VALUES,
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     0));
}

static int generateSoilMoisture1Year (HTML_IMG *img)
{
    int         i;
    char        lbls[YEARLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->yearStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_DAY))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    ntime += 4*WV_SECONDS_IN_HOUR;
    for (i = 0; i < YEARLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%s %d", monthLabels[loctime.tm_mon], loctime.tm_mday);
        labels[i] = lbls[i];
        ntime += WV_SECONDS_IN_DAY;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->yearValues[DATA_INDEX_soilMoist1],
                     4,
                     0,
                     5,
                     1.0,
                     labels,
                     YEARLY_NUM_VALUES,
                     YEARLY_NUM_VALUES,
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     0));
}


static int generateSoilMoisture2Day (HTML_IMG *img)
{
    int         i, j;
    char        temp[256];

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_SAMPLE))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, j = img->mgrWork->dayStart; i < DAILY_NUM_VALUES(img->mgrWork) - 1; i ++, j ++)
    {
        if (j >= DAILY_NUM_VALUES(img->mgrWork) - 1)
            j = 0;

        labels[i] = sampleLabels[j];
    }
    labels[i] = sampleLabels[DAILY_NUM_VALUES(img->mgrWork) - 1];
    
    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->dayValues[DATA_INDEX_soilMoist2],
                     6,
                     0,
                     5,
                     1.0,
                     labels,
                     DAILY_NUM_VALUES(img->mgrWork),
                     DAILY_NUM_VALUES(img->mgrWork),
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     0));
}

static int generateSoilMoisture2Month (HTML_IMG *img)
{
    int         i;
    char        lbls[MONTHLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->monthStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0; i < MONTHLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%d", loctime.tm_mday);
        labels[i] = lbls[i];
        ntime += WV_SECONDS_IN_HOUR;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->monthValues[DATA_INDEX_soilMoist2],
                     14,
                     0,
                     5,
                     1.0,
                     labels,
                     MONTHLY_NUM_VALUES,
                     MONTHLY_NUM_VALUES,
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     0));
}

static int generateSoilMoisture2Year (HTML_IMG *img)
{
    int         i;
    char        lbls[YEARLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->yearStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_DAY))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    ntime += 4*WV_SECONDS_IN_HOUR;
    for (i = 0; i < YEARLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%s %d", monthLabels[loctime.tm_mon], loctime.tm_mday);
        labels[i] = lbls[i];
        ntime += WV_SECONDS_IN_DAY;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->yearValues[DATA_INDEX_soilMoist2],
                     4,
                     0,
                     5,
                     1.0,
                     labels,
                     YEARLY_NUM_VALUES,
                     YEARLY_NUM_VALUES,
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     0));
}


static int generateSoilMoisture3Day (HTML_IMG *img)
{
    int         i, j;
    char        temp[256];

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_SAMPLE))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, j = img->mgrWork->dayStart; i < DAILY_NUM_VALUES(img->mgrWork) - 1; i ++, j ++)
    {
        if (j >= DAILY_NUM_VALUES(img->mgrWork) - 1)
            j = 0;

        labels[i] = sampleLabels[j];
    }
    labels[i] = sampleLabels[DAILY_NUM_VALUES(img->mgrWork) - 1];
    
    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->dayValues[DATA_INDEX_soilMoist3],
                     6,
                     0,
                     5,
                     1.0,
                     labels,
                     DAILY_NUM_VALUES(img->mgrWork),
                     DAILY_NUM_VALUES(img->mgrWork),
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     0));
}

static int generateSoilMoisture3Month (HTML_IMG *img)
{
    int         i;
    char        lbls[MONTHLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->monthStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0; i < MONTHLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%d", loctime.tm_mday);
        labels[i] = lbls[i];
        ntime += WV_SECONDS_IN_HOUR;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->monthValues[DATA_INDEX_soilMoist3],
                     14,
                     0,
                     5,
                     1.0,
                     labels,
                     MONTHLY_NUM_VALUES,
                     MONTHLY_NUM_VALUES,
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     0));
}

static int generateSoilMoisture3Year (HTML_IMG *img)
{
    int         i;
    char        lbls[YEARLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->yearStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_DAY))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    ntime += 4*WV_SECONDS_IN_HOUR;
    for (i = 0; i < YEARLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%s %d", monthLabels[loctime.tm_mon], loctime.tm_mday);
        labels[i] = lbls[i];
        ntime += WV_SECONDS_IN_DAY;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->yearValues[DATA_INDEX_soilMoist3],
                     4,
                     0,
                     5,
                     1.0,
                     labels,
                     YEARLY_NUM_VALUES,
                     YEARLY_NUM_VALUES,
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     0));
}


static int generateSoilMoisture4Day (HTML_IMG *img)
{
    int         i, j;
    char        temp[256];

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_SAMPLE))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, j = img->mgrWork->dayStart; i < DAILY_NUM_VALUES(img->mgrWork) - 1; i ++, j ++)
    {
        if (j >= DAILY_NUM_VALUES(img->mgrWork) - 1)
            j = 0;

        labels[i] = sampleLabels[j];
    }
    labels[i] = sampleLabels[DAILY_NUM_VALUES(img->mgrWork) - 1];
    
    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->dayValues[DATA_INDEX_soilMoist4],
                     6,
                     0,
                     5,
                     1.0,
                     labels,
                     DAILY_NUM_VALUES(img->mgrWork),
                     DAILY_NUM_VALUES(img->mgrWork),
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     0));
}

static int generateSoilMoisture4Month (HTML_IMG *img)
{
    int         i;
    char        lbls[MONTHLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->monthStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0; i < MONTHLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%d", loctime.tm_mday);
        labels[i] = lbls[i];
        ntime += WV_SECONDS_IN_HOUR;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->monthValues[DATA_INDEX_soilMoist4],
                     14,
                     0,
                     5,
                     1.0,
                     labels,
                     MONTHLY_NUM_VALUES,
                     MONTHLY_NUM_VALUES,
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     0));
}

static int generateSoilMoisture4Year (HTML_IMG *img)
{
    int         i;
    char        lbls[YEARLY_NUM_VALUES][8];
    char        temp[256];
    time_t      ntime = img->mgrWork->yearStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_DAY))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    ntime += 4*WV_SECONDS_IN_HOUR;
    for (i = 0; i < YEARLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        sprintf (lbls[i], "%s %d", monthLabels[loctime.tm_mon], loctime.tm_mday);
        labels[i] = lbls[i];
        ntime += WV_SECONDS_IN_DAY;
    }

    return (htmlGenPngChart (temp,
                     img->mgrWork->isMetricUnits,
                     img->mgrWork->yearValues[DATA_INDEX_soilMoist4],
                     4,
                     0,
                     5,
                     1.0,
                     labels,
                     YEARLY_NUM_VALUES,
                     YEARLY_NUM_VALUES,
                     img->title,
                     img->units,
                     img->decimalPlaces,
                     img->mgrWork->dateFormat,
                     0));
}

//////////////////////////////////////////////////////////////////////////////
//////////////  E X T E N D E D   D A T A   E N D      ///////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////  D I A L   I M A G E S   S T A R T      ///////////////////////
//////////////////////////////////////////////////////////////////////////////

static int generateDialTemp (HTML_IMG *img)
{
    char        temp[256];
    float       feelslike;

    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    if (img->mgrWork->loopStore.windchill < img->mgrWork->loopStore.outTemp)
    {
        feelslike = img->mgrWork->loopStore.windchill;
    }
    else if (img->mgrWork->loopStore.heatindex != img->mgrWork->loopStore.outTemp)
    {
        feelslike = img->mgrWork->loopStore.heatindex;
    }
    else
    {
        feelslike = img->mgrWork->loopStore.outTemp;
    }
    
    return (htmlGenPngDialTemperature (temp,
                                       img->mgrWork->isMetricUnits,
                                       img->mgrWork->loopStore.outTemp,
                                       sensorGetDailyLow(img->mgrWork->hilowStore.sensor, SENSOR_OUTTEMP),
                                       sensorGetDailyHigh(img->mgrWork->hilowStore.sensor, SENSOR_OUTTEMP),
                                       feelslike,
                                       img->title,
                                       img->units));
}

static int generateDialHumidity (HTML_IMG *img)
{
    char        temp[256];

    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    return (htmlGenPngDialHumidity (temp,
                                    (float)img->mgrWork->loopStore.outHumidity,
                                    sensorGetDailyLow(img->mgrWork->hilowStore.sensor, SENSOR_OUTHUMID),
                                    sensorGetDailyHigh(img->mgrWork->hilowStore.sensor, SENSOR_OUTHUMID),
                                    img->title));
}


static int generateDialNetRainDay (HTML_IMG *img)
{
    char        temp[256];

    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    return (htmlGenPngDialNetRain (temp,
                                   img->mgrWork->isExtendedData,
                                   img->mgrWork->isMetricUnits,
                                   img->mgrWork->loopStore.dayRain,
                                   img->mgrWork->loopStore.dayET,
                                   img->title,
                                   img->units));
}


static int generateDialNetRainMonth (HTML_IMG *img)
{
    char        temp[256];

    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    return (htmlGenPngDialNetRain (temp,
                                   img->mgrWork->isExtendedData,
                                   img->mgrWork->isMetricUnits,
                                   img->mgrWork->loopStore.monthRain,
                                   img->mgrWork->loopStore.monthET,
                                   img->title,
                                   img->units));
}


static int generateDialNetRainYear (HTML_IMG *img)
{
    char        temp[256];

    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    return (htmlGenPngDialNetRain (temp,
                                   img->mgrWork->isExtendedData,
                                   img->mgrWork->isMetricUnits,
                                   img->mgrWork->loopStore.yearRain,
                                   img->mgrWork->loopStore.yearET,
                                   img->title,
                                   img->units));
}



//////////////////////////////////////////////////////////////////////////////
//////////////  D I A L   I M A G E S   S T O P        ///////////////////////
//////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////
///////////    W I N D R O S E   I M A G E S   S T A R T        //////////////
//////////////////////////////////////////////////////////////////////////////

#if 1
static int generateWindroseDay (HTML_IMG *img)
{
    char        temp[256];
    int         retVal;

    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    retVal = htmlGenPngDialWindRose (temp,
                                     img->mgrWork->dayValues[DATA_INDEX_windDir],
                                     DAILY_NUM_VALUES(img->mgrWork),
                                     img->title,
                                     WR_SAMPLE_WIDTH_DAY);

    return retVal;
}

static int generateWindroseWeek (HTML_IMG *img)
{
    char        temp[256];
    int         i, skipNo;
    int         retVal;
    time_t      ntime = img->mgrWork->weekStartTime_T;
    struct tm   loctime;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
	// nothing for us here
	return ERROR_ABORT;
    }

    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, skipNo = 0; i < WEEKLY_NUM_VALUES; i ++)
    {
	localtime_r (&ntime, &loctime);
	if (loctime.tm_hour == 0)
	{
	    break;
	}
	else
	{
	    skipNo ++;
	    ntime += WV_SECONDS_IN_HOUR;
	}
    }

    retVal = htmlGenPngDialWindRose (temp,
                                     &img->mgrWork->weekValues[DATA_INDEX_windDir][skipNo],
                                     WEEKLY_NUM_VALUES - skipNo,
                                     img->title,
                                     WR_SAMPLE_WIDTH_WEEK);

    return retVal;
}

static int generateWindroseMonth (HTML_IMG *img)
{
    int         i, skipNo;
    char        temp[256];
    time_t      ntime = img->mgrWork->monthStartTime_T;
    struct tm   loctime;
    int         retVal;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }

    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, skipNo = 0; i < MONTHLY_NUM_VALUES; i ++)
    {
        localtime_r (&ntime, &loctime);
        if (loctime.tm_hour == 0)
        {
            break;
        }
        else
        {
            skipNo ++;
            ntime += WV_SECONDS_IN_HOUR;
        }
    }

    retVal = htmlGenPngDialWindRose (temp,
                                     &img->mgrWork->monthValues[DATA_INDEX_windDir][skipNo],
                                     MONTHLY_NUM_VALUES - skipNo,
                                     img->title,
                                     WR_SAMPLE_WIDTH_MONTH);

    return retVal;
}

static int generateWindroseYear (HTML_IMG *img)
{
    char        temp[256];
    int         retVal;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_SAMPLE))
    {
        // nothing for us here
        return ERROR_ABORT;
    }

    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    retVal = htmlGenPngDialWindRose (temp,
                                     img->mgrWork->yearValues[DATA_INDEX_windDir],
                                     YEARLY_NUM_VALUES,
                                     img->title,
                                     WR_SAMPLE_WIDTH_YEAR);

    return retVal;
}
#else
static int generateWindroseDay (HTML_IMG *img)
{
    char        temp[256];
    time_t      start = time(NULL) - WV_SECONDS_IN_DAY;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_SAMPLE))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    return (htmlGenPngWindRoseSmall(temp,
                                    img->mgrWork->isMetricUnits,
                                    img->mgrWork->dayValues[DATA_INDEX_windSpeed],
                                    img->mgrWork->dayValues[DATA_INDEX_windDir],
                                    DAILY_NUM_VALUES(img->mgrWork),
                                    start,
                                    time(NULL),
                                    img->mgrWork->dateFormat));
}

static int generateWindroseWeek (HTML_IMG *img)
{
    time_t      start = time(NULL) - WV_SECONDS_IN_WEEK;
    char        temp[256];

    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    return (htmlGenPngWindRoseSmall(temp,
                                    img->mgrWork->isMetricUnits,
                                    img->mgrWork->weekValues[DATA_INDEX_windSpeed],
                                    img->mgrWork->weekValues[DATA_INDEX_windDir],
                                    WEEKLY_NUM_VALUES,
                                    start,
                                    time(NULL),
                                    img->mgrWork->dateFormat));
}

static int generateWindroseMonth (HTML_IMG *img)
{
    time_t      start = time(NULL) - WV_SECONDS_IN_MONTH;
    char        temp[256];

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    return (htmlGenPngWindRoseSmall(temp,
                                    img->mgrWork->isMetricUnits,
                                    img->mgrWork->monthValues[DATA_INDEX_windSpeed],
                                    img->mgrWork->monthValues[DATA_INDEX_windDir],
                                    MONTHLY_NUM_VALUES,
                                    start,
                                    time(NULL),
                                    img->mgrWork->dateFormat));
}

static int generateWindroseYear (HTML_IMG *img)
{
    time_t      start = time(NULL) - WV_SECONDS_IN_YEAR;
    char        temp[256];
    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_DAY))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    return (htmlGenPngWindRoseSmall(temp,
                                    img->mgrWork->isMetricUnits,
                                    img->mgrWork->yearValues[DATA_INDEX_windSpeed],
                                    img->mgrWork->yearValues[DATA_INDEX_windDir],
                                    YEARLY_NUM_VALUES,
                                    start,
                                    time(NULL),
                                    img->mgrWork->dateFormat));
}

static int generateWindroseBigDay (HTML_IMG *img)
{
    char        temp[256];
    time_t      start = time(NULL) - WV_SECONDS_IN_DAY;

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_SAMPLE))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    return (htmlGenPngWindRoseBig(temp,
                                  img->mgrWork->isMetricUnits,
                                  img->mgrWork->dayValues[DATA_INDEX_windSpeed],
                                  img->mgrWork->dayValues[DATA_INDEX_windDir],
                                  DAILY_NUM_VALUES(img->mgrWork),
                                  start,
                                  time(NULL),
                                  img->mgrWork->dateFormat));
}

static int generateWindroseBigWeek (HTML_IMG *img)
{
    time_t      start = time(NULL) - WV_SECONDS_IN_WEEK;
    char        temp[256];

    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    return (htmlGenPngWindRoseBig(temp,
                                  img->mgrWork->isMetricUnits,
                                  img->mgrWork->weekValues[DATA_INDEX_windSpeed],
                                  img->mgrWork->weekValues[DATA_INDEX_windDir],
                                  WEEKLY_NUM_VALUES,
                                  start,
                                  time(NULL),
                                  img->mgrWork->dateFormat));
}

static int generateWindroseBigMonth (HTML_IMG *img)
{
    time_t      start = time(NULL) - WV_SECONDS_IN_MONTH;
    char        temp[256];

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_HOUR))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    return (htmlGenPngWindRoseBig(temp,
                                  img->mgrWork->isMetricUnits,
                                  img->mgrWork->monthValues[DATA_INDEX_windSpeed],
                                  img->mgrWork->monthValues[DATA_INDEX_windDir],
                                  MONTHLY_NUM_VALUES,
                                  start,
                                  time(NULL),
                                  img->mgrWork->dateFormat));
}

static int generateWindroseBigYear (HTML_IMG *img)
{
    time_t      start = time(NULL) - WV_SECONDS_IN_YEAR;
    char        temp[256];
    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_DAY))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    return (htmlGenPngWindRoseBig(temp,
                                  img->mgrWork->isMetricUnits,
                                  img->mgrWork->yearValues[DATA_INDEX_windSpeed],
                                  img->mgrWork->yearValues[DATA_INDEX_windDir],
                                  YEARLY_NUM_VALUES,
                                  start,
                                  time(NULL),
                                  img->mgrWork->dateFormat));
}
#endif

//////////////////////////////////////////////////////////////////////////////
////////////    W I N D R O S E   I M A G E S   S T O P        ///////////////
//////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////
//////////////  D I A G N O S T I C S   S T A R T      ///////////////////////
//////////////////////////////////////////////////////////////////////////////

static int generateRxCheck (HTML_IMG *img)
{
    int         i, j;
    char        temp[256];

    if (!(img->mgrWork->newArchiveMask & NEW_ARCHIVE_SAMPLE))
    {
        // nothing for us here
        return ERROR_ABORT;
    }
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);

    for (i = 0, j = img->mgrWork->dayStart; i < DAILY_NUM_VALUES(img->mgrWork) - 1; i ++, j ++)
    {
        if (j >= DAILY_NUM_VALUES(img->mgrWork) - 1)
            j = 0;

        labels[i] = sampleLabels[j];
    }
    labels[i] = sampleLabels[DAILY_NUM_VALUES(img->mgrWork) - 1];

    return (htmlGenPngPercentChart (temp,
                            img->mgrWork->isMetricUnits,
                            img->mgrWork->dayValues[DATA_INDEX_rxCheckPercent],
                            6,
                            labels,
                            DAILY_NUM_VALUES(img->mgrWork),
                            DAILY_NUM_VALUES(img->mgrWork),
                            img->title,
                            img->mgrWork->dateFormat));
}

//////////////////////////////////////////////////////////////////////////////
//////////////  D I A G N O S T I C S   S T O P        ///////////////////////
//////////////////////////////////////////////////////////////////////////////

//  ... This is the generator jump table:
//  ... Indexes from images.conf are used to index into this table.
//  ... Thus, order is VERY important!
int (*images_generators[]) (HTML_IMG *img) =
    {
        generateTemp,                           // 0
        generateHumidity,
        generateDewpoint,
        generateWindChill,
        generateHeatIndex,
        generateBarometer,
        generateDayRain,
        generateStormRain,
        generateRainRate,
        generateMonthRain,
        generateYearRain,                       // 10
        generateWind,
        generateTempDay,
        generateTempMonth,
        generateTempYear,
        generateHumidDay,
        generateHumidMonth,
        generateHumidYear,
        generateDewDay,
        generateDewMonth,
        generateDewYear,                        // 20
        generateWSpeedDay,
        generateWSpeedMonth,
        generateWSpeedYear,
        generateWDirDay,
        generateWDirMonth,
        generateWDirYear,
        generateHiWSpeedDay,
        generateHiWSpeedMonth,
        generateHiWSpeedYear,
        generateBaromDay,                       // 30
        generateBaromMonth,
        generateBaromYear,
        generateRainDay,
        generateRainMonth,
        generateRainYear,
        generateWChillDay,
        generateWChillMonth,
        generateWChillYear,
        generateHIndexDay,
        generateHIndexMonth,                    // 40
        generateHIndexYear,
        generateCompositeTempDay,
        generateCompositeHIndexWChillDay,
        generateCompositeTempMonth,
        generateCompositeHIndexWChillMonth,
        generateRadiationDay,
        generateRadiationMonth,
        generateRadiationYear,
        generateUVDay,
        generateUVMonth,                        // 50
        generateUVYear,
        generateETDay,
        generateETMonth,
        generateETYear,
        generateLeafTemp1Day,
        generateLeafTemp1Month,
        generateLeafTemp1Year,
        generateLeafTemp2Day,
        generateLeafTemp2Month,
        generateLeafTemp2Year,                  // 60
        generateLeafWetness1Day,
        generateLeafWetness1Month,
        generateLeafWetness1Year,
        generateLeafWetness2Day,
        generateLeafWetness2Month,
        generateLeafWetness2Year,
        generateSoilTemp1Day,
        generateSoilTemp1Month,
        generateSoilTemp1Year,
        generateSoilTemp2Day,                   // 70
        generateSoilTemp2Month,
        generateSoilTemp2Year,
        generateSoilTemp3Day,
        generateSoilTemp3Month,
        generateSoilTemp3Year,
        generateSoilTemp4Day,
        generateSoilTemp4Month,
        generateSoilTemp4Year,
        generateExtraHumid1Day,
        generateExtraHumid1Month,               // 80
        generateExtraHumid1Year,
        generateExtraHumid2Day,
        generateExtraHumid2Month,
        generateExtraHumid2Year,
        generateExtraTemp1Day,
        generateExtraTemp1Month,
        generateExtraTemp1Year,
        generateExtraTemp2Day,
        generateExtraTemp2Month,
        generateExtraTemp2Year,                 // 90
        generateExtraTemp3Day,
        generateExtraTemp3Month,
        generateExtraTemp3Year,
        generateSoilMoisture1Day,
        generateSoilMoisture1Month,
        generateSoilMoisture1Year,
        generateSoilMoisture2Day,
        generateSoilMoisture2Month,
        generateSoilMoisture2Year,
        generateSoilMoisture3Day,               // 100
        generateSoilMoisture3Month,
        generateSoilMoisture3Year,
        generateSoilMoisture4Day,
        generateSoilMoisture4Month,
        generateSoilMoisture4Year,
        generateUV,
        generateRadiation,
        generateET,
        generateDialTemp,
        generateDialHumidity,                   // 110
        generateRxCheck,
        generateDialNetRainDay,
        generateDialNetRainMonth,
        generateDialNetRainYear,
        generateCompositeInTempHumidDay,
        generateCompositeInTempHumidMonth,
        generateTempWeek,
        generateHumidWeek,
        generateDewWeek,
        generateWSpeedWeek,                     // 120
        generateWDirWeek,
        generateHiWSpeedWeek,
        generateBaromWeek,
        generateRainWeek,
        generateWChillWeek,
        generateHIndexWeek,
        generateCompositeTempWeek,
        generateCompositeHIndexWChillWeek,
        generateCompositeInTempHumidWeek,
        generateRadiationWeek,                  // 130
        generateUVWeek,
        generateETWeek,
        generateWindroseDay,
        generateWindroseWeek,
        generateWindroseMonth,
        generateWindroseYear,
        generateCompositeWindDay,
        generateCompositeWindWeek,
        generateCompositeWindMonth,
        generateCompositeWindYear,              // 140
        NULL
    };


