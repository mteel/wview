#ifndef INC_httph
#define INC_httph
/*---------------------------------------------------------------------------
 
  FILENAME:
        http.h
 
  PURPOSE:
        Provide the wview http process definitions.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        07/12/2005      M.S. Teel       0               
        12/09/2007      M.S. Teel       1               Add WeatherForYou.com
 
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
#include <radsystem.h>
#include <radsemaphores.h>
#include <radbuffers.h>
#include <radlist.h>
#include <radqueue.h>
#include <radtimers.h>
#include <radevents.h>
#include <radtimeUtils.h>
#include <radsysutils.h>
#include <radprocess.h>
#include <radsocket.h>
#include <curl/curl.h>

/*  ... Local include files
*/
#include <services.h>
#include <sysdefs.h>
#include <datadefs.h>
#include <sensor.h>
#include <dbsqlite.h>
#include <wvconfig.h>
#include <status.h>



/*  ... API definitions
*/


/*  !!!!!!!!!!!!!!!!!!  HIDDEN, NOT FOR API USE  !!!!!!!!!!!!!!!!!!
*/
typedef enum
{
    HTTP_STATS_CONNECT_ERRORS       = 0,
    HTTP_STATS_PKTS_SENT
} HTTP_STATS;


typedef struct
{
    pid_t           myPid;
    char            pidFile[128];
    char            fifoFile[128];
    char            statusFile[128];
    char            daemonQname[128];
    char            wviewdir[128];
    char            stationId[128];
    char            password[64];
    char            youstationId[128];
    char            youpassword[64];
    int             inMainLoop;
    int             exiting;
} WVIEW_HTTPD_WORK;

/*  !!!!!!!!!!!!!!!!!!!!  END HIDDEN SECTION  !!!!!!!!!!!!!!!!!!!!!
*/



/* ... API function prototypes
*/



#endif
