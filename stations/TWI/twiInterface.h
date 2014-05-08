#ifndef INC_twiInterfaceh
#define INC_twiInterfaceh
/*---------------------------------------------------------------------------

  FILENAME:
        twiInterface.h

  PURPOSE:
        Provide the TWI station interface API and utilities.

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
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <termios.h>
#include <time.h>
#include <errno.h>
#include <math.h>

/*  ... Library include files
*/
#include <sysdefs.h>
#include <radtimeUtils.h>
#include <radmsgLog.h>
#include <radsysutils.h>

/*  ... Local include files
*/
#include <datadefs.h>
#include <dbsqlite.h>
#include <config.h>
#include <computedData.h>
#include <daemon.h>
#include <station.h>
#include <serial.h>
#include <ethernet.h>

#include <twiProtocol.h>


// Define the rain rate acuumulator period (minutes):
#define TWI_RAIN_RATE_PERIOD            5



// define WXT510-specific interface data here
typedef struct
{
    int             elevation;
    float           latitude;
    float           longitude;
    int             archiveInterval;
    TWI_DATA        twiReadings;
    float           totalRain;              // to track accumulator changes
    WV_ACCUM_ID     rainRateAccumulator;    // to compute rain rate
    int             baudrate;
} TWI_IF_DATA;


// Prototypes:

// Autobaud the TWI station:
// Returns the baudrate or ERROR:
extern int twiConfig(WVIEWD_WORK *work);


#endif

