/*---------------------------------------------------------------------------
 
  FILENAME:
        ethernet.c
 
  PURPOSE:
        Provide utilities for station communication over ethernet via a 
        transparent port capable terminal server.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        06/09/2005      M.S. Teel       0               Original
 
  NOTES:
        wview medium-specific routines to be supplied:
        
        xxxMediumInit    - sets up function pointers and work area
        xxxInit          - opens the interface and configures it
        xxxRead          - blocking read until specified bytes are read
        xxxWrite         - write on medium
        xxxExit          - cleanup and close interface
        xxxFlush         - flush the input buffer from the VP
        xxxDrain         - wait/block until the transmit buffer is empty
        
        See daemon.h for details of the WVIEW_MEDIUM structure.

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
#include <ethernet.h>
#include <daemon.h>
#include <station.h>

/*  ... global memory declarations
*/

/*  ... local memory
*/

static int IsBlocking(RADSOCK_ID id)
{
    int             flags;

    if ((flags = fcntl(id->sockfd, F_GETFL, 0)) < 0)
    {
        return FALSE;
    }
    
    if ((flags & O_NONBLOCK) != 0)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}



//////////////////////////////////////////////////////////////////////////////
//  ... medium callback functions
//////////////////////////////////////////////////////////////////////////////

static int ethernetInit (WVIEW_MEDIUM *med, char *deviceName)
{
    MEDIUM_ETHERNET *eth = (MEDIUM_ETHERNET *)med->workData;
    
    radMsgLog (PRI_STATUS, "ethernetInit: connecting to station @ %s:%d ...",
               eth->host, eth->port);
    eth->sockId = radSocketClientCreate (eth->host, eth->port);
    if (eth->sockId == NULL)
    {
        radMsgLog (PRI_HIGH, "ethernetInit: radSocketClientCreate failed for %s:%d",
                   eth->host, eth->port);
        return ERROR;
    }
    
    // make the socket non-blocking
    if (radSocketSetBlocking (eth->sockId, FALSE) == ERROR)
    {
        radMsgLog (PRI_HIGH, "ethernetInit: radSocketSetBlocking failed for %s:%d",
                   eth->host, eth->port);
        radSocketDestroy (eth->sockId);
        eth->sockId = NULL;
        return ERROR;
    }
        
    // set this so the upper layer can do selects on the socket descriptor
    med->fd = radSocketGetDescriptor (eth->sockId);

    return OK;
}

static void ethernetExit (WVIEW_MEDIUM *med)
{
    MEDIUM_ETHERNET *eth = (MEDIUM_ETHERNET *)med->workData;

    if (eth->sockId)
        radSocketDestroy (eth->sockId);
    
    return;
}

static int ethernetRestart (WVIEW_MEDIUM *med)
{
    ethernetExit(med);
    radMsgLog (PRI_HIGH, "ethernetRestart: attempting socket restart");
    while ((!wviewdIsExiting()) && (ethernetInit(med, NULL) == ERROR))
    {
        radMsgLog (PRI_HIGH, "ethernetRestart: socket restart failed");
        radUtilsSleep(5000);
        radMsgLog (PRI_HIGH, "ethernetRestart: retrying socket restart");
    }
    if (!wviewdIsExiting())
    {
        radMsgLog (PRI_HIGH, "ethernetRestart: socket recovered");
    }
    return OK;
}

static int ethernetWrite (WVIEW_MEDIUM *med, void *buffer, int length)
{
    MEDIUM_ETHERNET *eth = (MEDIUM_ETHERNET *)med->workData;
    int             retVal;

    retVal = radSocketWriteExact (eth->sockId, buffer, length);
    if (retVal <= 0)
    {
        // Socket error; try to re-establish:
        radMsgLog (PRI_HIGH, "ethernetWrite: write error!");
        ethernetRestart(med);
        return ERROR;
    }
    else if (retVal != length)
    {
        radMsgLog (PRI_HIGH, "ethernetWrite: failed: errno %d: %s",
                   errno, strerror(errno));
        return ERROR;
    }
    
    return length;
}

