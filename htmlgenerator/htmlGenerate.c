/*---------------------------------------------------------------------------
 
  FILENAME:
        htmlGenerate.c
 
  PURPOSE:
        Provide the wview html generator utilities.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        08/30/03        M.S. Teel       0               Original
        06/26/2005      S. Pacenka      1               Add metric support to
                                                        the temp dial
        02/16/2008      M.B. Clark      2               Added ability to customize
                                                        color/size of plots using
                                                        graphics.conf settings
        03/24/2008      W. Krenn        3               metric adaptations
        12/01/2009      M. Hornsby      4               Add Moon Rise and Set
 
  NOTES:
        This is by far the ugliest code in the wview source. Shortcuts are taken
        to optimize image and HTML generation.
 
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
#include "gd.h"
#include "gdfonts.h"
#include "gdfontt.h"
#include "gdfontmb.h"
#include "gdfontg.h"
#include "gdfontl.h"
#include <radsysutils.h>
#include <radtextsearch.h>

/*  ... Local include files
*/
#include <services.h>
#include <html.h>
#include <glbucket.h>
#include <glchart.h>
#include <glmultichart.h>
#include <htmlGenerate.h>


/*  ... global memory declarations
*/

/*  ... global memory referenced
*/

/*  ... static (local) memory declarations
*/
#define _DEBUG_GENERATION                   FALSE

static TEXT_SEARCH_ID   tagSearchEngine;

static PLOT_PREFS       plotPrefs;

enum ConfigIds
{
    CFG_ID_BUCKET_TRANSPARENT               = 0,
    CFG_ID_BUCKET_BG_COLOR,
    CFG_ID_BUCKET_FG_COLOR,
    CFG_ID_BUCKET_CONTENT_COLOR,
    CFG_ID_BUCKET_HIGH_COLOR,
    CFG_ID_BUCKET_LOW_COLOR,
    CFG_ID_BUCKET_TITLE_BG_COLOR,
    CFG_ID_BUCKET_TITLE_FG_COLOR,
    CFG_ID_BUCKET_TEXT_COLOR,
    CFG_ID_BUCKET_TRANSPARENT_COLOR,
    CFG_ID_BUCKET_IMAGE_WIDTH ,
    CFG_ID_BUCKET_IMAGE_HEIGHT,
    CFG_ID_BUCKET_WIDTH,

    CFG_ID_CHART_TRANSPARENT,
    CFG_ID_CHART_IMAGE_BG_COLOR,
    CFG_ID_CHART_GRAPH_BG_COLOR,
    CFG_ID_CHART_GRID_COLOR,
    CFG_ID_CHART_FIRST_LINE_COLOR,
    CFG_ID_CHART_SECOND_LINE_COLOR,
    CFG_ID_CHART_THIRD_LINE_COLOR,
    CFG_ID_CHART_FOURTH_LINE_COLOR,
    CFG_ID_CHART_TITLE_BG_COLOR,
    CFG_ID_CHART_TITLE_FG_COLOR,
    CFG_ID_CHART_TEXT_COLOR,
    CFG_ID_CHART_WIDTH,
    CFG_ID_CHART_HEIGHT,

    CFG_ID_BAR_IMAGE_BG_COLOR,
    CFG_ID_BAR_GRAPH_BG_COLOR ,
    CFG_ID_BAR_BAR_COLOR,
    CFG_ID_BAR_TITLE_BG_COLOR,
    CFG_ID_BAR_TITLE_FG_COLOR,
    CFG_ID_BAR_GRID_COLOR,
    CFG_ID_BAR_TEXT_COLOR,
    CFG_ID_BAR_WIDTH,
    CFG_ID_BAR_HEIGHT,

    CFG_ID_DIAL_TRANSPARENT,
    CFG_ID_DIAL_BG_COLOR,
    CFG_ID_DIAL_IMAGE_BG_COLOR,
    CFG_ID_DIAL_CENTER_COLOR,
    CFG_ID_DIAL_CENTER_TEXT_COLOR,
    CFG_ID_DIAL_CENTER_HIGH_COLOR,
    CFG_ID_DIAL_POINTER_COLOR,
    CFG_ID_DIAL_POINTER_OUTLINE_COLOR,
    CFG_ID_DIAL_HIGH_COLOR,
    CFG_ID_DIAL_LOW_COLOR,
    CFG_ID_DIAL_APP_COLOR,
    CFG_ID_DIAL_TEXT_COLOR,
    CFG_ID_DIAL_IMAGE_WIDTH,
    CFG_ID_DIAL_DIAMETER,
    CFG_ID_DIAL_CTR_DIAMETER,

    CFG_ID_MULTICHART_FIRST_LINE_COLOR,
    CFG_ID_MULTICHART_SECOND_LINE_COLOR,
    CFG_ID_MULTICHART_THIRD_LINE_COLOR,
    CFG_ID_MULTICHART_FOURTH_LINE_COLOR
};


static char *configIDs[] =
    {
        "BUCKET_TRANSPARENT",                   // Background transparent?
        "BUCKET_BG_COLOR",                      // Background color of image
        "BUCKET_FG_COLOR",                      // Foreground color (lines)
        "BUCKET_CONTENT_COLOR",                 // Color fill for bucket
        "BUCKET_HIGH_COLOR",                    // Color of line for high value
        "BUCKET_LOW_COLOR",                     // Color or line for low value
        "BUCKET_TITLE_BG_COLOR",                // Background color for title
        "BUCKET_TITLE_FG_COLOR",                // Foreground (text) color for title
        "BUCKET_TEXT_COLOR",                    // Color of text labels
        "BUCKET_TRANSPARENT_COLOR",             // Not implemented yet
        "BUCKET_IMAGE_WIDTH",                   // Width of entire image
        "BUCKET_IMAGE_HEIGHT",                  // Height of enitre image
        "BUCKET_WIDTH",                         // Width of bucket itself

        "CHART_TRANSPARENT",                    // Background transparent?
        "CHART_IMAGE_BG_COLOR",                 // Background color of image
        "CHART_GRAPH_BG_COLOR",                 // Background color of plot
        "CHART_GRID_COLOR",                     // Color of grid lines
        "CHART_FIRST_LINE_COLOR",               // Color of first line
        "CHART_SECOND_LINE_COLOR",              // Color of second line
        "CHART_THIRD_LINE_COLOR",               // Color of third line
        "CHART_FOURTH_LINE_COLOR",              // Color of fourth line
        "CHART_TITLE_BG_COLOR",                 // Background color for title
        "CHART_TITLE_FG_COLOR",                 // Foreground color for title
        "CHART_TEXT_COLOR",                     // Color of text labels
        "CHART_WIDTH",                          // Width of entire image
        "CHART_HEIGHT",                         // Height of entire image

        "BAR_IMAGE_BG_COLOR",                   // Background color of image
        "BAR_GRAPH_BG_COLOR",                   // Background color of plot
        "BAR_BAR_COLOR",                        // Color of bars
        "BAR_TITLE_BG_COLOR",                   // Background color of title
        "BAR_TITLE_FG_COLOR",                   // Foreground color of title
        "BAR_GRID_COLOR",                       // Color of grid lines
        "BAR_TEXT_COLOR",                       // Color of text labels
        "BAR_WIDTH",                            // Width of entire image
        "BAR_HEIGHT",                           // Height of entire image

        "DIAL_TRANSPARENT",                     // Background transparent?
        "DIAL_BG_COLOR",                        // Background color of dial
        "DIAL_IMAGE_BG_COLOR",                  // Background color of image (transparent)
        "DIAL_CENTER_COLOR",                    // Color of circle in center of image
        "DIAL_CENTER_TEXT_COLOR",               // Text color in image center (title)
        "DIAL_CENTER_HIGH_COLOR",               // Text color for high value
        "DIAL_POINTER_COLOR",                   // Pointer color
        "DIAL_POINTER_OUTLINE_COLOR",           // Color of pointer outline
        "DIAL_HIGH_COLOR",                      // Color of high-value tick
        "DIAL_LOW_COLOR",                       // Color of low-value tick
        "DIAL_APP_COLOR",                       //
        "DIAL_TEXT_COLOR",                      // Color of labels
        "DIAL_IMAGE_WIDTH",                     // Width of image
        "DIAL_DIAMETER",                        // Diameter of dial
        "DIAL_CTR_DIAMETER",                     // Diameter of center circle

        "MULTICHART_FIRST_LINE_COLOR",          // Color of first line
        "MULTICHART_SECOND_LINE_COLOR",         // Color of second line
        "MULTICHART_THIRD_LINE_COLOR",          // Color of third line
        "MULTICHART_FOURTH_LINE_COLOR"          // Color of forth line
    };

//  ... define the HTML data tags
//  ... "computeTag" below depends on the order of these strings!
static char *dataTags[] =
    {
        "<!--tempUnit-->",
        "<!--humUnit-->",
        "<!--windUnit-->",
        "<!--barUnit-->",
        "<!--rateUnit-->",
        "<!--rainUnit-->",
        "<!--stationCity-->",
        "<!--stationState-->",
        "<!--stationDate-->",
        "<!--stationTime-->",
        "<!--sunriseTime-->",                   // 10
        "<!--sunsetTime-->",
        "<!--outsideTemp-->",
        "<!--windChill-->",
        "<!--outsideHumidity-->",
        "<!--outsideHeatIndex-->",
        "<!--windDirection-->",
        "<!--windSpeed-->",
        "<!--outsideDewPt-->",
        "<!--barometer-->",
        "<!--rainRate-->",                      // 20
        "<!--dailyRain-->",
        "<!--monthlyRain-->",
        "<!--stormRain-->",
        "<!--totalRain-->",
        "<!--hiOutsideTemp-->",
        "<!--hiOutsideTempTime-->",
        "<!--lowOutsideTemp-->",
        "<!--lowOutsideTempTime-->",
        "<!--hiHumidity-->",
        "<!--hiHumTime-->",                     // 30
        "<!--lowHumidity-->",
        "<!--lowHumTime-->",
        "<!--hiDewpoint-->",
        "<!--hiDewpointTime-->",
        "<!--lowDewpoint-->",
        "<!--lowDewpointTime-->",
        "<!--hiWindSpeed-->",
        "<!--hiWindSpeedTime-->",
        "<!--hiBarometer-->",
        "<!--hiBarometerTime-->",               // 40
        "<!--lowBarometer-->",
        "<!--lowBarometerTime-->",
        "<!--hiRainRate-->",
        "<!--hiRainRateTime-->",
        "<!--lowWindchill-->",
        "<!--lowWindchillTime-->",
        "<!--hiHeatindex-->",
        "<!--hiHeatindexTime-->",
        "<!--hiMonthlyOutsideTemp-->",
        "<!--lowMonthlyOutsideTemp-->",         // 50
        "<!--hiMonthlyHumidity-->",
        "<!--lowMonthlyHumidity-->",
        "<!--hiMonthlyDewpoint-->",
        "<!--lowMonthlyDewpoint-->",
        "<!--hiMonthlyWindSpeed-->",
        "<!--hiMonthlyBarometer-->",
        "<!--lowMonthlyBarometer-->",
        "<!--lowMonthlyWindchill-->",
        "<!--hiMonthlyHeatindex-->",
        "<!--hiMonthlyRainRate-->",             // 60
        "<!--hiYearlyOutsideTemp-->",
        "<!--lowYearlyOutsideTemp-->",
        "<!--hiYearlyHumidity-->",
        "<!--lowYearlyHumidity-->",
        "<!--hiYearlyDewpoint-->",
        "<!--lowYearlyDewpoint-->",
        "<!--hiYearlyWindSpeed-->",
        "<!--hiYearlyBarometer-->",
        "<!--lowYearlyBarometer-->",
        "<!--lowYearlyWindchill-->",            // 70
        "<!--hiYearlyHeatindex-->",
        "<!--hiYearlyRainRate-->",
        "<!--wviewVersion-->",
        "<!--wviewUpTime-->",
        "<!--UV-->",
        "<!--ET-->",
        "<!--solarRad-->",
        "<!--extraTemp1-->",
        "<!--extraTemp2-->",
        "<!--extraTemp3-->",                    // 80
        "<!--soilTemp1-->",
        "<!--soilTemp2-->",
        "<!--soilTemp3-->",
        "<!--soilTemp4-->",
        "<!--leafTemp1-->",
        "<!--leafTemp2-->",
        "<!--extraHumid1-->",
        "<!--extraHumid2-->",
        "<!--forecastRule-->",
        "<!--forecastIcon-->",                  // 90
        "<!--stationElevation-->",
        "<!--stationLatitude-->",
        "<!--stationLongitude-->",
        "<!--insideTemp-->",
        "<!--insideHumidity-->",

        "<!--hourrain-->",
        "<!--hourwindrun-->",
        "<!--houravgtemp-->",
        "<!--houravgwind-->",
        "<!--hourdomwinddir-->",                // 100
        "<!--houravghumid-->",
        "<!--houravgdewpt-->",
        "<!--houravgbarom-->",
        "<!--hourchangetemp-->",
        "<!--hourchangewind-->",
        "<!--hourchangewinddir-->",
        "<!--hourchangehumid-->",
        "<!--hourchangedewpt-->",
        "<!--hourchangebarom-->",
        "<!--daywindrun-->",                    // 110
        "<!--dayavgtemp-->",
        "<!--dayavgwind-->",
        "<!--daydomwinddir-->",
        "<!--dayavghumid-->",
        "<!--dayavgdewpt-->",
        "<!--dayavgbarom-->",
        "<!--daychangetemp-->",
        "<!--daychangewind-->",
        "<!--daychangewinddir-->",
        "<!--daychangehumid-->",                // 120
        "<!--daychangedewpt-->",
        "<!--daychangebarom-->",
        "<!--weekwindrun-->",
        "<!--weekavgtemp-->",
        "<!--weekavgwind-->",
        "<!--weekdomwinddir-->",
        "<!--weekavghumid-->",
        "<!--weekavgdewpt-->",
        "<!--weekavgbarom-->",
        "<!--weekchangetemp-->",                // 130
        "<!--weekchangewind-->",
        "<!--weekchangewinddir-->",
        "<!--weekchangehumid-->",
        "<!--weekchangedewpt-->",
        "<!--weekchangebarom-->",
        "<!--monthtodatewindrun-->",
        "<!--monthtodateavgtemp-->",
        "<!--monthtodateavgwind-->",
        "<!--monthtodatedomwinddir-->",
        "<!--monthtodateavghumid-->",           // 140
        "<!--monthtodateavgdewpt-->",
        "<!--monthtodateavgbarom-->",
        "<!--monthtodatemaxtempdate-->",
        "<!--monthtodatemintempdate-->",
        "<!--monthtodateminchilldate-->",
        "<!--monthtodatemaxheatdate-->",
        "<!--monthtodatemaxhumiddate-->",
        "<!--monthtodateminhumiddate-->",
        "<!--monthtodatemaxdewptdate-->",
        "<!--monthtodatemindewptdate-->",       // 150
        "<!--monthtodatemaxbaromdate-->",
        "<!--monthtodateminbaromdate-->",
        "<!--monthtodatemaxwinddate-->",
        "<!--monthtodatemaxgustdate-->",
        "<!--monthtodatemaxrainratedate-->",
        "<!--yeartodatewindrun-->",
        "<!--yeartodateavgtemp-->",
        "<!--yeartodateavgwind-->",
        "<!--yeartodatedomwinddir-->",
        "<!--yeartodateavghumid-->",            // 160
        "<!--yeartodateavgdewpt-->",
        "<!--yeartodateavgbarom-->",
        "<!--yeartodatemaxtempdate-->",
        "<!--yeartodatemintempdate-->",
        "<!--yeartodateminchilldate-->",
        "<!--yeartodatemaxheatdate-->",
        "<!--yeartodatemaxhumiddate-->",
        "<!--yeartodateminhumiddate-->",
        "<!--yeartodatemaxdewptdate-->",
        "<!--yeartodatemindewptdate-->",        // 170
        "<!--yeartodatemaxbaromdate-->",
        "<!--yeartodateminbaromdate-->",
        "<!--yeartodatemaxwinddate-->",
        "<!--yeartodatemaxgustdate-->",
        "<!--yeartodatemaxrainratedate-->",
        "<!--windDirectionDegrees-->",
        "<!--PLACEHOLDER1-->",
        "<!--PLACEHOLDER2-->",
        "<!--PLACEHOLDER3-->",
        "<!--PLACEHOLDER4-->",                  // 180
        "<!--hiRadiation-->",
        "<!--hiRadiationTime-->",
        "<!--hiMonthlyRadiation-->",
        "<!--hiYearlyRadiation-->",
        "<!--hiUV-->",
        "<!--hiUVTime-->",
        "<!--hiMonthlyUV-->",
        "<!--hiYearlyUV-->",
        "<!--moonPhase-->",
        "<!--airDensityUnit-->",                // 190
        "<!--airDensity-->",
        "<!--cumulusBaseUnit-->",
        "<!--cumulusBase-->",
        "<!--soilMoist1-->",
        "<!--soilMoist2-->",
        "<!--leafWet1-->",
        "<!--leafWet2-->",
        "<!--dayhighwinddir-->",
        "<!--baromtrend-->",
        "<!--dailyRainMM-->",                   // 200
        "<!--stationDateMetric-->",
        "<!--middayTime-->",
        "<!--dayLength-->",
        "<!--civilriseTime-->",
        "<!--civilsetTime-->",
        "<!--astroriseTime-->",
        "<!--astrosetTime-->",
        "<!--stormStart-->",
        "<!--forecastIconFile-->",
        "<!--rainSeasonStart-->",               // 210
        "<!--intervalAvgWindChill-->",
        "<!--intervalAvgWindSpeed-->",
        "<!--stationPressure-->",
        "<!--altimeter-->",
        "<!--localRadarURL-->",
        "<!--localForecastURL-->",
        "<!--windGustSpeed-->",
        "<!--windGustDirectionDegrees-->",
        "<!--windBeaufortScale-->",
        "<!--intervalAvgBeaufortScale-->",      // 220
        "<!--stationType-->",

        "<!--wxt510Hail-->",
        "<!--wxt510Hailrate-->",
        "<!--wxt510HeatingTemp-->",
        "<!--wxt510HeatingVoltage-->",
        "<!--wxt510SupplyVoltage-->",
        "<!--wxt510ReferenceVoltage-->",
        "<!--wxt510RainDuration-->",
        "<!--wxt510RainPeakRate-->",
        "<!--wxt510HailDuration-->",            // 230
        "<!--wxt510HailPeakRate-->",
        "<!--wxt510Rain-->",

        "<!--rxCheckPercent-->",
        "<!--tenMinuteAvgWindSpeed-->",
        "<!--PLACEHOLDER1-->",
        "<!--PLACEHOLDER2-->",
        "<!--txBatteryStatus-->",
        "<!--consBatteryVoltage-->",

        "<!--stationTimeNoSecs-->",
        "<!--wxt510RainDurationMin-->",         // 240
        "<!--wxt510HailDurationMin-->",

        "<!--wmr918WindBatteryStatus-->",
        "<!--wmr918RainBatteryStatus-->",
        "<!--wmr918OutTempBatteryStatus-->",
        "<!--wmr918InTempBatteryStatus-->",

        "<!--windSpeed_ms-->",
        "<!--windGustSpeed_ms-->",
        "<!--intervalAvgWindSpeed_ms-->",
        "<!--tenMinuteAvgWindSpeed_ms-->",
        "<!--hiWindSpeed_ms-->",                // 250
        "<!--hiMonthlyWindSpeed_ms-->",
        "<!--hiYearlyWindSpeed_ms-->",
        "<!--houravgwind_ms-->",
        "<!--dayavgwind_ms-->",
        "<!--weekavgwind_ms-->",
        "<!--monthtodateavgwind_ms-->",
        "<!--yeartodateavgwind_ms-->",
        "<!--hourchangewind_ms-->",
        "<!--daychangewind_ms-->",
        "<!--weekchangewind_ms-->",             // 260

        "<!--windSpeed_kts-->",
        "<!--windGustSpeed_kts-->",
        "<!--intervalAvgWindSpeed_kts-->",
        "<!--tenMinuteAvgWindSpeed_kts-->",
        "<!--hiWindSpeed_kts-->",
        "<!--hiMonthlyWindSpeed_kts-->",
        "<!--hiYearlyWindSpeed_kts-->",
        "<!--houravgwind_kts-->",
        "<!--dayavgwind_kts-->",
        "<!--weekavgwind_kts-->",               // 270
        "<!--monthtodateavgwind_kts-->",
        "<!--yeartodateavgwind_kts-->",
        "<!--hourchangewind_kts-->",
        "<!--daychangewind_kts-->",
        "<!--weekchangewind_kts-->",

        "<!--wmr918Humid3-->",
        "<!--wmr918Pool-->",
        "<!--wmr918poolTempBatteryStatus-->",
        "<!--wmr918extra1BatteryStatus-->",
        "<!--wmr918extra2BatteryStatus-->",     // 280
        "<!--wmr918extra3BatteryStatus-->",

        "<!--hiAllTimeOutsideTemp-->",
        "<!--lowAllTimeOutsideTemp-->",
        "<!--hiAllTimeHumidity-->",
        "<!--lowAllTimeHumidity-->",
        "<!--hiAllTimeDewpoint-->",
        "<!--lowAllTimeDewpoint-->",
        "<!--hiAllTimeWindSpeed-->",
        "<!--hiAllTimeBarometer-->",
        "<!--lowAllTimeBarometer-->",           // 290
        "<!--lowAllTimeWindchill-->",
        "<!--hiAllTimeHeatindex-->",
        "<!--hiAllTimeRainRate-->",
        "<!--hiAllTimeRadiation-->",
        "<!--hiAllTimeUV-->",

        "<!--alltimeavgtemp-->",
        "<!--alltimeavgwind-->",
        "<!--alltimedomwinddir-->",
        "<!--alltimeavghumid-->",
        "<!--alltimeavgdewpt-->",               // 300
        "<!--alltimeavgbarom-->",

        "<!--alltimemaxtempdate-->",
        "<!--alltimemintempdate-->",
        "<!--alltimeminchilldate-->",
        "<!--alltimemaxheatdate-->",
        "<!--alltimemaxhumiddate-->",
        "<!--alltimeminhumiddate-->",
        "<!--alltimemaxdewptdate-->",
        "<!--alltimemindewptdate-->",
        "<!--alltimemaxbaromdate-->",           // 310
        "<!--alltimeminbaromdate-->",
        "<!--alltimemaxwinddate-->",
        "<!--alltimemaxgustdate-->",
        "<!--alltimemaxrainratedate-->",
        "<!--wmr918Tendency-->",
        "<!--stationName-->",
        "<!--moonriseTime-->",
        "<!--moonsetTime-->",
        "<!--apparentTemp-->",

        "<!--genExtraTemp1-->",                 // 320
        "<!--genExtraTemp2-->",
        "<!--genExtraTemp3-->",
        "<!--genExtraTemp4-->",
        "<!--genExtraTemp5-->",
        "<!--genExtraTemp6-->",
        "<!--genExtraTemp7-->",
        "<!--genExtraTemp8-->",
        "<!--genExtraTemp9-->",
        "<!--genExtraTemp10-->",
        "<!--genExtraTemp11-->",                // 330
        "<!--genExtraTemp12-->",
        "<!--genExtraTemp13-->",
        "<!--genExtraTemp14-->",
        "<!--genExtraTemp15-->",
        "<!--genExtraTemp16-->",
        "<!--genExtraHumidity1-->",
        "<!--genExtraHumidity2-->",
        "<!--genExtraHumidity3-->",
        "<!--genExtraHumidity4-->",
        "<!--genExtraHumidity5-->",             // 340
        "<!--genExtraHumidity6-->",
        "<!--genExtraHumidity7-->",
        "<!--genExtraHumidity8-->",
        "<!--genExtraHumidity9-->",
        "<!--genExtraHumidity10-->",
        "<!--genExtraHumidity11-->",
        "<!--genExtraHumidity12-->",
        "<!--genExtraHumidity13-->",
        "<!--genExtraHumidity14-->",
        "<!--genExtraHumidity15-->",            // 350
        "<!--genExtraHumidity16-->",
        "<!--genExtraTempBatteryStatus1-->",
        "<!--genExtraTempBatteryStatus2-->",
        "<!--genExtraTempBatteryStatus3-->",
        "<!--genExtraTempBatteryStatus4-->",
        "<!--genExtraTempBatteryStatus5-->",
        "<!--genExtraTempBatteryStatus6-->",
        "<!--genExtraTempBatteryStatus7-->",
        "<!--genExtraTempBatteryStatus8-->",
        "<!--genExtraTempBatteryStatus9-->",    // 360
        "<!--genExtraTempBatteryStatus10-->",
        "<!--genExtraTempBatteryStatus11-->",
        "<!--genExtraTempBatteryStatus12-->",
        "<!--genExtraTempBatteryStatus13-->",
        "<!--genExtraTempBatteryStatus14-->",
        "<!--genExtraTempBatteryStatus15-->",
        "<!--genExtraTempBatteryStatus16-->",
        "<!--genExtraWindBatteryStatus-->",
        "<!--genExtraOutTempBatteryStatus-->",
        "<!--genExtraConsoleBatteryStatus-->",  // 370
        "<!--genExtraUVBatteryStatus-->",
        "<!--genExtraSolarBatteryStatus-->",
        "<!--genExtraRainBatteryStatus-->",
        "<!--WetBulbTemp-->",

        NULL
    };

static char BPTrendLabels[4] =
    {
        '-',
        '~',
        '+',
        ' '
    };

