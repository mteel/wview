#ifndef INC_imagesh
#define INC_imagesh
/*---------------------------------------------------------------------------
 
  FILENAME:
        images.h
 
  PURPOSE:
        Provide the built-in wview image generator definitions.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        01/20/04        M.S. Teel       0               Original
 
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
#include <signal.h>

/*  ... Library include files
*/
#include <sysdefs.h>
#include <radsemaphores.h>
#include <radbuffers.h>
#include <radlist.h>

/*  ... Local include files
*/
#include <sensor.h>
#include <dbsqlite.h>
#include <datadefs.h>
#include <services.h>
#include <htmlMgr.h>



/*  ... API definitions
*/


/*  !!!!!!!!!!!!!!!!!!  HIDDEN, NOT FOR API USE  !!!!!!!!!!!!!!!!!!
*/

/*  !!!!!!!!!!!!!!!!!!!!  END HIDDEN SECTION  !!!!!!!!!!!!!!!!!!!!!
*/
//  ... this is the generator jump table
//  ... indexes from images.conf are used to index into this table
//  ... Thus, order is VERY important
extern int (*images_generators[]) (HTML_IMG *img);


/* ... function prototypes
*/

#endif
