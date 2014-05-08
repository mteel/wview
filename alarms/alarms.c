/*---------------------------------------------------------------------------
 
  FILENAME:
        alarms.c
 
  PURPOSE:
        Provide the wview alarms generator entry point.
 
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
#include <dbsqlite.h>
#include <alarms.h>


/*  ... global memory declarations
*/

/*  ... global memory referenced
*/

/*  ... static (local) memory declarations
*/
static WVIEW_ALARM_WORK alarmsWork;

static char*            alarmsStatusLabels[STATUS_STATS_MAX] =
{
    "Alarms defined",
    "Alarm scripts invoked",
    "Datafeed clients",
    "Datafeed packets sent"
};


/* ... methods
*/

static int executeScript (WVIEW_ALARM *alarm)
{
    int             retVal;
    char            *args[5];
    char            path[_MAX_PATH];
    char            type[64];
    char            thresh[64];
    char            trigVal[64];

    wvstrncpy (path, alarm->scriptToRun, _MAX_PATH);
    args[0] = path;

    sprintf (type, "%d", alarm->type);
    args[1] = type;

    sprintf (thresh, "%.3f", alarm->bound);
    args[2] = thresh;

    sprintf (trigVal, "%.3f", alarm->triggerValue);
    args[3] = trigVal;

    args[4] = 0;

    retVal = fork ();
    if (retVal == -1)
    {
        return -1;
    }
    else if (retVal == 0)
    {
        // we are the child!
        if (execv (path, args) == -1)
        {
            radMsgLog(PRI_HIGH, "executeScript: execv(%s) failed: %s", path, strerror(errno));
            exit(-1);
        }
        exit(0);
    }
    else
    {
        // we are the parent, pause a bit to let the script run 
        // and avoid a process "storm"
        radUtilsSleep (50);                 // 50 ms
    }

    return 0;
}

