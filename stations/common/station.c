/*---------------------------------------------------------------------------

  FILENAME:
        station.c

  PURPOSE:
        Provide the station abstraction utility.

  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        12/31/2005      M.S. Teel       0               Original

  NOTES:

  LICENSE:
        Copyright (c) 2005, Mark S. Teel (mark@teel.ws)

        This source code is released for free distribution under the terms
        of the GNU General Public License.

----------------------------------------------------------------------------*/

/*  ... System include files
*/

/*  ... Library include files
*/
#include <radprocutils.h>

/*  ... Local include files
*/
#include <station.h>

/*  ... global memory declarations
*/

static int newProcessEntryPoint (void * pargs)
{
    char*               binFile = (char*)pargs;

    return (system(binFile));
}

static int processAlertMessage (WVIEWD_WORK *work, EmailAlertTypes type)
{
    char        syscmnd[256];
    int         retVal;
    time_t      timenow = time(NULL);
    struct tm   bknTime;

    if (! work->IsAlertEmailsEnabled || 
        ! strlen(work->alertEmailToAdrs) ||
        ! strlen(work->alertEmailFromAdrs))
    {
        return OK;
    }

    localtime_r(&timenow, &bknTime);
    sprintf(syscmnd, "sendEmail -f %s -t %s -u \"%4.4d%2.2d%2.2d %2.2d:%2.2d: %s\" -m \"%s\"",
            work->alertEmailFromAdrs,
            work->alertEmailToAdrs,
            bknTime.tm_year + 1900,
            bknTime.tm_mon + 1,
            bknTime.tm_mday,
            bknTime.tm_hour,
            bknTime.tm_min,
            emailAlertGetSubject(type),
            emailAlertGetBody(type));

    radStartProcess (newProcessEntryPoint, syscmnd);
    return OK;
}


// send shutdown notification
int stationSendShutdown (WVIEWD_WORK *work)
{
    WVIEW_MSG_SHUTDOWN          msg;

    if (radMsgRouterMessageSend (WVIEW_MSG_TYPE_SHUTDOWN, &msg, sizeof(msg))
        == ERROR)
    {
        // can't send!
        radMsgLog (PRI_HIGH, "radMsgRouterMessageSend failed: shutdown");
        return ERROR;
    }
    
    return OK;
}

int stationSendArchiveNotifications (WVIEWD_WORK *work, float sampleRain)
{
    WVIEW_MSG_ARCHIVE_NOTIFY    notify;
    int                         retVal;
    HISTORY_DATA                store;

    notify.dateTime         = work->archiveDateTime;
    notify.intemp           = (int)floorf(work->loopPkt.inTemp * 10);
    notify.inhumidity       = work->loopPkt.inHumidity;
    notify.temp             = (int)floorf(work->loopPkt.outTemp * 10);
    notify.humidity         = work->loopPkt.outHumidity;
    notify.barom            = (int)floorf(work->loopPkt.barometer * 1000);
    notify.stationPressure  = (int)floorf(work->loopPkt.stationPressure * 1000);
    notify.altimeter        = (int)floorf(work->loopPkt.altimeter * 1000);
    notify.winddir          = work->loopPkt.windDir;
    notify.wspeedF          = work->loopPkt.windSpeedF;
    notify.dewpoint         = (int)floorf(work->loopPkt.dewpoint * 10);
    notify.hiwspeedF        = work->loopPkt.windGustF;
    notify.rxPercent        = work->loopPkt.rxCheckPercent;
    notify.sampleRain       = sampleRain;
    notify.UV               = work->loopPkt.UV;
    notify.radiation        = work->loopPkt.radiation;

    // Grab last 60 minutes and last 24 hours from database:
    retVal = dbsqliteArchiveGetAverages (FALSE,
                                         work->archiveInterval,
                                         &store,
                                         time(NULL) - WV_SECONDS_IN_HOUR,
                                         WV_SECONDS_IN_HOUR/SECONDS_IN_INTERVAL(work->archiveInterval));
    if (retVal <= 0)
    {
        notify.rainHour = ARCHIVE_VALUE_NULL;
    }
    else
    {
        notify.rainHour = store.values[DATA_INDEX_rain];
    }

    retVal = dbsqliteArchiveGetAverages (FALSE,
                                         work->archiveInterval,
                                         &store,
                                         time(NULL) - WV_SECONDS_IN_DAY,
                                         WV_SECONDS_IN_DAY/SECONDS_IN_INTERVAL(work->archiveInterval));
    if (retVal <= 0)
    {
        notify.rainDay = ARCHIVE_VALUE_NULL;
    }
    else
    {
        notify.rainDay = store.values[DATA_INDEX_rain];
    }

    notify.rainToday = sensorGetCumulative (&work->sensors.sensor[STF_DAY][SENSOR_RAIN]);


    if (radMsgRouterMessageSend (WVIEW_MSG_TYPE_ARCHIVE_NOTIFY, &notify, sizeof(notify))
        == ERROR)
    {
        // can't send!
        radMsgLog (PRI_HIGH, "radMsgRouterMessageSend failed: notify");
        return ERROR;
    }

    return OK;
}

