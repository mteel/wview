#ifndef INC_daemonh
#define INC_daemonh
/*---------------------------------------------------------------------------

  FILENAME:
        daemon.h

  PURPOSE:
        Provide the wview daemon definitions.

  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        08/14/03        M.S. Teel       0               Original

  NOTES:


  LICENSE:
        Copyright (c) 2004, Mark S. Teel (mark@teel.ws)

        This source code is released for free distribution under the terms
        of the GNU General Public License.

----------------------------------------------------------------------------*/

/*  ... System include files
*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>

/*  ... Library include files
*/
#include <sysdefs.h>
#include <radsemaphores.h>
#include <radbuffers.h>
#include <radqueue.h>
#include <radtimers.h>
#include <radevents.h>
#include <radtimeUtils.h>
#include <radprocess.h>
#include <radstates.h>
#include <radsocket.h>
#include <radthread.h>
#include <radmsgRouter.h>

/*  ... Local include files
*/
#include <dbsqlite.h>
#include <services.h>
#include <wvconfig.h>
#include <emailAlerts.h>
#include <status.h>
#include <hidapi.h>


/*  !!!!!!!!!!!!!!!!!!  HIDDEN, NOT FOR API USE  !!!!!!!!!!!!!!!!!!
*/

#define WVIEW_MAX_PATH          256

typedef enum
{
    WVIEW_STATS_LOOP_PKTS_RX    = 0,
    WVIEW_STATS_ARCHIVE_PKTS_RX
} WVIEW_STATS;


// define the weather station interface abstraction
#define WV_QUEUE_INPUT          1
#define WV_QUEUE_OUTPUT         2

typedef enum
{
    MEDIUM_TYPE_NONE    = 1,        // For simulator
    MEDIUM_TYPE_DEVICE,             // serial and ethernet devices
    MEDIUM_TYPE_USBHID              // USB HID interfaces
} WVIEW_MEDIUM_TYPE;

typedef struct _wview_medium
{
    WVIEW_MEDIUM_TYPE   type;
    void                *workData;

    // Only some of these are valid for a given medium type:
    // MEDIUM_TYPE_DEVICE
    int                 fd;
    int                 (*init) (struct _wview_medium *medium, char *deviceName);
    void                (*exit) (struct _wview_medium *medium);
    int                 (*restart) (struct _wview_medium *medium);
    int                 (*read) (struct _wview_medium *medium, void *bfr, int len, int timeout);
    int                 (*write) (struct _wview_medium *medium, void *buffer, int length);
    void                (*flush) (struct _wview_medium *medium, int queue);
    void                (*txdrain) (struct _wview_medium *medium);
    RADSOCK_ID          (*getsocket) (struct _wview_medium *medium);

    // MEDIUM_TYPE_USBHID
    hid_device*         hidDevice;
    int                 (*usbhidInit)(struct _wview_medium *medium);
    void                (*usbhidExit)(struct _wview_medium *medium);
    int                 (*usbhidRead) (struct _wview_medium *medium, void *bfr, int len, int timeout);
    int                 (*usbhidReadSpecial) (struct _wview_medium *medium, void *bfr, int len, int timeout);
    int                 (*usbhidWrite) (struct _wview_medium *medium, void *buffer, int length);
    
} WVIEW_MEDIUM;


// 4 hours in ms
#define WVD_TIME_SYNC_INTERVAL          (4L * 60L * 60L * 1000L)

// initial console wakeup number of attempts
#define WVD_INITIAL_WAKEUP_TRIES        20

// time to wait before attempting read recovery
#define WVD_READ_RECOVER_INTERVAL       2500L

#define WVD_READ_RECOVER_MAX_RETRIES    1000

// Number of archive intervals in 4 hours (flat line base values will trigger an alert):
#define WVD_FLATLINE_THRESHOLD(archiveInterval) (240/archiveInterval)

// the wview daemon work area
typedef struct
{
    pid_t           myPid;
    WVIEW_MEDIUM    medium;
    void            *stationData;               // station-specific data store
    int             runningFlag;
    RAD_THREAD_ID   threadId;                   // Non-NULL if station uses threads

    int             stationGeneratesArchives;
    char            pidFile[WVIEW_MAX_PATH];
    char            fifoFile[WVIEW_MAX_PATH];
    char            statusFile[WVIEW_MAX_PATH];
    char            stationType[64];
    char            stationInterface[16];
    char            stationDevice[WVIEW_MAX_PATH];
    char            stationHost[256];
    int             stationPort;
    int             stationIsWLIP;
    int             stationToggleDTR;
    int             stationRainSeasonStart;
    float           stationRainStormTrigger;
    int             stationRainStormIdleHours;
    float           stationRainYTDPreset;
    float           stationETYTDPreset;
    int             stationRainETPresetYear;
    uint16_t        archiveInterval;
    int16_t         latitude;
    int16_t         longitude;
    int16_t         elevation;
    time_t          archiveDateTime;
    time_t          nextArchiveTime;            // detect system clock changes
    TIMER_ID        archiveTimer;
    TIMER_ID        cdataTimer;
    TIMER_ID        pushTimer;
    TIMER_ID        syncTimer;
    TIMER_ID        ifTimer;
    uint32_t        cdataInterval;
    uint32_t        pushInterval;
    SENSOR_STORE    sensors;
    LOOP_PKT        loopPkt;
    LOOP_PKT        lastLoopPkt;
    int             numReadRetries;
    int             archiveRqstPending;

    // Calibration:
    float           calMBarometer;
    float           calCBarometer;
    float           calMPressure;
    float           calCPressure;
    float           calMAltimeter;
    float           calCAltimeter;
    float           calMInTemp;
    float           calCInTemp;
    float           calMOutTemp;
    float           calCOutTemp;
    float           calMInHumidity;
    float           calCInHumidity;
    float           calMOutHumidity;
    float           calCOutHumidity;
    float           calMWindSpeed;
    float           calCWindSpeed;
    float           calMWindDir;
    float           calCWindDir;
    float           calMRain;
    float           calCRain;
    float           calMRainRate;
    float           calCRainRate;

    // Alert Emails:
    int             IsAlertEmailsEnabled;
    char            alertEmailFromAdrs[WVIEW_STRING1_SIZE];
    char            alertEmailToAdrs[WVIEW_STRING1_SIZE];
    int             IsTestEmailEnabled;

    int             showStationIF;

    int             exiting;

} WVIEWD_WORK;


/*  !!!!!!!!!!!!!!!!!!!!  END HIDDEN SECTION  !!!!!!!!!!!!!!!!!!!!!
*/

// Retrieve exit status:
extern int wviewdIsExiting(void);


#endif

