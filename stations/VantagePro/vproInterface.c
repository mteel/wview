/*---------------------------------------------------------------------------

  FILENAME:
        vproInterface.c

  PURPOSE:
        Provide the non-medium-specific interface utilities.

  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        06/06/2005      M.S. Teel       0               Original
        04/12/2008      W. Krenn        1               vpifGetRainCollectorSize

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

/*  ... Local include files
*/
#include <vproInterface.h>
#include <Ccitt.h>

/*  ... global memory declarations
*/

/*  ... local memory
*/

#ifdef WORDS_BIGENDIAN
#define SHORT_SWAP(x)           (((x << 8) & 0xFF00) | ((x >> 8) & 0x00FF))
#else
#define SHORT_SWAP(x)           (x)
#endif

static VP_IF_DATA   vpWorkData;
static WV_ACCUM_ID  vp12HourTempAvg;

static void portConfig (int fd);
static void (*ArchiveIndicator) (ARCHIVE_PKT *newRecord);


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
    STIM            stim;
    ARCHIVE_PKT     newestRecord;
    time_t          nowTime = time(NULL) - (WV_SECONDS_IN_HOUR * 12);
    ARCHIVE_PKT     recordStore;

    memset (&vpWorkData, 0, sizeof(vpWorkData));
    vpWorkData.sampleRain = -1;
    vpWorkData.sampleET = -1;

    // save the archive indication callback
    ArchiveIndicator = archiveIndication;

    // set our work data pointer
    work->stationData = &vpWorkData;

    // This is now read from configuration...
    // work->stationGeneratesArchives = TRUE;


    // initialize the medium abstraction based on user configuration
    if (!strcmp (work->stationInterface, "serial"))
    {
        if (serialMediumInit (&work->medium, portConfig, O_RDWR | O_NOCTTY | O_NDELAY) == ERROR)
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

    // initialize the VP interface using the media specific routine
    if ((*(work->medium.init)) (&work->medium, work->stationDevice) == ERROR)
    {
        radMsgLog (PRI_HIGH, "stationInit: medium setup failed");
        return ERROR;
    }

    vpifWakeupConsole (work);
    vpifWakeupConsole (work);


#ifndef _VP_CONFIG_ONLY
    if (!strcmp (work->stationInterface, "serial"))
    {
        radMsgLog (PRI_STATUS, "Vantage Pro on %s opened ...",
                   work->stationDevice);
    }
    else if (!strcmp (work->stationInterface, "ethernet"))
    {
        radMsgLog (PRI_STATUS, "Vantage Pro on %s:%d opened ...",
                   work->stationHost, work->stationPort);
    }

    // get VP-specific configuration
    if (stationGetConfigValueBoolean (work,
                                      configItem_STATION_DO_RXCHECK,
                                      &vpWorkData.doRXCheck)
        == ERROR)
    {
        // just default to disabled
        radMsgLog(PRI_MEDIUM, "stationInit: can't retrieve rxcheck configuration - defaulting to OFF");
        vpWorkData.doRXCheck = 0;
    }
    else
    {
        radMsgLog(PRI_MEDIUM, "stationInit: VP rxcheck is %s", 
                  ((vpWorkData.doRXCheck) ? "ENABLED" : "DISABLED"));
    }

    // This must be done here so dmpafter will work:
    work->archiveDateTime = dbsqliteArchiveGetNewestTime(&newestRecord);
    if ((int)work->archiveDateTime == ERROR)
    {
        work->archiveDateTime = time(NULL) - WV_SECONDS_IN_MONTH;
        radMsgLog (PRI_STATUS, "stationInit: no archive records found in database!");
    }

    // now initialize the state machine
    vpWorkData.stateMachine = radStatesInit (work);
    if (vpWorkData.stateMachine == NULL)
    {
        radMsgLog (PRI_HIGH, "stationInit: radStatesInit failed");
        (*(work->medium.exit)) (&work->medium);
        return ERROR;
    }

    if (radStatesAddHandler (vpWorkData.stateMachine,
                             VPRO_STATE_STARTPROC,
                             vproStartProcState)
        == ERROR)
    {
        radMsgLog (PRI_HIGH, "stationInit: radStatesAddHandler failed");
        radStatesExit (vpWorkData.stateMachine);
        (*(work->medium.exit)) (&work->medium);
        return ERROR;
    }
    if (radStatesAddHandler (vpWorkData.stateMachine,
                             VPRO_STATE_RUN,
                             vproRunState)
        == ERROR)
    {
        radMsgLog (PRI_HIGH, "stationInit: radStatesAddHandler failed");
        radStatesExit (vpWorkData.stateMachine);
        (*(work->medium.exit)) (&work->medium);
        return ERROR;
    }
    if (radStatesAddHandler (vpWorkData.stateMachine,
                             VPRO_STATE_DMPAFT_RQST,
                             vproDumpAfterState)
        == ERROR)
    {
        radMsgLog (PRI_HIGH, "stationInit: radStatesAddHandler failed");
        radStatesExit (vpWorkData.stateMachine);
        (*(work->medium.exit)) (&work->medium);
        return ERROR;
    }
    if (radStatesAddHandler (vpWorkData.stateMachine,
                             VPRO_STATE_DMPAFT_ACK,
                             vproDumpAfterAckState)
        == ERROR)
    {
        radMsgLog (PRI_HIGH, "stationInit: radStatesAddHandler failed");
        radStatesExit (vpWorkData.stateMachine);
        (*(work->medium.exit)) (&work->medium);
        return ERROR;
    }
    if (radStatesAddHandler (vpWorkData.stateMachine,
                             VPRO_STATE_RECV_ARCH,
                             vproReceiveArchiveState)
        == ERROR)
    {
        radMsgLog (PRI_HIGH, "stationInit: radStatesAddHandler failed");
        radStatesExit (vpWorkData.stateMachine);
        (*(work->medium.exit)) (&work->medium);
        return ERROR;
    }
    if (radStatesAddHandler (vpWorkData.stateMachine,
                             VPRO_STATE_LOOP_RQST,
                             vproLoopState)
        == ERROR)
    {
        radMsgLog (PRI_HIGH, "stationInit: radStatesAddHandler failed");
        radStatesExit (vpWorkData.stateMachine);
        (*(work->medium.exit)) (&work->medium);
        return ERROR;
    }
    if (radStatesAddHandler (vpWorkData.stateMachine,
                             VPRO_STATE_READ_RECOVER,
                             vproReadRecoverState)
        == ERROR)
    {
        radMsgLog (PRI_HIGH, "stationInit: radStatesAddHandler failed");
        radStatesExit (vpWorkData.stateMachine);
        (*(work->medium.exit)) (&work->medium);
        return ERROR;
    }
    if (radStatesAddHandler (vpWorkData.stateMachine,
                             VPRO_STATE_ERROR,
                             vproErrorState)
        == ERROR)
    {
        radMsgLog (PRI_HIGH, "stationInit: radStatesAddHandler failed");
        radStatesExit (vpWorkData.stateMachine);
        (*(work->medium.exit)) (&work->medium);
        return ERROR;
    }

    radStatesSetState (vpWorkData.stateMachine, VPRO_STATE_STARTPROC);

    // Initialize the 12-hour temp accumulator:
    vp12HourTempAvg = sensorAccumInit(60 * 12);

    // Load data for the last 12 hours:
    while ((nowTime = dbsqliteArchiveGetNextRecord(nowTime, &recordStore)) != ERROR)
    {
        sensorAccumAddSample(vp12HourTempAvg, 
                             recordStore.dateTime, 
                             recordStore.value[DATA_INDEX_outTemp]);
    }

    radMsgLog (PRI_STATUS, "Starting station interface: VantagePro"); 

    // dummy up a stimulus to get the state machine running
    stim.type = STIM_DUMMY;
    radStatesProcess (vpWorkData.stateMachine, &stim);

#endif

    return OK;
}

// station-supplied exit function
//
// Returns: N/A
//
void stationExit (WVIEWD_WORK *work)
{
    radStatesExit (vpWorkData.stateMachine);
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
    //  wakeup the console
    if (vpifWakeupConsole (work) == ERROR)
    {
        radMsgLog (PRI_HIGH, "stationGetPosition: WAKEUP failed");
        return ERROR;
    }

    // get the station latitude and longitude and elevation
    if (vpifGetLatandLong (work) == ERROR)
    {
        radMsgLog (PRI_HIGH, "stationGetPosition: failed");
        return ERROR;
    }
    else
    {
        vpWorkData.elevation = work->elevation;

        radMsgLog (PRI_STATUS, "station location: elevation: %d feet",
                   work->elevation);

        radMsgLog (PRI_STATUS, "station location: latitude: %3.1f %c  longitude: %3.1f %c",
                   (float)abs(work->latitude)/10.0,
                   ((work->latitude < 0) ? 'S' : 'N'),
                   (float)abs(work->longitude)/10.0,
                   ((work->longitude < 0) ? 'W' : 'E'));

        return OK;
    }
}

// station-supplied function to indicate a time sync should be performed if the
// station maintains time, otherwise may be safely ignored
// -- Can Be Asynchronous --
//
// Returns: OK or ERROR
//
int stationSyncTime (WVIEWD_WORK *work)
{
    // let state machine do it when convenient
    vpWorkData.timeSyncFlag = TRUE;

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

    stim.type = VP_STIM_READINGS;

    // use the VP_STIM_READINGS stim to kick off the state machine
    radStatesProcess (vpWorkData.stateMachine, &stim);

    return OK;
}

// station-supplied function to indicate an archive record should be generated -
// MUST populate an ARCHIVE_PKT struct and indicate it to 'archiveIndication'
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
    STIM            stim;

    stim.type = VP_STIM_ARCHIVE;

    // use the VP_STIM_ARCHIVE stim to kick off the state machine
    radStatesProcess (vpWorkData.stateMachine, &stim);

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

    radStatesProcess (vpWorkData.stateMachine, &stim);
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

    radStatesProcess (vpWorkData.stateMachine, &stim);

    return;
}

////////////****////  S T A T I O N   A P I   E N D  ////****////////////


/*  ... ----- static (local) methods -----
*/
static void portConfig (int fd)
{
    struct termios  port, tty, old;
    int             portstatus;

    memset (&port, 0, sizeof(port));

    // Serial control options:
    port.c_cflag &= ~PARENB;      // No parity
    port.c_cflag &= ~CSTOPB;      // One stop bit
    port.c_cflag &= ~CSIZE;       // Character size mask
    port.c_cflag |= CS8;          // Character size 8 bits
    port.c_cflag |= CREAD;        // Enable Receiver
    port.c_cflag &= ~HUPCL;       // No "hangup"
    port.c_cflag |= CLOCAL;       // Ignore modem control lines

    cfsetispeed (&port, B19200);
    cfsetospeed (&port, B19200);

    // Serial local options:
    // Raw input = clear ICANON, ECHO, ECHOE, and ISIG
    // Disable misc other local features = clear FLUSHO, NOFLSH, TOSTOP, PENDIN, and IEXTEN
    // So we actually clear all flags in adtio.c_lflag
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
    tcflush(fd, TCIOFLUSH);

    return;
}


