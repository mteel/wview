#ifndef INC_wmr918protocolh
#define INC_wmr918protocolh
/*---------------------------------------------------------------------------
 
  FILENAME:
        wmr918protocol.h
 
  PURPOSE:
        Provide protocol utilities for WMR918 station communication.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        04/09/2008      M.S. Teel       0               Original
 
  NOTES:
        
 
  LICENSE:
        Copyright (c) 2008, Mark S. Teel (mark@teel.ws)
  
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
#include <errno.h>

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
#include <sensor.h>


#define WMR918_MAX_RETRIES             5
#define WMR918_BUFFER_LENGTH           16
#define WMR918_READ_TIMEOUT            500

// Define the rain rate acuumulator period (minutes):
#define WMR918_RAIN_RATE_PERIOD         5


#define WMR918GROUP_LENGTH_MAX         13

#define WMR918GROUP0	               0       /* 00 - anemometer and wind related data */
#define WMR918GROUP1	               1       /* 01 - rain guage */
#define WMR918GROUP2	               2       /* 02 - extra sensors */
#define WMR918GROUP3	               3       /* 03 - outside temp, humidity and dewpoint */
#define WMR918GROUP4	               4       /* 04 - pool */
#define WMR918GROUP5	               5       /* 05 - inside temp, humidity, dewpoint, and baro */
#define WMR918GROUP6	               6       /* 06 - inside temp, humidity, dewpoint, baro for
				                                       wmr968 and some wmr918's */
#define WMR918GROUPE                   14      /* 0e - sequence number */
#define WMR918GROUPF                   15      /* 0f - hourly status report */

// parsing helper macros:
#define LO(byte)	                   (byte & 0x0f)
#define HI(byte)	                   ((byte & 0xf0) >> 4)
#define MHI(byte)	                   ((byte & 0x0f) << 4)
#define NUM(byte)	                   (10 * HI(byte) + LO(byte))
#define BIT(byte, bit)	               ((byte & (1 << bit)) >> bit)

enum _SensorTypes
{
    WMR918_SENSOR_WIND              = 0x01,
    WMR918_SENSOR_RAIN              = 0x02,
    WMR918_SENSOR_OUT_TEMP          = 0x04,
    WMR918_SENSOR_IN_TEMP           = 0x08,
    WMR918_SENSOR_ALL               = 0x0F
};

// define the readings collector
typedef struct
{
    float           inTemp;
    float           outTemp;
    float           inHumidity;
    float           outHumidity;
    float           windDir;
    float           windSpeed;
    float           maxWindDir;
    float           maxWindSpeed;
    float           pressure;
    float           rain;
    float           rainrate;
    float           pool;
    float           extraTemp[3];
    float           extraHumidity[3];
    uint8_t         windBatteryStatus;
    uint8_t         rainBatteryStatus;
    uint8_t         outTempBatteryStatus;
    uint8_t         inTempBatteryStatus;
    uint8_t         poolTempBatteryStatus;
    uint8_t         extraBatteryStatus[3];
    uint8_t         tendency;

} WMR918_DATA;


// define the work area
typedef struct
{
    WMR918_DATA     sensorData;
    uint8_t         readData[WMR918_BUFFER_LENGTH];
    uint8_t         dataRXMask;
} WMR918_WORK;


// define WMR918-specific interface data here
typedef struct
{
    int             elevation;
    float           latitude;
    float           longitude;
    int             archiveInterval;
    WMR918_DATA     wmr918Readings;
    float           totalRain;              // to track cumulative changes
    WV_ACCUM_ID     rainRateAccumulator;    // to compute rain rate
    int             outsideChannel;
} WMR918_IF_DATA;


// call once during initialization
extern int wmr918Init (WVIEWD_WORK *work);

// do cleanup
extern void wmr918Exit (WVIEWD_WORK *work);

// read data from station:
extern void wmr918ReadData (WVIEWD_WORK *work);

// get loop packet data:
extern void wmr918GetReadings (WVIEWD_WORK *work);

#endif

