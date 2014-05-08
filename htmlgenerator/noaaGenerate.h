#ifndef INC_noaagenerateh
#define INC_noaagenerateh
/*---------------------------------------------------------------------------
 
  FILENAME:
        noaaGenerate.h
 
  PURPOSE:
        Provide the wview NOAA generator definitions.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        01/25/04        M.S. Teel       0               Original
 
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
#include <time.h>
#include <math.h>

/*  ... Library include files
*/
#include <sysdefs.h>
#include <radsemaphores.h>
#include <radbuffers.h>
#include <radlist.h>

/*  ... Local include files
*/
#include <datadefs.h>
#include <services.h>
#include <dbsqlite.h>
#include <windAverage.h>



/*  ... API definitions
*/

/*  !!!!!!!!!!!!!!!!!!  HIDDEN, NOT FOR API USE  !!!!!!!!!!!!!!!!!!
*/

typedef struct
{
    char            htmlPath    [_MAX_PATH];
    int             isMetric;
    int16_t         latitude;
    int16_t         longitude;
    int16_t         elevation;
    char            stationName[32];
    char            stationCity[32];
    char            stationState[32];
    time_t          lastDay;
}__attribute__ ((packed)) NOAA_WORK, *NOAA_ID;



/*  !!!!!!!!!!!!!!!!!!!!  END HIDDEN SECTION  !!!!!!!!!!!!!!!!!!!!!
*/


/* ... API function prototypes
*/

extern NOAA_ID noaaGenerateInit
(
    char    *htmlPath,
    int     isMetricUnits,
    int16_t latitude,
    int16_t longitude,
    int16_t elevation,
    char    *name,
    char    *city,
    char    *state,
    time_t  arcTime
);

extern int noaaGenerate (NOAA_ID id, time_t day);


#endif

