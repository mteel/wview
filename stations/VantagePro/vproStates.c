/*---------------------------------------------------------------------------

  FILENAME:
        vproStates.c

  PURPOSE:
        Provide the wview vpro state machine handlers.

  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        08/14/03        M.S. Teel       0               Original
        04/12/2008      W. Krenn        1               rainTicksPerInch
                                                        RainCollectorType

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
#include <station.h>
#include <daemon.h>
#include <vproInterface.h>


/*  ... global memory declarations
*/

/*  ... global memory referenced
*/

/*  ... static (local) memory declarations
*/

/* ... state machine methods
*/

int vproStartProcState (int state, void *stimulus, void *data)
{
    STIM                *stim = (STIM *)stimulus;
    WVIEWD_WORK         *work = (WVIEWD_WORK *)data;
    time_t              ntime;
    struct tm           locTime;
    int                 secsToWait, secsNow, numTries;

    switch (stim->type)
    {
    case STIM_DUMMY:
        //  ... this one starts this state machine
        ntime = time (NULL);
        localtime_r (&ntime, &locTime);

        if (locTime.tm_sec > 50 || locTime.tm_sec < 5)
        {
            if (locTime.tm_sec < 5)
                secsNow = locTime.tm_sec + 60;
            else
                secsNow = locTime.tm_sec;
            secsToWait = 65 - secsNow;              // start at 5 secs after

            // we are too close to the archive record being available
            // for comfort, wait until it has passed
            radMsgLog (PRI_STATUS,
                       "starting too close to the top-of-minute, "
                       "waiting %d secs before continuing",
                       secsToWait);
            radUtilsSleep (secsToWait * 1000);
        }

        // ... make sure we can wakeup the console
        numTries = 1;
        while ((vpifWakeupConsole (work) == ERROR) &&
               (numTries < WVD_INITIAL_WAKEUP_TRIES))
        {
            radMsgLog (PRI_HIGH, "vproStartProcState: WAKEUP failed - retry");
            radUtilsSleep (1000);
            numTries ++;
        }
        if (numTries == WVD_INITIAL_WAKEUP_TRIES)
        {
            radMsgLog (PRI_HIGH, "vproStartProcState: WAKEUP failed - ERROR");
            emailAlertSend(ALERT_TYPE_STATION_VP_WAKEUP);
            return VPRO_STATE_ERROR;
        }

        // we need the archive interval now
        if (vpifGetArchiveInterval (work) == ERROR)
        {
            radMsgLog (PRI_HIGH, "vproStartProcState: vpifGetArchiveInterval failed");
            emailAlertSend(ALERT_TYPE_STATION_READ);
            return VPRO_STATE_ERROR;
        }

        // sanity check the archive interval against the most recent record
        if (stationVerifyArchiveInterval (work) == ERROR)
        {
            radMsgLog (PRI_HIGH, "vproStartProcState: stationVerifyArchiveInterval failed!");
            radMsgLog (PRI_HIGH, "You must either move old /var/wview/archive files out of the way -or-");
            radMsgLog (PRI_HIGH, "fix the station setting...");
            radProcessEventsSend (NULL, STATION_INIT_COMPLETE_EVENT, 1);
            emailAlertSend(ALERT_TYPE_STATION_READ);
            return VPRO_STATE_ERROR;
        }
        else
        {
            radMsgLog (PRI_STATUS, "station archive interval: %d minutes",
                       work->archiveInterval);
        }

        // get the station RainCollectorSize
        if (vpifGetRainCollectorSize(work) == ERROR)
        {
            radMsgLog (PRI_HIGH, "vproStartProcState: vpifGetRainCollectorSize failed");
            emailAlertSend(ALERT_TYPE_STATION_READ);
            return VPRO_STATE_ERROR;
        }
        else
        {
            radMsgLog (PRI_STATUS, "station rain ticks/inch: %.0f",
                       ((VP_IF_DATA *)(work->stationData))->rainTicksPerInch);
        }


        // now do the initial archive record sync and loop data
        ((VP_IF_DATA *)(work->stationData))->doLoopFlag = TRUE;

        //  wakeup the console
        if (vpifWakeupConsole (work) == ERROR)
        {
            radMsgLog (PRI_HIGH, "vproStartProcState: WAKEUP2 failed - ERROR");
            emailAlertSend(ALERT_TYPE_STATION_VP_WAKEUP);
            return VPRO_STATE_ERROR;
        }

        if (work->stationGeneratesArchives)
        {
            // Sync to console archive records:
            if (vpifSendDumpAfterRqst (work) == ERROR)
            {
                radMsgLog (PRI_HIGH, "vproStartProcState: DMPAFT_RQST failed");
                emailAlertSend(ALERT_TYPE_STATION_ARCHIVE);
                return VPRO_STATE_ERROR;
            }

            radProcessTimerStart (work->ifTimer, VP_RESPONSE_TIMEOUT(work->stationIsWLIP));
            return VPRO_STATE_DMPAFT_RQST;
        }
        else
        {
            // Just retrieve loop data.
            if (vpifSendLoopRqst (work, 1) == ERROR)
            {
                radMsgLog (PRI_HIGH, "vproStartProcState: LOOP_RQST failed");
                emailAlertSend(ALERT_TYPE_STATION_LOOP);
                return VPRO_STATE_ERROR;
            }

            radProcessTimerStart (work->ifTimer, VP_RESPONSE_TIMEOUT(work->stationIsWLIP));
            return VPRO_STATE_LOOP_RQST;
        }

    case STIM_QMSG:
    case STIM_EVENT:
    case STIM_TIMER:
        break;
    }

    return state;
}