static int ethernetReadExact (WVIEW_MEDIUM *med, void *bfr, int len, int msTimeout)
{
    MEDIUM_ETHERNET *eth = (MEDIUM_ETHERNET *)med->workData;
    int             rval, cumTime = 0, index = 0, doRecover = FALSE;
    uint64_t        readTime;
    uint8_t         *ptr = (uint8_t *)bfr;
    fd_set          rfds, wfds;
    struct timeval  waitTime;          

    while (index < len && cumTime < msTimeout)
    {
        readTime = radTimeGetMSSinceEpoch ();
        doRecover = FALSE;
        if (IsBlocking(eth->sockId))
        {
            rval = radSocketReadExact (eth->sockId, &ptr[index], len - index);
            if ((rval == ERROR) || (rval == 0))
            {
                // Far end close/error; try to re-establish the socket:
                doRecover = TRUE;
            }
        }
        else
        {
            waitTime.tv_sec = (msTimeout-cumTime)/1000;
            waitTime.tv_usec = ((msTimeout-cumTime)%1000)*1000;
            FD_ZERO(&rfds);
            FD_ZERO(&wfds);
            FD_SET(radSocketGetDescriptor(eth->sockId), &rfds);
            rval = select(radSocketGetDescriptor(eth->sockId) + 1,
                          &rfds,
                          &wfds,
                          (fd_set*)0,
                          &waitTime);
            if (rval > 0)
            {
                if (FD_ISSET(radSocketGetDescriptor(eth->sockId), &rfds))
                {
                    // Socket ready for reading:
                    rval = radSocketReadExact (eth->sockId, &ptr[index], len - index);
                    if (rval < 0)
                    {
                        if (errno == EINTR || errno == EAGAIN)
                        {
                            // Try again:
                            rval = 0;
                        }
                        else
                        {
                            radMsgLog (PRI_HIGH, "ethernetReadExact: read error: %d: %s",
                                       errno, strerror(errno));
                            doRecover = TRUE;
                        }
                    }
                    else if (rval == 0)
                    {
                        // Far end close/error; try to re-establish the socket:
                        radMsgLog (PRI_HIGH, "ethernetReadExact: far-end appears to have closed.");
                        doRecover = TRUE;
                    }
                }
                else
                {
                    rval = 0;
                }
            }
            else
            {
                // select returned <= 0:
                if (rval < 0)
                {
                    if (errno == EINTR || errno == EAGAIN)
                    {
                        rval = 0;           // Try again:
                    }
                    else
                    {
                        // Recover for all other errors.
                        radMsgLog (PRI_HIGH, "ethernetReadExact: select error: %d: %s",
                                   errno, strerror(errno));
                        doRecover = TRUE;
                    }
                }
                else
                {
                    // Select timed out, return ERROR.
                    return ERROR;
                }
            }
        }

        // Do we need to attempt recovery?
        if (doRecover)
        {
            radMsgLog (PRI_HIGH, "ethernetReadExact: attempting socket recovery");
            ethernetRestart(med);

            // Go ahead and return ERROR, the caller can try again.
            return ERROR;
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
    
#if 0   /* keep this around for debug purposes */
if (index < len && len > 2)
{
    radMsgLog (PRI_MEDIUM, "ethernetReadExact: read %d of %d:", index, len);
    radMsgLogData (ptr, index);
}
#endif

    return ((index == len) ? len : ERROR);
}


#define ETH_FLUSH_TIME      20
static void ethernetFlush (WVIEW_MEDIUM *med, int queue)
{
    MEDIUM_ETHERNET *eth = (MEDIUM_ETHERNET *)med->workData;
    uint8_t         temp[SERIAL_BYTE_LENGTH_MAX];
    int             rval, cumTime = 0;
    uint64_t        readTime;

    while (cumTime < ETH_FLUSH_TIME)
    {
        readTime = radTimeGetMSSinceEpoch ();
        rval = radSocketReadExact (eth->sockId, temp, SERIAL_BYTE_LENGTH_MAX);
        if (rval < 0)
        {
            if (errno != EINTR && errno != EAGAIN)
            {
                return;
            }
        }

        readTime = radTimeGetMSSinceEpoch () - readTime;
        cumTime += (int)readTime;
        
        // now wait 5 ms (probably 10 ms on x86)
        readTime = radTimeGetMSSinceEpoch ();
        radUtilsSleep (5);
        readTime = radTimeGetMSSinceEpoch () - readTime;
        cumTime += (int)readTime;
        
        if (cumTime >= ETH_FLUSH_TIME && rval > 0)
        {
            // out of time, but data is still coming, fudge the time counter
            cumTime = ETH_FLUSH_TIME - 1;
        }
    }

    return;
}
        
static void ethernetDrain (WVIEW_MEDIUM *med)
{
    return;
}
        
static RADSOCK_ID ethernetGetSocket(WVIEW_MEDIUM *med)
{
    MEDIUM_ETHERNET *eth = (MEDIUM_ETHERNET *)med->workData;
    
    return eth->sockId;
}


/*  ... ----- API methods -----
*/

int ethernetMediumInit (WVIEW_MEDIUM *medium, char *hostname, int port)
{
    MEDIUM_ETHERNET     *work;
    
    memset (medium, 0, sizeof (*medium));
    
    work = (MEDIUM_ETHERNET *) malloc (sizeof (*work));
    if (work == NULL)
    {
        return ERROR;
    }
    memset (work, 0, sizeof (*work));
    
    medium->type = MEDIUM_TYPE_DEVICE;

    wvstrncpy (work->host, hostname, sizeof(work->host));
    work->port = port;
    
    // set our workData pointer for later use
    medium->workData = (void *)work;
    
    medium->init        = ethernetInit;
    medium->exit        = ethernetExit;
    medium->restart     = ethernetRestart;
    medium->read        = ethernetReadExact;
    medium->write       = ethernetWrite;
    medium->flush       = ethernetFlush;
    medium->txdrain     = ethernetDrain;
    medium->getsocket   = ethernetGetSocket;
    
    return OK;
}

