/*---------------------------------------------------------------------------
 
  FILENAME:
        htmlStates.c
 
  PURPOSE:
        Provide the wview HTML generator state machine handlers.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        08/30/03        M.S. Teel       0               Original
        12/01/2009      M. Hornsby      1               Moon Rise and Set
 
  NOTES:
        
 
  LICENSE:
        Copyright (c) 2004, Mark S. Teel (mark@teel.ws)
  
        This source code is released for free distribution under the terms 
        of the GNU General Public License.
  
----------------------------------------------------------------------------*/

/*  ... System include files
*/
#include <termios.h>

/*  ... Library include files
*/

/*  ... Local include files
*/
#include <services.h>
#include <html.h>
#include <glchart.h>


/*  ... global memory declarations
*/

/*  ... global memory referenced
*/

/*  ... static (local) memory declarations
*/

static void processNewArchiveRecord (HTML_WORK *work, WVIEW_MSG_ARCHIVE_NOTIFY *armsg)
{
    HISTORY_DATA        data;
    ARCHIVE_PKT         arcRecord;
    int                 currHour, currDay, currMonth, currYear;
    float               gmtOffsetHours;
    int                 startmin, starthour, startday, startmonth, startyear;
    int                 i, DSTFlag;
    int16_t             tempShort;
    time_t              ntime, baseTime;
    struct tm           locTime;
    int                 deltaArchiveIntervals;

    //  ... generate the mesonet file
    htmlgenMesonetFile (work->mgrId, armsg);

    //  ... check to see if a DST change has occured
    DSTFlag = wvutilsDetectDSTChange();
    if (DSTFlag != WVUTILS_DST_NO_CHANGE)
    {
        radMsgLog (PRI_STATUS, "DST change: updating astronomical times for new local time...");

        ntime = time (NULL);
        localtime_r (&ntime, &locTime);
        currDay     = locTime.tm_mday;
        currMonth   = locTime.tm_mon + 1;
        currYear    = locTime.tm_year + 1900;

        gmtOffsetHours = locTime.tm_gmtoff/(60.0*60.0);

        //  ... update the sun times:
        sunTimesGetSunRiseSet (currYear, currMonth, currDay,
                               (float)work->mgrId->stationLatitude/10,
                               (float)work->mgrId->stationLongitude/10,
                               RS_TYPE_SUN,
                               &work->mgrId->sunrise,
                               &work->mgrId->sunset);
        sunTimesGetSunRiseSet (currYear, currMonth, currDay,
                               (float)work->mgrId->stationLatitude/10,
                               (float)work->mgrId->stationLongitude/10,
                               RS_TYPE_CIVIL,
                               &work->mgrId->civilrise,
                               &work->mgrId->civilset);
        sunTimesGetSunRiseSet (currYear, currMonth, currDay,
                               (float)work->mgrId->stationLatitude/10,
                               (float)work->mgrId->stationLongitude/10,
                               RS_TYPE_ASTRO,
                               &work->mgrId->astrorise,
                               &work->mgrId->astroset);
        sunTimesGetSunRiseSet (currYear, currMonth, currDay,
                               (float)work->mgrId->stationLatitude/10,
                               (float)work->mgrId->stationLongitude/10,
                               RS_TYPE_MIDDAY,
                               &work->mgrId->midday,
                               &tempShort);
        work->mgrId->dayLength = 
                sunTimesGetDayLength (currYear, currMonth, currDay,
                                      (float)work->mgrId->stationLatitude/10,
                                      (float)work->mgrId->stationLongitude/10);

        GetMoonRiseSetTimes (currYear, currMonth, currDay,
                             (float)gmtOffsetHours,
                             (float)work->mgrId->stationLatitude/10,
                             (float)work->mgrId->stationLongitude/10,
                             &work->mgrId->moonrise,
                             NULL,
                             &work->mgrId->moonset,
                             NULL);

        // restart the generation timer as the change may leave it in limbo
        radProcessTimerStart (work->timer, 5000L);
    }

    baseTime = time (NULL);

    //  ... update our data by adding the new sample

    //// Interval change:

    //  ... determine time to start for last sample
    startmin    = wvutilsGetMin(armsg->dateTime);
    starthour   = wvutilsGetHour(armsg->dateTime);
    startday    = wvutilsGetDay(armsg->dateTime);
    startmonth  = wvutilsGetMonth(armsg->dateTime);
    startyear   = wvutilsGetYear(armsg->dateTime);

    currHour    = starthour;
    currDay     = startday;
    currMonth   = startmonth;
    currYear    = startyear;

    if (work->LastArchiveDateTime != 0)
    {
        // compute the number of archive intervals since the last good record
        deltaArchiveIntervals = armsg->dateTime - work->LastArchiveDateTime;
        deltaArchiveIntervals /= 60;
        if (deltaArchiveIntervals < work->archiveInterval)
        {
            // this is odd, report and bail out
            radMsgLog (PRI_MEDIUM, "processNewArchiveRecord: "
                                   "minutes since last archive record (%d) "
                                   "less than archive interval (%d) - "
                                   "ignoring record...",
                                   deltaArchiveIntervals,
                                   work->archiveInterval);
            return;
        }

        deltaArchiveIntervals /= work->archiveInterval;
    }
    else
    {
        deltaArchiveIntervals = 1;
    }

    work->LastArchiveDateTime = armsg->dateTime;

    if (dbsqliteArchiveGetAverages (work->mgrId->isMetricUnits,
                                    work->archiveInterval,
                                    &data,
                                    armsg->dateTime,
                                    1)
        <= 0)
    {
        // populate history data with ARCHIVE_VALUE_NULL
        for (i = 0; i < DATA_INDEX_MAX(work->isExtendedData); i ++)
        {
            data.values[i] = ARCHIVE_VALUE_NULL;
            data.samples[i] = 0;
        }
        radMsgLog (PRI_MEDIUM, "no data found for sample...");
    }

    wvutilsLogEvent (PRI_STATUS, "Adding %d minute sample for %4.4d-%2.2d-%2.2d %2.2d:%2.2d...", 
                     work->archiveInterval, startyear, startmonth, startday, starthour, startmin);

    //  ... set the archiveAvailable flag for generation...
    work->mgrId->newArchiveMask |= NEW_ARCHIVE_SAMPLE;

    // add the RX check value to the data here
    data.values[DATA_INDEX_rxCheckPercent] = armsg->rxPercent;

    htmlmgrAddSampleValue (work->mgrId, &data, deltaArchiveIntervals);
        
    // generate an ASCII entry in the day's browser file (if enabled)
    if (dbsqliteArchiveGetRecord(armsg->dateTime, &arcRecord) == OK)
    {
        arcrecGenerate(&arcRecord, work->mgrId->isMetricUnits);
    }


    //// Hour change?
    if (work->histLastHour != currHour)
    {
        // determine times to start for last hour:
        ntime = time (NULL);
        localtime_r (&ntime, &locTime);
        locTime.tm_sec   = 0;
        locTime.tm_min   = work->archiveInterval;
        locTime.tm_hour  = currHour;
        locTime.tm_mday  = currDay;
        locTime.tm_mon   = currMonth - 1;
        locTime.tm_year  = currYear - 1900;
        locTime.tm_isdst = -1;

        ntime = mktime (&locTime);
        ntime -= WV_SECONDS_IN_HOUR;
        localtime_r (&ntime, &locTime);

        starthour = locTime.tm_hour;

        if (dbsqliteArchiveGetAverages (work->mgrId->isMetricUnits,
                                        work->archiveInterval,
                                        &data,
                                        ntime,
                                        WV_SECONDS_IN_HOUR/SECONDS_IN_INTERVAL(work->archiveInterval))
            <= 0)
        {
            // populate history data with ARCHIVE_VALUE_NULL
            for (i = 0; i < DATA_INDEX_MAX(work->isExtendedData); i ++)
            {
                data.values[i] = ARCHIVE_VALUE_NULL;
                data.samples[i] = 0;
            }
            radMsgLog (PRI_MEDIUM, "no data found for last hour...");
        }

        wvutilsLogEvent (PRI_STATUS, "Adding hour sample...");

        //  ... set the archiveAvailable flag for generation...
        work->mgrId->newArchiveMask |= NEW_ARCHIVE_HOUR;

        htmlmgrAddHourValue (work->mgrId, &data);

        work->histLastHour = currHour;
    }

    //// Day change?
    if (work->histLastDay != currDay)
    {
        // determine times to start for last day:
        // normalize packed time:
        ntime = time (NULL);
        localtime_r (&ntime, &locTime);
        locTime.tm_sec   = 0;
        locTime.tm_min   = work->archiveInterval;
        locTime.tm_hour  = 0;
        locTime.tm_mday  = currDay;
        locTime.tm_mon   = currMonth - 1;
        locTime.tm_year  = currYear - 1900;
        locTime.tm_isdst = -1;
		gmtOffsetHours = locTime.tm_gmtoff/(60.0*60.0);

        ntime = mktime (&locTime);
        ntime -= WV_SECONDS_IN_DAY;
        localtime_r (&ntime, &locTime);

        if (dbsqliteArchiveGetAverages (work->mgrId->isMetricUnits,
                                        work->archiveInterval,
                                        &data,
                                        ntime,
                                        WV_SECONDS_IN_DAY/SECONDS_IN_INTERVAL(work->archiveInterval))
            <= 0)
        {
            // populate history data with ARCHIVE_VALUE_NULL
            for (i = 0; i < DATA_INDEX_MAX(work->isExtendedData); i ++)
            {
                data.values[i] = ARCHIVE_VALUE_NULL;
                data.samples[i] = 0;
            }
            radMsgLog (PRI_MEDIUM, "no data found for last day...");
        }
        else
        {
            // Add a day history record:
            wvutilsLogEvent(PRI_STATUS, "Storing day history for %s", ctime(&ntime));
            dbsqliteHistoryInsertDay(&data);
        }

        //  ... set the archiveAvailable flag for generation...
        work->mgrId->newArchiveMask |= NEW_ARCHIVE_DAY;

        wvutilsLogEvent(PRI_STATUS, "Adding day sample...");
        htmlmgrAddDayValue (work->mgrId, &data);

        work->histLastDay = currDay;

        //  Start the timeout to do NOAA updates:
        radProcessTimerStart(work->noaaTimer, HTML_NOAA_UPDATE_DELAY);

        //  ... update the sun times
        sunTimesGetSunRiseSet (currYear, currMonth, currDay,
                               (float)work->mgrId->stationLatitude/10,
                               (float)work->mgrId->stationLongitude/10,
                               RS_TYPE_SUN,
                               &work->mgrId->sunrise,
                               &work->mgrId->sunset);
        sunTimesGetSunRiseSet (currYear, currMonth, currDay,
                               (float)work->mgrId->stationLatitude/10,
                               (float)work->mgrId->stationLongitude/10,
                               RS_TYPE_CIVIL,
                               &work->mgrId->civilrise,
                               &work->mgrId->civilset);
        sunTimesGetSunRiseSet (currYear, currMonth, currDay,
                               (float)work->mgrId->stationLatitude/10,
                               (float)work->mgrId->stationLongitude/10,
                               RS_TYPE_ASTRO,
                               &work->mgrId->astrorise,
                               &work->mgrId->astroset);
        sunTimesGetSunRiseSet (currYear, currMonth, currDay,
                               (float)work->mgrId->stationLatitude/10,
                               (float)work->mgrId->stationLongitude/10,
                               RS_TYPE_MIDDAY,
                               &work->mgrId->midday,
                               &tempShort);
        work->mgrId->dayLength = 
                sunTimesGetDayLength (currYear, currMonth, currDay,
                                      (float)work->mgrId->stationLatitude/10,
                                      (float)work->mgrId->stationLongitude/10);

        GetMoonRiseSetTimes (currYear, currMonth, currDay,
                             (float)gmtOffsetHours,
                             (float)work->mgrId->stationLatitude/10,
                             (float)work->mgrId->stationLongitude/10,
                             &work->mgrId->moonrise,
                             NULL,
                             &work->mgrId->moonset,
                             NULL);

    }

    return;
}