static char *monthLabel[13] =
    {
        "UNK",
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

static char                 tendency[5][14] =
{
        "Clear",
        "Partly Cloudy",
        "Cloudy",
        "Rain",
        "Unknown"
};

static char *buildTimeTag (int16_t timeval)
{
    static char     ret[16];

    if (timeval < 0)
    {
        sprintf (ret, "--:--");
    }
    else
    {
        sprintf (ret, "%2.2d:%2.2d",
                 EXTRACT_PACKED_HOUR(timeval),
                 EXTRACT_PACKED_MINUTE(timeval));
    }
    return ret;
}

char *buildWindDirString (int dir)
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

char *buildTendencyString (int tend)
{
    int text;

    switch (tend)
    {
    case 0x0c: // Clear
            text = 0;
            break;
    case 0x06: // Partly Cloudy
            text = 1;
            break;
    case 0x02: // Cloudy
            text = 2;
            break;
    case 0x03: // Rain
            text = 3;
            break;
    default: // Unknown
            text = 4;
    }

    return tendency[text];
}

static char *makeduration (float x)
{
    int         hour, min, sec;
    static char duration[32];

    hour = (int)( x / 3600 );
    x -= (hour * 3600);
    min = (int)( x / 60 );
    sec = x - ( 60 * min );
    sprintf (duration, "%d:%2.2d:%2.2d", hour, min, sec);

    return duration;

}

static char* getBattStatus(uint8_t status)
{
    static char batteryStatus[16];

    if (status == 0)
        sprintf(batteryStatus, "LOW");
    else if (status == 1)
        sprintf(batteryStatus, "OK");
    else
        sprintf(batteryStatus, "UNKNOWN");
    return batteryStatus;
}

static void computeTag (HTML_MGR_ID id, char *tag, int len, char *store)
{
    time_t          ntime;
    struct tm       loctime;
    char            temp[SEARCH_TEXT_MAX];
    int             tagIndex, tempInt, tempInt1;
    float           tempfloat;
    SENSOR_STORE    *sensors = &id->hilowStore;

    // First, find the index:
    strncpy(temp, tag, len);
    temp[len] = 0;

    if (radtextsearchFind(tagSearchEngine, temp, &tagIndex) == ERROR)
    {
        store[0] = 0;
        return;
    }

    //  now "tagIndex" is the index for the matched tag:
    switch (tagIndex)
    {
    case 0:
        if (id->isMetricUnits)
            strcpy (store, " C");
        else
            strcpy (store, " F");
        break;
    case 1:
        strcpy (store, " %");
        break;
    case 2:
        sprintf (store, " %s", wvutilsGetWindUnitLabel());
        break;
    case 3:
        if (id->isMetricUnits)
            strcpy (store, " mb");
        else
            strcpy (store, " in");
        break;
    case 4:
        if (id->isMetricUnits)
        {
            if (wvutilsGetRainIsMM())
                strcpy (store, " mm/h");
            else
                strcpy (store, " cm/h");
        }
        else
            strcpy (store, " in/hr");
        break;
    case 5:
        if (id->isMetricUnits)
        {
            if (wvutilsGetRainIsMM())
                strcpy (store, " mm");
            else
                strcpy (store, " cm");
        }
        else
            strcpy (store, " in");
        break;
    case 6:
        wvstrncpy (store, id->stationCity, HTML_MAX_LINE_LENGTH);
        break;
    case 7:
        wvstrncpy (store, id->stationState, HTML_MAX_LINE_LENGTH);
        break;
    case 8:
        ntime = time(NULL);
        localtime_r (&ntime, &loctime);
        strftime(store, 256, id->dateFormat, &loctime);
        break;
    case 9:
        ntime = time (NULL);
        localtime_r (&ntime, &loctime);
        sprintf (store, "%2.2d:%2.2d:%2.2d",
                 loctime.tm_hour,
                 loctime.tm_min,
                 loctime.tm_sec);
        break;
    case 10:
        wvstrncpy (store, buildTimeTag (id->sunrise), HTML_MAX_LINE_LENGTH);
        break;
    case 11:
        wvstrncpy (store, buildTimeTag (id->sunset), HTML_MAX_LINE_LENGTH);
        break;
    case 12:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(id->loopStore.outTemp));
        }
        else
        {
            sprintf (store, "%.1f", id->loopStore.outTemp);
        }
        break;
    case 13:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(id->loopStore.windchill));
        }
        else
        {
            sprintf (store, "%.1f", id->loopStore.windchill);
        }
        break;
    case 14:
        sprintf (store, "%d", id->loopStore.outHumidity);
        break;
    case 15:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(id->loopStore.heatindex));
        }
        else
        {
            sprintf (store, "%.1f", id->loopStore.heatindex);
        }
        break;
    case 16:
        tempfloat = (float)id->loopStore.windDir + 11.24;
        tempfloat /= 22.5;
        tempInt = (int)tempfloat;
        tempInt %= 16;
        sprintf (store, "%s", buildWindDirString(tempInt));
        break;
    case 17:
        sprintf (store, 
                 "%.1f",
                 wvutilsGetWindSpeed(id->loopStore.windSpeedF));
        break;
    case 18:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(id->loopStore.dewpoint));
        }
        else
        {
            sprintf (store, "%.1f", id->loopStore.dewpoint);
        }
        break;
    case 19:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertINHGToHPA(id->loopStore.barometer));
        }
        else
        {
            sprintf (store, "%.3f", id->loopStore.barometer);
        }
        break;
    case 20:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.2f",
                     wvutilsConvertRainINToMetric(id->loopStore.rainRate));
        }
        else
        {
            sprintf (store, "%.2f", id->loopStore.rainRate);
        }
        break;
    case 21:
        if (id->isMetricUnits)
        {
            strcpy (store,
                    wvutilsPrintFloat(wvutilsConvertRainINToMetric(id->loopStore.dayRain), 2));
        }
        else
        {
            strcpy (store, wvutilsPrintFloat(id->loopStore.dayRain, 2));
        }
        break;
    case 22:
        if (id->isMetricUnits)
        {
            strcpy (store,
                    wvutilsPrintFloat(wvutilsConvertRainINToMetric(id->loopStore.monthRain), 2));
        }
        else
        {
            strcpy (store, wvutilsPrintFloat(id->loopStore.monthRain, 2));
        }
        break;
    case 23:
        if (id->isMetricUnits)
        {
            strcpy (store,
                    wvutilsPrintFloat(wvutilsConvertRainINToMetric(id->loopStore.stormRain), 2));
        }
        else
        {
            strcpy (store, wvutilsPrintFloat(id->loopStore.stormRain, 2));
        }
        break;
    case 24:
        if (id->isMetricUnits)
        {
            strcpy (store,
                    wvutilsPrintFloat(wvutilsConvertRainINToMetric(id->loopStore.yearRain), 2));
        }
        else
        {
            sprintf (store, "%s",
                     wvutilsPrintFloat(id->loopStore.yearRain, 2));
        }
        break;
    case 25:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(sensorGetDailyHigh(sensors->sensor, SENSOR_OUTTEMP)));
        }
        else
        {
            sprintf (store, "%.1f", sensorGetDailyHigh(sensors->sensor, SENSOR_OUTTEMP));
        }
        break;
    case 26:
        strcpy (store, sensorGetDailyHighTime(sensors->sensor, SENSOR_OUTTEMP, temp));
        break;
    case 27:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(sensorGetDailyLow(sensors->sensor, SENSOR_OUTTEMP)));
        }
        else
        {
            sprintf (store, "%.1f", sensorGetDailyLow(sensors->sensor, SENSOR_OUTTEMP));
        }
        break;
    case 28:
        strcpy (store, sensorGetDailyLowTime(sensors->sensor, SENSOR_OUTTEMP, temp));
        break;
    case 29:
        sprintf (store, "%d", (int)sensorGetDailyHigh(sensors->sensor, SENSOR_OUTHUMID));
        break;
    case 30:
        strcpy (store, sensorGetDailyHighTime(sensors->sensor, SENSOR_OUTHUMID, temp));
        break;
    case 31:
        sprintf (store, "%d", (int)sensorGetDailyLow(sensors->sensor, SENSOR_OUTHUMID));
        break;
    case 32:
        strcpy (store, sensorGetDailyLowTime(sensors->sensor, SENSOR_OUTHUMID, temp));
        break;
    case 33:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(sensorGetDailyHigh(sensors->sensor, SENSOR_DEWPOINT)));
        }
        else
        {
            sprintf (store, "%.1f", sensorGetDailyHigh(sensors->sensor, SENSOR_DEWPOINT));
        }
        break;
    case 34:
        strcpy (store, sensorGetDailyHighTime(sensors->sensor, SENSOR_DEWPOINT, temp));
        break;
    case 35:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(sensorGetDailyLow(sensors->sensor, SENSOR_DEWPOINT)));
        }
        else
        {
            sprintf (store, "%.1f", sensorGetDailyLow(sensors->sensor, SENSOR_DEWPOINT));
        }
        break;
    case 36:
        strcpy (store, sensorGetDailyLowTime(sensors->sensor, SENSOR_DEWPOINT, temp));
        break;
    case 37:
        sprintf (store, 
                 "%.1f", 
                 wvutilsGetWindSpeed(sensorGetDailyHigh(sensors->sensor, SENSOR_WGUST))
                 );
        break;
    case 38:
        strcpy (store, sensorGetDailyHighTime(sensors->sensor, SENSOR_WGUST, temp));
        break;
    case 39:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertINHGToHPA(sensorGetDailyHigh(sensors->sensor, SENSOR_BP)));
        }
        else
        {
            sprintf (store, "%.3f",
                     sensorGetDailyHigh(sensors->sensor, SENSOR_BP));
        }
        break;
    case 40:
        strcpy (store, sensorGetDailyHighTime(sensors->sensor, SENSOR_BP, temp));
        break;
    case 41:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertINHGToHPA(sensorGetDailyLow(sensors->sensor, SENSOR_BP)));
        }
        else
        {
            sprintf (store, "%.3f",
                     sensorGetDailyLow(sensors->sensor, SENSOR_BP));
        }
        break;
    case 42:
        strcpy (store, sensorGetDailyLowTime(sensors->sensor, SENSOR_BP, temp));
        break;
    case 43:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.2f",
                     wvutilsConvertRainINToMetric(sensorGetDailyHigh(sensors->sensor, SENSOR_RAINRATE)));
        }
        else
        {
            sprintf (store, "%.2f", sensorGetDailyHigh(sensors->sensor, SENSOR_RAINRATE));
        }
        break;
    case 44:
        if (sensorGetDailyHigh(sensors->sensor, SENSOR_RAINRATE) < 0.01)
            strcpy (store, "-----");
        else
            strcpy (store, sensorGetDailyHighTime(sensors->sensor, SENSOR_RAINRATE, temp));
        break;
    case 45:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(sensorGetDailyLow(sensors->sensor, SENSOR_WCHILL)));
        }
        else
        {
            sprintf (store, "%.1f", sensorGetDailyLow(sensors->sensor, SENSOR_WCHILL));
        }
        break;
    case 46:
        strcpy (store, sensorGetDailyLowTime(sensors->sensor, SENSOR_WCHILL, temp));
        break;
    case 47:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(sensorGetDailyHigh(sensors->sensor, SENSOR_HINDEX)));
        }
        else
        {
            sprintf (store, "%.1f", sensorGetDailyHigh(sensors->sensor, SENSOR_HINDEX));
        }
        break;
    case 48:
        strcpy (store, sensorGetDailyHighTime(sensors->sensor, SENSOR_HINDEX, temp));
        break;
    case 49:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(sensorGetHigh(&sensors->sensor[STF_MONTH][SENSOR_OUTTEMP])));
        }
        else
        {
            sprintf (store, "%.1f", sensorGetHigh(&sensors->sensor[STF_MONTH][SENSOR_OUTTEMP]));
        }
        break;
    case 50:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(sensorGetLow(&sensors->sensor[STF_MONTH][SENSOR_OUTTEMP])));
        }
        else
        {
            sprintf (store, "%.1f", sensorGetLow(&sensors->sensor[STF_MONTH][SENSOR_OUTTEMP]));
        }
        break;
    case 51:
        sprintf (store, "%d", (int)sensorGetHigh(&sensors->sensor[STF_MONTH][SENSOR_OUTHUMID]));
        break;
    case 52:
        sprintf (store, "%d", (int)sensorGetLow(&sensors->sensor[STF_MONTH][SENSOR_OUTHUMID]));
        break;
    case 53:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(sensorGetHigh(&sensors->sensor[STF_MONTH][SENSOR_DEWPOINT])));
        }
        else
        {
            sprintf (store, "%.1f", sensorGetHigh(&sensors->sensor[STF_MONTH][SENSOR_DEWPOINT]));
        }
        break;
    case 54:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(sensorGetLow(&sensors->sensor[STF_MONTH][SENSOR_DEWPOINT])));
        }
        else
        {
            sprintf (store, "%.1f", sensorGetLow(&sensors->sensor[STF_MONTH][SENSOR_DEWPOINT]));
        }
        break;
    case 55:
        sprintf (store, 
                 "%.1f",
                 wvutilsGetWindSpeed(sensorGetHigh(&sensors->sensor[STF_MONTH][SENSOR_WGUST]))
                 );
        break;
    case 56:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertINHGToHPA(sensorGetHigh(&sensors->sensor[STF_MONTH][SENSOR_BP])));
        }
        else
        {
            sprintf (store, "%.3f",
                     sensorGetHigh(&sensors->sensor[STF_MONTH][SENSOR_BP]));
        }
        break;
    case 57:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertINHGToHPA(sensorGetLow(&sensors->sensor[STF_MONTH][SENSOR_BP])));
        }
        else
        {
            sprintf (store, "%.3f",
                     sensorGetLow(&sensors->sensor[STF_MONTH][SENSOR_BP]));
        }
        break;
    case 58:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(sensorGetLow(&sensors->sensor[STF_MONTH][SENSOR_WCHILL])));
        }
        else
        {
            sprintf (store, "%.1f", sensorGetLow(&sensors->sensor[STF_MONTH][SENSOR_WCHILL]));
        }
        break;
    case 59:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(sensorGetHigh(&sensors->sensor[STF_MONTH][SENSOR_HINDEX])));
        }
        else
        {
            sprintf (store, "%.1f", sensorGetHigh(&sensors->sensor[STF_MONTH][SENSOR_HINDEX]));
        }
        break;
    case 60:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.2f",
                     wvutilsConvertRainINToMetric(sensorGetHigh(&sensors->sensor[STF_MONTH][SENSOR_RAINRATE])));
        }
        else
        {
            sprintf (store, "%.2f", sensorGetHigh(&sensors->sensor[STF_MONTH][SENSOR_RAINRATE]));
        }
        break;
    case 61:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(sensorGetHigh(&sensors->sensor[STF_YEAR][SENSOR_OUTTEMP])));
        }
        else
        {
            sprintf (store, "%.1f", sensorGetHigh(&sensors->sensor[STF_YEAR][SENSOR_OUTTEMP]));
        }
        break;
    case 62:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(sensorGetLow(&sensors->sensor[STF_YEAR][SENSOR_OUTTEMP])));
        }
        else
        {
            sprintf (store, "%.1f", sensorGetLow(&sensors->sensor[STF_YEAR][SENSOR_OUTTEMP]));
        }
        break;
    case 63:
        sprintf (store, "%d", (int)sensorGetHigh(&sensors->sensor[STF_YEAR][SENSOR_OUTHUMID]));
        break;
    case 64:
        sprintf (store, "%d", (int)sensorGetLow(&sensors->sensor[STF_YEAR][SENSOR_OUTHUMID]));
        break;
    case 65:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(sensorGetHigh(&sensors->sensor[STF_YEAR][SENSOR_DEWPOINT])));
        }
        else
        {
            sprintf (store, "%.1f", sensorGetHigh(&sensors->sensor[STF_YEAR][SENSOR_DEWPOINT]));
        }
        break;
    case 66:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(sensorGetLow(&sensors->sensor[STF_YEAR][SENSOR_DEWPOINT])));
        }
        else
        {
            sprintf (store, "%.1f", sensorGetLow(&sensors->sensor[STF_YEAR][SENSOR_DEWPOINT]));
        }
        break;
    case 67:
        sprintf (store, 
                 "%.1f",
                 wvutilsGetWindSpeed(sensorGetHigh(&sensors->sensor[STF_YEAR][SENSOR_WGUST]))
                 );
        break;
    case 68:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertINHGToHPA(sensorGetHigh(&sensors->sensor[STF_YEAR][SENSOR_BP])));
        }
        else
        {
            sprintf (store, "%.3f",
                     sensorGetHigh(&sensors->sensor[STF_YEAR][SENSOR_BP]));
        }
        break;
    case 69:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertINHGToHPA(sensorGetLow(&sensors->sensor[STF_YEAR][SENSOR_BP])));
        }
        else
        {
            sprintf (store, "%.3f",
                     sensorGetLow(&sensors->sensor[STF_YEAR][SENSOR_BP]));
        }
        break;
    case 70:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(sensorGetLow(&sensors->sensor[STF_YEAR][SENSOR_WCHILL])));
        }
        else
        {
            sprintf (store, "%.1f", sensorGetLow(&sensors->sensor[STF_YEAR][SENSOR_WCHILL]));
        }
        break;
    case 71:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(sensorGetHigh(&sensors->sensor[STF_YEAR][SENSOR_HINDEX])));
        }
        else
        {
            sprintf (store, "%.1f", sensorGetHigh(&sensors->sensor[STF_YEAR][SENSOR_HINDEX]));
        }
        break;
    case 72:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.2f",
                     wvutilsConvertRainINToMetric(sensorGetHigh(&sensors->sensor[STF_YEAR][SENSOR_RAINRATE])));
        }
        else
        {
            sprintf (store, "%.2f", sensorGetHigh(&sensors->sensor[STF_YEAR][SENSOR_RAINRATE]));
        }
        break;
    case 73:
        sprintf (store, "%s", globalWviewVersionStr);
        break;
    case 74:
        sprintf (store, "%s", radSystemGetUpTimeSTR (WVIEW_SYSTEM_ID));
        break;
    case 75:
        sprintf (store, "%.1f", (float)id->loopStore.UV);
        break;
    case 76:
        if (id->isMetricUnits)
        {
            strcpy (store,
                    wvutilsPrintFloat(wvutilsConvertRainINToMetric(id->loopStore.dayET), 3));
        }
        else
        {
            strcpy (store, wvutilsPrintFloat(id->loopStore.dayET, 3));
        }
        break;
    case 77:
        sprintf (store, "%.0f", (float)id->loopStore.radiation);
        break;
    case 78:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(id->loopStore.extraTemp[0]));
        }
        else
        {
            sprintf (store, "%.0f", id->loopStore.extraTemp[0]);
        }
        break;
    case 79:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(id->loopStore.extraTemp[1]));
        }
        else
        {
            sprintf (store, "%.0f", id->loopStore.extraTemp[1]);
        }
        break;
    case 80:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(id->loopStore.extraTemp[2]));
        }
        else
        {
            sprintf (store, "%.0f", id->loopStore.extraTemp[2]);
        }
        break;
    case 81:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(id->loopStore.soilTemp1));
        }
        else
        {
            sprintf (store, "%.0f", id->loopStore.soilTemp1);
        }
        break;
    case 82:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(id->loopStore.soilTemp2));
        }
        else
        {
            sprintf (store, "%.0f", id->loopStore.soilTemp2);
        }
        break;
    case 83:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(id->loopStore.soilTemp3));
        }
        else
        {
            sprintf (store, "%.0f", id->loopStore.soilTemp3);
        }
        break;
    case 84:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(id->loopStore.soilTemp4));
        }
        else
        {
            sprintf (store, "%.0f", id->loopStore.soilTemp4);
        }
        break;
    case 85:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(id->loopStore.leafTemp1));
        }
        else
        {
            sprintf (store, "%.0f", id->loopStore.leafTemp1);
        }
        break;
    case 86:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(id->loopStore.leafTemp2));
        }
        else
        {
            sprintf (store, "%.0f", id->loopStore.leafTemp2);
        }
        break;
    case 87:
        sprintf (store, "%d", (int)id->loopStore.extraHumidity[0]);
        break;
    case 88:
        sprintf (store, "%d", (int)id->loopStore.extraHumidity[1]);
        break;
    case 89:
        if (id->loopStore.forecastRule <= HTML_MAX_FCAST_RULE &&
                id->ForecastRuleText[id->loopStore.forecastRule] != NULL)
            wvstrncpy (store, 
                     id->ForecastRuleText[id->loopStore.forecastRule],
                     HTML_MAX_LINE_LENGTH);
        else
            store[0] = 0;
        break;
    case 90:
        // copy html for icon image
        tempInt = id->loopStore.forecastIcon;
        if (tempInt > 0 &&
                tempInt <= VP_FCAST_ICON_MAX &&
                id->ForecastIconFile[tempInt] != NULL)
        {
            sprintf (store, "<img src=\"%s\">", id->ForecastIconFile[tempInt]);
        }
        else
            store[0] = 0;
        break;
    case 91:
        if (id->isMetricUnits)
            sprintf (store, "%d m", (int)wvutilsConvertFeetToMeters((int)id->stationElevation));
        else
            sprintf (store, "%d ft", (int)id->stationElevation);
        break;
    case 92:
        sprintf (store, "%3.1f %c",
                 ((float)abs((int)id->stationLatitude))/10.0,
                 ((id->stationLatitude < 0) ? 'S' : 'N'));
        break;
    case 93:
        sprintf (store, "%3.1f %c",
                 ((float)abs((int)id->stationLongitude))/10.0,
                 ((id->stationLongitude < 0) ? 'W' : 'E'));
        break;
    case 94:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(id->loopStore.inTemp));
        }
        else
        {
            sprintf (store, "%.1f", id->loopStore.inTemp);
        }
        break;
    case 95:
        sprintf (store, "%d", id->loopStore.inHumidity);
        break;
    case 96:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.2f",
                     wvutilsConvertRainINToMetric(sensorGetCumulative(&sensors->sensor[STF_HOUR][SENSOR_RAIN])));
        }
        else
        {
            sprintf (store, "%.2f", sensorGetCumulative(&sensors->sensor[STF_HOUR][SENSOR_RAIN]));
        }
        break;
    case 97:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f km",
                     wvutilsConvertMilesToKilometers(sensorGetWindRun(STF_HOUR, &sensors->sensor[STF_HOUR][SENSOR_WSPEED])));
        }
        else
        {
            sprintf (store, "%.1f miles", sensorGetWindRun(STF_HOUR, &sensors->sensor[STF_HOUR][SENSOR_WSPEED]));
        }
        break;
    case 98:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(sensorGetAvg(&sensors->sensor[STF_HOUR][SENSOR_OUTTEMP])));
        }
        else
        {
            sprintf (store, "%.1f", sensorGetAvg(&sensors->sensor[STF_HOUR][SENSOR_OUTTEMP]));
        }
        break;
    case 99:
        sprintf (store, 
                 "%.1f",
                 wvutilsGetWindSpeed(sensorGetAvg(&sensors->sensor[STF_HOUR][SENSOR_WSPEED]))
                 );
        break;
    case 100:
        sprintf (store, "%d", windAverageCompute(&sensors->wind[STF_HOUR]));
        break;
    case 101:
        sprintf (store, "%.0f", sensorGetAvg(&sensors->sensor[STF_HOUR][SENSOR_OUTHUMID]));
        break;
    case 102:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(sensorGetAvg(&sensors->sensor[STF_HOUR][SENSOR_DEWPOINT])));
        }
        else
        {
            sprintf (store, "%.1f", sensorGetAvg(&sensors->sensor[STF_HOUR][SENSOR_DEWPOINT]));
        }
        break;
    case 103:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertINHGToHPA(sensorGetAvg(&sensors->sensor[STF_HOUR][SENSOR_BP])));
        }
        else
        {
            sprintf (store, "%.3f",
                     sensorGetAvg(&sensors->sensor[STF_HOUR][SENSOR_BP]));
        }
        break;
    case 104:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertDeltaFToC(id->hilowStore.hourchangetemp));
        }
        else
        {
            sprintf (store, "%.1f", id->hilowStore.hourchangetemp);
        }
        break;
    case 105:
        sprintf (store, 
                 "%.1f",
                 wvutilsGetWindSpeed(id->hilowStore.hourchangewind));
        break;
    case 106:
        sprintf (store, "%d", id->hilowStore.hourchangewinddir);
        break;
    case 107:
        sprintf (store, "%d", id->hilowStore.hourchangehumid);
        break;
    case 108:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertDeltaFToC(id->hilowStore.hourchangedewpt));
        }
        else
        {
            sprintf (store, "%.1f", id->hilowStore.hourchangedewpt);
        }
        break;
    case 109:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertINHGToHPA(id->hilowStore.hourchangebarom));
        }
        else
        {
            sprintf (store, "%.3f",
                     id->hilowStore.hourchangebarom);
        }
        break;
    case 110:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f km",
                     wvutilsConvertMilesToKilometers(sensorGetWindRun(STF_DAY, &sensors->sensor[STF_DAY][SENSOR_WSPEED])));
        }
        else
        {
            sprintf (store, "%.1f miles", sensorGetWindRun(STF_DAY, &sensors->sensor[STF_DAY][SENSOR_WSPEED]));
        }
        break;
    case 111:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(sensorGetAvg(&sensors->sensor[STF_DAY][SENSOR_OUTTEMP])));
        }
        else
        {
            sprintf (store, "%.1f", sensorGetAvg(&sensors->sensor[STF_DAY][SENSOR_OUTTEMP]));
        }
        break;
    case 112:
        sprintf (store, 
                 "%.1f",
                 wvutilsGetWindSpeed(sensorGetAvg(&sensors->sensor[STF_DAY][SENSOR_WSPEED])));
        break;
    case 113:
        sprintf (store, "%d", windAverageCompute(&sensors->wind[STF_DAY]));
        break;
    case 114:
        sprintf (store, "%.0f", sensorGetAvg(&sensors->sensor[STF_DAY][SENSOR_OUTHUMID]));
        break;
    case 115:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(sensorGetAvg(&sensors->sensor[STF_DAY][SENSOR_DEWPOINT])));
        }
        else
        {
            sprintf (store, "%.1f", sensorGetAvg(&sensors->sensor[STF_DAY][SENSOR_DEWPOINT]));
        }
        break;
    case 116:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertINHGToHPA(sensorGetAvg(&sensors->sensor[STF_DAY][SENSOR_BP])));
        }
        else
        {
            sprintf (store, "%.3f",
                     sensorGetAvg(&sensors->sensor[STF_DAY][SENSOR_BP]));
        }
        break;
    case 117:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertDeltaFToC(id->hilowStore.daychangetemp));
        }
        else
        {
            sprintf (store, "%.1f", id->hilowStore.daychangetemp);
        }
        break;
    case 118:
        sprintf (store, 
                 "%.1f",
                 wvutilsGetWindSpeed(id->hilowStore.daychangewind));
        break;
    case 119:
        sprintf (store, "%d", id->hilowStore.daychangewinddir);
        break;
    case 120:
        sprintf (store, "%d", id->hilowStore.daychangehumid);
        break;
    case 121:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertDeltaFToC(id->hilowStore.daychangedewpt));
        }
        else
        {
            sprintf (store, "%.1f", id->hilowStore.daychangedewpt);
        }
        break;
    case 122:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertINHGToHPA(id->hilowStore.daychangebarom));
        }
        else
        {
            sprintf (store, "%.3f",
                     id->hilowStore.daychangebarom);
        }
        break;
    case 123:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f km",
                     wvutilsConvertMilesToKilometers(sensorGetWindRun(STF_WEEK, &sensors->sensor[STF_WEEK][SENSOR_WSPEED])));
        }
        else
        {
            sprintf (store, "%.1f miles", sensorGetWindRun(STF_WEEK, &sensors->sensor[STF_WEEK][SENSOR_WSPEED]));
        }
        break;
    case 124:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(sensorGetAvg(&sensors->sensor[STF_WEEK][SENSOR_OUTTEMP])));
        }
        else
        {
            sprintf (store, "%.1f", sensorGetAvg(&sensors->sensor[STF_WEEK][SENSOR_OUTTEMP]));
        }
        break;
    case 125:
        sprintf (store, 
                 "%.1f",
                 wvutilsGetWindSpeed(sensorGetAvg(&sensors->sensor[STF_WEEK][SENSOR_WSPEED])));
        break;
    case 126:
        sprintf (store, "%d", windAverageCompute(&sensors->wind[STF_WEEK]));
        break;
    case 127:
        sprintf (store, "%.0f", sensorGetAvg(&sensors->sensor[STF_WEEK][SENSOR_OUTHUMID]));
        break;
    case 128:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(sensorGetAvg(&sensors->sensor[STF_WEEK][SENSOR_DEWPOINT])));
        }
        else
        {
            sprintf (store, "%.1f", sensorGetAvg(&sensors->sensor[STF_WEEK][SENSOR_DEWPOINT]));
        }
        break;
    case 129:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertINHGToHPA(sensorGetAvg(&sensors->sensor[STF_WEEK][SENSOR_BP])));
        }
        else
        {
            sprintf (store, "%.3f",
                     sensorGetAvg(&sensors->sensor[STF_WEEK][SENSOR_BP]));
        }
        break;
    case 130:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertDeltaFToC(id->hilowStore.weekchangetemp));
        }
        else
        {
            sprintf (store, "%.1f", id->hilowStore.weekchangetemp);
        }
        break;
    case 131:
        sprintf (store, 
                 "%.1f",
                 wvutilsGetWindSpeed(id->hilowStore.weekchangewind));
        break;
    case 132:
        sprintf (store, "%d", id->hilowStore.weekchangewinddir);
        break;
    case 133:
        sprintf (store, "%d", id->hilowStore.weekchangehumid);
        break;
    case 134:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertDeltaFToC(id->hilowStore.weekchangedewpt));
        }
        else
        {
            sprintf (store, "%.1f", id->hilowStore.weekchangedewpt);
        }
        break;
    case 135:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertINHGToHPA(id->hilowStore.weekchangebarom));
        }
        else
        {
            sprintf (store, "%.3f",
                     id->hilowStore.weekchangebarom);
        }
        break;
    case 136:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f km",
                     wvutilsConvertMilesToKilometers(sensorGetWindRun(STF_MONTH, &sensors->sensor[STF_MONTH][SENSOR_WSPEED])));
        }
        else
        {
            sprintf (store, "%.1f miles", sensorGetWindRun(STF_MONTH, &sensors->sensor[STF_MONTH][SENSOR_WSPEED]));
        }
        break;
    case 137:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(sensorGetAvg(&sensors->sensor[STF_MONTH][SENSOR_OUTTEMP])));
        }
        else
        {
            sprintf (store, "%.1f", sensorGetAvg(&sensors->sensor[STF_MONTH][SENSOR_OUTTEMP]));
        }
        break;
    case 138:
        sprintf (store, 
                 "%.1f",
                 wvutilsGetWindSpeed(sensorGetAvg(&sensors->sensor[STF_MONTH][SENSOR_WSPEED])));
        break;
    case 139:
        sprintf (store, "%d", windAverageCompute(&sensors->wind[STF_MONTH]));
        break;
    case 140:
        sprintf (store, "%.0f", sensorGetAvg(&sensors->sensor[STF_MONTH][SENSOR_OUTHUMID]));
        break;
    case 141:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(sensorGetAvg(&sensors->sensor[STF_MONTH][SENSOR_DEWPOINT])));
        }
        else
        {
            sprintf (store, "%.1f", sensorGetAvg(&sensors->sensor[STF_MONTH][SENSOR_DEWPOINT]));
        }
        break;
    case 142:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertINHGToHPA(sensorGetAvg(&sensors->sensor[STF_MONTH][SENSOR_BP])));
        }
        else
        {
            sprintf (store, "%.3f",
                     sensorGetAvg(&sensors->sensor[STF_MONTH][SENSOR_BP]));
        }
        break;
    case 143:
        strcpy (store, sensorGetHighDate(&sensors->sensor[STF_MONTH][SENSOR_OUTTEMP],
                                         temp, id->dateFormat));
        break;
    case 144:
        strcpy (store, sensorGetLowDate(&sensors->sensor[STF_MONTH][SENSOR_OUTTEMP],
                                        temp, id->dateFormat));
        break;
    case 145:
        strcpy (store, sensorGetLowDate(&sensors->sensor[STF_MONTH][SENSOR_WCHILL],
                                        temp, id->dateFormat));
        break;
    case 146:
        strcpy (store, sensorGetHighDate(&sensors->sensor[STF_MONTH][SENSOR_HINDEX],
                                         temp, id->dateFormat));
        break;
    case 147:
        strcpy (store, sensorGetHighDate(&sensors->sensor[STF_MONTH][SENSOR_OUTHUMID],
                                         temp, id->dateFormat));
        break;
    case 148:
        strcpy (store, sensorGetLowDate(&sensors->sensor[STF_MONTH][SENSOR_OUTHUMID],
                                        temp, id->dateFormat));
        break;
    case 149:
        strcpy (store, sensorGetHighDate(&sensors->sensor[STF_MONTH][SENSOR_DEWPOINT],
                                         temp, id->dateFormat));
        break;
    case 150:
        strcpy (store, sensorGetLowDate(&sensors->sensor[STF_MONTH][SENSOR_DEWPOINT],
                                        temp, id->dateFormat));
        break;
    case 151:
        strcpy (store, sensorGetHighDate(&sensors->sensor[STF_MONTH][SENSOR_BP],
                                         temp, id->dateFormat));
        break;
    case 152:
        strcpy (store, sensorGetLowDate(&sensors->sensor[STF_MONTH][SENSOR_BP],
                                        temp, id->dateFormat));
        break;
    case 153:
        strcpy (store, sensorGetHighDate(&sensors->sensor[STF_MONTH][SENSOR_WSPEED],
                                         temp, id->dateFormat));
        break;
    case 154:
        strcpy (store, sensorGetHighDate(&sensors->sensor[STF_MONTH][SENSOR_WGUST],
                                         temp, id->dateFormat));
        break;
    case 155:
        strcpy (store, sensorGetHighDate(&sensors->sensor[STF_MONTH][SENSOR_RAINRATE],
                                         temp, id->dateFormat));
        break;
    case 156:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f km",
                     wvutilsConvertMilesToKilometers(sensorGetWindRun(STF_YEAR, &sensors->sensor[STF_YEAR][SENSOR_WSPEED])));
        }
        else
        {
            sprintf (store, "%.1f miles", sensorGetWindRun(STF_YEAR, &sensors->sensor[STF_YEAR][SENSOR_WSPEED]));
        }
        break;
    case 157:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(sensorGetAvg(&sensors->sensor[STF_YEAR][SENSOR_OUTTEMP])));
        }
        else
        {
            sprintf (store, "%.1f", sensorGetAvg(&sensors->sensor[STF_YEAR][SENSOR_OUTTEMP]));
        }
        break;
    case 158:
        sprintf (store, 
                 "%.1f",
                 wvutilsGetWindSpeed(sensorGetAvg(&sensors->sensor[STF_YEAR][SENSOR_WSPEED])));
        break;
    case 159:
        sprintf (store, "%d", windAverageCompute(&sensors->wind[STF_YEAR]));
        break;
    case 160:
        sprintf (store, "%.0f", sensorGetAvg(&sensors->sensor[STF_YEAR][SENSOR_OUTHUMID]));
        break;
    case 161:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(sensorGetAvg(&sensors->sensor[STF_YEAR][SENSOR_DEWPOINT])));
        }
        else
        {
            sprintf (store, "%.1f", sensorGetAvg(&sensors->sensor[STF_YEAR][SENSOR_DEWPOINT]));
        }
        break;
    case 162:
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertINHGToHPA(sensorGetAvg(&sensors->sensor[STF_YEAR][SENSOR_BP])));
        }
        else
        {
            sprintf (store, "%.3f",
                     sensorGetAvg(&sensors->sensor[STF_YEAR][SENSOR_BP]));
        }
        break;
    case 163:
        strcpy (store, sensorGetHighDate(&sensors->sensor[STF_YEAR][SENSOR_OUTTEMP],
                                         temp, id->dateFormat));
        break;
    case 164:
        strcpy (store, sensorGetLowDate(&sensors->sensor[STF_YEAR][SENSOR_OUTTEMP],
                                        temp, id->dateFormat));
        break;
    case 165:
        strcpy (store, sensorGetLowDate(&sensors->sensor[STF_YEAR][SENSOR_WCHILL],
                                        temp, id->dateFormat));
        break;
    case 166:
        strcpy (store, sensorGetHighDate(&sensors->sensor[STF_YEAR][SENSOR_HINDEX],
                                         temp, id->dateFormat));
        break;
    case 167:
        strcpy (store, sensorGetHighDate(&sensors->sensor[STF_YEAR][SENSOR_OUTHUMID],
                                         temp, id->dateFormat));
        break;
    case 168:
        strcpy (store, sensorGetLowDate(&sensors->sensor[STF_YEAR][SENSOR_OUTHUMID],
                                        temp, id->dateFormat));
        break;
    case 169:
        strcpy (store, sensorGetHighDate(&sensors->sensor[STF_YEAR][SENSOR_DEWPOINT],
                                         temp, id->dateFormat));
        break;
    case 170:
        strcpy (store, sensorGetLowDate(&sensors->sensor[STF_YEAR][SENSOR_DEWPOINT],
                                        temp, id->dateFormat));
        break;
    case 171:
        strcpy (store, sensorGetHighDate(&sensors->sensor[STF_YEAR][SENSOR_BP],
                                         temp, id->dateFormat));
        break;
    case 172:
        strcpy (store, sensorGetLowDate(&sensors->sensor[STF_YEAR][SENSOR_BP],
                                        temp, id->dateFormat));
        break;
    case 173:
        strcpy (store, sensorGetHighDate(&sensors->sensor[STF_YEAR][SENSOR_WSPEED],
                                         temp, id->dateFormat));
        break;
    case 174:
        strcpy (store, sensorGetHighDate(&sensors->sensor[STF_YEAR][SENSOR_WGUST],
                                         temp, id->dateFormat));
        break;
    case 175:
        strcpy (store, sensorGetHighDate(&sensors->sensor[STF_YEAR][SENSOR_RAINRATE],
                                         temp, id->dateFormat));
        break;
    case 176:
        sprintf (store, "%d", id->loopStore.windDir);
        break;
    case 181:
        sprintf (store, "%.0f", sensorGetDailyHigh(sensors->sensor, SENSOR_SOLRAD));
        break;
    case 182:
        strcpy (store, sensorGetDailyHighTime(sensors->sensor, SENSOR_SOLRAD, temp));
        break;
    case 183:
        sprintf (store, "%.0f", sensorGetHigh(&sensors->sensor[STF_MONTH][SENSOR_SOLRAD]));
        break;
    case 184:
        sprintf (store, "%.0f", sensorGetHigh(&sensors->sensor[STF_YEAR][SENSOR_SOLRAD]));
        break;
    case 185:
        sprintf (store, "%.1f", sensorGetDailyHigh(sensors->sensor, SENSOR_UV));
        break;
    case 186:
        strcpy (store, sensorGetDailyHighTime(sensors->sensor, SENSOR_UV, temp));
        break;
    case 187:
        sprintf (store, "%.1f", sensorGetHigh(&sensors->sensor[STF_MONTH][SENSOR_UV]));
        break;
    case 188:
        sprintf (store, "%.1f", sensorGetHigh(&sensors->sensor[STF_YEAR][SENSOR_UV]));
        break;
    case 189:
        sprintf (store, "%s", lunarPhaseGet (id->mphaseIncrease,
                                             id->mphaseDecrease,
                                             id->mphaseFull));
        break;
    case 190:
        sprintf (store, " kg/m^3");
        break;
    case 191:
        sprintf (store, "%.3f",
                 wvutilsCalculateAirDensity (id->loopStore.outTemp,
                                             id->loopStore.barometer,
                                             id->loopStore.dewpoint));
        break;
    case 192:
        if (id->isMetricUnits)
            strcpy (store, " m");
        else
            strcpy (store, " ft");
        break;
    case 193:
        tempfloat = id->loopStore.outTemp;
        tempfloat -= id->loopStore.dewpoint;
        tempfloat *= 228;
        if (id->isMetricUnits)
        {
            tempfloat = wvutilsConvertFeetToMeters (tempfloat);
        }
        sprintf (store, "%.0f", tempfloat);
        break;
    case 194:
        sprintf (store, "%.0f", (float)id->loopStore.soilMoist1);
        break;
    case 195:
        sprintf (store, "%.0f", (float)id->loopStore.soilMoist2);
        break;
    case 196:
        sprintf (store, "%.0f", (float)id->loopStore.leafWet1);
        break;
    case 197:
        sprintf (store, "%.0f", (float)id->loopStore.leafWet2);
        break;
    case 198:
        // day high wind direction
        tempfloat = sensorGetDailyWhenHigh(sensors->sensor, SENSOR_WGUST) + 11.24;
        tempfloat /= 22.5;
        tempInt = (int)tempfloat;
        tempInt %= 16;
        sprintf (store, "%s", buildWindDirString(tempInt));
        break;
    case 199:
        // baromtrend
        sprintf (store, "%c", BPTrendLabels[id->baromTrendIndicator]);
        break;
    case 200:
        // dailyRainMM
        sprintf (store, "%.1f",
                 wvutilsConvertINToMM(sensorGetCumulative(&id->hilowStore.sensor[STF_DAY][SENSOR_RAIN])));
        break;
    case 201:
        // stationDateMetric
        ntime = time (NULL);
        localtime_r (&ntime, &loctime);
        sprintf (store, "%4.4d%2.2d%2.2d",
                 loctime.tm_year+1900,
                 loctime.tm_mon+1,
                 loctime.tm_mday);
        break;
    case 202:
        // middayTime
        strcpy (store, buildTimeTag (id->midday));
        break;
    case 203:
        // dayLength
        strcpy (store, buildTimeTag (id->dayLength));
        break;
    case 204:
        // civilriseTime
        strcpy (store, buildTimeTag (id->civilrise));
        break;
    case 205:
        // civilsetTime
        strcpy (store, buildTimeTag (id->civilset));
        break;
    case 206:
        // astroriseTime
        strcpy (store, buildTimeTag (id->astrorise));
        break;
    case 207:
        // astrosetTime
        strcpy (store, buildTimeTag (id->astroset));
        break;
    case 208:
        // stormStart
        if (id->loopStore.stormStart == (time_t)0)
        {
            strcpy (store, "-------- -----");
            break;
        }
        else
        {
            time_t Time = (time_t)id->loopStore.stormStart;
            localtime_r (&Time, &loctime);
            strftime(store, 64, id->dateFormat, &loctime);
            tempInt = strlen(store);
            snprintf(&store[tempInt], 64, " %2.2d:%2.2d",
                     loctime.tm_hour, loctime.tm_min);
            break;
        }
    case 209:
        // forecastIconFile
        tempInt = id->loopStore.forecastIcon;
        if (tempInt > 0 &&
                tempInt <= VP_FCAST_ICON_MAX &&
                id->ForecastIconFile[tempInt] != NULL)
        {
            sprintf (store, "%s", id->ForecastIconFile[tempInt]);
        }
        else
            store[0] = 0;
        break;
    case 210:
        // rainSeasonStart (month)
        sprintf (store, "%s", monthLabel[id->loopStore.yearRainMonth]);
        break;
    case 211:
        // intervalAvgWindChill
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(id->loopStore.intervalAvgWCHILL));
        }
        else
        {
            sprintf (store, "%.1f", id->loopStore.intervalAvgWCHILL);
        }
        break;
    case 212:
        // intervalAvgWindSpeed
        sprintf (store, 
                 "%.1f",
                 wvutilsGetWindSpeed(id->loopStore.intervalAvgWSPEEDF));
        break;
    case 213:
        // stationPressure
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertINHGToHPA(id->loopStore.stationPressure));
        }
        else
        {
            sprintf (store, "%.3f",
                     id->loopStore.stationPressure);
        }
        break;
    case 214:
        // altimeter
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertINHGToHPA(id->loopStore.altimeter));
        }
        else
        {
            sprintf (store, "%.3f",
                     id->loopStore.altimeter);
        }
        break;

    case 215:
        // localRadarURL
        wvstrncpy (store, id->radarURL, HTML_MAX_LINE_LENGTH);
        break;

    case 216:
        // localForecastURL
        wvstrncpy (store, id->forecastURL, HTML_MAX_LINE_LENGTH);
        break;

    case 217:
        // windGustSpeed - "Current" wind gust speed
        tempfloat = sensorGetHigh(&sensors->sensor[STF_INTERVAL][SENSOR_WGUST]);
        if (tempfloat <= 0)
        {
            tempfloat = 0;
        }
        sprintf (store, 
                 "%.1f",
                 wvutilsGetWindSpeed(tempfloat));
        break;

    case 218:
        // windGustDirectionDegrees - "Current" wind gust direction
        sprintf (store, "%.0f", sensorGetWhenHigh(&sensors->sensor[STF_INTERVAL][SENSOR_WGUST]));
        break;

    case 219:
        // windBeaufortScale
        sprintf (store, "%s", wvutilsConvertToBeaufortScale((int)id->loopStore.windSpeedF));
        break;

    case 220:
        // intervalAvgBeaufortScale
        sprintf (store, "%s", wvutilsConvertToBeaufortScale((int)id->loopStore.intervalAvgWSPEEDF));
        break;

    case 221:
        // station type
        strcpy (store, id->stationType);
        break;

    case 222:
        //    "<!--wxt510Hail-->"
        if (id->isMetricUnits)
        {
            sprintf (store, "%.3f",
                     wvutilsConvertRainINToMetric((float)id->loopStore.wxt510Hail));
        }
        else
        {
            sprintf (store, "%.2f",id->loopStore.wxt510Hail);
        }
        break;

    case 223:
        //    "<!--wxt510Hailrate-->"
        if (id->isMetricUnits)
        {
            sprintf (store, "%.3f",
                     wvutilsConvertRainINToMetric((float)id->loopStore.wxt510Hailrate));
        }
        else
        {
            sprintf (store, "%.2f",id->loopStore.wxt510Hailrate);
        }
        break;

    case 224:
        //    "<!--wxt510HeatingTemp-->"
        if (id->isMetricUnits)
        {
            tempfloat = (float)(id->loopStore.wxt510HeatingTemp);
            if (tempfloat != 0)
            {
                sprintf (store, "%.1f",
                         wvutilsConvertFToC(tempfloat));
            }
            else
            {
                sprintf (store, "%.1f", 0.0);
            }
        }
        else
        {
            sprintf (store, "%.1f", id->loopStore.wxt510HeatingTemp);
        }
        break;

    case 225:
        //    "<!--wxt510HeatingVoltage-->"
        sprintf (store, "%.1f",id->loopStore.wxt510HeatingVoltage);
        break;

    case 226:
        //    "<!--wxt510SupplyVoltage-->"
        sprintf (store, "%.1f",id->loopStore.wxt510SupplyVoltage);
        break;

    case 227:
        //    "<!--wxt510ReferenceVoltage-->"
        sprintf (store, "%.3f",id->loopStore.wxt510ReferenceVoltage);
        break;

    case 228:
        //    ""<!--wxt510RainDuration-->"
        wvstrncpy (store, 
                   makeduration(id->loopStore.wxt510RainDuration),
                   HTML_MAX_LINE_LENGTH);
        break;

    case 229:
        //    ""<!--wxt510RainPeakRate-->"
        if (id->isMetricUnits)
        {
            sprintf (store, "%.3f",
                     wvutilsConvertRainINToMetric((float)id->loopStore.wxt510RainPeakRate));
        }
        else
        {
            sprintf (store, "%.2f",id->loopStore.wxt510RainPeakRate);
        }
        break;

    case 230:
        //    "<!--wxt510HailDuration-->"
        wvstrncpy (store, 
                   makeduration(id->loopStore.wxt510HailDuration),
                   HTML_MAX_LINE_LENGTH);
        break;

    case 231:
        //    "<!--wxt510HailPeakRate-->"
        if (id->isMetricUnits)
        {
            sprintf (store, "%.3f",
                     wvutilsConvertRainINToMetric((float)id->loopStore.wxt510HailPeakRate));
        }
        else
        {
            sprintf (store, "%.2f",id->loopStore.wxt510HailPeakRate);
        }
        break;

    case 232:
        //    "<!--wxt510Rain-->"
        if (id->isMetricUnits)
        {
            sprintf (store, "%.3f",
                     wvutilsConvertRainINToMetric((float)id->loopStore.wxt510Rain));
        }
        else
        {
            sprintf (store, "%.3f",id->loopStore.wxt510Rain);
        }
        break;

    case 233:
        // "<!--rxCheckPercent-->", uint16_t              rxCheckPercent;          0 - 100
        sprintf (store, "%.1f", (float)id->loopStore.rxCheckPercent);
        break;

    case 234:
        // "<!--tenMinuteAvgWindSpeed-->", uint16_t              tenMinuteAvgWindSpeed;  mph
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertMPHToKPH((float)id->loopStore.tenMinuteAvgWindSpeed));
        }
        else
        {
            sprintf (store, "%d", id->loopStore.tenMinuteAvgWindSpeed);
        }
        break;

    case 237:
        // "<!--txBatteryStatus-->",  uint16_t              txBatteryStatus;         VP only
        sprintf (store, "%2.2x", id->loopStore.txBatteryStatus);
        break;

    case 238:
        // "<!--consBatteryVoltage-->",   uint16_t              consBatteryVoltage;   VP only
        tempfloat = (((float)id->loopStore.consBatteryVoltage * 300)/512)/100;
        sprintf (store, "%.2f", tempfloat);
        break;

    case 239:
        ntime = time (NULL);
        // "<!--stationTimeNoSecs-->",
        localtime_r (&ntime, &loctime);
        sprintf (store, "%2.2d:%2.2d",
                 loctime.tm_hour,
                 loctime.tm_min);
        break;

    case 240:
        //    "<!--wxt510RainDurationMin-->",
        tempInt = ((id->loopStore.wxt510RainDuration)/60);
        sprintf (store, "%d", tempInt);
        break;

    case 241:
        //    "<!--wxt510HailDurationMin-->",
        tempInt = ((id->loopStore.wxt510HailDuration)/60);
        sprintf (store, "%d", tempInt);
        break;


    case 242:   // case 222:
        // wmr918WindBatteryStatus
        sprintf (store, "%d", (int)id->loopStore.wmr918WindBatteryStatus);
        break;

    case 243:
        // wmr918RainBatteryStatus
        sprintf (store, "%d", (int)id->loopStore.wmr918RainBatteryStatus);
        break;

    case 244:
        // wmr918OutTempBatteryStatus
        sprintf (store, "%d", (int)id->loopStore.wmr918OutTempBatteryStatus);
        break;

    case 245:
        // wmr918InTempBatteryStatus
        sprintf (store, "%d", (int)id->loopStore.wmr918InTempBatteryStatus);
        break;

        // ###### Begin EXTRA Wind #######################
    case 246:
        // "<!--windSpeed_ms-->"
        tempfloat = wvutilsConvertMPHToMPS (id->loopStore.windSpeedF);
        sprintf (store, "%.1f", tempfloat);
        break;

    case 247:
        //  "<!--windGustSpeed_ms-->"
        tempfloat = (float)sensorGetHigh(&sensors->sensor[STF_INTERVAL][SENSOR_WGUST]);
        if (tempfloat < 0)
        {
            tempfloat = id->loopStore.windSpeedF;
        }
        tempfloat = wvutilsConvertMPHToMPS (tempfloat);
        sprintf (store, "%.1f", tempfloat);
        break;

    case 248:
        // <!--intervalAvgWindSpeed_ms-->
        tempfloat = wvutilsConvertMPHToMPS (id->loopStore.intervalAvgWSPEEDF);
        sprintf (store, "%.1f", tempfloat);
        break;

    case 249:
        // "<!--tenMinuteAvgWindSpeed_ms-->"
        tempfloat = wvutilsConvertMPHToMPS ((float)id->loopStore.tenMinuteAvgWindSpeed);
        sprintf (store, "%.1f", tempfloat);
        break;

    case 250:
        // <!--hiDailyWindSpeed_ms-->
        tempfloat = wvutilsConvertMPHToMPS ((float)sensorGetDailyHigh(sensors->sensor, SENSOR_WGUST));
        sprintf (store, "%.1f", tempfloat);
        break;

    case 251:
        // <!--hiMonthlyWindSpeed_ms-->
        tempfloat = wvutilsConvertMPHToMPS ((float)sensorGetHigh(&sensors->sensor[STF_MONTH][SENSOR_WGUST]));
        sprintf (store, "%.1f", tempfloat);
        break;

    case 252:
        // <!--hiYearlyWindSpeed_ms-->
        tempfloat = wvutilsConvertMPHToMPS ((float)sensorGetHigh(&sensors->sensor[STF_YEAR][SENSOR_WGUST]));
        sprintf (store, "%.1f", tempfloat);
        break;

    case 253:
        // <!--houravgwind_ms-->
        tempfloat = wvutilsConvertMPHToMPS ((float)sensorGetAvg(&sensors->sensor[STF_HOUR][SENSOR_WSPEED]));
        sprintf (store, "%.1f", tempfloat);
        break;

    case 254:
        // <!--dayavgwind_ms-->
        tempfloat = wvutilsConvertMPHToMPS ((float)sensorGetAvg(&sensors->sensor[STF_DAY][SENSOR_WSPEED]));
        sprintf (store, "%.1f", tempfloat);
        break;

    case 255:
        // <!--weekavgwind_ms-->
        tempfloat = wvutilsConvertMPHToMPS ((float)sensorGetAvg(&sensors->sensor[STF_WEEK][SENSOR_WSPEED]));
        sprintf (store, "%.1f", tempfloat);
        break;

    case 256:
        // <!--monthtodateavgwind_ms-->
        tempfloat = wvutilsConvertMPHToMPS ((float)sensorGetAvg(&sensors->sensor[STF_MONTH][SENSOR_WSPEED]));
        sprintf (store, "%.1f", tempfloat);
        break;

    case 257:
        // <!--yeartodateavgwind_ms-->
        tempfloat = wvutilsConvertMPHToMPS ((float)sensorGetAvg(&sensors->sensor[STF_YEAR][SENSOR_WSPEED]));
        sprintf (store, "%.1f", tempfloat);
        break;

    case 258:
        // <!--hourchangewind_ms-->
        tempfloat = wvutilsConvertMPHToMPS ((float)id->hilowStore.hourchangewind);
        sprintf (store, "%.1f", tempfloat);
        break;

    case 259:
        // <!--daychangewind_ms-->
        tempfloat = wvutilsConvertMPHToMPS ((float)id->hilowStore.daychangewind);
        sprintf (store, "%.1f", tempfloat);
        break;

    case 260:
        // <!--weekchangewind_ms-->
        tempfloat = wvutilsConvertMPHToMPS ((float)id->hilowStore.weekchangewind * 0.447027);
        sprintf (store, "%.1f", tempfloat);
        break;

    case 261:
        // "<!--windSpeed_kts-->",
        tempfloat = wvutilsConvertMPHToKnots (id->loopStore.windSpeedF);
        sprintf (store, "%.1f", tempfloat);
        break;

    case 262:
        //  "<!--windGustSpeed_kts-->",
        tempfloat = sensorGetHigh(&sensors->sensor[STF_INTERVAL][SENSOR_WGUST]);
        if (tempfloat < 0)
        {
            tempfloat = (id->loopStore.windSpeedF);
        }
        tempfloat = wvutilsConvertMPHToKnots (tempfloat);
        sprintf (store, "%.1f", tempfloat);
        break;

    case 263:
        // <!--intervalAvgWindSpeed_kts-->
        tempfloat = wvutilsConvertMPHToKnots (id->loopStore.intervalAvgWSPEEDF);
        sprintf (store, "%.1f", tempfloat);
        break;

    case 264:
        // "<!--tenMinuteAvgWindSpeed_kts-->"
        tempfloat = wvutilsConvertMPHToKnots ((float)(id->loopStore.tenMinuteAvgWindSpeed));
        sprintf (store, "%.1f", tempfloat);
        break;

    case 265:
        // <!--hiWindSpeed_kts-->
        tempfloat = wvutilsConvertMPHToKnots ((float)sensorGetDailyHigh(sensors->sensor, SENSOR_WGUST));
        sprintf (store, "%.1f", tempfloat);
        break;

    case 266:
        // <!--hiMonthlyWindSpeed_kts-->
        tempfloat = wvutilsConvertMPHToKnots ((float)sensorGetHigh(&sensors->sensor[STF_MONTH][SENSOR_WGUST]));
        sprintf (store, "%.1f", tempfloat);
        break;

    case 267:
        // <!--hiYearlyWindSpeed_kts-->
        tempfloat = wvutilsConvertMPHToKnots ((float)sensorGetHigh(&sensors->sensor[STF_YEAR][SENSOR_WGUST]));
        sprintf (store, "%.1f", tempfloat);
        break;

    case 268:
        // <!--houravgwind_kts-->
        tempfloat = wvutilsConvertMPHToKnots ((float)sensorGetAvg(&sensors->sensor[STF_HOUR][SENSOR_WSPEED]));
        sprintf (store, "%.1f", tempfloat);
        break;

    case 269:
        // <!--dayavgwind_kts-->
        tempfloat = wvutilsConvertMPHToKnots ((float)sensorGetAvg(&sensors->sensor[STF_DAY][SENSOR_WSPEED]));
        sprintf (store, "%.1f", tempfloat);
        break;

    case 270:
        // <!--weekavgwind_kts-->
        tempfloat = wvutilsConvertMPHToKnots ((float)sensorGetAvg(&sensors->sensor[STF_WEEK][SENSOR_WSPEED]));
        sprintf (store, "%.1f", tempfloat);
        break;

    case 271:
        // <!--monthtodateavgwind_kts-->
        tempfloat = wvutilsConvertMPHToKnots ((float)sensorGetAvg(&sensors->sensor[STF_MONTH][SENSOR_WSPEED]));
        sprintf (store, "%.1f", tempfloat);
        break;

    case 272:
        // <!--yeartodateavgwind_kts-->
        tempfloat = wvutilsConvertMPHToKnots ((float)sensorGetAvg(&sensors->sensor[STF_YEAR][SENSOR_WSPEED]));
        sprintf (store, "%.1f", tempfloat);
        break;

    case 273:
        // <!--hourchangewind_kts-->
        tempfloat = wvutilsConvertMPHToKnots ((float)(id->hilowStore.hourchangewind));
        sprintf (store, "%.1f", tempfloat);
        break;

    case 274:
        // <!--daychangewind_kts-->
        tempfloat = wvutilsConvertMPHToKnots ((float)(id->hilowStore.daychangewind));
        sprintf (store, "%.1f", tempfloat);
        break;

    case 275:
        // <!--weekchangewind_kts-->
        tempfloat = wvutilsConvertMPHToKnots ((float)(id->hilowStore.weekchangewind));
        sprintf (store, "%.1f", tempfloat);
        break;

    case 276:
        // "<!--wmr918Humid3-->",
        sprintf (store, "%d", (int)id->loopStore.wmr918Humid3);
        break;
    case 277:
        // "<!--wmr918Pool-->",
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f", wvutilsConvertFToC((float)id->loopStore.wmr918Pool));
        }
        else
        {
            sprintf (store, "%.0f", (float)id->loopStore.wmr918Pool);
        }
        break;
    case 278:
        // "<!--wmr918poolTempBatteryStatus-->",
        sprintf (store, "%d", (int)id->loopStore.wmr918poolTempBatteryStatus);
        break;
    case 279:
        // "<!--wmr918extra1BatteryStatus-->",
        sprintf (store, "%d", (int)id->loopStore.wmr918extra1BatteryStatus);
        break;
    case 280:
        // "<!--wmr918extra2BatteryStatus-->",
        sprintf (store, "%d", (int)id->loopStore.wmr918extra2BatteryStatus);
        break;
    case 281:
        // "<!--wmr918extra3BatteryStatus-->",
        sprintf (store, "%d", (int)id->loopStore.wmr918extra3BatteryStatus);
        break;
    case 282:
        // "<!--hiAllTimeOutsideTemp-->",
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(sensorGetHigh(&sensors->sensor[STF_ALL][SENSOR_OUTTEMP])));
        }
        else
        {
            sprintf (store, "%.1f", sensorGetHigh(&sensors->sensor[STF_ALL][SENSOR_OUTTEMP]));
        }
        break;
    case 283:
        // "<!--lowAllTimeOutsideTemp-->",
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(sensorGetLow(&sensors->sensor[STF_ALL][SENSOR_OUTTEMP])));
        }
        else
        {
            sprintf (store, "%.1f", sensorGetLow(&sensors->sensor[STF_ALL][SENSOR_OUTTEMP]));
        }
        break;
    case 284:
        // "<!--hiAllTimeHumidity-->",
        sprintf (store, "%d", (int)sensorGetHigh(&sensors->sensor[STF_ALL][SENSOR_OUTHUMID]));
        break;
    case 285:
        // "<!--lowAllTimeHumidity-->",
        sprintf (store, "%d", (int)sensorGetLow(&sensors->sensor[STF_ALL][SENSOR_OUTHUMID]));
        break;
    case 286:
        // "<!--hiAllTimeDewpoint-->",
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(sensorGetHigh(&sensors->sensor[STF_ALL][SENSOR_DEWPOINT])));
        }
        else
        {
            sprintf (store, "%.1f", sensorGetHigh(&sensors->sensor[STF_ALL][SENSOR_DEWPOINT]));
        }
        break;
    case 287:
        // "<!--lowAllTimeDewpoint-->",
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(sensorGetLow(&sensors->sensor[STF_ALL][SENSOR_DEWPOINT])));
        }
        else
        {
            sprintf (store, "%.1f", sensorGetLow(&sensors->sensor[STF_ALL][SENSOR_DEWPOINT]));
        }
        break;
    case 288:
        // "<!--hiAllTimeWindSpeed-->",
        sprintf (store, 
                 "%.1f",
                 wvutilsGetWindSpeed(sensorGetHigh(&sensors->sensor[STF_ALL][SENSOR_WGUST])));
        break;
    case 289:
        // "<!--hiAllTimeBarometer-->",
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertINHGToHPA(sensorGetHigh(&sensors->sensor[STF_ALL][SENSOR_BP])));
        }
        else
        {
            sprintf (store, "%.3f",
                     sensorGetHigh(&sensors->sensor[STF_ALL][SENSOR_BP]));
        }
        break;
    case 290:
        // "<!--lowAllTimeBarometer-->",
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertINHGToHPA(sensorGetLow(&sensors->sensor[STF_ALL][SENSOR_BP])));
        }
        else
        {
            sprintf (store, "%.3f",
                     sensorGetLow(&sensors->sensor[STF_ALL][SENSOR_BP]));
        }
        break;
    case 291:
        // "<!--lowAllTimeWindchill-->",
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(sensorGetLow(&sensors->sensor[STF_ALL][SENSOR_WCHILL])));
        }
        else
        {
            sprintf (store, "%.1f", sensorGetLow(&sensors->sensor[STF_ALL][SENSOR_WCHILL]));
        }
        break;
    case 292:
        // "<!--hiAllTimeHeatindex-->",
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(sensorGetHigh(&sensors->sensor[STF_ALL][SENSOR_HINDEX])));
        }
        else
        {
            sprintf (store, "%.1f", sensorGetHigh(&sensors->sensor[STF_ALL][SENSOR_HINDEX]));
        }
        break;
    case 293:
        // "<!--hiAllTimeRainRate-->",
        if (id->isMetricUnits)
        {
            sprintf (store, "%.2f",
                     wvutilsConvertRainINToMetric(sensorGetHigh(&sensors->sensor[STF_ALL][SENSOR_RAINRATE])));
        }
        else
        {
            sprintf (store, "%.2f", sensorGetHigh(&sensors->sensor[STF_ALL][SENSOR_RAINRATE]));
        }
        break;
    case 294:
        // "<!--hiAllTimeRadiation-->",
        sprintf (store, "%.0f", sensorGetHigh(&sensors->sensor[STF_ALL][SENSOR_SOLRAD]));
        break;
    case 295:
        // "<!--hiAllTimeUV-->",
        sprintf (store, "%.1f", sensorGetHigh(&sensors->sensor[STF_ALL][SENSOR_UV]));
        break;
    case 296:
        // "<!--alltimeavgtemp-->",
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(sensorGetAvg(&sensors->sensor[STF_ALL][SENSOR_OUTTEMP])));
        }
        else
        {
            sprintf (store, "%.1f", sensorGetAvg(&sensors->sensor[STF_ALL][SENSOR_OUTTEMP]));
        }
        break;
    case 297:
        // "<!--alltimeavgwind-->",
        sprintf (store, 
                 "%.1f",
                 wvutilsGetWindSpeed(sensorGetAvg(&sensors->sensor[STF_ALL][SENSOR_WSPEED])));
        break;
    case 298:
        // "<!--alltimedomwinddir-->",
        sprintf (store, "%d", windAverageCompute(&sensors->wind[STF_ALL]));
        break;
    case 299:
        // "<!--alltimeavghumid-->",
        sprintf (store, "%.0f", sensorGetAvg(&sensors->sensor[STF_ALL][SENSOR_OUTHUMID]));
        break;
    case 300:
        // "<!--alltimeavgdewpt-->",
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(sensorGetAvg(&sensors->sensor[STF_ALL][SENSOR_DEWPOINT])));
        }
        else
        {
            sprintf (store, "%.1f", sensorGetAvg(&sensors->sensor[STF_ALL][SENSOR_DEWPOINT]));
        }
        break;
    case 301:
        // "<!--alltimeavgbarom-->",
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertINHGToHPA(sensorGetAvg(&sensors->sensor[STF_ALL][SENSOR_BP])));
        }
        else
        {
            sprintf (store, "%.3f",
                     sensorGetAvg(&sensors->sensor[STF_ALL][SENSOR_BP]));
        }
        break;
    case 302:
        // "<!--alltimemaxtempdate-->",
        strcpy (store, sensorGetHighDate(&sensors->sensor[STF_ALL][SENSOR_OUTTEMP],
                                         temp, id->dateFormat));
        break;
    case 303:
        // "<!--alltimemintempdate-->",
        strcpy (store, sensorGetLowDate(&sensors->sensor[STF_ALL][SENSOR_OUTTEMP],
                                        temp, id->dateFormat));
        break;
    case 304:
        // "<!--alltimeminchilldate-->",
        strcpy (store, sensorGetLowDate(&sensors->sensor[STF_ALL][SENSOR_WCHILL],
                                        temp, id->dateFormat));
        break;
    case 305:
        // "<!--alltimemaxheatdate-->",
        strcpy (store, sensorGetHighDate(&sensors->sensor[STF_ALL][SENSOR_HINDEX],
                                         temp, id->dateFormat));
        break;
    case 306:
        // "<!--alltimemaxhumiddate-->",
        strcpy (store, sensorGetHighDate(&sensors->sensor[STF_ALL][SENSOR_OUTHUMID],
                                         temp, id->dateFormat));
        break;
    case 307:
        // "<!--alltimeminhumiddate-->",
        strcpy (store, sensorGetLowDate(&sensors->sensor[STF_ALL][SENSOR_OUTHUMID],
                                        temp, id->dateFormat));
        break;
    case 308:
        // "<!--alltimemaxdewptdate-->",
        strcpy (store, sensorGetHighDate(&sensors->sensor[STF_ALL][SENSOR_DEWPOINT],
                                         temp, id->dateFormat));
        break;
    case 309:
        // "<!--alltimemindewptdate-->",
        strcpy (store, sensorGetLowDate(&sensors->sensor[STF_ALL][SENSOR_DEWPOINT],
                                        temp, id->dateFormat));
        break;
    case 310:
        // "<!--alltimemaxbaromdate-->",
        strcpy (store, sensorGetHighDate(&sensors->sensor[STF_ALL][SENSOR_BP],
                                         temp, id->dateFormat));
        break;
    case 311:
        // "<!--alltimeminbaromdate-->",
        strcpy (store, sensorGetLowDate(&sensors->sensor[STF_ALL][SENSOR_BP],
                                        temp, id->dateFormat));
        break;
    case 312:
        // "<!--alltimemaxwinddate-->",
        strcpy (store, sensorGetHighDate(&sensors->sensor[STF_ALL][SENSOR_WSPEED],
                                         temp, id->dateFormat));
        break;
    case 313:
        // "<!--alltimemaxgustdate-->",
        strcpy (store, sensorGetHighDate(&sensors->sensor[STF_ALL][SENSOR_WGUST],
                                         temp, id->dateFormat));
        break;
    case 314:
        // "<!--alltimemaxrainratedate-->",
        strcpy (store, sensorGetHighDate(&sensors->sensor[STF_ALL][SENSOR_RAINRATE],
                                         temp, id->dateFormat));
        break;
    case 315:
        // "<!--wmr918Tendency-->",
        sprintf (store, "%s", buildTendencyString((int)id->loopStore.wmr918Tendency));
        break;
    case 316:
        // "<!--stationName-->",
        wvstrncpy (store, id->stationName, HTML_MAX_LINE_LENGTH);
        break;
    case 317:
        // "<!--moonriseTime-->",
        if (id->moonrise == -2 && id->moonset == -2)
            sprintf (store, "Down all day");
        else if (id->moonrise == -1 && id->moonset == -1)
            sprintf (store, "Up all day");
        else if (id->moonrise  > 0)
            sprintf (store , "%2.2d:%2.2d", id->moonrise/100, id->moonrise%100);
        else
            sprintf (store, "--:--"); // No Moon Rise
        break;
    case 318:
        // "<!--moonsetTime-->",
        if (id->moonrise == -2 && id->moonset == -2)
            sprintf (store, "Down all day");
        else if (id->moonrise == -1 && id->moonset == -1)
            sprintf (store, "Up all day");
        else if (id->moonset  > 0)
            sprintf (store,  "%2.2d:%2.2d", id->moonset/100, id->moonset%100);
        else
            sprintf (store, "--:--"); // No Moon Set
        break;
    case 319:
       // "<!--apparentTemp-->",
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(wvutilsCalculateApparentTemp(id->loopStore.outTemp,
                                                                     id->loopStore.windSpeedF,
                                                                     id->loopStore.outHumidity)));

        }
        else
        {
            sprintf (store, "%.1f",
                     wvutilsCalculateApparentTemp(id->loopStore.outTemp, id->loopStore.windSpeedF,
                                                  id->loopStore.outHumidity));
        }
        break;
    case 320:
       // "<!--genExtraTemp1-->",
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f", wvutilsConvertFToC(id->loopStore.extraTemp[0]));

        }
        else
        {
            sprintf (store, "%.1f", id->loopStore.extraTemp[0]);
        }
        break;
    case 321:
       // "<!--genExtraTemp2-->",
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f", wvutilsConvertFToC(id->loopStore.extraTemp[1]));

        }
        else
        {
            sprintf (store, "%.1f", id->loopStore.extraTemp[1]);
        }
        break;
    case 322:
       // "<!--genExtraTemp3-->",
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f", wvutilsConvertFToC(id->loopStore.extraTemp[2]));

        }
        else
        {
            sprintf (store, "%.1f", id->loopStore.extraTemp[2]);
        }
        break;
    case 323:
       // "<!--genExtraTemp4-->",
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f", wvutilsConvertFToC(id->loopStore.extraTemp[3]));

        }
        else
        {
            sprintf (store, "%.1f", id->loopStore.extraTemp[3]);
        }
        break;
    case 324:
       // "<!--genExtraTemp5-->",
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f", wvutilsConvertFToC(id->loopStore.extraTemp[4]));

        }
        else
        {
            sprintf (store, "%.1f", id->loopStore.extraTemp[4]);
        }
        break;
    case 325:
       // "<!--genExtraTemp6-->",
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f", wvutilsConvertFToC(id->loopStore.extraTemp[5]));

        }
        else
        {
            sprintf (store, "%.1f", id->loopStore.extraTemp[5]);
        }
        break;
    case 326:
       // "<!--genExtraTemp7-->",
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f", wvutilsConvertFToC(id->loopStore.extraTemp[6]));

        }
        else
        {
            sprintf (store, "%.1f", id->loopStore.extraTemp[6]);
        }
        break;
    case 327:
       // "<!--genExtraTemp8-->",
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f", wvutilsConvertFToC(id->loopStore.extraTemp[7]));

        }
        else
        {
            sprintf (store, "%.1f", id->loopStore.extraTemp[7]);
        }
        break;
    case 328:
       // "<!--genExtraTemp9-->",
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f", wvutilsConvertFToC(id->loopStore.extraTemp[8]));

        }
        else
        {
            sprintf (store, "%.1f", id->loopStore.extraTemp[8]);
        }
        break;
    case 329:
       // "<!--genExtraTemp10-->",
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f", wvutilsConvertFToC(id->loopStore.extraTemp[9]));

        }
        else
        {
            sprintf (store, "%.1f", id->loopStore.extraTemp[9]);
        }
        break;
    case 330:
       // "<!--genExtraTemp11-->",
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f", wvutilsConvertFToC(id->loopStore.extraTemp[10]));

        }
        else
        {
            sprintf (store, "%.1f", id->loopStore.extraTemp[10]);
        }
        break;
    case 331:
       // "<!--genExtraTemp12-->",
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f", wvutilsConvertFToC(id->loopStore.extraTemp[11]));

        }
        else
        {
            sprintf (store, "%.1f", id->loopStore.extraTemp[11]);
        }
        break;
    case 332:
       // "<!--genExtraTemp13-->",
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f", wvutilsConvertFToC(id->loopStore.extraTemp[12]));

        }
        else
        {
            sprintf (store, "%.1f", id->loopStore.extraTemp[12]);
        }
        break;
    case 333:
       // "<!--genExtraTemp14-->",
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f", wvutilsConvertFToC(id->loopStore.extraTemp[13]));

        }
        else
        {
            sprintf (store, "%.1f", id->loopStore.extraTemp[13]);
        }
        break;
    case 334:
       // "<!--genExtraTemp15-->",
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f", wvutilsConvertFToC(id->loopStore.extraTemp[14]));

        }
        else
        {
            sprintf (store, "%.1f", id->loopStore.extraTemp[14]);
        }
        break;
    case 335:
       // "<!--genExtraTemp16-->",
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f", wvutilsConvertFToC(id->loopStore.extraTemp[15]));

        }
        else
        {
            sprintf (store, "%.1f", id->loopStore.extraTemp[15]);
        }
        break;
    case 336:
        // "<!--genExtraHumidity1-->",
        sprintf (store, "%d", id->loopStore.extraHumidity[0]);
        break;
    case 337:
        // "<!--genExtraHumidity2-->",
        sprintf (store, "%d", id->loopStore.extraHumidity[1]);
        break;
    case 338:
        // "<!--genExtraHumidity3-->",
        sprintf (store, "%d", id->loopStore.extraHumidity[2]);
        break;
    case 339:
        // "<!--genExtraHumidity4-->",
        sprintf (store, "%d", id->loopStore.extraHumidity[3]);
        break;
    case 340:
        // "<!--genExtraHumidity5-->",
        sprintf (store, "%d", id->loopStore.extraHumidity[4]);
        break;
    case 341:
        // "<!--genExtraHumidity6-->",
        sprintf (store, "%d", id->loopStore.extraHumidity[5]);
        break;
    case 342:
        // "<!--genExtraHumidity7-->",
        sprintf (store, "%d", id->loopStore.extraHumidity[6]);
        break;
    case 343:
        // "<!--genExtraHumidity8-->",
        sprintf (store, "%d", id->loopStore.extraHumidity[7]);
        break;
    case 344:
        // "<!--genExtraHumidity9-->",
        sprintf (store, "%d", id->loopStore.extraHumidity[8]);
        break;
    case 345:
        // "<!--genExtraHumidity10-->",
        sprintf (store, "%d", id->loopStore.extraHumidity[9]);
        break;
    case 346:
        // "<!--genExtraHumidity11-->",
        sprintf (store, "%d", id->loopStore.extraHumidity[10]);
        break;
    case 347:
        // "<!--genExtraHumidity12-->",
        sprintf (store, "%d", id->loopStore.extraHumidity[11]);
        break;
    case 348:
        // "<!--genExtraHumidity13-->",
        sprintf (store, "%d", id->loopStore.extraHumidity[12]);
        break;
    case 349:
        // "<!--genExtraHumidity14-->",
        sprintf (store, "%d", id->loopStore.extraHumidity[13]);
        break;
    case 350:
        // "<!--genExtraHumidity15-->",
        sprintf (store, "%d", id->loopStore.extraHumidity[14]);
        break;
    case 351:
        // "<!--genExtraHumidity16-->",
        sprintf (store, "%d", id->loopStore.extraHumidity[15]);
        break;
    case 352:
        // "<!--genExtraTempBatteryStatus1-->",
        sprintf (store, "%s", getBattStatus(id->loopStore.extraTempBatteryStatus[0]));
        break;
    case 353:
        // "<!--genExtraTempBatteryStatus2-->",
        sprintf (store, "%s", getBattStatus(id->loopStore.extraTempBatteryStatus[1]));
        break;
    case 354:
        // "<!--genExtraTempBatteryStatus3-->",
        sprintf (store, "%s", getBattStatus(id->loopStore.extraTempBatteryStatus[2]));
        break;
    case 355:
        // "<!--genExtraTempBatteryStatus4-->",
        sprintf (store, "%s", getBattStatus(id->loopStore.extraTempBatteryStatus[3]));
        break;
    case 356:
        // "<!--genExtraTempBatteryStatus5-->",
        sprintf (store, "%s", getBattStatus(id->loopStore.extraTempBatteryStatus[4]));
        break;
    case 357:
        // "<!--genExtraTempBatteryStatus6-->",
        sprintf (store, "%s", getBattStatus(id->loopStore.extraTempBatteryStatus[5]));
        break;
    case 358:
        // "<!--genExtraTempBatteryStatus7-->",
        sprintf (store, "%s", getBattStatus(id->loopStore.extraTempBatteryStatus[6]));
        break;
    case 359:
        // "<!--genExtraTempBatteryStatus8-->",
        sprintf (store, "%s", getBattStatus(id->loopStore.extraTempBatteryStatus[7]));
        break;
    case 360:
        // "<!--genExtraTempBatteryStatus9-->",
        sprintf (store, "%s", getBattStatus(id->loopStore.extraTempBatteryStatus[8]));
        break;
    case 361:
        // "<!--genExtraTempBatteryStatus10-->",
        sprintf (store, "%s", getBattStatus(id->loopStore.extraTempBatteryStatus[9]));
        break;
    case 362:
        // "<!--genExtraTempBatteryStatus11-->",
        sprintf (store, "%s", getBattStatus(id->loopStore.extraTempBatteryStatus[10]));
        break;
    case 363:
        // "<!--genExtraTempBatteryStatus12-->",
        sprintf (store, "%s", getBattStatus(id->loopStore.extraTempBatteryStatus[11]));
        break;
    case 364:
        // "<!--genExtraTempBatteryStatus13-->",
        sprintf (store, "%s", getBattStatus(id->loopStore.extraTempBatteryStatus[12]));
        break;
    case 365:
        // "<!--genExtraTempBatteryStatus14-->",
        sprintf (store, "%s", getBattStatus(id->loopStore.extraTempBatteryStatus[13]));
        break;
    case 366:
        // "<!--genExtraTempBatteryStatus15-->",
        sprintf (store, "%s", getBattStatus(id->loopStore.extraTempBatteryStatus[14]));
        break;
    case 367:
        // "<!--genExtraTempBatteryStatus16-->",
        sprintf (store, "%s", getBattStatus(id->loopStore.extraTempBatteryStatus[15]));
        break;
    case 368:
        // "<!--genExtraWindBatteryStatus-->",
        sprintf (store, "%s", getBattStatus(id->loopStore.windBatteryStatus));
        break;
    case 369:
        // "<!--genExtraOutTempBatteryStatus-->",
        sprintf (store, "%s", getBattStatus(id->loopStore.outTempBatteryStatus));
        break;
    case 370:
        // "<!--genExtraConsoleBatteryStatus-->",
        sprintf (store, "%s", getBattStatus(id->loopStore.consoleBatteryStatus));
        break;
    case 371:
        // "<!--genExtraUVBatteryStatus-->",
        sprintf (store, "%s", getBattStatus(id->loopStore.uvBatteryStatus));
        break;
    case 372:
        // "<!--genExtraSolarBatteryStatus-->",
        sprintf (store, "%s", getBattStatus(id->loopStore.solarBatteryStatus));
        break;
    case 373:
        // "<!--genExtraRainBatteryStatus-->",
        sprintf (store, "%s", getBattStatus(id->loopStore.rainBatteryStatus));
        break;
    case 374:
        // "<!--WetBulbTemp-->",
        if (id->isMetricUnits)
        {
            sprintf (store, "%.1f",
                     wvutilsConvertFToC(wvutilsCalculateWetBulbTemp(id->loopStore.outTemp,
                                                                    id->loopStore.outHumidity,
                                                                    id->loopStore.barometer)));
        }
         else
        {
            sprintf (store, "%.1f",
                     wvutilsCalculateWetBulbTemp(id->loopStore.outTemp, id->loopStore.outHumidity,
                                                 id->loopStore.barometer));
        }
        break;
    default:
        store[0] = 0;
    }

    return;
}

