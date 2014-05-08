/*---------------------------------------------------------------------------
 
  FILENAME:
        ssh.c
 
  PURPOSE:
        Provide the wview ssh generator entry point.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        05/28/2005      M.S. Teel       0               Original
 
  NOTES:
        
 
  LICENSE:
        Copyright (c) 2004, Mark S. Teel (mark@teel.ws)
  
        This source code is released for free distribution under the terms 
        of the GNU General Public License.
  
----------------------------------------------------------------------------*/

/*  ... System include files
*/
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/*  ... Library include files
*/
#include <radsystem.h>

/*  ... Local include files
*/
#include <ssh.h>


/*  ... global memory declarations
*/

/*  ... global memory referenced
*/

/*  ... static (local) memory declarations
*/
static SSH_WORK         sshWork;

static char*            sshStatusLabels[STATUS_STATS_MAX] =
{
    "Rules defined",
    "Rules sent",
    NULL,
    NULL
};

/* ... methods
*/
/*  ... system initialization
*/
static int sshSysInit (SSH_WORK *work)
{
    char            devPath[256], temp[64];
    struct stat     fileData;

    /*  ... check for our daemon's pid file, don't run if it isn't there
    */
    sprintf (devPath, "%s/%s", WVIEW_RUN_DIR, WVD_LOCK_FILE_NAME);
    if (stat (devPath, &fileData) != 0)
    {
        radMsgLogInit (PROC_NAME_SSH, TRUE, TRUE);
        radMsgLog (PRI_CATASTROPHIC, 
                   "wviewd process not running - aborting!");
        radMsgLogExit ();
        return ERROR;
    }

    sprintf (work->pidFile, "%s/%s", WVIEW_RUN_DIR, SSH_LOCK_FILE_NAME);
    sprintf (work->fifoFile, "%s/dev/%s", WVIEW_RUN_DIR, PROC_NAME_SSH);
    sprintf (work->statusFile, "%s/%s", WVIEW_STATUS_DIRECTORY, SSH_STATUS_FILE_NAME);
    sprintf (work->daemonQname, "%s/dev/%s", WVIEW_RUN_DIR, PROC_NAME_DAEMON);
    sprintf (work->wviewdir, "%s", WVIEW_RUN_DIR);

    /*  ... check for our pid file, don't run if it IS there
    */
    if (stat (work->pidFile, &fileData) == 0)
    {
        radMsgLogInit (PROC_NAME_SSH, TRUE, TRUE);
        radMsgLog (PRI_CATASTROPHIC, 
                   "lock file %s exists, older copy may be running - aborting!",
                   work->pidFile);
        radMsgLogExit ();
        return ERROR;
    }

    return OK;
}

/*  ... system exit
*/
static int sshSysExit (SSH_WORK *work)
{
    struct stat     fileData;

    /*  ... delete our pid file
    */
    if (stat (work->pidFile, &fileData) == 0)
    {
        unlink (work->pidFile);
    }

    return OK;
}


void SSHDefaultSigHandler (int signum)
{
    int         retVal;

    switch (signum)
    {
        case SIGHUP:
            // user wants us to change the verbosity setting
            retVal = wvutilsToggleVerbosity ();
            radMsgLog (PRI_STATUS, "wviewsshd: SIGHUP - toggling log verbosity %s",
                       ((retVal == 0) ? "OFF" : "ON"));

            radProcessSignalCatch(signum, SSHDefaultSigHandler);
            return;

        case SIGBUS:
        case SIGFPE:
        case SIGSEGV:
        case SIGXFSZ:
        case SIGSYS:
            // unrecoverable radProcessSignalCatch- we must exit right now!
            radMsgLog (PRI_CATASTROPHIC, 
                       "wviewsshd: recv unrecoverable signal %d: aborting!",
                       signum);
            if (!sshWork.exiting)
            {
                radTimerDelete (sshWork.timer);
                sshSysExit (&sshWork);
                radProcessExit ();
                radSystemExit (WVIEW_SYSTEM_ID);
            }
            abort ();
        
        case SIGCHLD:
            // it is normal behavior to have children finishing up
            wvutilsWaitForChildren();
            radProcessSignalCatch(signum, SSHDefaultSigHandler);
            break;

        default:
            // we can allow the process to exit normally...
            if (sshWork.exiting)
            {
                radProcessSignalCatch(signum, SSHDefaultSigHandler);
                return;
            }
        
            radMsgLog (PRI_HIGH, "wviewsshd: recv signal %d: exiting!", signum);
        
            sshWork.exiting = TRUE;
            radProcessSetExitFlag ();
        
            radProcessSignalCatch(signum, SSHDefaultSigHandler);
            break;
    }

    return;
}

