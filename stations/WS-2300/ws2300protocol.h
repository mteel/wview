#ifndef INC_ws2300protocolh
#define INC_ws2300protocolh
/*---------------------------------------------------------------------------
 
  FILENAME:
        ws2300protocol.h
 
  PURPOSE:
        Provide protocol utilities for WS-2300 station communication.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        02/27/2008      M.S. Teel       0               Original
 
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


#define WS2300_MAX_RETRIES              10
#define WS_RESPONSE_TIMEOUT             1000L
#define WS2300_BUFFER_LENGTH            32
#define WS2300_READ_TIMEOUT             1000
#define WS2300_NUM_GOOD_REQUIRED        3


// Define the rain rate acuumulator period (minutes):
#define WS2300_RAIN_RATE_PERIOD         5


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

} WS2300_DATA;

typedef enum
{
    WS_SENSOR_IN_TEMP           = 0,
    WS_SENSOR_OUT_TEMP,
    WS_SENSOR_IN_HUMIDITY,
    WS_SENSOR_OUT_HUMIDITY,
    WS_SENSOR_WIND,
    WS_SENSOR_PRESSURE,
    WS_SENSOR_RAIN,
    WS_SENSOR_MAX
} WSSensorTypes;

typedef struct
{
    int             address;
    int             bytes;
} WS_SENSOR_INFO;


// define the work area
typedef struct
{
    WS2300_DATA     sensorData;
    uint16_t        sensorStatus;
    WSSensorTypes   currentSensor;
    uint8_t         commandData[WS2300_BUFFER_LENGTH];
    uint8_t         readData[WS2300_BUFFER_LENGTH];
    float           lastAttempt;
    int             numTries;
    int             numGood;
} WS2300_WORK;


// ... define the state machine states
typedef enum
{
    WS_STATE_STARTPROC            = 1,
    WS_STATE_RUN,
    WS_STATE_ADRS0_ACK,
    WS_STATE_ADRS1_ACK,
    WS_STATE_ADRS2_ACK,
    WS_STATE_ADRS3_ACK,
    WS_STATE_NUMBYTES_ACK,
    WS_STATE_ERROR
} WS_STATES;


// define WXT510-specific interface data here
typedef struct
{
    STATES_ID       stateMachine;
    int             elevation;
    float           latitude;
    float           longitude;
    int             archiveInterval;
    WS2300_DATA     ws2300Readings;
    float           totalRain;              // to track cumulative changes
    WV_ACCUM_ID     rainRateAccumulator;    // to compute rain rate
} WS2300_IF_DATA;


// define an extra stimulus type for our purposes
typedef enum
{
    WS_STIM_READINGS        = STIM_IO + 1
} WSStims;


// function prototypes

extern int wsStartProcState (int state, void *stimulus, void *data);
extern int wsRunState (int state, void *stimulus, void *data);
extern int wsAdrs0State (int state, void *stimulus, void *data);
extern int wsAdrs1State (int state, void *stimulus, void *data);
extern int wsAdrs2State (int state, void *stimulus, void *data);
extern int wsAdrs3State (int state, void *stimulus, void *data);
extern int wsNumBytesState (int state, void *stimulus, void *data);
extern int wsErrorState (int state, void *stimulus, void *data);


// call once during initialization
extern int ws2300Init (WVIEWD_WORK *work);

// do cleanup
extern void ws2300Exit (WVIEWD_WORK *work);

#endif

