#ifndef INC_dbfilesh
#define INC_dbfilesh
/*---------------------------------------------------------------------------
 
  FILENAME:
        dbfiles.h
 
  PURPOSE:
        Provide the wview daemon database files definitions.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        08/16/03        M.S. Teel       0               Original
        04/12/2008      W. Krenn        1               RainCollectorType
 
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
#include <math.h>
#include <sys/stat.h>

/*  ... Library include files
*/
#include <sysdefs.h>
#include <datadefs.h>
#include <windAverage.h>

/*  ... Local include files
*/


/*  ... API definitions
*/

/*  !!!!!!!!!!!!!!!!!!  HIDDEN, NOT FOR API USE  !!!!!!!!!!!!!!!!!!
*/
// define 'B' type records first - this is what we use internally
// 'A' < Firmware Version 04/24/2002 <= 'B'
typedef struct
{
    uint16_t              date;
    uint16_t              time;
    int16_t               outTemp;
    int16_t               highOutTemp;
    int16_t               lowOutTemp;
    uint16_t              rain;
    uint16_t              highRainRate;
    uint16_t              barometer;
    uint16_t              radiation;
    uint16_t              windSamples;
    uint16_t              inTemp;
    uint8_t               inHumidity;
    uint8_t               outHumidity;
    uint8_t               avgWindSpeed;
    uint8_t               highWindSpeed;
    uint8_t               highWindDir;
    uint8_t               prevWindDir;
    uint8_t               UV;                 // extra data begins here
    uint8_t               ET;
    uint16_t              highRadiation;
    uint8_t               highUV;
    uint8_t               fcstRule;
    uint8_t               leafTemp1;
    uint8_t               leafTemp2;
    uint8_t               leafWet1;
    uint8_t               leafWet2;
    uint8_t               soilTemp1;
    uint8_t               soilTemp2;
    uint8_t               soilTemp3;
    uint8_t               soilTemp4;
    uint8_t               recordType;
    uint8_t               extraHumid1;
    uint8_t               extraHumid2;
    uint8_t               extraTemp1;
    uint8_t               extraTemp2;
    uint8_t               extraTemp3;
    uint8_t               soilMoist1;
    uint8_t               soilMoist2;
    uint8_t               soilMoist3;
    uint8_t               soilMoist4;
}
__attribute__ ((packed)) ARCHIVE_RECORD;


// define the old 'A' format here
typedef struct
{
    uint16_t              date;
    uint16_t              time;
    int16_t               outTemp;
    int16_t               highOutTemp;
    int16_t               lowOutTemp;
    uint16_t              rain;
    uint16_t              highRainRate;
    uint16_t              barometer;
    uint16_t              radiation;
    uint16_t              windSamples;
    uint16_t              inTemp;
    uint8_t               inHumidity;
    uint8_t               outHumidity;
    uint8_t               avgWindSpeed;
    uint8_t               highWindSpeed;
    uint8_t               highWindDir;
    uint8_t               prevWindDir;
    uint8_t               UV;                 // extra data begins here
    uint8_t               ET;
    uint8_t               invalid1;
    uint8_t               soilMoist1;
    uint8_t               soilMoist2;
    uint8_t               soilMoist3;
    uint8_t               soilMoist4;
    uint8_t               soilTemp1;
    uint8_t               soilTemp2;
    uint8_t               soilTemp3;
    uint8_t               soilTemp4;
    uint8_t               leafWet1;
    uint8_t               leafWet2;
    uint8_t               leafWet3;
    uint8_t               leafWet4;
    uint8_t               extraTemp1;
    uint8_t               extraTemp2;
    uint8_t               extraHumid1;
    uint8_t               extraHumid2;
    uint16_t              reedClosed;
    uint16_t              reedOpen;
    uint8_t               unused1;
}
__attribute__ ((packed)) ARCHIVE_RECORD_A;


#define DBFILES_RECORD_SIZE         88

