#ifndef INC_sysdefsh
#define INC_sysdefsh
/*---------------------------------------------------------------------------
 
  FILENAME:
        sysdefs.h
 
  PURPOSE:
        Define system definitions.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        08/24/03        M.S. Teel       0               Original
        11/02/2004      K. McGuire      1               ARM Mods and 
                                                        CONS_ARCHIVE_INTERVAL
        01/10/2006      M.S. Teel       2               Update for station
                                                        abstraction

  NOTES:


  LICENSE:
        Copyright (c) 2004, Mark S. Teel (mark@teel.ws)
  
        This source code is released for free distribution under the terms 
        of the GNU General Public License.
  
----------------------------------------------------------------------------*/

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "config.h"
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#include <radsysdefs.h>
#include <radmsgLog.h>



// define the GLOBAL version string here
#define globalWviewVersionStr       PACKAGE_STRING


// define some global debug macros
#define __DEBUG_BUFFERS             FALSE


// define the radlib system ID for all wview daemons
#define WVIEW_SYSTEM_ID             0x01


// Define the wvconfig semaphore index:
#define WVIEW_CONFIG_SEM_INDEX      (SEM_INDEX_USER_START + 1)

// Define the archive DB semaphore index:
#define WVIEW_ARCHIVE_SEM_INDEX     (SEM_INDEX_USER_START + 2)


// define the standard system file locations
#define WVIEW_CONFIG_DIR            WV_CONFIG_DIR
#define WVIEW_RUN_DIR               WV_RUN_DIR

// Define common string sizes:
#define WVIEW_STRING1_SIZE          128
#define WVIEW_STRING2_SIZE          256
#define WVIEW_STRING3_SIZE          512


// Database defs:
#define WVIEW_CONFIG_DATABASE       "wview-conf.sdb"

#define WVIEW_ARCHIVE_DATABASE      "wview-archive.sdb"
#define WVIEW_ARCHIVE_MARKER_FILE   "archive_marker"

#define WVIEW_HISTORY_DATABASE      "wview-history.sdb"
#define WVIEW_DAY_HISTORY_TABLE     "dayHistory"

#define WVIEW_HILOW_DATABASE        "wview-hilow.sdb"
#define WVIEW_HILOW_META_TABLE      "metainfo"
#define WVIEW_HILOW_WINDDIR_TABLE   "windDir"
#define WVIEW_HILOW_MARKER_FILE     "hilow_marker"

#define WVIEW_NOAA_DATABASE         "wview-noaa.sdb"
#define WVIEW_NOAA_TABLE            "noaaHistory"
#define WVIEW_NOAA_MARKER_FILE      "noaa_marker"


// define the process names, etc.

#define WVD_LOCK_FILE_NAME              "wviewd.pid"
#define HTML_LOCK_FILE_NAME             "htmlgend.pid"
#define HTML_INDICATE_FILE_NAME         "wview-running"
#define FTP_LOCK_FILE_NAME              "wviewftpd.pid"
#define SSH_LOCK_FILE_NAME              "wviewsshd.pid"
#define ALARMS_LOCK_FILE_NAME           "wvalarmd.pid"
#define CWOP_LOCK_FILE_NAME             "wvcwopd.pid"
#define HTTP_LOCK_FILE_NAME             "wvhttpd.pid"
#define PMON_LOCK_FILE_NAME             "wvpmond.pid"

#define PROC_NAME_DAEMON                "wviewd"
#define PROC_NUM_TIMERS_DAEMON          5

#define PROC_NAME_HTML                  "htmlgend"
#define PROC_NUM_TIMERS_HTML            3
#define HTML_GRAPHICS_CONFIG_FILENAME   "graphics.conf"
#define HTML_PRE_GEN_SCRIPT             "pre-generate.sh"
#define HTML_POST_GEN_SCRIPT            "post-generate.sh"

#define PROC_NAME_FTP                   "wviewftpd"
#define PROC_NUM_TIMERS_FTP             1
#define FTP_MARKER_FILE                 "ftp_marker"

#define PROC_NAME_SSH                   "wviewsshd"
#define PROC_NUM_TIMERS_SSH             1