static int replaceDataTags (HTML_MGR_ID id, char *oldline, char *newline)
{
    register int    i, j, k;
    int             len, taglen, found;
    char            temp[HTML_MAX_LINE_LENGTH];

    newline[0] = 0;

    for (i = 0, j = 0; oldline[i] != 0; i ++)
    {
        if (!strncmp (&oldline[i], "<!--", 4))
        {
            // get tag length
            found = FALSE;
            for (k = i, taglen = 1; oldline[k] != 0; k ++, taglen ++)
            {
                if (oldline[k] == '>')
                {
                    found = TRUE;
                    break;
                }
            }

            if (found)
            {
                // first check for file inclusion
                if (!strncmp (&oldline[i], "<!--include ", 12))
                {
                    // we must include an external file here:
                    // only copy the include file name, no tag delimiters,
                    // then return TRUE indicating a file should be included;
                    // we ignore any other tags after a file inclusion tag...
                    strncpy (newline, &oldline[i + 12], taglen - 15);
                    newline[taglen - 15] = 0;
                    return TRUE;
                }
                else
                {
                    computeTag (id, &oldline[i], taglen, temp);
                    len = strlen (temp);
                    if (len > 0)
                    {
                        // wview tag found, do the replacement
                        for (k = 0; k < len; k ++)
                        {
                            newline[j] = temp[k];
                            j ++;
                        }
                    }
                    else
                    {
                        // just copy old to new unchanged
                        for (k = 0; k < taglen; k ++)
                        {
                            newline[j] = oldline[i+k];
                            j ++;
                        }
                    }
                }

                // move the oldline index past the data tag
                i += (taglen-1);
            }
            else
            {
                // no closing '>' found, just move on
                newline[j] = oldline[i];
                j ++;
            }
        }
        else
        {
            newline[j] = oldline[i];
            j ++;
        }
    }
    newline[j] = 0;
    return FALSE;
}

