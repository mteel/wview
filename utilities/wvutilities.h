#ifndef INC_wvutilitiesh
#define INC_wvutilitiesh
/*---------------------------------------------------------------------------
 
  FILENAME:
        wvutilities.h
 
  PURPOSE:
        Provide the wview utility program common definitions.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        02/26/2006      M.S. Teel       0               Original
 
  NOTES:
        
 
  LICENSE:
        Copyright (c) 2006, Mark S. Teel (mark@teel.ws)
  
        This source code is released for free distribution under the terms 
        of the GNU General Public License.
  
----------------------------------------------------------------------------*/

// System include files
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

// Library include files

// Local include files
#include <sysdefs.h>
#include <datadefs.h>
#include <dbsqlite.h>
#include <dbfiles.h>
#include <sensor.h>


// data defs


// API function prototypes

// Convert the endianness of WLK files as given by 'doLE2BE'
extern int wvuConvertWLKFiles (char *srcDir, char *destDir, int doLE2BE);

#endif

