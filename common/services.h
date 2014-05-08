#ifndef INC_servicesh
#define INC_servicesh
/*---------------------------------------------------------------------------
 
  FILENAME:
        services.h
 
  PURPOSE:
        Provide the wview daemon application sevices definitions.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        08/24/03        M.S. Teel       0               Original
 
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
#include <string.h>
#include <signal.h>
#include <math.h>
#include <sys/stat.h>

/*  ... Library include files
*/
#include <sysdefs.h>

/*  ... Local include files
*/
#include <datadefs.h>


/*  ... API definitions
*/

/*  ... request types for the WVIEW_MSG_TYPE_REQUEST message
*/
typedef enum
{
    WVIEW_RQST_TYPE_STATION_INFO        = 1,
    WVIEW_RQST_TYPE_HILOW_DATA          = 2,
    WVIEW_RQST_TYPE_LOOP_DATA           = 3
} WVIEW_RQST_TYPES;

/*  ... message types:
    ... currently the wviewd daemon only receives the WVIEW_MSG_TYPE_REQUEST
    ... message and sends the other types
*/
typedef enum
{
    WVIEW_MSG_TYPE_REQUEST              = 1,
    WVIEW_MSG_TYPE_STATION_INFO         = 2,
    WVIEW_MSG_TYPE_HILOW_DATA           = 3,
    WVIEW_MSG_TYPE_LOOP_DATA            = 4,
    WVIEW_MSG_TYPE_LOOP_DATA_SVC        = 5,
    WVIEW_MSG_TYPE_ARCHIVE_NOTIFY       = 6,
    WVIEW_MSG_TYPE_ARCHIVE_DATA         = 7,

    // These are outside the "RQST_TYPE" paradigm:
    WVIEW_MSG_TYPE_POLL                 = 8,
    WVIEW_MSG_TYPE_POLL_RESPONSE        = 9,
    WVIEW_MSG_TYPE_ALERT                = 10,
    WVIEW_MSG_TYPE_SHUTDOWN             = 11,
    WVIEW_MSG_TYPE_STATION_DATA         = 12        // msg structure is station-specific

} WVIEW_MSG_TYPES;


/*  ... WVIEW_MSG_TYPE_REQUEST
    ... requestType is one of WVIEW_RQST_TYPES;
    ... will cause that msg/action to be sent/performed
*/
typedef struct
{
    int             requestType;
    int             enable;                     // enable/disable msg reception
    // for "SVC" requests
}
__attribute__ ((packed)) WVIEW_MSG_REQUEST;

/*  ... WVIEW_MSG_TYPE_STATION_INFO
    ... returns station info
*/
typedef struct
{
    time_t          lastArcTime;
    int16_t         archiveInterval;
    int16_t         latitude;
    int16_t         longitude;
    int16_t         elevation;
    char            stationType[_MAX_PATH];

}
__attribute__ ((packed)) WVIEW_MSG_STATION_INFO;

/*  ... WVIEW_MSG_TYPE_HILOW_DATA
    ... returns the most recent HILOW data
*/
typedef struct
{
    SENSOR_STORE    hilowData;

}
__attribute__ ((packed)) WVIEW_MSG_HILOW_DATA;

/*  ... WVIEW_MSG_TYPE_LOOP_DATA
    ... returns the most recent loop data
*/
typedef struct
{
    LOOP_PKT        loopData;

}
__attribute__ ((packed)) WVIEW_MSG_LOOP_DATA;

/*  ... WVIEW_MSG_TYPE_ARCHIVE_DATA
    ... publishes archive records
*/
typedef struct
{
    ARCHIVE_PKT     archiveData;

}
__attribute__ ((packed)) WVIEW_MSG_ARCHIVE_DATA;

/*  ... WVIEW_MSG_TYPE_ARCHIVE_NOTIFY
    ... notification of the reception of an archive record from the VP console
    ... and pertinent data from that record
*/
typedef struct
{
    time_t          dateTime;
    int             intemp;
    int             inhumidity;
    int             temp;
    int             humidity;
    int             barom;
    int             stationPressure;
    int             altimeter;
    int             winddir;
    float           wspeedF;
    float           hiwspeedF;
    float           sampleRain;
    float           rainHour;
    float           rainDay;
    float           rainToday;
    int             dewpoint;
    float           UV;
    float           radiation;
    int             rxPercent;

}
__attribute__ ((packed)) WVIEW_MSG_ARCHIVE_NOTIFY;


//  WVIEW_MSG_TYPE_POLL:
//  Sent by wvpmond to detect hung processes
typedef struct
{
    int             mask;
}
__attribute__ ((packed)) WVIEW_MSG_POLL;


//  WVIEW_MSG_TYPE_POLL_RESPONSE:
//  Sent by wview processes to indicate they are healthy
typedef struct
{
    int             pid;
}
__attribute__ ((packed)) WVIEW_MSG_POLL_RESPONSE;


//  WVIEW_MSG_TYPE_ALERT:
//  Sent by wview processes to indicate an alert
typedef struct
{
    int             alertType;
}
__attribute__ ((packed)) WVIEW_MSG_ALERT;


//  WVIEW_MSG_TYPE_SHUTDOWN:
//  Sent by wviewd process to indicate he is shutting down:
typedef struct
{
    int             placeholder;
}
__attribute__ ((packed)) WVIEW_MSG_SHUTDOWN;


#endif