int vproRunState (int state, void *stimulus, void *data)
{
    STIM                    *stim = (STIM *)stimulus;
    WVIEWD_WORK             *work = (WVIEWD_WORK *)data;

    switch (stim->type)
    {
    case VP_STIM_READINGS:
        ((VP_IF_DATA *)(work->stationData))->doLoopFlag = TRUE;

        // check to see if we need to retry the archive record
        if (((VP_IF_DATA *)(work->stationData))->archiveRetryFlag)
        {
            radMsgLog (PRI_MEDIUM, "vproRunState: retrying archive record from console:");
            radMsgLog (PRI_MEDIUM, "vproRunState: you may need to cycle power on the console"
                                   "(including batteries) to resolve this condition.");
            emailAlertSend(ALERT_TYPE_STATION_ARCHIVE);
            if (vpifWakeupConsole (work) == ERROR)
            {
                radMsgLog (PRI_HIGH, "vproRunState: ARC WAKEUP failed");
                return VPRO_STATE_RUN;
            }

            if (vpifSendDumpAfterRqst (work) == ERROR)
            {
                radMsgLog (PRI_HIGH, "vproRunState: DMPAFT_RQST failed");
                emailAlertSend(ALERT_TYPE_STATION_ARCHIVE);
                return VPRO_STATE_ERROR;
            }

            radProcessTimerStart (work->ifTimer, VP_RESPONSE_TIMEOUT(work->stationIsWLIP));
            return VPRO_STATE_DMPAFT_RQST;
        }
        else
        {
            //  ... wakeup the console
            if (vpifWakeupConsole (work) == ERROR)
            {
                radMsgLog (PRI_HIGH, "vproRunState: LOOP WAKEUP failed");
                return VPRO_STATE_RUN;
            }

            if (vpifSendLoopRqst (work, 1) == ERROR)
            {
                radMsgLog (PRI_HIGH, "vproRunState: LOOP_RQST failed");
                emailAlertSend(ALERT_TYPE_STATION_LOOP);
                return VPRO_STATE_ERROR;
            }

            radProcessTimerStart (work->ifTimer, VP_RESPONSE_TIMEOUT(work->stationIsWLIP));
            return VPRO_STATE_LOOP_RQST;
        }

    case VP_STIM_ARCHIVE:
        ((VP_IF_DATA *)(work->stationData))->archiveRetryFlag = TRUE;
        //  ... wakeup the console
        if (vpifWakeupConsole (work) == ERROR)
        {
            radMsgLog (PRI_HIGH, "vproRunState: ARC WAKEUP failed: retrying...");
            radUtilsSleep (1000);
            if (vpifWakeupConsole (work) == ERROR)
            {
                radMsgLog (PRI_HIGH, "vproRunState: 2nd ARC WAKEUP failed!");
                return VPRO_STATE_RUN;
            }
        }

        if (vpifSendDumpAfterRqst (work) == ERROR)
        {
            radMsgLog (PRI_HIGH, "vproRunState: DMPAFT_RQST failed");
            emailAlertSend(ALERT_TYPE_STATION_ARCHIVE);
            return VPRO_STATE_ERROR;
        }

        radProcessTimerStart (work->ifTimer, VP_RESPONSE_TIMEOUT(work->stationIsWLIP));
        return VPRO_STATE_DMPAFT_RQST;

    case STIM_IO:
        //  ... read data from the station
        if (vpifReadMessage (work, FALSE) == ERROR)
        {
            radProcessTimerStart (work->ifTimer, WVD_READ_RECOVER_INTERVAL);
            emailAlertSend(ALERT_TYPE_STATION_READ);
            return VPRO_STATE_READ_RECOVER;
        }

        return state;
    }

    return state;
}

