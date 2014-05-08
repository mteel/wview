/*---------------------------------------------------------------------------
 
  FILENAME:
        virtualInterface.c
 
  PURPOSE:
        Provide the virtual station interface API and utilities.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        12/14/2009      M.S. Teel       0               Original

  NOTES:

  LICENSE:
        Copyright (c) 2009, Mark S. Teel (mark@teel.ws)

        This source code is released for free distribution under the terms
        of the GNU General Public License.
 
----------------------------------------------------------------------------*/

/*  ... System include files
*/

/*  ... Library include files
*/

/*  ... Local include files
*/
#include <virtualInterface.h>

/*  ... global memory declarations
*/

/*  ... local memory
*/

static VIRTUAL_IF_DATA  virtualWorkData;

void                    (*ArchiveIndicator) (ARCHIVE_PKT* newRecord);

static void             storeLoopPkt (LOOP_PKT *dest, VIRTUAL_IF_DATA *src);
static int              RestoreConnection(WVIEWD_WORK *work);


////////////****////****  S T A T I O N   A P I  ****////****////////////
/////  Must be provided by each supported wview station interface  //////

// station-supplied init function
// -- Can Be Asynchronous - event indication required --
//
// MUST:
//   - set the 'stationGeneratesArchives' flag in WVIEWD_WORK:
//     if the station generates archive records (TRUE) or they should be
//     generated automatically by the daemon from the sensor readings (FALSE)
//   - Initialize the 'stationData' store for station work area
//   - Initialize the interface medium
//   - do initial LOOP acquisition
//   - do any catch-up on archive records if there is a data logger
//   - 'work->runningFlag' can be used for start up synchronization but should
//     not be modified by the station interface code
//   - indicate init is done by sending the STATION_INIT_COMPLETE_EVENT event to
//     this process (radProcessEventsSend (NULL, STATION_INIT_COMPLETE_EVENT, 0))
//
// OPTIONAL:
//   - Initialize a state machine or any other construct required for the
//     station interface - these should be stored in the 'stationData' store
//
// 'archiveIndication' - indication callback used to pass back an archive record
//   generated as a result of 'stationGetArchive' being called; should receive a
//   NULL pointer for 'newRecord' if no record available; only used if
//   'stationGeneratesArchives' flag is set to TRUE by the station interface
//
// Returns: OK or ERROR
//
int stationInit
(
    WVIEWD_WORK     *work,
    void            (*archiveIndication)(ARCHIVE_PKT* newRecord)
)
{
    int             i;
    time_t          nowTime = time(NULL) - (WV_SECONDS_IN_HOUR * 12);
    ARCHIVE_PKT     recordStore;
    ARCHIVE_PKT     newestRecord;
    char            tempStr[WVIEW_MAX_PATH];

    memset (&virtualWorkData, 0, sizeof(virtualWorkData));

    // save the archive indication callback (we should never need it)
    ArchiveIndicator = archiveIndication;

    // set our work data pointer
    work->stationData = &virtualWorkData;

    // set the Archive Generation flag to indicate the VIRTUAL DOES NOT
    // generate them
    work->stationGeneratesArchives = TRUE;

    // initialize the medium abstraction based on user configuration
    if (!strcmp (work->stationInterface, "serial"))
    {
        radMsgLog (PRI_HIGH, "stationInit: serial medium not supported for virtual station!");
        return ERROR;
    }
    else if (!strcmp (work->stationInterface, "ethernet"))
    {
        if (ethernetMediumInit (&work->medium, work->stationHost, work->stationPort)
            == ERROR)
        {
            radMsgLog (PRI_HIGH, "stationInit: ethernet MediumInit failed");
            return ERROR;
        }
    }
    else
    {
        radMsgLog (PRI_HIGH, "stationInit: medium %s not supported",
                   work->stationInterface);
        return ERROR;
    }

    // initialize the interface using the media specific routine
    if ((*(work->medium.init))(&work->medium, work->stationDevice) == ERROR)
    {
        radMsgLog (PRI_HIGH, "stationInit: medium setup failed");
        return ERROR;
    }

    // Make the socket blocking:
    radSocketSetBlocking ((*(work->medium.getsocket))(&work->medium), TRUE);

    // Reset the stationType to include the host:port:
    sprintf(tempStr, "%s:%s:%d", 
            work->stationType, work->stationHost, work->stationPort);
    wvstrncpy(work->stationType, tempStr, sizeof(work->stationType));

    if (!strcmp (work->stationInterface, "ethernet"))
    {
        radMsgLog (PRI_STATUS, "VIRTUAL on %s:%d opened ...",
                   work->stationHost, work->stationPort);
    }

    // grab the station configuration now
    if (stationGetConfigValueInt (work,
                                  STATION_PARM_ELEVATION,
                                  &virtualWorkData.elevation)
            == ERROR)
    {
        radMsgLog (PRI_HIGH, "stationInit: stationGetConfigValueInt ELEV failed!");
        (*(work->medium.exit)) (&work->medium);
        return ERROR;
    }
    if (stationGetConfigValueFloat (work,
                                    STATION_PARM_LATITUDE,
                                    &virtualWorkData.latitude)
            == ERROR)
    {
        radMsgLog (PRI_HIGH, "stationInit: stationGetConfigValueInt LAT failed!");
        (*(work->medium.exit)) (&work->medium);
        return ERROR;
    }
    if (stationGetConfigValueFloat (work,
                                    STATION_PARM_LONGITUDE,
                                    &virtualWorkData.longitude)
            == ERROR)
    {
        radMsgLog (PRI_HIGH, "stationInit: stationGetConfigValueInt LONG failed!");
        (*(work->medium.exit)) (&work->medium);
        return ERROR;
    }
    if (stationGetConfigValueInt (work,
                                  STATION_PARM_ARC_INTERVAL,
                                  &virtualWorkData.archiveInterval)
            == ERROR)
    {
        radMsgLog (PRI_HIGH, "stationInit: stationGetConfigValueInt ARCINT failed!");
        (*(work->medium.exit)) (&work->medium);
        return ERROR;
    }

    // set the work archive interval now
    work->archiveInterval = virtualWorkData.archiveInterval;

    // sanity check the archive interval against the most recent record
    if (stationVerifyArchiveInterval (work) == ERROR)
    {
        // bad magic!
        radMsgLog (PRI_HIGH, "stationInit: stationVerifyArchiveInterval failed!");
        radMsgLog (PRI_HIGH, "You must either move old archive data out of the way -or-");
        radMsgLog (PRI_HIGH, "fix the interval setting...");
        (*(work->medium.exit)) (&work->medium);
        return ERROR;
    }
    else
    {
        radMsgLog (PRI_STATUS, "station archive interval: %d minutes",
                   work->archiveInterval);
    }

    radMsgLog (PRI_STATUS, "Starting station interface: VIRTUAL"); 

    // This must be done here:
    work->archiveDateTime = dbsqliteArchiveGetNewestTime(&newestRecord);
    if ((int)work->archiveDateTime == ERROR)
    {
        work->archiveDateTime = 0;
        radMsgLog (PRI_STATUS, "stationInit: no archive records found in database!");
    }

    // initialize the station interface
    if (virtualProtocolInit(work) == ERROR)
    {
        radMsgLog (PRI_HIGH, "stationInit: virtualProtocolInit failed!");
        (*(work->medium.exit)) (&work->medium);
        return ERROR;
    }

    // do the initial GetReadings now
    if (virtualProtocolGetReadings(work, &virtualWorkData.virtualReadings) != OK)
    {
        radMsgLog (PRI_HIGH, "stationInit: initial virtualProtocolGetReadings failed!");
        virtualProtocolExit (work);
        (*(work->medium.exit)) (&work->medium);
        return ERROR;
    }

    // populate the LOOP structure
    storeLoopPkt (&work->loopPkt, &virtualWorkData);

    // we must indicate successful completion here -
    // even though we are synchronous, the daemon wants to see this event
    radProcessEventsSend(NULL, STATION_INIT_COMPLETE_EVENT, 0);

    return OK;
}