static int createOutFile (HTML_MGR_ID id, char *templatefile, uint64_t startTime)
{
    FILE        *infile, *outfile, *incfile;
    char        *ptr;
    char        oldfname[WVIEW_STRING2_SIZE];
    char        newfname[WVIEW_STRING2_SIZE];
    char        includefname[WVIEW_STRING2_SIZE];
    char        line[HTML_MAX_LINE_LENGTH], newline[HTML_MAX_LINE_LENGTH];

    sprintf (oldfname, "%s/%s", id->htmlPath, templatefile);

    if (!strcmp (templatefile, "index.htx"))
    {
        // rename the output file to "index.html"
        sprintf (newfname, "%s/index.html", id->imagePath);
    }
    else if (!strcmp (templatefile, "index-day.htx") &&
             wvutilsIsDayTime (id->sunrise, id->sunset))
    {
        // rename the output file to "index.html"
        sprintf (newfname, "%s/index.html", id->imagePath);
    }
    else if (!strcmp (templatefile, "index-night.htx") &&
             !wvutilsIsDayTime (id->sunrise, id->sunset))
    {
        // rename the output file to "index.html"
        sprintf (newfname, "%s/index.html", id->imagePath);
    }
    else
    {
        // non-home page template
        sprintf (newfname, "%s/%s", id->imagePath, templatefile);

        // fix the extension
        ptr = strrchr (newfname, '.');
        if (ptr == NULL)
        {
            radMsgLog (PRI_MEDIUM, "createOutFile: no extension found!");
            return ERROR;
        }

        // is this an html template?
        if (!strcmp (ptr, ".htx"))
        {
            strcpy (ptr, ".htm");
        }
        // else is this an xml template?
        if (!strcmp (ptr, ".xtx"))
        {
            strcpy (ptr, ".xml");
        }
        // else can we apply the generic 'abcx' => 'abc' rule?
        else if (ptr[strlen(ptr)-1] == 'x')
        {
            ptr[strlen(ptr)-1] = 0;
        }
        // else default to html template
        else
        {
            strcpy (ptr, ".htm");
        }
    }

#if _DEBUG_GENERATION
    wvutilsLogEvent(PRI_HIGH, "GENERATION: TEMPLATE: %s OPEN INFILE: %u", 
                    oldfname, (uint32_t)(radTimeGetMSSinceEpoch() - startTime));
#endif

    //  ... now open the files up
    infile = fopen (oldfname, "r");
    if (infile == NULL)
    {
        radMsgLog (PRI_MEDIUM, "createOutFile: cannot open %s for reading!",
                   oldfname);
        return ERROR;
    }

#if _DEBUG_GENERATION
    wvutilsLogEvent(PRI_HIGH, "GENERATION: TEMPLATE: %s OPEN OUTFILE: %u", 
                    oldfname, (uint32_t)(radTimeGetMSSinceEpoch() - startTime));
#endif

    outfile = fopen (newfname, "w");
    if (outfile == NULL)
    {
        radMsgLog (PRI_MEDIUM, "createOutFile: cannot open %s for writing!",
                   newfname);
        fclose (infile);
        return ERROR;
    }

#if _DEBUG_GENERATION
    wvutilsLogEvent(PRI_HIGH, "GENERATION: TEMPLATE: %s START: %u", 
                    oldfname, (uint32_t)(radTimeGetMSSinceEpoch() - startTime));
#endif

    //  ... now read each line of the template -
    //  ... replacing any data tags
    //  ... then writing to the output file
    while (fgets (line, HTML_MAX_LINE_LENGTH, infile) != NULL)
    {
        if (replaceDataTags (id, line, newline) == TRUE)
        {
            // we must include an external file here, we expect to find it
            // in the output directory for images and expansions...
            sprintf (includefname, "%s/%s", id->imagePath, newline);
            incfile = fopen (includefname, "r");
            if (incfile == NULL)
            {
                radMsgLog (PRI_MEDIUM, "createOutFile: cannot open %s for reading!",
                           includefname);
                fclose (infile);
                fclose (outfile);
                return ERROR;
            }
            while (fgets (line, HTML_MAX_LINE_LENGTH, incfile) != NULL)
            {
                if (fputs (line, outfile) == EOF)
                {
                    fclose (incfile);
                    fclose (infile);
                    fclose (outfile);
                    return ERROR;
                }
            }

            // done
            fclose (incfile);
        }
        else
        {
            if (fputs (newline, outfile) == EOF)
            {
                fclose (infile);
                fclose (outfile);
                return ERROR;
            }
        }
    }

#if _DEBUG_GENERATION
wvutilsLogEvent(PRI_HIGH, "GENERATION: TEMPLATE: %s FINISH: %u", 
                oldfname, (uint32_t)(radTimeGetMSSinceEpoch() - startTime));
#endif

    fclose (infile);
    fclose (outfile);
    return OK;
}

