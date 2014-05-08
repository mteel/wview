#ifndef INC_alarmsh
#define INC_alarmsh
/*---------------------------------------------------------------------------
 
  FILENAME:
        alarms.h
 
  PURPOSE:
        Provide the wview alarm process definitions.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        05/22/2005      M.S. Teel       0               Original
 
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

/*  ... Library include files
*/
#include <sysdefs.h>
#include <radsystem.h>
#include <radsemaphores.h>
#include <radbuffers.h>
#include <radlist.h>
#include <radqueue.h>
#include <radtimers.h>
#include <radevents.h>
#include <radtimeUtils.h>
#include <radsysutils.h>
#include <radprocess.h>
#include <radsocket.h>
#include <radmsgRouter.h>

/*  ... Local include files
*/
#include <services.h>
#include <datadefs.h>
#include <datafeed.h>
#include <wvconfig.h>
#include <dbsqlite.h>
#include <status.h>


/*  ... API definitions
*/

//  define alarm types here
typedef enum
{
    Barometer           = 0,
    InsideTemp,
    InsideHumidity,
    OutsideTemp,
    WindSpeed,
    TenMinuteAvgWindSpeed,
    WindDirection,
    OutsideHumidity,
    RainRate,
    StormRain,
    DayRain,
    MonthRain,
    YearRain,
    TxBatteryStatus,
    ConsoleBatteryVoltage,
    DewPoint,
    WindChill,
    HeatIndex,
    Radiation,
    UV,
    ET,

    // VP-specific values
    ExtraTemp1,
    ExtraTemp2,
    ExtraTemp3,
    SoilTemp1,
    SoilTemp2,
    SoilTemp3,
    SoilTemp4,
    LeafTemp1,
    LeafTemp2,
    ExtraHumid1,
    ExtraHumid2,

    // WXT-510-specific values
    Wxt510Hail,
    Wxt510Hailrate,
    Wxt510HeatingTemp,
    Wxt510HeatingVoltage,
    Wxt510SupplyVoltage,
    Wxt510ReferenceVoltage

} ALARM_TYPE;


/*  !!!!!!!!!!!!!!!!!!  HIDDEN, NOT FOR API USE  !!!!!!!!!!!!!!!!!!
*/

#define WVIEW_ALARM_SCRIPT_LENGTH       512

typedef enum
{
    ALARM_STATS_ALARMS          = 0,
    ALARM_STATS_SCRIPTS_RUN,
    ALARM_STATS_CLIENTS,
    ALARM_STATS_PKTS_SENT
} ALARM_STATS;


// define an alarm definition structure
typedef struct
{
    NODE            node;
    int             type;               // see above (ALARM_TYPE)
    int             isMax;              // if != 0, value is an upper bound
                                        // else it is a lower bound
    float           bound;
    int             abateSecs;          // alarm abatement after trigger
    uint32_t        abateStart;
    char            scriptToRun[WVIEW_ALARM_SCRIPT_LENGTH];
    int             triggered;          // to prevent repeat notifications
    float           triggerValue;
} WVIEW_ALARM;

typedef struct
{
    NODE            node;
    RADSOCK_ID      client;
    int             syncInProgress;
} WVIEW_ALARM_CLIENT;

typedef struct
{
    pid_t           myPid;
    char            pidFile[128];
    char            fifoFile[128];
    char            statusFile[128];
    char            daemonQname[128];
    char            wviewdir[128];
    int             isMetric;
    int             doTest;
    int             doTestNumber;
    RADLIST         alarmList;
    RADLIST         clientList;
    RADSOCK_ID      dataFeedServer;
    int             inMainLoop;
    int             sigpipe;
    int             exiting;
} WVIEW_ALARM_WORK;

/*  !!!!!!!!!!!!!!!!!!!!  END HIDDEN SECTION  !!!!!!!!!!!!!!!!!!!!!
*/

// Define the maximum number of alarm definitions:
#define ALARMS_MAX              10


/* ... API function prototypes
*/



#endif
