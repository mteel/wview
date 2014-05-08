/*---------------------------------------------------------------------------

  FILENAME:
        daemon.c

  PURPOSE:
        Provide the wview daemon entry point.

  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        08/14/03        M.S. Teel       0               Original
        08/04/2008      M.S. Teel       1               Change config to 
                                                        wvconfig.h

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
#include <signal.h>

/*  ... Library include files
*/
#include <radsystem.h>

/*  ... Local include files
*/
#include <daemon.h>
#include <station.h>
#include <computedData.h>
#include <stormRain.h>

/*  ... global memory declarations
*/

/*  ... global memory referenced
*/

/*  ... static (local) memory declarations
*/
static WVIEWD_WORK      wviewdWork;

static char*            wviewStatusLabels[STATUS_STATS_MAX] =
{
    "LOOP packets received",
    "Archive packets generated",
    "",
    ""
};


/* ... methods
*/

static int daemonStationLoopComplete (void)
{
    float           tempf, sampleRain, sampleET;

    if (!wviewdWork.runningFlag)
    {
        return OK;
    }

    // Adjust for calibrations:
    // DO NOT calibrate pressure here:

    wviewdWork.loopPkt.inTemp           *= wviewdWork.calMInTemp;
    wviewdWork.loopPkt.inTemp           += wviewdWork.calCInTemp;

    wviewdWork.loopPkt.outTemp          *= wviewdWork.calMOutTemp;
    wviewdWork.loopPkt.outTemp          += wviewdWork.calCOutTemp;

    wviewdWork.loopPkt.inHumidity       *= wviewdWork.calMInHumidity;
    wviewdWork.loopPkt.inHumidity       += wviewdWork.calCInHumidity;
    if (wviewdWork.loopPkt.inHumidity > 100)
    {
        wviewdWork.loopPkt.inHumidity = 100;
    }

    wviewdWork.loopPkt.outHumidity      *= wviewdWork.calMOutHumidity;
    wviewdWork.loopPkt.outHumidity      += wviewdWork.calCOutHumidity;
    if (wviewdWork.loopPkt.outHumidity > 100)
    {
        wviewdWork.loopPkt.outHumidity = 100;
    }

    wviewdWork.loopPkt.windSpeedF       *= wviewdWork.calMWindSpeed;
    wviewdWork.loopPkt.windSpeedF       += wviewdWork.calCWindSpeed;

    wviewdWork.loopPkt.windDir          *= wviewdWork.calMWindDir;
    wviewdWork.loopPkt.windDir          += wviewdWork.calCWindDir;
    wviewdWork.loopPkt.windDir          %= 360;

    wviewdWork.loopPkt.windGustF        *= wviewdWork.calMWindSpeed;
    wviewdWork.loopPkt.windGustF        += wviewdWork.calCWindSpeed;

    wviewdWork.loopPkt.sampleRain       *= wviewdWork.calMRain;
    wviewdWork.loopPkt.sampleRain       += wviewdWork.calCRain;

    wviewdWork.loopPkt.rainRate         *= wviewdWork.calMRainRate;
    wviewdWork.loopPkt.rainRate         += wviewdWork.calCRainRate;

    // now calculate a few after all calibrations:
    wviewdWork.loopPkt.dewpoint = wvutilsCalculateDewpoint(wviewdWork.loopPkt.outTemp,
                                                           (float)wviewdWork.loopPkt.outHumidity);
    wviewdWork.loopPkt.windchill = wvutilsCalculateWindChill(wviewdWork.loopPkt.outTemp,
                                                             wviewdWork.loopPkt.windSpeedF);
    wviewdWork.loopPkt.heatindex = wvutilsCalculateHeatIndex(wviewdWork.loopPkt.outTemp,
                                                             (float)wviewdWork.loopPkt.outHumidity);

    // store the results:
    computedDataStoreSample (&wviewdWork);

    sampleRain = sensorGetCumulative(&wviewdWork.sensors.sensor[STF_INTERVAL][SENSOR_RAIN]);
    sampleET = sensorGetCumulative(&wviewdWork.sensors.sensor[STF_INTERVAL][SENSOR_ET]);

    // do some post-processing on the LOOP data:
    tempf = stormRainGet();
    if (tempf > 0)
        tempf += sampleRain;
    wviewdWork.loopPkt.stormRain        = tempf;
    wviewdWork.loopPkt.stormStart       = stormRainGetStartTimeT();

    tempf = sensorGetCumulative(&wviewdWork.sensors.sensor[STF_DAY][SENSOR_RAIN]);
    tempf += sampleRain;
    wviewdWork.loopPkt.dayRain          = tempf;

    tempf = sensorGetCumulative(&wviewdWork.sensors.sensor[STF_MONTH][SENSOR_RAIN]);
    tempf += sampleRain;
    wviewdWork.loopPkt.monthRain        = tempf;

    tempf = sensorGetCumulative(&wviewdWork.sensors.sensor[STF_YEAR][SENSOR_RAIN]);
    tempf += sampleRain;
    wviewdWork.loopPkt.yearRain         = tempf;

    if (sampleET > ARCHIVE_VALUE_NULL)
    {
        tempf = sensorGetCumulative(&wviewdWork.sensors.sensor[STF_DAY][SENSOR_ET]);
        tempf += sampleET;
        wviewdWork.loopPkt.dayET            = tempf;
    
        tempf = sensorGetCumulative(&wviewdWork.sensors.sensor[STF_MONTH][SENSOR_ET]);
        tempf += sampleET;
        wviewdWork.loopPkt.monthET          = tempf;
    
        tempf = sensorGetCumulative(&wviewdWork.sensors.sensor[STF_YEAR][SENSOR_ET]);
        tempf += sampleET;
        wviewdWork.loopPkt.yearET           = tempf;
    }

    wviewdWork.loopPkt.yearRainMonth    = wviewdWork.stationRainSeasonStart;

    statusIncrementStat(WVIEW_STATS_LOOP_PKTS_RX);
    return OK;
}

