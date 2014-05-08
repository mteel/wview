#ifndef INC_etherneth
#define INC_etherneth
/*---------------------------------------------------------------------------
 
  FILENAME:
        ethernet.h
 
  PURPOSE:
        Provide utilities for station communication over ethernet via a 
        transparent port capable terminal server.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        06/09/2005      M.S. Teel       0               Original
 
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
#include <sys/socket.h>
#include <string.h>

/*  ... Library include files
*/
#include <sysdefs.h>
#include <radtimeUtils.h>
#include <radsocket.h>

/*  ... Local include files
*/
#include <datadefs.h>
#include <dbsqlite.h>
#include <daemon.h>


// define our work area
typedef struct
{
    RADSOCK_ID      sockId;
    char            host[WVIEW_STRING1_SIZE];
    WVIEWD_WORK*    wviewWork;
    int             port;
} MEDIUM_ETHERNET;



/* ... function prototypes
*/

// this is the only globally visible method
extern int ethernetMediumInit (WVIEW_MEDIUM *medium, char *hostname, int port);

#endif

