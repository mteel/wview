#ifndef INC_htmlmgrh
#define INC_htmlmgrh
/*---------------------------------------------------------------------------
 
  FILENAME:
        htmlMgr.h
 
  PURPOSE:
        Provide the wview HTML generator definitions.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        08/30/03        M.S. Teel       0               Original
 
  NOTES:
        
 
  LICENSE:
        Copyright (c) 2004, Mark S. Teel (mark@teel.ws)
  
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

/*  ... Library include files
*/
#include <sysdefs.h>
#include <radsemaphores.h>
#include <radbuffers.h>
#include <radlist.h>

/*  ... Local include files
*/
#include <sensor.h>
#include <dbsqlite.h>
#include <datadefs.h>
#include <services.h>
#include <status.h>



/*  ... API definitions
*/

// Note: these macros are 1 bigger than the actual datapoints per period
// to allow for the graph data points required

// Samples per hour:
#define HOURLY_NUM_VALUES(x)    (60/x->archiveInterval)	

// Samples per day: (samples/hour * 24 hours) + 1
#define DAILY_NUM_VALUES(x)     (((60/x->archiveInterval) * 24) + 1)	
#define MAX_DAILY_NUM_VALUES    ((60 * 24) + 1)	

// Samples per week: (24 hours * 7 days) + 1
#define WEEKLY_NUM_VALUES       ((24 * 7) + 1)

// Samples per month: (24 hours * 28 days) + 1
#define MONTHLY_NUM_VALUES      ((24 * 28) + 1)

// Samples per year: (365 days) + 1
#define YEARLY_NUM_VALUES       (365 + 1)

// Barometer Trend Max Values
#define BP_MAX_VALUES           (4 * 60)

#define FORECAST_ICON_FN_MAX    128


/*  !!!!!!!!!!!!!!!!!!  HIDDEN, NOT FOR API USE  !!!!!!!!!!!!!!!!!!
*/

#define HTML_MAX_LINE_LENGTH        1536
#define HTML_MAX_FCAST_RULE         196

typedef enum
{
    NEW_ARCHIVE_SAMPLE          = 0x1,
    NEW_ARCHIVE_HOUR            = 0x2,
    NEW_ARCHIVE_DAY             = 0x4,
    NEW_ARCHIVE_ALL             = 0x7
} NEW_ARCHIVE_BITS;

//  ... define the work area for this beast
typedef struct
{
    char                *imagePath;
    char                *htmlPath;
    char                stationName[64];
    char                stationType[128];
    char                stationCity[64];
    char                stationState[64];
    int                 stationElevation;
    int                 stationLatitude;
    int                 stationLongitude;
    char                mphaseIncrease[32];
    char                mphaseDecrease[32];
    char                mphaseFull[32];
    char                radarURL[256];
    char                forecastURL[256];
    SENSOR_STORE        hilowStore;
    LOOP_PKT            loopStore;
    int                 isMetricUnits;
    int                 archiveInterval;
    int                 isExtendedData;
    int16_t             sunrise;
    int16_t             sunset;
    int16_t             midday;
    int16_t             dayLength;
    int16_t             civilrise;
    int16_t             civilset;
    int16_t             astrorise;
    int16_t             astroset;
    int16_t             moonrise;
    int16_t             moonset;
    RADLIST             imgList;
    RADLIST             templateList;
    float               dayValues[DATA_INDEX_MAX][MAX_DAILY_NUM_VALUES];
    int                 dayStart;
    float               weekValues[DATA_INDEX_MAX][WEEKLY_NUM_VALUES];
    time_t              weekStartTime_T;
    float               monthValues[DATA_INDEX_MAX][MONTHLY_NUM_VALUES];
    time_t              monthStartTime_T;
    float               yearValues[DATA_INDEX_MAX][YEARLY_NUM_VALUES];
    time_t              yearStartTime_T;
    int                 newArchiveMask;
    float               baromTrendValues[BP_MAX_VALUES];    // 4 hours
    int                 baromTrendNumValues;
    int                 baromTrendIndex;
    int                 baromTrendIndexMax;
    int                 baromTrendIndicator;                // -1,0,1
    char                *ForecastRuleText[HTML_MAX_FCAST_RULE+1];
    char                *ForecastIconFile[VP_FCAST_ICON_MAX+1];
    char                dateFormat[256];
    int                 isDualUnits;
    int                 imagesGenerated;
    int                 templatesGenerated;
} HTML_MGR, *HTML_MGR_ID;


//  ... define the image data "node"
typedef struct htmlImageTag
{
    NODE                node;
    HTML_MGR_ID         mgrWork;
    char                fname[64];
    char                title[32];
    char                units[32];
    int                 decimalPlaces;
    int                 (*generator) (struct htmlImageTag *);
} HTML_IMG;


//  ... define the html template "node"
typedef struct htmlTemplateTag
{
    NODE                node;
    char                fname[128];
} HTML_TMPL;


/*  !!!!!!!!!!!!!!!!!!!!  END HIDDEN SECTION  !!!!!!!!!!!!!!!!!!!!!
*/


/* ... API function prototypes
*/

extern HTML_MGR_ID htmlmgrInit
(
    char            *installPath,
    int             isMetricUnits,
    char            *imagePath,
    char            *htmlPath,
    int             arcInterval,
    int             isExtendedData,
    char            *name,
    char            *city,
    char            *state,
    int16_t         elevation,
    int16_t         latitude,
    int16_t         longitude,
    char            *mphaseIncrease,
    char            *mphaseDecrease,
    char            *mphaseFull,
    char            *radarURL,
    char            *forecastURL,
    char            *dateFormat,
    int             isDualUnits
);

extern int htmlmgrReReadImageFiles
(
    HTML_MGR_ID     id,
    char            *installPath
);

extern void htmlmgrExit
(
    HTML_MGR_ID     id
);

extern int htmlmgrBPTrendInit
(
    HTML_MGR_ID         id,
    int                 timerIntervalMINs
);

extern int htmlmgrGenerate
(
    HTML_MGR_ID     id
);

extern int htmlmgrHistoryInit (HTML_MGR_ID id);
extern int htmlmgrAddSampleValue (HTML_MGR_ID id, HISTORY_DATA *data, int numIntervals);
extern int htmlmgrAddHourValue (HTML_MGR_ID id, HISTORY_DATA *data);
extern int htmlmgrAddDayValue (HTML_MGR_ID id, HISTORY_DATA *data);
extern void htmlmgrSetSampleLabels(HTML_MGR_ID id);


//  Define the htmlUtils API here

// Set dual units based on primary units:
extern void htmlutilsSetDualUnits (int isMetric, const char *units, char *dualUnits);


#endif