int vproDumpAfterState (int state, void *stimulus, void *data)
{
    STIM                *stim = (STIM *)stimulus;
    WVIEWD_WORK         *work = (WVIEWD_WORK *)data;

    switch (stim->type)
    {
    case STIM_TIMER:
        // serial IF timer expiry
        // wakeup the console
        if (vpifWakeupConsole (work) == ERROR)
        {
            radMsgLog (PRI_HIGH, "vproDumpAfterState: WAKEUP failed");
            return VPRO_STATE_RUN;
        }

        if (vpifSendDumpAfterRqst (work) == ERROR)
        {
            radMsgLog (PRI_HIGH, "vproDumpAfterState: DMPAFT_RQST failed");
            emailAlertSend(ALERT_TYPE_STATION_ARCHIVE);
            return VPRO_STATE_ERROR;
        }

        radProcessTimerStart (work->ifTimer, VP_RESPONSE_TIMEOUT(work->stationIsWLIP));
        return state;

    case STIM_IO:
        radProcessTimerStop (work->ifTimer);

        /*  ... read data from the station
        */
        if (vpifReadMessage (work, TRUE) == ERROR)
        {
            radProcessTimerStart (work->ifTimer, WVD_READ_RECOVER_INTERVAL);
            emailAlertSend(ALERT_TYPE_STATION_READ);
            return VPRO_STATE_READ_RECOVER;
        }

        if (vpifSendDumpDateTimeRqst (work) == ERROR)
        {
            radMsgLog (PRI_HIGH, "vproDumpAfterState: DMP_DATETIME failed");
            emailAlertSend(ALERT_TYPE_STATION_ARCHIVE);
            return VPRO_STATE_ERROR;
        }

        radProcessTimerStart (work->ifTimer, 2*VP_RESPONSE_TIMEOUT(work->stationIsWLIP));
        return VPRO_STATE_DMPAFT_ACK;
    }

    return state;
}