enum DailySummaryTimeIndexes
{
    High_Outside_Temperature        = 0,
    Low_Outside_Temperature         = 1,
    High_Inside_Temperature         = 2,
    Low_Inside_Temperature          = 3,
    High_Wind_Chill                 = 4,
    Low_Wind_Chill                  = 5,
    High_Dew_Point                  = 6,
    Low_Dew_Point                   = 7,
    High_Outside_Humidity           = 8,
    Low_Outside_Humidity            = 9,
    High_Inside_Humidity            = 10,
    Low_Inside_Humidity             = 11,
    High_Barometer                  = 12,
    Low_Barometer                   = 13,
    High_Wind_Speed                 = 14,
    High_Average_Wind_Speed         = 15,
    High_Rain_Rate                  = 16,
    High_UV                         = 17,

    High_Solar_Rad                  = 0,
    High_Outside_Heat_Index         = 1,
    Low_Outside_Heat_Index          = 2,
    High_Outside_THSW_Index         = 3,
    Low_Outside_THSW_Index          = 4,
    High_Outside_THW_Index          = 5,
    Low_Outside_THW_Index           = 6,
    High_Outside_Wet_Bulb_Temp      = 7,
    Low_Outside_Wet_Bulb_Temp       = 8
};


//  ... this describes each day in an archive file
typedef struct
{
    int16_t     recordsInDay;       // includes the daily summary record (counts as 2)
    int         startPos;           // The index (starting at 0) of the daily summary record
}
__attribute__ ((packed)) DayIndex;

//  ... Header for each archive file
typedef struct
{
    char        idCode [16];        // = {'W','D','A','T','5','.','0',0,0,0,0,0,0,0,5,0}
    int         totalRecords;
    DayIndex    dayIndex [32];      // index records for each day. Index 0 is not used
    // (i.e. the 1'st is at index 1, not index 0)
}
__attribute__ ((packed)) HeaderBlock;


//  ... After the Header are a series of 88-byte records with one of the following
//  ... formats.  Note that each day will begin with a daily summary record.

//  ... Daily Summary Record (176 bytes)
typedef struct
{
    uint8_t   dataType1;
    uint8_t   reserved1;              // this will cause rest of fields to start on an even address

    int16_t   dataSpan;               // total # minutes accounted for by physical recs for this day
    int16_t   hiOutTemp, lowOutTemp;  // tenths of a degree F
    int16_t   hiInTemp, lowInTemp;    // tenths of a degree F
    int16_t   avgOutTemp, avgInTemp;  // tenths of a degree F (integrated over the day)
    int16_t   hiChill, lowChill;      // tenths of a degree F
    int16_t   hiDew, lowDew;          // tenths of a degree F
    int16_t   avgChill, avgDew;       // tenths of a degree F
    int16_t   hiOutHum, lowOutHum;    // tenths of a percent
    int16_t   hiInHum, lowInHum;      // tenths of a percent
    int16_t   avgOutHum;              // tenths of a percent
    int16_t   hiBar, lowBar;          // thousanths of an inch Hg
    int16_t   avgBar;                 // thousanths of an inch Hg
    int16_t   hiSpeed, avgSpeed;      // tenths of an MPH
    int16_t   dailyWindRunTotal;      // 1/10'th of an mile
    int16_t   hi10MinSpeed;           // the highest average wind speed record
    uint8_t   dirHiSpeed, hi10MinDir; // direction code (0-15, 255)
    int16_t   dailyRainTotal;         // 1/1000'th of an inch
    int16_t   hiRainRate;             // 1/100'th inch/hr ???
    int16_t   dailyUVDose;            // 1/10'th of a standard MED
    uint8_t   hiUV;                   // tenth of a UV Index
    uint8_t   timeValues1[27];        // space for 18 time values (see below)

    uint8_t   dataType2;              // start of 2nd summary record
    uint8_t   reserved2;              // this will cause rest of fields to start on an even address

    // this field is not used now.
    uint16_t  todaysWeather;          // bitmapped weather conditions (Fog, T-Storm, hurricane, etc)

    int16_t   numWindPackets;         // # of valid packets containing wind data,
    // this is used to indicate reception quality
    int16_t   hiSolar;                // Watts per meter squared
    int16_t   dailySolarEnergy;       // 1/10'th Ly
    int16_t   minSunlight;            // number of accumulated minutes where the avg solar rad > 150
    int16_t   dailyETTotal;           // 1/1000'th of an inch
    int16_t   hiHeat, lowHeat;        // tenths of a degree F
    int16_t   avgHeat;                // tenths of a degree F
    int16_t   hiTHSW, lowTHSW;        // tenths of a degree F
    int16_t   hiTHW, lowTHW;          // tenths of a degree F

    int16_t   integratedHeatDD65;     // integrated Heating Degree Days (65F threshold)
    // tenths of a degree F - Day

    // Wet bulb values are not calculated
    int16_t   hiWetBulb, lowWetBulb;  // tenths of a degree F
    int16_t   avgWetBulb;             // tenths of a degree F

    uint8_t   dirBins[24];            // space for 16 direction bins
    // (Used to calculate monthly dominent Dir)

    uint8_t   timeValues2[15];        // space for 10 time values (see below)

    int16_t   integratedCoolDD65;     // integrated Cooling Degree Days (65F threshold)
    // tenths of a degree F - Day
    uint8_t   reserved3[11];
}
__attribute__ ((packed)) DailySummaryRecord;

