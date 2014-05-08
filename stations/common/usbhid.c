/*---------------------------------------------------------------------------
 
  FILENAME:
        usbhid.c
 
  PURPOSE:
        Provide the weather station USB HID medium utilities.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        02/15/2011      M.S. Teel       0               Original
 
  NOTES:
        wview medium-specific routines to be supplied:
        
        usbhidMediumInit       - sets up function pointers and work area
        usbhidInit             - initialize
        usbhidExit             - exit
        usbhidRead             - blocking read until specified bytes are read
        usbhidWrite            - write on medium
        
        See daemon.h for details of the WVIEW_MEDIUM structure.
 
  LICENSE:
        Copyright (c) 2011, Mark S. Teel (mark@teel.ws)
  
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
#include <daemon.h>
#include <station.h>
#include <usbhid.h>

/*  ... global memory declarations
*/

/*  ... local memory
*/
static MEDIUM_USBHID    mediumUSB;
static void usbhidExit (WVIEW_MEDIUM *med);

//////////////////////////////////////////////////////////////////////////////
//  ... medium callback functions
//////////////////////////////////////////////////////////////////////////////

static int usbhidInit (WVIEW_MEDIUM *medium)
{
    MEDIUM_USBHID   *usbhidWork = (MEDIUM_USBHID *)medium->workData;

    // Open the HID device:
    medium->hidDevice = hid_open(usbhidWork->vendorId, usbhidWork->productId);
    if (medium->hidDevice == NULL)
    {
        radMsgLog (PRI_HIGH, "USBHID: hid_open failed!");
        return ERROR;
    }

    // Set to non-blocking so we can do timed read exact:
    if (hid_set_nonblocking(medium->hidDevice, !usbhidWork->blocking) != 0)
    {
        radMsgLog (PRI_HIGH, "USBHID: hid_set_nonblocking failed!");
        usbhidExit(medium);
        return ERROR;
    }

    return OK;
}

static void usbhidExit (WVIEW_MEDIUM *med)
{
    if (med->hidDevice != NULL)
        hid_close(med->hidDevice);
	med->hidDevice = NULL;

    return;
}

static int usbhidRead
(
    struct _wview_medium    *medium,
    void                    *buffer,
    int                     length, 
    int                     msTimeout
)
{
    int                     rval, cumTime = 0, index = 0;
    uint64_t                readTime;
    uint8_t                 *ptr = (uint8_t *)buffer;
    MEDIUM_USBHID           *usbhidWork = (MEDIUM_USBHID *)medium->workData;

    while (index < length)
    {
        // Bail out if non-blocking IO and timeout has expired.
        if (!usbhidWork->blocking && cumTime >= msTimeout)
        {
            return index;
        }

        readTime = radTimeGetMSSinceEpoch ();
        rval = hid_read(medium->hidDevice, &ptr[index], length - index);
        if (rval < 0)
        {
            if (errno != EINTR && errno != EAGAIN)
            {
                return ERROR;
            }
        }
        else
        {
            if (rval > 0 && usbhidWork->debug)
            {
                radMsgLog (PRI_STATUS, "usbhidRead:");
                radMsgLogData(&ptr[index], rval);
            }
            index += rval;
        }

        readTime = radTimeGetMSSinceEpoch () - readTime;
        cumTime += (int)readTime;
        if (!usbhidWork->blocking && index < length && cumTime < msTimeout)
        {
            readTime = radTimeGetMSSinceEpoch ();
            radUtilsSleep (9);
            readTime = radTimeGetMSSinceEpoch () - readTime;
            cumTime += (int)readTime;
        }
    }

    return index;
}

static int usbhidReadSpecial
(
    struct _wview_medium    *medium,
    void                    *buffer,
    int                     length, 
    int                     msTimeout
)
{
    int                     rval, cumTime = 0, index = 0, i, blockLen;
    uint64_t                readTime;
    uint8_t                 *ptr = (uint8_t *)buffer;
    uint8_t                 readBuf[8];
    MEDIUM_USBHID           *usbhidWork = (MEDIUM_USBHID *)medium->workData;

    while (index < length && cumTime < msTimeout)
    {
        readTime = radTimeGetMSSinceEpoch ();

        // Read in 8-byte blocks:
        rval = hid_read(medium->hidDevice, &readBuf[0], 8);
        if (rval < 0)
        {
            if (errno != EINTR && errno != EAGAIN)
            {
                return ERROR;
            }
        }
        else if (rval > 0)
        {
            // Now we should have an 8-byte block of data:
            // Process the first byte as a length field:
            blockLen = (int)readBuf[0];
            if ((blockLen > 7) || (blockLen > (rval-1)))
            {
                // Impossible:
                return ERROR;
            }
    
            // Copy the length given:
            memcpy(&ptr[index], &readBuf[1], MIN(blockLen,(length-index)));
            index += MIN(blockLen,(length-index));
        }

        readTime = radTimeGetMSSinceEpoch () - readTime;
        cumTime += (int)readTime;
        if ((index < length) && (cumTime < msTimeout))
        {
            readTime = radTimeGetMSSinceEpoch ();
            radUtilsSleep (9);
            readTime = radTimeGetMSSinceEpoch () - readTime;
            cumTime += (int)readTime;
        }
    }

    if (index > 0 && usbhidWork->debug)
    {
        radMsgLog (PRI_STATUS, "usbhidReadSpecial:");
        radMsgLogData(&ptr[0], index);
    }

    return index;
}

static int usbhidWrite
(
    struct _wview_medium    *medium,
    void                    *buffer,
    int                     length
)
{
    int                     retVal, index = 0;
    MEDIUM_USBHID           *usbhidWork = (MEDIUM_USBHID *)medium->workData;
    uint8_t*                sendPtr = (uint8_t*)buffer;

    // Write in 8-byte chunks:
    while (index < length)
    {
        retVal = hid_write(medium->hidDevice, &sendPtr[index], 8);
        if (retVal != 8)
        {
            if (retVal == -1)
            {
                if (usbhidWork->debug)
                {
                    radMsgLog (PRI_HIGH, "USBHID: hid_write failed");
                }
                return ERROR;
            }
            else
            {
                if (usbhidWork->debug)
                {
                    radMsgLog (PRI_HIGH, "USBHID: hid_write is SHORT: %d of %d",
                               retVal, length);
                }
                return retVal;
            }
        }

        index += 8;
    }

    if (usbhidWork->debug)
    {
        radMsgLog (PRI_STATUS, "usbhidWrite:");
        radMsgLogData(buffer, length);
    }

    return length;
}


// ... ----- API methods -----

int usbhidMediumInit
(
    WVIEW_MEDIUM    *medium,
    uint16_t        vendor_id,
    uint16_t        product_id,
    int             enableDebug,
    int             blocking
)
{
    MEDIUM_USBHID   *work = &mediumUSB;

    memset (medium, 0, sizeof (*medium));
    memset (work, 0, sizeof (*work));

    work->vendorId  = vendor_id;
    work->productId = product_id;
    work->debug     = enableDebug;
    work->blocking  = blocking;

    medium->type = MEDIUM_TYPE_USBHID;

    // set our workData pointer for later use
    medium->workData = (void *)work;

    medium->usbhidInit          = usbhidInit;
    medium->usbhidExit          = usbhidExit;
    medium->usbhidRead          = usbhidRead;
    medium->usbhidReadSpecial   = usbhidReadSpecial;
    medium->usbhidWrite         = usbhidWrite;

    return OK;
}