// station-supplied exit function
//
// Returns: N/A
//
void stationExit (WVIEWD_WORK *work)
{
    virtualProtocolExit (work);
    (*(work->medium.exit)) (&work->medium);

    return;
}

// station-supplied function to retrieve positional info (lat, long, elev) -
// should populate 'work' fields: latitude, longitude, elevation
// -- Synchronous --
//
// - If station does not store these parameters, they can be retrieved from the
//   wview.conf file (see daemon.c for example conf file use) - user must choose
//   station type "Generic" when running the wviewconfig script
//
// Returns: OK or ERROR
//
int stationGetPosition (WVIEWD_WORK *work)
{
    // just set the values from our internal store - we retrieved them in
    // stationInit
    work->elevation     = (int16_t)virtualWorkData.elevation;
    if (virtualWorkData.latitude >= 0)
        work->latitude      = (int16_t)((virtualWorkData.latitude*10)+0.5);
    else
        work->latitude      = (int16_t)((virtualWorkData.latitude*10)-0.5);
    if (virtualWorkData.longitude >= 0)
        work->longitude     = (int16_t)((virtualWorkData.longitude*10)+0.5);
    else
        work->longitude     = (int16_t)((virtualWorkData.longitude*10)-0.5);

    radMsgLog (PRI_STATUS, "station location: elevation: %d feet",
               work->elevation);

    radMsgLog (PRI_STATUS, "station location: latitude: %3.1f %c  longitude: %3.1f %c",
               (float)abs(work->latitude)/10.0,
               ((work->latitude < 0) ? 'S' : 'N'),
               (float)abs(work->longitude)/10.0,
               ((work->longitude < 0) ? 'W' : 'E'));

    return OK;
}