//  ... standard archive record (88 bytes)
typedef struct
{
    uint8_t   dataType;
    uint8_t   archiveInterval;        // number of minutes in the archive
    uint8_t   iconFlags;              // Icon associated with this record, plus Edit flags
    uint8_t   moreFlags;              // Tx Id, etc.

    int16_t   packedTime;             // minutes past midnight of the end of the archive period
    int16_t   outsideTemp;            // tenths of a degree F
    int16_t   hiOutsideTemp;          // tenths of a degree F
    int16_t   lowOutsideTemp;         // tenths of a degree F
    int16_t   insideTemp;             // tenths of a degree F
    int16_t   barometer;              // thousanths of an inch Hg
    int16_t   outsideHum;             // tenths of a percent
    int16_t   insideHum;              // tenths of a percent
    uint16_t  rain;                   // number of clicks + rain collector type code
    int16_t   hiRainRate;             // clicks per hour
    int16_t   windSpeed;              // tenths of an MPH
    int16_t   hiWindSpeed;            // tenths of an MPH
    uint8_t   windDirection;          // direction code (0-15, 255)
    uint8_t   hiWindDirection;        // direction code (0-15, 255)
    int16_t   numWindSamples;         // number of valid ISS packets containing wind data
    // this is a good indication of reception
    int16_t   solarRad, hisolarRad;   // Watts per meter squared
    uint8_t   UV, hiUV;               // tenth of a UV Index

    uint8_t   leafTemp[4];            // (whole degrees F) + 90

    int16_t   newSensors[7];          // reserved for future use
    uint8_t   forecast;               // forecast code during the archive interval

    uint8_t   ET;                     // thousandths of an inch

    uint8_t   soilTemp[6];            // (whole degrees F) + 90
    uint8_t   soilMoisture[6];        // centibars of dryness
    uint8_t   leafWetness[4];         // Leaf Wetness code (0-15, 255)
    uint8_t   extraTemp[7];           // (whole degrees F) + 90
    uint8_t   extraHum[7];            // whole percent
}
__attribute__ ((packed)) ArchiveRecord;

/*  !!!!!!!!!!!!!!!!!!!!  END HIDDEN SECTION  !!!!!!!!!!!!!!!!!!!!!
*/


/*  ... API methods
*/

/*  ... 0 <= dir <= 15
*/
extern char *dbfBuildWindDirString (int dir);


/*  ... provide a routine to generate an ASCII browser view file
    ... 'outputFilename' containing archive records;
    ... 'start' and 'stop' are of the form: (("year" - 2000) * 100) + "month"
    ... and are inclusive;
    ... returns OK or ERROR
*/
extern int dbfExportArchiveFile
(
    char            *archivePath,
    uint16_t        start,
    uint16_t        stop,
    char            *outputFilename
);