#ifndef _VP_CONFIG_ONLY

#ifdef WORDS_BIGENDIAN
static void swapArchiveRecord (ARCHIVE_RECORD *rec)
{
    rec->date = SHORT_SWAP(rec->date);
    rec->time = SHORT_SWAP(rec->time);
    rec->outTemp = SHORT_SWAP(rec->outTemp);
    rec->highOutTemp = SHORT_SWAP(rec->highOutTemp);
    rec->lowOutTemp = SHORT_SWAP(rec->lowOutTemp);
    rec->rain = SHORT_SWAP(rec->rain);
    rec->highRainRate = SHORT_SWAP(rec->highRainRate);
    rec->barometer = SHORT_SWAP(rec->barometer);
    rec->radiation = SHORT_SWAP(rec->radiation);
    rec->windSamples = SHORT_SWAP(rec->windSamples);
    rec->inTemp = SHORT_SWAP(rec->inTemp);

    return;
}
#else
static void swapArchiveRecord (ARCHIVE_RECORD *rec)
{
    return;
}
#endif


//  Calculate the "e ^ (-mgh/RT)" term for pressure conversions:
static double calcPressureTerm(float tempF, float elevationFT)
{
    double      exponent;
    double      elevMeters = (double)wvutilsConvertFeetToMeters(elevationFT);
    double      tempKelvin = (double)wvutilsConvertFToC(tempF) + 273.15;

    // e ^ -elevMeters/(tempK * 29.263)
    exponent = (-elevMeters);

    // degrees Kelvin (T)
    exponent /= (tempKelvin * 29.263);

    // e ^ (-mgh/RT)
    exponent = exp(exponent);

    return exponent;
}

//  calculate station pressure from sea level pressure (using 12-hour temp avg):
static float vpConvertSLPToSP(float SLP, float tempF, float elevationFT)
{
    double      SP, PT;

    // Formula used: SP = SLP * PressureTerm
    // compute PressureTerm:
    PT = calcPressureTerm (tempF, elevationFT);
    SP = SLP * PT;

    return (float)SP;
}


static void convertToArchivePkt(WVIEWD_WORK *work, ARCHIVE_RECORD* newRecord, ARCHIVE_PKT* archivePkt)
{
    struct tm       bknTime;
    Data_Indices    index;

    // create the time_t time for the record:
    bknTime.tm_year  = EXTRACT_PACKED_YEAR(newRecord->date) - 1900;
    bknTime.tm_mon   = EXTRACT_PACKED_MONTH(newRecord->date) - 1;
    bknTime.tm_mday  = EXTRACT_PACKED_DAY(newRecord->date);
    bknTime.tm_hour  = EXTRACT_PACKED_HOUR(newRecord->time);
    bknTime.tm_min   = EXTRACT_PACKED_MINUTE(newRecord->time);
    bknTime.tm_sec   = 0;
    bknTime.tm_isdst = -1;

    archivePkt->dateTime = (int32_t)mktime(&bknTime);
    archivePkt->usUnits  = 1;
    archivePkt->interval = work->archiveInterval;

    // Set all values to NULL by default:
    for (index = DATA_INDEX_barometer; index < DATA_INDEX_MAX; index ++)
    {
        archivePkt->value[index] = ARCHIVE_VALUE_NULL;
    }

    // Set the values we can:
    if (-1500 < newRecord->outTemp && newRecord->outTemp < 1500)
        archivePkt->value[DATA_INDEX_outTemp]        = (float)newRecord->outTemp/10.0;
    if (1000 < newRecord->barometer && newRecord->barometer < 40000)
    {
        archivePkt->value[DATA_INDEX_barometer]      = (float)newRecord->barometer/1000.0;
        archivePkt->value[DATA_INDEX_pressure]       
            = vpConvertSLPToSP((float)archivePkt->value[DATA_INDEX_barometer],
                               sensorAccumGetAverage(vp12HourTempAvg),
                               (float)vpWorkData.elevation);
        archivePkt->value[DATA_INDEX_altimeter]      
            = wvutilsConvertSPToAltimeter((float)archivePkt->value[DATA_INDEX_pressure],
                                          (float)vpWorkData.elevation);
    }
    if (newRecord->inTemp < 2000)
        archivePkt->value[DATA_INDEX_inTemp]        = (float)newRecord->inTemp/10.0;
    if (newRecord->inHumidity <= 100)
        archivePkt->value[DATA_INDEX_inHumidity]    = (float)newRecord->inHumidity;
    if (newRecord->outHumidity <= 100)
        archivePkt->value[DATA_INDEX_outHumidity]   = (float)newRecord->outHumidity;
    if (newRecord->avgWindSpeed <= 250)
        archivePkt->value[DATA_INDEX_windSpeed]     = (float)newRecord->avgWindSpeed;
    if (newRecord->prevWindDir < 16)
        archivePkt->value[DATA_INDEX_windDir]       = (float)newRecord->prevWindDir * 22.5;
    if (newRecord->highWindSpeed <= 250)
        archivePkt->value[DATA_INDEX_windGust]      = (float)newRecord->highWindSpeed;
    if (newRecord->highWindDir < 16)
        archivePkt->value[DATA_INDEX_windGustDir]   = (float)newRecord->highWindDir * 22.5;
    archivePkt->value[DATA_INDEX_rainRate]       
        = (float)newRecord->highRainRate/vpWorkData.rainTicksPerInch;
    archivePkt->value[DATA_INDEX_rain]           
        = ((float)(newRecord->rain & 0xFFF))/vpWorkData.rainTicksPerInch;
    if (archivePkt->value[DATA_INDEX_outTemp] != ARCHIVE_VALUE_NULL &&
        archivePkt->value[DATA_INDEX_outHumidity] != ARCHIVE_VALUE_NULL)
    {
        archivePkt->value[DATA_INDEX_dewpoint]       
            = wvutilsCalculateDewpoint ((float)archivePkt->value[DATA_INDEX_outTemp],
                                        (float)archivePkt->value[DATA_INDEX_outHumidity]);
        archivePkt->value[DATA_INDEX_heatindex]      
            = wvutilsCalculateHeatIndex ((float)archivePkt->value[DATA_INDEX_outTemp],
                                         (float)archivePkt->value[DATA_INDEX_outHumidity]);
    }
    if (archivePkt->value[DATA_INDEX_outTemp] != ARCHIVE_VALUE_NULL &&
        archivePkt->value[DATA_INDEX_windSpeed] != ARCHIVE_VALUE_NULL)
    {
        archivePkt->value[DATA_INDEX_windchill]      
            = wvutilsCalculateWindChill ((float)archivePkt->value[DATA_INDEX_outTemp],
                                         (float)archivePkt->value[DATA_INDEX_windSpeed]);
    }
    if (newRecord->ET != 0xFF)
        archivePkt->value[DATA_INDEX_ET]            = (float)newRecord->ET/1000.0;
    if ((uint16_t)newRecord->radiation != 0x7FFF && 
        (uint16_t)newRecord->radiation != 0xFFFF && 
        (float)newRecord->radiation >= 0 && 
        (float)newRecord->radiation <= 1800)
    {
        archivePkt->value[DATA_INDEX_radiation]  = (float)newRecord->radiation;
    }
    if (newRecord->UV != 0xFF)
    {
        archivePkt->value[DATA_INDEX_UV]         = (float)newRecord->UV/10.0;
    }
    if (newRecord->extraTemp1 != 0xFF)
    {
        archivePkt->value[DATA_INDEX_extraTemp1] = (float)(newRecord->extraTemp1 - 90);
    }
    if (newRecord->extraTemp2 != 0xFF)
    {
        archivePkt->value[DATA_INDEX_extraTemp2] = (float)(newRecord->extraTemp2 - 90);
    }
    if (newRecord->extraTemp3 != 0xFF)
    {
        archivePkt->value[DATA_INDEX_extraTemp3] = (float)(newRecord->extraTemp3 - 90);
    }
    if (newRecord->soilTemp1 != 0xFF)
    {
        archivePkt->value[DATA_INDEX_soilTemp1]  = (float)(newRecord->soilTemp1 - 90);
    }
    if (newRecord->soilTemp2 != 0xFF)
    {
        archivePkt->value[DATA_INDEX_soilTemp2]  = (float)(newRecord->soilTemp2 - 90);
    }
    if (newRecord->soilTemp3 != 0xFF)
    {
        archivePkt->value[DATA_INDEX_soilTemp3]  = (float)(newRecord->soilTemp3 - 90);
    }
    if (newRecord->soilTemp4 != 0xFF)
    {
        archivePkt->value[DATA_INDEX_soilTemp4]  = (float)(newRecord->soilTemp4 - 90);
    }
    if (newRecord->leafTemp1 != 0xFF)
    {
        archivePkt->value[DATA_INDEX_leafTemp1]  = (float)(newRecord->leafTemp1 - 90);
    }
    if (newRecord->leafTemp2 != 0xFF)
    {
        archivePkt->value[DATA_INDEX_leafTemp2]  = (float)(newRecord->leafTemp2 - 90);
    }
    if (newRecord->extraHumid1 != 0xFF)
    {
        archivePkt->value[DATA_INDEX_extraHumid1] = (float)newRecord->extraHumid1;
    }
    if (newRecord->extraHumid2 != 0xFF)
    {
        archivePkt->value[DATA_INDEX_extraHumid2] = (float)newRecord->extraHumid2;
    }
    if (newRecord->soilMoist1 != 0xFF)
    {
        archivePkt->value[DATA_INDEX_soilMoist1] = (float)newRecord->soilMoist1;
    }
    if (newRecord->soilMoist2 != 0xFF)
    {
        archivePkt->value[DATA_INDEX_soilMoist2] = (float)newRecord->soilMoist2;
    }
    if (newRecord->soilMoist3 != 0xFF)
    {
        archivePkt->value[DATA_INDEX_soilMoist3] = (float)newRecord->soilMoist3;
    }
    if (newRecord->soilMoist4 != 0xFF)
    {
        archivePkt->value[DATA_INDEX_soilMoist4] = (float)newRecord->soilMoist4;
    }
    if (newRecord->leafWet1 != 0xFF)
    {
        archivePkt->value[DATA_INDEX_leafWet1]   = (float)newRecord->leafWet1;
    }
    if (newRecord->leafWet2 != 0xFF)
    {
        archivePkt->value[DATA_INDEX_leafWet2]   = (float)newRecord->leafWet2;
    }

    if (work->loopPkt.rxCheckPercent != 0xFFFF)
        archivePkt->value[DATA_INDEX_rxCheckPercent] = (float)work->loopPkt.rxCheckPercent;
    archivePkt->value[DATA_INDEX_txBatteryStatus] = (float)work->loopPkt.txBatteryStatus;
    archivePkt->value[DATA_INDEX_consBatteryVoltage] = (float)work->loopPkt.consBatteryVoltage;

    return;
}

