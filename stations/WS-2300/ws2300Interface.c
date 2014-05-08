/*---------------------------------------------------------------------------
 
  FILENAME:
        ws2300Interface.c
 
  PURPOSE:
        Provide the La Crosse WS-2300 family station interface API and utilities.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        02/27/2008      M.S. Teel       0               Original
 
  NOTES:
        
 
  LICENSE:
        Copyright (c) 2008, Mark S. Teel (mark@teel.ws)
  
        This source code is released for free distribution under the terms 
        of the GNU General Public License.
  
----------------------------------------------------------------------------*/

/*  ... System include files
*/

/*  ... Library include files
*/

/*  ... Local include files
*/
#include <ws2300Interface.h>

/*  ... global memory declarations
*/

/*  ... local memory
*/

static WS2300_IF_DATA   ws2300WorkData;
static void             (*ArchiveIndicator) (ARCHIVE_PKT *newRecord);

static void serialPortConfig (int fd);


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
    STIM            stim;

    memset (&ws2300WorkData, 0, sizeof(ws2300WorkData));

    // save the archive indication callback (we should never need it)
    ArchiveIndicator = archiveIndication;

    // set our work data pointer
    work->stationData = &ws2300WorkData;

    // set the Archive Generation flag to indicate the WS-2300 DOES NOT
    // generate them
    work->stationGeneratesArchives = FALSE;

    // initialize the medium abstraction based on user configuration
    if (!strcmp (work->stationInterface, "serial"))
    {
        if (serialMediumInit (&work->medium, serialPortConfig, O_RDWR | O_NOCTTY | O_NDELAY) == ERROR)
        {
            radMsgLog (PRI_HIGH, "stationInit: serial MediumInit failed");
            return ERROR;
        }
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
    if ((*(work->medium.init)) (&work->medium, work->stationDevice) == ERROR)
    {
        radMsgLog (PRI_HIGH, "stationInit: medium setup failed");
        return ERROR;
    }

    if (!strcmp (work->stationInterface, "serial"))
    {
        radMsgLog (PRI_STATUS, "WS-2300 on %s opened ...",
                   work->stationDevice);
    }
    else if (!strcmp (work->stationInterface, "ethernet"))
    {
        radMsgLog (PRI_STATUS, "WS-2300 on %s:%d opened ...",
                   work->stationHost, work->stationPort);
    }

    // grab the station configuration now
    if (stationGetConfigValueInt (work,
                                  STATION_PARM_ELEVATION,
                                  &ws2300WorkData.elevation)
            == ERROR)
    {
        radMsgLog (PRI_HIGH, "stationInit: stationGetConfigValueInt ELEV failed!");
        (*(work->medium.exit)) (&work->medium);
        return ERROR;
    }
    if (stationGetConfigValueFloat (work,
                                    STATION_PARM_LATITUDE,
                                    &ws2300WorkData.latitude)
            == ERROR)
    {
        radMsgLog (PRI_HIGH, "stationInit: stationGetConfigValueInt LAT failed!");
        (*(work->medium.exit)) (&work->medium);
        return ERROR;
    }
    if (stationGetConfigValueFloat (work,
                                    STATION_PARM_LONGITUDE,
                                    &ws2300WorkData.longitude)
            == ERROR)
    {
        radMsgLog (PRI_HIGH, "stationInit: stationGetConfigValueInt LONG failed!");
        (*(work->medium.exit)) (&work->medium);
        return ERROR;
    }
    if (stationGetConfigValueInt (work,
                                  STATION_PARM_ARC_INTERVAL,
                                  &ws2300WorkData.archiveInterval)
            == ERROR)
    {
        radMsgLog (PRI_HIGH, "stationInit: stationGetConfigValueInt ARCINT failed!");
        (*(work->medium.exit)) (&work->medium);
        return ERROR;
    }

    // now initialize the state machine
    ws2300WorkData.stateMachine = radStatesInit (work);
    if (ws2300WorkData.stateMachine == NULL)
    {
        radMsgLog (PRI_HIGH, "stationInit: radStatesInit failed");
        (*(work->medium.exit)) (&work->medium);
        return ERROR;
    }

    if (radStatesAddHandler (ws2300WorkData.stateMachine, 
                             WS_STATE_STARTPROC,
                             wsStartProcState)
        == ERROR)
    {
        radMsgLog (PRI_HIGH, "stationInit: radStatesAddHandler failed");
        radStatesExit (ws2300WorkData.stateMachine);
        (*(work->medium.exit)) (&work->medium);
        return ERROR;
    }
    if (radStatesAddHandler (ws2300WorkData.stateMachine, 
                             WS_STATE_RUN,
                             wsRunState)
        == ERROR)
    {
        radMsgLog (PRI_HIGH, "stationInit: radStatesAddHandler failed");
        radStatesExit (ws2300WorkData.stateMachine);
        (*(work->medium.exit)) (&work->medium);
        return ERROR;
    }
    if (radStatesAddHandler (ws2300WorkData.stateMachine, 
                             WS_STATE_ADRS0_ACK,
                             wsAdrs0State)
        == ERROR)
    {
        radMsgLog (PRI_HIGH, "stationInit: radStatesAddHandler failed");
        radStatesExit (ws2300WorkData.stateMachine);
        (*(work->medium.exit)) (&work->medium);
        return ERROR;
    }
    if (radStatesAddHandler (ws2300WorkData.stateMachine, 
                             WS_STATE_ADRS1_ACK,
                             wsAdrs1State)
        == ERROR)
    {
        radMsgLog (PRI_HIGH, "stationInit: radStatesAddHandler failed");
        radStatesExit (ws2300WorkData.stateMachine);
        (*(work->medium.exit)) (&work->medium);
        return ERROR;
    }
    if (radStatesAddHandler (ws2300WorkData.stateMachine, 
                             WS_STATE_ADRS2_ACK,
                             wsAdrs2State)
        == ERROR)
    {
        radMsgLog (PRI_HIGH, "stationInit: radStatesAddHandler failed");
        radStatesExit (ws2300WorkData.stateMachine);
        (*(work->medium.exit)) (&work->medium);
        return ERROR;
    }
    if (radStatesAddHandler (ws2300WorkData.stateMachine, 
                             WS_STATE_ADRS3_ACK,
                             wsAdrs3State)
        == ERROR)
    {
        radMsgLog (PRI_HIGH, "stationInit: radStatesAddHandler failed");
        radStatesExit (ws2300WorkData.stateMachine);
        (*(work->medium.exit)) (&work->medium);
        return ERROR;
    }
    if (radStatesAddHandler (ws2300WorkData.stateMachine, 
                             WS_STATE_NUMBYTES_ACK,
                             wsNumBytesState)
        == ERROR)
    {
        radMsgLog (PRI_HIGH, "stationInit: radStatesAddHandler failed");
        radStatesExit (ws2300WorkData.stateMachine);
        (*(work->medium.exit)) (&work->medium);
        return ERROR;
    }
    if (radStatesAddHandler (ws2300WorkData.stateMachine, 
                             WS_STATE_ERROR,
                             wsErrorState)
        == ERROR)
    {
        radMsgLog (PRI_HIGH, "stationInit: radStatesAddHandler failed");
        radStatesExit (ws2300WorkData.stateMachine);
        (*(work->medium.exit)) (&work->medium);
        return ERROR;
    }

    radStatesSetState (ws2300WorkData.stateMachine, WS_STATE_STARTPROC);

    radMsgLog (PRI_STATUS, "Starting station interface: WS2300"); 

    // dummy up a stimulus to get the state machine running
    stim.type = STIM_DUMMY;
    radStatesProcess (ws2300WorkData.stateMachine, &stim);

    return OK;
}

