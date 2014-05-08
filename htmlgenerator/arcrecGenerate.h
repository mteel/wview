#ifndef INC_arcrecgenerateh
#define INC_arcrecgenerateh
/*---------------------------------------------------------------------------

  FILENAME:
        arcrecGenerate.h

  PURPOSE:
        Provide the wview archive record browser generator definitions.

  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        07/05/2005      M.S. Teel       0               Original
        04/12/2008      W. Krenn        1               metric adaptions

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
#include <sys/wait.h>
#include <string.h>
#include <time.h>
#include <math.h>

/*  ... Library include files
*/
#include <sysdefs.h>
#include <radsemaphores.h>
#include <radbuffers.h>
#include <radlist.h>

/*  ... Local include files
*/
#include <datadefs.h>
#include <services.h>
#include <dbsqlite.h>



/*  ... API definitions
*/

#define ARCREC_HEADER_FILENAME          "arcrec-header.conf"


/*  !!!!!!!!!!!!!!!!!!  HIDDEN, NOT FOR API USE  !!!!!!!!!!!!!!!!!!
*/

#define ARCREC_HEADER_MAX_LENGTH        1024

typedef struct
{
    char            htmlPath    [_MAX_PATH];
    int             daysToKeep;
    char            header[ARCREC_HEADER_MAX_LENGTH];
}__attribute__ ((packed)) ARCREC_WORK, *ARCREC_ID;



/*  !!!!!!!!!!!!!!!!!!!!  END HIDDEN SECTION  !!!!!!!!!!!!!!!!!!!!!
*/


/* ... API function prototypes
*/

extern int arcrecGenerateInit
(
    char        *htmlPath,
    int         daysToKeep,
    int         isMetric,
    int         arcInterval
);

extern int arcrecGenerate (ARCHIVE_PKT* record, int isMetric);


#endif

