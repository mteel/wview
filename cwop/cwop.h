#ifndef INC_cwoph
#define INC_cwoph
/*---------------------------------------------------------------------------
 
  FILENAME:
        cwop.h
 
  PURPOSE:
        Provide the wview CWOP process definitions.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        07/12/2005      M.S. Teel       0               Original
 
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
#include <radmsgRouter.h>

/*  ... Local include files
*/
#include <services.h>
#include <sysdefs.h>
#include <datadefs.h>
#include <status.h>
#include <wvconfig.h>


#define CWOP_MINUTE_INTERVAL            60000

/*  ... API definitions
*/

/*  !!!!!!!!!!!!!!!!!!  HIDDEN, NOT FOR API USE  !!!!!!!!!!!!!!!!!!
*/

typedef enum
{
    CWOP_STATS_CONNECT_ERRORS       = 0,
    CWOP_STATS_PKTS_SENT
} CWOP_STATS;


typedef struct
{
    pid_t                       myPid;
    TIMER_ID                    timer;
    char                        pidFile[128];
    char                        fifoFile[128];
    char                        statusFile[128];
    char                        daemonQname[128];
    char                        wviewdir[128];
    WVIEW_MSG_ARCHIVE_NOTIFY    ArchiveMsg;
    char                        callSign[64];
    int                         callSignOffset;
    int                         reportInterval;
    int                         rxFirstArchive;
    int                         rxArchive;
    char                        server1[128];
    int                         portNo1;
    char                        server2[128];
    int                         portNo2;
    char                        server3[128];
    int                         portNo3;
    char                        latitude[16];
    char                        longitude[16];
    int                         logWXPackets;
    int                         inMainLoop;
    int                         exiting;
} WVIEW_CWOP_WORK;

/*  !!!!!!!!!!!!!!!!!!!!  END HIDDEN SECTION  !!!!!!!!!!!!!!!!!!!!!
*/



/* ... API function prototypes
*/



#endif
