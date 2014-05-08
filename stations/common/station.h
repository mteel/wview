#ifndef INC_stationh
#define INC_stationh
/*---------------------------------------------------------------------------
 
  FILENAME:
        station.h
 
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
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

/*  ... Library include files
*/
#include <sysdefs.h>
#include <radtimeUtils.h>
#include <radmsgLog.h>
#include <radsysutils.h>

/*  ... Local include files
*/
#include <sysdefs.h>
#include <datadefs.h>
#include <services.h>
#include <sensor.h>
#include <dbsqlite.h>
#include <daemon.h>

#define SERIAL_BYTE_LENGTH_MAX          1024


//////////////////////////////////////////////////////////////////////////////
// Station-Specific Prototypes (to be supplied by station interface)
//////////////////////////////////////////////////////////////////////////////

// station-supplied init function
// -- Can Be Asynchronous - event indication required --
//
// MUST (in this order):
//   - set the 'stationGeneratesArchives' flag in WVIEWD_WORK:
//     if the station generates archive records (TRUE) or they should be 
//     generated automatically by the daemon from the sensor readings (FALSE)
//   - Initialize the 'stationData' store for station work area
//   - Initialize the interface medium
//   - determine the station archive interval - either from the station itself
//     or from user configuration in wview.conf - and set the 
//     'work->archiveInterval' variable (in minutes) in WVIEWD_WORK
//   - VERIFY the archive interval by calling 'stationVerifyArchiveInterval' -
//     If they don't match, indicate an errant start via the call:
//     radProcessEventsSend (NULL, STATION_INIT_COMPLETE_EVENT, 1)
//     and stopping further init activities here!
//   - do any catch-up on archive records if there is a data logger (can be 
//     asynchronous) - the 'work->runningFlag' can be used for start up 
//     synchronization but should not be modified by the station interface code
//   - do initial LOOP acquisition
//   - indicate successful initialization is done via the call:
//     radProcessEventsSend (NULL, STATION_INIT_COMPLETE_EVENT, 0)
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
extern int stationInit
(
    WVIEWD_WORK     *work,
    void            (*archiveIndication)(ARCHIVE_PKT* newRecord)
);


// station-supplied exit function
//
// Returns: N/A
//
extern void stationExit (WVIEWD_WORK *work);


// station-supplied function to retrieve positional info (lat, long, elev) -
// should populate WVIEWD_WORK fields: latitude, longitude, elevation
// -- Synchronous --
//
// - If station does not store these parameters, they can be retrieved from the
//   wview.conf file (see the 'stationGetConfigValue' utilities below) - user 
//   must choose station type "Generic" when running the wviewconfig script
//
// Returns: OK or ERROR
//
extern int stationGetPosition (WVIEWD_WORK *work);


// station-supplied function to indicate a time sync should be performed if the
// station maintains time, otherwise may be safely ignored
// -- Can Be Asynchronous --
//
// Returns: OK or ERROR
//
extern int stationSyncTime (WVIEWD_WORK *work);


// station-supplied function to indicate sensor readings should be performed -
// should populate 'work' struct: loopPkt (see datadefs.h for minimum field reqs)
// -- Can Be Asynchronous --
//
// - indicate readings are complete by sending the STATION_LOOP_COMPLETE_EVENT
//   event to this process (radProcessEventsSend (NULL, STATION_LOOP_COMPLETE_EVENT, 0))
//
// Returns: OK or ERROR
//
extern int stationGetReadings (WVIEWD_WORK *work);


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
extern int stationGetArchive (WVIEWD_WORK *work);


// station-supplied function to indicate data is available on the station 
// interface medium (serial or ethernet) -
// It is the responsibility of the station interface to read the data from the 
// medium and process appropriately. The data does not have to be read within
// the context of this function, but may be used to stimulate a state machine.
// -- Synchronous --
//
// Returns: N/A
//
extern void stationDataIndicate (WVIEWD_WORK *work);


// station-supplied function to receive IPM messages - any message received by
// the generic station message handler which is not recognized will be passed
// to the station-specific code through this function.
// It is the responsibility of the station interface to process the message 
// appropriately (or ignore it).
// -- Synchronous --
//
// Returns: N/A
//
extern void stationMessageIndicate (WVIEWD_WORK *work, int msgType, void *msg);


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
extern void stationIFTimerExpiry (WVIEWD_WORK *work);


//////////////////////////////////////////////////////////////////////////////
// Station-Generic Definitions
//////////////////////////////////////////////////////////////////////////////

#define STATION_PARM_ELEVATION          "STATION_ELEVATION"
#define STATION_PARM_LATITUDE           "STATION_LATITUDE"
#define STATION_PARM_LONGITUDE          "STATION_LONGITUDE"
#define STATION_PARM_ARC_INTERVAL       "STATION_ARCHIVE_INTERVAL"
#define STATION_PARM_OUTSIDE_CHANNEL    "STATION_OUTSIDE_CHANNEL"



//////////////////////////////////////////////////////////////////////////////
// Station-Generic Prototypes
//////////////////////////////////////////////////////////////////////////////

// send shutdown notification
extern int stationSendShutdown (WVIEWD_WORK *work);

// send archive record notification
extern int stationSendArchiveNotifications (WVIEWD_WORK *work, float sampleRain);

// send the archive path message
extern int stationProcessInfoResponses (WVIEWD_WORK *work);

// process received IPM messages
extern int stationProcessIPM (WVIEWD_WORK *work, char *srcQueueName, int msgType, void *msg);

// push LOOP data to clients
extern int stationPushDataToClients (WVIEWD_WORK *work);

// push Archive data to clients
extern int stationPushArchiveToClients(WVIEWD_WORK *work, ARCHIVE_PKT* pktToSend);

// start the archive timer uniformly
extern void stationStartArchiveTimerUniform (WVIEWD_WORK *work);

// start the data acquisition timer uniformly
extern void stationStartCDataTimerUniform (WVIEWD_WORK *work);

// start the station time sync timer (if required) uniformly:
// Returns TRUE if sync should be done or FALSE otherwise
extern int stationStartSyncTimerUniform (WVIEWD_WORK *work, int firstTime);

// verify the archive interval configured in the station or set by the user in
// wview-conf.sdb to confirm it matches the archive data
extern int stationVerifyArchiveInterval (WVIEWD_WORK *work);

// several to retrieve config values out of wview.conf
extern int stationGetConfigValueInt (WVIEWD_WORK *work, char *configName, int *store);
extern int stationGetConfigValueFloat (WVIEWD_WORK *work, char *configName, float *store);
extern int stationGetConfigValueBoolean (WVIEWD_WORK *work, char *configName, int *store);

// Clear optional station data in the loop packt:
extern void stationClearLoopData (WVIEWD_WORK *work);

#endif