int vproDumpAfterAckState (int state, void *stimulus, void *data)
{
    STIM                *stim = (STIM *)stimulus;
    WVIEWD_WORK         *work = (WVIEWD_WORK *)data;

    switch (stim->type)
    {
    case STIM_TIMER:
        // serial IF timer expiry
        // cancel the download
        if (vpifSendCancel (work) == ERROR)
        {
            radMsgLog (PRI_HIGH, "vproDumpAfterAckState: CANCEL failed");
            emailAlertSend(ALERT_TYPE_STATION_ARCHIVE);
            return VPRO_STATE_ERROR;
        }

        if (vpifWakeupConsole (work) == ERROR)
        {
            radMsgLog (PRI_HIGH, "vproDumpAfterAckState: WAKEUP failed");
            return VPRO_STATE_RUN;
        }

        if (vpifSendDumpDateTimeRqst (work) == ERROR)
        {
            radMsgLog (PRI_HIGH, "vproDumpAfterAckState: DMP_DATETIME failed");
            emailAlertSend(ALERT_TYPE_STATION_ARCHIVE);
            return VPRO_STATE_ERROR;
        }

        radProcessTimerStart (work->ifTimer, 2*VP_RESPONSE_TIMEOUT(work->stationIsWLIP));
        return state;

    case STIM_IO:
        radProcessTimerStop (work->ifTimer);

        /*  ... read data from the station
        */
        if (vpifReadMessage (work, TRUE) == ERROR)
        {
            radProcessTimerStart (work->ifTimer, WVD_READ_RECOVER_INTERVAL);
            radMsgLog (PRI_HIGH, "vproDumpAfterAckState: station read failed");
            emailAlertSend(ALERT_TYPE_STATION_READ);
            return VPRO_STATE_READ_RECOVER;
        }

        if (((VP_IF_DATA *)(work->stationData))->archivePages > 0)
        {
            if (vpifSendAck (work) == ERROR)
            {
                radMsgLog (PRI_HIGH, "vproDumpAfterAckState: ACK failed");
                emailAlertSend(ALERT_TYPE_STATION_ARCHIVE);
                return VPRO_STATE_ERROR;
            }

            ((VP_IF_DATA *)(work->stationData))->reqMsgType = SER_MSG_ARCHIVE;
            radProcessTimerStart (work->ifTimer, VP_RESPONSE_TIMEOUT(work->stationIsWLIP));
#if 0
            radMsgLog (PRI_STATUS, "downloading %d pages from Vantage Pro...",
                       ((VP_IF_DATA *)(work->stationData))->archivePages);
#endif
            ((VP_IF_DATA *)(work->stationData))->archiveCurrentPage = 0;
            return VPRO_STATE_RECV_ARCH;
        }
        else
        {
            if (vpifSendCancel (work) == ERROR)
            {
                radMsgLog (PRI_HIGH, "vproDumpAfterAckState: CANCEL failed");
                emailAlertSend(ALERT_TYPE_STATION_ARCHIVE);
                return VPRO_STATE_ERROR;
            }

            if (((VP_IF_DATA *)(work->stationData))->doLoopFlag)
            {
                // continue with the data acquisition
                if (vpifWakeupConsole (work) == ERROR)
                {
                    radMsgLog (PRI_HIGH, "vproDumpAfterAckState (initial): WAKEUP failed");
                    emailAlertSend(ALERT_TYPE_STATION_VP_WAKEUP);
                    return VPRO_STATE_ERROR;
                }

                if (vpifSendLoopRqst (work, 1) == ERROR)
                {
                    radMsgLog (PRI_HIGH, "vproDumpAfterAckState: LOOP_RQST failed");
                    emailAlertSend(ALERT_TYPE_STATION_LOOP);
                    return VPRO_STATE_ERROR;
                }

                radProcessTimerStart (work->ifTimer, VP_RESPONSE_TIMEOUT(work->stationIsWLIP));
                return VPRO_STATE_LOOP_RQST;
            }

            // go back to IDLE
            return VPRO_STATE_RUN;
        }
    }

    return state;
}

