/*---------------------------------------------------------------------------
 
  FILENAME:
        http.c
 
  PURPOSE:
        Provide the wview http packet generator entry point.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        07/12/2005      M.S. Teel       0               
        12/09/2007      M.S. Teel       1               Add WeatherForYou.com
 
  NOTES:
        
 
  LICENSE:
        Copyright (c) 2005, Mark S. Teel (mark@teel.ws)
  
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
#include <radprocess.h>
#include <radmsgRouter.h>

/*  ... Local include files
*/
#include <http.h>


/*  ... global memory declarations
*/

/*  ... global memory referenced
*/

/*  ... static (local) memory declarations
*/
static WVIEW_HTTPD_WORK         httpWork;

static char                     httpBuffer[1024];

static char*                    httpStatusLabels[STATUS_STATS_MAX] =
{
    "Connection errors",
    "Packets sent",
    NULL,
    NULL
};

/* ... methods
*/
// curl recv callback
static size_t recv_data (void *buffer, size_t size, size_t nmemb, void *userp)
{
    // we don't care about the recvd data
    return size*nmemb;
}

static void processWUNDERGROUND (WVIEW_MSG_ARCHIVE_NOTIFY *notify)
{
    RADSOCK_ID          socket;
    time_t              ntime;
    struct tm           gmTime;
    int                 length = 0;
    char                *serv;
    int                 port;
    char                version[64];
    CURL                *curl;
    CURLcode            res;
    char                curlError[CURL_ERROR_SIZE];
    char                tempBfr[194];
    
    // format the WUNDERGROUND data
    ntime = time (NULL);
    gmtime_r (&ntime, &gmTime);
    length = sprintf (httpBuffer, "http://weatherstation.wunderground.com/weatherstation/updateweatherstation.php?");
    length += sprintf (&httpBuffer[length], "ID=%s&PASSWORD=%s",
                       httpWork.stationId, httpWork.password);

    length += sprintf (&httpBuffer[length], "&dateutc=%4.4d-%2.2d-%2.2d+%2.2d:%2.2d:%2.2d",
                       gmTime.tm_year + 1900, gmTime.tm_mon + 1, gmTime.tm_mday, 
                       gmTime.tm_hour, gmTime.tm_min, gmTime.tm_sec);

    // check for any wind registered
    if (notify->wspeedF >= 0 || notify->hiwspeedF >= 0)
    {
        length += sprintf (&httpBuffer[length], "&winddir=%3.3d", notify->winddir);
    }
    
    length += sprintf (&httpBuffer[length], "&windspeedmph=%3.1f",
                       (notify->wspeedF >= 0) ? notify->wspeedF : 0);
    length += sprintf (&httpBuffer[length], "&windgustmph=%3.1f",
                       (notify->hiwspeedF >= 0) ? notify->hiwspeedF : 0);

    if (notify->humidity >= 0 && notify->humidity <= 100)
    {
        length += sprintf (&httpBuffer[length], "&humidity=%d", notify->humidity);
    }

    length += sprintf (&httpBuffer[length], "&tempf=%3.3d.%1.1d", 
                       notify->temp/10, notify->temp%10);
    
    length += sprintf (&httpBuffer[length], "&rainin=%.2f", notify->rainHour);
    
    length += sprintf (&httpBuffer[length], "&dailyrainin=%.2f", notify->rainToday);

    length += sprintf (&httpBuffer[length], "&baromin=%2.2d.%2.2d",
                       notify->barom/1000, (notify->barom%1000)/10);

    length += sprintf (&httpBuffer[length], "&dewptf=%2.2d.%3.3d", 
                       notify->dewpoint/10, (notify->dewpoint%10)*100);

    if (notify->radiation != 0xFFFF)
        length += sprintf (&httpBuffer[length], "&solarradiation=%d", (int)notify->radiation);

    if (notify->UV >= 0 && notify->UV < 20)
        length += sprintf (&httpBuffer[length], "&UV=%.1f", notify->UV);

    strcpy (version, globalWviewVersionStr);
    version[5] = '-';
    length += sprintf (&httpBuffer[length], "&weather=&clouds=&softwaretype=%s&action=updateraw",
                       version);


    wvstrncpy (tempBfr, httpBuffer, 192);
    wvutilsLogEvent (PRI_STATUS, "WUNDERGROUND-send: %s", tempBfr);
    if (strlen(httpBuffer) > 191)
    {
        wvstrncpy (tempBfr, &httpBuffer[192], 192);
        wvutilsLogEvent (PRI_STATUS, "WUNDERGROUND-send: %s", tempBfr);
    }


    // OK, now use libcurl to execute the HTTP "GET"
    curl = curl_easy_init ();
    if (curl == NULL)
    {
        radMsgLog (PRI_HIGH, "WUNDERGROUND-error: failed to initialize curl!");
        statusUpdateMessage("HTTP-WU: failed to initialize curl");
        statusIncrementStat(HTTP_STATS_CONNECT_ERRORS);
        return;
    }

    curl_easy_setopt (curl, CURLOPT_URL, httpBuffer);
    curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, recv_data);
    curl_easy_setopt (curl, CURLOPT_ERRORBUFFER, curlError);
    curl_easy_setopt (curl, CURLOPT_TIMEOUT, 60);

    res = curl_easy_perform (curl);
    if (res != 0)
    {
        statusUpdateMessage("HTTP-WU: failed curl_easy_perform");
        statusIncrementStat(HTTP_STATS_CONNECT_ERRORS);
        radMsgLog (PRI_HIGH, "WUNDERGROUND-error: %s", curlError);
    }
    
    curl_easy_cleanup (curl);
    statusIncrementStat(HTTP_STATS_PKTS_SENT);

    return;
}

