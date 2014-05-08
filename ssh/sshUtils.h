#ifndef INC_sshutilsh
#define INC_sshutilsh
/*---------------------------------------------------------------------------
 
  FILENAME:
        sshUtils.h
 
  PURPOSE:
        Provide the ssh utility definitions.
 
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
#include <radtimers.h>
#include <radlist.h>
#include <radtimeUtils.h>
#include <radsysutils.h>

/*  ... Local include files
*/
#include <datadefs.h>
#include <wvconfig.h>
#include <status.h>



/*  ... API definitions
*/
#define SSH_MAX_RULES           5
#define SSH_PORT_DEFAULT        22


/*  !!!!!!!!!!!!!!!!!!  HIDDEN, NOT FOR API USE  !!!!!!!!!!!!!!!!!!
*/
typedef enum
{
    SSH_STATS_RULES_DEFINED     = 0,
    SSH_STATS_RULES_SENT
} SSH_STATS;


typedef struct
{
    NODE        node;
    char        host[128];
    int         sshPort;
    char        sshUser[64];
    char        src[_MAX_PATH];
    char        dest[_MAX_PATH];
    int         interval;
    int         currentCount;
    int         timeout;
} *SSH_RULE_ID;


typedef struct
{
    char        rsyncPath[_MAX_PATH];
    RADLIST     rules;
    int         rulesSent;
}SSH_DATA, *SSH_ID;


/*  !!!!!!!!!!!!!!!!!!!!  END HIDDEN SECTION  !!!!!!!!!!!!!!!!!!!!!
*/



/* ... API function prototypes
*/
extern int sshUtilsInit (SSH_DATA* data);

//  ... process the rules list; returns number of rules successfully sent
extern int sshUtilsSendFiles (SSH_ID id, char *workdir);

extern void sshUtilsExit (SSH_ID);


#endif