static int requestDataPackets (HTML_WORK *work)
{
    WVIEW_MSG_REQUEST   msg;

    msg.requestType = WVIEW_RQST_TYPE_LOOP_DATA;
    if (radMsgRouterMessageSend (WVIEW_MSG_TYPE_REQUEST, &msg, sizeof(msg)) == ERROR)
    {
        radMsgLog (PRI_HIGH, "requestDataPackets: radMsgRouterMessageSend failed!");
        return ERROR;
    }

    msg.requestType = WVIEW_RQST_TYPE_HILOW_DATA;
    if (radMsgRouterMessageSend (WVIEW_MSG_TYPE_REQUEST, &msg, sizeof(msg)) == ERROR)
    {
        radMsgLog (PRI_HIGH, "requestDataPackets: radMsgRouterMessageSend failed!");
        return ERROR;
    }

    return OK;
}



int htmlIdleState (int state, void *stimulus, void *data)
{
    STIM                *stim = (STIM *)stimulus;
    HTML_WORK           *work = (HTML_WORK *)data;
    uint16_t            year, month, day, hour, minute, second;
    WVIEW_MSG_REQUEST   msg;

    switch (stim->type)
    {
    case STIM_DUMMY:
        // this one starts this state machine - send the archive request
        msg.requestType = WVIEW_RQST_TYPE_STATION_INFO;
        if (radMsgRouterMessageSend (WVIEW_MSG_TYPE_REQUEST, &msg, sizeof(msg)) == ERROR)
        {
            radMsgLog (PRI_HIGH, "htmlIdleState: radMsgRouterMessageSend failed!");
            statusUpdateMessage("radMsgRouterMessageSend failed!");
            statusUpdate(STATUS_ERROR);
            return HTML_STATE_ERROR;
        }

        // initialize the DST state change detector here
        wvutilsDetectDSTInit ();

        statusUpdate(STATUS_WAITING_FOR_WVIEW);
        return HTML_STATE_STATION_INFO;
    }

    return state;
}

