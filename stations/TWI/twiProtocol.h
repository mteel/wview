#ifndef INC_twiProtocolh
#define INC_twiProtocolh
/*---------------------------------------------------------------------------
 
  FILENAME:
        twiProtocol.h
 
  PURPOSE:
        Provide protocol utilities for TWI station communication.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        12/14/2009      M.S. Teel       0               Original
 
 
  NOTES:
 
 
  LICENSE:
        Copyright (c) 2009, Mark S. Teel (mark@teel.ws)
 
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


#define TWI_RESPONSE_TIMEOUT            1000

#define TWI_BYTE_LENGTH_MAX             128
#define TWI_DELIMITERS                  " "

#define TWI_CR                          0x0D
#define TWI_LF                          0x0A

// define the data item indexes:
typedef enum
{
    TWI_DATA_WIND_DIR           = 3,
    TWI_DATA_WIND_SPD           = 4,
    TWI_DATA_TEMP_AUX           = 5,
    TWI_DATA_TEMP_IN            = 6,
    TWI_DATA_TEMP_OUT           = 7,
    TWI_DATA_HUMIDITY           = 8,
    TWI_DATA_BAROM              = 9,
    TWI_DATA_RAIN_DAY           = 10,
    TWI_DATA_RAIN_MONTH         = 11,
    TWI_DATA_RAIN_RATE          = 12
} TWI_TYPES;


// define the readings collector
typedef struct
{
    float           inTemp;
    float           outTemp;
    float           auxTemp;
    float           humidity;
    float           windDir;
    float           windSpeed;
    float           bp;
    float           dayrain;
    float           monthrain;
    float           rainrate;

}
TWI_DATA;


// define the work area
typedef struct
{
    TWI_DATA        sensorData;

}
TWI_WORK;


// function prototypes

// call once during initialization
extern int twiProtocolInit(WVIEWD_WORK *work);

// call once during initialization after first sensor readings
extern int twiProtocolPostInit(WVIEWD_WORK *work);

// do cleanup
extern void twiProtocolExit(WVIEWD_WORK *work);

// initiate a synchronous sensor collection
extern int twiProtocolGetReadings(WVIEWD_WORK *work, TWI_DATA *store);

// send a properly formatted line to the station, optionally checking
// the response
extern int twiProtocolWriteLineToStation
(
    WVIEWD_WORK     *work,
    char            *strToSend,
    int             flushRXBuffer
);

// read a properly formatted data line from the station:
// returns OK or ERROR:
extern int twiProtocolReadLine (WVIEWD_WORK *work);


#endif