static void storeLoopPkt (WVIEWD_WORK *work, LOOP_DATA *src)
{
    LOOP_PKT*   dest = &(work->loopPkt);
    uint16_t    tempshort;
    int         intTemp;

    // Clear optional data:
    stationClearLoopData(work);


    // do this magic to track rainfall and ET per LOOP sample
	intTemp = (int)SHORT_SWAP(src->dayRain);

    if (vpWorkData.sampleRain == -1 && intTemp != 0xFFFF && intTemp != 0x7FFF)
    {
        vpWorkData.sampleRain = intTemp;
        dest->sampleRain = 0.0;
    }
    else if (intTemp != 0xFFFF && intTemp != 0x7FFF)
    {
        if (intTemp - vpWorkData.sampleRain >= 0)
        {
            dest->sampleRain = intTemp - vpWorkData.sampleRain;
            dest->sampleRain /= vpWorkData.rainTicksPerInch;
        }
        else
        {
            // we had a day rollover, all rain must have come in this sample
            dest->sampleRain = intTemp;
            dest->sampleRain /= vpWorkData.rainTicksPerInch;
        }
        vpWorkData.sampleRain = intTemp;
    }
    else
    {
        dest->sampleRain = 0.0;
    }

    intTemp = (int)SHORT_SWAP(src->dayET);
    if (vpWorkData.sampleET == -1 && intTemp != 0xFFFF && intTemp != 0x7FFF)
    {
        vpWorkData.sampleET = intTemp;
        dest->sampleET = 0.0;
    }
    else if (intTemp != 0xFFFF && intTemp != 0x7FFF)
    {
        if (intTemp - vpWorkData.sampleET >= 0)
        {
            dest->sampleET = intTemp - vpWorkData.sampleET;
            dest->sampleET /= 1000.0;
        }
        else
        {
            dest->sampleET = intTemp;
            dest->sampleET /= 1000.0;
        }
        vpWorkData.sampleET = intTemp;
    }
    else
    {
        dest->sampleET = 0.0;
    }


    // store the RX Check value here
    dest->rxCheckPercent = vpWorkData.rxCheckPercent;

    src->outTemp = SHORT_SWAP(src->outTemp);
    if (src->outTemp != 0x7FFF)
        dest->outTemp = (float)src->outTemp/10.0;

    // Pressure:
    if (src->barometer != 0xFFFF)
    {
        // VP produces sea level pressure (SLP):
        tempshort = SHORT_SWAP(src->barometer);
        dest->barometer = (float)tempshort/1000.0;

        // Apply calibration here so the computed values reflect it:
        dest->barometer *= work->calMBarometer;
        dest->barometer += work->calCBarometer;

        // calculate station pressure:
        dest->stationPressure = vpConvertSLPToSP(dest->barometer, 
                                                 sensorAccumGetAverage(vp12HourTempAvg), 
                                                 (float)vpWorkData.elevation);

        // now calculate altimeter:
        dest->altimeter = wvutilsConvertSPToAltimeter(dest->stationPressure,
                                                      (float)vpWorkData.elevation);
    }

    src->inTemp = SHORT_SWAP(src->inTemp);
    if (src->inTemp != 0x7FFF)
        dest->inTemp = (float)src->inTemp/10.0;
    if (src->inHumidity != 0xFF)
        dest->inHumidity = src->inHumidity;
    if (src->outHumidity != 0xFF)
        dest->outHumidity = src->outHumidity;
    if (src->windSpeed != 0xFF)
    {
        dest->windSpeedF = (float)src->windSpeed;
        dest->windGustF = (float)src->windSpeed;
    }
    tempshort = SHORT_SWAP(src->windDir);
    if (tempshort != 0xFFFF && tempshort != 0x7FFF)
    {
        dest->windDir = tempshort;
        dest->windGustDir = tempshort;
    }
    tempshort = SHORT_SWAP(src->rainRate);
    if (tempshort != 0xFFFF && tempshort != 0x7FFF)
    {
	    dest->rainRate = (float)tempshort/vpWorkData.rainTicksPerInch;
    }
    if (src->UV != 0xFF)
        dest->UV = (float)src->UV/10;
    else
        dest->UV = -1;

    tempshort = SHORT_SWAP(src->radiation);
    if (tempshort != 0x7FFF && tempshort != 0xFFFF && tempshort <= 1800)
        dest->radiation = tempshort;
    else
        dest->radiation = 0xFFFF;

    if (src->tenMinuteAvgWindSpeed != 0xFF)
        dest->tenMinuteAvgWindSpeed = src->tenMinuteAvgWindSpeed;

    dest->forecastIcon = src->forecastIcon;
    dest->forecastRule = src->forecastRule;
    dest->txBatteryStatus = src->txBatteryStatus;
    dest->consBatteryVoltage = SHORT_SWAP(src->consBatteryVoltage);
    dest->extraTemp[0] = (float)(src->extraTemp1 - 90);
    dest->extraTemp[1] = (float)(src->extraTemp2 - 90);
    dest->extraTemp[2] = (float)(src->extraTemp3 - 90);
    dest->soilTemp1 = (float)(src->soilTemp1 - 90);
    dest->soilTemp2 = (float)(src->soilTemp2 - 90);
    dest->soilTemp3 = (float)(src->soilTemp3 - 90);
    dest->soilTemp4 = (float)(src->soilTemp4 - 90);
    dest->leafTemp1 = (float)(src->leafTemp1 - 90);
    dest->leafTemp2 = (float)(src->leafTemp2 - 90);
    dest->extraHumidity[0] = src->extraHumid1;
    dest->extraHumidity[1] = src->extraHumid2;
    dest->soilMoist1 = src->soilMoist1;
    dest->soilMoist2 = src->soilMoist2;
    if (src->leafWet1 != 0xFF)
        dest->leafWet1 = src->leafWet1;
    if (src->leafWet2 != 0xFF)
        dest->leafWet2 = src->leafWet2;

    return;
}

static int processArchivePage (WVIEWD_WORK *work, ARCHIVE_PAGE *page)
{
    int                 i, start, tempInt, numNewRecords = 0;
    ARCHIVE_RECORD*     newRecord;
    ARCHIVE_PKT         archivePkt;
    float               tempf;
    uint16_t            date, ntime, tempRainBits;

    start = vpWorkData.archiveRecOffset;
    vpWorkData.archiveRecOffset = 0;

    for (i = start; i < 5; i ++)
    {
        //  ... swap shorts for big endian before we get started...
        swapArchiveRecord (&page->record[i]);

        date = INSERT_PACKED_DATE(wvutilsGetYear(work->archiveDateTime),
                                  wvutilsGetMonth(work->archiveDateTime),
                                  wvutilsGetDay(work->archiveDateTime));
        ntime = (100 * wvutilsGetHour(work->archiveDateTime)) + 
                wvutilsGetMin(work->archiveDateTime);

        if ((page->record[i].date < date) ||
            (page->record[i].date == date && page->record[i].time <= ntime) ||
            (page->record[i].date == 0xFFFF) ||
            (page->record[i].time == 0xFFFF))
        {
            //  ... this is an old record or uninitialized, skip it
            continue;
        }

        numNewRecords ++;

        // save the high wind speed
        work->loopPkt.windGustF = (float)page->record[i].highWindSpeed;

        // clear the retry flag here
        vpWorkData.archiveRetryFlag = FALSE;

        newRecord = &page->record[i];

        // Convert to the wview internal archive format:
        convertToArchivePkt(work, newRecord, &archivePkt);

        // Calibrate archive record contents:
        archivePkt.value[DATA_INDEX_barometer]        *= work->calMBarometer;
        archivePkt.value[DATA_INDEX_barometer]        += work->calCBarometer;

        archivePkt.value[DATA_INDEX_inTemp]           *= work->calMInTemp;
        archivePkt.value[DATA_INDEX_inTemp]           += work->calCInTemp;

        archivePkt.value[DATA_INDEX_outTemp]          *= work->calMOutTemp;
        archivePkt.value[DATA_INDEX_outTemp]          += work->calCOutTemp;

        archivePkt.value[DATA_INDEX_inHumidity]       *= work->calMInHumidity;
        archivePkt.value[DATA_INDEX_inHumidity]       += work->calCInHumidity;
        if (archivePkt.value[DATA_INDEX_inHumidity] > 100)
        {
            archivePkt.value[DATA_INDEX_inHumidity] = 100;
        }

        archivePkt.value[DATA_INDEX_outHumidity]      *= work->calMOutHumidity;
        archivePkt.value[DATA_INDEX_outHumidity]      += work->calCOutHumidity;
        if (archivePkt.value[DATA_INDEX_outHumidity] > 100)
        {
            archivePkt.value[DATA_INDEX_outHumidity] = 100;
        }

        archivePkt.value[DATA_INDEX_windSpeed]          *= work->calMWindSpeed;
        archivePkt.value[DATA_INDEX_windSpeed]          += work->calCWindSpeed;

        if (archivePkt.value[DATA_INDEX_windDir] > ARCHIVE_VALUE_NULL)
        {
            archivePkt.value[DATA_INDEX_windDir]          *= work->calMWindDir;
            archivePkt.value[DATA_INDEX_windDir]          += work->calCWindDir;
            if (archivePkt.value[DATA_INDEX_windDir] < 0)
            {
                archivePkt.value[DATA_INDEX_windDir] += 360;
            }
            if (archivePkt.value[DATA_INDEX_windDir] > 360)
            {
                archivePkt.value[DATA_INDEX_windDir] -= 360;
            }
        }
        archivePkt.value[DATA_INDEX_rain]          *= work->calMRain;
        archivePkt.value[DATA_INDEX_rain]          += work->calCRain;
        if (archivePkt.value[DATA_INDEX_rain] < 0)
        {
            archivePkt.value[DATA_INDEX_rain] = 0;
        }

        archivePkt.value[DATA_INDEX_rainRate]          *= work->calMRainRate;
        archivePkt.value[DATA_INDEX_rainRate]          += work->calCRainRate;
        if (archivePkt.value[DATA_INDEX_rainRate] < 0)
        {
            archivePkt.value[DATA_INDEX_rainRate] = 0;
        }

        // Add for the 12-hour temp average:
        sensorAccumAddSample(vp12HourTempAvg, archivePkt.dateTime, archivePkt.value[DATA_INDEX_outTemp]);

        // indicate through the station API:
        (*ArchiveIndicator) (&archivePkt);

        // If not running yet, add to HILOW database:
        if (!work->runningFlag)
        {
            dbsqliteHiLowStoreArchive(&archivePkt);
        }
        else
        {
            // Add all but cumulative:
            dbsqliteHiLowUpdateArchive(&archivePkt);
        }
    }

    vpWorkData.archiveCurrentPage ++;
    return numNewRecords;
}
#endif


