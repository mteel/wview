/*---------------------------------------------------------------------------
 
  FILENAME:
        twiInterface.c
 
  PURPOSE:
        Provide the TWI station interface API and utilities.
 
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
#include <twiInterface.h>

/*  ... global memory declarations
*/

/*  ... local memory
*/

static TWI_IF_DATA      twiWorkData;
static WV_ACCUM_ID      twi12HourTempAvg;
static WVIEWD_WORK*     pwviewWork;

static void             (*ArchiveIndicator) (ARCHIVE_PKT* newRecord);

static void             serialPortConfig (int fd);
static void             storeLoopPkt (WVIEWD_WORK* work, LOOP_PKT *dest, TWI_DATA *src);


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
static float twiConvertSLPToSP(float SLP, float tempF, float elevationFT)
{
    double      SP, PT;

    // Formula used: SP = SLP * PressureTerm
    // compute PressureTerm:
    PT = calcPressureTerm (tempF, elevationFT);
    SP = SLP * PT;

    return (float)SP;
}


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
    ARCHIVE_PKT     recordStore;
    time_t          nowTime;

    memset (&twiWorkData, 0, sizeof(twiWorkData));
    pwviewWork = work;
    twiWorkData.baudrate = B19200;

    // save the archive indication callback (we should never need it)
    ArchiveIndicator = archiveIndication;

    // set our work data pointer
    work->stationData = &twiWorkData;

    // set the Archive Generation flag to indicate the TWI DOES NOT
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

    // Autobaud the station now:
    twiWorkData.baudrate = twiConfig(work);
    if (twiWorkData.baudrate <= 0)
    {
        radMsgLog (PRI_HIGH, "TWI: twiProtocolInit: autobaud failed!");
        return ERROR;
    }

    radMsgLog (PRI_STATUS, "TWI: autobaud = %d", twiWorkData.baudrate);

    // Reconfigure the serial port here to ensure consistency:
    serialPortConfig(work->medium.fd);
    tcflush (work->medium.fd, TCIFLUSH);
    tcflush (work->medium.fd, TCOFLUSH);

    if (!strcmp (work->stationInterface, "serial"))
    {
        radMsgLog (PRI_STATUS, "TWI on %s opened ...",
                   work->stationDevice);
    }
    else if (!strcmp (work->stationInterface, "ethernet"))
    {
        radMsgLog (PRI_STATUS, "TWI on %s:%d opened ...",
                   work->stationHost, work->stationPort);
    }

    // grab the station configuration now
    if (stationGetConfigValueInt (work,
                                  STATION_PARM_ELEVATION,
                                  &twiWorkData.elevation)
            == ERROR)
    {
        radMsgLog (PRI_HIGH, "stationInit: stationGetConfigValueInt ELEV failed!");
        (*(work->medium.exit)) (&work->medium);
        return ERROR;
    }
    if (stationGetConfigValueFloat (work,
                                    STATION_PARM_LATITUDE,
                                    &twiWorkData.latitude)
            == ERROR)
    {
        radMsgLog (PRI_HIGH, "stationInit: stationGetConfigValueInt LAT failed!");
        (*(work->medium.exit)) (&work->medium);
        return ERROR;
    }
    if (stationGetConfigValueFloat (work,
                                    STATION_PARM_LONGITUDE,
                                    &twiWorkData.longitude)
            == ERROR)
    {
        radMsgLog (PRI_HIGH, "stationInit: stationGetConfigValueInt LONG failed!");
        (*(work->medium.exit)) (&work->medium);
        return ERROR;
    }
    if (stationGetConfigValueInt (work,
                                  STATION_PARM_ARC_INTERVAL,
                                  &twiWorkData.archiveInterval)
            == ERROR)
    {
        radMsgLog (PRI_HIGH, "stationInit: stationGetConfigValueInt ARCINT failed!");
        (*(work->medium.exit)) (&work->medium);
        return ERROR;
    }

    // set the work archive interval now
    work->archiveInterval = twiWorkData.archiveInterval;

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

    twiWorkData.totalRain = 0;

   // Create the rain accumulator (TWI_RAIN_RATE_PERIOD minute age)
    // so we can compute rain rate:
    twiWorkData.rainRateAccumulator = sensorAccumInit(TWI_RAIN_RATE_PERIOD);

    // Populate the accumulator with the last TWI_RAIN_RATE_PERIOD minutes:
    nowTime = time(NULL) - (WV_SECONDS_IN_HOUR/(60/TWI_RAIN_RATE_PERIOD));
    while ((nowTime = dbsqliteArchiveGetNextRecord(nowTime, &recordStore)) != ERROR)
    {
        sensorAccumAddSample(twiWorkData.rainRateAccumulator,
                             recordStore.dateTime,
                             recordStore.value[DATA_INDEX_rain]);
    }

    radMsgLog (PRI_STATUS, "Starting station interface: TWI"); 

    // initialize the station interface
    if (twiProtocolInit(work) == ERROR)
    {
        radMsgLog (PRI_HIGH, "stationInit: twiProtocolInit failed!");
        (*(work->medium.exit)) (&work->medium);
        return ERROR;
    }

    // do the initial GetReadings now
    if (twiProtocolGetReadings(work, &twiWorkData.twiReadings) != OK)
    {
        radMsgLog (PRI_HIGH, "stationInit: initial twiProtocolGetReadings failed!");
        twiProtocolExit (work);
        (*(work->medium.exit)) (&work->medium);
        return ERROR;
    }


    // Initialize the 12-hour temp accumulator:
    twi12HourTempAvg = sensorAccumInit(60 * 12);

    // Load data for the last 12 hours:
    nowTime = time(NULL) - (WV_SECONDS_IN_HOUR * 12);
    while ((nowTime = dbsqliteArchiveGetNextRecord(nowTime, &recordStore)) != ERROR)
    {
        sensorAccumAddSample(twi12HourTempAvg, 
                             recordStore.dateTime, 
                             recordStore.value[DATA_INDEX_outTemp]);
    }

    // populate the LOOP structure
    storeLoopPkt (work, &work->loopPkt, &twiWorkData.twiReadings);

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
    twiProtocolExit (work);
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
    work->elevation     = (int16_t)twiWorkData.elevation;
    if (twiWorkData.latitude >= 0)
        work->latitude      = (int16_t)((twiWorkData.latitude*10)+0.5);
    else
        work->latitude      = (int16_t)((twiWorkData.latitude*10)-0.5);
    if (twiWorkData.longitude >= 0)
        work->longitude     = (int16_t)((twiWorkData.longitude*10)+0.5);
    else
        work->longitude     = (int16_t)((twiWorkData.longitude*10)-0.5);

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
    // TWI does not keep time...
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
    if (twiProtocolGetReadings(work, &twiWorkData.twiReadings) == OK)
    {
        // populate the LOOP structure
        storeLoopPkt(work, &work->loopPkt, &twiWorkData.twiReadings);

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
    // just indicate a NULL record, TWI does not generate them (and this
    // function should never be called!)
    (*ArchiveIndicator)(NULL);
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
    // TWI station is synchronous...
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

static void serialPortConfig (int fd)
{
    struct termios  port, tty, old;

    tcgetattr (fd, &port);

    cfsetispeed (&port, twiWorkData.baudrate);
    cfsetospeed (&port, twiWorkData.baudrate);

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

    // bump DTR if enabled:
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

static void storeLoopPkt (WVIEWD_WORK* work, LOOP_PKT *dest, TWI_DATA *src)
{
    float           tempfloat;
    static time_t   lastTempUpdate;
    time_t          nowTime = time(NULL);

    // Clear optional data:
    stationClearLoopData(work);

    // Pressure:
    if ((10 < src->bp && src->bp < 50) &&
        (-150 < src->outTemp && src->outTemp < 200))
    {
        if ((time(NULL) - lastTempUpdate) >= (twiWorkData.archiveInterval * 60))
        {
            // Add for the 12-hour temp average sample:
            // mimic an archive interval:
            sensorAccumAddSample(twi12HourTempAvg, time(NULL), src->outTemp);
            lastTempUpdate = time(NULL);
        }

        // TWI produces sea level pressure (SLP):
        dest->barometer = src->bp;

        // Apply calibration here so the computed values reflect it:
        dest->barometer *= work->calMBarometer;
        dest->barometer += work->calCBarometer;

        // calculate station pressure:
        dest->stationPressure = twiConvertSLPToSP(dest->barometer, 
                                                 sensorAccumGetAverage(twi12HourTempAvg), 
                                                 (float)twiWorkData.elevation);

        // now calculate altimeter:
        dest->altimeter = wvutilsConvertSPToAltimeter(dest->stationPressure,
                                                      (float)twiWorkData.elevation);
    }


    if (-150 < src->inTemp && src->inTemp < 200)
    {
        dest->inTemp                        = src->inTemp;
    }

    if (-150 < src->outTemp && src->outTemp < 200)
    {
        dest->outTemp                       = src->outTemp;
    }

    if (0 <= src->humidity && src->humidity <= 100)
    {
        dest->outHumidity                   = (uint16_t)src->humidity;
        dest->inHumidity                    = (uint16_t)src->humidity;
    }

    if (0 <= src->windSpeed && src->windSpeed <= 250)
    {
        tempfloat = src->windSpeed;
        dest->windSpeedF                    = tempfloat;
        dest->windGustF                     = tempfloat;
    }

    if (0 <= src->windDir && src->windDir < 360)
    {
        tempfloat = src->windDir;
        tempfloat += 0.5;
        dest->windDir                       = (uint16_t)tempfloat;
        dest->windGustDir                   = (uint16_t)tempfloat;
    }

    dest->rainRate                          = src->rainrate;

    // process the rain accumulator:
    if (0 <= src->dayrain)
    {
        if (! work->runningFlag)
        {
            // just starting, so start with whatever the station reports:
            twiWorkData.totalRain = src->dayrain;
            dest->sampleRain = 0;
        }
        else
        {
            // process the rain accumulator
            if (src->dayrain - twiWorkData.totalRain >= 0)
            {
                dest->sampleRain = src->dayrain - twiWorkData.totalRain;
                twiWorkData.totalRain = src->dayrain;
            }
            else
            {
                // we had a counter reset...
                dest->sampleRain = src->dayrain;
                twiWorkData.totalRain = src->dayrain;
            }
        }

        if (dest->sampleRain > 2)
        {
            // Not possible, filter it out:
            dest->sampleRain = 0;
        }

        // Update the rain accumulator:
        sensorAccumAddSample (twiWorkData.rainRateAccumulator, nowTime, dest->sampleRain);
        dest->rainRate    = sensorAccumGetTotal (twiWorkData.rainRateAccumulator);
        dest->rainRate   *= (60/TWI_RAIN_RATE_PERIOD);
    }
    else
    {
        dest->sampleRain = 0;
        sensorAccumAddSample (twiWorkData.rainRateAccumulator, nowTime, dest->sampleRain);
        dest->rainRate   = sensorAccumGetTotal (twiWorkData.rainRateAccumulator);
        dest->rainRate   *= (60/TWI_RAIN_RATE_PERIOD);
    }

    return;
}