static void msgHandler
(
    char        *srcQueueName,
    UINT        msgType,
    void        *msg,
    UINT        length,
    void        *userData
)
{
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

static void timerHandler (void *parm)
{
    uint64_t    msOffset = radTimeGetMSSinceEpoch ();
    int64_t     netOffset;
    uint64_t    xoffset;

    //  ... allow for timer latency
    if (sshWork.msOffset == 0ULL)
    {
        // first time through
        sshWork.msOffset = msOffset;
        netOffset = 0LL;
    }
    else
    {
        netOffset = msOffset - sshWork.msOffset;
    }

    sshWork.msOffset += 60000ULL;               // ALWAYS 1 minute
    xoffset = sshWork.msOffset;

    // Cope with long scheduling delays or with the time being changed
    int32_t timerWait = (int32_t)(60000LL - netOffset);
    if ((timerWait <= 0) || (timerWait > 120000))
    {
        radMsgLog (PRI_HIGH, "wviewsshd: unusual wait %d; fixing it up", timerWait);
        // Keep the offset into the minute to keep updates aligned with the original alignment.
        uint64_t baseNow = (msOffset / 60000ULL) * 60000ULL;
        netOffset = 60000ULL;
        sshWork.msOffset = baseNow + netOffset + (sshWork.msOffset % 60000ULL);
        radMsgLog (PRI_HIGH, "wviewsshd: fixed offset is %llu", sshWork.msOffset);
    }

    radProcessTimerStart (sshWork.timer, (uint32_t)(60000LL - netOffset));

    //  ... process the ssh rules
    sshUtilsSendFiles (sshWork.sshId, sshWork.wviewdir);

    return;
}


/*  ... the main entry point for the ssh process
*/
int main (int argc, char *argv[])
{
    void            (*alarmHandler)(int);
    STIM            stim;
    int             i;
    int             seconds;
    time_t          ntime;
    struct tm       locTime;
    int             offset, retVal;
    long            msOffset;
    FILE            *pidfile;
    int             runAsDaemon = TRUE;

    if (argc > 1)
    {
        if (!strcmp(argv[1], "-f"))
        {
            runAsDaemon = FALSE;
        }
    }

    memset (&sshWork, 0, sizeof (sshWork));

    /*  ... initialize some system stuff first
    */
    retVal = sshSysInit (&sshWork);
    if (retVal == ERROR)
    {
        radMsgLogInit (PROC_NAME_SSH, FALSE, TRUE);
        radMsgLog (PRI_CATASTROPHIC, "ssh init failed!");
        radMsgLogExit ();
        exit (1);
    }
    else if (retVal == ERROR_ABORT)
    {
        exit (2);
    }


    /*  ... call the global radlib system init function
    */
    if (radSystemInit (WVIEW_SYSTEM_ID) == ERROR)
    {
        radMsgLogInit (PROC_NAME_SSH, TRUE, TRUE);
        radMsgLog (PRI_CATASTROPHIC, "radSystemInit failed!");
        radMsgLogExit ();
        exit (1);
    }


    /*  ... call the radlib process init function
    */
    if (radProcessInit (PROC_NAME_SSH,
                        sshWork.fifoFile,
                        PROC_NUM_TIMERS_SSH,
                        runAsDaemon,                // TRUE for daemon
                        msgHandler,
                        evtHandler,
                        NULL)
        == ERROR)
    {
        printf ("\nradProcessInit failed: %s\n\n", PROC_NAME_SSH);
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }

    sshWork.myPid = getpid ();
    pidfile = fopen (sshWork.pidFile, "w");
    if (pidfile == NULL)
    {
        radMsgLog (PRI_CATASTROPHIC, "lock file create failed!");
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }
    fprintf (pidfile, "%d", getpid ());
    fclose (pidfile);


    alarmHandler = radProcessSignalGetHandler (SIGALRM);
    radProcessSignalCatchAll (SSHDefaultSigHandler);
    radProcessSignalCatch (SIGALRM, alarmHandler);
    radProcessSignalRelease(SIGABRT);


    sshWork.timer = radTimerCreate (NULL, timerHandler, NULL);
    if (sshWork.timer == NULL)
    {
        radMsgLog (PRI_HIGH, "radTimerCreate failed - exiting");
        sshSysExit (&sshWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }

    //  ... initialize the ssh utilities
    retVal = sshUtilsInit (&sshWork.sshData);
    if (retVal != OK)
    {
        if (retVal == ERROR)
        {
            radMsgLog (PRI_HIGH, "sshUtilsInit failed - exiting");
        }
        radTimerDelete (sshWork.timer);
        sshSysExit (&sshWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }
    else
    {
        sshWork.sshId = &sshWork.sshData;
    }

    if (statusInit(sshWork.statusFile, sshStatusLabels) == ERROR)
    {
        radMsgLog (PRI_HIGH, "statusInit failed - exiting...");
        sshSysExit (&sshWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }
    statusUpdate(STATUS_BOOTING);
    statusUpdateStat(SSH_STATS_RULES_DEFINED, 
                     radListGetNumberOfNodes(&sshWork.sshData.rules));

    //  ... start THE timer
    ntime = time (NULL);
    localtime_r (&ntime, &locTime);
    seconds = locTime.tm_sec - 15;          // start at 15 secs past

    /*  ... start the ssh timer to go off 1 min past the next archive record
    */
    offset = locTime.tm_min % 5;
    if (offset)
        offset = 6 - offset;
    else
        offset = 1;

    if (seconds < -60)
    {
        offset += 1;
        offset += 60;
    }

    radMsgLog (PRI_HIGH,
               "SSH: starting updates in %d mins %d secs",
               ((seconds > 0) ? ((offset > 0) ? offset-1 : offset) : offset),
                       (seconds > 0) ? 60 - seconds : (-1) * seconds);

    msOffset = radTimeGetMSSinceEpoch () % 1000;
    msOffset -= 250;                        // land on 250 ms mark


    sshWork.msOffset = 0;
    radProcessTimerStart (sshWork.timer,
                          ((((offset * 60) - seconds) * 1000)) - msOffset);

    statusUpdate(STATUS_RUNNING);
    statusUpdateMessage("Normal operation");


    while (!sshWork.exiting)
    {
        /*  ... wait on timers, events, file descriptors, msgs, everything!
        */
        if (radProcessWait (0) == ERROR)
        {
            sshWork.exiting = TRUE;
        }
    }


    statusUpdateMessage("exiting normally");
    radMsgLog (PRI_STATUS, "exiting normally...");
    statusUpdate(STATUS_SHUTDOWN);

    if (sshWork.sshId != NULL)
    {
        sshUtilsExit (sshWork.sshId);
    }
    radTimerDelete (sshWork.timer);
    sshSysExit (&sshWork);
    radProcessExit ();
    radSystemExit (WVIEW_SYSTEM_ID);
    exit (0);
}

