/*---------------------------------------------------------------------------
 
  FILENAME:
        simulator.c
 
  PURPOSE:
        Provide the station simulator interface API and utilities.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        01/19/2006      M.S. Teel       0               Original
 
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
#include <simulator.h>

/*  ... global memory declarations
*/

/*  ... local memory
*/
#define PI                  3.1415926535897932384626433832795
#define RAD                 (PI/180.0)
#define PERIOD_FACTOR       4

static SIMULATOR_IF_DATA    simWorkData;
static void                 (*ArchiveIndicator) (ARCHIVE_PKT* newRecord);
static int                  DataGenerator;

static void                 serialPortConfig (int fd);
static void                 storeLoopPkt (WVIEWD_WORK *work, LOOP_PKT *dest);



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
    int             minutes;

    memset (&simWorkData, 0, sizeof(simWorkData));

    // save the archive indication callback (we should never need it)
    ArchiveIndicator = archiveIndication;

    // set our work data pointer
    work->stationData = &simWorkData;

    // set the Archive Generation flag to indicate the Simulator DOES NOT 
    // generate them
    work->stationGeneratesArchives = FALSE;

    // initialize the medium abstraction based on user configuration
    // we won't really open a serial channel...
    work->medium.type = MEDIUM_TYPE_NONE;

    // grab the station configuration now
    if (stationGetConfigValueInt (work, 
                                  STATION_PARM_ELEVATION, 
                                  &simWorkData.elevation) 
        == ERROR)
    {
        radMsgLog (PRI_HIGH, "stationInit: stationGetConfigValueInt ELEV failed!");
        (*(work->medium.exit)) (&work->medium);
        return ERROR;
    }
    if (stationGetConfigValueFloat (work, 
                                    STATION_PARM_LATITUDE, 
                                    &simWorkData.latitude) 
        == ERROR)
    {
        radMsgLog (PRI_HIGH, "stationInit: stationGetConfigValueInt LAT failed!");
        (*(work->medium.exit)) (&work->medium);
        return ERROR;
    }
    if (stationGetConfigValueFloat (work, 
                                    STATION_PARM_LONGITUDE, 
                                    &simWorkData.longitude) 
        == ERROR)
    {
        radMsgLog (PRI_HIGH, "stationInit: stationGetConfigValueInt LONG failed!");
        (*(work->medium.exit)) (&work->medium);
        return ERROR;
    }
    if (stationGetConfigValueInt (work, 
                                  STATION_PARM_ARC_INTERVAL, 
                                  &simWorkData.archiveInterval) 
        == ERROR)
    {
        radMsgLog (PRI_HIGH, "stationInit: stationGetConfigValueInt ARCINT failed!");
        (*(work->medium.exit)) (&work->medium);
        return ERROR;
    }

    // set the work archive interval now
    work->archiveInterval = simWorkData.archiveInterval;

    // sanity check the archive interval against the most recent record
    if (stationVerifyArchiveInterval (work) == ERROR)
    {
        // bad magic!
        radMsgLog (PRI_HIGH, "stationInit: stationVerifyArchiveInterval failed!");
        radMsgLog (PRI_HIGH, "You must either move old archive data out of the way -or-");
        radMsgLog (PRI_HIGH, "fix the interval setting...");
        return ERROR;
    }
    else
    {
        radMsgLog (PRI_STATUS, "station archive interval: %d minutes",
                   work->archiveInterval);
    }


    // initialize the station interface
    // nothing to do here...
    radMsgLog (PRI_STATUS, "Starting station interface: Simulator"); 


    // do the initial GetReadings now
    // populate the LOOP structure
    storeLoopPkt (work, &work->loopPkt);

    // compute the data generation period
    minutes = 360 * PERIOD_FACTOR;
    minutes *= (work->cdataInterval/1000);
    minutes /= 60;

    radMsgLog (PRI_STATUS, "Simulator station opened: %d minute data generation period...",
               minutes);

    // we can indicate successful completion here!
    radProcessEventsSend (NULL, STATION_INIT_COMPLETE_EVENT, 0);

    return OK;
}