static void GetTimeFormat (char *timestr, char *dateFormat)
{
    time_t      ntime;
    struct tm   tmtime;
    char        dateTime[256];

    ntime = time (NULL);
    tmtime =  *localtime(&ntime);
    strncpy(dateTime, dateFormat, 240);
    strncat(dateTime, " %R",15);
    strftime(timestr, 256, dateTime, &tmtime);
    return;
}



//  ... API methods

int htmlGenerateInit (void)
{
    char            instance[32], value[_MAX_PATH];
    CF_ID           configFileId;
    struct stat     fileStatus;
    int             index;

    // Set up tag matching search tree:
    tagSearchEngine = radtextsearchInit();
    if (tagSearchEngine == NULL)
    {
        radMsgLog(PRI_CATASTROPHIC, "htmlGenerateInit: radtextsearchInit failed!");
        return ERROR;
    }

    // Insert all of our tags:
    for (index = 0; dataTags[index] != NULL; index ++)
    {
        if (radtextsearchInsert(tagSearchEngine, dataTags[index], index) == ERROR)
        {
            radMsgLog(PRI_CATASTROPHIC, "htmlGenerateInit: radtextsearchInsert %d failed!", index);
            return ERROR;
        }
    }

    radMsgLog(PRI_STATUS, "Tag Search red-black tree: max black node tree height: %d", 
              radtextsearchDebug(tagSearchEngine->root));


    // Plot Prefs:
    //Set default values before attempting to read from conf file
    plotPrefs.bucketTransparent         = 0;
    plotPrefs.bucketBgColor             = 0xF5F5F500;
    plotPrefs.bucketFgColor             = 0x00000000;
    plotPrefs.bucketContentColor        = 0x4282B400;
    plotPrefs.bucketHighColor           = 0xFF000000;
    plotPrefs.bucketLowColor            = 0x00BFFF00;
    plotPrefs.bucketTitleBgColor        = 0xD8D8D800;
    plotPrefs.bucketTitleFgColor        = 0x00000000;
    plotPrefs.bucketTextColor           = 0x00000000;
    plotPrefs.bucketTransparentColor    = 0x12121200;
    plotPrefs.bucketImageWidth          = 120;
    plotPrefs.bucketImageHeight         = 240;
    plotPrefs.bucketWidth               = 36;

    plotPrefs.chartTransparent          = 0;
    plotPrefs.chartImageBgColor         = 0xF5F5F500;
    plotPrefs.chartGraphBgColor         = 0xD8D8D800;
    plotPrefs.chartGridColor            = 0xA0A00000;
    plotPrefs.chartFirstLineColor       = 0x4282B400;
    plotPrefs.chartSecondLineColor      = 0xB4424200;
    plotPrefs.chartThirdLineColor       = 0x42B44200;
    plotPrefs.chartFourthLineColor      = 0xECA44200;
    plotPrefs.chartTitleBgColor         = 0xD8D8D800;
    plotPrefs.chartTitleFgColor         = 0x00000000;
    plotPrefs.chartTextColor            = 0x00000000;
    plotPrefs.chartWidth                = 300;
    plotPrefs.chartHeight               = 180;

    plotPrefs.barImageBgColor           = 0xF5F5F500;
    plotPrefs.barGraphBgColor           = 0xD8D8D800;
    plotPrefs.barBarColor               = 0x6666FF00;
    plotPrefs.barTitleBgColor           = 0xD8D8D800;
    plotPrefs.barTitleFgColor           = 0x00000000;
    plotPrefs.barGridColor              = 0xA0A00000;
    plotPrefs.barTextColor              = 0x00000000;
    plotPrefs.barWidth                  = 300;
    plotPrefs.barHeight                 = 180;

    plotPrefs.dialTransparent           = 0;
    plotPrefs.dialBgColor               = 0xf5f5f500;
    plotPrefs.dialImageBgColor          = 0x12121200;
    plotPrefs.dialCenterColor           = 0x00000000;
    plotPrefs.dialCenterTextColor       = 0x0000ff00;
    plotPrefs.dialCenterHighColor       = 0xff000000;
    plotPrefs.dialPointerColor          = 0x6666FF00;
    plotPrefs.dialPointerOutlineColor   = 0x6666ff00;
    plotPrefs.dialTextColor             = 0x00000000;
    plotPrefs.dialHighColor             = 0xff000000;
    plotPrefs.dialLowColor              = 0x0000ff00;
    plotPrefs.dialAppColor              = 0x00ff0000;
    plotPrefs.dialImageWidth            = 160;
    plotPrefs.dialDiameter              = 156;
    plotPrefs.dialCtrDiameter           = 15;

    plotPrefs.multichartFirstLineColor       = 0xF4875300;
    plotPrefs.multichartSecondLineColor      = 0x40C62D00;
    plotPrefs.multichartThirdLineColor       = 0x0000FF00;
    plotPrefs.multichartFourthLineColor      = 0xFF000000;

    // ... get our configuration values
    sprintf (value, "%s/%s", WVIEW_CONFIG_DIR, HTML_GRAPHICS_CONFIG_FILENAME);
    if (stat (value, &fileStatus) == -1)
    {
        radMsgLog (PRI_MEDIUM, "%s/%s file missing - using defaults...",
                   WVIEW_CONFIG_DIR, HTML_GRAPHICS_CONFIG_FILENAME);
    }
    else
    {
        configFileId = radCfOpen (value);
        if (configFileId == NULL)
        {
            radMsgLog (PRI_MEDIUM, "radCfOpen failed for %s - using defaults...",
                       HTML_GRAPHICS_CONFIG_FILENAME);
        }
        else
        {
            // Read values from graphics.conf, if present
            // Bucket Plots
            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_BUCKET_TRANSPARENT],
                                    instance, value) != ERROR)
            {
                plotPrefs.bucketTransparent = atoi (value);
            }

            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_BUCKET_BG_COLOR],
                                    instance, value) != ERROR)
            {
                sscanf(value, "%x", &plotPrefs.bucketBgColor);
            }

            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_BUCKET_FG_COLOR],
                                    instance, value) != ERROR)
            {
                sscanf(value, "%x", &plotPrefs.bucketFgColor);
            }

            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_BUCKET_CONTENT_COLOR],
                                    instance, value) != ERROR)
            {
                sscanf(value, "%x", &plotPrefs.bucketContentColor);
            }

            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_BUCKET_HIGH_COLOR],
                                    instance, value) != ERROR)
            {
                sscanf(value, "%x", &plotPrefs.bucketHighColor);
            }

            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_BUCKET_LOW_COLOR],
                                    instance, value) != ERROR)
            {
                sscanf(value, "%x", &plotPrefs.bucketLowColor);
            }

            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_BUCKET_TITLE_BG_COLOR],
                                    instance, value) != ERROR)
            {
                sscanf(value, "%x", &plotPrefs.bucketTitleBgColor);
            }

            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_BUCKET_TITLE_FG_COLOR],
                                    instance, value) != ERROR)
            {
                sscanf(value, "%x", &plotPrefs.bucketTitleFgColor);
            }

            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_BUCKET_TEXT_COLOR],
                                    instance, value) != ERROR)
            {
                sscanf(value, "%x", &plotPrefs.bucketTextColor);
            }

            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_BUCKET_IMAGE_WIDTH],
                                    instance, value) != ERROR)
            {
                sscanf(value, "%d", &plotPrefs.bucketImageWidth);
            }

            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_BUCKET_IMAGE_HEIGHT],
                                    instance, value) != ERROR)
            {
                sscanf(value, "%d", &plotPrefs.bucketImageHeight);
            }

            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_BUCKET_WIDTH],
                                    instance, value) != ERROR)
            {
                sscanf(value, "%d", &plotPrefs.bucketWidth);
            }


            // Chart Plots
            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_CHART_TRANSPARENT],
                                    instance, value) != ERROR)
            {
                plotPrefs.chartTransparent = atoi (value);
            }

            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_CHART_IMAGE_BG_COLOR],
                                    instance, value) != ERROR)
            {
                sscanf(value, "%x", &plotPrefs.chartImageBgColor);
            }


            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_CHART_GRAPH_BG_COLOR],
                                    instance, value) != ERROR)
            {
                sscanf(value, "%x", &plotPrefs.chartGraphBgColor);
            }

            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_CHART_GRID_COLOR],
                                    instance, value) != ERROR)
            {
                sscanf(value, "%x", &plotPrefs.chartGridColor);

            }

            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_CHART_FIRST_LINE_COLOR],
                                    instance, value) != ERROR)
            {
                sscanf(value, "%x", &plotPrefs.chartFirstLineColor);
            }

            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_CHART_SECOND_LINE_COLOR],
                                    instance, value) != ERROR)
            {
                sscanf(value, "%x", &plotPrefs.chartSecondLineColor);
            }

            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_CHART_THIRD_LINE_COLOR],
                                    instance, value) != ERROR)
            {
                sscanf(value, "%x", &plotPrefs.chartThirdLineColor);
            }

            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_CHART_FOURTH_LINE_COLOR],
                                    instance, value) != ERROR)
            {
                sscanf(value, "%x", &plotPrefs.chartFourthLineColor);
            }

            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_CHART_TITLE_BG_COLOR],
                                    instance, value) != ERROR)
            {
                sscanf(value, "%x", &plotPrefs.chartTitleBgColor);
            }

            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_CHART_TITLE_FG_COLOR],
                                    instance, value) != ERROR)
            {
                sscanf(value, "%x", &plotPrefs.chartTitleFgColor);
            }

            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_CHART_TEXT_COLOR],
                                    instance, value) != ERROR)
            {
                sscanf(value, "%x", &plotPrefs.chartTextColor);
            }

            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_CHART_WIDTH],
                                    instance, value) != ERROR)
            {
                sscanf(value, "%d", &plotPrefs.chartWidth);
            }

            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_CHART_HEIGHT],
                                    instance, value) != ERROR)
            {
                sscanf(value, "%d", &plotPrefs.chartHeight);
            }



            // Bar Charts
            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_BAR_IMAGE_BG_COLOR],
                                    instance, value) != ERROR)
            {
                sscanf(value, "%x", &plotPrefs.barImageBgColor);
            }


            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_BAR_GRAPH_BG_COLOR],
                                    instance, value) != ERROR)
            {
                sscanf(value, "%x", &plotPrefs.barGraphBgColor);
            }

            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_BAR_BAR_COLOR],
                                    instance, value) != ERROR)
            {
                sscanf(value, "%x", &plotPrefs.barBarColor);

            }

            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_BAR_TITLE_BG_COLOR],
                                    instance, value) != ERROR)
            {
                sscanf(value, "%x", &plotPrefs.barTitleBgColor);
            }

            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_BAR_TITLE_FG_COLOR],
                                    instance, value) != ERROR)
            {
                sscanf(value, "%x", &plotPrefs.barTitleFgColor);
            }

            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_BAR_GRID_COLOR],
                                    instance, value) != ERROR)
            {
                sscanf(value, "%x", &plotPrefs.barGridColor);
            }

            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_BAR_TEXT_COLOR],
                                    instance, value) != ERROR)
            {
                sscanf(value, "%x", &plotPrefs.barTextColor);
            }

            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_BAR_WIDTH],
                                    instance, value) != ERROR)
            {
                sscanf(value, "%d", &plotPrefs.barWidth);
            }

            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_BAR_HEIGHT],
                                    instance, value) != ERROR)
            {
                sscanf(value, "%d", &plotPrefs.barHeight);
            }


            // Dial Charts
            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_DIAL_TRANSPARENT],
                                    instance, value) != ERROR)
            {
                plotPrefs.dialTransparent = atoi (value);
            }

            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_DIAL_BG_COLOR],
                                    instance, value) != ERROR)
            {
                sscanf(value, "%x", &plotPrefs.dialBgColor);
            }


            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_DIAL_IMAGE_BG_COLOR],
                                    instance, value) != ERROR)
            {
                sscanf(value, "%x", &plotPrefs.dialImageBgColor);
            }

            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_DIAL_CENTER_COLOR],
                                    instance, value) != ERROR)
            {
                sscanf(value, "%x", &plotPrefs.dialCenterColor);

            }

            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_DIAL_CENTER_TEXT_COLOR],
                                    instance, value) != ERROR)
            {
                sscanf(value, "%x", &plotPrefs.dialCenterTextColor);
            }

            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_DIAL_CENTER_HIGH_COLOR],
                                    instance, value) != ERROR)
            {
                sscanf(value, "%x", &plotPrefs.dialCenterHighColor);
            }

            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_DIAL_POINTER_COLOR],
                                    instance, value) != ERROR)
            {
                sscanf(value, "%x", &plotPrefs.dialPointerColor);
            }

            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_DIAL_POINTER_OUTLINE_COLOR],
                                    instance, value) != ERROR)
            {
                sscanf(value, "%x", &plotPrefs.dialPointerOutlineColor);
            }

            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_DIAL_HIGH_COLOR],
                                    instance, value) != ERROR)
            {
                sscanf(value, "%x", &plotPrefs.dialHighColor);
            }

            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_DIAL_LOW_COLOR],
                                    instance, value) != ERROR)
            {
                sscanf(value, "%x", &plotPrefs.dialLowColor);
            }

            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_DIAL_APP_COLOR],
                                    instance, value) != ERROR)
            {
                sscanf(value, "%x", &plotPrefs.dialAppColor);
            }

            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_DIAL_TEXT_COLOR],
                                    instance, value) != ERROR)
            {
                sscanf(value, "%x", &plotPrefs.dialTextColor);
            }

            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_DIAL_IMAGE_WIDTH],
                                    instance, value) != ERROR)
            {
                sscanf(value, "%d", &plotPrefs.dialImageWidth);
            }

            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_DIAL_DIAMETER],
                                    instance, value) != ERROR)
            {
                sscanf(value, "%d", &plotPrefs.dialDiameter);
            }

            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_DIAL_CTR_DIAMETER],
                                    instance, value) != ERROR)
            {
                sscanf(value, "%d", &plotPrefs.dialCtrDiameter);
            }

            // MultiChart Line Color
            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_MULTICHART_FIRST_LINE_COLOR],
                                    instance, value) != ERROR)
            {
                sscanf(value, "%x", &plotPrefs.multichartFirstLineColor);
            }

            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_MULTICHART_SECOND_LINE_COLOR],
                                    instance, value) != ERROR)
            {
                sscanf(value, "%x", &plotPrefs.multichartSecondLineColor);
            }

            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_MULTICHART_THIRD_LINE_COLOR],
                                    instance, value) != ERROR)
            {
                sscanf(value, "%x", &plotPrefs.multichartThirdLineColor);
            }

            if (radCfGetFirstEntry (configFileId, configIDs[CFG_ID_MULTICHART_FOURTH_LINE_COLOR],
                                    instance, value) != ERROR)
            {
                sscanf(value, "%x", &plotPrefs.multichartFourthLineColor);
            }

            radCfClose (configFileId);
        }
    }

    return OK;
}

int htmlgenGetChartDefaultLine (void)
{
    return plotPrefs.chartFirstLineColor;
}
int htmlgenGetChartDefaultSecondLine (void)
{
    return plotPrefs.chartSecondLineColor;
}
int htmlgenGetChartDefaultThirdLine (void)
{
    return plotPrefs.chartThirdLineColor;
}
int htmlgenGetChartDefaultFourthLine (void)
{
    return plotPrefs.chartFourthLineColor;
}


int htmlgenGetMultiChartDefaultFirstLine (void)
{
    return plotPrefs.multichartFirstLineColor;
}

int htmlgenGetMultiChartDefaultSecondLine (void)
{
    return plotPrefs.multichartSecondLineColor;
}

int htmlgenGetMultiChartDefaultThirdLine (void)
{
    return plotPrefs.multichartThirdLineColor;
}

int htmlgenGetMultiChartDefaultFourthLine (void)
{
    return plotPrefs.multichartFourthLineColor;
}

int htmlgenOutputFiles (HTML_MGR_ID id, uint64_t startTime)
{
    register HTML_TMPL  *tmpl;
    int                 count = 0;

    for (tmpl = (HTML_TMPL *)radListGetFirst (&id->templateList);
            tmpl != NULL;
            tmpl = (HTML_TMPL *)radListGetNext (&id->templateList, (NODE_PTR)tmpl))
    {
#if _DEBUG_GENERATION
        wvutilsLogEvent(PRI_HIGH, "GENERATION: TEMPLATE: %s: %u ms",
                        tmpl->fname, (uint32_t)(radTimeGetMSSinceEpoch() - startTime));
#endif

        if (createOutFile (id, tmpl->fname, startTime) == ERROR)
        {
            radMsgLog (PRI_MEDIUM, "htmlgenOutputFiles: %s failed!", tmpl->fname);
        }
        else
        {
            count ++;
        }
    }

    return count;
}