static void processWEATHERFORYOU (WVIEW_MSG_ARCHIVE_NOTIFY *notify)
{
    RADSOCK_ID          socket;
    time_t              ntime;
    struct tm           gmTime;
    int                 length = 0;
    char                *serv;
    int                 port;
    char                version[64];
    CURL                *curl;
    CURLcode            res;
    char                curlError[CURL_ERROR_SIZE];
    char                tempBfr[194];
    
    // format the WEATHERFORYOU data
    ntime = time (NULL);
    gmtime_r (&ntime, &gmTime);
    length = sprintf (httpBuffer, "http://www.pwsweather.com/pwsupdate/pwsupdate.php?");
    length += sprintf (&httpBuffer[length], "ID=%s&PASSWORD=%s",
                       httpWork.youstationId, httpWork.youpassword);

    length += sprintf (&httpBuffer[length], "&dateutc=%4.4d-%2.2d-%2.2d+%2.2d:%2.2d:%2.2d",
                       gmTime.tm_year + 1900, gmTime.tm_mon + 1, gmTime.tm_mday, 
                       gmTime.tm_hour, gmTime.tm_min, gmTime.tm_sec);

    // check for any wind registered
    if (notify->wspeedF >= 0 || notify->hiwspeedF >= 0)
    {
        length += sprintf (&httpBuffer[length], "&winddir=%3.3d", notify->winddir);
    }
    
    length += sprintf (&httpBuffer[length], "&windspeedmph=%.1f",
                       (notify->wspeedF >= 0) ? notify->wspeedF : 0);
    length += sprintf (&httpBuffer[length], "&windgustmph=%.1f",
                       (notify->hiwspeedF >= 0) ? notify->hiwspeedF : 0);

    length += sprintf (&httpBuffer[length], "&tempf=%d.%d", 
                       notify->temp/10, notify->temp%10);
    
    length += sprintf (&httpBuffer[length], "&rainin=%.2f", notify->rainHour);
    
    length += sprintf (&httpBuffer[length], "&dailyrainin=%.2f", notify->rainToday);

    length += sprintf (&httpBuffer[length], "&baromin=%2.2d.%2.2d",
                       notify->barom/1000, (notify->barom%1000)/10);

    length += sprintf (&httpBuffer[length], "&dewptf=%d.%d", 
                       notify->dewpoint/10, notify->dewpoint%10);

    if (notify->humidity >= 0 && notify->humidity <= 100)
    {
        length += sprintf (&httpBuffer[length], "&humidity=%d", notify->humidity);
    }

    if (notify->radiation != 0xFFFF)
        length += sprintf (&httpBuffer[length], "&solarradiation=%d", (int)notify->radiation);

    if (notify->UV >= 0 && notify->UV < 20)
        length += sprintf (&httpBuffer[length], "&UV=%.1f", notify->UV);

    strcpy (version, globalWviewVersionStr);
    version[5] = '-';
    length += sprintf (&httpBuffer[length], "&weather=&softwaretype=%s&action=updateraw",
                       version);


    wvstrncpy (tempBfr, httpBuffer, 192);
    wvutilsLogEvent (PRI_STATUS, "WEATHERFORYOU-send: %s", tempBfr);
    if (strlen(httpBuffer) > 192)
    {
        wvstrncpy (tempBfr, &httpBuffer[192], 192);
        wvutilsLogEvent (PRI_STATUS, "WEATHERFORYOU-send: %s", tempBfr);
    }


    // OK, now use libcurl to execute the HTTP "GET"
    curl = curl_easy_init ();
    if (curl == NULL)
    {
        radMsgLog (PRI_HIGH, "WEATHERFORYOU-error: failed to initialize curl!");
        statusUpdateMessage("HTTP-WFY: failed to initialize curl");
        statusIncrementStat(HTTP_STATS_CONNECT_ERRORS);
        return;
    }
    
    curl_easy_setopt (curl, CURLOPT_URL, httpBuffer);
    curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, recv_data);
    curl_easy_setopt (curl, CURLOPT_ERRORBUFFER, curlError);
    curl_easy_setopt (curl, CURLOPT_TIMEOUT, 60);

    res = curl_easy_perform (curl);
    if (res != 0)
    {
        radMsgLog (PRI_HIGH, "WEATHERFORYOU-error: %s", curlError);
        statusUpdateMessage("HTTP-WFY: failed curl_easy_perform");
        statusIncrementStat(HTTP_STATS_CONNECT_ERRORS);
    }
    
    curl_easy_cleanup (curl);
    statusIncrementStat(HTTP_STATS_PKTS_SENT);

    return;
}