// station-supplied exit function
//
// Returns: N/A
//
void stationExit (WVIEWD_WORK *work)
{
    ws2300Exit (work);
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
    work->elevation     = (int16_t)ws2300WorkData.elevation;
    if (ws2300WorkData.latitude >= 0)
        work->latitude      = (int16_t)((ws2300WorkData.latitude*10)+0.5);
    else
        work->latitude      = (int16_t)((ws2300WorkData.latitude*10)-0.5);
    if (ws2300WorkData.longitude >= 0)
        work->longitude     = (int16_t)((ws2300WorkData.longitude*10)+0.5);
    else
        work->longitude     = (int16_t)((ws2300WorkData.longitude*10)-0.5);

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
    // We don't use the WS-2300 time...
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
    STIM        stim;

    stim.type = WS_STIM_READINGS;

    // use the VP_STIM_READINGS stim to kick off the state machine
    radStatesProcess (ws2300WorkData.stateMachine, &stim);

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
    // just indicate a NULL record, WS-2300 does not generate them (and this
    // function should never be called!)
    (*ArchiveIndicator) (NULL);
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
    STIM            stim;
    
    stim.type = STIM_IO;

    radStatesProcess (ws2300WorkData.stateMachine, &stim);
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
    STIM            stim;
    
    stim.type = STIM_TIMER;

    radStatesProcess (ws2300WorkData.stateMachine, &stim);
    return;
}