static int readAlarmsConfig (void)
{
    WVIEW_ALARM     *alarm;
    int             i, numAlarms = 0;
    int             iValue;
    char            type[_MAX_PATH];
    int             boolMAX;
    double          thresh;
    int             abate;
    char            exec[_MAX_PATH];
    char            conftype[64];
    const char*     temp;

    if (wvconfigInit(FALSE) == ERROR)
    {
        radMsgLog (PRI_CATASTROPHIC, "wvconfigInit failed!");
        return ERROR;
    }

    // Is the alarms daemon enabled?
    iValue = wvconfigGetBooleanValue(configItem_ENABLE_ALARMS);
    if (iValue == ERROR || iValue == 0)
    {
        wvconfigExit ();
        return ERROR_ABORT;
    }

    // set the wview verbosity setting
    if (wvutilsSetVerbosity (WV_VERBOSE_WVALARMD) == ERROR)
    {
        radMsgLog (PRI_CATASTROPHIC, "wvutilsSetVerbosity failed!");
        wvconfigExit ();
        return ERROR_ABORT;
    }

    // get the metric units flag:
    iValue = wvconfigGetBooleanValue(configItem_ALARMS_STATION_METRIC);
    if (iValue == ERROR)
    {
        alarmsWork.isMetric = 0;
    }
    else
    {
        alarmsWork.isMetric = iValue;
    }

    // get the do test flag:
    iValue = wvconfigGetBooleanValue(configItem_ALARMS_DO_TEST);
    if (iValue <= 0)
    {
        alarmsWork.doTest = FALSE;
    }
    else
    {
        alarmsWork.doTest = TRUE;
        alarmsWork.doTestNumber = wvconfigGetINTValue(configItem_ALARMS_DO_TEST_NUMBER);
    }

    for (i = 1; i <= ALARMS_MAX; i ++)
    {
        sprintf (conftype, "ALARMS_%1.1d_TYPE", i);
        temp = wvconfigGetStringValue(conftype);
        if (temp == NULL)
        {
            // No type defined - continue:
            continue;
        }
        wvstrncpy(type, temp, _MAX_PATH);

        sprintf (conftype, "ALARMS_%1.1d_MAX", i);
        boolMAX = wvconfigGetBooleanValue(conftype);
        if (boolMAX == ERROR)
        {
            continue;
        }

        sprintf (conftype, "ALARMS_%1.1d_THRESHOLD", i);
        thresh = wvconfigGetDOUBLEValue(conftype);

        sprintf (conftype, "ALARMS_%1.1d_ABATEMENT", i);
        abate = wvconfigGetINTValue(conftype);

        sprintf (conftype, "ALARMS_%1.1d_EXECUTE", i);
        temp = wvconfigGetStringValue(conftype);
        if (temp == NULL)
        {
            // No type defined - continue:
            continue;
        }
        wvstrncpy(exec, temp, _MAX_PATH);

        alarm = (WVIEW_ALARM *) malloc (sizeof (*alarm));
        if (alarm == NULL)
        {
            for (alarm = (WVIEW_ALARM *)radListRemoveFirst (&alarmsWork.alarmList);
                 alarm != NULL;
                 alarm = (WVIEW_ALARM *)radListRemoveFirst (&alarmsWork.alarmList))
            {
                free (alarm);
            }

            return ERROR;
        }
        memset (alarm, 0, sizeof (*alarm));


        // get the type
        if (!strcmp (type, "Barometer"))
        {
            alarm->type = Barometer;
        }
        else if (!strcmp (type, "InsideTemp"))
        {
            alarm->type = InsideTemp;
        }
        else if (!strcmp (type, "InsideHumidity"))
        {
            alarm->type = InsideHumidity;
        }
        else if (!strcmp (type, "OutsideTemp"))
        {
            alarm->type = OutsideTemp;
        }
        else if (!strcmp (type, "WindSpeed"))
        {
            alarm->type = WindSpeed;
        }
        else if (!strcmp (type, "TenMinuteAvgWindSpeed"))
        {
            alarm->type = TenMinuteAvgWindSpeed;
        }
        else if (!strcmp (type, "WindDirection"))
        {
            alarm->type = WindDirection;
        }
        else if (!strcmp (type, "OutsideHumidity"))
        {
            alarm->type = OutsideHumidity;
        }
        else if (!strcmp (type, "RainRate"))
        {
            alarm->type = RainRate;
        }
        else if (!strcmp (type, "StormRain"))
        {
            alarm->type = StormRain;
        }
        else if (!strcmp (type, "DayRain"))
        {
            alarm->type = DayRain;
        }
        else if (!strcmp (type, "MonthRain"))
        {
            alarm->type = MonthRain;
        }
        else if (!strcmp (type, "YearRain"))
        {
            alarm->type = YearRain;
        }
        else if (!strcmp (type, "TxBatteryStatus"))
        {
            alarm->type = TxBatteryStatus;
        }
        else if (!strcmp (type, "ConsoleBatteryVoltage"))
        {
            alarm->type = ConsoleBatteryVoltage;
        }
        else if (!strcmp (type, "DewPoint"))
        {
            alarm->type = DewPoint;
        }
        else if (!strcmp (type, "WindChill"))
        {
            alarm->type = WindChill;
        }
        else if (!strcmp (type, "HeatIndex"))
        {
            alarm->type = HeatIndex;
        }
        else if (!strcmp (type, "Radiation"))
        {
            alarm->type = Radiation;
        }
        else if (!strcmp (type, "UV"))
        {
            alarm->type = UV;
        }
        else if (!strcmp (type, "ET"))
        {
            alarm->type = ET;
        }
        else if (!strcmp (type, "ExtraTemp1"))
        {
            alarm->type = ExtraTemp1;
        }
        else if (!strcmp (type, "ExtraTemp2"))
        {
            alarm->type = ExtraTemp2;
        }
        else if (!strcmp (type, "ExtraTemp3"))
        {
            alarm->type = ExtraTemp3;
        }
        else if (!strcmp (type, "SoilTemp1"))
        {
            alarm->type = SoilTemp1;
        }
        else if (!strcmp (type, "SoilTemp2"))
        {
            alarm->type = SoilTemp2;
        }
        else if (!strcmp (type, "SoilTemp3"))
        {
            alarm->type = SoilTemp3;
        }
        else if (!strcmp (type, "SoilTemp4"))
        {
            alarm->type = SoilTemp4;
        }
        else if (!strcmp (type, "LeafTemp1"))
        {
            alarm->type = LeafTemp1;
        }
        else if (!strcmp (type, "LeafTemp2"))
        {
            alarm->type = LeafTemp2;
        }
        else if (!strcmp (type, "ExtraHumid1"))
        {
            alarm->type = ExtraHumid1;
        }
        else if (!strcmp (type, "ExtraHumid2"))
        {
            alarm->type = ExtraHumid2;
        }
        else if (!strcmp (type, "Wxt510Hail"))
        {
            alarm->type = Wxt510Hail;
        }
        else if (!strcmp (type, "Wxt510Hailrate"))
        {
            alarm->type = Wxt510Hailrate;
        }
        else if (!strcmp (type, "Wxt510HeatingTemp"))
        {
            alarm->type = Wxt510HeatingTemp;
        }
        else if (!strcmp (type, "Wxt510HeatingVoltage"))
        {
            alarm->type = Wxt510HeatingVoltage;
        }
        else if (!strcmp (type, "Wxt510SupplyVoltage"))
        {
            alarm->type = Wxt510SupplyVoltage;
        }
        else if (!strcmp (type, "Wxt510ReferenceVoltage"))
        {
            alarm->type = Wxt510ReferenceVoltage;
        }
        else
        {
            free (alarm);
            radMsgLog (PRI_MEDIUM, "invalid alarm type %s - skipping...",
                       type);
            continue;
        }

        //  do the max/min flag
        alarm->isMax = boolMAX;

        //  now the bounding value
        alarm->bound = (float)thresh;

        //  now the abatement seconds
        alarm->abateSecs = abate;

        //  finally, the alarm script
        wvstrncpy (alarm->scriptToRun, exec, WVIEW_ALARM_SCRIPT_LENGTH);

        radListAddToEnd (&alarmsWork.alarmList, (NODE_PTR)alarm);
    }

    wvconfigExit ();
    return (radListGetNumberOfNodes (&alarmsWork.alarmList));
}

