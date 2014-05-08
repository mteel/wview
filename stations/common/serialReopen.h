#ifndef INC_serialReopenh
#define INC_serialReopenh
/*---------------------------------------------------------------------------
 
  FILENAME:
        serialReopen.h
 
  PURPOSE:
        Provide utilities for serial communications. Port is not left open
        but instead is opened for each read/write access.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        06/13/10        M.S. Teel       0               Original
 
  NOTES:
        
 
  LICENSE:
        Copyright (c) 2010, Mark S. Teel (mark@teel.ws)
  
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
} MEDIUM_SERIAL_REOPEN;


/* ... function prototypes
*/

// this is the only globally visible method
extern int serialReopenMediumInit (WVIEW_MEDIUM *medium, void (*portInit)(int fd), int openFlags);

#endif

