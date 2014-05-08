#ifndef INC_windaverageh
#define INC_windaverageh
/*---------------------------------------------------------------------------
 
  FILENAME:
        windAverage.h
 
  PURPOSE:
        Define the windAverage API.
        This utilizes consensus averaging to avoid the problems associated 
        with values around North (i.e., 359 and 1 averaging to 180, 
        or South, instead of 0, or North).
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        01/31/2004      M.S. Teel       0               Original
 
  NOTES:
        
 
  LICENSE:
        Copyright (c) 2004, Mark S. Teel (mark@teel.ws)
  
        This source code is released for free distribution under the terms 
        of the GNU General Public License.
  
----------------------------------------------------------------------------*/

//  ... includes
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

#include <sysdefs.h>
#include <datadefs.h>


//  ... macro definitions


//  ... API prototypes

extern void windAverageReset (WAVG_ID id);

//  ... add a data point (wind observation) to the data set;
//  ... direction MUST be an integer in the range [0,359]
extern void windAverageAddValue (WAVG_ID id, int direction);

//  ... add a set of wind direction bins:
extern void windAverageAddBins (WAVG_ID id, int* bins);

//  ... use consensus averaging to compute the average wind dir
extern int windAverageCompute (WAVG_ID id);

#endif


