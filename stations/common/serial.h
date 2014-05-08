#ifndef INC_serialh
#define INC_serialh
/*---------------------------------------------------------------------------
 
  FILENAME:
        serial.h
 
  PURPOSE:
        Provide utilities for serial communications.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        08/15/03        M.S. Teel       0               Original
 
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
#include <string.h>

/*  ... Library include files
*/
#include <sysdefs.h>
#include <radtimeUtils.h>

/*  ... Local include files
*/
#include <datadefs.h>
#include <dbsqlite.h>
#include <daemon.h>


// define our work area
typedef struct
{
    void        (*portInit) (int fd);
    int         openFlags;
    char        device[WVIEW_STRING2_SIZE];
} MEDIUM_SERIAL;


/* ... function prototypes
*/

// this is the only globally visible method
extern int serialMediumInit (WVIEW_MEDIUM *medium, void (*portInit)(int fd), int openFlags);

#endif