int stationProcessInfoResponses (WVIEWD_WORK *work)
{
    WVIEW_MSG_STATION_INFO  apath;

    if (!work->archiveRqstPending)
    {
        return OK;
    }

    apath.lastArcTime = work->archiveDateTime;
    apath.archiveInterval = work->archiveInterval;
    apath.latitude = work->latitude;
    apath.longitude = work->longitude;
    apath.elevation = work->elevation;
    if (! work->showStationIF)
    {
        sprintf(apath.stationType, "%s", work->stationType);
    }
    else
    {
        if (work->medium.type == MEDIUM_TYPE_USBHID)
        {
            sprintf(apath.stationType, "%s (USB)", work->stationType);
        }
        else if (work->medium.type == MEDIUM_TYPE_NONE)
        {
            sprintf(apath.stationType, "%s", work->stationType);
        }
        else if (!strcmp(work->stationInterface, "ethernet"))
        {
            sprintf(apath.stationType, "%s (%s:%d)",
                    work->stationType, work->stationHost, work->stationPort);
        }
        else
        {
            sprintf(apath.stationType, "%s (%s)",
                    work->stationType, work->stationDevice);
        }
    }

    if (radMsgRouterMessageSend (WVIEW_MSG_TYPE_STATION_INFO, &apath, sizeof (apath))
        == ERROR)
    {
        radMsgLog (PRI_HIGH, "radMsgRouterMessageSend failed Archive Path");
        return ERROR;
    }

    work->archiveRqstPending = FALSE;
    return OK;
}

int stationProcessIPM (WVIEWD_WORK *work, char *srcQueueName, int msgType, void *msg)
{
    WVIEW_MSG_REQUEST           *msgRqst;
    WVIEW_MSG_LOOP_DATA         loop;
    WVIEW_MSG_HILOW_DATA        hilow;
    WVIEW_MSG_ALERT*            alert;
    int                         retVal, i;

    switch (msgType)
    {
        case WVIEW_MSG_TYPE_REQUEST:
            msgRqst = (WVIEW_MSG_REQUEST *)msg;

            switch (msgRqst->requestType)
            {
                case WVIEW_RQST_TYPE_STATION_INFO:
                    // flag that a request has been tendered
                    work->archiveRqstPending = TRUE;

                    // are we currently in a serial cycle?
                    if (!work->runningFlag)
                    {
                        // yes, just bail out for now
                        return OK;
                    }
                    else
                    {
                        // no, send it now
                        return (stationProcessInfoResponses (work));
                    }

                case WVIEW_RQST_TYPE_LOOP_DATA:
                    loop.loopData = work->loopPkt;
                    if (loop.loopData.sampleET == ARCHIVE_VALUE_NULL)
                        loop.loopData.sampleET = 0;
                    if (loop.loopData.radiation == 0xFFFF)
                        loop.loopData.radiation = 0;
                    if (loop.loopData.UV < 0)
                        loop.loopData.UV = 0;
                    if (loop.loopData.rxCheckPercent == 0xFFFF)
                        loop.loopData.rxCheckPercent = 0;
                    if (loop.loopData.wxt510Hail == ARCHIVE_VALUE_NULL)
                        loop.loopData.wxt510Hail = 0;
                    if (loop.loopData.wxt510Hailrate == ARCHIVE_VALUE_NULL)
                        loop.loopData.wxt510Hailrate = 0;
                    if (loop.loopData.wxt510HeatingTemp == ARCHIVE_VALUE_NULL)
                        loop.loopData.wxt510HeatingTemp = 0;
                    if (loop.loopData.wxt510HeatingVoltage == ARCHIVE_VALUE_NULL)
                        loop.loopData.wxt510HeatingVoltage = 0;
                    if (loop.loopData.wxt510SupplyVoltage == ARCHIVE_VALUE_NULL)
                        loop.loopData.wxt510SupplyVoltage = 0;
                    if (loop.loopData.wxt510ReferenceVoltage == ARCHIVE_VALUE_NULL)
                        loop.loopData.wxt510ReferenceVoltage = 0;
                    if (loop.loopData.wxt510RainDuration == ARCHIVE_VALUE_NULL)
                        loop.loopData.wxt510RainDuration = 0;
                    if (loop.loopData.wxt510RainPeakRate == ARCHIVE_VALUE_NULL)
                        loop.loopData.wxt510RainPeakRate = 0;
                    if (loop.loopData.wxt510HailDuration == ARCHIVE_VALUE_NULL)
                        loop.loopData.wxt510HailDuration = 0;
                    if (loop.loopData.wxt510HailPeakRate == ARCHIVE_VALUE_NULL)
                        loop.loopData.wxt510HailPeakRate = 0;
                    if (loop.loopData.wxt510Rain == ARCHIVE_VALUE_NULL)
                        loop.loopData.wxt510Rain = 0;

                    if (radMsgRouterMessageSend (WVIEW_MSG_TYPE_LOOP_DATA,
                                                 &loop,
                                                 sizeof(loop))
                        == ERROR)
                    {
                        radMsgLog (PRI_HIGH, "radMsgRouterMessageSend failed LOOP");
                        return ERROR;
                    }

                    return OK;

                case WVIEW_RQST_TYPE_HILOW_DATA:
                    hilow.hilowData = work->sensors;
                    if (radMsgRouterMessageSend (WVIEW_MSG_TYPE_HILOW_DATA,
                                                 &hilow,
                                                 sizeof(hilow))
                        == ERROR)
                    {
                        radMsgLog (PRI_HIGH, "radMsgRouterMessageSend failed HILOW %d", sizeof(WVIEW_MSG_HILOW_DATA));
                        return ERROR;
                    }

                    return OK;
            }
            break;

        case WVIEW_MSG_TYPE_ALERT:
            alert = (WVIEW_MSG_ALERT*)msg;
            if (processAlertMessage(work, alert->alertType) == ERROR)
            {
                radMsgLog (PRI_HIGH, "Email Alert Send failed - are sendmail and sendEmail installed?");
            }
            break;

        default:
            // Pass it through to the station-specific function:
            stationMessageIndicate(work, msgType, msg);
            break;
    }

    return OK;
}