static int daemonStationInitComplete (void *eventData)
{
    ARCHIVE_PKT         newestRecord;

    if (eventData != 0)
    {
        // failed startup!
        radMsgLog (PRI_HIGH, "daemonStationInitComplete: station startup failed!");
        return ERROR;
    }

    if (!wviewdWork.runningFlag)
    {
        wviewdWork.runningFlag = TRUE;

        // set the positional data:
        if (stationGetPosition (&wviewdWork) == ERROR)
        {
            radMsgLog (PRI_HIGH, "daemonStationInitComplete: stationGetPosition failed!");
            emailAlertSend(ALERT_TYPE_STATION_READ);
            return ERROR;
        }

        // !!! the order of these calls is very important !!!

        // Initialize the HILOW database interface:
        computedDataInit (&wviewdWork);

        stormRainInit (wviewdWork.stationRainStormTrigger,
                       wviewdWork.stationRainStormIdleHours);

        computedDataClearInterval (&wviewdWork);

        // Clear last loop packet store:
        memset(&wviewdWork.lastLoopPkt, 0, sizeof(wviewdWork.lastLoopPkt));

        // we know it just finished an initial sensor readings, it is a
        // REQUIREMENT of the stationInit API...
        daemonStationLoopComplete ();

        // do an initial update to propogate the initial readings
        // (so we have some data to start with)
        computedDataUpdate (&wviewdWork);

        // start the timers...
        stationStartArchiveTimerUniform (&wviewdWork);
        stationStartCDataTimerUniform (&wviewdWork);
        radProcessTimerStart (wviewdWork.pushTimer, 30000L);  // first run
        stationStartSyncTimerUniform (&wviewdWork, TRUE);     // first run

        radMsgLog (PRI_STATUS, "-- Station Init Complete --");

        // get the newest archive file record date/time
        wviewdWork.archiveDateTime = dbsqliteArchiveGetNewestTime(&newestRecord);
        if ((int)wviewdWork.archiveDateTime == ERROR)
        {
            wviewdWork.archiveDateTime = time(NULL);
            radMsgLog (PRI_STATUS, "no archive records found in database!");
        }
        else
        {
            radMsgLog (PRI_STATUS, "newest archive record: %4.4d-%2.2d-%2.2d %2.2d:%2.2d",
                       wvutilsGetYear(wviewdWork.archiveDateTime),
                       wvutilsGetMonth(wviewdWork.archiveDateTime),
                       wvutilsGetDay(wviewdWork.archiveDateTime),
                       wvutilsGetHour(wviewdWork.archiveDateTime),
                       wvutilsGetMin(wviewdWork.archiveDateTime));
        }

        // finally, answer all the WVIEW_RQST_TYPE_STATION_INFO requestors
        // so they can continue initialization - our data is ready:
        stationProcessInfoResponses(&wviewdWork);
    }

    return OK;
}

static void daemonCheckArchiveRecord (ARCHIVE_PKT *newRecord)
{
    static ARCHIVE_PKT      LastPacket;
    static int              NoChangeCounter = 0;

    // See if any base values have changed:
    if (LastPacket.value[DATA_INDEX_outTemp] != newRecord->value[DATA_INDEX_outTemp])
    {
        NoChangeCounter = 0;
    }
    else if (LastPacket.value[DATA_INDEX_windSpeed] != newRecord->value[DATA_INDEX_windSpeed])
    {
        NoChangeCounter = 0;
    }
    else if (LastPacket.value[DATA_INDEX_windDir] != newRecord->value[DATA_INDEX_windDir])
    {
        NoChangeCounter = 0;
    }

    // If counter exceeds threshold, send alert and reset:
    if (NoChangeCounter >= WVD_FLATLINE_THRESHOLD(wviewdWork.archiveInterval))
    {
        radMsgLog (PRI_MEDIUM, "daemonCheckArchiveRecord: basic data values not changing!");
        emailAlertSend(ALERT_TYPE_STATION_FLATLINE);
        NoChangeCounter = 0;
    }

    // Always reset last packet:
    LastPacket = *newRecord;
}

static void daemonStoreArchiveRecord (ARCHIVE_PKT *newRecord)
{
    float           sampleRain;
    int             deltaTime;

    if (newRecord == NULL)
    {
        radMsgLog (PRI_MEDIUM, "daemonStoreArchiveRecord: record is NULL!");
        return;
    }

    deltaTime = newRecord->dateTime - wviewdWork.archiveDateTime;
    if (deltaTime == 0)
    {
        // discard it, same as previous record
        radMsgLog (PRI_MEDIUM,
                   "daemonStoreArchiveRecord: record has same timestamp as previous!");
        return;
    }
    else if (deltaTime < 0)
    {
        // chunk it, it is just wrong
        radMsgLog (PRI_MEDIUM,
                   "StoreArchiveRecord: record has earlier timestamp than previous (DST change?)");
        return;
    }

    wviewdWork.archiveDateTime = newRecord->dateTime;

    wvutilsLogEvent (PRI_STATUS, "storing record for %4.4d-%2.2d-%2.2d %2.2d:%2.2d",
                     wvutilsGetYear(newRecord->dateTime),
                     wvutilsGetMonth(newRecord->dateTime),
                     wvutilsGetDay(newRecord->dateTime),
                     wvutilsGetHour(newRecord->dateTime),
                     wvutilsGetMin(newRecord->dateTime));

    if (dbsqliteArchiveStoreRecord(newRecord) == ERROR)
    {
        radMsgLog (PRI_MEDIUM, "daemonStoreArchiveRecord: dbsqliteArchiveStoreRecord failed!!!");
        emailAlertSend(ALERT_TYPE_FILE_IO);
        return;
    }

    // Check for flatline values:
    daemonCheckArchiveRecord(newRecord);

    // if we are running normally (out of init), do normal activities:
    if (wviewdWork.runningFlag)
    {
        // Check to see if a DST change has occured:
        // Note: wvutilsDetectDSTChange can only be called once per process per
        //       DST event.
        if (wvutilsDetectDSTChange() != WVUTILS_DST_NO_CHANGE)
        {
            radMsgLog (PRI_STATUS, 
                       "DST change: scheduling station time update (if supported)");

            // Update the time zone info:
            tzset();

            // Adjust station time:
            stationSyncTime(&wviewdWork);
        }

        // compute storm rain:
        if (newRecord->value[DATA_INDEX_rain] > ARCHIVE_VALUE_NULL &&
            newRecord->value[DATA_INDEX_rainRate] > ARCHIVE_VALUE_NULL)
        {
            stormRainUpdate ((float)newRecord->value[DATA_INDEX_rainRate],
                             (float)newRecord->value[DATA_INDEX_rain]);
        }

        // sync to sensors:
        wviewdWork.loopPkt.stormRain  = stormRainGet();
        wviewdWork.loopPkt.stormStart = stormRainGetStartTimeT();
        wviewdWork.loopPkt.dayRain    = sensorGetCumulative(&wviewdWork.sensors.sensor[STF_DAY][SENSOR_RAIN]);
        wviewdWork.loopPkt.monthRain  = sensorGetCumulative(&wviewdWork.sensors.sensor[STF_MONTH][SENSOR_RAIN]);
        wviewdWork.loopPkt.yearRain   = sensorGetCumulative(&wviewdWork.sensors.sensor[STF_YEAR][SENSOR_RAIN]);
        wviewdWork.loopPkt.dayET      = sensorGetCumulative(&wviewdWork.sensors.sensor[STF_DAY][SENSOR_ET]);
        wviewdWork.loopPkt.monthET    = sensorGetCumulative(&wviewdWork.sensors.sensor[STF_MONTH][SENSOR_ET]);
        wviewdWork.loopPkt.yearET     = sensorGetCumulative(&wviewdWork.sensors.sensor[STF_YEAR][SENSOR_ET]);

        // send archive notification:
        stationSendArchiveNotifications (&wviewdWork, (float)newRecord->value[DATA_INDEX_rain]);
    }

    statusIncrementStat(WVIEW_STATS_ARCHIVE_PKTS_RX);
    return;
}