int htmlgenMesonetFile (HTML_MGR_ID id, WVIEW_MSG_ARCHIVE_NOTIFY *armsg)
{
    FILE        *outfile;
    char        newfname[256];
    char        newline[128];
    time_t      ntime;
    struct tm   tmtime;

    sprintf (newfname, "%s/formateddata.txt", id->imagePath);

    outfile = fopen (newfname, "w");
    if (outfile == NULL)
    {
        return ERROR;
    }

    // OK, let's write out the ASCII data...
    ntime = time (NULL);
    localtime_r (&ntime, &tmtime);

    fprintf (outfile, "%2.2d-%2.2d-%2.2d, %2.2d%2.2d\n",
             tmtime.tm_mday,
             tmtime.tm_mon + 1,
             tmtime.tm_year - 100,
             tmtime.tm_hour,
             tmtime.tm_min);

    fprintf (outfile, "TempIn %.0f\n",
             wvutilsConvertFToC ((float)armsg->intemp/10));
    fprintf (outfile, "TempEx %.0f\n",
             wvutilsConvertFToC ((float)armsg->temp/10));

    fprintf (outfile, "WindHi %d\n", (int)armsg->hiwspeedF);
    fprintf (outfile, "WindAv %d\n", (int)armsg->wspeedF);
    fprintf (outfile, "WindDr %d\n", armsg->winddir);

    fprintf (outfile, "BarmPs %d\n", armsg->barom);

    fprintf (outfile, "HumdIn %d\n", armsg->inhumidity);
    fprintf (outfile, "HumdEx %d\n", armsg->humidity);

    fprintf (outfile, "RnFall %.2f\n", armsg->rainHour);
    fprintf (outfile, "DailyRnFall %.2f\n", armsg->rainDay);

    fclose (outfile);
    return OK;
}


int htmlGenPngBucket
(
    char                *fname,
    int                 isMetric,
    float               value,
    float               low,
    float               high,
    char                *title,
    char                *units,
    int                 decimalPlaces,
    float               lowScale,
    float               highScale,
    char                *dateFormat,
    int                 isDualUnits
)
{
    char                timestr[WVIEW_STRING2_SIZE];
    register BUCKET_ID  id = bucketCreate (plotPrefs.bucketImageWidth, plotPrefs.bucketImageHeight,
                                           plotPrefs.bucketWidth, title);
    int                 retVal;

    if (id == NULL)
    {
        return ERROR;
    }

    if (value <= ARCHIVE_VALUE_NULL)
    {
        bucketDestroy (id);
        return ERROR;
    }

    GetTimeFormat (timestr, dateFormat);

    bucketSetScale (id,
                    (double)lowScale,
                    (double)highScale,
                    pow (10.0, (double)(1 - decimalPlaces)));
    bucketSetValues (id, (double)high, (double)low, (double)value);
    bucketSetUnits (id, units);
    bucketSetDecimalPlaces (id, decimalPlaces);

    bucketSetDateTime (id, timestr);

    bucketSetBGColor (id, plotPrefs.bucketBgColor);

    bucketSetBucketColor (id, plotPrefs.bucketFgColor);
    bucketSetContentColor (id, plotPrefs.bucketContentColor);
    bucketSetHighLowColors (id, plotPrefs.bucketHighColor, plotPrefs.bucketLowColor);
    bucketSetTitleColors (id, plotPrefs.bucketTitleFgColor, plotPrefs.bucketTitleBgColor);
    bucketSetTextColor (id, plotPrefs.bucketTextColor);
    bucketSetTransparency (id, plotPrefs.bucketTransparent);
    bucketSetDualUnits (id, isDualUnits);
    id->isMetric = isMetric;

    bucketRender (id);
    retVal = bucketSave (id, fname);
    bucketDestroy (id);

    return retVal;
}

int htmlGenPngRainBucket
(
    char                *fname,
    int                 isMetric,
    float               value,
    float               low,
    float               high,
    char                *title,
    char                *units,
    int                 decimalPlaces,
    float               lowScale,
    float               highScale,
    char                *dateFormat,
    int                 isDualUnits
)
{
    char                timestr[WVIEW_STRING2_SIZE];
    register BUCKET_ID  id = bucketCreate (plotPrefs.bucketImageWidth, plotPrefs.bucketImageHeight,
                                           plotPrefs.bucketWidth, title);
    int                 retVal;

    if (id == NULL)
    {
        return ERROR;
    }

    if (value <= ARCHIVE_VALUE_NULL)
    {
        bucketDestroy (id);
        return ERROR;
    }

    GetTimeFormat (timestr, dateFormat);

    bucketSetScale (id,
                    (double)lowScale,
                    (double)highScale,
                    pow (10.0, (double)(1 - decimalPlaces)));
    bucketSetValues (id, (double)high, (double)low, (double)value);
    bucketSetUnits (id, units);
    bucketSetDecimalPlaces (id, decimalPlaces);

    bucketSetDateTime (id, timestr);

    bucketSetBGColor (id, plotPrefs.bucketBgColor);
    bucketSetBucketColor (id, plotPrefs.bucketFgColor);
    bucketSetContentColor (id, plotPrefs.bucketContentColor);
    bucketSetHighLowColors (id, plotPrefs.bucketHighColor, plotPrefs.bucketLowColor);
    bucketSetTitleColors (id, plotPrefs.bucketTitleFgColor, plotPrefs.bucketTitleBgColor);
    bucketSetTextColor (id, plotPrefs.bucketTextColor);
    bucketSetTransparency (id, plotPrefs.bucketTransparent);
    bucketSetDualUnits (id, isDualUnits);
    // id->isMetric = 0;

    bucketRender (id);
    retVal = bucketSave (id, fname);
    bucketDestroy (id);

    return retVal;
}

int htmlGenPngBaromBucket
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
)
{
    char                timestr[WVIEW_STRING2_SIZE];
    register BUCKET_ID  id = bucketCreate (plotPrefs.bucketImageWidth, plotPrefs.bucketImageHeight,
                                           plotPrefs.bucketWidth, title);
    int                 retVal;

    if (id == NULL)
    {
        return ERROR;
    }

    if (value <= ARCHIVE_VALUE_NULL)
    {
        bucketDestroy (id);
        return ERROR;
    }

    GetTimeFormat (timestr, dateFormat);

    bucketSetScale (id, 29.5, 30.49, 0.005);
    bucketSetValues (id, (double)high, (double)low, (double)value);
    bucketSetUnits (id, units);
    bucketSetDecimalPlaces (id, decimalPlaces);

    bucketSetDateTime (id, timestr);

    bucketSetBGColor (id, plotPrefs.bucketBgColor);
    bucketSetBucketColor (id, plotPrefs.bucketFgColor);
    bucketSetContentColor (id, plotPrefs.bucketContentColor);
    bucketSetHighLowColors (id, plotPrefs.bucketHighColor, plotPrefs.bucketLowColor);
    bucketSetTitleColors (id, plotPrefs.bucketTitleFgColor, plotPrefs.bucketTitleBgColor);
    bucketSetTextColor (id, plotPrefs.bucketTextColor);
    bucketSetTransparency (id, plotPrefs.bucketTransparent);
    bucketSetDualUnits (id, isDualUnits);

    bucketRender (id);
    retVal = bucketSave (id, fname);
    bucketDestroy (id);

    return retVal;
}

int htmlGenMetricPngBaromBucket
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
)
{
    char                timestr[WVIEW_STRING2_SIZE];
    register BUCKET_ID  id = bucketCreate (plotPrefs.bucketImageWidth, plotPrefs.bucketImageHeight,
                                           plotPrefs.bucketWidth, title);
    int                 retVal;

    if (id == NULL)
    {
        return ERROR;
    }

    if (value <= ARCHIVE_VALUE_NULL)
    {
        bucketDestroy (id);
        return ERROR;
    }

    GetTimeFormat (timestr, dateFormat);

    bucketSetScale (id, 1000.0, 1039.9, 0.1);
    bucketSetValues (id, (double)high, (double)low, (double)value);
    bucketSetUnits (id, units);
    bucketSetDecimalPlaces (id, decimalPlaces);

    bucketSetDateTime (id, timestr);

    bucketSetBGColor (id, plotPrefs.bucketBgColor);
    bucketSetBucketColor (id, plotPrefs.bucketFgColor);
    bucketSetContentColor (id, plotPrefs.bucketContentColor);
    bucketSetHighLowColors (id, plotPrefs.bucketHighColor, plotPrefs.bucketLowColor);
    bucketSetTitleColors (id, plotPrefs.bucketTitleFgColor, plotPrefs.bucketTitleBgColor);
    bucketSetTextColor (id, plotPrefs.bucketTextColor);
    bucketSetTransparency (id, plotPrefs.bucketTransparent);
    bucketSetDualUnits (id, isDualUnits);

    bucketRender (id);
    retVal = bucketSave (id, fname);
    bucketDestroy (id);

    return retVal;
}

int htmlGenPngPercentBucket
(
    char                *fname,
    int                 isMetric,
    float               value,
    float               low,
    float               high,
    char                *title,
    char                *units,
    int                 decimalPlaces,
    char                *dateFormat,
    int                 isDualUnits
)
{
    char                timestr[WVIEW_STRING2_SIZE];
    register BUCKET_ID  id = bucketCreate (plotPrefs.bucketImageWidth, plotPrefs.bucketImageHeight,
                                           plotPrefs.bucketWidth, title);
    int                 retVal;

    if (id == NULL)
    {
        return ERROR;
    }

    if (value < 0)
    {
        bucketDestroy (id);
        return ERROR;
    }

    GetTimeFormat (timestr, dateFormat);

    bucketSetScale (id, 0.0, 99.0, pow (10.0, (double)(1 - decimalPlaces)));
    bucketSetValues (id, (double)high, (double)low, (double)value);
    bucketSetUnits (id, units);
    bucketSetDecimalPlaces (id, decimalPlaces);

    bucketSetDateTime (id, timestr);

    bucketSetBGColor (id, plotPrefs.bucketBgColor);
    bucketSetBucketColor (id, plotPrefs.bucketFgColor);
    bucketSetContentColor (id, plotPrefs.bucketContentColor);
    bucketSetHighLowColors (id, plotPrefs.bucketHighColor, plotPrefs.bucketLowColor);
    bucketSetTitleColors (id, plotPrefs.bucketTitleFgColor, plotPrefs.bucketTitleBgColor);
    bucketSetTextColor (id, plotPrefs.bucketTextColor);
    bucketSetTransparency (id, plotPrefs.bucketTransparent);


    bucketRender (id);
    retVal = bucketSave (id, fname);
    bucketDestroy (id);

    return retVal;
}

int htmlGenPngChart
(
    char                *fname,
    int                 isMetric,
    float               *values,
    int                 xHashes,
    float               min,
    float               max,
    float               step,
    char                *labels[],
    int                 numValues,
    int                 totalValues,
    char                *title,
    char                *units,
    int                 decimalPlaces,
    char                *dateFormat,
    int                 isDualUnits
)
{
    register int        i;
    register CHART_ID   id;
    char                timestr[WVIEW_STRING2_SIZE];
    int                 retVal;


    id = chartCreate (plotPrefs.chartWidth, plotPrefs.chartHeight, title, units, CHART_TYPE_LINE);
    if (id == NULL)
    {
        return ERROR;
    }

    GetTimeFormat (timestr, dateFormat);

    chartSetBGColor (id, plotPrefs.chartImageBgColor);
    chartSetChartColor (id, plotPrefs.chartGraphBgColor);
    chartSetGridColor (id, plotPrefs.chartGridColor);
    chartSetLineColor (id, plotPrefs.chartFirstLineColor);
    chartSetTitleColors (id, plotPrefs.chartTitleFgColor, plotPrefs.chartTitleBgColor);
    chartSetTextColor (id, plotPrefs.chartTextColor);
    chartSetTransparency (id, plotPrefs.chartTransparent);

    chartSetXScale (id, 0);
    chartSetXHashes (id, xHashes);
    chartSetYScale (id, min, max, step, decimalPlaces);
    chartSetDateTime (id, timestr);

    for (i = 0; i < numValues; i ++)
    {
        chartAddPoint (id, values[i], labels[i]);
    }
    for (i = numValues; i < totalValues; i ++)
    {
        chartAddPoint (id, ARCHIVE_VALUE_NULL, labels[i]);
    }
    id->isMetric = isMetric;
    if (isDualUnits)
    {
        chartSetDualUnits (id);
    }
    else
    {
        strcpy (id->DualUnit, "");
    }

    chartRender (id);
    retVal = chartSave (id, fname);
    chartDestroy (id);

    return retVal;
}

int htmlGenPngBarChart
(
    char                *fname,
    int                 isMetric,
    float               *values,
    int                 xHashes,
    float               min,
    float               max,
    float               step,
    char                *labels[],
    int                 numValues,
    char                *title,
    char                *units,
    int                 decimalPlaces,
    int                 numPointsPerBar,
    int                 numBars,
    int                 numPointsInFirstBar,
    char                *dateFormat,
    int                 isDualUnits
)
{
    register int        i, j, k, bars = 0;
    register CHART_ID   id;
    char                *barLabel, *lastBarLabel, timestr[WVIEW_STRING2_SIZE];
    float               barTotal = 0.0;
    int                 retVal;

    id = chartCreate (plotPrefs.barWidth, plotPrefs.barHeight, title, units, CHART_TYPE_BAR);
    if (id == NULL)
    {
        return ERROR;
    }

    GetTimeFormat (timestr, dateFormat);

    chartSetBGColor (id, plotPrefs.barImageBgColor);
    chartSetChartColor (id, plotPrefs.barGraphBgColor);
    chartSetGridColor (id, plotPrefs.barGridColor);
    chartSetLineColor (id, plotPrefs.barBarColor );
    chartSetTitleColors (id, plotPrefs.barTitleFgColor, plotPrefs.barTitleBgColor);
    chartSetTextColor (id, plotPrefs.barTextColor);
    chartSetTransparency (id, plotPrefs.chartTransparent);

    chartSetXScale (id, 0);
    chartSetXHashes (id, xHashes);
    chartSetYScale (id, min, max, step, decimalPlaces);
    chartSetDateTime (id, timestr);

    barLabel = labels[0];
    for (i = 0, j = numPointsInFirstBar; (i < numValues) && (bars < numBars); i ++)
    {
        if (values[i] > ARCHIVE_VALUE_NULL)
        {
            barTotal += values[i];
        }

        if (-- j <= 0)
        {
            if (++bars == numBars)
            {
                chartAddPoint (id, barTotal, labels[i]);
            }
            else
            {
                chartAddPoint (id, barTotal, barLabel);
            }

            barTotal = 0.0;
            lastBarLabel = barLabel;
            for (k = 1; (i + k) < numValues; k ++)
            {
                barLabel = labels[i+k];
                if (strcmp(lastBarLabel, barLabel))
                {
                    break;
                }
            }

            j = numPointsPerBar;
        }
    }

    // if last is not a complete bar, add it anyway
    if (bars < numBars)
    {
        chartAddPoint (id, barTotal, labels[i-1]);
    }

    // add the dummy last one for labeling purposes
    chartAddPoint (id, 0, labels[i]);
    id->isMetric = isMetric;
    if (isDualUnits)
    {
        chartSetDualUnits (id);
    }
    else
    {
        strcpy (id->DualUnit, "");
    }
    chartRender (id);
    retVal = chartSave (id, fname);
    chartDestroy (id);

    return retVal;
}

int htmlGenPngPercentChart
(
    char                *fname,
    int                 isMetric,
    float               *values,
    int                 xHashes,
    char                *labels[],
    int                 numValues,
    int                 totalValues,
    char                *title,
    char                *dateFormat
)
{
    register int        i;
    register CHART_ID   id;
    char                timestr[WVIEW_STRING2_SIZE];
    int                 retVal;

    id = chartCreate (plotPrefs.chartWidth, plotPrefs.chartHeight, title, " % ", CHART_TYPE_LINE);
    if (id == NULL)
    {
        return ERROR;
    }

    GetTimeFormat (timestr, dateFormat);

    chartSetBGColor (id, plotPrefs.chartImageBgColor);
    chartSetChartColor (id, plotPrefs.chartGraphBgColor);
    chartSetGridColor (id, plotPrefs.chartGridColor);
    chartSetLineColor (id, plotPrefs.chartFirstLineColor );
    chartSetTitleColors (id, plotPrefs.chartTitleFgColor, plotPrefs.chartTitleBgColor);
    chartSetTextColor (id, plotPrefs.chartTextColor);
    chartSetTransparency (id, plotPrefs.chartTransparent);

    chartSetXScale (id, 0);
    chartSetXHashes (id, xHashes);
    chartSetYScale (id, 0, 100, 20, 0);
    chartSetDateTime (id, timestr);

    for (i = 0; i < numValues; i ++)
    {
        chartAddPoint (id, values[i], labels[i]);
    }
    for (i = numValues; i < totalValues; i ++)
    {
        chartAddPoint (id, ARCHIVE_VALUE_NULL, labels[i]);
    }
    strcpy (id->DualUnit, "");

    chartRender (id);
    retVal = chartSave (id, fname);
    chartDestroy (id);

    return retVal;
}

int htmlGenPngWDIRChart
(
    char                *fname,
    int                 isMetric,
    float               *values,
    int                 xHashes,
    char                *labels[],
    int                 numValues,
    int                 totalValues,
    char                *title,
    char                *units,
    int                 decimalPlaces,
    char                *dateFormat
)
{
    register int        i;
    register CHART_ID   id;
    char                timestr[WVIEW_STRING2_SIZE];
    float               min = 0.0, max = 359.0;
    int                 retVal;

    id = chartCreate (plotPrefs.chartWidth, plotPrefs.chartHeight, title, units, CHART_TYPE_SCATTER);
    if (id == NULL)
    {
        return ERROR;
    }

    GetTimeFormat (timestr, dateFormat);

    chartSetBGColor (id, plotPrefs.chartImageBgColor);
    chartSetChartColor (id, plotPrefs.chartGraphBgColor);
    chartSetGridColor (id, plotPrefs.chartGridColor);
    chartSetLineColor (id, plotPrefs.chartFirstLineColor );
    chartSetTitleColors (id, plotPrefs.chartTitleFgColor, plotPrefs.chartTitleBgColor);
    chartSetTextColor (id, plotPrefs.chartTextColor);
    chartSetTransparency (id, plotPrefs.chartTransparent);

    chartSetXScale (id, 0);
    chartSetXHashes (id, xHashes);
    chartSetYScale (id, min, max, 45.0, 0);
    chartSetDateTime (id, timestr);

    for (i = 0; i < numValues; i ++)
    {
        chartAddPoint (id, values[i], labels[i]);
    }
    for (i = numValues; i < totalValues; i ++)
    {
        chartAddPoint (id, ARCHIVE_VALUE_NULL, labels[i]);
    }

    strcpy (id->DualUnit, "");

    chartRender (id);
    retVal = chartSave (id, fname);
    chartDestroy (id);

    return retVal;
}

int htmlGenPngMultiChart
(
    char                    *fname,
    int                     isMetric,
    MC_DATASET              *datasets,
    int                     numDataSets,
    int                     xHashes,
    float                   min,
    float                   max,
    float                   step,
    char                    *labels[],
    int                     numValues,
    int                     totalValues,
    char                    *units,
    int                     decimalPlaces,
    char                    *dateFormat,
    int                     isDualUnits
)
{
    register int            i, j;
    register MULTICHART_ID  id;
    char                    timestr[WVIEW_STRING2_SIZE];
    char                    *legends[MC_MAX_DATA_SETS];
    double                  values[MC_MAX_DATA_SETS];
    int                     retVal = OK;

    if (numDataSets > MC_MAX_DATA_SETS)
        numDataSets = MC_MAX_DATA_SETS;

    for (i = 0; i < numDataSets; i ++)
    {
        legends[i] = datasets[i].legend;
    }

    id = multiChartCreate (plotPrefs.chartWidth, plotPrefs.chartHeight, units, numDataSets, legends);
    if (id == NULL)
    {
        return ERROR;
    }

    GetTimeFormat (timestr, dateFormat);

    multiChartSetBGColor (id, plotPrefs.chartImageBgColor);
    multiChartSetChartColor (id, plotPrefs.chartGraphBgColor);
    multiChartSetGridColor (id, plotPrefs.chartGridColor);
    multiChartSetTitleColors (id, plotPrefs.chartTitleFgColor, plotPrefs.chartTitleBgColor);
    multiChartSetTextColor (id, plotPrefs.chartTextColor);
    multiChartSetTransparency (id, plotPrefs.chartTransparent);

    multiChartSetXScale (id, 0);
    multiChartSetXHashes (id, xHashes);
    multiChartSetYScale (id, min, max, step, decimalPlaces);
    multiChartSetDateTime (id, timestr);

    for (i = 0; i < numDataSets; i ++)
    {
        multiChartSetLineColor (id, i, datasets[i].lineColor);
    }

    for (i = 0; i < numValues; i ++)
    {
        for (j = 0; j < numDataSets; j ++)
        {
            values[j] = datasets[j].valueset[i];
        }

        multiChartAddPoint (id, values, labels[i]);
    }
    for (i = numValues; i < totalValues; i ++)
    {
        for (j = 0; j < numDataSets; j ++)
        {
            values[j] = ARCHIVE_VALUE_NULL;
        }

        multiChartAddPoint (id, values, labels[i]);
    }


    id->isMetric = isMetric;
    if (isDualUnits)
    {
        multiChartSetDualUnits (id);
    }
    else
    {
        strcpy (id->DualUnit, "");
    }

    multiChartRender (id);
    retVal = multiChartSave (id, fname);
    multiChartDestroy (id);

    return retVal;
}


////////////// D I A L S ///////////////
// General Dial Constants
#define DIAL_TICK_INNER     ((float)((plotPrefs.dialDiameter/2)-3))
#define DIAL_TICK_OUTER     ((float)((plotPrefs.dialDiameter/2)-1))
#define DIAL_TICK_LABEL     ((float)((plotPrefs.dialDiameter/2)-16))
#define DIAL_POINTER_TIP    DIAL_TICK_INNER

static char     dialPaletteFile[256];
static int      tickcolor, centercolor, centertext, centerhigh;
static int      bgcolor, dialcolor, pointcolor, highcolor, lowcolor, outlinecolor, appcolor;

// initialize the basic dial palette
int htmlGenPngDialInit (char *workingDir)
{
    gdImagePtr  im;
    FILE        *outFile;

    sprintf (dialPaletteFile, "%s/dialPalette.gd", workingDir);

    im = gdImageCreate (plotPrefs.dialImageWidth, plotPrefs.dialImageWidth);

    // create the colors
    tickcolor   = gdImageColorAllocateAlpha (im,
                  GLC_RED(plotPrefs.dialTextColor),
                  GLC_GREEN(plotPrefs.dialTextColor),
                  GLC_BLUE(plotPrefs.dialTextColor),
                  GLC_ALPHA(plotPrefs.dialTextColor));

    centercolor = gdImageColorAllocateAlpha (im,
                  GLC_RED(plotPrefs.dialCenterColor),
                  GLC_GREEN(plotPrefs.dialCenterColor),
                  GLC_BLUE(plotPrefs.dialCenterColor),
                  GLC_ALPHA(plotPrefs.dialCenterColor));

    centertext  = gdImageColorAllocateAlpha (im,
                  GLC_RED(plotPrefs.dialCenterTextColor),
                  GLC_GREEN(plotPrefs.dialCenterTextColor),
                  GLC_BLUE(plotPrefs.dialCenterTextColor),
                  GLC_ALPHA(plotPrefs.dialCenterTextColor));
    centerhigh  = gdImageColorAllocateAlpha (im,
                  GLC_RED(plotPrefs.dialCenterHighColor),
                  GLC_GREEN(plotPrefs.dialCenterHighColor),
                  GLC_BLUE(plotPrefs.dialCenterHighColor),
                  GLC_ALPHA(plotPrefs.dialCenterHighColor));
    bgcolor     = gdImageColorAllocateAlpha (im,
                  GLC_RED(plotPrefs.dialImageBgColor),
                  GLC_GREEN(plotPrefs.dialImageBgColor),
                  GLC_BLUE(plotPrefs.dialImageBgColor),
                  GLC_ALPHA(plotPrefs.dialImageBgColor));

    gdImageColorTransparent (im, bgcolor);

    if (plotPrefs.dialTransparent)
    {
        dialcolor = bgcolor;
    }
    else
    {
        dialcolor = gdImageColorAllocateAlpha (im,
                                               GLC_RED(plotPrefs.dialBgColor),
                                               GLC_GREEN(plotPrefs.dialBgColor),
                                               GLC_BLUE(plotPrefs.dialBgColor),
                                               GLC_ALPHA(plotPrefs.dialBgColor));
    }

    pointcolor  = gdImageColorAllocateAlpha (im,
                  GLC_RED(plotPrefs.dialPointerColor),
                  GLC_GREEN(plotPrefs.dialPointerColor),
                  GLC_BLUE(plotPrefs.dialPointerColor),
                  GLC_ALPHA(plotPrefs.dialPointerColor));

    highcolor   = gdImageColorAllocateAlpha (im,
                  GLC_RED(plotPrefs.dialHighColor),
                  GLC_GREEN(plotPrefs.dialHighColor),
                  GLC_BLUE(plotPrefs.dialHighColor),
                  GLC_ALPHA(plotPrefs.dialHighColor));

    lowcolor    = gdImageColorAllocateAlpha (im,
                  GLC_RED(plotPrefs.dialLowColor),
                  GLC_GREEN(plotPrefs.dialLowColor),
                  GLC_BLUE(plotPrefs.dialLowColor),
                  GLC_ALPHA(plotPrefs.dialLowColor));

    outlinecolor = gdImageColorAllocateAlpha (im,
                   GLC_RED(plotPrefs.dialPointerOutlineColor),
                   GLC_GREEN(plotPrefs.dialPointerOutlineColor),
                   GLC_BLUE(plotPrefs.dialPointerOutlineColor),
                   GLC_ALPHA(plotPrefs.dialPointerOutlineColor));

    appcolor    = gdImageColorAllocateAlpha (im,
                  GLC_RED(plotPrefs.dialAppColor),
                  GLC_GREEN(plotPrefs.dialAppColor),
                  GLC_BLUE(plotPrefs.dialAppColor),
                  GLC_BLUE(plotPrefs.dialAppColor));

    // OPTIMIZE - these two cost ~ 6% of total FULL generation
    gdImageFilledRectangle (im, 0, 0, plotPrefs.dialImageWidth, plotPrefs.dialImageWidth, bgcolor);
    gdImageFilledEllipse (im,
                          (plotPrefs.dialImageWidth/2),
                          (plotPrefs.dialImageWidth/2),
                          plotPrefs.dialDiameter,
                          plotPrefs.dialDiameter,
                          dialcolor);

    // Open output file in binary mode
    outFile = fopen (dialPaletteFile, "wb");
    if (outFile == NULL)
    {
        radMsgLog (PRI_HIGH, "htmlGenPngDialInit: fopen failed: %s",
                   strerror(errno));
        gdImageDestroy (im);
        return ERROR;
    }

    // Write gd format file
    gdImageGd (im, outFile);

    // Close file
    fclose (outFile);

    // Destroy image
    gdImageDestroy (im);

    return OK;
}

