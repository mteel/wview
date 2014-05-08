#ifndef INC_emailalertsh
#define INC_emailalertsh
/*---------------------------------------------------------------------------
 
  FILENAME:
        emailAlerts.h
 
  PURPOSE:
        Define the Alert Email API.
        Define system alert types.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        03/07/2009      M.S. Teel       0               Original
 
  NOTES:
        
 
  LICENSE:
        Copyright (c) 2009, Mark S. Teel (mark@teel.ws)
  
        This source code is released for free distribution under the terms 
        of the GNU General Public License.
  
----------------------------------------------------------------------------*/

//  ... includes
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

#include <sysdefs.h>
#include <services.h>


//  ... definitions
#define ALERT_NOTIFY_INTERVAL           3600

typedef enum
{
    ALERT_TYPE_TEST                     = 0,
    ALERT_TYPE_FILE_IO,
    ALERT_TYPE_STATION_VP_WAKEUP,
    ALERT_TYPE_STATION_LOOP,
    ALERT_TYPE_STATION_ARCHIVE,
    ALERT_TYPE_STATION_DEVICE,
    ALERT_TYPE_STATION_READ,
    ALERT_TYPE_STATION_FLATLINE,
    ALERT_TYPE_MAX
} EmailAlertTypes;


typedef struct
{
    char*               subject;
    char*               body;
}__attribute__ ((packed)) ALERT_INFO;



//  ... API prototypes

//  ... send an alert email with given alert type:
extern int emailAlertSend (EmailAlertTypes type);

extern char* emailAlertGetSubject (EmailAlertTypes type);

extern char* emailAlertGetBody (EmailAlertTypes type);

#endif


