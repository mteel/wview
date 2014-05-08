#ifndef INC_dbsqliteh
#define INC_dbsqliteh
/*---------------------------------------------------------------------------
 
  FILENAME:
        dbsqlite.h
 
  PURPOSE:
        Provide the wview daemon archive database definitions.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        08/31/2008      M.S. Teel       0               Original
 
  NOTES:
 
 
  LICENSE:
        Copyright (c) 2008, Mark S. Teel (mark@teel.ws)
 
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
#include <math.h>
#include <sys/stat.h>

/*  ... Library include files
*/
#include <radsqlite.h>

/*  ... Local include files
*/
#include <sysdefs.h>
#include <datadefs.h>
#include <windAverage.h>


#define FIELD_IS_NULL(x)        (((x->type & SQLITE_FIELD_VALUE_IS_NULL) == 0) ? FALSE : TRUE)


/*  ... API definitions
*/

/*  !!!!!!!!!!!!!!!!!!  HIDDEN, NOT FOR API USE  !!!!!!!!!!!!!!!!!!
*/



/*  !!!!!!!!!!!!!!!!!!!!  END HIDDEN SECTION  !!!!!!!!!!!!!!!!!!!!!
*/


/*  ... API methods
*/

// ---------------------- History Computation ---------------------

/*  ... calculate averages over a given period of time
    ... (given in arcInterval samples);
    ... this will zero out the HISTORY_DATA store before beginning
    ... the processing;
    ... returns number of minutes found or ERROR
*/
extern int dbsqliteArchiveGetAverages
(
    int             isMetricUnits,
    int             arcInterval,
    HISTORY_DATA    *store,
    time_t          startTime,
    int             numSamples
);


// --------------------- ARCHIVE Day History ----------------------

extern int dbsqliteWriteDailyArchiveReport
(
    char            *filename,
    time_t          timeval,
    int             isMetric,
    int             arcInterval,
    void            (*writeHeader) (FILE *file)
);

extern int dbsqliteUpdateDailyArchiveReport
(
    char            *file,
    ARCHIVE_PKT     *data,
    void            (*writeHeader) (FILE *file),
    int             isMetric
);


// ----------------------- Archive Database -----------------------

// Note: wview processes now open the database session at init and do not close
//       until the process is exiting...

// Initialize the database interface (returns OK or ERROR):
extern int dbsqliteArchiveInit(void);

// Clean up the database interface:
extern void dbsqliteArchiveExit(void);

// set a PRAGMA to modify the operation of the SQLite library:
// Returns: OK or ERROR
extern int dbsqliteArchivePragmaSet(char* pragma, char* setting);

//  ... Reset the default archive database location (used by conversion utils);
extern void dbsqliteArchiveSetPath (char* newPath);
extern char* dbsqliteArchiveGetPath (void);

//  ... append an archive record to the archive database;
//  ... returns OK or ERROR
extern int dbsqliteArchiveStoreRecord (ARCHIVE_PKT* record);

//  ... search the archive database for the most recent archive record date;
//  ... returns time or ERROR if no archives found
extern time_t dbsqliteArchiveGetNewestTime (ARCHIVE_PKT* newestRecord);

//  ... search the archive database for the next archive record after 'dateTime';
//  ... returns new time or ERROR if no archive found
extern time_t dbsqliteArchiveGetNextRecord (time_t  dateTime, ARCHIVE_PKT* recordStore);

//  ... search the archive database for the archive record with timestamp 'dateTime';
//  ... returns OK or ERROR if no archive record found

extern int dbsqliteArchiveGetRecord (time_t dateTime, ARCHIVE_PKT* recordStore);

//  ... search the archive database for the first archive record in the given 
//  ... timestamp range;
//  ... returns OK or ERROR if no archive record found

extern int dbsqliteArchiveGetFirstRecord
(
    time_t          startTime, 
    time_t          stopTime, 
    ARCHIVE_PKT*    recordStore
);

//  ... execute the given function for each record in the given timestamp range;
//  ... 'selectClause' if not NULL can contain specific columns of interest
//  ... (dateTime,usUnits,interval are always included);
//  ... returns the number of records found or ERROR

