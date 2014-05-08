#ifndef INC_nmea0183h
#define INC_nmea0183h
/*---------------------------------------------------------------------------
 
  FILENAME:
        nmea0183.h
 
  PURPOSE:
        Provide protocol utilities for NMEA 0183 station communication.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        01/14/2006      M.S. Teel       0               Original
        03/23/2008 W. Krenn     1               add hail/rain duration
 
 
  NOTES:
 
 
  LICENSE:
        Copyright (c) 2006, Mark S. Teel (mark@teel.ws)
 
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
#include <sys/socket.h>
#include <string.h>

/*  ... Library include files
*/
#include <sysdefs.h>
#include <radtimeUtils.h>
#include <radsocket.h>

/*  ... Local include files
*/
#include <datadefs.h>
#include <dbsqlite.h>
#include <daemon.h>
#include <parser.h>


#ifndef _WXT510_CONFIG_ONLY
#define NMEA_RESPONSE_TIMEOUT           2000
#else
#define NMEA_RESPONSE_TIMEOUT           500
#endif

#define NMEA_BYTE_LENGTH_MAX            256
#define NMEA_CR                         0x0D
#define NMEA_LF                         0x0A
#define NMEA_DELIMITERS                 ","
#define NMEA_WIXDR_ID                   "$WIXDR"


// define the "done" bit assignments
enum
{
    NMEA_DONE_TEMP          = 0x0001,
    NMEA_DONE_HUMIDITY      = 0x0002,
    NMEA_DONE_WINDDIR       = 0x0004,
    NMEA_DONE_WINDSPD       = 0x0008,
    NMEA_DONE_MAXWINDDIR    = 0x0010,
    NMEA_DONE_MAXWINDSPD    = 0x0020,
    NMEA_DONE_PRESSURE      = 0x0040,
    NMEA_DONE_RAIN          = 0x0080,
    NMEA_DONE_RAINRATE      = 0x0100,
    NMEA_DONE_HAIL          = 0x0200,
    NMEA_DONE_HAILRATE      = 0x0400,
    NMEA_DONE_HEAT_TEMP     = 0x0800,
    NMEA_DONE_HEAT_VOLT     = 0x1000,
    NMEA_DONE_SUP_VOLT      = 0x2000,
    NMEA_DONE_REF_VOLT      = 0x4000,
    NMEA_DONE_ALL           = 0x7FFF
};


// define the readings collector
typedef struct
{
    float           temperature;
    float           humidity;
    float           windDir;
    float           windSpeed;
    float           maxWindDir;
    float           maxWindSpeed;
    float           pressure;
    float           rain;
    float           rainrate;
    float           hail;
    float           hailrate;
    float           heatingTemp;
    float           heatingVoltage;
    float           supplyVoltage;
    float           referenceVoltage;

    float           rainduration;
    float           rainpeakrate;
    float           hailduration;
    float           hailpeakrate;

}
NMEA0183_DATA;


// define the work area
typedef struct
{
    NMEA0183_DATA   sensorData;
    uint16_t        sensorStatus;

}
NMEA0183_WORK;


// function prototypes

// call once during initialization
extern int nmea0183Init (WVIEWD_WORK *work);

// call once during initialization after first sensor readings
extern int nmea0183PostInit (WVIEWD_WORK *work);

// do cleanup
extern void nmea0183Exit (WVIEWD_WORK *work);

// initiate a synchronous sensor collection
extern int nmea0183GetReadings (WVIEWD_WORK *work, NMEA0183_DATA *store);

// send a properly formatted line to the station, optionally checking
// the response
extern int nmea0183WriteLineToStation
    (
        WVIEWD_WORK     *work,
        char            *strToSend,
        char            *expectedResp,
        int             generateCS,
        int             flushRXBuffer
    );

// reset the rain/hail accumulators to zero
extern int nmea0183ResetAccumulators (WVIEWD_WORK *work);

#endif
