#ifndef INC_vproInterfaceh
#define INC_vproInterfaceh
/*---------------------------------------------------------------------------

  FILENAME:
        vproInterface.h

  PURPOSE:
        Provide non-medium-specific utilities for the Davis Vantage Pro
        interface protocol.

  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        06/07/2005      M.S. Teel       0               Original
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
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <termios.h>
#include <time.h>
#include <errno.h>
#include <math.h>

/*  ... Library include files
*/
#include <sysdefs.h>
#include <radtimeUtils.h>
#include <radmsgLog.h>
#include <radsysutils.h>

/*  ... Local include files
*/
#include <datadefs.h>
#include <dbsqlite.h>
#include <dbfiles.h>
#include <config.h>
#include <computedData.h>
#include <daemon.h>
#include <station.h>
#include <serial.h>
#include <ethernet.h>
#include <sensor.h>

/*  ... some definitions
*/

#define VP_BYTE_LENGTH_MAX              SERIAL_BYTE_LENGTH_MAX

#define VP_ACK                          0x06
#define VP_NAK                          0x21
#define VP_CANCEL                       0x1B
#define VP_BADCRC                       0x18
#define VP_CR                           0x0D
#define VP_LF                           0x0A


#define VP_RESPONSE_TIMEOUT(isIP)       ((isIP) ? 10000L : 5000L)

// time to wait before attempting archive record again:
#define VPIF_RETRY_ARCHIVE_INTERVAL     10000L

#define VP_PARM_DO_RXCHECK              "DO_RXCHECK"


//  ... define the message types we receive
typedef enum
{
    SER_MSG_ACK             = 1,
    SER_MSG_ARCHIVE         = 2,
    SER_MSG_DMPAFT_HDR      = 3,
    SER_MSG_LOOP            = 4,
    SER_MSG_NONE            = -1
} SER_MSG_TYPES;


//  ... define the messages we receive
typedef struct
{
    uint8_t             interval;
    uint16_t            crc;
}__attribute__ ((packed)) ARCHIVE_INTERVAL;

typedef struct
{
    uint16_t            pages;
    uint16_t            firstRecIndex;
    uint16_t            crc;
}__attribute__ ((packed)) DMPAFT_HDR;

typedef struct
{
    uint8_t             seqNo;
    ARCHIVE_RECORD      record[5];
    uint8_t             unused[4];
    uint16_t            crc;
}__attribute__ ((packed)) ARCHIVE_PAGE;

//  ... serial format
/*  ... define the LOOP (current readings) serial data format
*/
typedef struct
{
    uint8_t             name[4];
    uint8_t             type;
    uint16_t            nextRecord;
    uint16_t            barometer;
    int16_t             inTemp;
    uint8_t             inHumidity;
    int16_t             outTemp;
    uint8_t             windSpeed;
    uint8_t             tenMinuteAvgWindSpeed;
    uint16_t            windDir;
    uint8_t             extraTemp1;
    uint8_t             extraTemp2;
    uint8_t             extraTemp3;
    uint8_t             resvExtraTemps[4];
    uint8_t             soilTemp1;
    uint8_t             soilTemp2;
    uint8_t             soilTemp3;
    uint8_t             soilTemp4;
    uint8_t             leafTemp1;
    uint8_t             leafTemp2;
    uint8_t             resvLeafTemp[2];
    uint8_t             outHumidity;
    uint8_t             extraHumid1;
    uint8_t             extraHumid2;
    uint8_t             resvHumidity[5];
    uint16_t            rainRate;
    uint8_t             UV;
    uint16_t            radiation;
    uint16_t            stormRain;
    uint16_t            stormStartDate;
    uint16_t            dayRain;
    uint16_t            monthRain;
    uint16_t            yearRain;
    uint16_t            dayET;
    uint16_t            monthET;
    uint16_t            yearET;
    uint8_t             soilMoist1;
    uint8_t             soilMoist2;
    uint8_t             resvSoilMoist[2];
    uint8_t             leafWet1;
    uint8_t             leafWet2;
    uint8_t             resvLeafWet[2];
    uint8_t             resvAlarms[16];
    uint8_t             txBatteryStatus;
    uint16_t            consBatteryVoltage;
    uint8_t             forecastIcon;
    uint8_t             forecastRule;
    uint16_t            sunrise;
    uint16_t            sunset;
    uint8_t             lf;
    uint8_t             cr;
    uint16_t            crc;
}__attribute__ ((packed)) LOOP_DATA;