////////////****////  S T A T I O N   A P I   E N D  ////****////////////


//  ... ----- static (local) methods ----- ...

static void serialPortConfig (int fd)
{
    struct termios  port;
    int             portstatus;

    // We want full control of what is set and simply reset the entire port struct
    memset (&port, 0, sizeof(port));

    // Serial control options:
    port.c_cflag &= ~PARENB;            // No parity
    port.c_cflag &= ~CSTOPB;            // One stop bit
    port.c_cflag &= ~CSIZE;             // Character size mask
    port.c_cflag |= CS8;                // Character size 8 bits
    port.c_cflag |= CREAD;              // Enable Receiver
    port.c_cflag &= ~HUPCL;             // No "hangup"
    port.c_cflag &= ~CRTSCTS;           // No flowcontrol
    port.c_cflag |= CLOCAL;             // Ignore modem control lines

    // Baudrate, for newer systems
    cfsetispeed (&port, B2400);
    cfsetospeed (&port, B2400);

    // Serial local options:
    // Raw input = clear ICANON, ECHO, ECHOE, and ISIG
    // Disable misc other local features = clear FLUSHO, NOFLSH, TOSTOP, PENDIN, and IEXTEN
    // So we actually clear all flags in port.c_lflag
    port.c_lflag = 0;

    // Serial input options:
    // Disable parity check = clear INPCK, PARMRK, and ISTRIP
    // Disable software flow control = clear IXON, IXOFF, and IXANY
    // Disable any translation of CR and LF = clear INLCR, IGNCR, and ICRNL
    // Ignore break condition on input = set IGNBRK
    // Ignore parity errors just in case = set IGNPAR;
    // So we can clear all flags except IGNBRK and IGNPAR
    port.c_iflag = IGNBRK|IGNPAR;

    // Serial output options:
    // Raw output should disable all other output options
    port.c_oflag &= ~OPOST;

    port.c_cc[VTIME] = 10;              // timer 1s
    port.c_cc[VMIN] = 0;                // blocking read until 1 char

    tcsetattr (fd, TCSANOW, &port);
    tcflush (fd, TCIOFLUSH);

    // Set DTR low and RTS high and leave other ctrl lines untouched
    ioctl (fd, TIOCMGET, &portstatus);  // get current port status
    portstatus &= ~TIOCM_DTR;
    portstatus |= TIOCM_RTS;
    ioctl (fd, TIOCMSET, &portstatus);  // set current port status

    return;
}

