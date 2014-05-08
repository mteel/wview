/*---------------------------------------------------------------------------
 
  FILENAME:
        serialReopen.c
 
  PURPOSE:
        Provide utilities for serial communications. Port is not left open
        but instead is opened for each read/write access.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        06/13/10        M.S. Teel       0               Original
 
  NOTES:
        wview medium-specific routines to be supplied:
        
        xxxMediumInit    - sets up function pointers and work area
        xxxInit          - opens the interface and configures it
        xxxRead          - blocking read until specified bytes are read
        xxxWrite         - write on medium
        xxxExit          - cleanup and close interface
        
        See daemon.h for details of the WVIEW_MEDIUM structure.
 
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
#include <sys/file.h>
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

/*  ... Local include files
*/
#include <services.h>
#include <serialReopen.h>
#include <daemon.h>
#include <station.h>

/*  ... global memory declarations
*/

/*  ... local memory
*/
static MEDIUM_SERIAL_REOPEN     mediumSerialReopen;


static int openSerialPort(WVIEW_MEDIUM *med)
{
    MEDIUM_SERIAL_REOPEN   *serialWork = (MEDIUM_SERIAL_REOPEN *)med->workData;

    //  ... open our serial channel
    med->fd = open (med->device, serialWork->openFlags);
    if (med->fd == -1)
    {
        radMsgLog (PRI_CATASTROPHIC, "Serial device %s failed to open: %s",
                   med->device, strerror(errno));
        return ERROR;
    }

    if (flock (med->fd, LOCK_EX) < 0)
    {
        if (errno == EOPNOTSUPP)
        {
            radMsgLog(PRI_MEDIUM, "serial device locking not supported by OS for %s",
                      med->device);
        }
        else
        {
            radMsgLog (PRI_CATASTROPHIC, "Serial device %s locked by other program!",
                       med->device);
            return ERROR;
        }
    }

    // configure it
    serialWork->portInit (med->fd);

    tcflush (med->fd, TCIFLUSH);
    tcflush (med->fd, TCOFLUSH);

    radUtilsSleep (1);
    return OK;

}

static int closeSerialPort(WVIEW_MEDIUM *med)
{
    if (med->fd != -1)
        close(med->fd);
    med->fd = -1;
    return OK;
}


//////////////////////////////////////////////////////////////////////////////
//  ... medium callback functions
//////////////////////////////////////////////////////////////////////////////

static int serialReopenInit (WVIEW_MEDIUM *med, char *deviceName)
{
    strncpy(med->device, deviceName, WVIEW_MAX_PATH);
    med->fd = -1;
    return OK;
}

static void serialReopenExit (WVIEW_MEDIUM *med)
{
    tcflush (med->fd, TCIFLUSH);
    tcflush (med->fd, TCOFLUSH);
    closeSerialPort(med);
    return;
}

static int serialReopenWrite (WVIEW_MEDIUM *med, void *buffer, int length)
{
    int     retVal;
    int     wrerrno = 0;

    retVal = write (med->fd, buffer, length);
    if (retVal != length)
    {
        if (retVal == -1)
        {
            wrerrno = errno;
        }

        return retVal;
    }

    // ... now wait for the TX buffer to empty out (this blocks)
    tcdrain (med->fd);

    return retVal;
}

static int serialReopenReadExact (WVIEW_MEDIUM *med, void *bfr, int len, int msTimeout)
{
    int         rval, cumTime = 0, index = 0;
    uint64_t    readTime;
    uint8_t     *ptr = (uint8_t *)bfr;

    while (index < len && cumTime < msTimeout)
    {
        readTime = radTimeGetMSSinceEpoch ();
        rval = read (med->fd, &ptr[index], len - index);
        if (rval < 0)
        {
            if (errno != EINTR && errno != EAGAIN)
            {
                return ERROR;
            }
        }
        else
        {
            index += rval;
        }

        readTime = radTimeGetMSSinceEpoch () - readTime;
        cumTime += (int)readTime;
        if (index < len && cumTime < msTimeout)
        {
            readTime = radTimeGetMSSinceEpoch ();
            radUtilsSleep (9);
            readTime = radTimeGetMSSinceEpoch () - readTime;
            cumTime += (int)readTime;
        }
    }

    return ((index == len) ? len : ERROR);
}

static void serialReopenFlush (WVIEW_MEDIUM *med, int queue)
{
    if (queue == WV_QUEUE_INPUT)
    {
        tcflush (med->fd, TCIFLUSH);
    }
    else if (queue == WV_QUEUE_OUTPUT)
    {
        tcflush (med->fd, TCOFLUSH);
    }

    return;
}

static void serialReopenDrain (WVIEW_MEDIUM *med)
{
    tcdrain (med->fd);
    return;
}

static RADSOCK_ID serialReopenGetSocket(WVIEW_MEDIUM *med)
{
    return NULL;
}

static void serialReopenSessionStart(struct _wview_medium *medium)
{
    return;
}

static void serialReopenSessionEnd(struct _wview_medium *medium)
{
    return;
}


// ... ----- API methods -----

int serialReopenMediumInit (WVIEW_MEDIUM *medium, void (*portInit)(int fd), int openFlags)
{
    MEDIUM_SERIAL_REOPEN       *work = &mediumSerialReopen;

    memset (medium, 0, sizeof (*medium));
    memset (work, 0, sizeof (*work));

    work->portInit  = portInit;
    work->openFlags = openFlags;

    // set our workData pointer for later use
    medium->workData = (void *)work;

    medium->init            = serialReopenInit;
    medium->exit            = serialReopenExit;
    medium->sessionStart    = serialReopenSessionStart;
    medium->sessionEnd      = serialReopenSessionEnd;
    medium->read            = serialReopenReadExact;
    medium->write           = serialReopenWrite;
    medium->flush           = serialReopenFlush;
    medium->txdrain         = serialReopenDrain;
    medium->getsocket       = serialReopenGetSocket;

    return OK;
}