// define Vantage Pro specific interface data here
typedef struct
{
    STATES_ID       stateMachine;
    SER_MSG_TYPES   reqMsgType;
    int16_t         elevation;
    uint16_t        archivePages;
    uint16_t        archiveCurrentPage;
    uint16_t        archiveRecOffset;
    int             rxCheckGood;
    int             rxCheckMissed;
    int             rxCheckCRC;
    int             doRXCheck;
    char            rxCheck[64];
    uint16_t        rxCheckPercent;
    int             timeSyncFlag;
    int             archiveRetryFlag;
    int             doLoopFlag;
    int             doArchiveFlag;
    int             sampleRain;             // to track dailyRain changes
    int             sampleET;               // to track dayET changes

    // vpconfig only
    char            fwVersion[32];
    int             rainSeasonStart;
    int16_t         windDirectionCal;
    uint8_t         listenChannels;
    uint8_t         retransmitChannel;
    uint8_t         transmitterType[16];
    int             rainCollectorSize;
    int             windCupSize;

    // rain
    uint16_t        RainCollectorType;
    float           rainTicksPerInch;

} VP_IF_DATA;


// define some extra stimulus types for our purposes
typedef enum
{
    VP_STIM_READINGS        = STIM_IO + 1,
    VP_STIM_ARCHIVE         = STIM_IO + 2
} VPStims;


// ... function prototypes

// ... flush the input buffer from the VP
extern void vpifFlush (WVIEWD_WORK *work);

// ... wake up the Vantage Pro console;
// ... returns OK or ERROR
extern int vpifWakeupConsole (WVIEWD_WORK *work);

// ... get the date and time from the Vantage Pro console;
// ... returns OK or ERROR
extern int vpifGetTime
(
    WVIEWD_WORK *work,
    uint16_t    *year,
    uint16_t    *month,
    uint16_t    *day,
    uint16_t    *hour,
    uint16_t    *minute,
    uint16_t    *second
);

// ... get the console's stored latitude and longitude
// ... return OK or ERROR
extern int vpifGetLatandLong
(
    WVIEWD_WORK *work
);

// ... get the console's archive interval
extern int vpifGetArchiveInterval (WVIEWD_WORK *work);

// ... get the console's rain collector size:
extern int vpifGetRainCollectorSize (WVIEWD_WORK *work);

// ... set the date and time for the Vantage Pro console;
// ... returns OK or ERROR
extern int vpifSetTime
(
    WVIEWD_WORK *work,
    uint16_t    year,               // ex: 2001 1998 2004
    uint16_t    month,
    uint16_t    day,
    uint16_t    hour,               // 24 hours
    uint16_t    minute,
    uint16_t    second
);

// ... set the GMT offset for the Vantage Pro console;
// ... returns OK or ERROR
extern int vpifSetGMTOffset
(
    WVIEWD_WORK *work,
    int         *offset
);

extern int vpifGetAck (WVIEWD_WORK *work, int msWait);
extern int vpifGetRXCheck (WVIEWD_WORK *work);
extern void vpifIndicateStationUp (void);
extern void vpifIndicateLoopDone (void);
extern int vpifSynchronizeConsoleClock (WVIEWD_WORK *work);


// ... this guy reads/parses msgs and updates the internal data stores;
// ... uses the "reqMsgType" of the work area to determine msg type to read;
// ... returns OK or ERROR
extern int vpifReadMessage (WVIEWD_WORK *work, int expectACKFirst);


// ... send msgs to the console;
// ... will set the "reqMsgType" of the work area appropriately;
// ... returns OK or ERROR
extern int vpifSendAck (WVIEWD_WORK *work);
extern int vpifSendNak (WVIEWD_WORK *work);
extern int vpifSendCancel (WVIEWD_WORK *work);
extern int vpifSendDumpAfterRqst (WVIEWD_WORK *work);
extern int vpifSendDumpDateTimeRqst (WVIEWD_WORK *work);
extern int vpifSendLoopRqst (WVIEWD_WORK *work, int number);


