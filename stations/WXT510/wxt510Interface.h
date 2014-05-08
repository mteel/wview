#ifndef INC_wxt510Interfaceh
#define INC_wxt510Interfaceh
/*---------------------------------------------------------------------------

  FILENAME:
        wxt510Interface.h

  PURPOSE:
        Provide the WXT-510 station interface API and utilities.

  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        01/14/2006      M.S. Teel       0               Original
        03/24/2008      W. Krenn        1               Hail

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

#include <nmea0183.h>

#define WXT_RAIN_RESET_INTERVAL         (1000 * 60 * 60 * 24)   // 1 day



// define WXT510-specific interface data here
typedef struct
{
    int             elevation;
    float           latitude;
    float           longitude;
    int             archiveInterval;
    NMEA0183_DATA   nmeaReadings;
    float           totalRain;              // to track accumulator changes
    float           totalHail;
} WXT510_IF_DATA;


#endif
