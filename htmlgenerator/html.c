/*---------------------------------------------------------------------------
 
  FILENAME:
         html.c
 
  PURPOSE:
        Provide the wview html generator entry point.
 
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
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/*  ... Library include files
*/
#include <radsystem.h>

/*  ... Local include files
*/
#include <wvconfig.h>
#include <html.h>


/*  ... global memory declarations
*/

/*  ... global memory referenced
*/

/*  ... static (local) memory declarations
*/
static HTML_WORK        htmlWork;

static char*                    htmlStatusLabels[STATUS_STATS_MAX] =
{
    "Images defined",
    "Templates defined",
    "Images generated",
    "Templates generated"
};

/* ... methods
*/
/*  ... system initialization
*/
static int htmlSysInit (HTML_WORK *work)
{
    char            devPath[256];
    struct stat     fileData;

    /*  ... check for our daemon's pid file, don't run if it isn't there
    */
    sprintf (devPath, "%s/%s", WVIEW_RUN_DIR, WVD_LOCK_FILE_NAME);
    if (stat (devPath, &fileData) != 0)
    {
        radMsgLogInit (PROC_NAME_HTML, FALSE, TRUE);
        radMsgLog (PRI_CATASTROPHIC, "wviewd process not running - aborting!");
        radMsgLogExit ();
        return ERROR;
    }

    sprintf (work->pidFile, "%s/%s", WVIEW_RUN_DIR, HTML_LOCK_FILE_NAME);
    sprintf (work->indicateFile, "%s/%s", WVIEW_RUN_DIR, HTML_INDICATE_FILE_NAME);
    sprintf (work->fifoFile, "%s/dev/%s", WVIEW_RUN_DIR, PROC_NAME_HTML);
    sprintf (work->statusFile, "%s/%s", WVIEW_STATUS_DIRECTORY, HTML_STATUS_FILE_NAME);
    sprintf (work->daemonQname, "%s/dev/%s", WVIEW_RUN_DIR, PROC_NAME_DAEMON);

    
    /*  ... check for our html template directory and bail if it is not there
    */
    sprintf (devPath, "%s/html", WVIEW_CONFIG_DIR);
    if (stat (devPath, &fileData) != 0)
    {
        radMsgLogInit (PROC_NAME_HTML, FALSE, TRUE);
        radMsgLog (PRI_CATASTROPHIC, "HTML template directory %s is missing...aborting!",
                   devPath);
        radMsgLogExit ();
        return ERROR;
    }

    /*  ... check for our pid file, don't run if it is there
    */
    if (stat (work->pidFile, &fileData) == 0)
    {
        radMsgLogInit (PROC_NAME_HTML, FALSE, TRUE);
        radMsgLog (PRI_CATASTROPHIC, "lock file %s exists, older copy may be running - aborting!",
                   work->pidFile);
        radMsgLogExit ();
        return ERROR;
    }

    /*  ... check for our indicator file, delete it if it is there
    */
    if (stat (work->indicateFile, &fileData) == 0)
    {
        unlink(work->indicateFile);
    }

    /*  ... create our device directory if it is not there
    */
    sprintf (devPath, "%s/dev", WVIEW_RUN_DIR);
    if (stat (devPath, &fileData) != 0)
    {
        if (mkdir (devPath, 0755) != 0)
        {
            radMsgLogInit (PROC_NAME_HTML, FALSE, TRUE);
            radMsgLog (PRI_CATASTROPHIC, "Cannot create device dir: %s...",
                       devPath);
            radMsgLogExit ();
            return ERROR;
        }
    }

    /*  ... create our image directory if it is not there
    */
    sprintf (devPath, "%s/img", WVIEW_RUN_DIR);
    if (stat (devPath, &fileData) != 0)
    {
        if (mkdir (devPath, 0755) != 0)
        {
            radMsgLogInit (PROC_NAME_HTML, FALSE, TRUE);
            radMsgLog (PRI_CATASTROPHIC, "Cannot create image dir: %s...",
                       devPath);
            radMsgLogExit ();
            return ERROR;
        }
    }

    return 0;
}

