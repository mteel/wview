/*---------------------------------------------------------------------------
 
  FILENAME:
        wxt510Interface.c
 
  PURPOSE:
        Provide the WXT-510 station interface API and utilities.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        01/14/2006      M.S. Teel       0               Original
        03/23/2008      W. Krenn        1               modified rain/hail/heating
 
  NOTES:
 
 
  LICENSE:
        Copyright (c) 2006, Mark S. Teel (mark@teel.ws)
 
        This source code is released for free distribution under the terms
        of the GNU General Public License.
 
----------------------------------------------------------------------------*/

/*  ... System include files
*/

/*  ... Library include files
*/

/*  ... Local include files
*/
#include <wxt510Interface.h>

/*  ... global memory declarations
*/

/*  ... local memory
*/

static WXT510_IF_DATA   wxt510WorkData;
static WVIEWD_WORK*     pwviewWork;

static void             (*ArchiveIndicator) (ARCHIVE_PKT* newRecord);

static void serialPortConfig (int fd);
static void storeLoopPkt (WVIEWD_WORK* work, LOOP_PKT *dest, NMEA0183_DATA *src);



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

    memset (&wxt510WorkData, 0, sizeof(wxt510WorkData));
    pwviewWork = work;

    // save the archive indication callback (we should never need it)
    ArchiveIndicator = archiveIndication;

    // set our work data pointer
    work->stationData = &wxt510WorkData;

    // set the Archive Generation flag to indicate the WXT-510 DOES NOT
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
        radMsgLog (PRI_STATUS, "WXT510 on %s opened ...",
                   work->stationDevice);
    }
    else if (!strcmp (work->stationInterface, "ethernet"))
    {
        radMsgLog (PRI_STATUS, "WXT510 on %s:%d opened ...",
                   work->stationHost, work->stationPort);
    }

#ifndef _WXT510_CONFIG_ONLY
    // grab the station configuration now
    if (stationGetConfigValueInt (work,
                                  STATION_PARM_ELEVATION,
                                  &wxt510WorkData.elevation)
            == ERROR)
    {
        radMsgLog (PRI_HIGH, "stationInit: stationGetConfigValueInt ELEV failed!");
        (*(work->medium.exit)) (&work->medium);
        return ERROR;
    }
    if (stationGetConfigValueFloat (work,
                                    STATION_PARM_LATITUDE,
                                    &wxt510WorkData.latitude)
            == ERROR)
    {
        radMsgLog (PRI_HIGH, "stationInit: stationGetConfigValueInt LAT failed!");
        (*(work->medium.exit)) (&work->medium);
        return ERROR;
    }
    if (stationGetConfigValueFloat (work,
                                    STATION_PARM_LONGITUDE,
                                    &wxt510WorkData.longitude)
            == ERROR)
    {
        radMsgLog (PRI_HIGH, "stationInit: stationGetConfigValueInt LONG failed!");
        (*(work->medium.exit)) (&work->medium);
        return ERROR;
    }
    if (stationGetConfigValueInt (work,
                                  STATION_PARM_ARC_INTERVAL,
                                  &wxt510WorkData.archiveInterval)
            == ERROR)
    {
        radMsgLog (PRI_HIGH, "stationInit: stationGetConfigValueInt ARCINT failed!");
        (*(work->medium.exit)) (&work->medium);
        return ERROR;
    }

    // set the work archive interval now
    work->archiveInterval = wxt510WorkData.archiveInterval;

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

    wxt510WorkData.totalRain = 0;
    wxt510WorkData.totalHail = 0;

    radMsgLog (PRI_STATUS, "Starting station interface: WXT510"); 

    // initialize the station interface
    if (nmea0183Init(work) == ERROR)
    {
        radMsgLog (PRI_HIGH, "stationInit: nmea0183Init failed!");
        (*(work->medium.exit)) (&work->medium);
        return ERROR;
    }

    // do the initial GetReadings now
    if (nmea0183GetReadings(work, &wxt510WorkData.nmeaReadings) != OK)
    {
        radMsgLog (PRI_HIGH, "stationInit: initial nmea0183GetReadings failed!");
        nmea0183Exit (work);
        (*(work->medium.exit)) (&work->medium);
        return ERROR;
    }

    // do final initialization tasks
    if (nmea0183PostInit(work) == ERROR)
    {
        radMsgLog (PRI_HIGH, "stationInit: nmea0183PostInit failed!");
        (*(work->medium.exit)) (&work->medium);
        return ERROR;
    }


    // populate the LOOP structure
    storeLoopPkt (work, &work->loopPkt, &wxt510WorkData.nmeaReadings);

    // start the rain accumulator reset timer -
    // we use the provided interface timer (ifTimer) for this
    radProcessTimerStart (work->ifTimer, (uint32_t)WXT_RAIN_RESET_INTERVAL);

    // we must indicate successful completion here -
    // even though we are synchronous, the daemon wants to see this event
    radProcessEventsSend (NULL, STATION_INIT_COMPLETE_EVENT, 0);