//////////////////////////////
///// Wind Dial //////////////
static struct xyCoords windTicks[8] =
    {
        {
            0.0, 1.0
        },
        { 0.7071067811, 0.7071067811 },
        { 1.0, 0.0 },
        { 0.7071067811, -0.7071067811 },
        { 0.0, -1.0 },
        { -0.7071067811, -0.7071067811 },
        { -1.0, 0.0 },
        { -0.7071067811, 0.7071067811 }
    };
static char *windLabels[8] =
    {
        "N", "NE", "E", "SE", "S", "SW", "W", "NW"
    };
static struct xyCoords windLabelLocations[8] =
    {
        {
            -0.04, 1.22
        },
        { 0.72, 0.86 },
        { 1.12, 0.10 },
        { 0.72, -0.66 },
        { -0.04, -1.02 },
        { -0.84, -0.66 },
        { -1.16, 0.10 },
        { -0.84, 0.86 }
    };

int htmlGenPngDialWind
(
    char                *fname,
    int                 direction,              // 0-359 degrees
    int                 highDirection,          // 0-359 degrees
    float               speedF,
    float               highSpeedF,
    char                *title
)
{
    register gdImagePtr im;
    gdPoint             points[3];
    int                 i, xpix, ypix, sticx, sticy, eticx, eticy;
    float               x, y;
    register double     angle, perpangle, cosine, sine;
    char                text[64];
    register FILE       *pngFile, *inFile;

    // suck in the dial palette
    inFile = fopen (dialPaletteFile, "rb");
    if (inFile == NULL)
    {
        return ERROR;
    }

    im = gdImageCreateFromGd (inFile);
    fclose (inFile);
    if (im == NULL)
    {
        return ERROR;
    }

    pngFile = fopen (fname, "wb");
    if (pngFile == NULL)
    {
        gdImageDestroy (im);
        return ERROR;
    }


    direction       = direction + 270;
    highDirection   = highDirection + 270;

    //  ... draw the current direction
    angle = (2.0*M_PI*direction)/360.0;
    perpangle = angle + M_PI_2;
    sine = sin(perpangle);
    cosine = cos(perpangle);

    points[0].x = (plotPrefs.dialImageWidth/2) + ((int)((plotPrefs.dialCtrDiameter/2) * cosine));
    points[0].y = (plotPrefs.dialImageWidth/2) + ((int)((plotPrefs.dialCtrDiameter/2) * sine));
    points[1].x = (plotPrefs.dialImageWidth/2) - ((int)((plotPrefs.dialCtrDiameter/2) * cosine));
    points[1].y = (plotPrefs.dialImageWidth/2) - ((int)((plotPrefs.dialCtrDiameter/2) * sine));
    points[2].x = (plotPrefs.dialImageWidth/2) + (int)(DIAL_POINTER_TIP * cos(angle));
    points[2].y = (plotPrefs.dialImageWidth/2) + (int)(DIAL_POINTER_TIP * sin(angle));


    //  ... draw a 3-sided polygon (triangle) for the dial pointer
    gdImageFilledPolygon (im, points, 3, pointcolor);
    gdImagePolygon (im, points, 3, outlinecolor);

    //  ... dial center and the windspeed
    gdImageFilledEllipse (im, (plotPrefs.dialImageWidth/2), (plotPrefs.dialImageWidth/2),
                          plotPrefs.dialCtrDiameter, plotPrefs.dialCtrDiameter, centercolor);

    sprintf (text, "%2.1f", wvutilsGetWindSpeed(speedF));

    // center it
    angle = plotPrefs.dialDiameter - (gdFontGiant->w * strlen (text));
    angle /= 2;
    angle += gdFontGiant->w/2;
    if (angle < 0)
        angle = 0;
    gdImageString (im, gdFontGiant, (int)angle, (plotPrefs.dialImageWidth/2)+32, (uint8_t *)text, centertext);
    sprintf (text, "%2.1f", wvutilsGetWindSpeed(highSpeedF));

    // center it
    angle = plotPrefs.dialDiameter - (gdFontSmall->w * strlen (text));
    angle /= 2;
    angle += gdFontSmall->w/2;
    if (angle < 0)
        angle = 0;
    gdImageString (im, gdFontSmall, (int)angle, (plotPrefs.dialImageWidth/2)+46, (uint8_t *)text, centerhigh);


    //  ... create the dial ticks and labels
    for (i = 0; i < 8; i ++)
    {
        xpix  = (plotPrefs.dialImageWidth/2) + (int)(DIAL_TICK_LABEL * windLabelLocations[i].x);
        ypix  = (plotPrefs.dialImageWidth/2) - (int)(DIAL_TICK_LABEL * windLabelLocations[i].y);
        sticx = (plotPrefs.dialImageWidth/2) + (int)(DIAL_TICK_INNER * windTicks[i].x);
        sticy = (plotPrefs.dialImageWidth/2) - (int)(DIAL_TICK_INNER * windTicks[i].y);
        eticx = (plotPrefs.dialImageWidth/2) + (int)(DIAL_TICK_OUTER * windTicks[i].x);
        eticy = (plotPrefs.dialImageWidth/2) - (int)(DIAL_TICK_OUTER * windTicks[i].y);

        gdImageSetThickness (im, 2);
        gdImageLine (im, sticx, sticy, eticx, eticy, tickcolor);
        gdImageString (im, gdFontSmall, xpix, ypix, (uint8_t *)windLabels[i], tickcolor);
    }


    //  ... draw the high wind speed tick
    angle = (2.0*M_PI*highDirection)/360.0;
    sine = sin(angle);
    cosine = cos(angle);
    sticx = (plotPrefs.dialImageWidth/2) + (int)((DIAL_TICK_INNER-1) * cosine);
    sticy = (plotPrefs.dialImageWidth/2) + (int)((DIAL_TICK_INNER-1) * sine);
    eticx = (plotPrefs.dialImageWidth/2) + (int)(DIAL_TICK_OUTER * cosine);
    eticy = (plotPrefs.dialImageWidth/2) + (int)(DIAL_TICK_OUTER * sine);

    gdImageSetThickness (im, 3);
    gdImageLine (im, sticx, sticy, eticx, eticy, highcolor);


    //  ... title
    sprintf (text, "%s", title);

    // center it
    angle = plotPrefs.dialDiameter - (gdFontGiant->w * strlen (text));
    angle /= 2;
    angle += gdFontGiant->w/2;
    if (angle < 0)
        angle = 0;

    gdImageString (im,
                   gdFontGiant,
                   (int)angle,
                   (plotPrefs.dialImageWidth/2)-52,
                   (uint8_t *)text,
                   tickcolor);

    gdImagePng (im, pngFile);
    fclose (pngFile);
    gdImageDestroy (im);

    return OK;
}


//////////////////////////////
///// Temperature Dial ///////
static struct xyCoords tempTicks[9] =
    {
        {
            0.0, -1.0
        }
        ,              // 0 angular degrees
        { -0.6428, -0.7660 },       // 40
        { -0.9848, -0.1736 },       // 80
        { -0.8660, 0.5 },           // 120
        { -0.3420, 0.9397 },        // 160
        { 0.3420, 0.9397 },         // 200
        { 0.8660, 0.5 },            // 240
        { 0.9848, -0.1736 },        // 280
        { 0.6428, -0.7660 }         // 320
    };
static char *tempLabelsF[9] =
    {
        "-40", "-20", "0", "20", "40", "60", "80", "100", "120"
    };

static char *tempLabelsC[9] =
    {
        "-30", "-20", "-10", "0", "10", "20", "30", "40", "50"
    };

static struct xyCoords tempLabelLocationsF[9] =
    {
        {
            0.0, -1.0
        }
        ,              // 0 angular degrees
        { -0.84, -0.74 },           // 40
        { -1.14, -0.10 },           // 80
        { -1.04, 0.64 },            // 120
        { -0.44, 1.16 },            // 160
        { 0.30, 1.16 },             // 200
        { 0.88, 0.64 },             // 240
        { 0.88, -0.10 },            // 280
        { 0.58, -0.74 }             // 320
    };

static struct xyCoords tempLabelLocationsC[9] =
    {
        {
            0.0, -1.0
        }
        ,              // 0 angular degrees
        { -0.84, -0.74 },           // 40
        { -1.12, -0.10 },           // 80
        { -1.0, 0.68 },             // 120
        { -0.44, 1.16 },            // 160
        { 0.30, 1.16 },             // 200
        { 0.88, 0.64 },             // 240
        { 0.98, -0.10 },            // 280
        { 0.60, -0.74 }             // 320
    };

int htmlGenPngDialTemperature
(
    char                *fname,
    int                 isMetricUnits,
    float               temperature,        // -40 to 140 degrees, 180 total, always degrees F
    float               low,
    float               high,
    float               feelsLike,
    char                *title,
    char                *units
)
{
    register gdImagePtr im;
    gdPoint             points[3];
    int                 i, xpix, ypix, sticx, sticy, eticx, eticy;
    float               x, y;
    float               degAngle, angle, perpangle, cosine, sine;
    char                text[64];
    float               baseTemp, mult;
    register FILE       *pngFile, *inFile;

    // suck in the dial palette
    inFile = fopen (dialPaletteFile, "rb");
    if (inFile == NULL)
    {
        return ERROR;
    }

    im = gdImageCreateFromGd (inFile);
    fclose (inFile);
    if (im == NULL)
    {
        return ERROR;
    }

    pngFile = fopen (fname, "wb");
    if (pngFile == NULL)
    {
        return ERROR;
    }

    // prepare to plot in metric or English
    if (isMetricUnits)
    {
        temperature = wvutilsConvertFToC (temperature);
        low         = wvutilsConvertFToC (low);
        high        = wvutilsConvertFToC (high);
        if (feelsLike != DIAL_TEMP_FEELSLIKE_DISABLE)
            feelsLike = wvutilsConvertFToC (feelsLike);
        baseTemp    = 30;     // - minimum temperature
        mult        = 4;          // degrees of circle per degree of temperature
    }
    else
    {
        baseTemp    = 40;
        mult        = 2;
    }

    // normalize the values given
    temperature   += baseTemp;
    low           += baseTemp;
    high          += baseTemp;
    if (feelsLike != DIAL_TEMP_FEELSLIKE_DISABLE)
        feelsLike += baseTemp;

    if (feelsLike != DIAL_TEMP_FEELSLIKE_DISABLE)
    {
        //  ... draw the current apparent temp
        degAngle = (mult * feelsLike) + 90;
        angle = (2 * M_PI * degAngle) / 360.0;
        perpangle = angle + M_PI_2;
        sine = sin(perpangle);
        cosine = cos(perpangle);

        points[0].x = (plotPrefs.dialImageWidth/2) +
                      ((int)(((float)(plotPrefs.dialCtrDiameter/2)) * cosine));
        points[0].y = (plotPrefs.dialImageWidth/2) +
                      ((int)(((float)(plotPrefs.dialCtrDiameter/2)) * sine));
        points[1].x = (plotPrefs.dialImageWidth/2) -
                      ((int)(((float)(plotPrefs.dialCtrDiameter/2)) * cosine));
        points[1].y = (plotPrefs.dialImageWidth/2) -
                      ((int)(((float)(plotPrefs.dialCtrDiameter/2)) * sine));
        points[2].x = (plotPrefs.dialImageWidth/2) + (int)((DIAL_POINTER_TIP-2) * cos(angle));
        points[2].y = (plotPrefs.dialImageWidth/2) + (int)((DIAL_POINTER_TIP-2) * sin(angle));


        //  ... draw a 3-sided polygon (triangle) for the apparent pointer
        gdImageFilledPolygon (im, points, 3, appcolor);
    }

    //  ... draw the current temperature
    degAngle = (mult * temperature) + 90;
    angle = (2 * M_PI * degAngle) / 360.0;
    perpangle = angle + M_PI_2;
    sine = sin(perpangle);
    cosine = cos(perpangle);

    points[0].x = (plotPrefs.dialImageWidth/2) +
                  ((int)(((float)(plotPrefs.dialCtrDiameter/2)) * cosine));
    points[0].y = (plotPrefs.dialImageWidth/2) +
                  ((int)(((float)(plotPrefs.dialCtrDiameter/2)) * sine));
    points[1].x = (plotPrefs.dialImageWidth/2) -
                  ((int)(((float)(plotPrefs.dialCtrDiameter/2)) * cosine));
    points[1].y = (plotPrefs.dialImageWidth/2) -
                  ((int)(((float)(plotPrefs.dialCtrDiameter/2)) * sine));
    points[2].x = (plotPrefs.dialImageWidth/2) + (int)(DIAL_POINTER_TIP * cos(angle));
    points[2].y = (plotPrefs.dialImageWidth/2) + (int)(DIAL_POINTER_TIP * sin(angle));


    //  ... draw a 3-sided polygon (triangle) for the dial pointer
    gdImageFilledPolygon (im, points, 3, pointcolor);
    gdImagePolygon (im, points, 3, outlinecolor);

    //  ... dial center and the text
    gdImageFilledEllipse (im, (plotPrefs.dialImageWidth/2), (plotPrefs.dialImageWidth/2),
                          plotPrefs.dialCtrDiameter, plotPrefs.dialCtrDiameter, centercolor);

    sprintf (text, "%.1f", temperature - baseTemp);
    // center it
    degAngle = plotPrefs.dialDiameter - (gdFontGiant->w * strlen (text));
    degAngle /= 2;
    degAngle += gdFontGiant->w/2;
    if (degAngle < 0)
        degAngle = 0;
    gdImageString (im,
                   gdFontGiant,
                   (int)degAngle,
                   (plotPrefs.dialImageWidth/2)+32,
                   (uint8_t *)text,
                   centertext);

    if (feelsLike != DIAL_TEMP_FEELSLIKE_DISABLE)
    {
        sprintf (text, "%.1f", feelsLike - baseTemp);
        // center it
        degAngle = plotPrefs.dialDiameter - (gdFontSmall->w * strlen (text));
        degAngle /= 2;
        degAngle += gdFontSmall->w/2;
        if (degAngle < 0)
            degAngle = 0;
        gdImageString (im,
                       gdFontSmall,
                       (int)degAngle,
                       (plotPrefs.dialImageWidth/2)+46,
                       (uint8_t *)text,
                       appcolor);
    }

    //  ... create the dial ticks and labels
    for (i = 1; i < 9; i ++)
    {
        if (isMetricUnits)
        {
            xpix  = (plotPrefs.dialImageWidth/2) + (int)(DIAL_TICK_LABEL * tempLabelLocationsC[i].x);
            ypix  = (plotPrefs.dialImageWidth/2) - (int)(DIAL_TICK_LABEL * tempLabelLocationsC[i].y);
        }
        else
        {
            xpix  = (plotPrefs.dialImageWidth/2) + (int)(DIAL_TICK_LABEL * tempLabelLocationsF[i].x);
            ypix  = (plotPrefs.dialImageWidth/2) - (int)(DIAL_TICK_LABEL * tempLabelLocationsF[i].y);
        }

        sticx = (plotPrefs.dialImageWidth/2) + (int)(DIAL_TICK_INNER * tempTicks[i].x);
        sticy = (plotPrefs.dialImageWidth/2) - (int)(DIAL_TICK_INNER * tempTicks[i].y);
        eticx = (plotPrefs.dialImageWidth/2) + (int)(DIAL_TICK_OUTER * tempTicks[i].x);
        eticy = (plotPrefs.dialImageWidth/2) - (int)(DIAL_TICK_OUTER * tempTicks[i].y);

        gdImageSetThickness (im, 2);
        gdImageLine (im, sticx, sticy, eticx, eticy, tickcolor);
        if (isMetricUnits)
        {
            gdImageString (im, gdFontSmall, xpix, ypix, (uint8_t *)tempLabelsC[i], tickcolor);
        }
        else
        {
            gdImageString (im, gdFontSmall, xpix, ypix, (uint8_t *)tempLabelsF[i], tickcolor);
        }
    }

    //  ... low
    degAngle = (mult * low) + 90;
    angle = (2 * M_PI * degAngle) / 360.0;
    sine = sin(angle);
    cosine = cos(angle);

    sticx = (plotPrefs.dialImageWidth/2) + (int)((DIAL_TICK_INNER-1) * cosine);
    sticy = (plotPrefs.dialImageWidth/2) + (int)((DIAL_TICK_INNER-1) * sine);
    eticx = (plotPrefs.dialImageWidth/2) + (int)(DIAL_TICK_OUTER * cosine);
    eticy = (plotPrefs.dialImageWidth/2) + (int)(DIAL_TICK_OUTER * sine);

    gdImageSetThickness (im, 3);
    gdImageLine (im, sticx, sticy, eticx, eticy, lowcolor);

    //  ... high
    degAngle = (mult * high) + 90;
    angle = (2 * M_PI * degAngle) / 360.0;
    sine = sin(angle);
    cosine = cos(angle);

    sticx = (plotPrefs.dialImageWidth/2) + (int)((DIAL_TICK_INNER-1) * cosine);
    sticy = (plotPrefs.dialImageWidth/2) + (int)((DIAL_TICK_INNER-1) * sine);
    eticx = (plotPrefs.dialImageWidth/2) + (int)(DIAL_TICK_OUTER * cosine);
    eticy = (plotPrefs.dialImageWidth/2) + (int)(DIAL_TICK_OUTER * sine);

    gdImageSetThickness (im, 3);
    gdImageLine (im, sticx, sticy, eticx, eticy, highcolor);


    //  ... title and units
    sprintf (text, "%s %s", title, units);

    // center it
    degAngle = plotPrefs.dialDiameter - (gdFontGiant->w * strlen (text));
    degAngle /= 2;
    degAngle += gdFontGiant->w/2;
    if (degAngle < 0)
        degAngle = 0;

    gdImageString (im,
                   gdFontGiant,
                   (int)degAngle,
                   (plotPrefs.dialImageWidth/2)-52,
                   (uint8_t *)text,
                   tickcolor);

    gdImagePng (im, pngFile);
    fclose (pngFile);
    gdImageDestroy (im);

    return OK;
}

//////////////////////////////
///// Humidity Dial //////////
static struct xyCoords himidTicks[12] =
    {
        {
            0.0, -1.0
        }
        ,              // 0 angular degrees
        { -0.5, -0.8660 },          // 30
        { -0.8660, -0.5 },          // 60
        { -1.0, 0.0 },              // 90
        { -0.8660, 0.5 },           // 120
        { -0.5, 0.8660 },           // 150
        { 0.0, 1.0 },               // 180
        { 0.5, 0.8660 },            // 210
        { 0.8660, 0.5 },            // 240
        { 1.0, 0.0 },               // 270
        { 0.8660, -0.5 },           // 300
        { 0.5, -0.8660 }            // 330
    };
static char *humidLabels[12] =
    {
        "-10", "0", "10", "20", "30", "40", "50", "60", "70", "80", "90", "100"
    };
static struct xyCoords humidLabelLocations[12] =
    {
        {
            0.0, -1.0
        }
        ,              // 0 angular degrees
        { -0.58, -0.86 },           // 30
        { -1.0, -0.43 },            // 60
        { -1.18, 0.12 },            // 90
        { -1.0, 0.66 },             // 120
        { -0.62, 1.06 },            // 150
        { -0.10, 1.24 },            // 180
        { 0.46, 1.06 },             // 210
        { 0.86, 0.66 },             // 240
        { 1.0, 0.12 },              // 270
        { 0.86, -0.43 },            // 300
        { 0.4, -0.86 }              // 330
    };

int htmlGenPngDialHumidity
(
    char                *fname,
    float               humidity,        // 0 to 100 %, 100 total
    float               low,
    float               high,
    char                *title
)
{
    register gdImagePtr im;
    gdPoint             points[3];
    int                 i, xpix, ypix, sticx, sticy, eticx, eticy;
    float               x, y;
    float               degAngle, angle, perpangle, cosine, sine;
    char                text[64];
    register FILE       *pngFile, *inFile;

    // suck in the dial palette
    inFile = fopen (dialPaletteFile, "rb");
    if (inFile == NULL)
    {
        return ERROR;
    }

    im = gdImageCreateFromGd (inFile);
    fclose (inFile);
    if (im == NULL)
    {
        return ERROR;
    }

    pngFile = fopen (fname, "wb");
    if (pngFile == NULL)
    {
        return ERROR;
    }

    // normalize the values given
    humidity    += 10;
    low         += 10;
    high        += 10;

    //  ... draw the current temperature
    degAngle = (3 * humidity) + 90;
    angle = (2 * M_PI * degAngle) / 360.0;
    perpangle = angle + M_PI_2;
    sine = sin(perpangle);
    cosine = cos(perpangle);

    points[0].x = (plotPrefs.dialImageWidth/2) +
                  ((int)(((float)(plotPrefs.dialCtrDiameter/2)) * cosine));
    points[0].y = (plotPrefs.dialImageWidth/2) +
                  ((int)(((float)(plotPrefs.dialCtrDiameter/2)) * sine));
    points[1].x = (plotPrefs.dialImageWidth/2) -
                  ((int)(((float)(plotPrefs.dialCtrDiameter/2)) * cosine));
    points[1].y = (plotPrefs.dialImageWidth/2) -
                  ((int)(((float)(plotPrefs.dialCtrDiameter/2)) * sine));
    points[2].x = (plotPrefs.dialImageWidth/2) + (int)(DIAL_POINTER_TIP * cos(angle));
    points[2].y = (plotPrefs.dialImageWidth/2) + (int)(DIAL_POINTER_TIP * sin(angle));


    //  ... draw a 3-sided polygon (triangle) for the dial pointer
    gdImageFilledPolygon (im, points, 3, pointcolor);
    gdImagePolygon(im, points, 3, outlinecolor);


    //  ... dial center and the text
    gdImageFilledEllipse(im, (plotPrefs.dialImageWidth/2), (plotPrefs.dialImageWidth/2),
                         plotPrefs.dialCtrDiameter, plotPrefs.dialCtrDiameter, centercolor);

    sprintf (text, "%.0f", humidity-10);
    // center it
    degAngle = plotPrefs.dialDiameter - (gdFontGiant->w * strlen (text));
    degAngle /= 2;
    degAngle += gdFontGiant->w/2;
    if (degAngle < 0)
        degAngle = 0;
    gdImageString (im,
                   gdFontGiant,
                   (int)degAngle,
                   (plotPrefs.dialImageWidth/2)+32,
                   (uint8_t *)text,
                   centertext);


    //  ... create the dial ticks and labels
    for (i = 1; i < 12; i ++)
    {
        xpix  = (plotPrefs.dialImageWidth/2) + (int)(DIAL_TICK_LABEL * humidLabelLocations[i].x);
        ypix  = (plotPrefs.dialImageWidth/2) - (int)(DIAL_TICK_LABEL * humidLabelLocations[i].y);
        sticx = (plotPrefs.dialImageWidth/2) + (int)(DIAL_TICK_INNER * himidTicks[i].x);
        sticy = (plotPrefs.dialImageWidth/2) - (int)(DIAL_TICK_INNER * himidTicks[i].y);
        eticx = (plotPrefs.dialImageWidth/2) + (int)(DIAL_TICK_OUTER * himidTicks[i].x);
        eticy = (plotPrefs.dialImageWidth/2) - (int)(DIAL_TICK_OUTER * himidTicks[i].y);

        gdImageSetThickness (im, 2);
        gdImageLine (im, sticx, sticy, eticx, eticy, tickcolor);
        gdImageString (im, gdFontSmall, xpix, ypix, (uint8_t *)humidLabels[i], tickcolor);
    }


    //  ... low
    degAngle = (3 * low) + 90;
    angle = (2 * M_PI * degAngle) / 360.0;
    sine = sin(angle);
    cosine = cos(angle);

    sticx = (plotPrefs.dialImageWidth/2) + (int)((DIAL_TICK_INNER-1) * cosine);
    sticy = (plotPrefs.dialImageWidth/2) + (int)((DIAL_TICK_INNER-1) * sine);
    eticx = (plotPrefs.dialImageWidth/2) + (int)(DIAL_TICK_OUTER * cosine);
    eticy = (plotPrefs.dialImageWidth/2) + (int)(DIAL_TICK_OUTER * sine);

    gdImageSetThickness (im, 3);
    gdImageLine (im, sticx, sticy, eticx, eticy, lowcolor);

    //  ... high
    degAngle = (3 * high) + 90;
    angle = (2 * M_PI * degAngle) / 360.0;
    sine = sin(angle);
    cosine = cos(angle);

    sticx = (plotPrefs.dialImageWidth/2) + (int)((DIAL_TICK_INNER-1) * cosine);
    sticy = (plotPrefs.dialImageWidth/2) + (int)((DIAL_TICK_INNER-1) * sine);
    eticx = (plotPrefs.dialImageWidth/2) + (int)(DIAL_TICK_OUTER * cosine);
    eticy = (plotPrefs.dialImageWidth/2) + (int)(DIAL_TICK_OUTER * sine);

    gdImageSetThickness (im, 3);
    gdImageLine (im, sticx, sticy, eticx, eticy, highcolor);


    //  ... title and units
    sprintf (text, "%s", title);

    // center it
    degAngle = plotPrefs.dialDiameter - (gdFontGiant->w * strlen (text));
    degAngle /= 2;
    degAngle += gdFontGiant->w/2;
    if (degAngle < 0)
        degAngle = 0;

    gdImageString (im,
                   gdFontGiant,
                   (int)degAngle,
                   (plotPrefs.dialImageWidth/2)-52,
                   (uint8_t *)text,
                   tickcolor);

    gdImagePng (im, pngFile);
    fclose (pngFile);
    gdImageDestroy (im);

    return OK;
}
//////////////////////////////
/// Day Net Rain Dial ////////
static struct xyCoords netrainTicks[12] =
    {
        { 0.0, -1.0 },              // 0 angular degrees
        { -0.5, -0.8660 },          // 30
        { -0.8660, -0.5 },          // 60
        { -1.0, 0.0 },              // 90
        { -0.8660, 0.5 },           // 120
        { -0.5, 0.8660 },           // 150
        { 0.0, 1.0 },               // 180
        { 0.5, 0.8660 },            // 210
        { 0.8660, 0.5 },            // 240
        { 1.0, 0.0 },               // 270
        { 0.8660, -0.5 },           // 300
        { 0.5, -0.8660 }            // 330
    };