int stationPushDataToClients (WVIEWD_WORK *work)
{
    WVIEW_MSG_LOOP_DATA loop;

    if (!work->runningFlag)
    {
        return OK;
    }

    memcpy (&loop.loopData, &work->loopPkt, sizeof (loop.loopData));

    if (radMsgRouterMessageSend (WVIEW_MSG_TYPE_LOOP_DATA_SVC, &loop, sizeof(loop))
        == ERROR)
    {
        radMsgLog (PRI_HIGH, "radMsgRouterMessageSend failed for loop transmit!");
        return ERROR;
    }

    radProcessTimerStart (work->pushTimer, work->pushInterval);
    return OK;
}

int stationPushArchiveToClients(WVIEWD_WORK *work, ARCHIVE_PKT* pktToSend)
{
    WVIEW_MSG_ARCHIVE_DATA  arc;

    if (!work->runningFlag)
    {
        return OK;
    }

    memcpy (&arc.archiveData, pktToSend, sizeof (*pktToSend));

    if (radMsgRouterMessageSend (WVIEW_MSG_TYPE_ARCHIVE_DATA, &arc, sizeof(arc))
        == ERROR)
    {
        radMsgLog (PRI_HIGH, "radMsgRouterMessageSend failed for archive transmit!");
        return ERROR;
    }

    return OK;
}

void stationStartArchiveTimerUniform (WVIEWD_WORK *work)
{
    time_t              ntime;
    struct tm           currTime;
    int                 intSECS;

    // get the current time
    ntime = time (NULL);
    localtime_r (&ntime, &currTime);

    // get the next archive interval delta in seconds
    intSECS  = currTime.tm_min/work->archiveInterval;
    intSECS *= work->archiveInterval;
    intSECS += work->archiveInterval;
    intSECS -= currTime.tm_min;                     // delta in minutes
    intSECS *= 60;

    // we try to land 4 seconds after top-of-minute
    intSECS += (4 - currTime.tm_sec);

    // now we have the delta in seconds
    work->nextArchiveTime = ntime + intSECS;
    radTimerStart (work->archiveTimer, intSECS*1000);
}

void stationStartCDataTimerUniform (WVIEWD_WORK *work)
{
    time_t              ntime;
    struct tm           locTime;
    int                 moduloVal, cdataintSECS;

    ntime = time (NULL);
    localtime_r (&ntime, &locTime);
    cdataintSECS = work->cdataInterval/1000;

    // try to hit the start of next cdataInterval
    moduloVal = locTime.tm_sec % cdataintSECS;
    moduloVal = cdataintSECS - moduloVal;

    radTimerStart (work->cdataTimer, moduloVal*1000);
}