// station-supplied function to indicate a time sync should be performed if the
// station maintains time, otherwise may be safely ignored
// -- Can Be Asynchronous --
//
// Returns: OK or ERROR
//
int stationSyncTime (WVIEWD_WORK *work)
{
    // VIRTUAL does not keep time...
    return OK;
}

// station-supplied function to indicate sensor readings should be performed -
// should populate 'work' struct: loopPkt (see datadefs.h for minimum field reqs)
// -- Can Be Asynchronous --
//
// - indicate readings are complete by sending the STATION_LOOP_COMPLETE_EVENT
//   event to this process (radProcessEventsSend (NULL, STATION_LOOP_COMPLETE_EVENT, 0))
//
// Returns: OK or ERROR
//
int stationGetReadings (WVIEWD_WORK *work)
{
    // we will do this synchronously...

    // get readings from station
    if (virtualProtocolGetReadings(work, &virtualWorkData.virtualReadings) == OK)
    {
        // populate the LOOP structure
        storeLoopPkt(&work->loopPkt, &virtualWorkData);

        // indicate we are done
        radProcessEventsSend(NULL, STATION_LOOP_COMPLETE_EVENT, 0);
    }

    return OK;
}

// station-supplied function to indicate an archive record should be generated -
// MUST populate an ARCHIVE_RECORD struct and indicate it to 'archiveIndication'
// function passed into 'stationInit'
// -- Asynchronous - callback indication required --
//
// Returns: OK or ERROR
//
// Note: 'archiveIndication' should receive a NULL pointer for the newRecord if
//       no record is available
// Note: This function will only be invoked by the wview daemon if the
//       'stationInit' function set the 'stationGeneratesArchives' to TRUE
//
int stationGetArchive (WVIEWD_WORK *work)
{
    // Let the protocol module handle archive requests:
    virtualProtocolGetArchive(work);
    return OK;
}

// station-supplied function to indicate data is available on the station
// interface medium (serial or ethernet) -
// It is the responsibility of the station interface to read the data from the
// medium and process appropriately. The data does not have to be read within
// the context of this function, but may be used to stimulate a state machine.
// -- Synchronous --
//
// Returns: N/A
//
void stationDataIndicate (WVIEWD_WORK *work)
{
    int         retVal, done = FALSE;

    if (virtualProtocolDataIndicate(work) == ERROR_ABORT)
    {
        // We need to try to reconnect:
        radMsgLog (PRI_HIGH, "VIRTUAL: remote station lost - retrying...");
        virtualProtocolExit(work);
        (*(work->medium.exit))(&work->medium);

        while (! done && ! work->exiting)
        {
            retVal = RestoreConnection(work);
            if (retVal == ERROR_ABORT)
            {
                radMsgLog (PRI_HIGH, "VIRTUAL: restore connection failed - exiting");
                radProcessSetExitFlag();
                done = TRUE;
            }
            else if (retVal == OK)
            {
                radMsgLog (PRI_HIGH, "VIRTUAL: restore connection success");
                virtualProtocolInit(work);
                done = TRUE;
            }
            else
            {
                radMsgLog (PRI_HIGH, "VIRTUAL: try again in 15 seconds to restore connection.");
                radUtilsSleep(15000);
            }
        }
    }

    return;
}

// station-supplied function to receive IPM messages - any message received by
// the generic station message handler which is not recognized will be passed
// to the station-specific code through this function.
// It is the responsibility of the station interface to process the message 
// appropriately (or ignore it).
// -- Synchronous --
//
// Returns: N/A
//
void stationMessageIndicate (WVIEWD_WORK *work, int msgType, void *msg)
{
    // N/A
    return;
}

// station-supplied function to indicate the interface timer has expired -
// It is the responsibility of the station interface to start/stop the interface
// timer as needed for the particular station requirements.
// The station interface timer is specified by the 'ifTimer' member of the
// WVIEWD_WORK structure. No other timers in that structure should be manipulated
// in any way by the station interface code.
// -- Synchronous --
//
// Returns: N/A
//
void stationIFTimerExpiry (WVIEWD_WORK *work)
{
    return;
}


////////////****////  S T A T I O N   A P I   E N D  ////****////////////


//  ... ----- static (local) methods ----- ...

static int RestoreConnection(WVIEWD_WORK *work)
{
    if (ethernetMediumInit (&work->medium, work->stationHost, work->stationPort)
        == ERROR)
    {
        radMsgLog (PRI_HIGH, "RestoreConnection: ethernet MediumInit failed");
        return ERROR_ABORT;
    }

    // initialize the interface using the media specific routine
    if ((*(work->medium.init))(&work->medium, work->stationDevice) == ERROR)
    {
        radMsgLog (PRI_HIGH, "RestoreConnection: medium setup failed");
        return ERROR;
    }

    // Make the socket blocking:
    radSocketSetBlocking ((*(work->medium.getsocket))(&work->medium), TRUE);

    return OK;
}

static void storeLoopPkt (LOOP_PKT *dest, VIRTUAL_IF_DATA *src)
{
    *dest = src->virtualReadings;

    return;
}

