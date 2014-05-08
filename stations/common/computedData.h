#ifndef INC_computedDatah
#define INC_computedDatah
/*---------------------------------------------------------------------------
 
  FILENAME:
        computedData.h
 
  PURPOSE:
        Provide utilities to compute and store HILOW values.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        08/07/2005      M.S. Teel       0               Original
 
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
#include <sensor.h>
#include <datadefs.h>
#include <dbsqlite.h>
#include "daemon.h"

/*  ... some definitions
*/
typedef struct
{
    int         currentHour;
    int         currentDay;
    int         currentMonth;
    int         currentYear;
} COMPDATA_WORK;


/* ... function prototypes
*/

// initialize the computed values from the archive records
extern int computedDataInit (WVIEWD_WORK *work);

// save sensors and exit
extern void computedDataExit (WVIEWD_WORK *work);

// update the computed values based on a new archive interval
extern int computedDataUpdate (WVIEWD_WORK *work);

// update the computed values based on a new archive record
extern int computedDataNewArchive (WVIEWD_WORK *work, ARCHIVE_PKT *newRecord);

// store a data sample in the current archive interval store
extern int computedDataStoreSample (WVIEWD_WORK *work);

// create an archive record from our sensor store
extern ARCHIVE_PKT *computedDataGenerateArchive (WVIEWD_WORK *work);

// clear the archive store
extern void computedDataClearInterval (WVIEWD_WORK *work);


#endif