#define PROC_NAME_ALARMS                "wvalarmd"
#define PROC_NUM_TIMERS_ALARMS          0

#define PROC_NAME_CWOP                  "wvcwopd"
#define PROC_NUM_TIMERS_CWOP            1

#define PROC_NAME_HTTP                  "wvhttpd"
#define PROC_NUM_TIMERS_HTTP            0

#define PROC_NAME_PMON                  "wvpmond"
#define PROC_NUM_TIMERS_PMON            2

#define WVIEW_STATUS_DIRECTORY          "/var/run/wview"
#define WVIEW_STATUS_FILE_NAME          "wview.sts"
#define HTML_STATUS_FILE_NAME           "html.sts"
#define FTP_STATUS_FILE_NAME            "ftp.sts"
#define SSH_STATUS_FILE_NAME            "ssh.sts"
#define ALARMS_STATUS_FILE_NAME         "alarms.sts"
#define CWOP_STATUS_FILE_NAME           "cwop.sts"
#define HTTP_STATUS_FILE_NAME           "http.sts"
#define PMON_STATUS_FILE_NAME           "pmon.sts"


// define all wview events here for consistency
#define STATION_INIT_COMPLETE_EVENT     0x00010000
#define STATION_LOOP_COMPLETE_EVENT     0x00020000


// some general time definitions
#define WV_SECONDS_IN_HOUR              3600
#define WV_SECONDS_IN_DAY               86400
#define WV_SECONDS_IN_WEEK              (7*WV_SECONDS_IN_DAY)
#define WV_SECONDS_IN_MONTH             (28*WV_SECONDS_IN_DAY)
#define WV_SECONDS_IN_YEAR              (365*WV_SECONDS_IN_DAY)
#define WV_MINUTES_IN_DAY               1440
#define WV_MINUTES_IN_YEAR              (365*WV_MINUTES_IN_DAY)
#define SECONDS_IN_INTERVAL(x)          (x * 60)


// packed time utilities
#define EXTRACT_PACKED_YEAR(x)          ((((x) >> 9) & 0x3F) + 2000)
#define EXTRACT_PACKED_MONTH(x)         (((x) >> 5) & 0xF)
#define EXTRACT_PACKED_DAY(x)           ((x) & 0x1F)
#define EXTRACT_PACKED_HOUR(y)          ((y) / 100)
#define EXTRACT_PACKED_MINUTE(y)        ((y) % 100)

#define INSERT_PACKED_DATE(y,m,d)       (((((y)-2000) & 0x3F) << 9) | (((m) & 0xF) << 5) | ((d) & 0x1F))


#ifndef _MAX_PATH
#define _MAX_PATH                       512
#endif


// Define the maximum radMsgLog length:
#define RADMSGLOG_MAX_LENGTH            448


// Define a max for date strings:
#define WV_MAX_DATE_LENGTH              128


// VP Icon bit definitions
#define VP_FCAST_ICON_MAX               0x1F
#define VP_FCAST_ICON_RAIN              0x01
#define VP_FCAST_ICON_CLOUD             0x02
#define VP_FCAST_ICON_PART_SUN          0x04
#define VP_FCAST_ICON_SUNNY             0x08
#define VP_FCAST_ICON_SNOW              0x10


// Process Monitor process types
typedef enum
{
    PMON_PROCESS_WVIEWD                 = 0,
    PMON_PROCESS_HTMLGEND,
    PMON_PROCESS_WVALARMD,
    PMON_PROCESS_WVCWOPD,
    PMON_PROCESS_WVHTTPD,
    PMON_PROCESS_MAX
} PMON_PROCESS_TYPES;

#define PMON_PROCESS_SET(mask,type)     (mask | (0x01 << type))
#define PMON_PROCESS_ISSET(mask,type)   ((mask >> type) & 0x01)


// utility prototypes

// define the verbosity bit definitions
enum _verbosity_bits
{
    WV_VERBOSE_WVIEWD       = 0x01,
    WV_VERBOSE_HTMLGEND     = 0x02,
    WV_VERBOSE_WVALARMD     = 0x04,
    WV_VERBOSE_WVIEWFTPD    = 0x08,
    WV_VERBOSE_WVIEWSSHD    = 0x10,
    WV_VERBOSE_WVCWOPD      = 0x20,
    WV_VERBOSE_WVWUNDERD    = 0x40,
    WV_VERBOSE_ALL          = 0x7F
};