/*  ... provide a routine to generate an ASCII browser view file
    ... 'outputFilename' containing daily summary records;
    ... 'start' and 'stop' are of the form: (("year" - 2000) * 100) + "month"
    ... and are inclusive;
    ... returns OK or ERROR
*/
extern int dbfExportDailySummaryFile
(
    char            *archivePath,
    uint16_t        start,
    uint16_t        stop,
    char            *outputFilename
);

/*  ... calculate averages over a given period of time
    ... (given in arcInterval samples);
    ... this will zero out the HISTORY_DATA store before beginning
    ... the processing;
    ... returns OK or ERROR
*/
extern int dbfGetAverages
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
);


/*  ... dbfGetNoaaDay: summarize day records for NOAA reports;
    ... this will zero out the NOAA_DAY_REC store before beginning;
    ... returns OK or ERROR if day not found in archives
*/
extern int dbfGetNoaaDay
(
    char            *archivePath,
    NOAA_DAY_REC    *store,
    int             year,
    int             month,
    int             day
);

extern int dbfWriteDailyArchiveReport
(
    char            *archivePath,
    char            *filename,
    time_t          timeval,
    int             isMetric,
    int             isMetricMM,
    int             arcInterval,
    void            (*writeHeader) (FILE *file)
);

extern int dbfUpdateDailyArchiveReport
(
    char            *file,
    HISTORY_DATA    *data,
    void            (*writeHeader) (FILE *file),
    int             isMetric
);


/*  ... ######!!! The following utilities are for the daemon only !!!######
*/
/*  ... append an archive record to the appropriate dbfile;
    ... if the date extends into a new month, this will create the new dbfile
    ... and insert this archive record;
    ... returns OK or ERROR
*/
extern int dbfStoreArchiveRecord
(
    char            *archivePath,
    ARCHIVE_RECORD  *record,
    int             archiveInterval,
    uint16_t        RainCollectorType
);

/*  ... delete all day records starting on a certain day for that month;
    ... returns OK or ERROR
*/
int dbfDeleteArchiveRecords
(
    char            *archivePath,
    int             year,
    int             month,
    int             day
);

/*  ... search the archive path for the most recent archive record date;
    ... places the most recent date and time in 'date' and 'time' or all
    ... zero's if no archive record found;
    ... returns OK or ERROR if no archives found
*/
extern int dbfGetNewestArchiveTime
(
    char            *archivePath,
    uint16_t        curMonth,
    uint16_t        curYear,
    uint16_t        *date,
    uint16_t        *time,
    ArchiveRecord   *newestRecord
);

/*  ... search the archive path for the next archive record after 'date' and
    ... 'time' (in packed format);
    ... places the most recent date and time in 'date' and 'time' if archive
    ... record is found;
    ... returns OK or ERROR if no archive found
*/
extern int dbfGetNextArchiveRecord
(
    char            *archivePath,
    uint16_t        *date,
    uint16_t        *time,
    ArchiveRecord   *recordStore
);

/*  ... dbfCopyArchiveRecordToConsoleRecord - copy an archive file record to
    ... the station message record format
*/
extern int dbfCopyArchiveRecordToConsoleRecord
(
    ArchiveRecord       *src,
    ARCHIVE_RECORD      *dest,
    uint16_t            date,
    uint16_t            time
);

/*  ... dbfIncrementConsoleTime - increment the station time representation by
    ... 'numMinutes' minutes
*/
extern int dbfIncrementConsoleTime
(
    ARCHIVE_RECORD      *record,
    int                 numMinutes
);

// dbfComputePackedDelta - compute the delta in minutes between 2 packed times
extern int dbfComputePackedDelta
(
    uint16_t          oldDate,
    uint16_t          oldTime,
    uint16_t          newDate,
    uint16_t          newTime
);

#endif