// station-supplied exit function
//
// Returns: N/A
//
void stationExit (WVIEWD_WORK *work)
{
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
    work->elevation     = (int16_t)simWorkData.elevation;
    if (simWorkData.latitude >= 0)
        work->latitude      = (int16_t)((simWorkData.latitude*10)+0.5);
    else
        work->latitude      = (int16_t)((simWorkData.latitude*10)-0.5);
    if (simWorkData.longitude >= 0)
        work->longitude     = (int16_t)((simWorkData.longitude*10)+0.5);
    else
        work->longitude     = (int16_t)((simWorkData.longitude*10)-0.5);
    
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
    // Simulator does not keep time...
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

    // populate the LOOP structure (with dummy data)
    storeLoopPkt (work, &work->loopPkt);

    // indicate we are done
    radProcessEventsSend (NULL, STATION_LOOP_COMPLETE_EVENT, 0);
    
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
    // just indicate a NULL record, Simulator does not generate them (and this 
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
    // Simulator station is synchronous...
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
    // Simulator station is synchronous...
    return;
}

////////////****////  S T A T I O N   A P I   E N D  ////****////////////


//  ... ----- static (local) methods ----- ...

static void storeLoopPkt (WVIEWD_WORK *work, LOOP_PKT *dest)
{
    float       tempfloat;

    // Clear optional data:
    stationClearLoopData(work);


    // vary BP from 29.75 to 30.25
    tempfloat = 30.00;
    tempfloat += sin((float)DataGenerator*RAD/PERIOD_FACTOR) * 0.25;
    // Sim "produces" sea level pressure (SLP)
    dest->barometer = tempfloat;

    // vary TEMP from 60 to 80
    tempfloat = 70;
    tempfloat += sin((float)DataGenerator*RAD/PERIOD_FACTOR) * 10.0;
    dest->outTemp = tempfloat;

    // Apply calibration here so the computed values reflect it:
    dest->barometer *= work->calMBarometer;
    dest->barometer += work->calCBarometer;

    // calculate station pressure by giving a negative elevation 
    dest->stationPressure = wvutilsConvertSPToSLP(dest->barometer, 
                                                dest->outTemp,
                                                (float)(-simWorkData.elevation));

    // calculate altimeter
    dest->altimeter = wvutilsConvertSPToAltimeter(dest->stationPressure,
                                                (float)simWorkData.elevation);

    // vary HUMIDITY from 60 to 80
    tempfloat = 70.0;
    tempfloat -= sin((float)DataGenerator*RAD/PERIOD_FACTOR) * 10.0;
    dest->outHumidity = (uint16_t)tempfloat;

    // vary WINDSPD from 5 to 15
    tempfloat = 10;
    tempfloat += sin((float)DataGenerator*RAD/PERIOD_FACTOR) * 5.0;
    dest->windSpeedF = tempfloat;

    // vary WINDDIR from 0 to 90
    tempfloat = 0;
    tempfloat += fabsf(sin((float)DataGenerator*RAD/PERIOD_FACTOR)) * 90.0;
    dest->windDir = (uint16_t)tempfloat;

    // vary MAXWIND from 15 to 25
    tempfloat = 20;
    tempfloat += sin((float)DataGenerator*RAD/PERIOD_FACTOR) * 5.0;
    dest->windGustF = tempfloat;

    // vary MAXWINDDIR from 0 to 90
    tempfloat = 45;
    tempfloat += sin((float)DataGenerator*RAD/PERIOD_FACTOR) * 45.0;
    dest->windGustDir = (uint16_t)tempfloat;

    // vary RAINRATE from 0.00 to 4.8
    tempfloat = 0;
    tempfloat += fabsf(sin((float)DataGenerator*RAD/PERIOD_FACTOR)) * 4.8;
    dest->rainRate = tempfloat;

    // vary RAIN from 0.00 to 0.02
    tempfloat = 0;
    tempfloat += fabsf(sin((float)DataGenerator*RAD/PERIOD_FACTOR)) * 0.02;
    dest->sampleRain = tempfloat;

    // bump our data generator index
    DataGenerator ++;
    DataGenerator %= (360*PERIOD_FACTOR);
    
    return;
}