// Define wind units enumerations:
typedef enum
{
    HTML_WINDUNITS_MPH              = 0,
    HTML_WINDUNITS_MS,
    HTML_WINDUNITS_KNOTS,
    HTML_WINDUNITS_KMH
} HTML_WUNITS;

extern void wvutilsSetWindUnits(HTML_WUNITS units);
extern char* wvutilsGetWindUnitLabel(void);
extern float wvutilsGetWindSpeed(float mph);
extern float wvutilsGetWindSpeedMetric(float kph);

// wvconfigInit() must have been called before calling this:
extern int  wvutilsSetVerbosity (uint16_t daemonBitMask);

extern int wvutilsToggleVerbosity (void);
extern void wvutilsLogEvent (int priority, char *format, ...);

extern float wvutilsCalculateHeatIndex (float temp, float humidity);
extern float wvutilsCalculateWindChill (float temp, float windspeed);
extern float wvutilsCalculateDewpoint (float temp, float humidity);

/****************************************************************************
 * Converters
 ***************************************************************************/
extern void wvutilsSetRainIsMM(int setValue);
extern int wvutilsGetRainIsMM(void);
extern float wvutilsConvertFToC (float fahrenValue);
extern float wvutilsConvertCToF (float celsiusValue);
extern float wvutilsConvertDeltaFToC (float fahrenValue);
extern float wvutilsConvertMPHToKPH (float mph);
extern float wvutilsConvertMPHToMPS (float mph);
extern float wvutilsConvertMPHToKnots (float mph);
extern float wvutilsConvertKPHToMPH (float kph);
extern float wvutilsConvertKPHToMPS (float kph);
extern float wvutilsConvertKPHToKnots (float kph);
extern float wvutilsConvertMPSToKPH (float mps);
extern float wvutilsConvertMPSToMPH (float mps);
extern float wvutilsConvertMPSToKnots (float mps);
extern float wvutilsConvertKnotsToKPH (float mps);
extern float wvutilsConvertKnotsToMPH (float mps);
extern float wvutilsConvertKnotsToMPS (float mps);
extern float wvutilsConvertINHGToHPA (float inches);
extern float wvutilsConvertHPAToINHG (float mb);
extern float wvutilsConvertRainINToMetric (float inches);
extern float wvutilsConvertRainMetricToIN (float inches);
extern float wvutilsConvertINToCM (float inches);
extern float wvutilsConvertCMToIN (float cm);
extern float wvutilsConvertINToMM (float inches);
extern float wvutilsConvertMMToIN (float mm);

extern float wvutilsConvertFeetToMeters (float feet);
extern float wvutilsConvertKilometersToMiles (float km);
extern float wvutilsConvertMilesToKilometers (float miles);

extern char *wvutilsConvertToBeaufortScale (int windSpeed);


//  calculate the air density in kg/m^3
extern float wvutilsCalculateAirDensity (float tempF, float bp, float dp);

//  calculate sea level pressure from station pressure
extern float wvutilsConvertSPToSLP (float SP, float tempF, float elevationFT);

//  calculate station pressure from sea level pressure
extern float wvutilsConvertSLPToSP (float SLP, float tempF, float elevationFT);

//  calculate altimeter pressure from station pressure
extern float wvutilsConvertSPToAltimeter (float SPInches, float elevationFT);

// Calculate the apparent temperature:
extern float wvutilsCalculateApparentTemp(float temp, float windspeed, float humidity);

// Calculate wet bulb temperature
extern float wvutilsCalculateWetBulbTemp( float temp, float humidity, float pressure);

// convert packed date/time to time_t
extern time_t wvutilsPackedTimeToTimeT (uint16_t packedDate, uint16_t packedTime);

// calculate the time difference in packed date/time format
// does not consider leap years
// returns the delta in minutes
extern int wvutilsCalculatePackedTimeDelta
(
    uint16_t      newDate,
    uint16_t      newTime,
    uint16_t      oldDate,
    uint16_t      oldTime
);

