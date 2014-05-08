/*---------------------------------------------------------------------------

  FILENAME:
        virtualProtocol.c

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
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <termios.h>
#include <time.h>
#include <errno.h>
#include <math.h>


/*  ... Library include files
*/
#include <radmsgLog.h>
#include <radsysutils.h>
#include <radtimeUtils.h>

/*  ... Local include files
*/
#include <services.h>
#include <daemon.h>
#include <station.h>
#include <virtualProtocol.h>

/*  ... global memory declarations
*/
extern void     (*ArchiveIndicator)(ARCHIVE_PKT* newRecord);

/*  ... local memory
*/

static VIRTUAL_WORK         virtualWork;


/*  ... methods
*/

static int ReadDataFrame(WVIEWD_WORK *work)
{
    int             retVal;
    LOOP_PKT        loopData;
    LOOP_PKT        hostLoopData;
    ARCHIVE_PKT     archiveRecord;
    ARCHIVE_PKT     hostRecord;
    RADSOCK_ID      sockId = (*work->medium.getsocket)(&work->medium);

    /* try to find the start frame (this blocks if the socket is empty) */
    retVal = datafeedSyncStartOfFrame(sockId);
    switch (retVal)
    {
        case ERROR:
            return ERROR;

        case ERROR_ABORT:
            return ERROR_ABORT;

        case FALSE:
            return FALSE;

        case DF_LOOP_PKT_TYPE:
            /* OK, we have a loop update coming (this may block) */
            if ((*work->medium.read)(&work->medium, (void *)&loopData, sizeof (loopData), 1000) 
                != sizeof (loopData))
            {
                return FALSE;
            }
            
            // Convert from network byte order:
            datafeedConvertLOOP_NTOH(&hostLoopData, &loopData);

            // Store it:
            virtualWork.data.loopData = hostLoopData;
            return DF_LOOP_PKT_TYPE;

        case DF_ARCHIVE_PKT_TYPE:
            /* OK, we have an archive coming (this may block) */
            if ((*work->medium.read)(&work->medium, (void *)&archiveRecord, sizeof(archiveRecord), 1000) 
                != sizeof (archiveRecord))
            {
                return FALSE;
            }

            // Convert from network byte order:
            datafeedConvertArchive_NTOH(&hostRecord, &archiveRecord);

            virtualWork.data.archiveData = hostRecord;

            if (hostRecord.dateTime != 0)
            {
                time_t archtime;
                archtime = hostRecord.dateTime;
                radMsgLog (PRI_STATUS, "VIRTUAL: RX archive from remote station: %s", ctime(&archtime));
            }

            if (virtualWork.IsArchiveNeeded)
            {
                // Indicate it right now:
                virtualWork.IsArchiveReceived = FALSE;
                virtualWork.IsArchiveNeeded = FALSE;
                (*ArchiveIndicator)(&virtualWork.data.archiveData);

                // Add all but cumulative:
                dbsqliteHiLowUpdateArchive(&virtualWork.data.archiveData);
            }
            else
            {
                // Indicate a new record is ready:
                virtualWork.IsArchiveReceived = TRUE;
            }

            return DF_ARCHIVE_PKT_TYPE;
    }

    return FALSE;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////  A P I  ///////////////////////////////////
int virtualProtocolInit (WVIEWD_WORK *work)
{
    time_t          nowtime = time(NULL);
    uint32_t        dateTime;
    int             retVal, retries = 0;

    memset (&virtualWork, 0, sizeof(virtualWork));

    // Let the socket settle down:
    radUtilsSleep (100);

    // We need to sync up the archive records:
    while (work->archiveDateTime < nowtime)
    {
        // Send the request:
        // write the frame start:
        if ((*work->medium.write)(&work->medium, 
                                  (void *)DF_RQST_ARCHIVE_START_FRAME, 
                                  DF_START_FRAME_LENGTH)
            != DF_START_FRAME_LENGTH)
        {
            radMsgLog (PRI_HIGH, "VIRTUAL: write sync error!");
            return ERROR;
        }

        // Send the dateTime:
        // convert to network byte order:
        dateTime = htonl(work->archiveDateTime);
        if ((*work->medium.write)(&work->medium, &dateTime, sizeof(dateTime))
            != sizeof(dateTime))
        {
            radMsgLog (PRI_HIGH, "VIRTUAL: write data error!");
            return ERROR;
        }

        // OK, block waiting on it:
        retVal = 0;
        retries = 0;
        while (retVal != DF_ARCHIVE_PKT_TYPE && retries < VIRTUAL_MAX_RETRIES)
        {
            retVal = ReadDataFrame(work);
            if (retVal == ERROR || retVal == ERROR_ABORT)
            {
                radMsgLog (PRI_HIGH, "VIRTUAL: socket error - aborting!");
                return ERROR;
            }
            retries ++;
        }
        if (retries == VIRTUAL_MAX_RETRIES)
        {
            // No Taco Bueno:
            radMsgLog (PRI_HIGH, "VIRTUAL: archive read failed on %d tries", retries);
            return ERROR;
        }

        // Check the time stamp:
        if (virtualWork.data.archiveData.dateTime < work->archiveDateTime)
        {
            // No new records:
            radMsgLog (PRI_STATUS, "VIRTUAL: all archive records received");
            work->archiveDateTime = nowtime;
        }
        else
        {
            // Good stuff:
            (*ArchiveIndicator)(&virtualWork.data.archiveData);

            // If not running yet, add to HILOW database:
            dbsqliteHiLowStoreArchive(&virtualWork.data.archiveData);

            work->archiveDateTime = virtualWork.data.archiveData.dateTime;
        }
    }

    // Reset the archive handling flags:
    virtualWork.IsArchiveReceived = FALSE;
    virtualWork.IsArchiveNeeded = FALSE;

    // Now we need to wait for initial readings:
    retVal = 0;
    radMsgLog (PRI_STATUS, "VIRTUAL: waiting for initial readings...");
    while (retVal != DF_LOOP_PKT_TYPE)
    {
        retVal = ReadDataFrame(work);
        if (retVal == ERROR || retVal == ERROR_ABORT)
        {
            radMsgLog (PRI_HIGH, "VIRTUAL: socket error - aborting!");
            return ERROR;
        }
        else if (retVal == DF_LOOP_PKT_TYPE)
        {
            radMsgLog (PRI_STATUS, "VIRTUAL: RX LOOP");
        }
    }

    return OK;
}

void virtualProtocolExit (WVIEWD_WORK *work)
{
    // nothing to clean up...
    return;
}

int virtualProtocolDataIndicate(WVIEWD_WORK *work)
{
    return ReadDataFrame(work);
}

int virtualProtocolGetReadings(WVIEWD_WORK *work, LOOP_PKT *store)
{
    // Use the last LOOP data we received:
    *store = virtualWork.data.loopData;

    return OK;
}

int virtualProtocolGetArchive(WVIEWD_WORK *work)
{
    if (! virtualWork.IsArchiveReceived)
    {
        virtualWork.IsArchiveNeeded = TRUE;
        return FALSE;
    }

    // Indicate it and reset flags:
    virtualWork.IsArchiveReceived = FALSE;
    virtualWork.IsArchiveNeeded = FALSE;
    (*ArchiveIndicator)(&virtualWork.data.archiveData);

    // Add all but cumulative:
    dbsqliteHiLowUpdateArchive(&virtualWork.data.archiveData);

    return TRUE;
}