#endif

    return OK;
}

#ifndef _WXT510_CONFIG_ONLY

// station-supplied exit function
//
// Returns: N/A
//
void stationExit (WVIEWD_WORK *work)
{
    nmea0183Exit (work);
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
    work->elevation     = (int16_t)wxt510WorkData.elevation;
    if (wxt510WorkData.latitude >= 0)
        work->latitude      = (int16_t)((wxt510WorkData.latitude*10)+0.5);
    else
        work->latitude      = (int16_t)((wxt510WorkData.latitude*10)-0.5);
    if (wxt510WorkData.longitude >= 0)
        work->longitude     = (int16_t)((wxt510WorkData.longitude*10)+0.5);
    else
        work->longitude     = (int16_t)((wxt510WorkData.longitude*10)-0.5);

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
    // WXT-510 does not keep time...
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
    if (nmea0183GetReadings(work, &wxt510WorkData.nmeaReadings) == OK)
    {
        // populate the LOOP structure
        storeLoopPkt (work, &work->loopPkt, &wxt510WorkData.nmeaReadings);

        // indicate we are done
        radProcessEventsSend (NULL, STATION_LOOP_COMPLETE_EVENT, 0);
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
    // just indicate a NULL record, WXT-510 does not generate them (and this
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
    // WXT510 station is synchronous...
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
    // the rain accumulator should be reset
    nmea0183ResetAccumulators (work);

    // reset our container
    wxt510WorkData.totalRain = 0;

    // restart the timer
    radProcessTimerStart (work->ifTimer, (uint32_t)WXT_RAIN_RESET_INTERVAL);

    return;
}

#endif

////////////****////  S T A T I O N   A P I   E N D  ////****////////////


//  ... ----- static (local) methods ----- ...

static void serialPortConfig (int fd)
{
    struct termios  port, tty, old;

    tcgetattr (fd, &port);

    cfsetispeed (&port, B19200);
    cfsetospeed (&port, B19200);

    // set port to 8N1
    port.c_cflag &= ~PARENB;
    port.c_cflag &= ~CSTOPB;
    port.c_cflag &= ~CSIZE;
    port.c_cflag &= ~CRTSCTS;                   // turn OFF H/W flow control
    port.c_cflag |= CS8;
    port.c_cflag |= (CREAD | CLOCAL);

    port.c_iflag &= ~(IXON | IXOFF | IXANY);    // turn off SW flow control

    port.c_iflag &= ~(INLCR | ICRNL);           // turn off other input magic

    port.c_oflag = 0;                           // NO output magic wanted

    port.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    tcsetattr (fd, TCSAFLUSH, &port);

    // bump DTR
    if (pwviewWork->stationToggleDTR)
    {
        tcgetattr (fd, &tty);
        tcgetattr (fd, &old);
        cfsetospeed (&tty, B0);
        cfsetispeed (&tty, B0);
        tcsetattr (fd, TCSANOW, &tty);
        sleep (1);
        tcsetattr (fd, TCSANOW, &old);
    }

    return;
}

#ifndef _WXT510_CONFIG_ONLY

static void storeLoopPkt (WVIEWD_WORK* work, LOOP_PKT *dest, NMEA0183_DATA *src)
{
    float       tempfloat;

    // Clear optional data:
    stationClearLoopData(work);

    if ((10 < src->pressure && src->pressure < 50) &&
        (-200 < src->temperature && src->temperature < 200))
    {
        // WXT-510 produces station pressure
        dest->stationPressure               = src->pressure;
    
        // Apply calibration here so the computed values reflect it:
        dest->stationPressure *= work->calMPressure;
        dest->stationPressure += work->calCPressure;
    
        // compute sea-level pressure (BP)
        tempfloat = wvutilsConvertSPToSLP(dest->stationPressure,
                                          src->temperature,
                                          (float)wxt510WorkData.elevation);
        dest->barometer                     = tempfloat;
    
        // calculate altimeter
        tempfloat = wvutilsConvertSPToAltimeter(dest->stationPressure,
                                                (float)wxt510WorkData.elevation);
        dest->altimeter                     = tempfloat;
    }

    if (-200 < src->temperature && src->temperature < 200)
        dest->outTemp                       = src->temperature;

    if (0 <= src->humidity && src->humidity <= 100)
    {
        tempfloat = src->humidity;
        tempfloat += 0.5;
        dest->outHumidity                   = (uint16_t)tempfloat;
    }

    if (0 <= src->windSpeed && src->windSpeed <= 250)
    {
        tempfloat = src->windSpeed;
        dest->windSpeedF                    = tempfloat;
    }

    if (0 <= src->windDir && src->windDir <= 360)
    {
        tempfloat = src->windDir;
        tempfloat += 0.5;
        dest->windDir                       = (uint16_t)tempfloat;
    }

    if (0 <= src->maxWindSpeed && src->maxWindSpeed <= 250)
    {
        tempfloat = src->maxWindSpeed;
        dest->windGustF                     = tempfloat;
    }

    if (0 <= src->maxWindDir && src->maxWindDir <= 360)
    {
        tempfloat = src->maxWindDir;
        tempfloat += 0.5;
        dest->windGustDir                   = (uint16_t)tempfloat;
    }

    dest->rainRate                          = src->rainrate;

    // process the rain accumulator
    if (0 <= src->rain)
    {
        if (src->rain - wxt510WorkData.totalRain >= 0)
        {
            dest->sampleRain = src->rain - wxt510WorkData.totalRain;
            wxt510WorkData.totalRain = src->rain;
        }
        else
        {
            // we had a counter reset...
            dest->sampleRain = src->rain;
            wxt510WorkData.totalRain = src->rain;
        }

        if (dest->sampleRain > 2)
        {
            // Not possible, filter it out:
            dest->sampleRain = 0;
        }
    }
    else
    {
        dest->sampleRain = 0;
    }

    // process the hail accumulator
    if (0 <= src->rain)
    {
        if (src->hail - wxt510WorkData.totalHail >= 0)
        {
            dest->wxt510Hail = src->hail - wxt510WorkData.totalHail;
            wxt510WorkData.totalHail = src->hail;
        }
        else
        {
            // we had a counter reset...
            dest->wxt510Hail = src->hail;
            wxt510WorkData.totalHail = src->hail;
        }

        if (dest->wxt510Hail > 5)
        {
            // Not possible, filter it out:
            dest->wxt510Hail = 0;
        }
    }
    else
    {
        dest->wxt510Hail = 0;
    }

    // finally the Vaisala-specific ones
    dest->wxt510Hailrate                = src->hailrate;
    dest->wxt510HeatingTemp             = src->heatingTemp;
    dest->wxt510HeatingVoltage          = src->heatingVoltage;
    dest->wxt510SupplyVoltage           = src->supplyVoltage;
    dest->wxt510ReferenceVoltage        = src->referenceVoltage;

    dest->wxt510RainDuration            = src->rainduration;
    dest->wxt510RainPeakRate            = src->rainpeakrate;
    dest->wxt510HailDuration            = src->hailduration;
    dest->wxt510HailPeakRate            = src->hailpeakrate;
    dest->wxt510Rain                    = src->rain;

    return;
}

#endif