int vproReceiveArchiveState (int state, void *stimulus, void *data)
{
    STIM                *stim = (STIM *)stimulus;
    WVIEWD_WORK         *work = (WVIEWD_WORK *)data;
    int                 recsRX = 0;

    switch (stim->type)
    {
    case STIM_TIMER:
        // serial IF timer expiry
        radMsgLog (PRI_HIGH, "vproReceiveArchiveState: timed out waiting for archive page from VP console!");

        if (vpifSendCancel (work) == ERROR)
        {
            radMsgLog (PRI_HIGH, "vproReceiveArchiveState: CANCEL failed");
            emailAlertSend(ALERT_TYPE_STATION_ARCHIVE);
            return VPRO_STATE_ERROR;
        }

        if (!work->runningFlag)
        {
            // continue with the data acquisition
            if (vpifWakeupConsole (work) == ERROR)
            {
                radMsgLog (PRI_HIGH, "vproReceiveArchiveState (initial): WAKEUP failed");
                emailAlertSend(ALERT_TYPE_STATION_VP_WAKEUP);
                return VPRO_STATE_ERROR;
            }

            if (vpifSendLoopRqst (work, 1) == ERROR)
            {
                radMsgLog (PRI_HIGH, "vproReceiveArchiveState: LOOP_RQST failed");
                emailAlertSend(ALERT_TYPE_STATION_LOOP);
                return VPRO_STATE_ERROR;
            }

            radProcessTimerStart (work->ifTimer, VP_RESPONSE_TIMEOUT(work->stationIsWLIP));
            return VPRO_STATE_LOOP_RQST;
        }

        // go back to IDLE
        return VPRO_STATE_RUN;

    case STIM_IO:
        radProcessTimerStop (work->ifTimer);

        // read data from the station:
        recsRX = vpifReadMessage (work, TRUE);
        if (recsRX == ERROR)
        {
            // Don't let this lock up the IF:
            radMsgLog (PRI_HIGH, "vproReceiveArchiveState: read archive page failed");

            radUtilsSleep (50);

            if (vpifSendCancel (work) == ERROR)
            {
                radMsgLog (PRI_HIGH, "vproReceiveArchiveState: CANCEL failed");
                emailAlertSend(ALERT_TYPE_STATION_ARCHIVE);
                return VPRO_STATE_ERROR;
            }

            radUtilsSleep (50);

            if (((VP_IF_DATA *)(work->stationData))->doLoopFlag)
            {
                // continue with the data acquisition:
                if (vpifWakeupConsole (work) == ERROR)
                {
                    radMsgLog (PRI_HIGH, "vproReceiveArchiveState (initial): WAKEUP failed");
                    emailAlertSend(ALERT_TYPE_STATION_VP_WAKEUP);
                    return VPRO_STATE_ERROR;
                }

                if (vpifSendLoopRqst (work, 1) == ERROR)
                {
                    radMsgLog (PRI_HIGH, "vproReceiveArchiveState: LOOP_RQST failed");
                    emailAlertSend(ALERT_TYPE_STATION_LOOP);
                    return VPRO_STATE_ERROR;
                }

                radProcessTimerStart (work->ifTimer, VP_RESPONSE_TIMEOUT(work->stationIsWLIP));
                return VPRO_STATE_LOOP_RQST;
            }

            // go back to IDLE
            return VPRO_STATE_RUN;
        }

        if (((VP_IF_DATA *)(work->stationData))->archiveCurrentPage <
            ((VP_IF_DATA *)(work->stationData))->archivePages)
        {
            radProcessTimerStart (work->ifTimer, VP_RESPONSE_TIMEOUT(work->stationIsWLIP));
            return state;
        }
        else
        {
#if 0
            radMsgLog (PRI_STATUS, "... %d pages done",
                       ((VP_IF_DATA *)(work->stationData))->archivePages);
#endif

            radUtilsSleep (250);        // let him send any pending data

            if (vpifSendCancel (work) == ERROR)
            {
                radMsgLog (PRI_HIGH, "vproReceiveArchiveState: CANCEL failed");
                emailAlertSend(ALERT_TYPE_STATION_ARCHIVE);
                return VPRO_STATE_ERROR;
            }

            vpifFlush (work);
            radUtilsSleep (1);

            if (vpifWakeupConsole (work) == ERROR)
            {
                radMsgLog (PRI_HIGH, "vproReceiveArchiveState: WAKEUP1 failed");
                return VPRO_STATE_RUN;
            }

            if (((VP_IF_DATA *)(work->stationData))->doRXCheck)
            {
                // get the RX stats from the VP
                if (vpifGetRXCheck (work) == ERROR)
                {
                    // uh oh - give the VP console a short break...
                    radMsgLog (PRI_MEDIUM,
                               "vproReceiveArchiveState: vpifGetRXCheck failed: %s",
                               ((VP_IF_DATA *)(work->stationData))->rxCheck);
                    radUtilsSleep (1000);
                }

                // always wait a tad here...
                radUtilsSleep (250);
            }

            if (((VP_IF_DATA *)(work->stationData))->doLoopFlag)
            {
                // continue with the data acquisition
                if (vpifWakeupConsole (work) == ERROR)
                {
                    radMsgLog (PRI_HIGH, "vproReceiveArchiveState (initial): WAKEUP failed");
                    emailAlertSend(ALERT_TYPE_STATION_VP_WAKEUP);
                    return VPRO_STATE_ERROR;
                }

                if (vpifSendLoopRqst (work, 1) == ERROR)
                {
                    radMsgLog (PRI_HIGH, "vproReceiveArchiveState: LOOP_RQST failed");
                    emailAlertSend(ALERT_TYPE_STATION_LOOP);
                    return VPRO_STATE_ERROR;
                }

                radProcessTimerStart (work->ifTimer, VP_RESPONSE_TIMEOUT(work->stationIsWLIP));
                return VPRO_STATE_LOOP_RQST;
            }

            // go back to IDLE
            return VPRO_STATE_RUN;
        }
    }

    return state;
}