int htmlStationInfoState (int state, void *stimulus, void *data)
{
    STIM                        *stim = (STIM *)stimulus;
    HTML_WORK                   *work = (HTML_WORK *)data;
    int                         seconds;
    WVIEW_MSG_STATION_INFO      *msg = (WVIEW_MSG_STATION_INFO *)stim->msg;
    time_t                      ntime;
    struct tm                   locTime;
    long                        offset, msOffset;
    long                        oldSecs, newSecs;
    int                         currDay, currMonth, currYear;
    float                       gmtOffsetHours;
    int16_t                     tempShort;
    FILE*                       indicateFile;

    ntime = time (NULL);
    localtime_r (&ntime, &locTime);
    currDay     = locTime.tm_mday;
    currMonth   = locTime.tm_mon + 1;
    currYear    = locTime.tm_year + 1900;
    gmtOffsetHours = locTime.tm_gmtoff/(60.0*60.0);

    switch (stim->type)
    {
    case STIM_QMSG:
        if (stim->msgType == WVIEW_MSG_TYPE_STATION_INFO)
        {
            radMsgRouterMessageDeregister(WVIEW_MSG_TYPE_STATION_INFO);
            ntime = msg->lastArcTime;
            localtime_r (&ntime, &locTime);
            radMsgLog (PRI_STATUS, 
                       "received station info from wviewd: "
                       "%4.4d%2.2d%2.2d %2.2d:%2.2d:%2.2d",
                       locTime.tm_year + 1900,
                       locTime.tm_mon + 1,
                       locTime.tm_mday,
                       locTime.tm_hour,
                       locTime.tm_min,
                       locTime.tm_sec);

            work->archiveInterval = msg->archiveInterval;
            
            // Initialize the archive database interface:
            if (dbsqliteArchiveInit() == ERROR)
            {
                radMsgLog (PRI_HIGH, "dbsqliteArchiveInit failed");
                statusUpdateMessage("dbsqliteArchiveInit failed!");
                statusUpdate(STATUS_ERROR);
                return HTML_STATE_ERROR;
            }


            // initialize the htmlMgr now that we know the archive interval:
            work->mgrId = htmlmgrInit (WVIEW_CONFIG_DIR, 
                                       work->isMetricUnits,
                                       work->imagePath,
                                       work->htmlPath,
                                       work->archiveInterval,
                                       work->isExtendedData,
                                       work->stationName,
                                       work->stationCity,
                                       work->stationState,
                                       msg->elevation,
                                       msg->latitude,
                                       msg->longitude,
                                       work->mphaseIncrease,
                                       work->mphaseDecrease,
                                       work->mphaseFull,
                                       work->radarURL,
                                       work->forecastURL,
                                       work->dateFormat,
                                       work->isDualUnits);
            if (work->mgrId == NULL)
            {
                radMsgLog (PRI_HIGH, "htlmgrInit failed!");
                statusUpdateMessage("htlmgrInit failed!");
                statusUpdate(STATUS_ERROR);
                return HTML_STATE_ERROR;
            }
        
            wvstrncpy (work->mgrId->stationType,
                       msg->stationType,
                       sizeof(work->mgrId->stationType));

            //  ... initialize the sun times now
            sunTimesGetSunRiseSet (currYear, currMonth, currDay,
                                   (float)work->mgrId->stationLatitude/10,
                                   (float)work->mgrId->stationLongitude/10,
                                   RS_TYPE_SUN,
                                   &work->mgrId->sunrise,
                                   &work->mgrId->sunset);
            sunTimesGetSunRiseSet (currYear, currMonth, currDay,
                                   (float)work->mgrId->stationLatitude/10,
                                   (float)work->mgrId->stationLongitude/10,
                                   RS_TYPE_CIVIL,
                                   &work->mgrId->civilrise,
                                   &work->mgrId->civilset);
            sunTimesGetSunRiseSet (currYear, currMonth, currDay,
                                   (float)work->mgrId->stationLatitude/10,
                                   (float)work->mgrId->stationLongitude/10,
                                   RS_TYPE_ASTRO,
                                   &work->mgrId->astrorise,
                                   &work->mgrId->astroset);
            sunTimesGetSunRiseSet (currYear, currMonth, currDay,
                                   (float)work->mgrId->stationLatitude/10,
                                   (float)work->mgrId->stationLongitude/10,
                                   RS_TYPE_MIDDAY,
                                   &work->mgrId->midday,
                                   &tempShort);
            work->mgrId->dayLength = 
                    sunTimesGetDayLength (currYear, currMonth, currDay,
                                          (float)work->mgrId->stationLatitude/10,
                                          (float)work->mgrId->stationLongitude/10);

            GetMoonRiseSetTimes (currYear, currMonth, currDay,
                                 (float)gmtOffsetHours,
                                 (float)work->mgrId->stationLatitude/10,
                                 (float)work->mgrId->stationLongitude/10,
                                 &work->mgrId->moonrise,
                                 NULL,
                                 &work->mgrId->moonset,
                                 NULL);



            //  ... initialize the barometric pressure trend algorithm:
            radMsgLog (PRI_STATUS, "initializing barometric pressure trend");        
            if (htmlmgrBPTrendInit (work->mgrId, work->timerInterval/60000)
                == ERROR)
            {
                radMsgLog (PRI_HIGH, "htmlmgrBPTrendInit failed!");
                statusUpdateMessage("htmlmgrBPTrendInit failed!");
                statusUpdate(STATUS_ERROR);
                return HTML_STATE_ERROR;
            }

           
            //  ... load up our historical stores for the history charts:
            radMsgLog (PRI_STATUS, "initializing historical stores (this may take some time...)");  
            if (htmlmgrHistoryInit (work->mgrId) == ERROR)
            {
                radMsgLog (PRI_HIGH, "htmlStationInfoState: htmlmgrHistoryInit failed!");
                statusUpdateMessage("htmlmgrHistoryInit failed!");
                statusUpdate(STATUS_ERROR);
                return HTML_STATE_ERROR;
            }

            // store the "last" values
            ntime = time(NULL);
            localtime_r (&ntime, &locTime);
            work->histLastHour = locTime.tm_hour;
            work->histLastDay = locTime.tm_mday;

            
            // Initialize the HILOW database interface:
            // (this cannot occur before wviewd sends us the archive time)
            if (dbsqliteHiLowInit(FALSE) == ERROR)
            {
                radMsgLog (PRI_CATASTROPHIC, "dbsqliteHiLowInit failed!");
                statusUpdateMessage("dbsqliteHiLowInit failed!");
                statusUpdate(STATUS_ERROR);
                return HTML_STATE_ERROR;
            }

            // initialize the NOAA database:
            if (dbsqliteNOAAInit() == ERROR)
            {
                radMsgLog (PRI_CATASTROPHIC, "dbsqliteNOAAInit failed!");
                statusUpdateMessage("dbsqliteNOAAInit failed!");
                statusUpdate(STATUS_ERROR);
                return HTML_STATE_ERROR;
            }

            // initialize the NOAA report generator
            radMsgLog (PRI_STATUS, "NOAA: initializing reports (this may take some time...)");  
            work->noaaId = noaaGenerateInit (work->mgrId->imagePath,
                                             work->isMetricUnits,
                                             msg->latitude,
                                             msg->longitude,
                                             msg->elevation,
                                             work->stationName,
                                             work->stationCity,
                                             work->stationState,
                                             msg->lastArcTime);
            if (work->noaaId == NULL)
            {
                radMsgLog (PRI_CATASTROPHIC, "noaaGenerateInit failed!");
                statusUpdateMessage("noaaGenerateInit failed!");
                statusUpdate(STATUS_ERROR);
                return HTML_STATE_ERROR;
            }


            // initialize the archive record browser generator
            radMsgLog (PRI_STATUS, "ARCREC: initializing archive browser files (this may take some time...)");  
            if (arcrecGenerateInit (work->mgrId->imagePath,
                                    work->arcrecDaysToKeep,
                                    work->isMetricUnits,
                                    work->archiveInterval)
                == ERROR)
            {
                radMsgLog (PRI_CATASTROPHIC, "arcrecGenerateInit failed!");
                statusUpdateMessage("arcrecGenerateInit failed!");
                statusUpdate(STATUS_ERROR);
                return HTML_STATE_ERROR;
            }


            // initial data request then start the data acquisition timer
            if (requestDataPackets (work) == ERROR)
            {
                radMsgLog (PRI_HIGH, "htmlStationInfoState: requestDataPackets failed!");
                statusUpdateMessage("requestDataPackets failed!");
                statusUpdate(STATUS_ERROR);
                return HTML_STATE_ERROR;
            }

            radProcessTimerStart (work->rxTimer, HTML_RX_PACKETS_TIMEOUT);

            // wait here until a new second arrives
            ntime = time (NULL);
            localtime_r (&ntime, &locTime);
            oldSecs = newSecs = locTime.tm_sec;
            while (oldSecs == newSecs)
            {
                ntime = time (NULL);
                localtime_r (&ntime, &locTime);
                newSecs = locTime.tm_sec;
            }
            
            
            /*  ... start the data timer to go off at the next 5 minute
                ... interval plus the user offset
            */
            ntime = time (NULL);
            localtime_r (&ntime, &locTime);
            seconds = locTime.tm_sec - 10L;         // start at 10 secs past

            offset = locTime.tm_min % 5L;           // where are we in period?
            offset = 5 - offset;                    // how many mins to get to "0"?
            offset %= 5;                            // don't want "5"
            offset += work->startOffset;            // user start in period
            if (offset == 0L)                       // prevent negative timeout
                seconds -= 60L;                     // make neg so timeout is pos

            if (seconds < -60L)
            {
                offset += 1L;
                seconds += 60L;
            }

            radMsgLog (PRI_STATUS,
                       "starting html generation in %d mins %d secs",
                       ((seconds > 0L) ? ((offset > 0L) ? offset - 1L : offset) : offset),
                               (seconds > 0L) ? 60L - seconds : (-1L) * seconds);

            work->nextGenerationTime.tv_sec = (long)ntime + ((offset * 60L) - seconds);
            work->nextGenerationTime.tv_usec = 250L * 1000L;
            
            msOffset = 250L;                        // land on 250 ms mark

            radProcessTimerStart (work->timer,
                                  ((((offset * 60L) - seconds) * 1000L)) + msOffset);


            // 20041224 - Add an immediate generation cycle so longer 
            // duration archive intervals don't have to wait up to 30 minutes
            // for the first generation
            radMsgLog (PRI_STATUS, "doing initial html generation now...");
            work->numDataReceived = 0;

            statusUpdate(STATUS_RUNNING);
            statusUpdateMessage("Normal operation");

            // Save an indicator in the run directory so web apps know when we are up:
            indicateFile = fopen (work->indicateFile, "w");
            if (indicateFile == NULL)
            {
                radMsgLog (PRI_CATASTROPHIC, "indicator file create failed!");
                statusUpdateMessage("indicator file create failed!");
                statusUpdate(STATUS_ERROR);
                return HTML_STATE_ERROR;
            }
            fprintf (indicateFile, "%d", (int)msg->lastArcTime);
            fclose (indicateFile);

            return HTML_STATE_DATA;
        }

        break;
    }

    return state;
}

