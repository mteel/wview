#ifndef INC_te923protocolh
#define INC_te923protocolh
/*---------------------------------------------------------------------------
 
  FILENAME:
        te923Protocol.h
 
  PURPOSE:
        Provide protocol utilities for TE923 station communication.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        02/17/2011      M.S. Teel       0               Original
 
  NOTES:
        Parts of this implementation were inspired by the te923con project
        (C) Sebastian John (http://te923.fukz.org).
 
  LICENSE:
  
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

/* TE923  <vendorid, productid> */
#define TE923_VENDOR_ID                 0x1130
#define TE923_PRODUCT_ID                0x6801

#define TE923_BUFLEN                    35

#define TE923_MAX_CHANNELS              5
#define TE923_MAX_RETRIES               5


// Define the rain rate acuumulator period (minutes):
#define TE923_RAIN_RATE_PERIOD          5


// define the readings collector
typedef struct
{
    float   inhumidity;
    float   intemp;
    float   outhumidity[TE923_MAX_CHANNELS];
    float   outtemp[TE923_MAX_CHANNELS];
    float   pressure;
    float   windAvgSpeed;
    float   windGustSpeed;
    float   windDir;
    float   rain;
    float   UV;
    int     statusSensor[TE923_MAX_CHANNELS];
    int     statusUV;
    int     statusWind;
    int     statusRain;
} TE923_DATA;


// define the work area
typedef struct
{
    TE923_DATA      sensorData;
} TE923_WORK;


// define TE923-specific interface data here
typedef struct
{
    int             elevation;
    float           latitude;
    float           longitude;
    int             archiveInterval;
    TE923_DATA      te923Readings;
    float           totalRain;              // to track cumulative changes
    WV_ACCUM_ID     rainRateAccumulator;    // to compute rain rate
} TE923_IF_DATA;


// call once during initialization
extern int te923Init (WVIEWD_WORK *work);

// do cleanup
extern void te923Exit (WVIEWD_WORK *work);

// get loop packet data:
extern void te923GetReadings (WVIEWD_WORK *work);

#endif

