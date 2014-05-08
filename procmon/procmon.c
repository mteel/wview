/*---------------------------------------------------------------------------
 
  FILENAME:
        procmon.c
 
  PURPOSE:
        Provide the wview PMON packet generator entry point.
 
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
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

//  Library include files
#include <radsystem.h>

//  Local include files
#include <procmon.h>


//  static (local) memory declarations
static WVIEW_PMON_WORK procmonWork;

static char*            pmonStatusLabels[STATUS_STATS_MAX] =
{
    "Processes restarted",
    NULL,
    NULL,
    NULL
};

static const char*  configIDs[PMON_PROCESS_MAX] =
{
    configItemPROCMON_wviewd,
    configItemPROCMON_htmlgend,
    configItemPROCMON_wvalarmd,
    configItemPROCMON_wvcwopd,
    configItemPROCMON_wvhttpd
};

static const char*  procNames[PMON_PROCESS_MAX] =
{
    "wviewd",
    "htmlgend",
    "wvalarmd",
    "wvcwopd",
    "wvhttpd"
};

//  methods
void stripEOLValues(char* str)
{
    if (str[strlen(str)-1] == 0xa || str[strlen(str)-1] == 0xd)
    {
        str[strlen(str)-1] = 0;
    }
    if (str[strlen(str)-1] == 0xa || str[strlen(str)-1] == 0xd)
    {
        str[strlen(str)-1] = 0;
    }
}

int pmonGetProcessPid (char* pidFilePath)
{
    FILE        *pidfile;
    int         intValue;

    pidfile = fopen (pidFilePath, "r");
    if (pidfile == NULL)
    {
        radMsgLog (PRI_CATASTROPHIC, "PMON: failed to OPEN %s!", pidFilePath);
        return ERROR;
    }

    if (fscanf (pidfile, "%d", &intValue) != 1)
    {
        radMsgLog (PRI_CATASTROPHIC, "PMON: failed to READ %s!", pidFilePath);
        fclose (pidfile);
        return ERROR;
    }

    fclose (pidfile);
    return intValue;
}

static int waitForWviewDaemon (void)
{
    WVIEW_MSG_REQUEST       msg;
    char                    srcQName[QUEUE_NAME_LENGTH+1];
    UINT                    msgType;
    UINT                    length;
    void                    *recvBfr;
    int                     retVal, done = FALSE;

    // enable message reception from the radlib router for the archive path
    radMsgRouterMessageRegister (WVIEW_MSG_TYPE_STATION_INFO);

    msg.requestType = WVIEW_RQST_TYPE_STATION_INFO;

    if (radMsgRouterMessageSend (WVIEW_MSG_TYPE_REQUEST, &msg, sizeof(msg)) == ERROR)
    {
        radMsgLog (PRI_HIGH, "waitForWviewDaemon: radMsgRouterMessageSend failed!");
        radMsgRouterMessageDeregister (WVIEW_MSG_TYPE_STATION_INFO);
        return ERROR;
    }

    // now wait for the response here
    while (!done)
    {
        radUtilsSleep (50);
        
        
        if ((retVal = radQueueRecv (radProcessQueueGetID (),
                                    srcQName,
                                    &msgType,
                                    &recvBfr,
                                    &length))
            == FALSE)
        {
            continue;
        }
        else if (retVal == ERROR)
        {
            radMsgLog (PRI_STATUS, "waitForWviewDaemon: queue is closed!");
            radMsgRouterMessageDeregister (WVIEW_MSG_TYPE_STATION_INFO);
            return ERROR;
        }
    
        // is this what we want?
        if (msgType == WVIEW_MSG_TYPE_STATION_INFO)
        {
            // yes!
            done = TRUE;
        }
        else if (msgType == WVIEW_MSG_TYPE_SHUTDOWN)
        {
            radMsgLog (PRI_HIGH, "waitForWviewDaemon: received shutdown from wviewd"); 
            radMsgRouterMessageDeregister (WVIEW_MSG_TYPE_STATION_INFO);
            return ERROR;
        }
    
        // release the received buffer
        radBufferRls (recvBfr);
        }
    
    // disable message reception from the radlib router for the archive path
    radMsgRouterMessageDeregister (WVIEW_MSG_TYPE_STATION_INFO);

    return OK;
}

//  system initialization
static int procmonSysInit (WVIEW_PMON_WORK *work)
{
    char            devPath[256], temp[64];
    struct stat     fileData;

    //  check for our daemon's pid file, don't run if it isn't there
    sprintf (devPath, "%s/%s", WVIEW_RUN_DIR, WVD_LOCK_FILE_NAME);
    if (stat (devPath, &fileData) != 0)
    {
        radMsgLogInit (PROC_NAME_PMON, TRUE, TRUE);
        radMsgLog (PRI_CATASTROPHIC, 
                   "wviewd process not running - aborting!");
        radMsgLogExit ();
        return ERROR;
    }

    sprintf (work->pidFile, "%s/%s", WVIEW_RUN_DIR, PMON_LOCK_FILE_NAME);
    sprintf (work->fifoFile, "%s/dev/%s", WVIEW_RUN_DIR, PROC_NAME_PMON);
    sprintf (work->statusFile, "%s/%s", WVIEW_STATUS_DIRECTORY, PMON_STATUS_FILE_NAME);
    sprintf (work->daemonQname, "%s/dev/%s", WVIEW_RUN_DIR, PROC_NAME_DAEMON);
    sprintf (work->wviewdir, "%s", WVIEW_RUN_DIR);

    //  check for our pid file, don't run if it IS there
    if (stat (work->pidFile, &fileData) == 0)
    {
        radMsgLogInit (PROC_NAME_PMON, TRUE, TRUE);
        radMsgLog (PRI_CATASTROPHIC, 
                   "lock file %s exists, older copy may be running - aborting!",
                   work->pidFile);
        radMsgLogExit ();
        return ERROR;
    }

    return OK;
}

//  system exit
static int procmonSysExit (WVIEW_PMON_WORK *work)
{
    struct stat     fileData;

    //  delete our pid file
    if (stat (work->pidFile, &fileData) == 0)
    {
        unlink (work->pidFile);
    }

    return OK;
}

static void defaultSigHandler (int signum)
{
    int         retVal;

    switch (signum)
    {
        case SIGHUP:
            // user wants us to change the verbosity setting
            retVal = wvutilsToggleVerbosity ();
            radMsgLog (PRI_STATUS, "wvpmond: SIGHUP - toggling log verbosity %s",
                       ((retVal == 0) ? "OFF" : "ON"));

            radProcessSignalCatch(signum, defaultSigHandler);
            return;

        case SIGPIPE:
            // we have a far end socket disconnection, we'll handle it in the
            // "read/write" code
            radProcessSignalCatch(signum, defaultSigHandler);
            break;

        case SIGILL:
        case SIGBUS:
        case SIGFPE:
        case SIGSEGV:
        case SIGXFSZ:
        case SIGSYS:
            // unrecoverable radProcessSignalCatch- we must exit right now!
            radMsgLog (PRI_CATASTROPHIC, 
                       "wvpmond: recv unrecoverable signal %d: aborting!",
                       signum);
            abort ();
        
        case SIGCHLD:
            wvutilsWaitForChildren();
            radProcessSignalCatch(signum, defaultSigHandler);
            return;

        default:
            if (procmonWork.exiting)
            {
                radProcessSignalCatch(signum, defaultSigHandler);
                return;
            }

            procmonWork.exiting = TRUE;
            radProcessSetExitFlag ();

            // can we allow the process to exit normally?
            if (!procmonWork.inMainLoop)
            {
                // NO! - we gotta bail here!
                radMsgLog (PRI_HIGH, "wvpmond: recv signal %d: exiting now!", signum);
                radMsgRouterExit ();
                procmonSysExit (&procmonWork);
                radProcessExit ();
                radSystemExit (WVIEW_SYSTEM_ID);
                exit (0);                
            }
            
            // we can allow the process to exit normally...
            radMsgLog (PRI_HIGH, "wvpmond: recv signal %d: exiting!", signum);
        
            radProcessSignalCatch(signum, defaultSigHandler);
            break;
    }
    
    return;
}

static void msgHandler
(
    char                        *srcQueueName,
    UINT                        msgType,
    void                        *msg,
    UINT                        length,
    void                        *userData
)
{
    WVIEW_MSG_POLL_RESPONSE*    respMsg;
    int                         i;
    PMON_STIM                   stim;
    
    if (msgType == WVIEW_MSG_TYPE_POLL_RESPONSE)
    {
        // process poll response:
        respMsg = (WVIEW_MSG_POLL_RESPONSE*)msg;

        // Clear the corresponding process ticks:
        for (i = 0; i < PMON_PROCESS_MAX; i ++)
        {
            if (procmonWork.process[i].pid == respMsg->pid)
            {
                // Clear his tick counter:
                procmonWork.process[i].ticks = 0;

                // Tickle the state machine
                memset (&stim, 0, sizeof(stim));
                stim.index = i;
                stim.type  = PMON_STIM_RESPONSE;
                radStatesProcess (procmonWork.process[i].stateMachine, &stim);
                return;
            }
        }
    }

    return;
}

static void evtHandler
(
    UINT        eventsRx,
    UINT        rxData,
    void        *userData
)
{
    return;
}

static void pollTimerHandler (void *parm)
{
    int                 i, mask = 0;
    WVIEW_MSG_POLL      poll;
    PMON_STIM           stim;

    // Set active process bits:
    for (i = 0; i < PMON_PROCESS_MAX; i ++)
    {
        if (procmonWork.process[i].timeout <= 0 || 
            radStatesGetState (procmonWork.process[i].stateMachine) != PMON_STATE_IDLE)
        {
            // Skip this guy
            continue;
        }

        mask = PMON_PROCESS_SET(mask,i);
        procmonWork.process[i].ticks = procmonWork.process[i].timeout;

        // Tickle the state machine
        memset (&stim, 0, sizeof(stim));
        stim.index = i;
        stim.type  = PMON_STIM_POLL;
        radStatesProcess (procmonWork.process[i].stateMachine, &stim);
    }

    poll.mask = mask;
    if (radMsgRouterMessageSend (WVIEW_MSG_TYPE_POLL, &poll, sizeof(poll)) 
        == ERROR)
    {
        radMsgLog (PRI_HIGH, "pollTimerHandler: radMsgRouterMessageSend failed!");
    }

    radProcessTimerStart (procmonWork.pollTimer, PMON_POLL_INTERVAL);
    return;
}

static void tickTimerHandler (void *parm)
{
    int             i;
    PMON_STIM       stim;

    for (i = 0; i < PMON_PROCESS_MAX; i ++)
    {
        if (procmonWork.process[i].timeout <= 0 || 
            procmonWork.process[i].ticks <= 0)
        {
            // Skip this guy
            continue;
        }

        procmonWork.process[i].ticks --;
        if (procmonWork.process[i].ticks == 0)
        {
            // Tickle the state machine
            memset (&stim, 0, sizeof(stim));
            stim.index = i;
            stim.type  = PMON_STIM_TIMEOUT;
            radStatesProcess (procmonWork.process[i].stateMachine, &stim);
        }
    }

    radProcessTimerStart (procmonWork.tickTimer, PMON_TICK_INTERVAL);
    return;
}

//  the main entry point for the wvpmond process
int main (int argc, char *argv[])
{
    void            (*alarmHandler)(int);
    int             i, retVal, intValue, iValue;
    FILE            *pidfile, *markerFile;
    struct stat     fileStatus;
    char*           subString;
    char            markerPath[PMON_MAX_PATH], markerVal[32];
    int             runAsDaemon = TRUE;

    if (argc > 1)
    {
        if (!strcmp(argv[1], "-f"))
        {
            runAsDaemon = FALSE;
        }
    }

    memset (&procmonWork, 0, sizeof (procmonWork));

    //  initialize some system stuff first
    retVal = procmonSysInit (&procmonWork);
    if (retVal == ERROR)
    {
        radMsgLogInit (PROC_NAME_PMON, FALSE, TRUE);
        radMsgLog (PRI_CATASTROPHIC, "wvpmond init failed!\n");
        radMsgLogExit ();
        exit (1);
    }
    else if (retVal == ERROR_ABORT)
    {
        exit (2);
    }


    //  call the global radlib system init function
    if (radSystemInit (WVIEW_SYSTEM_ID) == ERROR)
    {
        radMsgLogInit (PROC_NAME_PMON, TRUE, TRUE);
        radMsgLog (PRI_CATASTROPHIC, "radSystemInit failed!");
        radMsgLogExit ();
        exit (1);
    }


    //  call the radlib process init function
    if (radProcessInit (PROC_NAME_PMON,
                        procmonWork.fifoFile,
                        PROC_NUM_TIMERS_PMON,
                        runAsDaemon,                    // TRUE for daemon
                        msgHandler,
                        evtHandler,
                        NULL)
        == ERROR)
    {
        printf ("\nradProcessInit failed: %s\n\n", PROC_NAME_PMON);
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }

    procmonWork.myPid = getpid ();
    pidfile = fopen (procmonWork.pidFile, "w");
    if (pidfile == NULL)
    {
        radMsgLog (PRI_CATASTROPHIC, "lock file create failed!\n");
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }
    fprintf (pidfile, "%d", getpid ());
    fclose (pidfile);


    alarmHandler = radProcessSignalGetHandler (SIGALRM);
    radProcessSignalCatchAll (defaultSigHandler);
    radProcessSignalCatch (SIGALRM, alarmHandler);
    radProcessSignalRelease(SIGABRT);

    if (wvconfigInit(FALSE) == ERROR)
    {
        radMsgLog (PRI_CATASTROPHIC, "wvconfigInit failed!\n");
        radMsgRouterExit ();
        procmonSysExit (&procmonWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }

    // Is the procmon daemon enabled?
    iValue = wvconfigGetBooleanValue(configItem_ENABLE_PROCMON);
    if (iValue == ERROR || iValue == 0)
    {
        radMsgLog (PRI_CATASTROPHIC, "process monitor daemon is disabled - exiting...");
        wvconfigExit ();
        radMsgRouterExit ();
        procmonSysExit (&procmonWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }

    // Close here; we'll re-open after waiting for the wviewd daemon to be ready:
    wvconfigExit();

    if (statusInit(procmonWork.statusFile, pmonStatusLabels) == ERROR)
    {
        radMsgLog (PRI_HIGH, "statusInit failed - exiting...");
        wvconfigExit ();
        radMsgRouterExit ();
        procmonSysExit (&procmonWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }
    statusUpdate(STATUS_BOOTING);

    //  register with the radlib message router
    if (radMsgRouterInit (WVIEW_RUN_DIR) == ERROR)
    {
        radMsgLog (PRI_HIGH, "radMsgRouterInit failed!");
        procmonSysExit (&procmonWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }

    // enable message reception from the radlib router for SHUTDOWN msgs
    radMsgRouterMessageRegister (WVIEW_MSG_TYPE_SHUTDOWN);

    // wait for the wview daemon to be ready
    if (waitForWviewDaemon () == ERROR)
    {
        radMsgLog (PRI_HIGH, "waitForWviewDaemon failed");
        statusUpdate(STATUS_ERROR);
        radMsgRouterExit ();
        procmonSysExit (&procmonWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }

    procmonWork.pollTimer = radProcessTimerCreate (NULL, pollTimerHandler, NULL);
    if (procmonWork.pollTimer == NULL)
    {
        radMsgLog (PRI_HIGH, "radTimerCreate failed");
        procmonSysExit (&procmonWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }

    procmonWork.tickTimer = radProcessTimerCreate (NULL, tickTimerHandler, NULL);
    if (procmonWork.tickTimer == NULL)
    {
        radMsgLog (PRI_HIGH, "radTimerCreate failed");
        procmonSysExit (&procmonWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }

    if (wvconfigInit(FALSE) == ERROR)
    {
        radMsgLog (PRI_CATASTROPHIC, "wvconfigInit failed!\n");
        statusUpdateMessage("wvconfigInit failed");
        statusUpdate(STATUS_ERROR);
        radMsgRouterExit ();
        procmonSysExit (&procmonWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }

    // Populate our process table:
    memset (procmonWork.process, 0, sizeof(PMON_PROCESS) * PMON_PROCESS_MAX);
    for (i = 0; i < PMON_PROCESS_MAX; i ++)
    {
        iValue = wvconfigGetINTValue(configIDs[i]);
        if (iValue < 0)
        {
            // we can't do without this!
            radMsgLog (PRI_CATASTROPHIC, "PMON: %s: failed!", configIDs[i]);
            statusUpdateMessage("wvconfigGetINTValue failed");
            statusUpdate(STATUS_ERROR);
            wvconfigExit();
            radMsgRouterExit ();
            procmonSysExit (&procmonWork);
            radProcessExit ();
            radSystemExit (WVIEW_SYSTEM_ID);
            exit (1);
        }
        else
        {
            procmonWork.process[i].timeout = iValue;
            radMsgLog (PRI_STATUS, "PMON: %s: %d", 
                       procNames[i], procmonWork.process[i].timeout);
        }

        if (procmonWork.process[i].timeout <= 0)
        {
            radMsgLog (PRI_STATUS, "PMON: %s process monitoring is disabled", 
                       procNames[i]);
            procmonWork.process[i].ticks = -1;
            procmonWork.process[i].pid = -1;
        }
        else
        {
            // Populate the rest of the values:
            wvstrncpy (procmonWork.process[i].binFile, argv[0], PMON_MAX_PATH);
            subString = strstr (procmonWork.process[i].binFile, "wvpmond");
            if (subString == NULL)
            {
                radMsgLog (PRI_CATASTROPHIC, "PMON: %s is invalid for bin path!", 
                           procmonWork.process[i].binFile);
                statusUpdateMessage("invalid for bin path");
                statusUpdate(STATUS_ERROR);
                wvconfigExit();
                radMsgRouterExit ();
                procmonSysExit (&procmonWork);
                radProcessExit ();
                radSystemExit (WVIEW_SYSTEM_ID);
                exit (1);
            }

            if (! strcmp(procNames[i], "wviewd"))
            {
                // Get the process name from the marker file:
                sprintf(markerPath, "%s/wview-binary", WVIEW_CONFIG_DIR);
                if (stat (markerPath, &fileStatus) != 0)
                {
                    radMsgLog (PRI_HIGH, "PMON: wview-binary file is missing - was 'make install' run?"); 
                    statusUpdateMessage("wview-binary file is missing");
                    statusUpdate(STATUS_ERROR);
                    radMsgRouterExit ();
                    procmonSysExit (&procmonWork);
                    radProcessExit ();
                    radSystemExit (WVIEW_SYSTEM_ID);
                    exit (1);
                }

                markerFile = fopen(markerPath, "r");
                if (markerFile == NULL)
                {
                    radMsgLog (PRI_HIGH, "PMON: failed to open wview-binary file - was 'make install' run?"); 
                    statusUpdateMessage("failed to open wview-binary file");
                    statusUpdate(STATUS_ERROR);
                    radMsgRouterExit ();
                    procmonSysExit (&procmonWork);
                    radProcessExit ();
                    radSystemExit (WVIEW_SYSTEM_ID);
                    exit (1);
                }
                if (fgets(markerVal, 32, markerFile) == NULL)
                {
                    radMsgLog (PRI_HIGH, "PMON: failed to read wview-binary file - was 'make install' run?"); 
                    statusUpdateMessage("failed to read wview-binary file");
                    statusUpdate(STATUS_ERROR);
                    fclose(markerFile);
                    radMsgRouterExit ();
                    procmonSysExit (&procmonWork);
                    radProcessExit ();
                    radSystemExit (WVIEW_SYSTEM_ID);
                    exit (1);
                }

                // Strip off CR/LF (caused by hand-editing of file!):
                stripEOLValues(markerVal);
                sprintf(subString, "%s", markerVal);
                fclose(markerFile);
            }
            else
            {
                sprintf (subString, "%s", procNames[i]);
            }

            sprintf (procmonWork.process[i].pidFile, "%s/%s.pid",
                     WVIEW_RUN_DIR, procNames[i]);
            if (stat (procmonWork.process[i].pidFile, &fileStatus) == -1)
            {
                // Process is not running, mark it invalid
                radMsgLog (PRI_STATUS, "PMON: pid file %s not present, disable monitoring...",
                           procmonWork.process[i].pidFile);
                procmonWork.process[i].timeout = 0;
                procmonWork.process[i].ticks = -1;
                procmonWork.process[i].pid = -1;
            }
            else
            {
                // Get his pid from the pidfile
                procmonWork.process[i].pid = pmonGetProcessPid (procmonWork.process[i].pidFile);
                if (procmonWork.process[i].pid == ERROR)
                {
                    radMsgLog (PRI_CATASTROPHIC, "PMON: failed to READ %s!",
                               procmonWork.process[i].pidFile);
                    statusUpdateMessage("failed to read pid file");
                    statusUpdate(STATUS_ERROR);
                    wvconfigExit();
                    radMsgRouterExit ();
                    procmonSysExit (&procmonWork);
                    radProcessExit ();
                    radSystemExit (WVIEW_SYSTEM_ID);
                    exit (1);
                }
            }

            // Setup the state machine
            procmonWork.process[i].stateMachine = radStatesInit (&procmonWork);
            radStatesAddHandler (procmonWork.process[i].stateMachine,
                                 PMON_STATE_IDLE,
                                 IdleStateHandler);
            radStatesAddHandler (procmonWork.process[i].stateMachine,
                                 PMON_STATE_WAIT_RESP,
                                 WaitRespStateHandler);
            radStatesAddHandler (procmonWork.process[i].stateMachine,
                                 PMON_STATE_WAIT_EXIT,
                                 WaitExitStateHandler);
            radStatesAddHandler (procmonWork.process[i].stateMachine,
                                 PMON_STATE_WAIT_START,
                                 WaitStartStateHandler);
            radStatesSetState (procmonWork.process[i].stateMachine, PMON_STATE_IDLE);
        }
    }

    wvconfigExit();


    radMsgRouterMessageRegister (WVIEW_MSG_TYPE_POLL_RESPONSE);

    // Start the timers:
    radProcessTimerStart (procmonWork.pollTimer, PMON_POLL_INTERVAL);
    radProcessTimerStart (procmonWork.tickTimer, PMON_TICK_INTERVAL);


    // enter normal processing
    procmonWork.inMainLoop = TRUE;
    statusUpdate(STATUS_RUNNING);
    statusUpdateMessage("Normal operation");
    radMsgLog (PRI_STATUS, "running...");
    
    while (!procmonWork.exiting)
    {
        // wait on something interesting
        if (radProcessWait (0) == ERROR)
        {
            procmonWork.exiting = TRUE;
        }
    }


    statusUpdateMessage("exiting normally");
    radMsgLog (PRI_STATUS, "exiting normally...");
    statusUpdate(STATUS_SHUTDOWN);

    radMsgRouterExit ();
    radProcessTimerDelete (procmonWork.pollTimer);
    radProcessTimerDelete (procmonWork.tickTimer);
    procmonSysExit (&procmonWork);
    radProcessExit ();
    radSystemExit (WVIEW_SYSTEM_ID);
    exit (0);
}