int vproLoopState (int state, void *stimulus, void *data)
{
    STIM                *stim = (STIM *)stimulus;
    WVIEWD_WORK         *work = (WVIEWD_WORK *)data;

    switch (stim->type)
    {
    case VP_STIM_ARCHIVE:
        ((VP_IF_DATA *)(work->stationData))->doArchiveFlag = TRUE;
        break;

    case STIM_TIMER:
        // serial IF timer expiry
        // wakeup the console
        if (vpifWakeupConsole (work) == ERROR)
        {
            radMsgLog (PRI_HIGH, "vproLoopState: WAKEUP failed");
            return VPRO_STATE_RUN;
        }

        if (vpifSendLoopRqst (work, 1) == ERROR)
        {
            radMsgLog (PRI_HIGH, "vproLoopState: LOOP_RQST failed");
            emailAlertSend(ALERT_TYPE_STATION_LOOP);
            return VPRO_STATE_ERROR;
        }

        radProcessTimerStart (work->ifTimer, VP_RESPONSE_TIMEOUT(work->stationIsWLIP));
        return state;

    case STIM_IO:
        radProcessTimerStop (work->ifTimer);

        // read data from the station
        if (vpifReadMessage (work, TRUE) == ERROR)
        {
            radProcessTimerStart (work->ifTimer, WVD_READ_RECOVER_INTERVAL);
            emailAlertSend(ALERT_TYPE_STATION_READ);
            return VPRO_STATE_READ_RECOVER;
        }

        ((VP_IF_DATA *)(work->stationData))->doLoopFlag = FALSE;

        // check to see if this was the first time through
        if (!work->runningFlag)
        {
            // indicate we are done with startup activities...
            vpifIndicateStationUp ();
        }
        else
        {
            // indicate the LOOP packet is done
            vpifIndicateLoopDone ();
        }

        // Check if we got an archive stimulus while doing the LOOP data:
        if (((VP_IF_DATA *)(work->stationData))->doArchiveFlag)
        {
            ((VP_IF_DATA *)(work->stationData))->doArchiveFlag = FALSE;
            if (vpifWakeupConsole (work) == ERROR)
            {
                radMsgLog (PRI_HIGH, "vproLoopState: WAKEUP failed");
                return VPRO_STATE_RUN;
            }

            if (vpifSendDumpAfterRqst (work) == ERROR)
            {
                radMsgLog (PRI_HIGH, "vproLoopState: DMPAFT_RQST failed");
                emailAlertSend(ALERT_TYPE_STATION_ARCHIVE);
                return VPRO_STATE_ERROR;
            }
            radProcessTimerStart (work->ifTimer, VP_RESPONSE_TIMEOUT(work->stationIsWLIP));
            return VPRO_STATE_DMPAFT_RQST;
        }

        // check to see if we have a pending time sync
        if (((VP_IF_DATA *)(work->stationData))->timeSyncFlag)
        {
            if (vpifSynchronizeConsoleClock (work) == OK)
            {
                ((VP_IF_DATA *)(work->stationData))->timeSyncFlag = 0;
            }
        }

        return VPRO_STATE_RUN;
    }

    return state;
}

int vproReadRecoverState (int state, void *stimulus, void *data)
{
    STIM                *stim = (STIM *)stimulus;
    WVIEWD_WORK         *work = (WVIEWD_WORK *)data;

    switch (stim->type)
    {
    case STIM_TIMER:
        // serial IF timer expiry
        // wakeup the console
        if (vpifWakeupConsole (work) == ERROR)
        {
            if (++work->numReadRetries > WVD_READ_RECOVER_MAX_RETRIES)
            {
                radMsgLog (PRI_HIGH,
                           "vproReadRecoverState: max retries attempted - giving up!");
                return VPRO_STATE_ERROR;
            }

            radProcessTimerStart (work->ifTimer, WVD_READ_RECOVER_INTERVAL);
            return state;
        }

        //  ... OK, let's return to normal
        work->numReadRetries = 0;
        return VPRO_STATE_RUN;

    case STIM_IO:
        /*  ... flush data from the station
        */
        vpifFlush (work);
        break;
    }

    return state;
}

static int errorStateReported = 0;
int vproErrorState (int state, void *stimulus, void *data)
{
    STIM                *stim = (STIM *)stimulus;
    WVIEWD_WORK         *work = (WVIEWD_WORK *)data;

    switch (stim->type)
    {
    case STIM_IO:
        vpifFlush (work);
    case STIM_QMSG:
    case STIM_EVENT:
    case STIM_TIMER:
        if (!errorStateReported)
        {
            radMsgLog (PRI_STATUS,
                       "%s:vproErrorState: received stimulus %d",
                       PROC_NAME_DAEMON,
                       stim->type);
            errorStateReported = 1;
        }
        break;
    }

    return state;
}