static void daemonArchiveIndication (ARCHIVE_PKT *newRecord)
{
    if (newRecord != NULL)
    {
        daemonStoreArchiveRecord (newRecord);

        // Push to internal clients:
        stationPushArchiveToClients(&wviewdWork, newRecord);
    }
    else if (wviewdWork.stationGeneratesArchives)
    {
        emailAlertSend(ALERT_TYPE_STATION_ARCHIVE);
    }

    return;
}


/*  ... system initialization
*/
static int daemonSysInit (WVIEWD_WORK *work)
{
    char            temp[256];
    char            *installPath;
    struct stat     fileData;
    FILE            *pidfile;

    /*  ... create our run directory if it is not there
    */
    sprintf (temp, "%s", WVIEW_RUN_DIR);
    if (stat (temp, &fileData) != 0)
    {
        if (mkdir (temp, 0755) != 0)
        {
            radMsgLogInit (PROC_NAME_DAEMON, TRUE, TRUE);
            radMsgLog (PRI_CATASTROPHIC,
                       "Cannot create run directory: %s - aborting!",
                       temp);
            radMsgLogExit ();
            return -1;
        }
    }

    /*  ... create our device directory if it is not there
    */
    sprintf (temp, "%s/dev", WVIEW_RUN_DIR);
    if (stat (temp, &fileData) != 0)
    {
        if (mkdir (temp, 0755) != 0)
        {
            radMsgLogInit (PROC_NAME_DAEMON, TRUE, TRUE);
            radMsgLog (PRI_CATASTROPHIC,
                       "Cannot create device directory: %s - aborting!",
                       temp);
            radMsgLogExit ();
            return -1;
        }
    }

    sprintf (work->pidFile, "%s/%s", WVIEW_RUN_DIR, WVD_LOCK_FILE_NAME);
    sprintf (work->fifoFile, "%s/dev/%s", WVIEW_RUN_DIR, PROC_NAME_DAEMON);
    sprintf (work->statusFile, "%s/%s", WVIEW_STATUS_DIRECTORY, WVIEW_STATUS_FILE_NAME);

    /*  ... check for our pid file, don't run if it is there
    */
    if (stat (work->pidFile, &fileData) == 0)
    {
        radMsgLogInit (PROC_NAME_DAEMON, TRUE, TRUE);
        radMsgLog (PRI_CATASTROPHIC,
                   "lock file %s exists, older copy may be running - aborting!",
                   work->pidFile);
        radMsgLogExit ();
        return -1;
    }

    return 0;
}

/*  ... system exit
*/
static int daemonSysExit (WVIEWD_WORK *work)
{
    struct stat     fileData;

    /*  ... delete our pid file
    */
    if (stat (work->pidFile, &fileData) == 0)
    {
        unlink (work->pidFile);
    }

    return 0;
}


