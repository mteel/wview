#ifndef INC_sshh
#define INC_sshh
/*---------------------------------------------------------------------------
 
  FILENAME:
        ssh.h
 
  PURPOSE:
        Provide the wview ssh generator definitions.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        05/28/2005      M.S. Teel       0               Original
 
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

/*  ... Library include files
*/
#include <sysdefs.h>
#include <radsemaphores.h>
#include <radbuffers.h>
#include <radqueue.h>
#include <radtimers.h>
#include <radevents.h>
#include <radtimeUtils.h>
#include <radsysutils.h>
#include <radprocess.h>

/*  ... Local include files
*/
#include <sshUtils.h>



/*  ... API definitions
*/

/*  !!!!!!!!!!!!!!!!!!  HIDDEN, NOT FOR API USE  !!!!!!!!!!!!!!!!!!
*/

typedef struct
{
    pid_t           myPid;
    char            pidFile[128];
    char            fifoFile[128];
    char            statusFile[128];
    char            daemonQname[128];
    char            wviewdir[128];
    TIMER_ID        timer;
    uint64_t        msOffset;
    SSH_DATA        sshData;
    SSH_ID          sshId;
    int             exiting;
} SSH_WORK;

/*  !!!!!!!!!!!!!!!!!!!!  END HIDDEN SECTION  !!!!!!!!!!!!!!!!!!!!!
*/



/* ... API function prototypes
*/



#endif
