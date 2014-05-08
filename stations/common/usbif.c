/*---------------------------------------------------------------------------
 
  FILENAME:
        usbif.c
 
  PURPOSE:
        Provide the weather station USB medium utilities.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        02/14/2011      M.S. Teel       0               Original
 
  NOTES:
        wview medium-specific routines to be supplied:
        
        usbifMediumInit        - sets up function pointers and work area
        usbifInit              - initialize
        usbifExit              - exit
        usbifInterruptRead     - interrupt driven reads
        usbifInterruptWrite    - interrupt driven writes
        usbifSendControlMsg    - send a control msg
        usbifGetDescriptor     - Get descriptor data
        usbifClaimInterface    - claim an interface
        usbifReleaseInterface  - release an interface
        usbifSetAltInterface   - set an alternate interface
        usbifSetConfiguration  - set configuration
        
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
#include <usbif.h>

/*  ... global memory declarations
*/

/*  ... local memory
*/
static MEDIUM_USB   mediumUSB;


static struct usbcompat_device *find_device(int vendor, int product)
{
    struct usbcompat_bus *bus;

    for (bus = usbcompat_get_busses(); bus != NULL; bus = bus->next)
    {
        struct usbcompat_device *dev;

        for (dev = bus->devices; dev != NULL; dev = dev->next)
        {
            if (dev->descriptor.idVendor == vendor && 
                dev->descriptor.idProduct == product)
            {
                // Found it!
                return dev;
            }
        }
    }
    return NULL;
}


//////////////////////////////////////////////////////////////////////////////
//  ... medium callback functions
//////////////////////////////////////////////////////////////////////////////

static int usbifInit (WVIEW_MEDIUM *med)
{
    MEDIUM_USB      *usbifWork = (MEDIUM_USB *)med->workData;
    int             retVal;
    uint8_t         buffer[1024];

    usbcompat_init();
    usbcompat_find_busses();
    usbcompat_find_devices();

    usbifWork->dev = find_device(usbifWork->vendorId, usbifWork->productId);
    if (usbifWork->dev == NULL)
    {
        radMsgLog (PRI_HIGH, "USB: device for vendor %d, product %d not found!",
                   usbifWork->vendorId, usbifWork->productId);
        return ERROR;
    }

    med->usbHandle = usbcompat_open(usbifWork->dev);
    if (med->usbHandle == NULL)
    {
        radMsgLog (PRI_HIGH, "USB: failed to obtain device handle");
        return ERROR;
    }

    retVal = usbcompat_get_driver_np(med->usbHandle, 0, buffer, sizeof(buffer));
    if (retVal == 0)
    {
        retVal = usbcompat_detach_kernel_driver_np(med->usbHandle, 0);
    }

    retVal = usbcompat_claim_interface(med->usbHandle, 0);
    if (retVal != 0)
    {
        radMsgLog (PRI_HIGH, "USB: claim_interface failed: %d", retVal);
        return ERROR;
    }

    retVal = usbcompat_set_altinterface(med->usbHandle, 0);
    if (retVal != 0)
    {
        radMsgLog (PRI_HIGH, "USB: set_altinterface failed: %d", retVal);
        return ERROR;
    }

    return OK;
}

static void usbifExit (WVIEW_MEDIUM *med)
{
	usbcompat_release_interface(med->usbHandle, 0);
	usbcompat_close(med->usbHandle);
	med->usbHandle = NULL;
    return;
}

static int usbifInterruptRead
(
    struct _wview_medium    *medium,
    int                     ep, 
    char                    *buffer,
    int                     length, 
    int                     timeout
)
{
    return usbcompat_interrupt_read(medium->usbHandle, ep, buffer, length, timeout);
}

static int usbifInterruptWrite
(
    struct _wview_medium    *medium,
    int                     ep, 
    char                    *buffer,
    int                     length, 
    int                     timeout
)
{
    return usbcompat_interrupt_write(medium->usbHandle, ep, buffer, length, timeout);
}

static int usbifSendControlMsg
(
    struct _wview_medium    *medium, 
    int                     bmRequestType,
    int                     bRequest, 
    int                     wValue, 
    int                     wIndex, 
    char                    *buffer, 
    int                     length, 
    int                     timeout
)
{
    return usbcompat_control_msg(medium->usbHandle, 
                                 bmRequestType, 
                                 bRequest,
                                 wValue, 
                                 wIndex, 
                                 buffer, 
                                 length,
                                 timeout);
}

static int usbifGetDescriptor
(
    struct _wview_medium    *medium,
    uint8_t                 type,
    uint8_t                 desc_index, 
    void                    *buffer, 
    int                     length
)
{
    return usbcompat_get_descriptor(medium->usbHandle, 
                                    type,
                                    desc_index, 
                                    buffer, 
                                    length);
}

static int usbifClaimInterface(struct _wview_medium *medium, int interface)
{
    return usbcompat_claim_interface(medium->usbHandle, interface);
}

static int usbifReleaseInterface(struct _wview_medium *medium, int interface)
{
    return usbcompat_release_interface(medium->usbHandle, interface);
}

static int usbifSetAltInterface(struct _wview_medium *medium, int alternate)
{
    return usbcompat_set_altinterface(medium->usbHandle, alternate);
}

static int usbifSetConfiguration(struct _wview_medium *medium, int configuration)
{
    return usbcompat_set_configuration(medium->usbHandle, configuration);
}
 

// ... ----- API methods -----

int usbifMediumInit
(
    WVIEW_MEDIUM    *medium,
    uint16_t        vendor_id,
    uint16_t        product_id
)
{
    MEDIUM_USB      *work = &mediumUSB;

    memset (medium, 0, sizeof (*medium));
    memset (work, 0, sizeof (*work));

    work->vendorId  = vendor_id;
    work->productId = product_id;

    medium->type = MEDIUM_TYPE_USB;

    // set our workData pointer for later use
    medium->workData = (void *)work;

    medium->usbInit             = usbifInit;
    medium->usbExit             = usbifExit;
    medium->usbInterruptRead    = usbifInterruptRead;
    medium->usbInterruptWrite   = usbifInterruptWrite;
    medium->usbSendControlMsg   = usbifSendControlMsg;
    medium->usbGetDescriptor    = usbifGetDescriptor;
    medium->usbClaimInterface   = usbifClaimInterface;
    medium->usbReleaseInterface = usbifReleaseInterface;
    medium->usbSetAltInterface  = usbifSetAltInterface;
    medium->usbSetConfiguration = usbifSetConfiguration;

    return OK;
}

