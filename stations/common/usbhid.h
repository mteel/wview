#ifndef INC_usbhidh
#define INC_usbhidh
/*---------------------------------------------------------------------------
 
  FILENAME:
        usbhid.h
 
  PURPOSE:
        Provide utilities for USB communications.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        02/15/11        M.S. Teel       0               Original
 
  NOTES:
        
 
  LICENSE:
        Copyright (c) 2011, Mark S. Teel (mark@teel.ws)
  
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
    uint16_t        vendorId;
    uint16_t        productId;
    int             blocking;
    int             debug;
} MEDIUM_USBHID;


/* ... function prototypes
*/

// this is the only globally visible method
extern int usbhidMediumInit 
(
    WVIEW_MEDIUM    *medium,
    uint16_t        vendor_id,
    uint16_t        product_id,
    int             enableDebug,
    int             blocking
);

#endif

