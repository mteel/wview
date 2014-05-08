#ifndef INC_procmonh
#define INC_procmonh
/*---------------------------------------------------------------------------
 
  FILENAME:
        procmon.h
 
  PURPOSE:
        Provide the wview pmon process definitions.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        11/20/2007      M.S. Teel       0               Original
 
  NOTES:
        
 
  LICENSE:
        Copyright (c) 2007, Mark S. Teel (mark@teel.ws)
  
        This source code is released for free distribution under the terms 
        of the GNU General Public License.
  
----------------------------------------------------------------------------*/

//  System include files
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

//  Library include files

#include <sysdefs.h>
#include <radsystem.h>
#include <radsemaphores.h>
#include <radbuffers.h>
#include <radlist.h>
#include <radqueue.h>
#include <radtimers.h>
#include <radevents.h>
#include <radstates.h>
#include <radtimeUtils.h>
#include <radsysutils.h>
#include <radprocess.h>
#include <radprocutils.h>
#include <radsocket.h>
#include <radmsgRouter.h>

//  Local include files

#include <services.h>
#include <sysdefs.h>
#include <status.h>
#include <datadefs.h>
#include <wvconfig.h>



//  !!!!!!!!!!!!!!!!!!  HIDDEN, NOT FOR API USE  !!!!!!!!!!!!!!!!!!

#define PMON_POLL_INTERVAL          60000           // 60 secs
#define PMON_TICK_INTERVAL          1000            // 1 sec
#define PMON_WAIT_EXIT_TIME         2               // 2 sec
#define PMON_WAIT_START_TIME        2               // 2 sec
#define PMON_MAX_PATH               256

typedef enum
{
    PMON_STATS_RESTARTS             = 0
} PMON_STATS;


typedef struct
{
    pid_t           pid;
    char            pidFile[PMON_MAX_PATH];
    char            binFile[PMON_MAX_PATH];
    int             timeout;
    int             ticks;
    STATES_ID       stateMachine;
} PMON_PROCESS;

typedef struct
{
    pid_t           myPid;
    char            pidFile[PMON_MAX_PATH];
    char            fifoFile[PMON_MAX_PATH];
    char            statusFile[PMON_MAX_PATH];
    char            daemonQname[PMON_MAX_PATH];
    char            wviewdir[PMON_MAX_PATH];
    PMON_PROCESS    process[PMON_PROCESS_MAX];
    TIMER_ID        pollTimer;
    TIMER_ID        tickTimer;
    int             inMainLoop;
    int             exiting;
} WVIEW_PMON_WORK;

enum PMONStates
{
    PMON_STATE_IDLE         = 0,
    PMON_STATE_WAIT_RESP,
    PMON_STATE_WAIT_EXIT,
    PMON_STATE_WAIT_START
};

typedef enum
{
    PMON_STIM_TIMEOUT       = 1,
    PMON_STIM_POLL,
    PMON_STIM_RESPONSE
} PMONStimTypes;

typedef struct
{
    int             index;
    PMONStimTypes   type;
} PMON_STIM;

//  !!!!!!!!!!!!!!!!!!!!  END HIDDEN SECTION  !!!!!!!!!!!!!!!!!!!!!

extern int IdleStateHandler (int state, void* stimulus, void* userData);
extern int WaitRespStateHandler (int state, void* stimulus, void* userData);
extern int WaitExitStateHandler (int state, void* stimulus, void* userData);
extern int WaitStartStateHandler (int state, void* stimulus, void* userData);
extern int pmonGetProcessPid (char* pidFilePath);

#endif