static uint16_t genCRC (void *data, int length)
{
    uint8_t         *ptr = (uint8_t *)data;
    uint16_t        crc = 0;
    register int    i;

    for (i = 0; i < length; i ++)
    {
        crc =  crc_table[(crc >> 8) ^ ptr[i]] ^ (crc << 8);
    }

    crc = ((crc >> 8) & 0xFF) | ((crc << 8) & 0xFF00);
    return crc;
}

static int readWithCRC (WVIEWD_WORK *work, void *bfr, int len, int msTimeout)
{
    int         retVal, index = 0;
    uint8_t     *ptr = (uint8_t *)bfr;
    uint16_t    crc = 0;

    retVal = (*work->medium.read) (&work->medium, bfr, len, msTimeout);
    if (retVal != len)
    {
        return ERROR;
    }

    for (index = 0; index < len; index ++)
    {
        crc =  crc_table [(crc >> 8) ^ ptr[index]] ^ (crc << 8);
    }

    return (crc == 0) ? (len) : (ERROR);
}

static int writeWithCRC (WVIEWD_WORK *work, void *buffer, int length)
{
    int         retVal;
    int         wrerrno = 0;
    uint16_t    crc;

    crc = SHORT_SWAP(genCRC (buffer, length));

    retVal = (*work->medium.write) (&work->medium, buffer, length);
    if (retVal != length)
    {
        if (retVal == -1)
        {
            wrerrno = errno;
        }

        return ERROR;
    }
    retVal = (*work->medium.write) (&work->medium, &crc, 2);
    if (retVal != 2)
    {
        if (retVal == -1)
        {
            wrerrno = errno;
        }

        return ERROR;
    }

    /*  ... now wait for the TX buffer to empty out (this blocks)
    */
    (*work->medium.txdrain) (&work->medium);

    return length;
}

static int wakeupConsole (WVIEWD_WORK *work)
{
    int         retVal;
    uint8_t     bfr[32];
    int         retries = 4;
 
    while (retries > 0)
    {
        bfr[0] = VP_CR;
        (*work->medium.flush) (&work->medium, WV_QUEUE_INPUT);
        retVal = (*work->medium.write) (&work->medium, bfr, 1);
        if (retVal != 1)
        {
            radMsgLog (PRI_MEDIUM, "wakeupConsole: Write ERROR!");
            return ERROR;
        }
    
        memset (bfr, 0, sizeof (bfr));

        // If this is a Weatherlink IP interface, allow more time for the wakeup:
        if (work->stationIsWLIP)
        {
            retVal = (*work->medium.read) (&work->medium, bfr, 2, 2000);
        }
        else if (retries == 4)
        {
            // Don't wait long on the first try for serial interfaces;
            // This is often ignored by the console:
            retVal = (*work->medium.read) (&work->medium, bfr, 2, 500);
        }
        else
        {
            // Wait longer after first try:
            retVal = (*work->medium.read) (&work->medium, bfr, 2, 1200);
        }

        if (retVal == 2 && bfr[0] == VP_LF && bfr[1] == VP_CR)
        {
            // good stuff:
            return OK;
        }
        else
        {
            // radMsgLog (PRI_MEDIUM, "wakeupConsole: bad read: retVal=%d", retVal);
        }

        retries -= 1;
    }

    // If here, try a restart (if the medium supports it):
    radMsgLog (PRI_MEDIUM, "wakeupConsole: failed");
    (*work->medium.restart) (&work->medium);
    return ERROR;
}



/*  ... ----- API methods -----
*/
void vpifIndicateStationUp (void)
{
    radProcessEventsSend (NULL, STATION_INIT_COMPLETE_EVENT, 0);
    return;
}

void vpifIndicateLoopDone (void)
{
    radProcessEventsSend (NULL, STATION_LOOP_COMPLETE_EVENT, 0);
    return;
}

void vpifFlush (WVIEWD_WORK *work)
{
    (*work->medium.flush) (&work->medium, WV_QUEUE_INPUT);
    return;
}

int vpifWakeupConsole (WVIEWD_WORK *work)
{
    vpifFlush (work);

    return wakeupConsole (work);
}

int vpifSynchronizeConsoleClock (WVIEWD_WORK *work)
{
    time_t          ntime;
    struct tm       locTime;
    int             gmtOffset;

    /*  ... wakeup the console
    */
    if (vpifWakeupConsole (work) == ERROR)
    {
        radMsgLog (PRI_HIGH, "vpifSynchronizeConsoleClock: WAKEUP1 failed");
        return ERROR;
    }

    /*  ... set the console GMT offset
    */
    if (vpifSetGMTOffset (work, &gmtOffset) == ERROR)
    {
        radMsgLog (PRI_HIGH, "vpifSynchronizeConsoleClock: GMT failed");
        return ERROR;
    }

    /*  ... wakeup the console
    */
    if (vpifWakeupConsole (work) == ERROR)
    {
        radMsgLog (PRI_HIGH, "vpifSynchronizeConsoleClock: WAKEUP2 failed");
        return ERROR;
    }

    /*  ... set the console date and time
    */
    ntime = time (NULL);
    localtime_r (&ntime, &locTime);

    if (vpifSetTime (work,
                       locTime.tm_year + 1900,
                       locTime.tm_mon + 1,
                       locTime.tm_mday,
                       locTime.tm_hour,
                       locTime.tm_min,
                       locTime.tm_sec)
        == ERROR)
    {
        radMsgLog (PRI_HIGH, "vpifSynchronizeConsoleClock: SETTIME failed");
        return ERROR;
    }

    radMsgLog (PRI_STATUS, "station time synchronized to: %2.2d-%2.2d-%4.4d %2.2d:%2.2d:%2.2d",
               locTime.tm_mon + 1, locTime.tm_mday, locTime.tm_year + 1900,
               locTime.tm_hour, locTime.tm_min, locTime.tm_sec);
    radMsgLog (PRI_STATUS, "station GMT offset synchronized to: %d hours, %d minutes",
               gmtOffset/60, gmtOffset%60);

    // the VP console is cranky after a time reset
    radUtilsSleep (1500);

    return OK;
}

int vpifGetTime
(
    WVIEWD_WORK *work,
    uint16_t    *year,
    uint16_t    *month,
    uint16_t    *day,
    uint16_t    *hour,
    uint16_t    *minute,
    uint16_t    *second
)
{
    int         retVal, len;
    uint8_t     bfr[32];

    strcpy ((char *)bfr, "GETTIME");
    len = strlen ((char *)bfr);
    bfr[len] = VP_LF;

    retVal = (*work->medium.write) (&work->medium, bfr, len+1);
    if (retVal != len+1)
    {
        return ERROR;
    }

    retVal = (*work->medium.read) (&work->medium, bfr, 1, 1000);
    if (retVal != 1)
    {
        return ERROR;
    }
    if (bfr[0] != VP_ACK)
    {
        return ERROR;
    }

    memset (bfr, 0, sizeof (bfr));
    retVal = readWithCRC (work, bfr, 8, 1000);
    if (retVal != 8)
    {
        return ERROR;
    }

    *second = bfr[0];
    *minute = bfr[1];
    *hour   = bfr[2];
    *day    = bfr[3];
    *month  = bfr[4];
    *year   = bfr[5] + 1900;

    return OK;
}

int vpifGetLatandLong
(
    WVIEWD_WORK *work
)
{
    int         retVal, len;
    int16_t     tmp[16];
    uint8_t     *bfr = (uint8_t *)tmp;

    strcpy ((char *)bfr, "EEBRD 0B 2");
    len = strlen ((char *)bfr);
    bfr[len] = VP_LF;

    if ((*work->medium.write) (&work->medium, bfr, len + 1) != len + 1)
    {
        return ERROR;
    }

    if (vpifGetAck (work, 2000) == ERROR)
    {
        (*work->medium.flush) (&work->medium, WV_QUEUE_INPUT);
        return ERROR;
    }

    memset (bfr, 0, sizeof (tmp));
    retVal = readWithCRC (work, bfr, 4, 2000);
    if (retVal != 4)
    {
        return ERROR;
    }

    work->latitude = SHORT_SWAP(tmp[0]);

    strcpy ((char *)bfr, "EEBRD 0D 2");
    len = strlen ((char *)bfr);
    bfr[len] = VP_LF;

    if ((*work->medium.write) (&work->medium, bfr, len + 1) != len + 1)
    {
        return ERROR;
    }

    if (vpifGetAck (work, 2000) == ERROR)
    {
        (*work->medium.flush) (&work->medium, WV_QUEUE_INPUT);
        return ERROR;
    }

    memset (bfr, 0, sizeof (tmp));
    retVal = readWithCRC (work, bfr, 4, 2000);
    if (retVal != 4)
    {
        return ERROR;
    }

    work->longitude = SHORT_SWAP(tmp[0]);

    strcpy ((char *)bfr, "EEBRD 0F 2");
    len = strlen ((char *)bfr);
    bfr[len] = VP_LF;

    if ((*work->medium.write) (&work->medium, bfr, len + 1) != len + 1)
    {
        return ERROR;
    }

    if (vpifGetAck (work, 2000) == ERROR)
    {
        (*work->medium.flush) (&work->medium, WV_QUEUE_INPUT);
        return ERROR;
    }

    memset (bfr, 0, sizeof (tmp));
    retVal = readWithCRC (work, bfr, 4, 2000);
    if (retVal != 4)
    {
        return ERROR;
    }

    work->elevation = SHORT_SWAP(tmp[0]);

    return OK;
}

