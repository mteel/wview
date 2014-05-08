#ifndef INC_ftputilsh
#define INC_ftputilsh
/*---------------------------------------------------------------------------
 
  FILENAME:
        ftpUtils.h
 
  PURPOSE:
        Provide the ftp utility definitions.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        01/08/04        M.S. Teel       0               Original
 
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
#include <ctype.h>
#include <curl/curl.h>

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

/*  !!!!!!!!!!!!!!!!!!  HIDDEN, NOT FOR API USE  !!!!!!!!!!!!!!!!!!
*/

#define FTP_MAX_RULES           10
#define FTP_MAX_CMND_LENGTH     512
#define OPTION_TRUE             1L
#define OPTION_FALSE            0L
#define FTP_MAX_PATH            256

typedef enum
{
    FTP_STATS_RULES_DEFINED     = 0,
    FTP_STATS_RULES_SENT,
    FTP_STATS_CONNECT_ERRORS
} FTP_STATS;


typedef enum
{
    FS_HOST         = 1,
    FS_USER         = 2,
    FS_PASS         = 3,
    FS_DIRECTORY    = 4,
    FS_BINARY       = 5,
    FS_ARGS         = 6,
    FS_RULE         = 7
} FileStates;

typedef struct
{
    NODE        node;
    char        src[128];
}*FTP_RULE_ID;


typedef struct
{
    RADLIST     rules;
    CURL*       curlHandle;
    int         rulesSent;
    char        host[96];
    char        user[64];
    char        pass[64];
    char        directory[256];
    int         IsPassive;
    int         interval;
    time_t      expiry;
}FTP_DATA, *FTP_ID;


/*  !!!!!!!!!!!!!!!!!!!!  END HIDDEN SECTION  !!!!!!!!!!!!!!!!!!!!!
*/



/* ... API function prototypes
*/
extern int ftpUtilsInit (FTP_DATA* data);

//  ... process the rules list; returns number of rules successfully sent
extern int ftpUtilsSendFiles (FTP_ID id, char *workdir);

extern void ftpUtilsExit (FTP_ID);


#endif