// increment a packed time value by the given minutes, rolls over at 24:00
extern uint16_t wvutilsIncrementPackedTime (uint16_t pTime, int minutes);


// define sun rise and set types
enum _rise_set_types
{
    RS_TYPE_SUN         = 1,
    RS_TYPE_CIVIL,
    RS_TYPE_ASTRO,
    RS_TYPE_MIDDAY
};

// all times returned as packed time (hour*100 + minutes)
// lat and long are floats representing degrees:
//   N  => +
//   S  => -
//   E  => +
//   W  => -

// Return Values:
//   packedRise = packedSet = -1     =>  the sun never sets
//   packedRise = packedSet = -2     =>  the sun never rises
//   otherwise the valid times in packed format
extern void sunTimesGetSunRiseSet
(
    int         year,
    int         month,
    int         day,
    float       latitude,
    float       longitude,
    int         type,       // RS_TYPE_SUN, RS_TYPE_CIVIL, RS_TYPE_ASTRO, RS_TYPE_MIDDAY
    int16_t     *packedRise,
    int16_t     *packedSet
);

// Get day length in packed format
extern int16_t sunTimesGetDayLength
(
    int         year,
    int         month,
    int         day,
    float       latitude,
    float       longitude
);

// produce a float string fixing the truncation annoyance
extern char *wvutilsPrintFloat (float value, int decPlaces);

// Define a NULL-terminating wvstrncpy:
extern int wvstrncpy(char *d, const char *s, size_t bufsize);

// Right-trim a string:
extern void wvstrtrim(char *d);

// Respond to PMON poll messages
extern void wvutilsSendPMONPollResponse (int mask, PMON_PROCESS_TYPES process);

// History start time utilities:
extern int    wvutilsGetDayStartTime   (int archiveInterval);
extern time_t wvutilsGetWeekStartTime  (int archiveInterval);
extern time_t wvutilsGetMonthStartTime (int archiveInterval);
extern time_t wvutilsGetYearStartTime  (int archiveInterval);

// Time utilities:
extern int wvutilsGetYear  (time_t ntime);
extern int wvutilsGetMonth (time_t ntime);
extern int wvutilsGetDay   (time_t ntime);
extern int wvutilsGetHour  (time_t ntime);
extern int wvutilsGetMin   (time_t ntime);
extern int wvutilsGetSec   (time_t ntime);

extern int wvutilsTimeIsToday(time_t checkTime);

//  Determine if it is day or night
//  Returns TRUE or FALSE
extern int wvutilsIsDayTime (int16_t timeSunrise, int16_t timeSunset);


//  return a string indicating the moon phase:
extern char *lunarPhaseGet (char *increase, char *decrease, char *full);


// Path utilities:
extern char* wvutilsGetArchivePath(void);
extern char* wvutilsGetConfigPath(void);

// Marker file utilities:
extern int    wvutilsWriteMarkerFile(const char* filePath, time_t marker);
extern time_t wvutilsReadMarkerFile(const char* filePath);


// Define a SIGCHLD handler to wait for child processes to exit:
// Should only be called from process signal handler.
extern void wvutilsWaitForChildren(void);


//  Initialize and detect a DST state change:
//  Uses the system clock and the tm struct value "tm_isdst" returned by the 
//  system call "localtime" to make the determination
extern int wvutilsDetectDSTInit(void);

typedef enum
{
    WVUTILS_DST_NO_CHANGE       = 0,            // no change
    WVUTILS_DST_FALL_BACK       = -1,           // End of DST
    WVUTILS_DST_SPRING_FORWARD  = 1             // Start od DST
} WVUTILS_DST_TYPE;

// Returns: WVUTILS_DST_TYPE
extern int wvutilsDetectDSTChange(void);


// Assign a degree value to received string representation for wind direction:
// Returns degree equivalent or -1 if ERROR:
extern int wvutilsConvertWindStrToDegrees(const char* windStr);

// Create the CWOP version string:
extern char* wvutilsCreateCWOPVersion(char* wviewStr);

#endif