int vpifGetArchiveInterval (WVIEWD_WORK *work)
{
    uint16_t            temp[32];                       // short align
    uint8_t             *ptr = (uint8_t *)temp;
    int                 len, retVal;
    ARCHIVE_INTERVAL    *interval = (ARCHIVE_INTERVAL *)temp;

    strcpy ((char *)ptr, "EEBRD 2D 1");
    len = strlen ((char *)ptr);
    ptr[len] = VP_LF;

    if ((*work->medium.write) (&work->medium, ptr, len + 1) != len + 1)
    {
        return ERROR;
    }

    if (vpifGetAck (work, 3000) == ERROR)
    {
        (*work->medium.flush) (&work->medium, WV_QUEUE_INPUT);
        return ERROR;
    }

    retVal = readWithCRC (work, interval, sizeof (ARCHIVE_INTERVAL), 3000);
    if (retVal != sizeof (ARCHIVE_INTERVAL))
    {
        (*work->medium.flush) (&work->medium, WV_QUEUE_INPUT);
        return ERROR;
    }

    work->archiveInterval = interval->interval;
    return OK;
}


int vpifGetRainCollectorSize (WVIEWD_WORK *work)
{
    int         retVal, len;
    int16_t     tmp[16];
    uint8_t     *bfr = (uint8_t *)tmp;
    float       tempfloat;

    strcpy ((char *)bfr, "EEBRD 2B 1");
    len = strlen ((char *)bfr);
    bfr[len] = VP_LF;

    if ((*work->medium.write) (&work->medium, bfr, len + 1) != len + 1)
    {
        return ERROR;
    }

    if (vpifGetAck (work, 2000) == ERROR)
    {
        (*work->medium.flush) (&work->medium, WV_QUEUE_INPUT);
        return ERROR;
    }

    memset (bfr, 0, sizeof (tmp));
    retVal = readWithCRC (work, bfr, 3, 2000);
    if (retVal != 3)
    {
        return ERROR;
    }

    retVal = bfr[0] & 0x30;

    if (retVal == 0x10)
    {
        vpWorkData.rainTicksPerInch = 127;              // 0.2 mm = 0.007874 in
        vpWorkData.RainCollectorType = 0x2000;
    }
    else if (retVal == 0x20)
    {
        vpWorkData.rainTicksPerInch = 254;              // 0.1 mm = 0.003937 in
        vpWorkData.RainCollectorType = 0x6000;
    }
    else
    {
        vpWorkData.rainTicksPerInch = 100;              // 0.01 in = 0.254 mm
        vpWorkData.RainCollectorType = 0x1000;
    }

    return OK;
}


int vpifSetTime
(
    WVIEWD_WORK *work,
    uint16_t      year,
    uint16_t      month,
    uint16_t      day,
    uint16_t      hour,
    uint16_t      minute,
    uint16_t      second
)
{
    int         retVal, len;
    uint16_t    usbfr[16];
    uint8_t     *bfr = (uint8_t *)usbfr;
    uint16_t    *crc;

    strcpy ((char *)bfr, "SETTIME");
    len = strlen ((char *)bfr);
    bfr[len] = VP_LF;

    retVal = (*work->medium.write) (&work->medium, bfr, len+1);
    if (retVal != len+1)
    {
        return ERROR;
    }

    if (vpifGetAck (work, 2000) == ERROR)
    {
        (*work->medium.flush) (&work->medium, WV_QUEUE_INPUT);
        return ERROR;
    }

    bfr[0]  = second;
    bfr[1]  = minute;
    bfr[2]  = hour;
    bfr[3]  = day;
    bfr[4]  = month;
    bfr[5]  = year - 1900;
    crc     = (uint16_t *)&bfr[6];
    *crc    = SHORT_SWAP(genCRC (bfr, 6));

    retVal = (*work->medium.write) (&work->medium, bfr, 8);
    if (retVal != 8)
    {
        return ERROR;
    }

    if (vpifGetAck (work, 2000) == ERROR)
    {
        (*work->medium.flush) (&work->medium, WV_QUEUE_INPUT);
        return ERROR;
    }

    return OK;
}

int vpifSetGMTOffset
(
    WVIEWD_WORK *work,
    int         *save
)
{
    int             retVal, len, gmtOffsetHours, gmtOffsetMinutes;
    int16_t         tmp[16];
    uint8_t         *bfr = (uint8_t *)tmp;
    time_t          nowtime = time (NULL);
    struct tm       bknTime;
    long            gmtMinsEast;

    localtime_r (&nowtime, &bknTime);

#ifdef HAVE_STRUCT_TM_TM_ZONE
    gmtMinsEast = bknTime.tm_gmtoff/60;
#else
    gmtMinsEast = -(timezone/60);
#endif

    gmtOffsetHours      = gmtMinsEast/60;
    gmtOffsetMinutes    = gmtMinsEast%60;
    *save               = gmtMinsEast;

    strcpy ((char *)bfr, "EEBWR 12 5");
    len = strlen ((char *)bfr);
    bfr[len] = VP_LF;

    if ((*work->medium.write) (&work->medium, bfr, len + 1) != len + 1)
    {
        return ERROR;
    }

    if (vpifGetAck (work, 2000) == ERROR)
    {
        (*work->medium.flush) (&work->medium, WV_QUEUE_INPUT);
        return ERROR;
    }

    memset (bfr, 0, 8);

    // set dst to manual and turn it off (it is reflected in the GMT offset)
    bfr[0] = 1;
    bfr[1] = 0;

    // set the GMT offset
    tmp[1] = SHORT_SWAP((int16_t)((gmtOffsetHours * 100) + gmtOffsetMinutes));

    // make sure the VP uses the GMT offset
    bfr[4] = 1;

    retVal = writeWithCRC (work, bfr, 5);
    if (retVal != 5)
    {
        return ERROR;
    }

    return OK;
}

/*  ... this guy reads/parses msgs and updates the internal data stores;
    ... returns OK or ERROR (except when ACK expected, then returns
    ... ACK, NAK, or ERROR)
*/
int vpifReadMessage (WVIEWD_WORK *work, int expectACK)
{
    uint16_t            temp[VP_BYTE_LENGTH_MAX/2];     // short align
    uint8_t             *chPtr = (uint8_t *)temp;
    ARCHIVE_PAGE        *arcRec = (ARCHIVE_PAGE *)temp;
    DMPAFT_HDR          *dmphdr = (DMPAFT_HDR *)temp;
    LOOP_DATA           *loop = (LOOP_DATA *)temp;
    int                 retVal;

    memset (temp, 0, sizeof(temp));
    switch (vpWorkData.reqMsgType)
    {
        case SER_MSG_ACK:
            retVal = (*work->medium.read) (&work->medium, chPtr, 1, 1000);
            if (retVal != 1)
            {
                return ERROR;
            }
            if (chPtr[0] == VP_ACK)
            {
                return VP_ACK;
            }
            else if (chPtr[0] == VP_NAK)
            {
                return VP_NAK;
            }
            else
            {
                return ERROR;
            }

#ifndef _VP_CONFIG_ONLY
        case SER_MSG_DMPAFT_HDR:
            if (expectACK)
            {
                if (vpifGetAck (work, 1000) == ERROR)
                {
                    (*work->medium.flush) (&work->medium, WV_QUEUE_INPUT);
                    return ERROR;
                }
            }
            retVal = readWithCRC (work, dmphdr, sizeof (DMPAFT_HDR), 3000);
            if (retVal != sizeof (DMPAFT_HDR))
            {
                (*work->medium.flush) (&work->medium, WV_QUEUE_INPUT);
                return ERROR;
            }

            vpWorkData.archivePages = SHORT_SWAP(dmphdr->pages);
            vpWorkData.archiveRecOffset = SHORT_SWAP(dmphdr->firstRecIndex);
            return OK;

        case SER_MSG_ARCHIVE:
            retVal = readWithCRC (work, arcRec, sizeof (ARCHIVE_PAGE), 5000);
            if (retVal != sizeof (ARCHIVE_PAGE))
            {
                (*work->medium.flush) (&work->medium, WV_QUEUE_INPUT);
                return ERROR;
            }

            /*  ... send ACK before processing to speed up download
            */
            if (vpWorkData.archiveCurrentPage < vpWorkData.archivePages - 1)
            {
                if (vpifSendAck (work) == ERROR)
                {
                    return ERROR;
                }
            }

            return (processArchivePage (work, arcRec));

        case SER_MSG_LOOP:
            if (expectACK)
            {
                if (vpifGetAck (work, 1000) == ERROR)
                {
                    (*work->medium.flush) (&work->medium, WV_QUEUE_INPUT);
                    return ERROR;
                }
            }
            retVal = readWithCRC (work, loop, sizeof (LOOP_DATA), 5000);
            if (retVal != sizeof (LOOP_DATA))
            {
                (*work->medium.flush) (&work->medium, WV_QUEUE_INPUT);
                return ERROR;
            }

            /*  ... store in IPM format
            */
            storeLoopPkt (work, loop);

            return OK;
#endif

        case SER_MSG_NONE:
        default:
            /*  ... just flush the receive buffer, we're not expecting anything!
            */
            (*work->medium.flush) (&work->medium, WV_QUEUE_INPUT);
            return OK;
    }
}

int vpifGetAck (WVIEWD_WORK *work, int msWait)
{
    uint8_t       temp[4];

    if ((*work->medium.read) (&work->medium, temp, 1, msWait) != 1)
    {
        (*work->medium.flush) (&work->medium, WV_QUEUE_INPUT);
        return ERROR;
    }

    // this "flushing" of CR/LF's takes care of many problems with terminal
    // servers who want to echo them or attach them to the end of packets -
    // while not bothering cleaner true serial interfaces...
    while (temp[0] == VP_LF || temp[0] == VP_CR)
    {
        // extra LF/CR, discard and read again
        if ((*work->medium.read) (&work->medium, temp, 1, msWait) != 1)
        {
            (*work->medium.flush) (&work->medium, WV_QUEUE_INPUT);
            return ERROR;
        }
    }

    if (temp[0] != VP_ACK)
    {
        (*work->medium.flush) (&work->medium, WV_QUEUE_INPUT);
        return ERROR;
    }

    return OK;
}


/*  ... send msgs to the console;
    ... will set the "reqMsgType" of the work area appropriately;
    ... returns OK or ERROR
*/
int vpifSendAck (WVIEWD_WORK *work)
{
    uint16_t              temp[32];                       // short align
    uint8_t               *ptr = (uint8_t *)temp;

    ptr[0] = VP_ACK;

    if ((*work->medium.write) (&work->medium, ptr, 1) != 1)
    {
        return ERROR;
    }

    return OK;
}

int vpifSendNak (WVIEWD_WORK *work)
{
    uint16_t              temp[32];                       // short align
    uint8_t               *ptr = (uint8_t *)temp;

    ptr[0] = VP_NAK;

    if ((*work->medium.write) (&work->medium, ptr, 1) != 1)
    {
        return ERROR;
    }

    return OK;
}

