#ifndef INC_statush
#define INC_statush
/*---------------------------------------------------------------------------
 
  FILENAME:
        status.h
 
  PURPOSE:
        Define the status reporting API.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        12/27/2009      M.S. Teel       0               Original
 
  NOTES:
        
 
  LICENSE:
        Copyright (c) 2009, Mark S. Teel (mark@teel.ws)
  
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
#include <services.h>


//  ... definitions

#define STATUS_STATS_MAX        4

typedef enum
{
    STATUS_NOT_STARTED          = 0,
    STATUS_BOOTING              = 1,
    STATUS_WAITING_FOR_WVIEW,
    STATUS_RUNNING,
    STATUS_SHUTDOWN,
    STATUS_ERROR
} STATUS_TYPE;
    
typedef struct
{
    char            filePath[_MAX_PATH];
    STATUS_TYPE     status;
    char            lastMessage[_MAX_PATH];
    int             stat[STATUS_STATS_MAX];
    char            statLabel[STATUS_STATS_MAX][64];
} STATUS_INFO;


//  ... API prototypes

//  ... initialize the status log:
extern int statusInit(const char* filePath, char* statLabel[STATUS_STATS_MAX]);

//  ... send a status update:
extern int statusUpdate(STATUS_TYPE status);

//  ... send a status update:
extern int statusUpdateMessage(const char* message);

//  ... send a status update:
extern int statusUpdateStat(int index, int value);

extern int statusIncrementStat(int index);

// Does not allow the value to be negative:
extern int statusDecrementStat(int index);

#endif