static int waitForWviewDaemon (void)
{
    WVIEW_MSG_REQUEST   msg;
    char                srcQName[QUEUE_NAME_LENGTH+1];
    UINT                msgType;
    UINT                length;
    void                *recvBfr;
    int                 retVal, done = FALSE;

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
            radMsgLog (PRI_STATUS, "waitForWviewDaemon: queue is closed!");
            statusUpdateMessage("waitForWviewDaemon: queue is closed!");
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
            statusUpdateMessage("waitForWviewDaemon: received shutdown from wviewd");
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

static int WriteArchiveToClient(RADSOCK_ID client, ARCHIVE_PKT* archive)
{
    ARCHIVE_PKT         dummy;

    // write the frame start so clients may sync to the beginning of each
    // data update
    if (radSocketWriteExact(client, 
                            (void *)DF_ARCHIVE_START_FRAME, 
                            DF_START_FRAME_LENGTH)
        != DF_START_FRAME_LENGTH)
    {
        return ERROR;
    }

    // If the rec is NULL, send a done indicator:
    if (archive == NULL)
    {
        dummy.dateTime = 0;
        radSocketWriteExact(client, &dummy, sizeof(dummy));
    }
    else
    {
        // write out the archive data in network byte order:
        if (radSocketWriteExact(client, archive, sizeof(*archive)) != sizeof(*archive))
        {
            return ERROR;
        }
    }

    statusIncrementStat(ALARM_STATS_PKTS_SENT);
    return OK;
}

static void pushArchiveToClients(ARCHIVE_PKT* archive)
{
    WVIEW_ALARM_CLIENT  *client, *oldClient;
    ARCHIVE_PKT         networkArchive;

    datafeedConvertArchive_HTON(&networkArchive, archive);

    // Push to each socket client:
    for (client = (WVIEW_ALARM_CLIENT *) radListGetFirst (&alarmsWork.clientList);
         client != NULL;
         client = (WVIEW_ALARM_CLIENT *) radListGetNext (&alarmsWork.clientList, 
                                                         (NODE_PTR)client))
    {
        // Check for archive sync in progress:
        if (client->syncInProgress)
        {
            continue;
        }

        // Write start frame and archive packet on the socket:
        if (WriteArchiveToClient(client->client, &networkArchive) == ERROR)
        {
            // write error, bail on this guy
            radMsgLog (PRI_HIGH, "ARCHIVE: write error to client %s:%d - closing socket...",
                       radSocketGetHost (client->client),
                       radSocketGetPort (client->client));
            statusDecrementStat(ALARM_STATS_CLIENTS);
            radProcessIODeRegisterDescriptorByFd(radSocketGetDescriptor(client->client));
            radSocketDestroy (client->client);
            oldClient = client;
            client = (WVIEW_ALARM_CLIENT *) 
                        radListGetPrevious (&alarmsWork.clientList, (NODE_PTR)client);
            radListRemove (&alarmsWork.clientList, (NODE_PTR)oldClient);
            free (oldClient);
            continue;
        }
    }

    return;
}

static void pushLoopToClients(LOOP_PKT* loopData)
{
    WVIEW_ALARM_CLIENT  *client, *oldClient;
    LOOP_PKT            networkLoop;

    datafeedConvertLOOP_HTON(&networkLoop, loopData);

    // Push to each socket client:
    for (client = (WVIEW_ALARM_CLIENT *) radListGetFirst (&alarmsWork.clientList);
         client != NULL;
         client = (WVIEW_ALARM_CLIENT *) radListGetNext (&alarmsWork.clientList, 
                                                         (NODE_PTR)client))
    {
        // write the frame start so clients may sync to the beginning of each
        // data update
        if (radSocketWriteExact (client->client, 
                                 (void *)DF_LOOP_START_FRAME, 
                                 DF_START_FRAME_LENGTH)
            != DF_START_FRAME_LENGTH)
        {
            // write error, bail on this guy
            radMsgLog (PRI_HIGH, "LOOP: write error to client %s:%d - closing socket...",
                       radSocketGetHost (client->client),
                       radSocketGetPort (client->client));
            statusDecrementStat(ALARM_STATS_CLIENTS);
            radProcessIODeRegisterDescriptorByFd(radSocketGetDescriptor(client->client));
            radSocketDestroy (client->client);
            oldClient = client;
            client = (WVIEW_ALARM_CLIENT *) 
                        radListGetPrevious (&alarmsWork.clientList, (NODE_PTR)client);
            radListRemove (&alarmsWork.clientList, (NODE_PTR)oldClient);
            free (oldClient);
            continue;
        }

        // write out the loop data in network byte order:
        if (radSocketWriteExact(client->client, &networkLoop, sizeof(networkLoop))
            != sizeof(networkLoop))
        {
            // write error, bail on this guy
            radMsgLog (PRI_HIGH, "LOOP: write error to client %s:%d - closing socket...",
                       radSocketGetHost (client->client),
                       radSocketGetPort (client->client));
            statusDecrementStat(ALARM_STATS_CLIENTS);
            radProcessIODeRegisterDescriptorByFd(radSocketGetDescriptor(client->client));
            radSocketDestroy (client->client);
            oldClient = client;
            client = (WVIEW_ALARM_CLIENT *) 
                        radListGetPrevious (&alarmsWork.clientList, (NODE_PTR)client);
            radListRemove (&alarmsWork.clientList, (NODE_PTR)oldClient);
            free (oldClient);
            continue;
        }

        statusIncrementStat(ALARM_STATS_PKTS_SENT);
    }

    return;
}

static void processAlarms (LOOP_PKT *loopData)
{
    WVIEW_ALARM         *alarm;
    float               tempFloat;

    // process the local alarms:
    for (alarm = (WVIEW_ALARM *) radListGetFirst (&alarmsWork.alarmList);
         alarm != NULL;
         alarm = (WVIEW_ALARM *) radListGetNext (&alarmsWork.alarmList, 
                                                 (NODE_PTR)alarm))
    {
        // first check to see if we are in abatement
        if (alarm->triggered)
        {
            if ((radTimeGetSECSinceEpoch () - alarm->abateStart) < alarm->abateSecs)
            {
                // abatement - go to the next alarm
                continue;
            }
            else
            {
                // clear trigger for future alarms
                alarm->triggered = FALSE;
            }
        }

        // switch on alarm type
        switch (alarm->type)
        {
            case Barometer:
                if (alarmsWork.isMetric)
                {
                    tempFloat = wvutilsConvertINHGToHPA(loopData->barometer);
                }
                else
                {
                    tempFloat = loopData->barometer;
                }
                break;

            case InsideTemp:
                if (alarmsWork.isMetric)
                {
                    tempFloat = wvutilsConvertFToC(loopData->inTemp);
                }
                else
                {
                    tempFloat = loopData->inTemp;
                }
                break;

            case InsideHumidity:
                tempFloat = (float)loopData->inHumidity;
                break;

            case OutsideTemp:
                if (alarmsWork.isMetric)
                {
                    tempFloat = wvutilsConvertFToC(loopData->outTemp);
                }
                else
                {
                    tempFloat = loopData->outTemp;
                }
                break;

            case WindSpeed:
                if (alarmsWork.isMetric)
                {
                    tempFloat = wvutilsConvertMPHToKPH(loopData->windSpeedF);
                }
                else
                {
                    tempFloat = loopData->windSpeedF;
                }
                break;

            case TenMinuteAvgWindSpeed:
                if (alarmsWork.isMetric)
                {
                    tempFloat = wvutilsConvertMPHToKPH((float)loopData->tenMinuteAvgWindSpeed);
                }
                else
                {
                    tempFloat = (float)loopData->tenMinuteAvgWindSpeed;
                }
                break;

            case WindDirection:
                tempFloat = (float)loopData->windDir;
                break;

            case OutsideHumidity:
                tempFloat = (float)loopData->outHumidity;
                break;

            case RainRate:
                if (alarmsWork.isMetric)
                {
                    tempFloat = wvutilsConvertRainINToMetric(loopData->rainRate);
                }
                else
                {
                    tempFloat = loopData->rainRate;
                }
                break;

            case StormRain:
                if (alarmsWork.isMetric)
                {
                    tempFloat = wvutilsConvertRainINToMetric(loopData->stormRain);
                }
                else
                {
                    tempFloat = loopData->stormRain;
                }
                break;

            case DayRain:
                if (alarmsWork.isMetric)
                {
                    tempFloat = wvutilsConvertRainINToMetric(loopData->dayRain);
                }
                else
                {
                    tempFloat = loopData->dayRain;
                }
                break;

            case MonthRain:
                if (alarmsWork.isMetric)
                {
                    tempFloat = wvutilsConvertRainINToMetric(loopData->monthRain);
                }
                else
                {
                    tempFloat = loopData->monthRain;
                }
                break;

            case YearRain:
                if (alarmsWork.isMetric)
                {
                    tempFloat = wvutilsConvertRainINToMetric(loopData->yearRain);
                }
                else
                {
                    tempFloat = loopData->yearRain;
                }
                break;

            case TxBatteryStatus:
                tempFloat = (float)loopData->txBatteryStatus;
                break;

            case ConsoleBatteryVoltage:
                tempFloat = (((float)loopData->consBatteryVoltage * 300)/512)/100;
                break;

            case DewPoint:
                if (alarmsWork.isMetric)
                {
                    tempFloat = wvutilsConvertFToC(loopData->dewpoint);
                }
                else
                {
                    tempFloat = loopData->dewpoint;
                }
                break;

            case WindChill:
                if (alarmsWork.isMetric)
                {
                    tempFloat = wvutilsConvertFToC(loopData->windchill);
                }
                else
                {
                    tempFloat = loopData->windchill;
                }
                break;

            case HeatIndex:
                if (alarmsWork.isMetric)
                {
                    tempFloat = wvutilsConvertFToC(loopData->heatindex);
                }
                else
                {
                    tempFloat = loopData->heatindex;
                }
                break;

            case Radiation:
                tempFloat = (float)loopData->radiation;
                break;

            case UV:
                tempFloat = (float)loopData->UV;
                break;

            case ET:
                if (alarmsWork.isMetric)
                {
                    tempFloat = wvutilsConvertRainINToMetric(loopData->dayET);
                }
                else
                {
                    tempFloat = loopData->dayET;
                }
                break;

            case ExtraTemp1:
                if (alarmsWork.isMetric)
                {
                    tempFloat = wvutilsConvertFToC(loopData->extraTemp[0]);
                }
                else
                {
                    tempFloat = loopData->extraTemp[0];
                }
                break;

            case ExtraTemp2:
                if (alarmsWork.isMetric)
                {
                    tempFloat = wvutilsConvertFToC(loopData->extraTemp[1]);
                }
                else
                {
                    tempFloat = loopData->extraTemp[1];
                }
                break;

            case ExtraTemp3:
                if (alarmsWork.isMetric)
                {
                    tempFloat = wvutilsConvertFToC(loopData->extraTemp[2]);
                }
                else
                {
                    tempFloat = loopData->extraTemp[2];
                }
                break;

            case SoilTemp1:
                if (alarmsWork.isMetric)
                {
                    tempFloat = wvutilsConvertFToC(loopData->soilTemp1);
                }
                else
                {
                    tempFloat = loopData->soilTemp1;
                }
                break;

            case SoilTemp2:
                if (alarmsWork.isMetric)
                {
                    tempFloat = wvutilsConvertFToC(loopData->soilTemp2);
                }
                else
                {
                    tempFloat = loopData->soilTemp2;
                }
                break;

            case SoilTemp3:
                if (alarmsWork.isMetric)
                {
                    tempFloat = wvutilsConvertFToC(loopData->soilTemp3);
                }
                else
                {
                    tempFloat = loopData->soilTemp3;
                }
                break;

            case SoilTemp4:
                if (alarmsWork.isMetric)
                {
                    tempFloat = wvutilsConvertFToC(loopData->soilTemp4);
                }
                else
                {
                    tempFloat = loopData->soilTemp4;
                }
                break;

            case LeafTemp1:
                if (alarmsWork.isMetric)
                {
                    tempFloat = wvutilsConvertFToC(loopData->leafTemp1);
                }
                else
                {
                    tempFloat = loopData->leafTemp1;
                }
                break;

            case LeafTemp2:
                if (alarmsWork.isMetric)
                {
                    tempFloat = wvutilsConvertFToC(loopData->leafTemp2);
                }
                else
                {
                    tempFloat = loopData->leafTemp2;
                }
                break;

            case ExtraHumid1:
                tempFloat = (float)loopData->extraHumidity[0];
                break;

            case ExtraHumid2:
                tempFloat = (float)loopData->extraHumidity[1];
                break;

            case Wxt510Hail:
                if (alarmsWork.isMetric)
                {
                    tempFloat = wvutilsConvertRainINToMetric(loopData->wxt510Hail);
                }
                else
                {
                    tempFloat = loopData->wxt510Hail;
                }
                break;

            case Wxt510Hailrate:
                if (alarmsWork.isMetric)
                {
                    tempFloat = wvutilsConvertRainINToMetric(loopData->wxt510Hailrate);
                }
                else
                {
                    tempFloat = loopData->wxt510Hailrate;
                }
                break;

            case Wxt510HeatingTemp:
                if (alarmsWork.isMetric)
                {
                    tempFloat = wvutilsConvertFToC(loopData->wxt510HeatingTemp);
                }
                else
                {
                    tempFloat = loopData->wxt510HeatingTemp;
                }
                break;

            case Wxt510HeatingVoltage:
                tempFloat = loopData->wxt510HeatingVoltage;
                break;

            case Wxt510SupplyVoltage:
                tempFloat = loopData->wxt510SupplyVoltage;
                break;

            case Wxt510ReferenceVoltage:
                tempFloat = loopData->wxt510ReferenceVoltage;
                break;

            default:
                // no match, blow it off
                continue;
        }

        // see if we tripped the breaker here
        if (alarm->isMax)
        {
            if (tempFloat >= alarm->bound)
            {
                // we did!
                alarm->triggered = TRUE;
                alarm->triggerValue = tempFloat;
                alarm->abateStart = radTimeGetSECSinceEpoch ();

                // run user script here
                statusIncrementStat(ALARM_STATS_SCRIPTS_RUN);
                if (executeScript (alarm) != 0)
                {
                    radMsgLog (PRI_MEDIUM, 
                               "processAlarms: script %s failed",
                               alarm->scriptToRun);
                }
            }
        }
        else
        {
            if (tempFloat <= alarm->bound)
            {
                // we did!
                alarm->triggered = TRUE;
                alarm->triggerValue = tempFloat;
                alarm->abateStart = radTimeGetSECSinceEpoch ();

                // run user script here
                statusIncrementStat(ALARM_STATS_SCRIPTS_RUN);
                if (executeScript (alarm) != 0)
                {
                    radMsgLog (PRI_MEDIUM, 
                               "processAlarms: script %s failed",
                               alarm->scriptToRun);
                }
            }
        }
    }

    return;
}

/*  ... system initialization
*/
static int alarmsSysInit (WVIEW_ALARM_WORK *work)
{
    char            devPath[256], temp[64];
    struct stat     fileData;

    /*  ... check for our daemon's pid file, don't run if it isn't there
    */
    sprintf (devPath, "%s/%s", WVIEW_RUN_DIR, WVD_LOCK_FILE_NAME);
    if (stat (devPath, &fileData) != 0)
    {
        radMsgLogInit (PROC_NAME_ALARMS, TRUE, TRUE);
        radMsgLog (PRI_CATASTROPHIC, 
                   "wviewd process is not running - aborting!");
        radMsgLogExit ();
        return ERROR;
    }

    sprintf (work->pidFile, "%s/%s", WVIEW_RUN_DIR, ALARMS_LOCK_FILE_NAME);
    sprintf (work->statusFile, "%s/%s", WVIEW_STATUS_DIRECTORY, ALARMS_STATUS_FILE_NAME);
    sprintf (work->fifoFile, "%s/dev/%s", WVIEW_RUN_DIR, PROC_NAME_ALARMS);
    sprintf (work->daemonQname, "%s/dev/%s", WVIEW_RUN_DIR, PROC_NAME_DAEMON);
    sprintf (work->wviewdir, "%s", WVIEW_RUN_DIR);

    /*  ... check for our pid file, don't run if it IS there
    */
    if (stat (work->pidFile, &fileData) == 0)
    {
        radMsgLogInit (PROC_NAME_ALARMS, TRUE, TRUE);
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
static int alarmsSysExit (WVIEW_ALARM_WORK *work)
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
            radMsgLog (PRI_STATUS, "wvalarmd: SIGHUP - toggling log verbosity %s",
                       ((retVal == 0) ? "OFF" : "ON"));

            radProcessSignalCatch(signum, defaultSigHandler);
            return;

        case SIGPIPE:
            // we have a far end socket disconnection, we'll handle it in the
            // "read/write" code
            alarmsWork.sigpipe = TRUE;
            radProcessSignalCatch(signum, defaultSigHandler);
            break;

        case SIGCHLD:
            wvutilsWaitForChildren();
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
                       "wvalarmd: recv catastrophic signal %d: aborting!", 
                       signum);
            abort ();

        default:
            // can we allow the process to exit normally?
            if (!alarmsWork.inMainLoop)
            {
                // NO! - we gotta bail here!
                statusUpdateMessage("wvalarmd: recv signal exiting now!");
                radMsgLog (PRI_HIGH, "wvalarmd: recv signal %d: exiting now!", signum);
                if (alarmsWork.dataFeedServer)
                    radSocketDestroy (alarmsWork.dataFeedServer);

                radMsgRouterExit ();
                alarmsSysExit (&alarmsWork);
                radProcessExit ();
                radSystemExit (WVIEW_SYSTEM_ID);
                exit (1);                
            }

            // we can allow the process to exit normally...
            statusUpdateMessage("wvalarmd: recv catastrophic signal exiting gracefully!");
            radMsgLog (PRI_HIGH, "wvalarmd: recv signal %d: exiting gracefully!", signum);

            alarmsWork.exiting = TRUE;
            radProcessSetExitFlag ();

            radProcessSignalCatch(signum, defaultSigHandler);
            break;
    }

    return;
}

