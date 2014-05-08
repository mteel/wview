#ifndef INC_virtualProtocolh
#define INC_virtualProtocolh
/*---------------------------------------------------------------------------
 
  FILENAME:
        virtualProtocol.h
 
  PURPOSE:
        Provide protocol utilities for virtual station communication.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        12/17/2009      M.S. Teel       0               Original
 
 
  NOTES:
 
 
  LICENSE:
        Copyright (c) 2009, Mark S. Teel (mark@teel.ws)
 
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
#include <datafeed.h>
#include <datadefs.h>
#include <dbsqlite.h>
#include <daemon.h>
#include <parser.h>
#include <ethernet.h>


#define VIRTUAL_RESPONSE_TIMEOUT        1000
#define VIRTUAL_MAX_RETRIES             5

typedef struct
{
    LOOP_PKT        loopData;
    ARCHIVE_PKT     archiveData;
} 
VIRTUAL_DATA;

// define the work area
typedef struct
{
    VIRTUAL_DATA    data;
    int             IsArchiveReceived;
    int             IsArchiveNeeded;
}
VIRTUAL_WORK;


// function prototypes

// call once during initialization
extern int virtualProtocolInit(WVIEWD_WORK *work);

// do cleanup
extern void virtualProtocolExit(WVIEWD_WORK *work);

// initiate a synchronous sensor collection:
extern int virtualProtocolGetReadings(WVIEWD_WORK *work, LOOP_PKT *store);

// indicate data is available:
extern int virtualProtocolDataIndicate(WVIEWD_WORK *work);

// Retrieve archive record or set "need it" flag:
extern int virtualProtocolGetArchive(WVIEWD_WORK *work);


#endif

