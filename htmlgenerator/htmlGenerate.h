#ifndef INC_htmlgenerateh
#define INC_htmlgenerateh
/*---------------------------------------------------------------------------
 
  FILENAME:
        htmlGenerate.h
 
  PURPOSE:
        Provide the wview HTML generator definitions.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        08/30/03        M.S. Teel       0               Original
        02/16/2008      M.B. Clark      1               Added ability to customize color/size
                                                          of plots using graphics.conf
        05/16/2008      W. Krenn        2               isDualUnits for Chart	   
 
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
#include <radconffile.h>

/*  ... Local include files
*/
#include <dbsqlite.h>
#include <datadefs.h>
#include <services.h>

#include <htmlMgr.h>
#include <glmultichart.h>


/*  ... API definitions
*/

typedef struct
{
    int bucketTransparent;
    int bucketBgColor;
    int bucketFgColor;
    int bucketContentColor;
    int bucketHighColor;
    int bucketLowColor;
    int bucketTitleBgColor;
    int bucketTitleFgColor;
    int bucketTextColor;
    int bucketTransparentColor;
    int bucketImageWidth;
    int bucketImageHeight;
    int bucketWidth;

    int chartTransparent;
    int chartImageBgColor;
    int chartGraphBgColor;
    int chartGridColor;
    int chartFirstLineColor;
    int chartSecondLineColor;
    int chartThirdLineColor;
    int chartFourthLineColor;
    int chartTitleBgColor;
    int chartTitleFgColor;
    int chartTextColor;
    int chartWidth;
    int chartHeight;

    int barImageBgColor;
    int barGraphBgColor;
    int barBarColor;
    int barTitleFgColor;
    int barTitleBgColor;
    int barGridColor;
    int barTextColor;
    int barWidth;
    int barHeight;

    int dialTransparent;
    int dialBgColor;
    int dialImageBgColor;
    int dialTickColor;
    int dialCenterColor;
    int dialCenterTextColor;
    int dialCenterHighColor;
    int dialPointerColor;
    int dialPointerOutlineColor;
    int dialTextColor;
    int dialHighColor;
    int dialLowColor;
    int dialAppColor;
    int dialTransparentColor;
    int dialImageWidth;
    int dialDiameter;
    int dialCtrDiameter;
    
    int multichartFirstLineColor;
    int multichartSecondLineColor;
    int multichartThirdLineColor;
    int multichartFourthLineColor;

} PLOT_PREFS;

/*  !!!!!!!!!!!!!!!!!!  HIDDEN, NOT FOR API USE  !!!!!!!!!!!!!!!!!!
*/


// utility macros for the VP packed date format
#define VP_YEAR(x)                  ((((x) >> 9) & 0x7F) + 2000)
#define VP_MONTH(x)                 (((x) >> 5) & 0x0F)
#define VP_DAY(x)                   ((x) & 0x1F)


struct xyCoords
{
    float       x;
    float       y;
};


/*  !!!!!!!!!!!!!!!!!!!!  END HIDDEN SECTION  !!!!!!!!!!!!!!!!!!!!!
*/


/* ... API function prototypes
*/
int htmlGenerateInit (void);
int htmlgenGetChartDefaultLine (void);
int htmlgenGetChartDefaultSecondLine (void);
int htmlgenGetChartDefaultThirdLine (void);

int htmlgenGetMultiChartDefaultFirstLine (void);
int htmlgenGetMultiChartDefaultSecondLine (void);
int htmlgenGetMultiChartDefaultThirdLine (void);
int htmlgenGetMultiChartDefaultFourthLine (void);

extern int htmlgenOutputFiles (HTML_MGR_ID id, uint64_t startTime);

extern int htmlgenMesonetFile (HTML_MGR_ID id, WVIEW_MSG_ARCHIVE_NOTIFY *armsg);

extern int htmlGenPngBucket
(
    char            *fname,
    int             isMetric,
    float           value,
    float           low,
    float           high,
    char            *title,
    char            *units,
    int             decimalPlaces,
    float           lowScale,
    float           highScale,
    char            *dateFormat,
    int             isDualUnits
);

extern int htmlGenPngRainBucket
(
    char            *fname,
    int             isMetric,
    float           value,
    float           low,
    float           high,
    char            *title,
    char            *units,
    int             decimalPlaces,
    float           lowScale,
    float           highScale,
    char            *dateFormat,
    int             isDualUnits
);

