#ifndef INC_dbdatabaseh
#define INC_dbdatabaseh
/*---------------------------------------------------------------------------
 
  FILENAME:
        dbdatabase.h
 
  PURPOSE:
        Define the SQL database archiving API.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        04/18/2005      M.S. Teel       0               Original
 
  NOTES:
        
 
  LICENSE:
        Copyright (c) 2004, Mark S. Teel (mark@teel.ws)
  
        This source code is released for free distribution under the terms 
        of the GNU General Public License.
  
----------------------------------------------------------------------------*/

//  ... includes
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include <sys/stat.h>

#include <sysdefs.h>
#include <radbuffers.h>
#include <raddatabase.h>

#include <datadefs.h>
#include <dbsqlite.h>


//  ... macro definitions


//  ... typedefs

typedef struct
{
    char            *name;
    UINT            type;
    int             maxLength;
} DB_ARCHIVE_DESCRIPTION;


typedef struct
{
    DATABASE_ID dbId;
    int         isEnabled;
    int         isExtended;
    char        host[512];
    char        username[64];
    char        password[64];
    char        databaseName[64];
    char        tableName[64];
    int         isMetricUnits;
} DBDATABASE_WORK;
    


//  ... API prototypes

extern int dbdatabaseInit 
(
    int             isEnabled,
    int             useExtended,
    char            *host,
    char            *username,
    char            *password,
    char            *databaseName,
    char            *tableName,
    int             isMetricUnits
);

extern int dbdatabaseOpen (void);
extern void dbdatabaseClose (void);
extern int dbdatabaseGetDateOfLastEntry (time_t* retTime);


// return values for dbdatabaseInsertRecord
#define DBDB_INSERT_OK              0
#define DBDB_INSERT_ERROR           -1
#define DBDB_INSERT_DUPLICATE       1

extern int dbdatabaseInsertRecord (ARCHIVE_PKT* newRecord);

#endif