// ... define the VP state machine states
typedef enum
{
    VPRO_STATE_STARTPROC            = 1,
    VPRO_STATE_RUN,
    VPRO_STATE_DMPAFT_RQST,
    VPRO_STATE_DMPAFT_ACK,
    VPRO_STATE_RECV_ARCH,
    VPRO_STATE_LOOP_RQST,
    VPRO_STATE_READ_RECOVER,
    VPRO_STATE_ERROR
} VPRO_STATES;

extern int vproStartProcState (int state, void *stimulus, void *data);
extern int vproRunState (int state, void *stimulus, void *data);
extern int vproDumpAfterState (int state, void *stimulus, void *data);
extern int vproDumpAfterAckState (int state, void *stimulus, void *data);
extern int vproReceiveArchiveState (int state, void *stimulus, void *data);
extern int vproLoopState (int state, void *stimulus, void *data);
extern int vproReadRecoverState (int state, void *stimulus, void *data);
extern int vproStopState (int state, void *stimulus, void *data);
extern int vproErrorState (int state, void *stimulus, void *data);


#ifdef _VP_CONFIG_ONLY
typedef enum
{
  VPRO_SENSOR_ISS                   = 0x00,
  VPRO_SENSOR_TEMP                  = 0x01,
  VPRO_SENSOR_HUM                   = 0x02,
  VPRO_SENSOR_TEMP_HUM              = 0x03,
  VPRO_SENSOR_WIND                  = 0x04,
  VPRO_SENSOR_RAIN                  = 0x05,
  VPRO_SENSOR_LEAF                  = 0x06,
  VPRO_SENSOR_SOIL                  = 0x07,
  VPRO_SENSOR_LEAF_SOIL             = 0x08,
  VPRO_SENSOR_SENSORLINK            = 0x09,
  VPRO_SENSOR_NONE                  = 0x0A
} VPRO_SENSOR_TYPES;

typedef enum
{
  VPRO_RAIN_COLLECTOR_0_01_IN       = 0x0,
  VPRO_RAIN_COLLECTOR_0_2_MM        = 0x1,
  VPRO_RAIN_COLLECTOR_0_1_MM        = 0x2
} VPRO_RAIN_COLLECTOR_SIZE;

//  ... methods for vpconfig only
extern int vpconfigGetArchiveInterval (WVIEWD_WORK *work);
extern int vpconfigGetFWVersion (WVIEWD_WORK *work);
extern int vpconfigGetRainSeasonStart (WVIEWD_WORK *work);
extern int vpconfigGetWindDirectionCal (WVIEWD_WORK *work);
extern int vpconfigGetTransmitters (WVIEWD_WORK *work);
extern int vpconfigGetRainCollectorSize (WVIEWD_WORK *work);
extern int vpconfigGetWindCupSize (WVIEWD_WORK *work);
extern int vpconfigSetInterval (WVIEWD_WORK *work, int interval);
extern int vpconfigClearArchiveMemory (WVIEWD_WORK *work);
extern int vpconfigSetElevation (WVIEWD_WORK *work, int elevation);
extern int vpconfigSetGain (WVIEWD_WORK *work, int on);
extern int vpconfigSetLatandLong (WVIEWD_WORK *work, int latitude, int longitude);
extern int vpconfigSetRainSeasonStart (WVIEWD_WORK *work, int startMonth);
extern int vpconfigSetRainYearToDate (WVIEWD_WORK *work, float rainAmount);
extern int vpconfigSetETYearToDate (WVIEWD_WORK *work, float etAmount);
extern int vpconfigSetWindDirectionCal (WVIEWD_WORK *work, int offset);
extern int vpconfigSetSensor (WVIEWD_WORK *work, int channel, VPRO_SENSOR_TYPES sensorType);
extern int vpconfigSetRetransmitChannel (WVIEWD_WORK *work, int channel);
extern int vpconfigSetRainCollectorSize (WVIEWD_WORK *work, VPRO_RAIN_COLLECTOR_SIZE rainCollectorSize);
extern int vpconfigSetWindCupSize (WVIEWD_WORK *work, int isLarge);
#endif

#endif