int htmlRunState (int state, void *stimulus, void *data)
{
    STIM                        *stim = (STIM *)stimulus;
    HTML_WORK                   *work = (HTML_WORK *)data;
    WVIEW_MSG_LOOP_DATA         *loop = (WVIEW_MSG_LOOP_DATA *)stim->msg;
    WVIEW_MSG_HILOW_DATA        *hilow = (WVIEW_MSG_HILOW_DATA *)stim->msg;
    WVIEW_MSG_ARCHIVE_NOTIFY    *arcNot = (WVIEW_MSG_ARCHIVE_NOTIFY *)stim->msg;

    switch (stim->type)
    {
        case STIM_QMSG:
            if (stim->msgType == WVIEW_MSG_TYPE_LOOP_DATA)
            {
                work->mgrId->loopStore = loop->loopData;
                return state;
            }
            else if (stim->msgType == WVIEW_MSG_TYPE_HILOW_DATA)
            {
                work->mgrId->hilowStore = hilow->hilowData;
                return state;
            }
            else if (stim->msgType == WVIEW_MSG_TYPE_ARCHIVE_NOTIFY)
            {
                processNewArchiveRecord (work, arcNot);
                return state;
            }

        break;

        case STIM_TIMER:
            if (stim->timerNumber == TIMER_GENERATE)
            {
                //  ... request the next batch of data
                if (requestDataPackets (work) == ERROR)
                {
                    radMsgLog (PRI_HIGH, "htmlRunState: requestDataPackets failed!");
                    statusUpdateMessage("requestDataPackets failed!");
                    statusUpdate(STATUS_ERROR);
                    return HTML_STATE_ERROR;
                }
    
                radProcessTimerStart(work->rxTimer, HTML_RX_PACKETS_TIMEOUT);
                work->numDataReceived = 0;
    
                return HTML_STATE_DATA;
            }
    
            return state;
        }

    return state;
}


