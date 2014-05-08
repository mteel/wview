#ifndef INC_htmlh
#define INC_htmlh
/*---------------------------------------------------------------------------
 
  FILENAME:
        html.h
 
  PURPOSE:
        Provide the wview HTML generator definitions.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        08/27/03        M.S. Teel       0               Original
 
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
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

/*  ... Library include files
*/
#include <sysdefs.h>
#include <radsemaphores.h>
#include <radbuffers.h>
#include <radqueue.h>
#include <radtimers.h>
#include <radevents.h>
#include <radtimeUtils.h>
#include <radsysutils.h>
#include <radprocess.h>
#include <radstates.h>
#include <radmsgRouter.h>

/*  ... Local include files
*/
#include <sensor.h>
#include <dbsqlite.h>
#include <datadefs.h>
#include <services.h>
#include <status.h>
#include <htmlMgr.h>
#include <noaaGenerate.h>
#include <arcrecGenerate.h>


/*  !!!!!!!!!!!!!!!!!!  HIDDEN, NOT FOR API USE  !!!!!!!!!!!!!!!!!!
*/
#define TIMER_GENERATE              1
#define TIMER_RX_PACKETS            2

#define HTML_NOAA_UPDATE_DELAY      30000           // 30 secs

#define HTML_RX_PACKETS_TIMEOUT     60000           // 60 secs


typedef enum
{
    HTML_STATS_IMAGES_DEFINED       = 0,
    HTML_STATS_TEMPLATES_DEFINED,
    HTML_STATS_IMAGES_GENERATED,
    HTML_STATS_TEMPLATES_GENERATED
} HTML_STATS;

typedef struct
{
    pid_t           myPid;
    char            pidFile[WVIEW_STRING2_SIZE];
    char            indicateFile[WVIEW_STRING2_SIZE];
    char            fifoFile[WVIEW_STRING2_SIZE];
    char            statusFile[WVIEW_STRING2_SIZE];
    char            daemonQname[WVIEW_STRING2_SIZE];
    char            configFile[WVIEW_STRING2_SIZE];
    int             archiveInterval;
    int             isMetricUnits;
    HTML_WUNITS     windUnits;
    char            imagePath[WVIEW_STRING2_SIZE];
    char            htmlPath[WVIEW_STRING2_SIZE];
    int             isExtendedData;
    STATES_ID       stateMachine;
    char            stationName[WVIEW_STRING1_SIZE];
    char            stationCity[WVIEW_STRING1_SIZE];
    char            stationState[WVIEW_STRING1_SIZE];
    int             arcrecDaysToKeep;
    char            mphaseIncrease[32];
    char            mphaseDecrease[32];
    char            mphaseFull[32];
    char            radarURL[WVIEW_STRING2_SIZE];
    char            forecastURL[WVIEW_STRING2_SIZE];
    TIMER_ID        timer;
    TIMER_ID        rxTimer;
    TIMER_ID        noaaTimer;
    struct timeval  nextGenerationTime;
    int             timerInterval;
    int             startOffset;
    time_t          LastArchiveDateTime;
    int             histLastHour;
    int             histLastDay;
    HTML_MGR_ID     mgrId;
    NOAA_ID         noaaId;
    int             numDataReceived;
    int             exiting;
    char            dateFormat[WVIEW_STRING1_SIZE];
    int             isDualUnits;
} HTML_WORK;


typedef enum
{
    HTML_STATE_IDLE             = 1,
    HTML_STATE_STATION_INFO,
    HTML_STATE_RUN,
    HTML_STATE_DATA,
    HTML_STATE_ERROR
} HTML_STATES;


extern int htmlIdleState (int state, void *stimulus, void *data);
extern int htmlStationInfoState (int state, void *stimulus, void *data);
extern int htmlRunState (int state, void *stimulus, void *data);
extern int htmlDataState (int state, void *stimulus, void *data);
extern int htmlErrorState (int state, void *stimulus, void *data);


/*  !!!!!!!!!!!!!!!!!!!!  END HIDDEN SECTION  !!!!!!!!!!!!!!!!!!!!!
*/



/* ... API function prototypes
*/

#endif