int vpifSendCancel (WVIEWD_WORK *work)
{
    uint16_t              temp[32];                       // short align
    uint8_t               *ptr = (uint8_t *)temp;

    ptr[0] = VP_CANCEL;

    if ((*work->medium.write) (&work->medium, ptr, 1) != 1)
    {
        return ERROR;
    }

    return OK;
}

int vpifSendDumpAfterRqst (WVIEWD_WORK *work)
{
    uint16_t            temp[32];                       // short align
    uint8_t             *ptr = (uint8_t *)temp;
    int                 len;

    strcpy ((char *)ptr, "DMPAFT");
    len = strlen ((char *)ptr);
    ptr[len] = VP_LF;

    if ((*work->medium.write) (&work->medium, ptr, len + 1) != len + 1)
    {
        return ERROR;
    }

    vpWorkData.reqMsgType = SER_MSG_ACK;
    return OK;
}

int vpifSendDumpDateTimeRqst (WVIEWD_WORK *work)
{
    uint16_t              temp[32];                       // short align
    uint16_t              date, ntime;

    date = INSERT_PACKED_DATE(wvutilsGetYear(work->archiveDateTime),
                              wvutilsGetMonth(work->archiveDateTime),
                              wvutilsGetDay(work->archiveDateTime));
    ntime = (100 * wvutilsGetHour(work->archiveDateTime)) + 
            wvutilsGetMin(work->archiveDateTime);

    temp[0] = SHORT_SWAP(date);
    temp[1] = SHORT_SWAP(ntime);
    temp[2] = SHORT_SWAP(genCRC (temp, 4));

    if ((*work->medium.write) (&work->medium, temp, 6) != 6)
    {
        return ERROR;
    }

    vpWorkData.reqMsgType = SER_MSG_DMPAFT_HDR;
    return OK;
}

int vpifSendLoopRqst (WVIEWD_WORK *work, int number)
{
    uint16_t            temp[32];                       // short align
    uint8_t             *ptr = (uint8_t *)temp;
    int                 len;

    sprintf ((char *)ptr, "LOOP %d", number);
    len = strlen ((char *)ptr);
    ptr[len] = VP_LF;

    if ((*work->medium.write) (&work->medium, ptr, len + 1) != len + 1)
    {
        return ERROR;
    }

    vpWorkData.reqMsgType = SER_MSG_LOOP;
    return OK;
}

int vpifGetRXCheck (WVIEWD_WORK *work)
{
    int                 i, len, retVal, done;
    uint16_t            temp[VP_BYTE_LENGTH_MAX/2];     // short align
    uint8_t             *ptr = (uint8_t *)temp;
    char                *token;
    int                 rxGood, rxMiss, rxCRC, tempint;

    vpWorkData.rxCheck[0] = 0;
    strcpy ((char *)ptr, "RXCHECK");
    len = strlen ((char *)ptr);
    ptr[len] = VP_CR;

    if ((*work->medium.write) (&work->medium, ptr, len + 1) != len + 1)
    {
        return ERROR;
    }

    // lose the <LF><CR>OK<LF><CR>
    if ((*work->medium.read) (&work->medium, ptr, 6, 2000) != 6)
    {
        (*work->medium.flush) (&work->medium, WV_QUEUE_INPUT);
        return ERROR;
    }

    memset (ptr, 0, 64);
    for (i = 0, done = 0; (done == 0) && (i < 63); i ++)
    {
        retVal = (*work->medium.read) (&work->medium, &ptr[i], 1, 2000);
        if (retVal != 1)
        {
            return ERROR;
        }

        if (ptr[i] == VP_CR)
            done = 1;
    }

    if (!done || i < 3)
    {
        return ERROR;
    }

    i -= 2;                     // lose the <CR> and <LF>
    ptr[i] = 0;

    strcpy (vpWorkData.rxCheck, (char *)ptr);

    token = strtok ((char *)ptr, " ");
    if (token == NULL)
    {
        return ERROR;
    }
    rxGood = atoi (token);

    token = strtok (NULL, " ");
    if (token == NULL)
    {
        return ERROR;
    }
    rxMiss = atoi (token);

    // toss re-syncs
    token = strtok (NULL, " ");
    if (token == NULL)
    {
        return ERROR;
    }
    // toss good-in-a-row
    token = strtok (NULL, " ");
    if (token == NULL)
    {
        return ERROR;
    }

    token = strtok (NULL, " ");
    if (token == NULL)
    {
        return ERROR;
    }
    rxCRC = atoi (token);
    if (rxCRC < 0)
        rxCRC += 65536;

    // now we have something to work with
    tempint = rxGood - vpWorkData.rxCheckGood;
    vpWorkData.rxCheckGood = rxGood;
    if (tempint < 0)
    {
        // must have rolled over the counts or manually reset
        vpWorkData.rxCheckMissed = rxMiss;
        vpWorkData.rxCheckCRC = rxCRC;
        return OK;
    }
    else
    {
        rxGood = tempint;
    }
    tempint = rxMiss - vpWorkData.rxCheckMissed;
    vpWorkData.rxCheckMissed = rxMiss;
    if (tempint < 0)
    {
        // rolled over
        vpWorkData.rxCheckCRC = rxCRC;
        return OK;
    }
    else
    {
        rxMiss = tempint;
    }
    tempint = rxCRC - vpWorkData.rxCheckCRC;
    vpWorkData.rxCheckCRC = rxCRC;
    if (tempint < 0)
    {
        // rolled over
        return OK;
    }
    else
    {
        rxCRC = tempint;
    }

    // finally calculate the percentage
    tempint = rxGood + rxMiss + rxCRC;
    if (tempint > 0)
    {
        vpWorkData.rxCheckPercent = (100 * rxGood)/tempint;
    }
    else
    {
        vpWorkData.rxCheckPercent = 100;
    }

//radMsgLog(PRI_MEDIUM, "RXCHECK: %s", vpWorkData.rxCheck);
    return OK;
}


//////// methods for vpconfig ////////
#ifdef _VP_CONFIG_ONLY

int vpconfigGetArchiveInterval
(
    WVIEWD_WORK *work
)
{
    int                 retVal, len;
    uint16_t            temp[VP_BYTE_LENGTH_MAX/2];     // short align
    uint8_t             *ptr = (uint8_t *)temp;
    ARCHIVE_INTERVAL    *interval = (ARCHIVE_INTERVAL *)temp;

    strcpy ((char *)ptr, "EEBRD 2D 1");
    len = strlen ((char *)ptr);
    ptr[len] = VP_LF;

    if ((*work->medium.write) (&work->medium, ptr, len + 1) != len + 1)
    {
        return ERROR;
    }

    if (vpifGetAck (work, 2000) == ERROR)
    {
        (*work->medium.flush) (&work->medium, WV_QUEUE_INPUT);
        return ERROR;
    }
    retVal = readWithCRC (work,
                          interval,
                          sizeof (ARCHIVE_INTERVAL),
                          2000);
    if (retVal != sizeof (ARCHIVE_INTERVAL))
    {
        (*work->medium.flush) (&work->medium, WV_QUEUE_INPUT);
        return ERROR;
    }

    work->archiveInterval = interval->interval;

    return OK;
}

int vpconfigGetFWVersion
(
    WVIEWD_WORK *work
)
{
    int                 i, retVal, len, done = 0;
    uint16_t            temp[VP_BYTE_LENGTH_MAX/2];     // short align
    uint8_t             *indexPtr, *ptr = (uint8_t *)temp;

    strcpy ((char *)ptr, "VER");
    len = strlen ((char *)ptr);
    ptr[len] = VP_LF;

    if ((*work->medium.write) (&work->medium, ptr, len + 1) != len + 1)
    {
        return ERROR;
    }

    // lose the <LF><CR>OK<LF><CR>
    if ((*work->medium.read) (&work->medium, ptr, 6, 2000) != 6)
    {
        (*work->medium.flush) (&work->medium, WV_QUEUE_INPUT);
        return ERROR;
    }

    indexPtr = ptr;
    memset (indexPtr, 0, 32);
    while (!done)
    {
        retVal = (*work->medium.read) (&work->medium, indexPtr, 1, 2000);
        if (retVal != 1)
        {
            done = 1;
        }

        if (*indexPtr == VP_CR)
            done = 1;

        indexPtr ++;
    }

    len = indexPtr - ptr;
    len -= 2;                   // lose the <CR> and <LF>
    ptr[len] = 0;


#if 0
printf ("@!@ GetRXCheck: ");
for (i = 0; i < len; i ++)
    printf ("%2.2X, ", ptr[i]);
printf ("\n len = %d\n", len);
#endif

    strcpy (vpWorkData.fwVersion, (char *)ptr);

    return OK;
}

int vpconfigGetRainSeasonStart
(
    WVIEWD_WORK *work
)
{
    int         retVal, len;
    int16_t     tmp[16];
    uint8_t     *bfr = (uint8_t *)tmp;

    strcpy ((char *)bfr, "EEBRD 2C 1");
    len = strlen ((char *)bfr);
    bfr[len] = VP_LF;

    if ((*work->medium.write) (&work->medium, bfr, len + 1) != len + 1)
    {
        return ERROR;
    }

    if (vpifGetAck (work, 2000) == ERROR)
    {
        (*work->medium.flush) (&work->medium, WV_QUEUE_INPUT);
        return ERROR;
    }

    memset (bfr, 0, sizeof (tmp));
    retVal = readWithCRC (work, bfr, 3, 2000);
    if (retVal != 3)
    {
        return ERROR;
    }

    vpWorkData.rainSeasonStart = bfr[0];

    return OK;
}

int vpconfigGetWindDirectionCal
(
    WVIEWD_WORK *work
)
{
    int         retVal, len;
    int16_t     tmp[16];
    uint8_t     *bfr = (uint8_t *)tmp;

    strcpy ((char *)bfr, "EEBRD 4D 2");
    len = strlen ((char *)bfr);
    bfr[len] = VP_LF;

    if ((*work->medium.write) (&work->medium, bfr, len + 1) != len + 1)
    {
        return ERROR;
    }

    if (vpifGetAck (work, 2000) == ERROR)
    {
        (*work->medium.flush) (&work->medium, WV_QUEUE_INPUT);
        return ERROR;
    }

    memset (bfr, 0, sizeof (tmp));
    retVal = readWithCRC (work, bfr, 4, 2000);
    if (retVal != 4)
    {
        return ERROR;
    }

    vpWorkData.windDirectionCal = SHORT_SWAP(tmp[0]);

    return OK;
}

