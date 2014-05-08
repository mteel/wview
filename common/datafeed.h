#ifndef INC_datafeedh
#define INC_datafeedh
/*---------------------------------------------------------------------------
 
  FILENAME:
        datafeed.h
 
  PURPOSE:
        Define the datafeed API.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        12/16/2009      M.S. Teel       0               Original
 
  NOTES:
        
 
  LICENSE:
        Copyright (c) 2009, Mark S. Teel (mark@teel.ws)
  
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
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <radtimeUtils.h>
#include <radsysutils.h>
#include <radsocket.h>

#define WVIEW_DATA_ONLY             TRUE
#include <datadefs.h>


//  ... macro definitions

// The listen port for wview - clients connect here:
#define WV_DATAFEED_PORT            11011


//  ... typedefs

// Define the start frame sequences for loop and archive packets:
#define DF_START_FRAME_LENGTH       8
#define DF_LOOP_PKT_TYPE            1
#define DF_ARCHIVE_PKT_TYPE         2
#define DF_RQST_ARCHIVE_PKT_TYPE    3

// Define some times:
#define DF_WAIT_FIRST               500
#define DF_WAIT_MORE                250


#ifdef DATAFEED_INSTANTIATE
const uint16_t DF_LOOP_START_FRAME[4] = 
{
    0xF388, 
    0xC6A2, 
    0xDADA, 
    0x0001
};
const uint16_t DF_ARCHIVE_START_FRAME[4] = 
{
    0xF388, 
    0xC6A2, 
    0xDADA, 
    0x0002
};

// This one is sent by the client to request the next archive record after the
// dateTime submitted; allows the client to synchronize archive data:
const uint16_t DF_RQST_ARCHIVE_START_FRAME[4] = 
{
    0xF388, 
    0xC6A2, 
    0xDADA, 
    0x0003
};
#else
extern const uint16_t DF_LOOP_START_FRAME[4]; 
extern const uint16_t DF_ARCHIVE_START_FRAME[4]; 
extern const uint16_t DF_RQST_ARCHIVE_START_FRAME[4];
#endif


//  ... API prototypes

// Frame sync utility:
// Returns DF_LOOP_PKT_TYPE, DF_ARCHIVE_PKT_TYPE or DF_RQST_ARCHIVE_PKT_TYPE 
//   if a valid frame header of one of those types is received, FALSE if not a 
//   valid frame header and ERROR if there is a socket error:
extern int datafeedSyncStartOfFrame(RADSOCK_ID socket);


// LOOP_PKT byteorder and fixed point conversions:
extern int datafeedConvertLOOP_HTON(LOOP_PKT* dest, LOOP_PKT* src);
extern int datafeedConvertLOOP_NTOH(LOOP_PKT* dest, LOOP_PKT* src);

// ARCHIVE_PKT byteorder and fixed point conversions:
extern int datafeedConvertArchive_HTON(ARCHIVE_PKT* dest, ARCHIVE_PKT* src);
extern int datafeedConvertArchive_NTOH(ARCHIVE_PKT* dest, ARCHIVE_PKT* src);


#endif

