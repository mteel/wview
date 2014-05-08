#ifndef INC_stormrainh
#define INC_stormrainh
/*---------------------------------------------------------------------------
 
  FILENAME:
        stormRain.h
 
  PURPOSE:
        Provide utilities to compute "storm" rain and start date.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        01/08/2006      M.S. Teel       0               Original
 
  NOTES:
        We define a storm to begin when an effective rain rate of 0.05 in/hr
        or greater is observed. A storm ends when 12 hours pass with no 
        recorded rainfall.
 
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
#include <string.h>

/*  ... Library include files
*/
#include <sysdefs.h>
#include <radtimeUtils.h>

/*  ... Local include files
*/
#include <sensor.h>
#include <datadefs.h>
#include <dbsqlite.h>
#include "daemon.h"



// work area
typedef struct
{
    float           startTrigger;       // rain rate that starts a storm
    int             idleHours;          // number of "no rain" hours that ends storm
    int             inStorm;            // flag indicating if we are in a storm
    time_t          startTime;          // when the storm start was triggered
    time_t          lastRainTime;       // last recorded rain time
    float           rain;               // accumulated storm rain
    char            strBuffer[32];      // place to return date/time requests
} STORMRAIN_WORK;


// function prototypes

// called once at init
extern void stormRainInit (float startTrigger, int idleHours);

// called for each new archive interval
extern void stormRainUpdate (float rainRate, float rainFall);

extern time_t stormRainGetStartTimeT (void);

// retrieve the storm rain amount
extern float stormRainGet (void);

#endif

