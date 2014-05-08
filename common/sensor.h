#ifndef INC_sensorh
#define INC_sensorh
/*---------------------------------------------------------------------------
 
  FILENAME:
        sensor.h
 
  PURPOSE:
        Define the sensor API.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        01/02/2006      M.S. Teel       0               Original
 
  NOTES:
        
 
  LICENSE:
        Copyright (c) 2006, Mark S. Teel (mark@teel.ws)
  
        This source code is released for free distribution under the terms 
        of the GNU General Public License.
  
----------------------------------------------------------------------------*/

//  ... includes
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include <sys/stat.h>

#include <radlist.h>
#include <radbuffers.h>

#include <sysdefs.h>
#include <datadefs.h>
#include <windAverage.h>


//  ... macro definitions

#define SENSOR_SAVE_FILENAME            "sensorstore.bin"

// Incrementing this will cause sensorstore.bin to be recreated upon first run:
// Last change version: 5.0.0
#define SENSOR_SAVE_MAGIC_NUMBER        0x5D372001


//  ... typedefs

// sample accumulator with time decay
typedef struct
{
    NODE                node;
    float               value;
    time_t              sampleTime;
} WV_ACCUM_SAMPLE;

typedef struct
{
    RADLIST             samples;
    int                 secondsInAccumulator;
    float               sum;
} WV_ACCUM, *WV_ACCUM_ID;


//  ... API prototypes

// Accumulator with time decay
extern WV_ACCUM_ID sensorAccumInit (int minutesInAccumulator);
extern void        sensorAccumExit (WV_ACCUM_ID id);
extern void        sensorAccumAddSample (WV_ACCUM_ID id, time_t timeStamp, float value);
extern float       sensorAccumGetTotal (WV_ACCUM_ID id);
extern float       sensorAccumGetAverage (WV_ACCUM_ID id);


// -- initialize a sensor --
extern void sensorInit (WV_SENSOR *sensor);


// these insert at the current time
extern void sensorUpdate (WV_SENSOR *sensor, float value);
extern void sensorUpdateWhen (WV_SENSOR *sensor, float value, float whenVal);

// Initialize sensor with given values:
extern void sensorSetValues
(
    WV_SENSOR           *sensor,
    float               low,
    time_t              time_low,
    float               high,
    time_t              time_high,
    float               when_high,
    float               cumulative,
    int                 samples
);

// these just modify high/low/cumulative values
extern void sensorUpdateLowValue (WV_SENSOR *sensor, float value);
extern void sensorUpdateHighValue (WV_SENSOR *sensor, float value);
extern void sensorUpdateWhenHighValue (WV_SENSOR *sensor, float value, float whenVal);
extern void sensorUpdateCumulative (WV_SENSOR *sensor, float value);
extern void sensorAddCumulative (WV_SENSOR *sensor, float value);

// add a sensor interval to a specific sensor
extern void sensorAddSample (WV_SENSOR *sensor, WV_SENSOR *sample, int doDebug);

// -- operations on sets of sensors --
// propogate a new data sample to the current interval
// assumes input of type "set[SENSOR_MAX]"
extern void sensorPropogateSample (WV_SENSOR *set, WV_SENSOR *sample);

// clear sensors for a SENSOR_MAX array
// assumes input of type "set[SENSOR_MAX]"
extern void sensorClearSet (WV_SENSOR *set);


// -- retrieve info from a sensor --
extern float sensorGetLow (WV_SENSOR *sensor);
extern char *sensorGetLowTime (WV_SENSOR *sensor, char *store);
extern char *sensorGetLowDate (WV_SENSOR *sensor, char *store, char *dateFormat);
extern float sensorGetHigh (WV_SENSOR *sensor);
extern float sensorGetWhenHigh (WV_SENSOR *sensor);
extern char *sensorGetHighTime (WV_SENSOR *sensor, char *store);
extern char *sensorGetHighDate (WV_SENSOR *sensor, char *store, char *dateFormat);
extern float sensorGetAvg (WV_SENSOR *sensor);
extern float sensorGetCumulative (WV_SENSOR *sensor);
extern int sensorGetSamples (WV_SENSOR *sensor);


// -- retrieve info from a group of sensors
// get a daily low value based on the RTF_INTERVAL and RTF_DAY time frames - 
extern float sensorGetDailyLow (WV_SENSOR set[STF_MAX][SENSOR_MAX], SENSOR_TYPES type);
extern char *sensorGetDailyLowTime (WV_SENSOR set[STF_MAX][SENSOR_MAX], SENSOR_TYPES type, char *store);

// get a daily high value based on the RTF_INTERVAL and RTF_DAY time frames - 
extern float sensorGetDailyHigh (WV_SENSOR set[STF_MAX][SENSOR_MAX], SENSOR_TYPES type);
extern char *sensorGetDailyHighTime (WV_SENSOR set[STF_MAX][SENSOR_MAX], SENSOR_TYPES type, char *store);

// get a daily "when" high value based on the RTF_INTERVAL and RTF_DAY time frames - 
extern float sensorGetDailyWhenHigh (WV_SENSOR set[STF_MAX][SENSOR_MAX], SENSOR_TYPES type);

// get a daily cumulative value based on the RTF_INTERVAL and RTF_DAY time frames - 
extern float sensorGetDailyCumulative (WV_SENSOR set[STF_MAX][SENSOR_MAX], SENSOR_TYPES type);

// -- special purpose utilities --
extern float sensorGetWindRun (SENSOR_TIMEFRAMES frame, WV_SENSOR *sensor);
extern char* sensorGetString (WV_SENSOR *sensor);

#endif