int vpconfigGetTransmitters
(
    WVIEWD_WORK *work
)
{
    int         retVal, len;
    uint16_t    tmp[16];
    uint8_t     *bfr = (uint8_t *)tmp;

    strcpy ((char *)bfr, "EEBRD 17 12");
    len = strlen ((char *)bfr);
    bfr[len] = VP_LF;

    if ((*work->medium.write) (&work->medium, bfr, len + 1) != len + 1)
    {
        return ERROR;
    }

    if (vpifGetAck (work, 2000) == ERROR)
    {
        (*work->medium.flush) (&work->medium, WV_QUEUE_INPUT);
        return ERROR;
    }

    memset (bfr, 0, sizeof (tmp));
    retVal = readWithCRC (work, bfr, 20, 2000);
    if (retVal != 20)
    {
        return ERROR;
    }

    vpWorkData.listenChannels = bfr[0];
    vpWorkData.retransmitChannel = bfr[1];
    memcpy(vpWorkData.transmitterType, bfr + 2, 16);

    return OK;
}

int vpconfigGetRainCollectorSize
(
    WVIEWD_WORK *work
)
{
    int         retVal, len;
    uint16_t    tmp[16];
    uint8_t     *bfr = (uint8_t *)tmp;

    strcpy ((char *)bfr, "EEBRD 2B 1");
    len = strlen ((char *)bfr);
    bfr[len] = VP_LF;

    if ((*work->medium.write) (&work->medium, bfr, len + 1) != len + 1)
    {
        return ERROR;
    }

    if (vpifGetAck (work, 2000) == ERROR)
    {
        (*work->medium.flush) (&work->medium, WV_QUEUE_INPUT);
        return ERROR;
    }

    memset (bfr, 0, sizeof (tmp));
    retVal = readWithCRC (work, bfr, 3, 2000);
    if (retVal != 3)
    {
        return ERROR;
    }

    vpWorkData.rainCollectorSize = (bfr[0] >> 4) & 0x3;

    return OK;
}

int vpconfigGetWindCupSize
(
    WVIEWD_WORK *work
)
{
    int         retVal, len;
    uint16_t    tmp[16];
    uint8_t     *bfr = (uint8_t *)tmp;

    strcpy ((char *)bfr, "EEBRD 2B 1");
    len = strlen ((char *)bfr);
    bfr[len] = VP_LF;

    if ((*work->medium.write) (&work->medium, bfr, len + 1) != len + 1)
    {
        return ERROR;
    }

    if (vpifGetAck (work, 2000) == ERROR)
    {
        (*work->medium.flush) (&work->medium, WV_QUEUE_INPUT);
        return ERROR;
    }

    memset (bfr, 0, sizeof (tmp));
    retVal = readWithCRC (work, bfr, 3, 2000);
    if (retVal != 3)
    {
        return ERROR;
    }

    vpWorkData.windCupSize = (bfr[0] >> 3) & 0x1;

    return OK;
}

int vpconfigSetInterval
(
    WVIEWD_WORK *work,
    int         interval
)
{
    int         i, retVal, len, done = 0;
    uint16_t    usbfr[16];
    uint8_t     *indexPtr, *bfr = (uint8_t *)usbfr;

    sprintf ((char *)bfr, "SETPER %d", interval);
    len = strlen ((char *)bfr);
    bfr[len] = VP_LF;

    retVal = (*work->medium.write) (&work->medium, bfr, len+1);
    if (retVal != len+1)
    {
        return ERROR;
    }

    // lose the <LF><CR>
    if ((*work->medium.read) (&work->medium, bfr, 2, 5000) != 2)
    {
        (*work->medium.flush) (&work->medium, WV_QUEUE_INPUT);
        return ERROR;
    }

    indexPtr = bfr;
    memset (indexPtr, 0, 32);
    while (!done)
    {
        retVal = (*work->medium.read) (&work->medium, indexPtr, 1, 2000);
        if (retVal != 1)
        {
            done = 1;
        }

        indexPtr ++;
    }

    len = (indexPtr-1) - bfr;
    len -= 2;                   // lose the <CR> and <LF>
    bfr[len] = 0;

    if (strncmp ((char *)bfr, "OK", 2))
    {
        return ERROR;
    }

    sleep (1);

    sprintf ((char *)bfr, "NEWSETUP");
    len = strlen ((char *)bfr);
    bfr[len] = VP_LF;

    retVal = (*work->medium.write) (&work->medium, bfr, len+1);
    if (retVal != len+1)
    {
        return ERROR;
    }

    if (vpifGetAck (work, 5000) == ERROR)
    {
        (*work->medium.flush) (&work->medium, WV_QUEUE_INPUT);
        return ERROR;
    }

    return OK;
}

int vpconfigClearArchiveMemory
(
    WVIEWD_WORK *work
)
{
    int         retVal, len;
    uint16_t    usbfr[16];
    uint8_t     *bfr = (uint8_t *)usbfr;

    sprintf ((char *)bfr, "CLRLOG");
    len = strlen ((char *)bfr);
    bfr[len] = VP_LF;

    retVal = (*work->medium.write) (&work->medium, bfr, len+1);
    if (retVal != len+1)
    {
        return ERROR;
    }

    if (vpifGetAck (work, 2000) == ERROR)
    {
        (*work->medium.flush) (&work->medium, WV_QUEUE_INPUT);
        return ERROR;
    }

    return OK;
}

int vpconfigSetElevation
(
    WVIEWD_WORK *work,
    int         value
)
{
    int         retVal, len, done = 0;
    uint16_t    usbfr[16];
    uint8_t     *indexPtr, *bfr = (uint8_t *)usbfr;

    sprintf ((char *)bfr, "BAR=0 %d", value);
    len = strlen ((char *)bfr);
    bfr[len] = VP_CR;
    bfr[len+1] = VP_LF;

    retVal = (*work->medium.write) (&work->medium, bfr, len+2);
    if (retVal != len+2)
    {
        return ERROR;
    }

    // lose the <LF><CR>
    if ((*work->medium.read) (&work->medium, bfr, 2, 5000) != 2)
    {
        (*work->medium.flush) (&work->medium, WV_QUEUE_INPUT);
        return ERROR;
    }

    indexPtr = bfr;
    memset (indexPtr, 0, 32);
    while (!done)
    {
        retVal = (*work->medium.read) (&work->medium, indexPtr, 1, 5000);
        if (retVal != 1)
        {
            done = 1;
        }

        if (*indexPtr == VP_CR)
            done = 1;

        indexPtr ++;
    }

    len = indexPtr - bfr;
    len -= 2;                   // lose the <CR> and <LF>
    bfr[len] = 0;

    if (strncmp ((char *)bfr, "OK", 2))
    {
        return ERROR;
    }

    return OK;
}

int vpconfigSetGain
(
    WVIEWD_WORK *work,
    int         on
)
{
    int                 i, retVal, len, done = 0;
    uint16_t            temp[VP_BYTE_LENGTH_MAX/2];     // short align
    uint8_t             *indexPtr, *ptr = (uint8_t *)temp;

    sprintf ((char *)ptr, "GAIN %d", on);
    len = strlen ((char *)ptr);
    ptr[len] = VP_LF;

    if ((*work->medium.write) (&work->medium, ptr, len + 1) != len + 1)
    {
        return ERROR;
    }

    // lose the <LF><CR>
    if ((*work->medium.read) (&work->medium, ptr, 2, 2000) != 2)
    {
        (*work->medium.flush) (&work->medium, WV_QUEUE_INPUT);
        return ERROR;
    }

    indexPtr = ptr;
    memset (indexPtr, 0, 32);
    while (!done)
    {
        retVal = (*work->medium.read) (&work->medium, indexPtr, 1, 2000);
        if (retVal != 1)
        {
            done = 1;
        }

        if (*indexPtr == VP_CR)
            done = 1;

        indexPtr ++;
    }

    len = indexPtr - ptr;
    len -= 2;                   // lose the <CR> and <LF>
    ptr[len] = 0;

    if (!strncmp ((char *)ptr, "OK", 2))
        return OK;
    else
        return ERROR;

    return OK;
}

int vpconfigSetLatandLong
(
    WVIEWD_WORK *work,
    int         latitude,
    int         longitude
)
{
    int         retVal, len;
    int16_t     tmp[16], lat1, long1;
    uint8_t     *bfr = (uint8_t *)tmp;

    strcpy ((char *)bfr, "EEBWR 0B 4");
    len = strlen ((char *)bfr);
    bfr[len] = VP_LF;

    if ((*work->medium.write) (&work->medium, bfr, len + 1) != len + 1)
    {
        return ERROR;
    }

    if (vpifGetAck (work, 2000) == ERROR)
    {
        (*work->medium.flush) (&work->medium, WV_QUEUE_INPUT);
        return ERROR;
    }

    lat1 = (int16_t)latitude;
    long1 = (int16_t)longitude;
    memset (bfr, 0, 4);
    tmp[0] = SHORT_SWAP(lat1);
    tmp[1] = SHORT_SWAP(long1);
    retVal = writeWithCRC (work, bfr, 4);
    if (retVal != 4)
    {
        return ERROR;
    }

    sleep (3);

    sprintf ((char *)bfr, "NEWSETUP");
    len = strlen ((char *)bfr);
    bfr[len] = VP_LF;

    retVal = (*work->medium.write) (&work->medium, bfr, len+1);
    if (retVal != len+1)
    {
        return ERROR;
    }

    if (vpifGetAck (work, 5000) == ERROR)
    {
        (*work->medium.flush) (&work->medium, WV_QUEUE_INPUT);
        return ERROR;
    }

    return OK;
}

int vpconfigSetRainSeasonStart
(
    WVIEWD_WORK *work,
    int         startMonth
)
{
    int         retVal, len;
    int16_t     tmp[16];
    uint8_t     *bfr = (uint8_t *)tmp;

    strcpy ((char *)bfr, "EEBWR 2C 1");
    len = strlen ((char *)bfr);
    bfr[len] = VP_LF;

    if ((*work->medium.write) (&work->medium, bfr, len + 1) != len + 1)
    {
        return ERROR;
    }

    if (vpifGetAck (work, 2000) == ERROR)
    {
        (*work->medium.flush) (&work->medium, WV_QUEUE_INPUT);
        return ERROR;
    }

    memset (bfr, 0, sizeof (tmp));
    bfr[0] = startMonth;
    retVal = writeWithCRC (work, bfr, 1);
    if (retVal != 1)
    {
        return ERROR;
    }

    return OK;
}