static void msgHandler
(
    char                    *srcQueueName,
    UINT                    msgType,
    void                    *msg,
    UINT                    length,
    void                    *userData
)
{
    LOOP_PKT*               loopData;
    ARCHIVE_PKT*            archiveData;

    switch (msgType)
    {
        case WVIEW_MSG_TYPE_LOOP_DATA_SVC:
        {
            loopData = &(((WVIEW_MSG_LOOP_DATA*)msg)->loopData);

            // Push LOOP to clients:
            pushLoopToClients(loopData);

            // process alarms
            processAlarms(loopData);
            break;
        }
        case WVIEW_MSG_TYPE_ARCHIVE_DATA:
        {
            archiveData = &(((WVIEW_MSG_ARCHIVE_DATA*)msg)->archiveData);

            // Push Archive to clients:
            pushArchiveToClients(archiveData);
            break;
        }
        case WVIEW_MSG_TYPE_POLL:
        {
            WVIEW_MSG_POLL*     pPoll = (WVIEW_MSG_POLL*)msg;
            wvutilsSendPMONPollResponse (pPoll->mask, PMON_PROCESS_WVALARMD);
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
    return;
}

static WVIEW_ALARM_CLIENT* FindClient(RADSOCK_ID clientSock)
{
    WVIEW_ALARM_CLIENT*     node;

    for (node = (WVIEW_ALARM_CLIENT*)radListGetFirst(&alarmsWork.clientList);
         node != NULL;
         node = (WVIEW_ALARM_CLIENT*)radListGetNext(&alarmsWork.clientList, (NODE_PTR)node))
    {
        if (node->client == clientSock)
        {
            // found him!
            return node;
        }
    }

    return NULL;
}

static void RemoveClient(RADSOCK_ID clientSock)
{
    WVIEW_ALARM_CLIENT*     node;

    for (node = (WVIEW_ALARM_CLIENT*)radListGetFirst(&alarmsWork.clientList);
         node != NULL;
         node = (WVIEW_ALARM_CLIENT*)radListGetNext(&alarmsWork.clientList, (NODE_PTR)node))
    {
        if (node->client == clientSock)
        {
            // found him!
            statusDecrementStat(ALARM_STATS_CLIENTS);
            radListRemove(&alarmsWork.clientList, (NODE_PTR)node);
            radProcessIODeRegisterDescriptorByFd(radSocketGetDescriptor(node->client));
            radSocketDestroy(node->client);
            free(node);
            return;
        }
    }
}

static void SendNextArchiveRecord(RADSOCK_ID client, uint32_t dateTime)
{
    ARCHIVE_PKT         recordStore;
    ARCHIVE_PKT         networkStore;
    WVIEW_ALARM_CLIENT* alarmClient;

    alarmClient = FindClient(client);
    if (alarmClient == NULL)
    {
        radMsgLog (PRI_HIGH, "SendNextArchiveRecord: failed to get client!");
        return;
    }

    if (dbsqliteArchiveInit() == ERROR)
    {
        radMsgLog (PRI_HIGH, "SendNextArchiveRecord: failed to open archive db!");
        return;
    }

    if (dbsqliteArchiveGetNextRecord((time_t)dateTime, &recordStore) == ERROR)
    {
        WriteArchiveToClient(client, NULL);
        alarmClient->syncInProgress = FALSE;
        return;
    }

    dbsqliteArchiveExit();


    // Mark the sync in progress:
    alarmClient->syncInProgress = TRUE;

    // OK, send the bloody thing:
    datafeedConvertArchive_HTON(&networkStore, &recordStore);
    if (WriteArchiveToClient(client, &networkStore) == ERROR)
    {
        statusUpdateMessage("SendNextArchiveRecord: failed to write archive record!");
        radMsgLog (PRI_HIGH, "SendNextArchiveRecord: failed to write archive record!");
        return;
    }

    return;
}

static void ClientDataRX (int fd, void *userData)
{
    RADSOCK_ID          client = (RADSOCK_ID)userData;
    int                 retVal;
    uint32_t            dateTime;

    retVal = datafeedSyncStartOfFrame(client);
    switch (retVal)
    {
        case ERROR:
            /* problems! - bail out */
            statusUpdateMessage("ClientDataRX: socket error during sync - disconnecting");
            radMsgLog (PRI_HIGH, "ClientDataRX: socket error during sync - disconnecting");
            RemoveClient(client);
            break;

        case ERROR_ABORT:
            // This guy has bailed out:
            statusUpdateMessage("ClientDataRX: socket far-end closed");
            radMsgLog (PRI_MEDIUM, "ClientDataRX: socket far-end closed");
            RemoveClient(client);
            break;
            
        case FALSE:
            radMsgLog (PRI_STATUS, "ClientDataRX: RX sync failure - ignoring");
            break;
    
        case DF_RQST_ARCHIVE_PKT_TYPE:
            // OK, read the unix time sent to retrieve the record:
            if (radSocketReadExact(client, (void *)&dateTime, sizeof(dateTime)) 
                != sizeof (dateTime))
            {
                statusUpdateMessage("ClientDataRX: socket read error - disconnecting");
                radMsgLog (PRI_HIGH, "ClientDataRX: socket read error - disconnecting");
                RemoveClient(client);
                break;
            }
    
            // Convert from network byte order:
            dateTime = ntohl(dateTime);
    
            // Now we have the date and time, get busy:
            SendNextArchiveRecord(client, dateTime);
            break;

        default:
            statusUpdateMessage("ClientDataRX: socket error during sync - disconnecting");
            radMsgLog (PRI_HIGH, "ClientDataRX: socket error during sync - disconnecting");
            RemoveClient(client);
            break;
    }

    return;
}

static void dataFeedAccept (int fd, void *userData)
{
    RADSOCK_ID          newConnection;
    WVIEW_ALARM_CLIENT  *client;

    newConnection = radSocketServerAcceptConnection(alarmsWork.dataFeedServer);
    if (newConnection == NULL)
    {
        statusUpdateMessage("dataFeed: accept connection failed!");
        radMsgLog (PRI_MEDIUM, "dataFeed: accept connection failed!");
        return;
    }

    // stick him on the data feed client list
    client = (WVIEW_ALARM_CLIENT *) malloc(sizeof(*client));
    if (client == NULL)
    {
        radMsgLog (PRI_MEDIUM, "dataFeedAccept: malloc failed!");
        radSocketDestroy(newConnection);
        return;
    }
    memset(client, 0, sizeof (*client));

    client->client = newConnection;

    radSocketSetBlocking(client->client, TRUE);

    // add it to our descriptors of interest:
    if (radProcessIORegisterDescriptor(radSocketGetDescriptor(client->client),
                                       ClientDataRX,
                                       (void*)client->client)
        == ERROR)
    {
        statusUpdateMessage("dataFeedAccept: register descriptor failed!");
        radMsgLog (PRI_MEDIUM, "dataFeedAccept: register descriptor failed!");
        radSocketDestroy(client->client);
        return;
    }

    radListAddToEnd(&alarmsWork.clientList, (NODE_PTR)client);

    statusIncrementStat(ALARM_STATS_CLIENTS);
    radMsgLog (PRI_STATUS, "dataFeed: client %s:%d accepted...",
               radSocketGetHost (client->client),
               radSocketGetPort (client->client));

    return;
}


/*  ... the main entry point for the alarm process
*/
int main (int argc, char *argv[])
{
    void            (*alarmHandler)(int);
    int             retVal;
    FILE            *pidfile;
    WVIEW_ALARM     *alarm;
    int             runAsDaemon = TRUE;

    if (argc > 1)
    {
        if (!strcmp(argv[1], "-f"))
        {
            runAsDaemon = FALSE;
        }
    }

    memset (&alarmsWork, 0, sizeof (alarmsWork));
    radListReset (&alarmsWork.alarmList);
    radListReset (&alarmsWork.clientList);

    /*  ... initialize some system stuff first
    */
    retVal = alarmsSysInit (&alarmsWork);
    if (retVal == ERROR)
    {
        radMsgLogInit (PROC_NAME_ALARMS, FALSE, TRUE);
        radMsgLog (PRI_CATASTROPHIC, "wvalarmd sysinit failed!");
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
        radMsgLogInit (PROC_NAME_ALARMS, TRUE, TRUE);
        radMsgLog (PRI_CATASTROPHIC, "radSystemInit failed!");
        radMsgLogExit ();
        exit (1);
    }


    /*  ... call the radlib process init function
    */
    if (radProcessInit (PROC_NAME_ALARMS,
                        alarmsWork.fifoFile,
                        PROC_NUM_TIMERS_ALARMS,
                        runAsDaemon,                // TRUE for daemon
                        msgHandler,
                        evtHandler,
                        NULL)
        == ERROR)
    {
        printf ("\nradProcessInit failed: %s\n\n", PROC_NAME_ALARMS);
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }

    alarmsWork.myPid = getpid ();
    pidfile = fopen (alarmsWork.pidFile, "w");
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


    // grab all of our alarm definitions from the config database
    retVal = readAlarmsConfig ();
    if (retVal == ERROR_ABORT)
    {
        radMsgLog (PRI_HIGH, "ALARM daemon not enabled - exiting...");
        alarmsSysExit (&alarmsWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (0);
    }
    else if (retVal < 0)
    {
        radMsgLog (PRI_HIGH, "readAlarmsConfig failed - "
                   "is there a problem with the wview config database?");
        alarmsSysExit (&alarmsWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }
    else
    {
        radMsgLog (PRI_STATUS, 
                   "alarms: added %d alarm definitions",
                   retVal);
    }

    if (statusInit(alarmsWork.statusFile, alarmsStatusLabels) == ERROR)
    {
        radMsgLog (PRI_HIGH, "ALARM status init failed - exiting...");
        alarmsSysExit (&alarmsWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }

    statusUpdate(STATUS_BOOTING);
    statusUpdateStat(ALARM_STATS_ALARMS, retVal);


    // wait a bit here before continuing
    radUtilsSleep (500);


    //  register with the radlib message router
    if (radMsgRouterInit (WVIEW_RUN_DIR) == ERROR)
    {
        statusUpdateMessage("radMsgRouterInit failed!");
        radMsgLog (PRI_HIGH, "radMsgRouterInit failed!");
        statusUpdate(STATUS_ERROR);
        alarmsSysExit (&alarmsWork);
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
        alarmsSysExit (&alarmsWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }

    // finally, initialize our data feed socket
    alarmsWork.dataFeedServer = radSocketServerCreate(WV_DATAFEED_PORT);
    if (alarmsWork.dataFeedServer == NULL)
    {
        statusUpdateMessage("radSocketServerCreate failed");
        radMsgLog (PRI_HIGH, "radSocketServerCreate failed...");
        statusUpdate(STATUS_ERROR);
        radMsgRouterExit ();
        alarmsSysExit (&alarmsWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }

    // add it to our descriptors of interest
    if (radProcessIORegisterDescriptor(radSocketGetDescriptor(alarmsWork.dataFeedServer),
                                       dataFeedAccept,
                                       NULL)
        == ERROR)
    {
        statusUpdateMessage("radProcessIORegisterDescriptor server failed");
        radMsgLog (PRI_HIGH, "radProcessIORegisterDescriptor failed...");
        statusUpdate(STATUS_ERROR);
        radSocketDestroy (alarmsWork.dataFeedServer);
        radMsgRouterExit ();
        alarmsSysExit (&alarmsWork);
        radProcessExit ();
        radSystemExit (WVIEW_SYSTEM_ID);
        exit (1);
    }

    // enable message reception from the radlib router for loop data
    radMsgRouterMessageRegister (WVIEW_MSG_TYPE_LOOP_DATA_SVC);

    // enable message reception from the radlib router for archive data
    radMsgRouterMessageRegister (WVIEW_MSG_TYPE_ARCHIVE_DATA);

    // enable message reception from the radlib router for POLL msgs
    radMsgRouterMessageRegister (WVIEW_MSG_TYPE_POLL);


    // enter normal processing
    alarmsWork.inMainLoop = TRUE;
    statusUpdate(STATUS_RUNNING);
    statusUpdateMessage("Normal operation");
    radMsgLog (PRI_STATUS, "running...");


    // Do we need to trigger a test alarm?
    if (alarmsWork.doTest)
    {
        if (1 <= alarmsWork.doTestNumber && alarmsWork.doTestNumber <= ALARMS_MAX)
        {
            // Generate the bad boy:
            retVal = 1;
            for (alarm = (WVIEW_ALARM *) radListGetFirst (&alarmsWork.alarmList);
                 retVal <= ALARMS_MAX && alarm != NULL;
                 alarm = (WVIEW_ALARM *) radListGetNext (&alarmsWork.alarmList, 
                                                         (NODE_PTR)alarm))
            {
                if (retVal == alarmsWork.doTestNumber)
                {
                    // This is the one to test:
                    alarm->triggerValue = -1;

                    // run user script here
                    if (executeScript(alarm) != 0)
                    {
                        radMsgLog (PRI_MEDIUM, 
                                   "Test Alarm %d: script %s failed",
                                   retVal, alarm->scriptToRun);
                    }
                    else
                    {
                        radMsgLog (PRI_MEDIUM, 
                                   "Test Alarm %d: script %s executed",
                                   retVal, alarm->scriptToRun);
                    }
                    retVal = ALARMS_MAX;
                }

                retVal ++;
            }
        }
        else
        {
            radMsgLog (PRI_MEDIUM, "Test Alarm: bad alarm index %d given!",
                       alarmsWork.doTestNumber);
        }
    }


    while (!alarmsWork.exiting)
    {
        // wait on something interesting
        if (radProcessWait (0) == ERROR)
        {
            alarmsWork.exiting = TRUE;
        }
    }


    statusUpdateMessage("exiting normally");
    radMsgLog (PRI_STATUS, "exiting normally...");
    statusUpdate(STATUS_SHUTDOWN);

    radSocketDestroy (alarmsWork.dataFeedServer);
    radMsgRouterExit ();
    alarmsSysExit (&alarmsWork);
    radProcessExit ();
    radSystemExit (WVIEW_SYSTEM_ID);
    exit (0);
}