int stationStartSyncTimerUniform (WVIEWD_WORK *work, int firstTime)
{
    time_t              ntime;
    struct tm           locTime;
    int                 tempVal;

    ntime = time (NULL);
    localtime_r (&ntime, &locTime);
    locTime.tm_sec %= 60;

    if (firstTime)
    {
        // Make sure the sync timer does not track with the archive interval
        // by doing the sync at 2:30 of any 5 minute interval:
        tempVal = locTime.tm_min % 5;
        tempVal = 2 - tempVal;
        if (tempVal < 0)
        {
            tempVal += 5;
        }
        tempVal *= 60;                      // Make it seconds

        if (locTime.tm_sec <= 30)
        {
            tempVal += 30 - locTime.tm_sec;
        }
        else
        {
            if (tempVal == 0)
            {
                tempVal = 5 * 60;           // Add 5 minutes
            }
            tempVal -= (locTime.tm_sec - 30);
        }
        if (tempVal == 0)
        {
            tempVal = 1;
        }

        radTimerStart (work->syncTimer, tempVal * 1000);
        return FALSE;
    }

    if (locTime.tm_sec < 10)
    {
        // Correct:
        radTimerStart (work->syncTimer, ((30 - locTime.tm_sec) * 1000));
    }
    else if (locTime.tm_sec > 45)
    {
        radTimerStart (work->syncTimer, ((90 - locTime.tm_sec) * 1000));
    }
    else
    {
        // try to hit 30 secs into the minute
        tempVal = 30 - locTime.tm_sec;
        radTimerStart (work->syncTimer, WVD_TIME_SYNC_INTERVAL + (tempVal * 1000));
        return TRUE;
    }

    return FALSE;
}

int stationVerifyArchiveInterval (WVIEWD_WORK *work)
{
    ARCHIVE_PKT     tempRec;

    // sanity check the archive interval against the most recent record
    memset (&tempRec, 0, sizeof(tempRec));
    if ((int)dbsqliteArchiveGetNewestTime(&tempRec) == ERROR)
    {
        // there must be no archive records - just return OK
        return OK;
    }

    if ((int)tempRec.interval != (int)work->archiveInterval)
    {
        radMsgLog (PRI_HIGH,
                   "verifyArchiveInterval: station value of %d does NOT match archive value of %d",
                   work->archiveInterval, (int)tempRec.interval);
        return ERROR;
    }
    else
    {
        return OK;
    }
}

int stationGetConfigValueInt (WVIEWD_WORK *work, char *configName, int *store)
{
    int             value;

    wvconfigInit (FALSE);
    value = wvconfigGetINTValue (configName);
    wvconfigExit ();
    *store = value;
    return OK;
}

int stationGetConfigValueBoolean (WVIEWD_WORK *work, char *configName, int *store)
{
    int             value;

    wvconfigInit (FALSE);
    value = wvconfigGetBooleanValue(configName);
    wvconfigExit ();
    *store = value;
    return OK;
}

int stationGetConfigValueFloat (WVIEWD_WORK *work, char *configName, float *store)
{
    float           tempfloat;

    wvconfigInit (FALSE);
    tempfloat = wvconfigGetDOUBLEValue (configName);
    wvconfigExit ();

    tempfloat *= 100;
    if (tempfloat < 0.0)
        tempfloat -= 0.5;
    else
        tempfloat += 0.5;
    tempfloat /= 100;
    *store = tempfloat;
    return OK;
}

void stationClearLoopData (WVIEWD_WORK *work)
{
    work->loopPkt.sampleET                      = ARCHIVE_VALUE_NULL;
    work->loopPkt.radiation                     = 0xFFFF;
    work->loopPkt.UV                            = -1;
    work->loopPkt.rxCheckPercent                = 0xFFFF;
    work->loopPkt.wxt510Hail                    = ARCHIVE_VALUE_NULL;
    work->loopPkt.wxt510Hailrate                = ARCHIVE_VALUE_NULL;
    work->loopPkt.wxt510HeatingTemp             = ARCHIVE_VALUE_NULL;
    work->loopPkt.wxt510HeatingVoltage          = ARCHIVE_VALUE_NULL;
    work->loopPkt.wxt510SupplyVoltage           = ARCHIVE_VALUE_NULL;
    work->loopPkt.wxt510ReferenceVoltage        = ARCHIVE_VALUE_NULL;
    work->loopPkt.wxt510RainDuration            = ARCHIVE_VALUE_NULL;
    work->loopPkt.wxt510RainPeakRate            = ARCHIVE_VALUE_NULL;
    work->loopPkt.wxt510HailDuration            = ARCHIVE_VALUE_NULL;
    work->loopPkt.wxt510HailPeakRate            = ARCHIVE_VALUE_NULL;
    work->loopPkt.wxt510Rain                    = ARCHIVE_VALUE_NULL;
    work->loopPkt.wmr918WindBatteryStatus       = 0xFF;
    work->loopPkt.wmr918RainBatteryStatus       = 0xFF;
    work->loopPkt.wmr918OutTempBatteryStatus    = 0xFF;
    work->loopPkt.wmr918InTempBatteryStatus     = 0xFF;

    return;
}

