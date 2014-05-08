/*---------------------------------------------------------------------------
 
  FILENAME:
        cwop.c
 
  PURPOSE:
        Provide the wview CWOP packet generator entry point.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        07/12/2005      M.S. Teel       0               Original
 
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

/*  ... Local include files
*/
#include <cwop.h>


/*  ... global memory declarations
*/

/*  ... global memory referenced
*/

/*  ... static (local) memory declarations
*/
static WVIEW_CWOP_WORK      cwopWork;

static char*                cwopStatusLabels[STATUS_STATS_MAX] =
{
    "Connection errors",
    "Packets sent",
    NULL,
    NULL
};

/* ... methods
*/

// From Gerry Creager: autogenerate passcode for non-CW/DW stations:
#define kKey            0x73e2                      // This is the seed for the key
static int16_t getPasscode (char *callSign)
{
    char    rootCall[16];
    char*   p1 = rootCall;
    int16_t hash;
    int16_t i, len;
    char*   ptr = rootCall;

    while ((*callSign != '-') && (*callSign != '\0')) 
    {
        *p1++ = toupper((int)(*callSign++));
    }

    *p1 = '\0';

    if (! strncmp (rootCall, "CW", 2) ||
        ! strncmp (rootCall, "DW", 2) ||
        ! strncmp (rootCall, "EW", 2) ||
        ! strncmp (rootCall, "FW", 2))
    {
        // Not a HAM, make sure it has been configured:
        if (! strncmp (&rootCall[2], "XXXX", 4))
        {
            // User has not configured a valid callsign:
            return -2;
        }
        else
        {
            // Return -1 for passcode:
            return -1;
        }
    }

    hash = kKey;                                    // Initialize with the key value
    i = 0;
    len = (int16_t)strlen (rootCall);

    while (i < len) 
    {
        // Loop through the string two bytes at a time
        hash ^= (unsigned char)(*ptr++) << 8;       // xor high byte with accumulated hash
        hash ^= (*ptr++);                           // xor low byte with accumulated hash
        i += 2;
    }

    return (hash & 0x7fff);                         // mask off the high bit so 
                                                    // result is always positive
}