extern int htmlGenPngBaromBucket
(
    char            *fname,
    float           value,
    float           low,
    float           high,
    char            *title,
    char            *units,
    int             decimalPlaces,
    char            *dateFormat,
    int             isDualUnits
);

extern int htmlGenMetricPngBaromBucket
(
    char                *fname,
    float               value,
    float               low,
    float               high,
    char                *title,
    char                *units,
    int                 decimalPlaces,
    char                *dateFormat,
    int                 isDualUnits
);

extern int htmlGenPngPercentBucket
(
    char            *fname,
    int             isMetric,
    float           value,
    float           low,
    float           high,
    char            *title,
    char            *units,
    int             decimalPlaces,
    char            *dateFormat,
    int             isDualUnits
);

#define CHART_MAX_TEMP(x)       (x ? (float)15.0 : (float)60.0)
#define CHART_MIN_TEMP(x)       (x ? (float)0.0 : (float)40.0)
extern int htmlGenPngChart
(
    char            *fname,
    int             isMetric,
    float           *values,
    int             xHashes,
    float           ymin,
    float           ymax,
    float           ystep,
    char            *labels[],
    int             numValues,
    int             totalValues,
    char            *title,
    char            *units,
    int             decimalPlaces,
    char            *dateFormat,
    int             isDualUnits    
);

extern int htmlGenPngBarChart
(
    char            *fname,
    int             isMetric,
    float           *values,
    int             xHashes,
    float           ymin,
    float           ymax,
    float           ystep,
    char            *labels[],          // size=numValues+1 for last label
    int             numValues,
    char            *title,
    char            *units,
    int             decimalPlaces,
    int             numPointsPerBar,
    int             numBars,
    int             numPointsInFirstBar,
    char            *dateFormat,
    int             isDualUnits
);

extern int htmlGenPngPercentChart
(
    char            *fname,
    int             isMetric,
    float           *values,
    int             xHashes,
    char            *labels[],
    int             numValues,
    int             totalValues,
    char            *title,
    char            *dateFormat
);

#define CHART_MIN_WSPEED(x)         0
#define CHART_MAX_WSPEED(x)         ((x) ? 60 : 40)
extern int htmlGenPngMultiChart
(
    char            *fname,
    int             isMetric,
    MC_DATASET      *datasets,
    int             numDataSets,
    int             xHashes,
    float           min,
    float           max,
    float           step,
    char            *labels[],
    int             numValues,
    int             totalValues,
    char            *units,
    int             decimalPlaces,
    char            *dateFormat,
    int             isDualUnits
);

extern int htmlGenPngWDIRChart
(
    char            *fname,
    int             isMetric,
    float           *values,
    int             xHashes,
    char            *labels[],
    int             numValues,
    int             totalValues,
    char            *title,
    char            *units,
    int             decimalPlaces,
    char            *dateFormat
);

extern int htmlGenPngDialInit (char *workingDir);

extern int htmlGenPngDialWind
(
    char            *fname,
    int             direction,               // 0 - 359 degrees
    int             highDirection,           // 0-359 degrees
    float           speed,
    float           highSpeed,
    char            *title
);

//  ... Use the following to disable the "feelsLike" depiction
#define DIAL_TEMP_FEELSLIKE_DISABLE    (float)(100000.0)
extern int htmlGenPngDialTemperature
(
    char                *fname,
    int                 isMetricUnits,
    float               temperature,         // -40 to 140 degrees, 180 total
    float               low,
    float               high,
    float               feelsLike,
    char                *title,
    char                *units
);

extern int htmlGenPngDialHumidity
(
    char                *fname,
    float               humidity,            // 0 to 100 %, 100 total
    float               low,
    float               high,
    char                *title
);

extern int htmlGenPngDialNetRain
(
    char                *fname,
    int                 isExtendedData,
    int                 isMetricUnits,
    float               rain,
    float               et,               
    char                *title,
    char                *units
);

// Note: sample width cannot be less than 10 degrees!
#define WR_SAMPLE_WIDTH_DAY             20
#define WR_SAMPLE_WIDTH_WEEK            30
#define WR_SAMPLE_WIDTH_MONTH           45
#define WR_SAMPLE_WIDTH_YEAR            45
#define WR_MAX_COUNTERS                 36
#define WR_WEDGE_SPACE                  4
extern int htmlGenPngDialWindRose
(
    char                *fname,
    float               values[],
    int                 numValues,
    char                *title,
    int                 sampleWidth
);


#endif