/*  ... system exit
*/
static int htmlSysExit (HTML_WORK *work)
{
    struct stat     fileData;

    /*  ... delete our pid file
    */
    if (stat (work->pidFile, &fileData) == 0)
    {
        unlink (work->pidFile);
    }
    if (stat (work->indicateFile, &fileData) == 0)
    {
        unlink (work->indicateFile);
    }

    return 0;
}

static void defaultSigHandler (int signum)
{
    int         retVal;

    switch (signum)
    {
        case SIGHUP:
            // user wants us to reload some config files:
            retVal = wvutilsToggleVerbosity ();
            radMsgLog (PRI_STATUS, "htmlgend: SIGHUP - toggling log verbosity %s",
                       ((retVal == 0) ? "OFF" : "ON"));
            radMsgLog (PRI_STATUS, "htmlgend: SIGHUP - "
                       "re-reading image/html config files");
            if (htmlWork.mgrId != NULL)
            {
                htmlmgrReReadImageFiles (htmlWork.mgrId, WVIEW_CONFIG_DIR);
            }
            radProcessSignalCatch(signum, defaultSigHandler);
            break;

        case SIGBUS:
        case SIGFPE:
        case SIGSEGV:
        case SIGXFSZ:
        case SIGSYS:
            // unrecoverable radProcessSignalCatch- we must exit right now!
            radMsgLog (PRI_CATASTROPHIC, 
                       "htmlgend: recv unrecoverable signal %d: aborting!",
                       signum);
            abort ();
        
        case SIGCHLD:
            wvutilsWaitForChildren();
            radProcessSignalCatch(signum, defaultSigHandler);
            break;

        default:
            // we can allow the process to exit normally:
            if (htmlWork.exiting)
            {
                radProcessSignalCatch(signum, defaultSigHandler);
                return;
            }
        
            radMsgLog (PRI_CATASTROPHIC, "htmlgend: recv signal %d: exiting!", signum);
        
            htmlWork.exiting = TRUE;
            radProcessSetExitFlag ();
        
            radProcessSignalCatch(signum, defaultSigHandler);
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
    STIM        stim;

    if (msgType == WVIEW_MSG_TYPE_POLL)
    {
        WVIEW_MSG_POLL*     pPoll = (WVIEW_MSG_POLL*)msg;
        wvutilsSendPMONPollResponse(pPoll->mask, PMON_PROCESS_HTMLGEND);
        return;
    }
    else if (msgType == WVIEW_MSG_TYPE_SHUTDOWN)
    {
        radMsgLog (PRI_HIGH, "htmlgend: received shutdown from wviewd"); 
        htmlWork.exiting = TRUE;
        return;
    }

    stim.type           = STIM_QMSG;
    stim.srcQueueName   = srcQueueName;
    stim.msgType        = msgType;
    stim.msg            = msg;
    stim.length         = length;

    radStatesProcess(htmlWork.stateMachine, &stim);

    return;
}

static void evtHandler
(
    UINT        eventsRx,
    UINT        rxData,
    void        *userData
)
{
    STIM        stim;

    stim.type           = STIM_EVENT;
    stim.eventsRx       = eventsRx;
    stim.eventData      = rxData;

    radStatesProcess(htmlWork.stateMachine, &stim);

    return;
}

static void timerHandler (void *parm)
{
    STIM            stim;
    struct timeval  timenow;
    uint32_t        netOffset;

    // get current time
    gettimeofday(&timenow, NULL);
    
    //  bump our expected next generation time
    htmlWork.nextGenerationTime.tv_sec += (htmlWork.timerInterval / 1000L);
    
    // compute the next timer period
    netOffset = (htmlWork.nextGenerationTime.tv_sec - timenow.tv_sec) * 1000L;
    netOffset += ((htmlWork.nextGenerationTime.tv_usec - timenow.tv_usec) / 1000L);

    radProcessTimerStart(htmlWork.timer, netOffset);

    memset(&stim, 0, sizeof (stim));

    stim.type           = STIM_TIMER;
    stim.timerNumber    = TIMER_GENERATE;

    radStatesProcess(htmlWork.stateMachine, &stim);

    return;
}

static void rxtimerHandler (void *parm)
{
    STIM            stim;

    memset(&stim, 0, sizeof (stim));

    stim.type           = STIM_TIMER;
    stim.timerNumber    = TIMER_RX_PACKETS;

    radStatesProcess(htmlWork.stateMachine, &stim);

    return;
}

static void noaaTimerHandler (void *parm)
{
    noaaGenerate(htmlWork.noaaId, time(NULL));
    return;
}


/*  ... the main entry point for the html process
*/
int main (int argc, char *argv[])
{
    void            (*alarmHandler)(int);
    STIM            stim;
    int             i, verbose;
    int             iValue;
    double          dValue;
    const char*     sValue;
    char            dateFmt[WVIEW_STRING1_SIZE];
    FILE            *pidfile;
    int             isMetricUnits;
    int             runAsDaemon = TRUE;

    if (argc > 1)
    {
        if (!strcmp(argv[1], "-f"))
        {
            runAsDaemon = FALSE;
        }
    }

    memset (&htmlWork, 0, sizeof (htmlWork));

    /*  ... initialize some system stuff first
    */
    if (htmlSysInit (&htmlWork) == -1)
    {
        radMsgLogInit (PROC_NAME_HTML, TRUE, TRUE);
        radMsgLog (PRI_CATASTROPHIC, "system init failed!");
        radMsgLogExit ();
        exit (1);
    }


    /*  ... call the global radlib system init function
    */
    if (radSystemInit (WVIEW_SYSTEM_ID) == ERROR)
    {
        radMsgLogInit (PROC_NAME_HTML, TRUE, TRUE);
        radMsgLog (PRI_CATASTROPHIC, "radSystemInit failed!");
        radMsgLogExit ();
        exit (1);
    }


    /*  ... call the radlib process init function
    */
    if (radProcessInit (PROC_NAME_HTML,
                        htmlWork.fifoFile,
                        PROC_NUM_TIMERS_HTML,
                        runAsDaemon,                // TRUE for daemon
                        msgHandler,
                        evtHandler,
                        NULL)
        == ERROR)
    {
        printf ("radProcessInit failed: %s", PROC_NAME_HTML);
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }

    htmlWork.myPid = getpid ();
    pidfile = fopen (htmlWork.pidFile, "w");
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
    radProcessSignalCatchAll (defaultSigHandler);
    radProcessSignalCatch (SIGALRM, alarmHandler);
    radProcessSignalRelease(SIGABRT);


    //  ... get our configuration values
    if (wvconfigInit(FALSE) == ERROR)
    {
        radMsgLog (PRI_CATASTROPHIC, "wvconfigInit failed!");
        htmlSysExit (&htmlWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }

    // Is the htmlgend daemon enabled?
    iValue = wvconfigGetBooleanValue(configItem_ENABLE_HTMLGEN);
    if (iValue == ERROR || iValue == 0)
    {
        wvconfigExit ();
        radMsgLog (PRI_CATASTROPHIC, "htmlgend daemon is NOT enabled - exiting...");
        htmlSysExit (&htmlWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }

    // get the wview verbosity setting
    if (wvutilsSetVerbosity (WV_VERBOSE_HTMLGEND) == ERROR)
    {
        wvconfigExit ();
        radMsgLog (PRI_CATASTROPHIC, "wvutilsSetVerbosity failed!");
        htmlSysExit (&htmlWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }
    
    
    // get the metric units flag first
    iValue = wvconfigGetBooleanValue(configItem_HTMLGEN_METRIC_UNITS);
    if (iValue == ERROR)
    {
        htmlWork.isMetricUnits = 0;
    }
    else
    {
        htmlWork.isMetricUnits = iValue;
    }

    if (htmlWork.isMetricUnits)
    {
        radMsgLog (PRI_STATUS, "!! configured for metric units/conversion !!");

        // try to get the CM/MM rain setting here too...
        iValue = wvconfigGetBooleanValue(configItem_HTMLGEN_METRIC_USE_RAIN_MM);
        if (iValue == ERROR)
        {
            // just assume MM units
            wvutilsSetRainIsMM(TRUE);
        }
        else
        {
            wvutilsSetRainIsMM(iValue);
        }
        if (wvutilsGetRainIsMM())
            radMsgLog (PRI_STATUS, "!! Rain units will be mm !!");
        else
            radMsgLog (PRI_STATUS, "!! Rain units will be cm !!");
    }

    // get the archive browser files-to-keep value
    iValue = wvconfigGetINTValue(configItem_HTMLGEN_ARCHIVE_BROWSER_FILES_TO_KEEP);
    htmlWork.arcrecDaysToKeep = iValue;

    sValue = wvconfigGetStringValue(configItem_HTMLGEN_MPHASE_INCREASE);
    if (sValue == NULL)
    {
        wvstrncpy (htmlWork.mphaseIncrease, "Waxing", sizeof(htmlWork.mphaseIncrease));
    }
    else
    {
        wvstrncpy (htmlWork.mphaseIncrease, sValue, sizeof(htmlWork.mphaseIncrease));
    }

    sValue = wvconfigGetStringValue(configItem_HTMLGEN_MPHASE_DECREASE);
    if (sValue == NULL)
    {
        wvstrncpy (htmlWork.mphaseDecrease, "Waning", sizeof(htmlWork.mphaseDecrease));
    }
    else
    {
        wvstrncpy (htmlWork.mphaseDecrease, sValue, sizeof(htmlWork.mphaseDecrease));
    }

    sValue = wvconfigGetStringValue(configItem_HTMLGEN_MPHASE_FULL);
    if (sValue == NULL)
    {
        wvstrncpy (htmlWork.mphaseFull, "Full", sizeof(htmlWork.mphaseFull));
    }
    else
    {
        wvstrncpy (htmlWork.mphaseFull, sValue, sizeof(htmlWork.mphaseFull));
    }

    sValue = wvconfigGetStringValue(configItem_HTMLGEN_LOCAL_RADAR_URL);
    if (sValue == NULL)
    {
        radMsgLog (PRI_STATUS, "Set your default radar image - using default");
        wvstrncpy (htmlWork.radarURL, 
                 "http://www.srh.noaa.gov/radar/images/DS.p19r0/SI.kfws/latest.gif",
                 sizeof(htmlWork.radarURL));
    }
    else
    {
        wvstrncpy (htmlWork.radarURL, sValue, sizeof(htmlWork.radarURL));
    }

    sValue = wvconfigGetStringValue(configItem_HTMLGEN_LOCAL_FORECAST_URL);
    if (sValue == NULL)
    {
        radMsgLog (PRI_STATUS, "Set your default forecast URL - using default");
        wvstrncpy (htmlWork.forecastURL, 
                 "http://www.wunderground.com/cgi-bin/findweather/getForecast?query=76233",
                 sizeof(htmlWork.forecastURL));
    }
    else
    {
        wvstrncpy (htmlWork.forecastURL, sValue, sizeof(htmlWork.forecastURL));
    }

    sValue = wvconfigGetStringValue(configItem_HTMLGEN_STATION_NAME);
    if (sValue == NULL)
    {
        radMsgLog (PRI_STATUS, "Set your station name - using 'WVIEW_STATION'");
        wvstrncpy (htmlWork.stationName, "WVIEW_STATION", sizeof(htmlWork.stationName));
    }
    else
    {
        wvstrncpy (htmlWork.stationName, sValue, sizeof(htmlWork.stationName));
    }

    sValue = wvconfigGetStringValue(configItem_HTMLGEN_STATION_CITY);
    if (sValue == NULL)
    {
        radMsgLog (PRI_STATUS, "Set your station city - using 'WVIEW_CITY'");
        wvstrncpy (htmlWork.stationCity, "WVIEW_CITY", sizeof(htmlWork.stationCity));
    }
    else
    {
        wvstrncpy (htmlWork.stationCity, sValue, sizeof(htmlWork.stationCity));
    }

    sValue = wvconfigGetStringValue(configItem_HTMLGEN_STATION_STATE);
    if (sValue == NULL)
    {
        radMsgLog (PRI_STATUS, "Set your station state - using 'WVIEW_STATE'");
        wvstrncpy (htmlWork.stationState, "WVIEW_STATE", sizeof(htmlWork.stationState));
    }
    else
    {
        wvstrncpy (htmlWork.stationState, sValue, sizeof(htmlWork.stationState));
    }

    sValue = wvconfigGetStringValue(configItem_HTMLGEN_IMAGE_PATH);
    if (sValue == NULL)
    {
        radMsgLog (PRI_CATASTROPHIC, "HTMLGEN_IMAGE_PATH not found!");
        wvconfigExit ();
        htmlSysExit (&htmlWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }
    else
    {
        wvstrncpy (htmlWork.imagePath, sValue, sizeof(htmlWork.imagePath));
        radMsgLog (PRI_STATUS, "generating to %s", htmlWork.imagePath);
    }

    sValue = wvconfigGetStringValue(configItem_HTMLGEN_HTML_PATH);
    if (sValue == NULL)
    {
        radMsgLog (PRI_CATASTROPHIC, "HTMLGEN_HTML_PATH not found!");
        wvconfigExit ();
        htmlSysExit (&htmlWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }
    else
    {
        wvstrncpy (htmlWork.htmlPath, sValue, sizeof(htmlWork.htmlPath));
        radMsgLog (PRI_STATUS, "templates at %s", htmlWork.htmlPath);
    }

    iValue = wvconfigGetINTValue(configItem_HTMLGEN_START_OFFSET);
    if (iValue < 0)
    {
        radMsgLog (PRI_CATASTROPHIC, "HTMLGEN_START_OFFSET not found - defaulting to 0");
        htmlWork.startOffset = 0;
    }
    else
    {
        htmlWork.startOffset = iValue;
    }

    iValue = wvconfigGetINTValue(configItem_HTMLGEN_GENERATE_INTERVAL);
    if (iValue <= 0)
    {
        radMsgLog (PRI_CATASTROPHIC, "HTMLGEN_GENERATE_INTERVAL not found - defaulting to 1");
        htmlWork.timerInterval = 60000;
    }
    else
    {
        htmlWork.timerInterval = 60000 * iValue;
    }

    iValue = wvconfigGetBooleanValue(configItem_HTMLGEN_EXTENDED_DATA);
    if (iValue >= 0)
    {
        htmlWork.isExtendedData = iValue;
    }

    sValue = wvconfigGetStringValue(configItem_HTMLGEN_DATE_FORMAT);
    if (sValue == NULL)
    {
        if (htmlWork.isMetricUnits)
        {
            wvstrncpy (htmlWork.dateFormat, "%Y%m%d %R", sizeof(htmlWork.dateFormat));
        }
        else
        {
            wvstrncpy (htmlWork.dateFormat, "%m/%d/%Y %R", sizeof(htmlWork.dateFormat));
        }
    }
    else
    {
        wvstrncpy (dateFmt, sValue, WVIEW_STRING1_SIZE);
        for (i = 0; i < WVIEW_STRING1_SIZE; i ++)
        {
            if (dateFmt[i] == 0)
            {
                break;
            }
            if (dateFmt[i] == '_')
            {
                dateFmt[i] = ' ';
            }
        }
        wvstrncpy (htmlWork.dateFormat, dateFmt, WVIEW_STRING1_SIZE);
    }

    // Are we displaying dual units?..
    iValue = wvconfigGetBooleanValue(configItem_HTMLGEN_DUAL_UNITS);
    if (iValue >= 0)
    {
        htmlWork.isDualUnits = iValue;
        if (htmlWork.isDualUnits)
            radMsgLog (PRI_STATUS, "!! Dual units will be displayed !!");
    }
    else
    {
        // just assume single units
        htmlWork.isDualUnits = 0;
    }

    // Get the default wind units:
    sValue = wvconfigGetStringValue(configItem_HTMLGEN_WIND_UNITS);
    if (sValue == NULL)
    {
        radMsgLog (PRI_MEDIUM, "HTMLGEN_WIND_UNITS not found, defaulting to MPH...");
        htmlWork.windUnits = HTML_WINDUNITS_MPH;
        wvutilsSetWindUnits(HTML_WINDUNITS_MPH);
    }
    else if (!strncmp(sValue, "mph", 3))
    {
        // mph
        htmlWork.windUnits = HTML_WINDUNITS_MPH;
        wvutilsSetWindUnits(HTML_WINDUNITS_MPH);
        radMsgLog (PRI_STATUS, "Using mph as default wind units");
    }
    else if (!strncmp(sValue, "m/s", 3))
    {
        // m/s
        htmlWork.windUnits = HTML_WINDUNITS_MS;
        wvutilsSetWindUnits(HTML_WINDUNITS_MS);
        radMsgLog (PRI_STATUS, "Using m/s as default wind units");
    }
    else if (!strncmp(sValue, "knots", 5))
    {
        // knots
        htmlWork.windUnits = HTML_WINDUNITS_KNOTS;
        wvutilsSetWindUnits(HTML_WINDUNITS_KNOTS);
        radMsgLog (PRI_STATUS, "Using knots as default wind units");
    }
    else if (!strncmp(sValue, "km/h", 4))
    {
        // km/h
        htmlWork.windUnits = HTML_WINDUNITS_KMH;
        wvutilsSetWindUnits(HTML_WINDUNITS_KMH);
        radMsgLog (PRI_STATUS, "Using km/h as default wind units");
    }
    else
    {
        // default to mph:
        htmlWork.windUnits = HTML_WINDUNITS_MPH;
        wvutilsSetWindUnits(HTML_WINDUNITS_MPH);
        radMsgLog (PRI_STATUS, "DB value not recognized: using mph as default wind units");
    }

    wvconfigExit ();

    if (statusInit(htmlWork.statusFile, htmlStatusLabels) == ERROR)
    {
        radMsgLog (PRI_HIGH, "statusInit failed");
        htmlSysExit (&htmlWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }
    statusUpdate(STATUS_BOOTING);


    // ... Initialize the generator:
    htmlGenerateInit ();

    // ... Initialize the state machine:
    htmlWork.stateMachine = radStatesInit (&htmlWork);
    if (htmlWork.stateMachine == NULL)
    {
        statusUpdateMessage("radStatesInit failed");
        statusUpdate(STATUS_ERROR);
        radMsgLog (PRI_HIGH, "radStatesInit failed");
        htmlSysExit (&htmlWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }

    if (radStatesAddHandler (htmlWork.stateMachine, HTML_STATE_IDLE,
                             htmlIdleState)
            == ERROR)
    {
        radMsgLog (PRI_HIGH, "radStatesAddHandler failed");
        statusUpdateMessage("radStatesAddHandler failed");
        statusUpdate(STATUS_ERROR);
        radStatesExit (htmlWork.stateMachine);
        htmlSysExit (&htmlWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }
    if (radStatesAddHandler (htmlWork.stateMachine, HTML_STATE_STATION_INFO,
                             htmlStationInfoState)
            == ERROR)
    {
        radMsgLog (PRI_HIGH, "radStatesAddHandler failed");
        statusUpdateMessage("radStatesAddHandler failed");
        statusUpdate(STATUS_ERROR);
        radStatesExit (htmlWork.stateMachine);
        htmlSysExit (&htmlWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }
    if (radStatesAddHandler (htmlWork.stateMachine, HTML_STATE_RUN,
                             htmlRunState)
            == ERROR)
    {
        radMsgLog (PRI_HIGH, "radStatesAddHandler failed");
        statusUpdateMessage("radStatesAddHandler failed");
        statusUpdate(STATUS_ERROR);
        radStatesExit (htmlWork.stateMachine);
        htmlSysExit (&htmlWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }
    if (radStatesAddHandler (htmlWork.stateMachine, HTML_STATE_DATA,
                             htmlDataState)
            == ERROR)
    {
        radMsgLog (PRI_HIGH, "radStatesAddHandler failed");
        statusUpdateMessage("radStatesAddHandler failed");
        statusUpdate(STATUS_ERROR);
        radStatesExit (htmlWork.stateMachine);
        htmlSysExit (&htmlWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }
    if (radStatesAddHandler (htmlWork.stateMachine, HTML_STATE_ERROR,
                             htmlErrorState)
            == ERROR)
    {
        radMsgLog (PRI_HIGH, "radStatesAddHandler failed");
        statusUpdateMessage("radStatesAddHandler failed");
        statusUpdate(STATUS_ERROR);
        radStatesExit (htmlWork.stateMachine);
        htmlSysExit (&htmlWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }

    radStatesSetState (htmlWork.stateMachine, HTML_STATE_IDLE);

    
    htmlWork.timer = radTimerCreate (NULL, timerHandler, NULL);
    if (htmlWork.timer == NULL)
    {
        radMsgLog (PRI_HIGH, "radTimerCreate failed");
        statusUpdateMessage("radTimerCreate failed");
        statusUpdate(STATUS_ERROR);
        radStatesExit (htmlWork.stateMachine);
        htmlSysExit (&htmlWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }

    htmlWork.rxTimer = radTimerCreate (NULL, rxtimerHandler, NULL);
    if (htmlWork.rxTimer == NULL)
    {
        radMsgLog (PRI_HIGH, "radTimerCreate 2 failed");
        statusUpdateMessage("radTimerCreate 2 failed");
        statusUpdate(STATUS_ERROR);
        radTimerDelete (htmlWork.timer);
        radStatesExit (htmlWork.stateMachine);
        htmlSysExit (&htmlWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }

    htmlWork.noaaTimer = radTimerCreate (NULL, noaaTimerHandler, NULL);
    if (htmlWork.noaaTimer == NULL)
    {
        radMsgLog (PRI_HIGH, "radTimerCreate 3 failed");
        statusUpdateMessage("radTimerCreate 3 failed");
        statusUpdate(STATUS_ERROR);
        radTimerDelete (htmlWork.rxTimer);
        radTimerDelete (htmlWork.timer);
        radStatesExit (htmlWork.stateMachine);
        htmlSysExit (&htmlWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }

    // wait a bit here before continuing
    radUtilsSleep (500);

    //  register with the radlib message router
    if (radMsgRouterInit (WVIEW_RUN_DIR) == ERROR)
    {
        radMsgLog (PRI_HIGH, "radMsgRouterInit failed!");
        statusUpdateMessage("radMsgRouterInit failed");
        statusUpdate(STATUS_ERROR);
        radTimerDelete (htmlWork.noaaTimer);
        radTimerDelete (htmlWork.rxTimer);
        radTimerDelete (htmlWork.timer);
        radStatesExit (htmlWork.stateMachine);
        htmlSysExit (&htmlWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }

    // enable message reception from the radlib router for worker requests
    radMsgRouterMessageRegister (WVIEW_MSG_TYPE_STATION_INFO);
    radMsgRouterMessageRegister (WVIEW_MSG_TYPE_HILOW_DATA);
    radMsgRouterMessageRegister (WVIEW_MSG_TYPE_LOOP_DATA);
    radMsgRouterMessageRegister (WVIEW_MSG_TYPE_ARCHIVE_NOTIFY);

    // enable message reception from the radlib router for POLL msgs
    radMsgRouterMessageRegister (WVIEW_MSG_TYPE_POLL);

    // enable message reception from the radlib router for SHUTDOWN msgs
    radMsgRouterMessageRegister (WVIEW_MSG_TYPE_SHUTDOWN);


    radMsgLog (PRI_STATUS, "running...");


    // dummy up a stimulus to get the state machine running
    stim.type = STIM_DUMMY;
    radStatesProcess (htmlWork.stateMachine, &stim);


    while (! htmlWork.exiting)
    {
        //  wait on timers, events, file descriptors, msgs
        if (radProcessWait(0) == ERROR)
        {
            htmlWork.exiting = TRUE;
        }
    }


    statusUpdateMessage("exiting normally");
    radMsgLog (PRI_STATUS, "exiting normally...");
    statusUpdate(STATUS_SHUTDOWN);

    radMsgRouterExit ();
    radTimerDelete (htmlWork.noaaTimer);
    radTimerDelete (htmlWork.rxTimer);
    radTimerDelete (htmlWork.timer);
    radStatesExit (htmlWork.stateMachine);
    
    if (htmlWork.mgrId != NULL)
        htmlmgrExit (htmlWork.mgrId);

    dbsqliteHiLowExit();
    dbsqliteNOAAExit();
    dbsqliteArchiveExit();
    htmlSysExit (&htmlWork);

    radProcessExit ();
    radSystemExit (WVIEW_SYSTEM_ID);
    exit (0);
}