int htmlDataState (int state, void *stimulus, void *data)
{
    STIM                        *stim = (STIM *)stimulus;
    HTML_WORK                   *work = (HTML_WORK *)data;
    WVIEW_MSG_LOOP_DATA         *loop = (WVIEW_MSG_LOOP_DATA *)stim->msg;
    WVIEW_MSG_HILOW_DATA        *hilow = (WVIEW_MSG_HILOW_DATA *)stim->msg;
    WVIEW_MSG_ARCHIVE_NOTIFY    *arcNot = (WVIEW_MSG_ARCHIVE_NOTIFY *)stim->msg;

    switch (stim->type)
    {
        case STIM_QMSG:
            if (stim->msgType == WVIEW_MSG_TYPE_LOOP_DATA)
            {
                work->mgrId->loopStore = loop->loopData;
                if (++ work->numDataReceived == 2)
                {
                    // we've got the data, generate some html
                    radProcessTimerStop(work->rxTimer);
                    htmlmgrGenerate (work->mgrId);
    
                    return HTML_STATE_RUN;
                }
            }
            else if (stim->msgType == WVIEW_MSG_TYPE_HILOW_DATA)
            {
                work->mgrId->hilowStore = hilow->hilowData;
                if (++ work->numDataReceived == 2)
                {
                    // we've got the data, generate some html
                    radProcessTimerStop(work->rxTimer);
                    htmlmgrGenerate (work->mgrId);
    
                    return HTML_STATE_RUN;
                }
            }
            else if (stim->msgType == WVIEW_MSG_TYPE_ARCHIVE_NOTIFY)
            {
                processNewArchiveRecord (work, arcNot);
                return state;
            }
    
            return state;

        case STIM_TIMER:
            if (stim->timerNumber == TIMER_RX_PACKETS)
            {
                // We have timed out waiting for pkts from wviewd -
                // Raise a stink:
                radMsgLog (PRI_HIGH, "wviewd NOT responding!");
                radMsgLog (PRI_HIGH, "Are there remnants of a previous failed start?");
                radMsgLog (PRI_HIGH, "Continuing - if this is not an isolated event you should investigate further.");
                return state;
            }

            return state;

        default:
            return state;
    }

    return state;
}

int htmlErrorState (int state, void *stimulus, void *data)
{
    STIM                *stim = (STIM *)stimulus;
    HTML_WORK           *work = (HTML_WORK *)data;


    switch (stim->type)
    {
    case STIM_QMSG:
    case STIM_TIMER:
        radMsgLog (PRI_STATUS, "htmlErrorState: received stimulus %d", stim->type);
        break;
    }

    return state;
}