static void defaultSigHandler (int signum)
{
    int         retVal;

    switch (signum)
    {
        case SIGHUP:
            // user wants us to change the verbosity setting
            retVal = wvutilsToggleVerbosity ();
            radMsgLog (PRI_STATUS, "wviewd: SIGHUP - toggling log verbosity %s",
                       ((retVal == 0) ? "OFF" : "ON"));

            radProcessSignalCatch(signum, defaultSigHandler);
            return;

        case SIGPIPE:
            // we have a far end socket disconnection, we'll handle it in the
            // "read/write" code
            radProcessSignalCatch(signum, defaultSigHandler);
            break;

        case SIGBUS:
        case SIGFPE:
        case SIGSEGV:
        case SIGXFSZ:
        case SIGSYS:
            // unrecoverable radProcessSignalCatch- we must exit right now!
            radMsgLog (PRI_CATASTROPHIC, "wviewd: recv sig %d: shutting down!", signum);
            abort();

        case SIGCHLD:
            wvutilsWaitForChildren();
            radProcessSignalCatch(signum, defaultSigHandler);
            return;

        default:
            // we can allow the process to exit normally...
            if (wviewdWork.exiting)
            {
                radProcessSignalCatch(signum, defaultSigHandler);
                return;
            }

            radMsgLog (PRI_HIGH, "wviewd: recv sig %d: exiting!", signum);

            wviewdWork.exiting = TRUE;
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
    if (msgType == WVIEW_MSG_TYPE_POLL)
    {
        WVIEW_MSG_POLL*     pPoll = (WVIEW_MSG_POLL*)msg;
        wvutilsSendPMONPollResponse (pPoll->mask, PMON_PROCESS_WVIEWD);
        return;
    }

    stationProcessIPM (&wviewdWork, srcQueueName, msgType, msg);
    return;
}

static void evtHandler
(
    UINT        eventsRx,
    UINT        rxData,
    void        *userData
)
{
    // bleed off our special events
    if (eventsRx & STATION_INIT_COMPLETE_EVENT)
    {
        // call the init complete handler
        if (daemonStationInitComplete(userData) == ERROR)
        {
            stationSendShutdown(&wviewdWork);
            wviewdWork.exiting = TRUE;
            return;
        }

        eventsRx &= ~STATION_INIT_COMPLETE_EVENT;
    }

    if (eventsRx & STATION_LOOP_COMPLETE_EVENT)
    {
        // call the loop readings complete handler
        daemonStationLoopComplete ();

        eventsRx &= ~STATION_LOOP_COMPLETE_EVENT;
    }

    return;
}


static void archiveTimerHandler (void *parm)
{
    ARCHIVE_PKT*    newRec;
    time_t          ntime;

    // get the current time
    ntime = time (NULL);

    // check to see if system time has changed
    if (ntime < (wviewdWork.nextArchiveTime - 4))
    {
        // time was set back since our last timer start - restart the timer
        radMsgLog (PRI_MEDIUM, "archiveTimerHandler: system time has skewed, adjusting...");
        stationStartArchiveTimerUniform (&wviewdWork);
        return;
    }

    if (wviewdWork.stationGeneratesArchives)
    {
        // tell the station to generate an archive record
        // (he will indicate it back to us)
        stationGetArchive (&wviewdWork);
    }
    else
    {
        // generate it on our own
        newRec = computedDataGenerateArchive(&wviewdWork);
        if (newRec != NULL)
        {
            daemonStoreArchiveRecord(newRec);

            // Push to internal clients:
            stationPushArchiveToClients(&wviewdWork, newRec);
        }
        else
        {
            radMsgLog (PRI_MEDIUM, "STATION: no new archive record generated "
                                   "probably caused by not receiving any LOOP data");
            emailAlertSend(ALERT_TYPE_STATION_ARCHIVE);
        }
    }

    // Update computed values:
    computedDataUpdate (&wviewdWork);

    // clear for the next archive period:
    computedDataClearInterval (&wviewdWork);

    // restart the timer
    stationStartArchiveTimerUniform (&wviewdWork);
    return;
}

static void cdtimerHandler (void *parm)
{
    // tell the station to acquire data
    stationGetReadings (&wviewdWork);

    // restart the timer
    stationStartCDataTimerUniform (&wviewdWork);
    return;
}

static void pushTimerHandler (void *parm)
{
    // ... send to clients
    stationPushDataToClients (&wviewdWork);
    return;
}

static void syncTimerHandler (void *parm)
{
    if (stationStartSyncTimerUniform(&wviewdWork, FALSE) == TRUE)
    {
        // tell the station to synchronize the station time (if required)
        stationSyncTime (&wviewdWork);
    }

    return;
}

static void ifTimerHandler (void *parm)
{
    // we just pass through the IF timer to the station-specific indication
    stationIFTimerExpiry (&wviewdWork);

    return;
}

static void stationDataCallback (int fd, void *userData)
{
    // we just indicate the IF data to the station-specific function
    stationDataIndicate (&wviewdWork);

    return;
}


/*  ... the main entry point for the daemon process
*/
int main (int argc, char *argv[])
{
    void            (*alarmHandler)(int);
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

    /*  ... start with a clean slate
    */
    memset (&wviewdWork, 0, sizeof (wviewdWork));

    /*  ... initialize some system stuff first
    */
    if (daemonSysInit (&wviewdWork) == -1)
    {
        radMsgLogInit (PROC_NAME_DAEMON, TRUE, TRUE);
        radMsgLog (PRI_CATASTROPHIC, "system init failed!\n");
        radMsgLogExit ();
        exit (1);
    }


    /*  ... call the global radlib system init function
    */
    if (radSystemInit (WVIEW_SYSTEM_ID) == ERROR)
    {
        radMsgLogInit (PROC_NAME_DAEMON, TRUE, TRUE);
        radMsgLog (PRI_CATASTROPHIC, "radSystemInit failed!");
        radMsgLogExit ();
        exit (1);
    }


    /*  ... call the radlib process init function
    */
    if (radProcessInit (PROC_NAME_DAEMON,
                        wviewdWork.fifoFile,
                        PROC_NUM_TIMERS_DAEMON,
                        runAsDaemon,                // TRUE for daemon
                        msgHandler,
                        evtHandler,
                        NULL)
        == ERROR)
    {
        printf ("\nradProcessInit failed: %s\n\n", PROC_NAME_DAEMON);
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }

    wviewdWork.myPid = getpid ();
    pidfile = fopen (wviewdWork.pidFile, "w");
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


    radMsgLog (PRI_STATUS, "%s starting ...", globalWviewVersionStr);
    radTimeGetMSSinceEpoch();
    wvutilsDetectDSTInit();

    // get our configuration values:
    if (wvconfigInit(TRUE) == ERROR)
    {
        radMsgLog (PRI_CATASTROPHIC, "config database is missing!!!\n");
        daemonSysExit (&wviewdWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }

    // get the wview verbosity setting
    if (wvutilsSetVerbosity (WV_VERBOSE_WVIEWD) == ERROR)
    {
        wvconfigExit();
        radMsgLog (PRI_CATASTROPHIC, "wvutilsSetVerbosity failed!");
        daemonSysExit (&wviewdWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }

    ///// STATION_INTERFACE PROCESSING BEGIN /////
    sValue = wvconfigGetStringValue(configItem_STATION_STATION_TYPE);
    if (sValue == NULL)
    {
        radMsgLog (PRI_MEDIUM,
                   "no station type given, defaulting to 'VantagePro'...");
        strcpy (wviewdWork.stationType, "VantagePro");
    }
    else
    {
        wvstrncpy(wviewdWork.stationType, sValue, sizeof(wviewdWork.stationType));
    }

    if ((!strcmp(wviewdWork.stationType, "WMRUSB")) ||
        (!strcmp(wviewdWork.stationType, "WH1080")))
    {
        // USB stations:
        radMsgLog (PRI_MEDIUM,
                   "station interface: native USB ...");
    }
    else
    {
        sValue = wvconfigGetStringValue(configItem_STATION_STATION_INTERFACE);
        if (sValue == NULL)
        {
            radMsgLog (PRI_MEDIUM,
                       "no station interface given, defaulting to 'serial'...");
            strcpy (wviewdWork.stationInterface, "serial");
        }
        else
        {
            wvstrncpy(wviewdWork.stationInterface, sValue, sizeof(wviewdWork.stationInterface));
        }
    
        // grab the Weatherlink retrieve archives flag:
        iValue = wvconfigGetBooleanValue(configItem_STATION_STATION_RETRIEVE_ARCHIVE);
        if (iValue >= 0)
        {
            wviewdWork.stationGeneratesArchives = iValue;
            radMsgLog (PRI_MEDIUM,
                       "station %s archive records",
                       ((iValue) ? "generates" : "does not generate"));
        }
        else
        {
            // Default to the typical scenario. Stations not supporting archives 
            // will overwrite it. 
            wviewdWork.stationGeneratesArchives = TRUE;
        }

        // process the interface type
        if (!strcmp (wviewdWork.stationInterface, "serial"))
        {
            radMsgLog (PRI_MEDIUM,
                       "station interface: serial ...");
    
            // we need a device name for serial IFs
            sValue = wvconfigGetStringValue(configItem_STATION_STATION_DEV);
            if (sValue == NULL)
            {
                wvconfigExit();
                radMsgLog (PRI_CATASTROPHIC,
                           "no serial device given, aborting...");
                daemonSysExit (&wviewdWork);
                radProcessExit ();
                radSystemExit (WVIEW_SYSTEM_ID);
                exit (1);
            }
            else
            {
                wvstrncpy(wviewdWork.stationDevice, sValue, sizeof(wviewdWork.stationDevice));
            }
    
            // grab the DTR toggle flag:
            iValue = wvconfigGetBooleanValue(configItem_STATION_STATION_DTR);
            if (iValue >= 0)
            {
                wviewdWork.stationToggleDTR = iValue;
            }
            else
            {
                wviewdWork.stationToggleDTR = TRUE;
            }
        }
        else if (!strcmp (wviewdWork.stationInterface, "ethernet"))
        {
            radMsgLog (PRI_MEDIUM,
                       "station interface: ethernet ...");
    
            // we need host and port for ethernet
            sValue = wvconfigGetStringValue(configItem_STATION_STATION_HOST);
            if (sValue == NULL)
            {
                wvconfigExit();
                radMsgLog (PRI_CATASTROPHIC,
                           "no hostname given, aborting...");
                daemonSysExit (&wviewdWork);
                radProcessExit ();
                radSystemExit (WVIEW_SYSTEM_ID);
                exit (1);
            }
            else
            {
                wvstrncpy(wviewdWork.stationHost, sValue, sizeof(wviewdWork.stationHost));
    
                iValue = wvconfigGetINTValue(configItem_STATION_STATION_PORT);
                if (iValue <= 0)
                {
                    wvconfigExit();
                    radMsgLog (PRI_CATASTROPHIC,
                               "no port given, aborting...");
                    daemonSysExit (&wviewdWork);
                    radProcessExit ();
                    radSystemExit (WVIEW_SYSTEM_ID);
                    exit (1);
                }
                else
                {
                    wviewdWork.stationPort = iValue;
    
                    // grab the Weatherlink IP flag:
                    iValue = wvconfigGetBooleanValue(configItem_STATION_STATION_WLIP);
                    if (iValue >= 0)
                    {
                        wviewdWork.stationIsWLIP = iValue;
                    }
                    else
                    {
                        wviewdWork.stationIsWLIP = FALSE;
                    }
                }
            }
        }
        else
        {
            // invalid type specified - abort
            wvconfigExit();
            radMsgLog (PRI_CATASTROPHIC,
                       "invalid STATION_INTERFACE %s given, aborting...",
                       wviewdWork.stationInterface);
            daemonSysExit (&wviewdWork);
            radProcessExit ();
            radSystemExit (WVIEW_SYSTEM_ID);
            exit (1);
        }
    }
    ///// STATION_INTERFACE PROCESSING END /////

    iValue = wvconfigGetINTValue(configItem_STATION_STATION_RAIN_SEASON_START);
    if (iValue <= 0)
    {
        radMsgLog (PRI_MEDIUM, "Rain Season Start Month not found - defaulting to 1 (JAN)...\n");
        wviewdWork.stationRainSeasonStart = 1;
    }
    else
    {
        wviewdWork.stationRainSeasonStart = iValue;
        if (wviewdWork.stationRainSeasonStart < 1 ||
            wviewdWork.stationRainSeasonStart > 12)
        {
            radMsgLog (PRI_MEDIUM, "Invalid Rain Season Start Month %d found - defaulting to 1 (JAN)...\n",
                       wviewdWork.stationRainSeasonStart);
            wviewdWork.stationRainSeasonStart = 1;
        }
        else
        {
            radMsgLog (PRI_STATUS, "Rain Season Start Month set to %d\n",
                       wviewdWork.stationRainSeasonStart);
        }
    }

    dValue = wvconfigGetDOUBLEValue(configItem_STATION_STATION_RAIN_STORM_TRIGGER_START);
    if (dValue <= 0.0)
    {
        radMsgLog (PRI_MEDIUM,
                   "no rain storm start trigger given, defaulting to 0.05 in/hr...");
        wviewdWork.stationRainStormTrigger = 0.05;
    }
    else
    {
        wviewdWork.stationRainStormTrigger = (float)dValue;
        radMsgLog (PRI_STATUS, "Rain Storm Start Trigger set to %5.2f in/hr\n",
                   wviewdWork.stationRainStormTrigger);
    }

    iValue = wvconfigGetINTValue(configItem_STATION_STATION_RAIN_STORM_IDLE_STOP);
    if (iValue <= 0)
    {
        radMsgLog (PRI_MEDIUM,
                   "no rain storm idle stop time given, defaulting to 12 hours...");
        wviewdWork.stationRainStormIdleHours = 12;
    }
    else
    {
        wviewdWork.stationRainStormIdleHours = iValue;
        radMsgLog (PRI_STATUS, "Rain Storm Stop Time set to %d hours\n",
                   wviewdWork.stationRainStormIdleHours);
    }

    dValue = wvconfigGetDOUBLEValue(configItem_STATION_STATION_RAIN_YTD);
    if (dValue < 0.0)
    {
        radMsgLog (PRI_MEDIUM,
                   "no rain YTD preset given, defaulting to 0.00 inches...");
        wviewdWork.stationRainStormIdleHours = 12;
    }
    else
    {
        wviewdWork.stationRainYTDPreset = (float)dValue;
        radMsgLog (PRI_STATUS, "Rain YTD preset set to %.2f inches\n",
                   wviewdWork.stationRainYTDPreset);
    }

    dValue = wvconfigGetDOUBLEValue(configItem_STATION_STATION_ET_YTD);
    if (dValue < 0.0)
    {
        radMsgLog (PRI_MEDIUM,
                   "no ET YTD preset given, defaulting to 0.000 inches...");
        wviewdWork.stationETYTDPreset = 0;
    }
    else
    {
        wviewdWork.stationETYTDPreset = (float)dValue;
        radMsgLog (PRI_STATUS, "ET YTD preset set to %.3f inches\n",
                   wviewdWork.stationETYTDPreset);
    }

    iValue = wvconfigGetINTValue(configItem_STATION_STATION_RAIN_ET_YTD_YEAR);
    if (iValue < 0)
    {
        radMsgLog (PRI_MEDIUM,
                   "no rain/ET YTD Year given, disabling...");
        wviewdWork.stationRainETPresetYear = 0;
    }
    else
    {
        wviewdWork.stationRainETPresetYear = iValue;
        if (wviewdWork.stationRainETPresetYear < 2000 ||
            wviewdWork.stationRainETPresetYear > 3000)
        {
            radMsgLog (PRI_MEDIUM,
                   "bad rain/ET YTD Year given, disabling...");
            wviewdWork.stationRainETPresetYear = 0;
        }
        else
        {
            radMsgLog (PRI_STATUS, "rain/ET YTD preset Year set to %d\n",
                       wviewdWork.stationRainETPresetYear);
        }
    }

    iValue = wvconfigGetINTValue(configItem_STATION_POLL_INTERVAL);
    if (iValue < 0)
    {
        radMsgLog (PRI_MEDIUM,
                   "no POLL_INTERVAL retrieved, setting to 30 seconds...");
        wviewdWork.cdataInterval = 30000;
    }
    else
    {
        wviewdWork.cdataInterval = iValue * 1000;
    }

    if (((wviewdWork.cdataInterval % 1000) != 0) ||
        ((wviewdWork.cdataInterval/1000) > 60) ||
        ((60 % (wviewdWork.cdataInterval/1000)) != 0))
    {
        radMsgLog (PRI_MEDIUM,
                   "station polling interval %d found in wview.conf is invalid:",
                   wviewdWork.cdataInterval);
        radMsgLog (PRI_MEDIUM,
                   "defaulting to 30 seconds ...");
        radMsgLog (PRI_MEDIUM,
                   "Note: station polling interval must be less than 60 seconds");
        radMsgLog (PRI_MEDIUM,
                   "      and an even divisor of 60 seconds (10000, 15000, 30000)");
        wviewdWork.cdataInterval = 30 * 1000;
    }
    else
    {
        radMsgLog (PRI_STATUS, "station polling interval set to %d seconds",
                   (wviewdWork.cdataInterval/1000));
    }

    iValue = wvconfigGetINTValue(configItem_STATION_PUSH_INTERVAL);
    if (iValue < 0)
    {
        radMsgLog (PRI_MEDIUM,
                   "no PUSH_INTERVAL retrieved, setting to 60 seconds...");
        wviewdWork.pushInterval = 60000;
    }
    else
    {
        wviewdWork.pushInterval = iValue * 1000;
    }


    // Calibration configuration:
    dValue = wvconfigGetDOUBLEValue(configItemCAL_MULT_BAROMETER);
    if (dValue <= 0.0)
    {
        wviewdWork.calMBarometer = 1.00;
    }
    else
    {
        wviewdWork.calMBarometer = dValue;
    }
    dValue = wvconfigGetDOUBLEValue(configItemCAL_CONST_BAROMETER);
    wviewdWork.calCBarometer = dValue;

    dValue = wvconfigGetDOUBLEValue(configItemCAL_MULT_PRESSURE);
    if (dValue <= 0.0)
    {
        wviewdWork.calMPressure = 1.00;
    }
    else
    {
        wviewdWork.calMPressure = dValue;
    }
    dValue = wvconfigGetDOUBLEValue(configItemCAL_CONST_PRESSURE);
    wviewdWork.calCPressure = dValue;

    dValue = wvconfigGetDOUBLEValue(configItemCAL_MULT_ALTIMETER);
    if (dValue <= 0.0)
    {
        wviewdWork.calMAltimeter = 1.00;
    }
    else
    {
        wviewdWork.calMAltimeter = dValue;
    }
    dValue = wvconfigGetDOUBLEValue(configItemCAL_CONST_ALTIMETER);
    wviewdWork.calCAltimeter = dValue;

    dValue = wvconfigGetDOUBLEValue(configItemCAL_MULT_INTEMP);
    if (dValue <= 0.0)
    {
        wviewdWork.calMInTemp = 1.00;
    }
    else
    {
        wviewdWork.calMInTemp = dValue;
    }
    dValue = wvconfigGetDOUBLEValue(configItemCAL_CONST_INTEMP);
    wviewdWork.calCInTemp = dValue;

    dValue = wvconfigGetDOUBLEValue(configItemCAL_MULT_OUTTEMP);
    if (dValue <= 0.0)
    {
        wviewdWork.calMOutTemp = 1.00;
    }
    else
    {
        wviewdWork.calMOutTemp = dValue;
    }
    dValue = wvconfigGetDOUBLEValue(configItemCAL_CONST_OUTTEMP);
    wviewdWork.calCOutTemp = dValue;

    dValue = wvconfigGetDOUBLEValue(configItemCAL_MULT_INHUMIDITY);
    if (dValue <= 0.0)
    {
        wviewdWork.calMInHumidity = 1.00;
    }
    else
    {
        wviewdWork.calMInHumidity = dValue;
    }
    dValue = wvconfigGetDOUBLEValue(configItemCAL_CONST_INHUMIDITY);
    wviewdWork.calCInHumidity = dValue;

    dValue = wvconfigGetDOUBLEValue(configItemCAL_MULT_OUTHUMIDITY);
    if (dValue <= 0.0)
    {
        wviewdWork.calMOutHumidity = 1.00;
    }
    else
    {
        wviewdWork.calMOutHumidity = dValue;
    }
    dValue = wvconfigGetDOUBLEValue(configItemCAL_CONST_OUTHUMIDITY);
    wviewdWork.calCOutHumidity = dValue;

    dValue = wvconfigGetDOUBLEValue(configItemCAL_MULT_WINDSPEED);
    if (dValue <= 0.0)
    {
        wviewdWork.calMWindSpeed = 1.00;
    }
    else
    {
        wviewdWork.calMWindSpeed = dValue;
    }
    dValue = wvconfigGetDOUBLEValue(configItemCAL_CONST_WINDSPEED);
    wviewdWork.calCWindSpeed = dValue;

    dValue = wvconfigGetDOUBLEValue(configItemCAL_MULT_WINDDIR);
    if (dValue <= 0.0)
    {
        wviewdWork.calMWindDir = 1.00;
    }
    else
    {
        wviewdWork.calMWindDir = dValue;
    }
    dValue = wvconfigGetDOUBLEValue(configItemCAL_CONST_WINDDIR);
    wviewdWork.calCWindDir = dValue;

    dValue = wvconfigGetDOUBLEValue(configItemCAL_MULT_RAIN);
    if (dValue <= 0.0)
    {
        wviewdWork.calMRain = 1.00;
    }
    else
    {
        wviewdWork.calMRain = dValue;
    }
    dValue = wvconfigGetDOUBLEValue(configItemCAL_CONST_RAIN);
    wviewdWork.calCRain = dValue;

    dValue = wvconfigGetDOUBLEValue(configItemCAL_MULT_RAINRATE);
    if (dValue <= 0.0)
    {
        wviewdWork.calMRainRate = 1.00;
    }
    else
    {
        wviewdWork.calMRainRate = dValue;
    }
    dValue = wvconfigGetDOUBLEValue(configItemCAL_CONST_RAINRATE);
    wviewdWork.calCRainRate = dValue;

    iValue = wvconfigGetBooleanValue(configItem_ENABLE_EMAIL);
    if (iValue >= 0)
    {
        wviewdWork.IsAlertEmailsEnabled = iValue;
    }
    if (wviewdWork.IsAlertEmailsEnabled)
    {
        sValue = wvconfigGetStringValue(configItem_TO_EMAIL_ADDRESS);
        if (sValue == NULL)
        {
            radMsgLog (PRI_HIGH, "NO alert email TO address given - disabling email alerts...");
            wviewdWork.IsAlertEmailsEnabled = 0;
        }
        else
        {
            wvstrncpy (wviewdWork.alertEmailToAdrs, sValue, sizeof(wviewdWork.alertEmailToAdrs));
        }
        sValue = wvconfigGetStringValue(configItem_FROM_EMAIL_ADDRESS);
        if (sValue == NULL)
        {
            radMsgLog (PRI_HIGH, "NO alert email FROM address given - disabling email alerts...");
            wviewdWork.IsAlertEmailsEnabled = 0;
        }
        else
        {
            wvstrncpy (wviewdWork.alertEmailFromAdrs, sValue, sizeof(wviewdWork.alertEmailFromAdrs));
        }
        iValue = wvconfigGetBooleanValue(configItem_SEND_TEST_EMAIL);
        if (iValue >= 0)
        {
            wviewdWork.IsTestEmailEnabled = iValue;
        }
    }
    iValue = wvconfigGetBooleanValue(configItem_HTMLGEN_STATION_SHOW_IF);
    if (iValue >= 0)
    {
        wviewdWork.showStationIF = iValue;
    }
    else
    {
        wviewdWork.showStationIF = TRUE;
    }

    wvconfigExit ();

    if (statusInit(wviewdWork.statusFile, wviewStatusLabels) == ERROR)
    {
        radMsgLog (PRI_HIGH, "statusInit failed - exiting...");
        daemonSysExit (&wviewdWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }
    statusUpdate(STATUS_BOOTING);

    // ... Initialize the archive database interface:
    if (dbsqliteArchiveInit() == ERROR)
    {
        radMsgLog (PRI_HIGH, "dbsqliteArchiveInit failed");
        statusUpdateMessage("dbsqliteArchiveInit failed");
        statusUpdate(STATUS_ERROR);
        daemonSysExit (&wviewdWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }


    // Initialize timers:
    wviewdWork.archiveTimer = radTimerCreate (NULL, archiveTimerHandler, NULL);
    if (wviewdWork.archiveTimer == NULL)
    {
        radMsgLog (PRI_HIGH, "radTimerCreate failed");
        statusUpdateMessage("radTimerCreate failed");
        statusUpdate(STATUS_ERROR);
        daemonSysExit (&wviewdWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }

    wviewdWork.cdataTimer = radTimerCreate (NULL, cdtimerHandler, NULL);
    if (wviewdWork.cdataTimer == NULL)
    {
        radMsgLog (PRI_HIGH, "radTimerCreate failed");
        statusUpdateMessage("radTimerCreate failed");
        statusUpdate(STATUS_ERROR);
        radTimerDelete (wviewdWork.archiveTimer);
        daemonSysExit (&wviewdWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }

    wviewdWork.pushTimer = radTimerCreate (NULL, pushTimerHandler, NULL);
    if (wviewdWork.pushTimer == NULL)
    {
        radMsgLog (PRI_HIGH, "radTimerCreate failed");
        statusUpdateMessage("radTimerCreate failed");
        statusUpdate(STATUS_ERROR);
        radTimerDelete (wviewdWork.cdataTimer);
        radTimerDelete (wviewdWork.archiveTimer);
        daemonSysExit (&wviewdWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }

    wviewdWork.syncTimer = radTimerCreate (NULL, syncTimerHandler, NULL);
    if (wviewdWork.syncTimer == NULL)
    {
        radMsgLog (PRI_HIGH, "sync radTimerCreate failed");
        statusUpdateMessage("radTimerCreate failed");
        statusUpdate(STATUS_ERROR);
        radTimerDelete (wviewdWork.cdataTimer);
        radTimerDelete (wviewdWork.pushTimer);
        radTimerDelete (wviewdWork.archiveTimer);
        daemonSysExit (&wviewdWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }

    wviewdWork.ifTimer = radTimerCreate (NULL, ifTimerHandler, NULL);
    if (wviewdWork.ifTimer == NULL)
    {
        radMsgLog (PRI_HIGH, "sync radTimerCreate failed");
        statusUpdateMessage("radTimerCreate failed");
        statusUpdate(STATUS_ERROR);
        radTimerDelete (wviewdWork.syncTimer);
        radTimerDelete (wviewdWork.cdataTimer);
        radTimerDelete (wviewdWork.pushTimer);
        radTimerDelete (wviewdWork.archiveTimer);
        daemonSysExit (&wviewdWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }

    radProcessEventsAdd (STATION_INIT_COMPLETE_EVENT);
    radProcessEventsAdd (STATION_LOOP_COMPLETE_EVENT);

    //  register with the radlib message router
    if (radMsgRouterInit (WVIEW_RUN_DIR) == ERROR)
    {
        radMsgLog (PRI_HIGH, "radMsgRouterInit failed!");
        statusUpdateMessage("radMsgRouterInit failed");
        statusUpdate(STATUS_ERROR);
        radTimerDelete (wviewdWork.ifTimer);
        radTimerDelete (wviewdWork.syncTimer);
        radTimerDelete (wviewdWork.cdataTimer);
        radTimerDelete (wviewdWork.pushTimer);
        radTimerDelete (wviewdWork.archiveTimer);
        daemonSysExit (&wviewdWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }

    // enable message reception from the radlib router for worker requests
    radMsgRouterMessageRegister (WVIEW_MSG_TYPE_REQUEST);

    // enable message reception from the radlib router for POLL msgs
    radMsgRouterMessageRegister (WVIEW_MSG_TYPE_POLL);

    // enable message reception from the radlib router for ALERT msgs
    radMsgRouterMessageRegister (WVIEW_MSG_TYPE_ALERT);

    // enable message reception from the radlib router for STATION_DATA msgs
    radMsgRouterMessageRegister (WVIEW_MSG_TYPE_STATION_DATA);


    // Initialize the HILOW database interface:
    // (this cannot occur before the MsgRouter is initialized)
    if (dbsqliteHiLowInit(TRUE) == ERROR)
    {
        radMsgLog (PRI_HIGH, "dbsqliteHiLowInit failed");
        statusUpdateMessage("dbsqliteHiLowInit failed");
        statusUpdate(STATUS_ERROR);
        stationSendShutdown(&wviewdWork);
        radMsgRouterExit ();
        radTimerDelete (wviewdWork.ifTimer);
        radTimerDelete (wviewdWork.syncTimer);
        radTimerDelete (wviewdWork.cdataTimer);
        radTimerDelete (wviewdWork.pushTimer);
        radTimerDelete (wviewdWork.archiveTimer);
        daemonSysExit (&wviewdWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }


    // initialize the station abstraction
    radMsgLog (PRI_STATUS, "-- Station Init Start --");
    if (stationInit (&wviewdWork, daemonArchiveIndication) == ERROR)
    {
        radMsgLog (PRI_HIGH, "stationInit failed!");
        statusUpdateMessage("stationInit failed");
        statusUpdate(STATUS_ERROR);
        stationSendShutdown(&wviewdWork);
        radMsgRouterExit ();
        radTimerDelete (wviewdWork.ifTimer);
        radTimerDelete (wviewdWork.syncTimer);
        radTimerDelete (wviewdWork.cdataTimer);
        radTimerDelete (wviewdWork.pushTimer);
        radTimerDelete (wviewdWork.archiveTimer);
        daemonSysExit (&wviewdWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }

    // register the station interface if it is device-based:
    if (wviewdWork.medium.type == MEDIUM_TYPE_DEVICE)
    {
        if (radProcessIORegisterDescriptor (wviewdWork.medium.fd,
                                            stationDataCallback,
                                            NULL)
            == ERROR)
        {
            radMsgLog (PRI_HIGH, "IORegDescriptor failed");
            statusUpdateMessage("IORegDescriptor failed");
            statusUpdate(STATUS_ERROR);
            stationSendShutdown(&wviewdWork);
            radMsgRouterExit ();
            radTimerDelete (wviewdWork.ifTimer);
            radTimerDelete (wviewdWork.syncTimer);
            radTimerDelete (wviewdWork.cdataTimer);
            radTimerDelete (wviewdWork.pushTimer);
            radTimerDelete (wviewdWork.archiveTimer);
            stationExit (&wviewdWork);
            daemonSysExit (&wviewdWork);
            radProcessExit ();
            radSystemExit (WVIEW_SYSTEM_ID);
            exit (1);
        }
    }

    // Send test email if it is enabled:
    if (wviewdWork.IsTestEmailEnabled)
    {
        radMsgLog(PRI_STATUS, "Sending test email...");
        emailAlertSend(ALERT_TYPE_TEST);
    }


    statusUpdate(STATUS_RUNNING);
    statusUpdateMessage("Normal operation");
    radMsgLog (PRI_STATUS, "running...");


    while (!wviewdWork.exiting)
    {
        // wait on timers, events, file descriptors, msgs
        if (radProcessWait (0) == ERROR)
        {
            wviewdWork.exiting = TRUE;
        }
    }


    statusUpdateMessage("exiting normally");
    radMsgLog (PRI_STATUS, "exiting normally...");
    statusUpdate(STATUS_SHUTDOWN);

    computedDataExit (&wviewdWork);
    radMsgRouterExit ();
    radTimerDelete (wviewdWork.ifTimer);
    radTimerDelete (wviewdWork.syncTimer);
    radTimerDelete (wviewdWork.pushTimer);
    radTimerDelete (wviewdWork.cdataTimer);
    radTimerDelete (wviewdWork.archiveTimer);
    stationExit (&wviewdWork);
    dbsqliteHiLowExit();
    dbsqliteArchiveExit();
    daemonSysExit (&wviewdWork);
    radProcessExit ();
    radSystemExit (WVIEW_SYSTEM_ID);
    exit (0);
}

// Retrieve exit status:
int wviewdIsExiting(void)
{
    return wviewdWork.exiting;
}