static void processCWOP ()
{
    RADSOCK_ID                          socket;
    time_t                              ntime;
    struct tm                           *gmTime;
    char                                cwopBuffer[256], login[64];
    int                                 length = 0;
    char                                *serv;
    int                                 port;
    int                                 passCode;
    volatile WVIEW_MSG_ARCHIVE_NOTIFY   Notify;

    memcpy ((void*)&Notify, (void*)&cwopWork.ArchiveMsg, sizeof(WVIEW_MSG_ARCHIVE_NOTIFY));

    // Confirm the callsign is valid:
    passCode = (int)getPasscode(cwopWork.callSign);
    if (passCode == -2)
    {
        statusUpdateMessage("CWOP-error: callsign is not valid - aborting transfer.");
        radMsgLog (PRI_HIGH, "CWOP-error: callsign %s is not valid - aborting transfer.",
                   cwopWork.callSign);
        return;
    }

    // format the CWOP data
    ntime = time (NULL);
    gmTime = gmtime (&ntime);
    length = sprintf (cwopBuffer, "%s>APRS,TCPIP*:@", cwopWork.callSign);
    length += sprintf (&cwopBuffer[length], "%2.2d%2.2d%2.2dz",
                       gmTime->tm_mday, gmTime->tm_hour, gmTime->tm_min);
    length += sprintf (&cwopBuffer[length], "%s/%s",
                       cwopWork.latitude, cwopWork.longitude);

    // check for any wind registered
    if (Notify.wspeedF < 0)
    {
        length += sprintf (&cwopBuffer[length], "_...");
    }
    else
    {
        length += sprintf (&cwopBuffer[length], "_%3.3d", Notify.winddir);
    }
    
    length += sprintf (&cwopBuffer[length], "/%3.3d", (int)Notify.wspeedF);
    length += sprintf (&cwopBuffer[length], "g%3.3d", (int)Notify.hiwspeedF);

    if (Notify.temp < 0)
    {
        if (((Notify.temp * (-1)) % 10) >= 5)
        {
            Notify.temp -= 10;
        }

        length += sprintf (&cwopBuffer[length], "t-%2.2d", 
                           (Notify.temp * (-1))/10);
    }
    else
    {
        if ((Notify.temp % 10) >= 5)
        {
            Notify.temp += 10;
        }

        length += sprintf (&cwopBuffer[length], "t%3.3d", Notify.temp/10);
    }

    if (Notify.rainHour >= 0)
    {
        length += sprintf (&cwopBuffer[length], "r%3.3d", (int)(Notify.rainHour*100));
    }
    if (Notify.rainDay >= 0)
    {
        length += sprintf (&cwopBuffer[length], "p%3.3d", (int)(Notify.rainDay*100));
    }
    
    if (Notify.humidity >= 0 && Notify.humidity <= 100)
    {
        length += sprintf (&cwopBuffer[length], "h%2.2d", Notify.humidity % 100);
    }
    
    length += sprintf (&cwopBuffer[length], "b%5.5d", 
                       (int)(10 * wvutilsConvertINHGToHPA((float)Notify.altimeter/1000.0)));

    // If there is radiation present, send it:
    if (Notify.radiation <= 1800)
    {
        length += sprintf (&cwopBuffer[length], "L%3.3d",
                           ((Notify.radiation <= 999) ? (int)Notify.radiation : 999));
    }

    sprintf (&cwopBuffer[length], ".%s", wvutilsCreateCWOPVersion(globalWviewVersionStr));

    // connect to the CWOP server - try the primary then secondary then tertiary:
    socket = radSocketClientCreateAny(cwopWork.server1, cwopWork.portNo1);
    if (socket == NULL)
    {
        // try the secondary server
        socket = radSocketClientCreateAny(cwopWork.server2, cwopWork.portNo2);
        if (socket == NULL)
        {
            // try the tertiary server
            socket = radSocketClientCreateAny(cwopWork.server3, cwopWork.portNo3);
            if (socket == NULL)
            {
                // we are all out of luck this time!
                radMsgLog (PRI_HIGH, 
                           "CWOP-connect: failed to connect to all 3 APRS servers!");
                return;
            }
            else
            {
                serv = cwopWork.server3;
                port = cwopWork.portNo3;
            }
        }
        else
        {
            serv = cwopWork.server2;
            port = cwopWork.portNo2;
        }
    }
    else
    {
        serv = cwopWork.server1;
        port = cwopWork.portNo1;
    }

    // wait 1 second ...
    radUtilsSleep (1000);
    
    // transmit the data 
    sprintf (login, "user %6s pass %d vers %s", 
             cwopWork.callSign, passCode, globalWviewVersionStr);
    length = strlen (login);
    login[length] = 0x0D;           // tack on the CR and LF
    login[length+1] = 0x0A;
    login[length+2] = 0;
    
    if (radSocketWriteExact (socket, login, length + 2) != (length + 2))
    {
        statusUpdateMessage("CWOP-error: failed to login to server");
        radMsgLog (PRI_HIGH, "CWOP-error: %d: failed to login to %s:%d!",
                   errno, serv, port);
        radSocketDestroy (socket);
        return;
    }
    
    // wait 3 seconds ...
    radUtilsSleep (3000);
    
    // write the data record
    wvutilsLogEvent (PRI_STATUS, "CWOP-send: %s", cwopBuffer);

    length = strlen (cwopBuffer);
    cwopBuffer[length] = 0x0D;       // tack on the CR and LF
    cwopBuffer[length+1] = 0x0A;
    cwopBuffer[length+2] = 0;
    if (radSocketWriteExact (socket, cwopBuffer, length + 2) != (length + 2))
    {
        statusUpdateMessage("CWOP-error: failed to write to server");
        radMsgLog (PRI_HIGH, "CWOP-error: %d: failed to write to %s:%d!",
                   errno, serv, port);
        radSocketDestroy (socket);
        return;
    }

    statusIncrementStat(CWOP_STATS_PKTS_SENT);

    // wait 3 more seconds ...
    radUtilsSleep (3000);

    // close connection and cleanup
    radSocketDestroy (socket);

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

    // enable message reception from the radlib router for the archive path
    radMsgRouterMessageRegister (WVIEW_MSG_TYPE_STATION_INFO);

    msg.requestType = WVIEW_RQST_TYPE_STATION_INFO;

    if (radMsgRouterMessageSend (WVIEW_MSG_TYPE_REQUEST, &msg, sizeof(msg)) == ERROR)
    {
        statusUpdateMessage("waitForWviewDaemon: radMsgRouterMessageSend failed!");
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
            cwopWork.reportInterval = ((WVIEW_MSG_STATION_INFO*)recvBfr)->archiveInterval;
            if (cwopWork.reportInterval < 5)
            {
                cwopWork.reportInterval = 5;
            }
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
static int cwopSysInit (WVIEW_CWOP_WORK *work)
{
    char            devPath[256], temp[64];
    struct stat     fileData;

    /*  ... check for our daemon's pid file, don't run if it isn't there
    */
    sprintf (devPath, "%s/%s", WVIEW_RUN_DIR, WVD_LOCK_FILE_NAME);
    if (stat (devPath, &fileData) != 0)
    {
        radMsgLogInit (PROC_NAME_CWOP, TRUE, TRUE);
        radMsgLog (PRI_CATASTROPHIC, 
                   "wviewd process not running - aborting!");
        radMsgLogExit ();
        return ERROR;
    }

    sprintf (work->pidFile, "%s/%s", WVIEW_RUN_DIR, CWOP_LOCK_FILE_NAME);
    sprintf (work->statusFile, "%s/%s", WVIEW_STATUS_DIRECTORY, CWOP_STATUS_FILE_NAME);
    sprintf (work->fifoFile, "%s/dev/%s", WVIEW_RUN_DIR, PROC_NAME_CWOP);
    sprintf (work->daemonQname, "%s/dev/%s", WVIEW_RUN_DIR, PROC_NAME_DAEMON);
    sprintf (work->wviewdir, "%s", WVIEW_RUN_DIR);

    /*  ... check for our pid file, don't run if it IS there
    */
    if (stat (work->pidFile, &fileData) == 0)
    {
        radMsgLogInit (PROC_NAME_CWOP, TRUE, TRUE);
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
static int cwopSysExit (WVIEW_CWOP_WORK *work)
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
            radMsgLog (PRI_STATUS, "wvcwopd: SIGHUP - toggling log verbosity %s",
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
                       "wvcwopd: recv unrecoverable signal %d: aborting!",
                       signum);
            abort ();
        
        case SIGCHLD:
            wvutilsWaitForChildren();
            radProcessSignalCatch(signum, defaultSigHandler);
            return;

        default:
            if (cwopWork.exiting)
            {
                radProcessSignalCatch(signum, defaultSigHandler);
                return;
            }

            // Exit here in case the socket transaction is hung
            statusUpdateMessage("exiting normally");
            statusUpdate(STATUS_SHUTDOWN);
            radMsgLog (PRI_HIGH, "wvcwopd: recv sig %d: exiting now!", signum);
            radMsgRouterExit ();
            cwopSysExit (&cwopWork);
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

    switch (msgType)
    {
        case WVIEW_MSG_TYPE_ARCHIVE_NOTIFY:
        {
            anMsg = (WVIEW_MSG_ARCHIVE_NOTIFY *)msg;

            // Just store the most recent for later submission based on call sign
            memcpy ((void*)&cwopWork.ArchiveMsg, anMsg, sizeof(WVIEW_MSG_ARCHIVE_NOTIFY));
            cwopWork.rxFirstArchive = TRUE;
            cwopWork.rxArchive = TRUE;
            break;
        }
        case WVIEW_MSG_TYPE_POLL:
        {
            WVIEW_MSG_POLL*     pPoll = (WVIEW_MSG_POLL*)msg;
            wvutilsSendPMONPollResponse (pPoll->mask, PMON_PROCESS_WVCWOPD);
            break;
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

static void timerHandler (void *parm)
{
    time_t          ntime;
    struct tm       locTime;

    radProcessTimerStart (cwopWork.timer, CWOP_MINUTE_INTERVAL);

    ntime = time (NULL);
    localtime_r (&ntime, &locTime);

    if ((locTime.tm_min % cwopWork.reportInterval) == cwopWork.callSignOffset)
    {
        // Time to send a packet if we have any new data:
        if (cwopWork.rxArchive)
        {
            processCWOP ();
            cwopWork.rxArchive = FALSE;
        }
        else
        {
            if (cwopWork.rxFirstArchive)
            {
                statusUpdateMessage("no new archive data received since last CWOP submission");
                radMsgLog (PRI_MEDIUM, 
                    "wvcwopd: no new archive data received since last CWOP submission:");
            }
        }
    }

    return;
}

/*  ... the main entry point for the wvcwopd process
*/
int main (int argc, char *argv[])
{
    void            (*alarmHandler)(int);
    int             retVal;
    FILE            *pidfile;
    int             iValue;
    double          dValue;
    const char*     sValue;
    int             runAsDaemon = TRUE;

    if (argc > 1)
    {
        if (!strcmp(argv[1], "-f"))
        {
            runAsDaemon = FALSE;
        }
    }

    memset (&cwopWork, 0, sizeof (cwopWork));

    /*  ... initialize some system stuff first
    */
    retVal = cwopSysInit (&cwopWork);
    if (retVal == ERROR)
    {
        radMsgLogInit (PROC_NAME_CWOP, FALSE, TRUE);
        radMsgLog (PRI_CATASTROPHIC, "wvcwopd init failed!");
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
        radMsgLogInit (PROC_NAME_CWOP, TRUE, TRUE);
        radMsgLog (PRI_CATASTROPHIC, "radSystemInit failed!");
        radMsgLogExit ();
        exit (1);
    }


    /*  ... call the radlib process init function
    */
    if (radProcessInit (PROC_NAME_CWOP,
                        cwopWork.fifoFile,
                        PROC_NUM_TIMERS_CWOP,
                        runAsDaemon,                // TRUE for daemon
                        msgHandler,
                        evtHandler,
                        NULL)
        == ERROR)
    {
        printf ("\nradProcessInit failed: %s\n\n", PROC_NAME_CWOP);
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }

    cwopWork.myPid = getpid ();
    pidfile = fopen (cwopWork.pidFile, "w");
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


    //  get our configuration values
    if (wvconfigInit(FALSE) == ERROR)
    {
        radMsgLog (PRI_CATASTROPHIC, "wvconfigInit failed!");
        cwopSysExit (&cwopWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }

    // Is the CWOP daemon enabled?
    iValue = wvconfigGetBooleanValue(configItem_ENABLE_CWOP);
    if (iValue == ERROR || iValue == 0)
    {
        wvconfigExit ();
        radMsgLog (PRI_CATASTROPHIC, "CWOP daemon is NOT enabled - exiting...");
        cwopSysExit (&cwopWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }

    // get the wview verbosity setting
    if (wvutilsSetVerbosity (WV_VERBOSE_WVCWOPD) == ERROR)
    {
        wvconfigExit ();
        radMsgLog (PRI_CATASTROPHIC, "wvutilsSetVerbosity failed!");
        cwopSysExit (&cwopWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }

    // get the APRS call sign
    sValue = wvconfigGetStringValue(configItemCWOP_APRS_CALL_SIGN);
    if (sValue == NULL)
    {
        // we can't do without this!
        wvconfigExit ();
        radMsgLog (PRI_CATASTROPHIC, "%s failed!", configItemCWOP_APRS_CALL_SIGN);
        cwopSysExit (&cwopWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }
    else
    {
        wvstrncpy (cwopWork.callSign, sValue, sizeof(cwopWork.callSign));
        // Trim trailing blanks:
        wvstrtrim (cwopWork.callSign);

        if (sValue[strlen(sValue)-1] >= '0' && sValue[strlen(sValue)-1] <= '9')
        {
            cwopWork.callSignOffset = atoi(&sValue[strlen(sValue)-1]);
        }
        else
        {
            cwopWork.callSignOffset = (sValue[strlen(sValue)-1] % 5);
        }
        cwopWork.callSignOffset %= 5;
    }

    // get the primary APRS server
    sValue = wvconfigGetStringValue(configItemCWOP_APRS_SERVER1);
    if (sValue == NULL)
    {
        // we can't do without this!
        wvconfigExit ();
        radMsgLog (PRI_CATASTROPHIC, "%s failed!", configItemCWOP_APRS_SERVER1);
        cwopSysExit (&cwopWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }
    else
    {
        wvstrncpy (cwopWork.server1, sValue, sizeof(cwopWork.server1));
    }

    // get the primary APRS port number
    iValue = wvconfigGetINTValue(configItemCWOP_APRS_PORTNO1);
    if (iValue < 0)
    {
        // we can't do without this!
        wvconfigExit ();
        radMsgLog (PRI_CATASTROPHIC, "%s failed!", configItemCWOP_APRS_PORTNO1);
        cwopSysExit (&cwopWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }
    else
    {
        cwopWork.portNo1 = iValue;
    }

    // get the secondary APRS server
    sValue = wvconfigGetStringValue(configItemCWOP_APRS_SERVER2);
    if (sValue == NULL)
    {
        // we can't do without this!
        wvconfigExit ();
        radMsgLog (PRI_CATASTROPHIC, "%s failed!", configItemCWOP_APRS_SERVER2);
        cwopSysExit (&cwopWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }
    else
    {
        wvstrncpy (cwopWork.server2, sValue, sizeof(cwopWork.server2));
    }

    // get the primary APRS port number
    iValue = wvconfigGetINTValue(configItemCWOP_APRS_PORTNO2);
    if (iValue < 0)
    {
        // we can't do without this!
        wvconfigExit ();
        radMsgLog (PRI_CATASTROPHIC, "%s failed!", configItemCWOP_APRS_PORTNO2);
        cwopSysExit (&cwopWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }
    else
    {
        cwopWork.portNo2 = iValue;
    }

    // get the tertiary APRS server
    sValue = wvconfigGetStringValue(configItemCWOP_APRS_SERVER3);
    if (sValue == NULL)
    {
        // we can't do without this!
        wvconfigExit ();
        radMsgLog (PRI_CATASTROPHIC, "%s failed!", configItemCWOP_APRS_SERVER3);
        cwopSysExit (&cwopWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }
    else
    {
        wvstrncpy (cwopWork.server3, sValue, sizeof(cwopWork.server3));
    }

    // get the primary APRS port number
    iValue = wvconfigGetINTValue(configItemCWOP_APRS_PORTNO3);
    if (iValue < 0)
    {
        // we can't do without this!
        wvconfigExit ();
        radMsgLog (PRI_CATASTROPHIC, "%s failed!", configItemCWOP_APRS_PORTNO3);
        cwopSysExit (&cwopWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }
    else
    {
        cwopWork.portNo3 = iValue;
    }

    // get the fine pitch latitude that APRS requires
    sValue = wvconfigGetStringValue(configItemCWOP_LATITUDE);
    if (sValue == NULL)
    {
        // we can't do without this!
        wvconfigExit ();
        radMsgLog (PRI_CATASTROPHIC, "%s failed!", configItemCWOP_LATITUDE);
        cwopSysExit (&cwopWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }
    else
    {
        wvstrncpy (cwopWork.latitude, sValue, sizeof(cwopWork.latitude));
    }

    // get the fine pitch longitude that APRS requires
    sValue = wvconfigGetStringValue(configItemCWOP_LONGITUDE);
    if (sValue == NULL)
    {
        // we can't do without this!
        wvconfigExit ();
        radMsgLog (PRI_CATASTROPHIC, "%s failed!", configItemCWOP_LONGITUDE);
        cwopSysExit (&cwopWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }
    else
    {
        wvstrncpy (cwopWork.longitude, sValue, sizeof(cwopWork.longitude));
    }

    // get the WX packet logging preference
    iValue = wvconfigGetBooleanValue(configItemCWOP_LOG_WX_PACKET);
    if (iValue <= 0)
    {
        // just disable it
        cwopWork.logWXPackets = 0;
    }
    else
    {
        cwopWork.logWXPackets = iValue;
    }

    wvconfigExit ();


    if (statusInit(cwopWork.statusFile, cwopStatusLabels) == ERROR)
    {
        radMsgLog (PRI_HIGH, "statusInit failed - exiting...");
        cwopSysExit (&cwopWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }
    statusUpdate(STATUS_BOOTING);


    // wait a bit here before continuing
    radUtilsSleep (500);


    cwopWork.timer = radTimerCreate (NULL, timerHandler, NULL);
    if (cwopWork.timer == NULL)
    {
        statusUpdateMessage("radTimerCreate failed");
        radMsgLog (PRI_HIGH, "radTimerCreate failed");
        statusUpdate(STATUS_ERROR);
        cwopSysExit (&cwopWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }
    radProcessTimerStart (cwopWork.timer, CWOP_MINUTE_INTERVAL);


    //  register with the radlib message router
    if (radMsgRouterInit (WVIEW_RUN_DIR) == ERROR)
    {
        statusUpdateMessage("radMsgRouterInit failed");
        radMsgLog (PRI_HIGH, "radMsgRouterInit failed!");
        statusUpdate(STATUS_ERROR);
        cwopSysExit (&cwopWork);
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
        cwopSysExit (&cwopWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }

    // enable message reception from the radlib router for archive notifications
    radMsgRouterMessageRegister (WVIEW_MSG_TYPE_ARCHIVE_NOTIFY);

    // enable message reception from the radlib router for POLL msgs
    radMsgRouterMessageRegister (WVIEW_MSG_TYPE_POLL);


    radMsgLog (PRI_STATUS, "CWOP: configured to submit station %s data to:",
               cwopWork.callSign);
    radMsgLog (PRI_STATUS, "CWOP: Primary:   %s:%d", cwopWork.server1, cwopWork.portNo1);
    radMsgLog (PRI_STATUS, "CWOP: Secondary: %s:%d", cwopWork.server2, cwopWork.portNo2);
    radMsgLog (PRI_STATUS, "CWOP: Tertiary:  %s:%d", cwopWork.server3, cwopWork.portNo3);
    radMsgLog (PRI_STATUS, "CWOP: Submitting every %d minutes at offset minute: %d",
               cwopWork.reportInterval, cwopWork.callSignOffset);


    // enter normal processing
    cwopWork.inMainLoop = TRUE;
    statusUpdate(STATUS_RUNNING);
    statusUpdateMessage("Normal operation");
    radMsgLog (PRI_STATUS, "running...");


    while (!cwopWork.exiting)
    {
        // wait on something interesting
        if (radProcessWait (0) == ERROR)
        {
            cwopWork.exiting = TRUE;
        }
    }


    statusUpdateMessage("exiting normally");
    radMsgLog (PRI_STATUS, "exiting normally...");
    statusUpdate(STATUS_SHUTDOWN);

    radMsgRouterExit ();
    radTimerDelete (cwopWork.timer);
    cwopSysExit (&cwopWork);
    radProcessExit ();
    radSystemExit (WVIEW_SYSTEM_ID);
    exit (0);
}