int vpconfigSetRainYearToDate
(
    WVIEWD_WORK *work,
    float       rainAmount
)
{
    int         retVal, len;
    int16_t     tmp[16];
    uint8_t     *bfr = (uint8_t *)tmp;

    rainAmount *=1000;
    rainAmount = (int)rainAmount;
    rainAmount += 1;
    rainAmount /= 10;

    retVal = (int)(rainAmount);              // rain collector = 0.01 inches
    sprintf ((char *)bfr, "PUTRAIN %d", retVal);
    len = strlen ((char *)bfr);
    bfr[len] = VP_LF;

    if ((*work->medium.write) (&work->medium, bfr, len + 1) != len + 1)
    {
        return ERROR;
    }

    if (vpifGetAck (work, 2000) == ERROR)
    {
        (*work->medium.flush) (&work->medium, WV_QUEUE_INPUT);
        return ERROR;
    }

    return OK;
}

int vpconfigSetETYearToDate
(
    WVIEWD_WORK *work,
    float       etAmount
)
{
    int         retVal, len;
    int16_t     tmp[16];
    uint8_t     *bfr = (uint8_t *)tmp;

    etAmount *=1000;
    etAmount = (int)etAmount;
    etAmount += 1;
    etAmount /= 10;

    retVal = (int)etAmount;                  // rain collector = 0.01 inches
    sprintf ((char *)bfr, "PUTET %d", retVal);
    len = strlen ((char *)bfr);
    bfr[len] = VP_LF;

    if ((*work->medium.write) (&work->medium, bfr, len + 1) != len + 1)
    {
        return ERROR;
    }

    if (vpifGetAck (work, 2000) == ERROR)
    {
        (*work->medium.flush) (&work->medium, WV_QUEUE_INPUT);
        return ERROR;
    }

    return OK;
}

int vpconfigSetWindDirectionCal
(
    WVIEWD_WORK *work,
    int         offset
)
{
    int         retVal, len;
    int16_t     tmp[16];
    uint8_t     *bfr = (uint8_t *)tmp;

    strcpy ((char *)bfr, "EEBWR 4D 2");
    len = strlen ((char *)bfr);
    bfr[len] = VP_LF;

    if ((*work->medium.write) (&work->medium, bfr, len + 1) != len + 1)
    {
        return ERROR;
    }

    if (vpifGetAck (work, 2000) == ERROR)
    {
        (*work->medium.flush) (&work->medium, WV_QUEUE_INPUT);
        return ERROR;
    }

    memset (bfr, 0, sizeof (tmp));
    tmp[0] = SHORT_SWAP((int16_t) offset);
    retVal = writeWithCRC (work, bfr, 2);
    if (retVal != 2)
    {
        return ERROR;
    }

    return OK;
}

int vpconfigSetSensor
(
    WVIEWD_WORK *work,
    int         channel,
    VPRO_SENSOR_TYPES type
)
{
    int         retVal, len, i, temperature_id = 0, humidity_id = 1;
    uint16_t    tmp[16];
    uint8_t     *bfr = (uint8_t *)tmp;

    // Fetch existing transmitter information.
    if (vpconfigGetTransmitters(work) != OK)
    {
        return ERROR;
    }

    // Set or clear the USETX bit corresponding to the channel.
    if (type == VPRO_SENSOR_NONE)
    {
      vpWorkData.listenChannels &= ~((uint8_t) 1 << (channel-1));
    }
    else
    {
      vpWorkData.listenChannels |= ((uint8_t) 1 << (channel-1));
    }

    // There can be only one ISS.
    if (type == VPRO_SENSOR_ISS)
    {
        for (i = 0; i < 8; i++)
        {
            if (channel == i+1) 
            {
                continue;
            }

            if ((vpWorkData.transmitterType[i*2] & 0xF) == VPRO_SENSOR_ISS)
            {
                // Downgrade to temperature/humidity sensor.
                vpWorkData.transmitterType[i*2] = VPRO_SENSOR_TEMP_HUM;
            }
        }
    }

    // Assign the sensor type on this channel.
    vpWorkData.transmitterType[(channel-1)*2] = type;

    // Extra temperature/humidity sensors must be sequential.
    for (i = 0; i < 8; i ++)
    {
        if (vpWorkData.transmitterType[i*2] == VPRO_SENSOR_TEMP)
        {
            vpWorkData.transmitterType[i*2+1] = temperature_id++;
        }
        else if (vpWorkData.transmitterType[i*2] == VPRO_SENSOR_HUM)
        {
            vpWorkData.transmitterType[i*2+1] = humidity_id++ << 4;
        }
        else if (vpWorkData.transmitterType[i*2] ==  VPRO_SENSOR_TEMP_HUM)
        {
            vpWorkData.transmitterType[i*2+1] = humidity_id++ << 4 | temperature_id++;
        }
    }

    // Send the new transmitter setup to the console.
    strcpy ((char *)bfr, "EEBWR 17 12");
    len = strlen ((char *)bfr);
    bfr[len] = VP_LF;

    if ((*work->medium.write) (&work->medium, bfr, len + 1) != len + 1)
    {
        return ERROR;
    }

    if (vpifGetAck (work, 2000) == ERROR)
    {
        (*work->medium.flush) (&work->medium, WV_QUEUE_INPUT);
        return ERROR;
    }

    memset (bfr, 0, sizeof (tmp));
    bfr[0] = vpWorkData.listenChannels;
    bfr[1] = vpWorkData.retransmitChannel;
    memcpy(bfr + 2, vpWorkData.transmitterType, 16);
    
    retVal = writeWithCRC (work, bfr, 18);
    if (retVal != 18)
    {
        return ERROR;
    }

    sleep (1);

    // Setting the retransmit channel requires clearing the archive memory
    sprintf ((char *)bfr, "NEWSETUP");
    len = strlen ((char *)bfr);
    bfr[len] = VP_LF;

    retVal = (*work->medium.write) (&work->medium, bfr, len+1);
    if (retVal != len+1)
    {
        return ERROR;
    }

    retVal = (*work->medium.read) (&work->medium, bfr, 1, 5000);
    if (retVal != 1)
    {
        return ERROR;
    }
    if (bfr[0] != VP_ACK)
    {
        return ERROR;
    }
    return OK;
}

int vpconfigSetRetransmitChannel
(
    WVIEWD_WORK *work,
    int         channel
)
{
    int         retVal, len;
    int16_t     tmp[16];
    uint8_t     *bfr = (uint8_t *)tmp;

    strcpy ((char *)bfr, "EEBWR 18 1");
    len = strlen ((char *)bfr);
    bfr[len] = VP_LF;

    if ((*work->medium.write) (&work->medium, bfr, len + 1) != len + 1)
    {
        return ERROR;
    }

    if (vpifGetAck (work, 2000) == ERROR)
    {
        (*work->medium.flush) (&work->medium, WV_QUEUE_INPUT);
        return ERROR;
    }

    memset (bfr, 0, sizeof (tmp));
    bfr[0] = channel;
    retVal = writeWithCRC (work, bfr, 1);
    if (retVal != 1)
    {
        return ERROR;
    }

    sleep (1);

    // Setting the retransmit channel requires clearing the archive memory
    sprintf ((char *)bfr, "NEWSETUP");
    len = strlen ((char *)bfr);
    bfr[len] = VP_LF;

    retVal = (*work->medium.write) (&work->medium, bfr, len+1);
    if (retVal != len+1)
    {
        return ERROR;
    }

    if (vpifGetAck (work, 5000) == ERROR)
    {
        (*work->medium.flush) (&work->medium, WV_QUEUE_INPUT);
        return ERROR;
    }

    return OK;
}

int vpconfigSetRainCollectorSize
(
    WVIEWD_WORK *work,
    VPRO_RAIN_COLLECTOR_SIZE size
)
{
    int         retVal, len;
    uint16_t    tmp[16];
    uint8_t     *bfr = (uint8_t *)tmp, newval;

    strcpy ((char *)bfr, "EEBRD 2B 1");
    len = strlen ((char *)bfr);
    bfr[len] = VP_LF;

    if ((*work->medium.write) (&work->medium, bfr, len + 1) != len + 1)
    {
        return ERROR;
    }

    if (vpifGetAck (work, 2000) == ERROR)
    {
        (*work->medium.flush) (&work->medium, WV_QUEUE_INPUT);
        return ERROR;
    }

    memset (bfr, 0, sizeof (tmp));
    retVal = readWithCRC (work, bfr, 3, 2000);
    if (retVal != 3)
    {
        return ERROR;
    }

    newval = bfr[0] & 0xCF | ((int) size << 4);

    strcpy ((char *)bfr, "EEBWR 2B 1");
    len = strlen ((char *)bfr);
    bfr[len] = VP_LF;

    if ((*work->medium.write) (&work->medium, bfr, len + 1) != len + 1)
    {
        return ERROR;
    }

    if (vpifGetAck (work, 2000) == ERROR)
    {
        (*work->medium.flush) (&work->medium, WV_QUEUE_INPUT);
        return ERROR;
    }

    memset (bfr, 0, sizeof (tmp));
    bfr[0] = newval;
    retVal = writeWithCRC (work, bfr, 1);
    if (retVal != 1)
    {
        return ERROR;
    }

    return OK;
}

int vpconfigSetWindCupSize
(
    WVIEWD_WORK *work,
    int isLarge
)
{
    int         retVal, len;
    uint16_t    tmp[16];
    uint8_t     *bfr = (uint8_t *)tmp, newval;

    strcpy ((char *)bfr, "EEBRD 2B 1");
    len = strlen ((char *)bfr);
    bfr[len] = VP_LF;

    if ((*work->medium.write) (&work->medium, bfr, len + 1) != len + 1)
    {
        return ERROR;
    }

    if (vpifGetAck (work, 2000) == ERROR)
    {
        (*work->medium.flush) (&work->medium, WV_QUEUE_INPUT);
        return ERROR;
    }

    memset (bfr, 0, sizeof (tmp));
    retVal = readWithCRC (work, bfr, 3, 2000);
    if (retVal != 3)
    {
        return ERROR;
    }

    newval = bfr[0] & 0xF7 | (isLarge == 0 ? 0 : 0x8);

    strcpy ((char *)bfr, "EEBWR 2B 1");
    len = strlen ((char *)bfr);
    bfr[len] = VP_LF;

    if ((*work->medium.write) (&work->medium, bfr, len + 1) != len + 1)
    {
        return ERROR;
    }

    if (vpifGetAck (work, 2000) == ERROR)
    {
        (*work->medium.flush) (&work->medium, WV_QUEUE_INPUT);
        return ERROR;
    }

    memset (bfr, 0, sizeof (tmp));
    bfr[0] = newval;
    retVal = writeWithCRC (work, bfr, 1);
    if (retVal != 1)
    {
        return ERROR;
    }

    return OK;
}
#endif