static char *netrainLabels0C[17] =
    {
        "-3.0", "-2.5", "-2.0", "-1.5", "-1.0", "-0.5", "0", "0.5", "1.0", "1.5", "2.0", "2.5", "3.0", "3.5", "4.0", "4.5", "5.0"
    };
static char *netrainLabels0F[17] =
    {
        "-1.2", "-1.0", "-0.8", "-0.6", "-0.4", "-0.2", "0", "0.2", "0.4", "0.6", "0.8", "1.0", "1.2", "1.4", "1.6", "1.8", "2.0"
    };
static char *netrainLabels1C[17] =
    {
        "-30 ", "-25 ", "-20 ", "-15 ", "-10 ", "-5  ", "0", " 5 ", " 10", " 15", " 20", " 25", " 30", " 35", " 40", " 45", " 50"
    };
static char *netrainLabels1F[17] =
    {
        "-12 ", "-10 ", "-8  ", "-6  ", "-4  ", "-2  ", "0", " 2 ", " 4 ", " 6 ", " 8 ", " 10", " 12", " 14", " 16", " 18", " 20"
    };
static char *netrainLabels2C[17] =
    {
        "-60 ", "-50 ", "-40 ", "-30 ", "-20 ", "-10 ", "0", " 10", " 20", " 30", " 40", " 50", " 60", " 70", " 80", " 90", "100"
    };
static char *netrainLabels2F[17] =
    {
        "-24 ", "-20 ", "-16 ", "-12 ", "-8  ", "-4  ", "0", " 4 ", " 8 ", " 12", " 16", " 20", " 24", " 28", " 32", " 36", " 40"
    };
static char *netrainLabels3C[17] =
    {
        "-120", "-100", "-80 ", "-60 ", "-40 ", "-20 ", "0", " 20", " 40", " 60", " 80", "100", "120", "140", "160", "180", "200"
    };
static char *netrainLabels3F[17] =
    {
        "-60 ", "-50 ", "-40 ", "-30 ", "-20 ", "-10 ", "0", " 10", " 20", " 30", " 40", " 50", " 60", " 70", " 80", " 90", "100"
    };
static char *netrainLabels4C[17] =
    {
        "-240", "-200", "-160", "-120", "-80 ", "-40 ", "0", " 40", " 80", "120", "160", "200", "240", "280", "320", "360", "400"
    };
static char *netrainLabels4F[17] =
    {
        "-120", "-100", "-80 ", "-60 ", "-40 ", "-20 ", "0", " 20", " 40", " 60", " 80", "100", "120", "140", "160", "180", "200"
    };
static char *netrainLabels5C[17] =
    {
        "-600", "-500", "-400", "-300", "-200", "-100", "0", "100", "200", "300", "400", "500", "600", "700", "800", "900", "1000"
    };
static char *netrainLabels5F[17] =
    {
        "-240", "-200", "-160", "-120", "-80 ", "-40 ", "0", " 40", " 80", "120", "160", "200", "240", "280", "320", "360", "400"
    };
static char *netrainLabels6C[17] =
    {
        "-1200", "-1000", "-800", "-600", "-400", "-200 ", "0", "200", "400", "600", "800", "1000", "1200", "1400", "1600", "1800", "2000"
    };
static char *netrainLabels7C[17] =
    {
        "-6000", "-5000", "-4000", "-3000", "-2000", "-1000", "0", "1000", "2000", "3000", "4000", "5000", "6000", "7000", "8000", "9000", "10000"
    };

static float netrainMinsC[2][8] = { { 0, 0, 0, 0, 0, 0, 0, 0 }, { -2.5, -25, -50, -100, -200, -500, -1000, -5000 } };
static float netrainMaxsC[2][8] = { { 5, 50, 100, 200, 400, 1000, 2000, 10000 }, { 2.5, 25, 50, 100, 200, 500, 1000, 5000 } };

static float netrainMinsF[2][6] = { { 0, 0, 0, 0, 0, 0 }, { -1, -10, -20, -50, -100, -200 } };
static float netrainMaxsF[2][6] = { { 2, 20, 40, 100, 200, 400 }, { 1, 10, 20, 50, 100, 200 } };

#define NETRAIN_ET_START            0
#define NETRAIN_NO_ET_START         5
#define NETRAIN_START(x)            ((x > 0) ? NETRAIN_ET_START : NETRAIN_NO_ET_START)


static struct xyCoords netrainLabelLocations[2][12] =
    {
        {
            { 0.0, -1.0 },              // 0 angular degrees
            { -0.62, -0.86 },           // 30
            { -1.0, -0.43 },            // 60
            { -1.18, 0.12 },            // 90
            { -1.0, 0.66 },             // 120
            { -0.62, 1.06 },            // 150
            { -0.14, 1.22 },            // 180
            { 0.32, 1.06 },             // 210
            { 0.76, 0.66 },             // 240
            { 0.92, 0.12 },             // 270
            { 0.76, -0.43 },            // 300
            { 0.32, -0.86 }             // 330
        },
        {
            { 0.0, -1.0 },              // 0 angular degrees
            { -0.62, -0.86 },           // 30
            { -1.0, -0.43 },            // 60
            { -1.18, 0.12 },            // 90
            { -1.0, 0.66 },             // 120
            { -0.62, 1.06 },            // 150
            { -0.04, 1.22 },            // 180
            { 0.32, 1.06 },             // 210
            { 0.76, 0.66 },             // 240
            { 0.92, 0.12 },             // 270
            { 0.76, -0.43 },            // 300
            { 0.32, -0.86 }             // 330
        }
    };

int htmlGenPngDialNetRain
(
    char                *fname,
    int                 isExtendedData,
    int                 isMetricUnits,
    float               rain,
    float               et,
    char                *title,
    char                *units
)
{
    register gdImagePtr im;
    gdPoint             points[3];
    int                 i, xpix, ypix, sticx, sticy, eticx, eticy;
    float               x, y;
    float               degAngle, angle, perpangle, cosine, sine;
    char                text[64];
    float               baserain, mult, precip, minval, maxval;
    char                *labels[12];
    register FILE       *pngFile, *inFile;
    int                 IsET = (((isExtendedData) && (et > 0)) ? 1 : 0);

    // suck in the dial palette
    inFile = fopen (dialPaletteFile, "rb");
    if (inFile == NULL)
    {
        return ERROR;
    }

    im = gdImageCreateFromGd (inFile);
    fclose (inFile);
    if (im == NULL)
    {
        return ERROR;
    }

    if (et < 0)
    {
        et = 0;
    }
    precip = rain - et;
    minval = MIN(rain,precip);
    maxval = MAX(rain,precip);
    if (minval < 0 && ! IsET)
    {
        minval = 0;
    }

    pngFile = fopen (fname, "wb");
    if (pngFile == NULL)
    {
        return ERROR;
    }

    // prepare to plot in metric or English
    if (isMetricUnits)
    {
        rain = wvutilsConvertRainINToMetric (rain);
        precip = wvutilsConvertRainINToMetric (precip);
        minval = MIN(rain,precip);
        maxval = MAX(rain,precip);
        if (minval < 0 && ! IsET)
        {
            minval = 0;
        }

        // handle both cm and mm units here
        if (netrainMinsC[IsET][0] <= minval && maxval <= netrainMaxsC[IsET][0])
        {
            baserain = netrainMinsC[IsET][0] - 0.5;     // - minimum rain
            mult = 60;                                  // degrees per unit
            for (i = NETRAIN_START(IsET); i < NETRAIN_START(IsET) + 12; i ++)
            {
                labels[i-NETRAIN_START(IsET)] = netrainLabels0C[i];
            }
        }
        else if (netrainMinsC[IsET][1] <= minval && maxval <= netrainMaxsC[IsET][1])
        {
            baserain = netrainMinsC[IsET][1] - 5;       // - minimum rain
            mult = 6;                                   // degrees per unit
            for (i = NETRAIN_START(IsET); i < NETRAIN_START(IsET) + 12; i ++)
            {
                labels[i-NETRAIN_START(IsET)] = netrainLabels1C[i];
            }
        }
        else if (netrainMinsC[IsET][2] <= minval && maxval <= netrainMaxsC[IsET][2])
        {
            baserain = netrainMinsC[IsET][2] - 10;      // - minimum rain
            mult = 3;                                   // degrees per unit
            for (i = NETRAIN_START(IsET); i < NETRAIN_START(IsET) + 12; i ++)
            {
                labels[i-NETRAIN_START(IsET)] = netrainLabels2C[i];
            }
        }
        else if (netrainMinsC[IsET][3] <= minval && maxval <= netrainMaxsC[IsET][3])
        {
            baserain = netrainMinsC[IsET][3] - 20;      // - minimum rain
            mult = 1.5;                                 // degrees per unit
            for (i = NETRAIN_START(IsET); i < NETRAIN_START(IsET) + 12; i ++)
            {
                labels[i-NETRAIN_START(IsET)] = netrainLabels3C[i];
            }
        }
        else if (netrainMinsC[IsET][4] <= minval && maxval <= netrainMaxsC[IsET][4])
        {
            baserain = netrainMinsC[IsET][4] - 40;      // - minimum rain
            mult = 0.75;                                // degrees per unit
            for (i = NETRAIN_START(IsET); i < NETRAIN_START(IsET) + 12; i ++)
            {
                labels[i-NETRAIN_START(IsET)] = netrainLabels4C[i];
            }
        }
        else if (netrainMinsC[IsET][5] <= minval && maxval <= netrainMaxsC[IsET][5])
        {
            baserain = netrainMinsC[IsET][5] - 100;     // - minimum rain
            mult = 0.3;                                 // degrees per unit
            for (i = NETRAIN_START(IsET); i < NETRAIN_START(IsET) + 12; i ++)
            {
                labels[i-NETRAIN_START(IsET)] = netrainLabels5C[i];
            }
        }
        else if (netrainMinsC[IsET][6] <= minval && maxval <= netrainMaxsC[IsET][6])
        {
            baserain = netrainMinsC[IsET][6] - 200;     // - minimum rain
            mult = 0.15;                                // degrees per unit
            for (i = NETRAIN_START(IsET); i < NETRAIN_START(IsET) + 12; i ++)
            {
                labels[i-NETRAIN_START(IsET)] = netrainLabels6C[i];
            }
        }
        else
        {
            baserain = netrainMinsC[IsET][7] - 1000;    // - minimum rain
            mult = 0.03;                                // degrees per unit
            for (i = NETRAIN_START(IsET); i < NETRAIN_START(IsET) + 12; i ++)
            {
                labels[i-NETRAIN_START(IsET)] = netrainLabels7C[i];
            }
        }
    }
    else
    {
        if (netrainMinsF[IsET][0] <= minval && maxval <= netrainMaxsF[IsET][0])
        {
            baserain = netrainMinsF[IsET][0] - 0.2;     // - minimum rain
            mult = 150;                                 // degrees per inch
            for (i = NETRAIN_START(IsET); i < NETRAIN_START(IsET) + 12; i ++)
            {
                labels[i-NETRAIN_START(IsET)] = netrainLabels0F[i];
            }
        }
        else if (netrainMinsF[IsET][1] <= minval && maxval <= netrainMaxsF[IsET][1])
        {
            baserain = netrainMinsF[IsET][1] - 2;       // - minimum rain
            mult = 15;                                  // degrees per inch
            for (i = NETRAIN_START(IsET); i < NETRAIN_START(IsET) + 12; i ++)
            {
                labels[i-NETRAIN_START(IsET)] = netrainLabels1F[i];
            }
        }
        else if (netrainMinsF[IsET][2] <= minval && maxval <= netrainMaxsF[IsET][2])
        {
            baserain = netrainMinsF[IsET][2] - 4;       // - minimum rain
            mult = 7.5;                                 // degrees per inch
            for (i = NETRAIN_START(IsET); i < NETRAIN_START(IsET) + 12; i ++)
            {
                labels[i-NETRAIN_START(IsET)] = netrainLabels2F[i];
            }
        }
        else if (netrainMinsF[IsET][3] <= minval && maxval <= netrainMaxsF[IsET][3])
        {
            baserain = netrainMinsF[IsET][3] - 10;      // - minimum rain
            mult = 3;                                   // degrees per inch
            for (i = NETRAIN_START(IsET); i < NETRAIN_START(IsET) + 12; i ++)
            {
                labels[i-NETRAIN_START(IsET)] = netrainLabels3F[i];
            }
        }
        else if (netrainMinsF[IsET][4] <= minval && maxval <= netrainMaxsF[IsET][4])
        {
            baserain = netrainMinsF[IsET][4] - 20;      // - minimum rain
            mult = 1.5;                                 // degrees per inch
            for (i = NETRAIN_START(IsET); i < NETRAIN_START(IsET) + 12; i ++)
            {
                labels[i-NETRAIN_START(IsET)] = netrainLabels4F[i];
            }
        }
        else
        {
            baserain = netrainMinsF[IsET][5] - 40;      // - minimum rain
            mult = 0.75;                                // degrees per inch
            for (i = NETRAIN_START(IsET); i < NETRAIN_START(IsET) + 12; i ++)
            {
                labels[i-NETRAIN_START(IsET)] = netrainLabels5F[i];
            }
        }
    }

    // normalize the values given
    rain += baserain;
    precip += baserain;

    //  ... draw the net rainfall if there is a value for ET
    if (IsET)
    {
        degAngle = (mult * precip) + 90;
        angle = (2 * M_PI * degAngle) / 360.0;
        perpangle = angle + M_PI_2;
        sine = sin(perpangle);
        cosine = cos(perpangle);

        points[0].x = (plotPrefs.dialImageWidth/2) +
                      ((int)(((float)(plotPrefs.dialCtrDiameter/2)) * cosine));
        points[0].y = (plotPrefs.dialImageWidth/2) +
                      ((int)(((float)(plotPrefs.dialCtrDiameter/2)) * sine));
        points[1].x = (plotPrefs.dialImageWidth/2) -
                      ((int)(((float)(plotPrefs.dialCtrDiameter/2)) * cosine));
        points[1].y = (plotPrefs.dialImageWidth/2) -
                      ((int)(((float)(plotPrefs.dialCtrDiameter/2)) * sine));
        points[2].x = (plotPrefs.dialImageWidth/2) + (int)((DIAL_POINTER_TIP-2) * cos(angle));
        points[2].y = (plotPrefs.dialImageWidth/2) + (int)((DIAL_POINTER_TIP-2) * sin(angle));

        //  ... draw a 3-sided polygon (triangle) for the net precip dial pointer
        gdImageFilledPolygon (im, points, 3, appcolor);
    }


    //  ... draw the gross rainfall
    if (IsET)
    {
        degAngle = (mult * rain) + 90;
    }
    else
    {
        degAngle = (mult * rain) + 150;
    }
    angle = (2 * M_PI * degAngle) / 360.0;
    perpangle = angle + M_PI_2;
    sine = sin(perpangle);
    cosine = cos(perpangle);

    points[0].x = (plotPrefs.dialImageWidth/2) +
                  ((int)(((float)(plotPrefs.dialCtrDiameter/2)) * cosine));
    points[0].y = (plotPrefs.dialImageWidth/2) +
                  ((int)(((float)(plotPrefs.dialCtrDiameter/2)) * sine));
    points[1].x = (plotPrefs.dialImageWidth/2) -
                  ((int)(((float)(plotPrefs.dialCtrDiameter/2)) * cosine));
    points[1].y = (plotPrefs.dialImageWidth/2) -
                  ((int)(((float)(plotPrefs.dialCtrDiameter/2)) * sine));
    points[2].x = (plotPrefs.dialImageWidth/2) + (int)(DIAL_POINTER_TIP * cos(angle));
    points[2].y = (plotPrefs.dialImageWidth/2) + (int)(DIAL_POINTER_TIP * sin(angle));

    //  ... draw a 3-sided polygon (triangle) for the gross rain dial pointer
    gdImageFilledPolygon (im, points, 3, pointcolor);
    gdImagePolygon (im, points, 3, outlinecolor);

    //  ... dial center and the text
    gdImageFilledEllipse (im, (plotPrefs.dialImageWidth/2), (plotPrefs.dialImageWidth/2),
                          plotPrefs.dialCtrDiameter, plotPrefs.dialCtrDiameter, centercolor);

    if (isMetricUnits && wvutilsGetRainIsMM())
        sprintf (text, "%.1f %s", rain - baserain, units);
    else
        sprintf (text, "%.2f %s", rain - baserain, units);

    // center it
    degAngle = plotPrefs.dialDiameter - (gdFontGiant->w * strlen (text));
    degAngle /= 2;
    degAngle += gdFontGiant->w/2;
    if (degAngle < 0)
        degAngle = 0;
    gdImageString (im,
                   gdFontGiant,
                   (int)degAngle,
                   (plotPrefs.dialImageWidth/2)+28,
                   (uint8_t *)text,
                   centertext);

    if (IsET)
    {
        if (isMetricUnits && wvutilsGetRainIsMM())
            sprintf (text, "%.1f %s", precip - baserain, units);
        else
            sprintf (text, "%.2f %s", precip - baserain, units);

        // center it
        degAngle = plotPrefs.dialDiameter - (gdFontSmall->w * strlen (text));
        degAngle /= 2;
        degAngle += gdFontSmall->w/2;
        if (degAngle < 0)
            degAngle = 0;
        gdImageString (im,
                       gdFontSmall,
                       (int)degAngle,
                       (plotPrefs.dialImageWidth/2)+42,
                       (uint8_t *)text,
                       appcolor);
    }


    //  ... create the dial ticks and labels
    for (i = 1; i < 12; i ++)
    {
        xpix  = (plotPrefs.dialImageWidth/2) + (int)(DIAL_TICK_LABEL * netrainLabelLocations[IsET][i].x);
        ypix  = (plotPrefs.dialImageWidth/2) - (int)(DIAL_TICK_LABEL * netrainLabelLocations[IsET][i].y);

        sticx = (plotPrefs.dialImageWidth/2) + (int)(DIAL_TICK_INNER * netrainTicks[i].x);
        sticy = (plotPrefs.dialImageWidth/2) - (int)(DIAL_TICK_INNER * netrainTicks[i].y);
        eticx = (plotPrefs.dialImageWidth/2) + (int)(DIAL_TICK_OUTER * netrainTicks[i].x);
        eticy = (plotPrefs.dialImageWidth/2) - (int)(DIAL_TICK_OUTER * netrainTicks[i].y);

        gdImageSetThickness (im, 2);
        gdImageLine (im, sticx, sticy, eticx, eticy, tickcolor);
        gdImageString (im, gdFontSmall, xpix, ypix, (uint8_t *)labels[i], tickcolor);
    }

    //  ... title
    sprintf (text, "%s", title);

    // center it
    degAngle = plotPrefs.dialDiameter - (gdFontGiant->w * strlen (text));
    degAngle /= 2;
    degAngle += gdFontGiant->w/2;
    if (degAngle < 0)
        degAngle = 0;

    gdImageString (im,
                   gdFontGiant,
                   (int)degAngle,
                   (plotPrefs.dialImageWidth/2)-52,
                   (uint8_t *)text,
                   tickcolor);

    gdImagePng (im, pngFile);
    fclose (pngFile);
    gdImageDestroy (im);

    return OK;
}

int htmlGenPngDialWindRose
(
    char                *fname,
    float               values[],
    int                 numValues,
    char                *title,
    int                 sampleWidth
)
{
    register int        i, bucket;
    register gdImagePtr im;
    char                text[64];
    register FILE       *pngFile, *inFile;
    int                 counters[WR_MAX_COUNTERS];
    int                 numCounters = 360/sampleWidth;
    int                 maxCount, numSamples;
    int                 xpix, ypix, sticx, sticy, eticx, eticy;


    // suck in the dial palette
    inFile = fopen (dialPaletteFile, "rb");
    if (inFile == NULL)
    {
        return ERROR;
    }

    im = gdImageCreateFromGd (inFile);
    fclose (inFile);
    if (im == NULL)
    {
        return ERROR;
    }

    pngFile = fopen (fname, "wb");
    if (pngFile == NULL)
    {
        gdImageDestroy (im);
        return ERROR;
    }

    // populate the counts in each direction
    memset((void *) counters, 0, sizeof counters);
    numSamples = 0;
    for (i = 0; i < numValues; i ++)
    {
        if (values[i] != ARCHIVE_VALUE_NULL)
        {
            bucket = (int) (values[i] + (sampleWidth - 1)/2)/sampleWidth;
            bucket %= numCounters;
            counters[bucket] ++;
            numSamples ++;
        }
    }

    // determine scale
    maxCount = 0;
    for (i = 0; i < numCounters; i ++)
    {
        if (counters[i] > maxCount)
            maxCount = counters[i];
    }

    // draw an arc in each direction, of length according to its count:
    for (i = 0; i < numCounters; i ++)
    {
        register int length;
        register int angle;

        if (counters[i] == 0)
            continue;

        // wedge length and direction
        length = (int) ((((float) counters[i])/maxCount)*(DIAL_POINTER_TIP*2) + 0.5);
        angle = 270 /*0deg = UP*/ + i*sampleWidth;

        // draw wedge
        gdImageFilledArc(im,
                   /* x */ plotPrefs.dialImageWidth/2,
                   /* y */ plotPrefs.dialImageWidth/2,
                   /* height */ length,
                   /* width */ length,
                   /* start angle */ (angle - sampleWidth/2) + WR_WEDGE_SPACE,
                   /* end angle */ (angle + sampleWidth/2) - WR_WEDGE_SPACE,
                   appcolor,
                   /* filled wedge */ gdPie);
        gdImageFilledArc(im,
                   /* x */ plotPrefs.dialImageWidth/2,
                   /* y */ plotPrefs.dialImageWidth/2,
                   /* height */ length,
                   /* width */ length,
                   /* start angle */ (angle - sampleWidth/2) + WR_WEDGE_SPACE,
                   /* end angle */ (angle + sampleWidth/2) - WR_WEDGE_SPACE,
                   outlinecolor,
                   /* wedge outline */ gdPie | gdEdged | gdNoFill);
    }

    // create the dial ticks and labels
    for (i = 0; i < 8; i ++)
    {
        xpix  = (plotPrefs.dialImageWidth/2) + (int)(DIAL_TICK_LABEL * windLabelLocations[i].x);
        ypix  = (plotPrefs.dialImageWidth/2) - (int)(DIAL_TICK_LABEL * windLabelLocations[i].y);
        sticx = (plotPrefs.dialImageWidth/2) + (int)(DIAL_TICK_INNER * windTicks[i].x);
        sticy = (plotPrefs.dialImageWidth/2) - (int)(DIAL_TICK_INNER * windTicks[i].y);
        eticx = (plotPrefs.dialImageWidth/2) + (int)(DIAL_TICK_OUTER * windTicks[i].x);
        eticy = (plotPrefs.dialImageWidth/2) - (int)(DIAL_TICK_OUTER * windTicks[i].y);
        gdImageSetThickness (im, 2);
        gdImageLine (im, sticx, sticy, eticx, eticy, tickcolor);
        gdImageString (im, gdFontSmall, xpix, ypix, (uint8_t *)windLabels[i], tickcolor);
    }

    // number of samples
    sprintf(text, "%d samples", numSamples);

    // center it
    i = plotPrefs.dialDiameter - (gdFontGiant->w * strlen (text));
    i /= 2;
    i += gdFontGiant->w/2;
    if (i < 0)
        i = 0;

    // draw it
    gdImageString (im, gdFontGiant, i, (plotPrefs.dialImageWidth/2)+26, (uint8_t *)text, centertext);

    // maximum count (i.e. scale)
    sprintf(text, "%d", maxCount);

    // center it
    i = plotPrefs.dialDiameter - (gdFontSmall->w * strlen (text));
    i /= 2;
    i += gdFontSmall->w/2;
    if (i < 0)
        i = 0;

    // draw it
    gdImageString (im, gdFontSmall, i, (plotPrefs.dialImageWidth/2)+40, (uint8_t *)text, centerhigh);

    // title
    sprintf (text, "%s", title);

    // center it
    i = plotPrefs.dialDiameter - (gdFontGiant->w * strlen (text));
    i /= 2;
    i += gdFontGiant->w/2;
    if (i < 0)
        i = 0;

    // draw it
    gdImageString (im, gdFontGiant, i, (plotPrefs.dialImageWidth/2)-52, (uint8_t *)text, tickcolor);

    gdImagePng (im, pngFile);
    fclose (pngFile);
    gdImageDestroy (im);

    return OK;
}

