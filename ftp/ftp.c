/*---------------------------------------------------------------------------
 
  FILENAME:
        ftp.c
 
  PURPOSE:
        Provide the wview ftp generator entry point.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        01/10/04        M.S. Teel       0               Original
 
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
#include <ftp.h>


/*  ... global memory declarations
*/

/*  ... global memory referenced
*/

/*  ... static (local) memory declarations
*/
static FTP_WORK         ftpWork;

static char*            ftpStatusLabels[STATUS_STATS_MAX] =
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
static int ftpSysInit (FTP_WORK *work)
{
    char            devPath[256], temp[64];
    struct stat     fileData;

    /*  ... check for our daemon's pid file, don't run if it isn't there
    */
    sprintf (devPath, "%s/%s", WVIEW_RUN_DIR, WVD_LOCK_FILE_NAME);
    if (stat (devPath, &fileData) != 0)
    {
        radMsgLogInit (PROC_NAME_FTP, TRUE, TRUE);
        radMsgLog (PRI_CATASTROPHIC, 
                   "wview daemon lock file %s does not exist - aborting!",
                   devPath);
        radMsgLogExit ();
        return ERROR;
    }

    sprintf (work->pidFile, "%s/%s", WVIEW_RUN_DIR, FTP_LOCK_FILE_NAME);
    sprintf (work->fifoFile, "%s/dev/%s", WVIEW_RUN_DIR, PROC_NAME_FTP);
    sprintf (work->statusFile, "%s/%s", WVIEW_STATUS_DIRECTORY, FTP_STATUS_FILE_NAME);
    sprintf (work->daemonQname, "%s/dev/%s", WVIEW_RUN_DIR, PROC_NAME_DAEMON);
    sprintf (work->wviewdir, "%s/img", WVIEW_RUN_DIR);

    /*  ... check for our pid file, don't run if it IS there
    */
    if (stat (work->pidFile, &fileData) == 0)
    {
        radMsgLogInit (PROC_NAME_FTP, TRUE, TRUE);
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
static int ftpSysExit (FTP_WORK *work)
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


void FTPDefaultSigHandler (int signum)
{
    int         retVal;

    switch (signum)
    {
        case SIGHUP:
            // user wants us to change the verbosity setting
            retVal = wvutilsToggleVerbosity ();
            radMsgLog (PRI_STATUS, "wviewftpd: SIGHUP - toggling log verbosity %s",
                       ((retVal == 0) ? "OFF" : "ON"));

            radProcessSignalCatch(signum, FTPDefaultSigHandler);
            return;

        case SIGBUS:
        case SIGFPE:
        case SIGSEGV:
        case SIGXFSZ:
        case SIGSYS:
            // unrecoverable radProcessSignalCatch- we must exit right now!
            radMsgLog (PRI_CATASTROPHIC, 
                       "wviewftpd: recv unrecoverable signal %d: aborting!",
                       signum);
            abort ();
        
        case SIGCHLD:
            // it is normal behavior to have children finishing up
            wvutilsWaitForChildren();
            radProcessSignalCatch(signum, FTPDefaultSigHandler);
            break;

        default:
            // we can allow the process to exit normally...
            if (ftpWork.exiting)
            {
                radProcessSignalCatch(signum, FTPDefaultSigHandler);
                return;
            }
        
            radMsgLog (PRI_CATASTROPHIC, "wviewftpd: recv signal %d: exiting!", signum);
        
            ftpWork.exiting = TRUE;
            radProcessSetExitFlag ();
        
            radProcessSignalCatch(signum, FTPDefaultSigHandler);
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
    int         rules;
    int64_t     netOffset;

    //  ... allow for timer latency
    if (ftpWork.msOffset == 0)
    {
        // first time through
        ftpWork.msOffset = msOffset;
        netOffset = 0;
    }
    else
    {
        netOffset = msOffset - ftpWork.msOffset;
        while (netOffset >= 60000LL)
        {
            netOffset -= 60000LL;
        }
    }

    while (ftpWork.msOffset < msOffset)
    {
        ftpWork.msOffset += 60000ULL;
    }

    radProcessTimerStart(ftpWork.timer, (uint32_t)(60000LL - netOffset));


    //  ... process the ftp rules
    rules = ftpUtilsSendFiles(ftpWork.ftpId, ftpWork.wviewdir);

    if (rules > 0)
    {
        wvutilsLogEvent (PRI_STATUS, "FTP: %d rules processed", rules);
    }
    else if (rules < 0)
    {
        wvutilsLogEvent (PRI_MEDIUM, "FTP: ftpUtilsSendFiles failed!");
    }

    return;
}


/*  ... the main entry point for the ftp process
*/
int main (int argc, char *argv[])
{
    void            (*alarmHandler)(int);
    STIM            stim;
    int             i;
    int             seconds;
    time_t          ntime;
    struct tm       locTime;
    long            offset, retVal;
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

    memset (&ftpWork, 0, sizeof (ftpWork));

    /*  ... initialize some system stuff first
    */
    retVal = ftpSysInit (&ftpWork);
    if (retVal == ERROR)
    {
        radMsgLogInit (PROC_NAME_FTP, FALSE, TRUE);
        radMsgLog (PRI_CATASTROPHIC, "ftp init failed!");
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
        radMsgLogInit (PROC_NAME_FTP, TRUE, TRUE);
        radMsgLog (PRI_CATASTROPHIC, "radSystemInit failed!");
        radMsgLogExit ();
        exit (1);
    }


    /*  ... call the radlib process init function
    */
    if (radProcessInit (PROC_NAME_FTP,
                        ftpWork.fifoFile,
                        PROC_NUM_TIMERS_FTP,
                        runAsDaemon,                // TRUE for daemon
                        msgHandler,
                        evtHandler,
                        NULL)
        == ERROR)
    {
        printf ("radProcessInit failed: %s", PROC_NAME_FTP);
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }

    ftpWork.myPid = getpid ();
    pidfile = fopen (ftpWork.pidFile, "w");
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
    radProcessSignalCatchAll (FTPDefaultSigHandler);
    radProcessSignalCatch (SIGALRM, alarmHandler);
    radProcessSignalRelease(SIGABRT);


    ftpWork.timer = radTimerCreate (NULL, timerHandler, NULL);
    if (ftpWork.timer == NULL)
    {
        radMsgLog (PRI_HIGH, "radTimerCreate failed");
        ftpSysExit (&ftpWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }

    //  ... initialize the ftp utilities
    retVal = ftpUtilsInit (&ftpWork.ftpData);

    if (retVal != OK)
    {
        if (retVal == ERROR)
        {
            radMsgLog (PRI_HIGH, "ftpUtilsInit failed");
        }
        radTimerDelete (ftpWork.timer);
        ftpSysExit (&ftpWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }
    else
    {
        ftpWork.ftpId = &ftpWork.ftpData;
    }


    if (statusInit(ftpWork.statusFile, ftpStatusLabels) == ERROR)
    {
        radMsgLog (PRI_HIGH, "ALARM status init failed - exiting...");
        ftpSysExit (&ftpWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }
    statusUpdate(STATUS_BOOTING);
    statusUpdateStat(FTP_STATS_RULES_DEFINED, 
                     radListGetNumberOfNodes(&ftpWork.ftpData.rules));

    //  ... start THE timer
    ntime = time (NULL);
    localtime_r (&ntime, &locTime);
    seconds = locTime.tm_sec - 15;         // start at 15 secs past

    /*  ... start the ftp timer to go off 1 min past the next 5 minute mark
    */
    offset = locTime.tm_min % 5;
    if (offset)
        offset = 6 - offset;
    else
        offset = 1;

    radMsgLog (PRI_HIGH,
               "starting ftp timer for %d mins %d secs",
               ((seconds > 0) ? ((offset > 0) ? offset-1 : offset) : offset),
                       (seconds > 0) ? 60 - seconds : (-1) * seconds);

    msOffset = radTimeGetMSSinceEpoch () % 1000;
    msOffset -= 250;                        // land on 250 ms mark


    ftpWork.msOffset = 0;
    radProcessTimerStart (ftpWork.timer,
                          ((((offset * 60) - seconds) * 1000)) - msOffset);

    statusUpdate(STATUS_RUNNING);
    statusUpdateMessage("Normal operation");


    while (! ftpWork.exiting)
    {
        /*  ... wait on timers, events, file descriptors, msgs, everything!
        */
        if (radProcessWait (0) == ERROR)
        {
            ftpWork.exiting = TRUE;
        }
    }


    statusUpdateMessage("exiting normally");
    radMsgLog (PRI_STATUS, "exiting normally...");
    statusUpdate(STATUS_SHUTDOWN);

    if (ftpWork.ftpId != NULL)
    {
        ftpUtilsExit (ftpWork.ftpId);
    }

    radTimerDelete (ftpWork.timer);
    ftpSysExit (&ftpWork);
    radProcessExit ();
    radSystemExit (WVIEW_SYSTEM_ID);
    exit (0);
}