extern int dbsqliteArchiveExecutePerRecord
(
    void            (*function)(ARCHIVE_PKT* rec, void* data),
    void*           userData,
    time_t          startTime, 
    time_t          stopTime,
    char*           selectClause
);

//  ... Retrieve the number of records matching the given "where" clause;
//  ... Returns the count or ERROR

extern int dbsqliteArchiveGetCount(char* whereClause);



// ------------------------ HILOW Database ------------------------
// Initialize the HILOW database:
// IF HILOW tables are to be synced to the archive database, 'update'
// should be TRUE;
// returns OK or ERROR
extern int dbsqliteHiLowInit(int update);

extern void dbsqliteHiLowExit(void);

// set a PRAGMA to modify the operation of the SQLite library:
// Returns: OK or ERROR
extern int dbsqliteHiLowPragmaSet(char* pragma, char* setting);


//  ... Update sensors for the given hour and time frame:
//  ... Returns number of records processed or ERROR
extern int dbsqliteHiLowGetHour
(
    time_t                  hour,
    SENSOR_STORE*           sensors,
    SENSOR_TIMEFRAMES       timeFrame
);

//  ... Update sensors for the given day and time frame:
//  ... Returns number of records processed or ERROR
extern int dbsqliteHiLowGetDay
(
    time_t                  day,
    SENSOR_STORE*           sensors,
    SENSOR_TIMEFRAMES       timeFrame
);

//  ... Update sensors for the given month and time frame:
//  ... Returns number of records processed or ERROR
extern int dbsqliteHiLowGetMonth
(
    time_t                  month,
    SENSOR_STORE*           sensors,
    SENSOR_TIMEFRAMES       timeFrame,
    int                     yearRainFlag
);

//  ... Update database with a timestamped LOOP sample:
//  ... No duplicate checking is done;
//  ... Returns OK or ERROR
extern int dbsqliteHiLowStoreSample
(
    time_t                  timestamp,
    LOOP_PKT*               sample
);

//  ... Update database with an archive record in lieu of LOOP samples:
//  ... No duplicate checking is done;
//  ... Returns OK or ERROR
extern int dbsqliteHiLowStoreArchive
(
    ARCHIVE_PKT*            record
);

//  ... Update database with an archive record, ignoring cumulative values:
//  ... No duplicate checking is done;
//  ... Returns OK or ERROR
extern int dbsqliteHiLowUpdateArchive (ARCHIVE_PKT* record);

//  ... Retrieve the last HILOW hour in the database;
//  ... Returns time or ERROR
extern time_t dbsqliteHiLowGetLastUpdate(void);



#ifdef BUILD_HTMLGEND
// ----------------------- History Database -----------------------

// Initialize the day history table:
extern void dbsqliteHistoryInit (void);

// Insert a day record in the history table:
// Returns OK or ERROR
extern int dbsqliteHistoryInsertDay(HISTORY_DATA* data);

// Retrieve a day from the history table:
// Returns OK or ERROR if not found
extern int dbsqliteHistoryGetDay(time_t date, HISTORY_DATA* store);


// ------------------------- NOAA Database ------------------------

extern int dbsqliteNOAAInit(void);
extern void dbsqliteNOAAExit(void);

/*  ... dbsqliteNOAAGetDay: summarize day records for NOAA reports;
    ... this will zero out the NOAA_DAY_REC store before beginning;
    ... returns OK or ERROR if day not found in archives
*/
extern int dbsqliteNOAAGetDay(NOAA_DAY_REC *store, time_t day);

/*  ... dbsqliteNOAAComputeNorms: compute monthly and yearly norms
*/
extern int dbsqliteNOAAComputeNorms
(
    float   *temps, 
    float   *rains, 
    float   *yearTemp, 
    float   *yearRain
);

/*  ... dbsqliteNOAAUpdate: bring the NOAA database up-to date:
*/
extern void dbsqliteNOAAUpdate(void);

#endif

#endif