static int waitForWviewDaemon (void)
{
    WVIEW_MSG_REQUEST       msg;
    char                    srcQName[QUEUE_NAME_LENGTH+1];
    UINT                    msgType;
    UINT                    length;
    void                    *recvBfr;
    int                     retVal, done = FALSE;
    WVIEW_MSG_STATION_INFO  *apMsg;

    // enable message reception from the radlib router for the archive path
    radMsgRouterMessageRegister (WVIEW_MSG_TYPE_STATION_INFO);

    msg.requestType = WVIEW_RQST_TYPE_STATION_INFO;

    if (radMsgRouterMessageSend (WVIEW_MSG_TYPE_REQUEST, &msg, sizeof(msg)) == ERROR)
    {
        radMsgLog (PRI_HIGH, "waitForWviewDaemon: radMsgRouterMessageSend failed!");
        radMsgRouterMessageDeregister (WVIEW_MSG_TYPE_STATION_INFO);
        return ERROR;
    }

    statusUpdate(STATUS_WAITING_FOR_WVIEW);

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
            statusUpdateMessage("waitForWviewDaemon: queue is closed!");
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
            statusUpdateMessage("waitForWviewDaemon: received shutdown from wviewd");
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

/*  ... system initialization
*/
static int httpSysInit (WVIEW_HTTPD_WORK *work)
{
    char            devPath[256], temp[64];
    struct stat     fileData;

    /*  ... check for our daemon's pid file, don't run if it isn't there
    */
    sprintf (devPath, "%s/%s", WVIEW_RUN_DIR, WVD_LOCK_FILE_NAME);
    if (stat (devPath, &fileData) != 0)
    {
        radMsgLogInit (PROC_NAME_HTTP, TRUE, TRUE);
        radMsgLog (PRI_CATASTROPHIC, 
                   "wviewd process no running - aborting!");
        radMsgLogExit ();
        return ERROR;
    }

    sprintf (work->pidFile, "%s/%s", WVIEW_RUN_DIR, HTTP_LOCK_FILE_NAME);
    sprintf (work->fifoFile, "%s/dev/%s", WVIEW_RUN_DIR, PROC_NAME_HTTP);
    sprintf (work->statusFile, "%s/%s", WVIEW_STATUS_DIRECTORY, HTTP_STATUS_FILE_NAME);
    sprintf (work->daemonQname, "%s/dev/%s", WVIEW_RUN_DIR, PROC_NAME_DAEMON);
    sprintf (work->wviewdir, "%s", WVIEW_RUN_DIR);

    /*  ... check for our pid file, don't run if it IS there
    */
    if (stat (work->pidFile, &fileData) == 0)
    {
        radMsgLogInit (PROC_NAME_HTTP, TRUE, TRUE);
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
static int httpSysExit (WVIEW_HTTPD_WORK *work)
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

static void defaultSigHandler (int signum)
{
    int         retVal;

    switch (signum)
    {
        case SIGHUP:
            // user wants us to change the verbosity setting
            retVal = wvutilsToggleVerbosity ();
            radMsgLog (PRI_STATUS, "wvhttpd: SIGHUP - toggling log verbosity %s",
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
            // unrecoverable - we must exit right now!
            radMsgLog (PRI_CATASTROPHIC, 
                       "wvhttpd: recv unrecoverable signal %d: aborting!",
                       signum);
            abort ();
        
        case SIGCHLD:
            wvutilsWaitForChildren();
            radProcessSignalCatch(signum, defaultSigHandler);
            return;

        default:
            if (httpWork.exiting)
            {
                radProcessSignalCatch(signum, defaultSigHandler);
                return;
            }

            // We exit now, we are probably hung on a "curl_easy_perform" call!
            radMsgLog (PRI_HIGH, "wvhttpd: recv signal %d: exiting now!", signum);
            statusUpdateMessage("exiting normally");
            statusUpdate(STATUS_SHUTDOWN);
            radMsgRouterExit ();
            httpSysExit (&httpWork);
            radProcessExit ();
            radSystemExit (WVIEW_SYSTEM_ID);
            exit (0);                
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
    WVIEW_MSG_ARCHIVE_NOTIFY    *anMsg;
    time_t                      nowTime = time (NULL);
    
    if (msgType == WVIEW_MSG_TYPE_ARCHIVE_NOTIFY)
    {
        anMsg = (WVIEW_MSG_ARCHIVE_NOTIFY *)msg;

        // process http data transfer
        if (strcmp (httpWork.stationId, "0"))
        {
            processWUNDERGROUND (anMsg);
        }
        if (strcmp (httpWork.youstationId, "0"))
        {
            processWEATHERFORYOU (anMsg);
        }
    }
    else if (msgType == WVIEW_MSG_TYPE_POLL)
    {
        WVIEW_MSG_POLL*     pPoll = (WVIEW_MSG_POLL*)msg;
        wvutilsSendPMONPollResponse (pPoll->mask, PMON_PROCESS_WVHTTPD);
        return;
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

static void timerHandler (void *parm)
{
    return;
}

/*  ... the main entry point for the wvhttpd process
*/
int main (int argc, char *argv[])
{
    void            (*alarmHandler)(int);
    int             retVal;
    int             iValue;
    const char*     sValue;
    FILE            *pidfile;
    struct stat     fileStatus;
    time_t          timeStamp, nowTime = time (NULL) - WV_SECONDS_IN_HOUR;
    ARCHIVE_PKT     recordStore;
    float           tempRain;
    int             runAsDaemon = TRUE;

    if (argc > 1)
    {
        if (!strcmp(argv[1], "-f"))
        {
            runAsDaemon = FALSE;
        }
    }

    memset (&httpWork, 0, sizeof (httpWork));

    /*  ... initialize some system stuff first
    */
    retVal = httpSysInit (&httpWork);
    if (retVal == ERROR)
    {
        radMsgLogInit (PROC_NAME_HTTP, FALSE, TRUE);
        radMsgLog (PRI_CATASTROPHIC, "wvhttpd init failed!\n");
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
        radMsgLogInit (PROC_NAME_HTTP, TRUE, TRUE);
        radMsgLog (PRI_CATASTROPHIC, "radSystemInit failed!");
        radMsgLogExit ();
        exit (1);
    }


    /*  ... call the radlib process init function
    */
    if (radProcessInit (PROC_NAME_HTTP,
                        httpWork.fifoFile,
                        PROC_NUM_TIMERS_HTTP,
                        runAsDaemon,                // TRUE for daemon
                        msgHandler,
                        evtHandler,
                        NULL)
        == ERROR)
    {
        printf ("\nradProcessInit failed: %s\n\n", PROC_NAME_HTTP);
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }

    httpWork.myPid = getpid ();
    pidfile = fopen (httpWork.pidFile, "w");
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


    /*  ... get our configuration values
    */
    if (wvconfigInit(FALSE) == ERROR)
    {
        radMsgLog (PRI_CATASTROPHIC, "wvconfigInit failed!");
        httpSysExit (&httpWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }

    // Is the HTTP daemon enabled?
    iValue = wvconfigGetBooleanValue(configItem_ENABLE_HTTP);
    if (iValue == ERROR || iValue == 0)
    {
        wvconfigExit ();
        radMsgLog (PRI_CATASTROPHIC, "HTTP daemon is disabled - exiting...");
        httpSysExit (&httpWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }

    // get the wview verbosity setting
    if (wvutilsSetVerbosity (WV_VERBOSE_WVWUNDERD) == ERROR)
    {
        wvconfigExit ();
        radMsgLog (PRI_CATASTROPHIC, "wvutilsSetVerbosity failed!");
        httpSysExit (&httpWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }

    // get the STATION ID
    sValue = wvconfigGetStringValue(configItemHTTP_WUSTATIONID);
    if (sValue == NULL)
    {
        radMsgLog (PRI_CATASTROPHIC, "%s not found: WUNDERGROUND set to disabled", configItemHTTP_WUSTATIONID);
        strcpy (httpWork.stationId, "0");
    }
    else
    {
        if (strlen(sValue) > 0)
        {
            wvstrncpy (httpWork.stationId, sValue, sizeof(httpWork.stationId));
            strcpy (httpWork.password, "0");
        }
        else
        {
            radMsgLog (PRI_MEDIUM, "WUNDERGROUND station ID not given - disabling...");
            strcpy (httpWork.stationId, "0");
            strcpy (httpWork.password, "0");
        }
    }

    // get the password
    sValue = wvconfigGetStringValue(configItemHTTP_WUPASSWD);
    if (sValue == NULL)
    {
        // we can't do without this!
        radMsgLog (PRI_CATASTROPHIC, "%s not found: WUNDERGROUND set to disabled", configItemHTTP_WUPASSWD);
        strcpy (httpWork.stationId, "0");
        strcpy (httpWork.password, "0");
    }
    else
    {
        if (strlen(sValue) > 0)
        {
            wvstrncpy (httpWork.password, sValue, sizeof(httpWork.password));
        }
        else
        {
            radMsgLog (PRI_MEDIUM, "WUNDERGROUND passwd not given - disabling...");
            strcpy (httpWork.stationId, "0");
            strcpy (httpWork.password, "0");
        }
    }

    // get the YOU STATION ID
    sValue = wvconfigGetStringValue(configItemHTTP_YOUSTATIONID);
    if (sValue == NULL)
    {
        // default to disabled
        radMsgLog (PRI_CATASTROPHIC, "%s not found: WEATHERFORYOU set to disabled", configItemHTTP_YOUSTATIONID);
        strcpy (httpWork.youstationId, "0");
    }
    else
    {
        if (strlen(sValue) > 0)
        {
            wvstrncpy (httpWork.youstationId, sValue, sizeof(httpWork.youstationId));
            strcpy (httpWork.youpassword, "0");
        }
        else
        {
            radMsgLog (PRI_MEDIUM, "WEATHERFORYOU station ID not given - disabling...");
            strcpy (httpWork.youstationId, "0");
            strcpy (httpWork.youpassword, "0");
        }
    }

    // get the YOU password
    sValue = wvconfigGetStringValue(configItemHTTP_YOUPASSWD);
    if (sValue == NULL)
    {
        // we can't do without this!
        radMsgLog (PRI_CATASTROPHIC, "%s not found: WEATHERFORYOU set to disabled", configItemHTTP_YOUPASSWD);
        strcpy (httpWork.youstationId, "0");
        strcpy (httpWork.youpassword, "0");        
    }
    else
    {
        if (strlen(sValue) > 0)
        {
            wvstrncpy (httpWork.youpassword, sValue, sizeof(httpWork.youpassword));
        }
        else
        {
            radMsgLog (PRI_MEDIUM, "WEATHERFORYOU passwd not given - disabling...");
            strcpy (httpWork.youstationId, "0");
            strcpy (httpWork.youpassword, "0");
        }
    }

    wvconfigExit();

    if (strcmp (httpWork.stationId, "0"))
    {
        radMsgLog (PRI_STATUS, "WUNDERGROUND: configured to submit station %s data to wunderground.com",
                   httpWork.stationId);
    }

    if (strcmp (httpWork.youstationId, "0"))
    {
        radMsgLog (PRI_STATUS, "WEATHERFORYOU: configured to submit station %s data to weatherforyou.com",
                   httpWork.youstationId);
    }

    if (statusInit(httpWork.statusFile, httpStatusLabels) == ERROR)
    {
        radMsgLog (PRI_HIGH, "CWOP status init failed - exiting...");
        httpSysExit (&httpWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }
    statusUpdate(STATUS_BOOTING);


    // wait a bit here before continuing
    radUtilsSleep (500);


    //  register with the radlib message router
    if (radMsgRouterInit (WVIEW_RUN_DIR) == ERROR)
    {
        statusUpdateMessage("radMsgRouterInit failed");
        statusUpdate(STATUS_ERROR);
        radMsgLog (PRI_HIGH, "radMsgRouterInit failed!");
        httpSysExit (&httpWork);
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
        httpSysExit (&httpWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }


    // ... Initialize the archive database interface:
    if (dbsqliteArchiveInit() == ERROR)
    {
        statusUpdateMessage("dbsqliteArchiveInit failed");
        statusUpdate(STATUS_ERROR);
        radMsgLog (PRI_HIGH, "dbsqliteArchiveInit failed");
        httpSysExit (&httpWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }

    // enable message reception from the radlib router for archive notifications
    radMsgRouterMessageRegister (WVIEW_MSG_TYPE_ARCHIVE_NOTIFY);

    // enable message reception from the radlib router for POLL msgs
    radMsgRouterMessageRegister (WVIEW_MSG_TYPE_POLL);


    // enter normal processing
    httpWork.inMainLoop = TRUE;
    statusUpdate(STATUS_RUNNING);
    statusUpdateMessage("Normal operation");
    radMsgLog (PRI_STATUS, "running...");


    while (!httpWork.exiting)
    {
        // wait on something interesting
        if (radProcessWait (0) == ERROR)
        {
            httpWork.exiting = TRUE;
        }
    }


    statusUpdateMessage("exiting normally");
    radMsgLog (PRI_STATUS, "exiting normally...");
    statusUpdate(STATUS_SHUTDOWN);

    radMsgRouterExit ();
    dbsqliteArchiveExit();
    httpSysExit (&httpWork);
    radProcessExit ();
    radSystemExit (WVIEW_SYSTEM_ID);
    exit (0);
}

