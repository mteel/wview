#ifndef INC_wmrusbprotocolh
#define INC_wmrusbprotocolh
/*---------------------------------------------------------------------------
 
  FILENAME:
        wmrusbprotocol.h
 
  PURPOSE:
        Provide protocol utilities for WMR station communication.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        03/10/2011      M.S. Teel       0               Original.
 
  NOTES:
        
 
  LICENSE:
  
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
#include <sys/socket.h>
#include <string.h>
#include <errno.h>

/*  ... Library include files
*/
#include <sysdefs.h>
#include <radtimeUtils.h>
#include <radthread.h>

/*  ... Local include files
*/
#include <datadefs.h>
#include <dbsqlite.h>
#include <usbhid.h>
#include <daemon.h>
#include <parser.h>
#include <sensor.h>

// Uncomment this to enable debug messages (lots of logs).
//#define WMR_DEBUG           1

// Uncomment this to enable byte counts (USB raw, stream, packet).
//#define WMR_COUNT_BYTES     1

// Uncomment this to dump raw USB RX bytes (lots of logs).
//#define WMR_DUMP_RAW_USB    1

#ifdef WMR_COUNT_BYTES
extern int      UsbRawBytes;
extern int      StreamBytes;
extern int      PacketBytes;
extern int      ChecksumBytes;
extern int      StatCount;
extern int      UnknownPacketType;
#endif


/* WMR-200  <vendorid, productid> */
#define WMR_VENDOR_ID               0x0fde
#define WMR_PRODUCT_ID              0xca01


#define WMR_BUFFER_LENGTH           255
#define WMR_READ_WAIT               1000
#define WMR_TEMP_SENSOR_COUNT       10
#define WMR_REESTABLISH_SLEEP       10000               // 10 secs
#define WMR_PROCESS_TIME_INTERVAL   1000                // 1 second
#define WMR_HEARTBEAT_INTERVAL      10                  // seconds

// Define the rain rate acuumulator period (minutes):
#define WMR_RAIN_RATE_PERIOD        5

// Define the WMRUSB station data message:
//  WVIEW_MSG_TYPE_STATION_DATA (WMRUSB version):
typedef struct
{
    uint8_t         data[WMR_BUFFER_LENGTH];
    int             length;
}
__attribute__ ((packed)) WMRUSB_MSG_DATA;



// Define WMR protocol types:
typedef enum
{
    WMR_PROTOCOL_UNKNOWN            = 0,
    WMR_PROTOCOL_FFFF               = 1,
    WMR_PROTOCOL_D0                 = 2
} WMR_PROTOCOL_TYPE;

// Used for RX mask:
enum _SensorTypes
{
    WMR_SENSOR_WIND                 = 0x01,
    WMR_SENSOR_RAIN                 = 0x02,
    WMR_SENSOR_OUT_TEMP             = 0x04,
    WMR_SENSOR_PRESSURE             = 0x08,
    WMR_SENSOR_ALL                  = 0x0D          // Don't require rain to start
};

#define WMR_TEMP_SENSOR_OUT         1
#define WMR_TEMP_SENSOR_IN          0

// Define WMR_D0 pkt types:
typedef enum
{
    WMR_D0_HISTORY                  = 0xD2,
    WMR_D0_RAIN                     = 0xD4,
    WMR_D0_TEMP                     = 0xD7,
    WMR_D0_PRESSURE                 = 0xD6,
    WMR_D0_WIND                     = 0xD3,
    WMR_D0_STATUS                   = 0xD9,
    WMR_D0_UV                       = 0xD5
} WMR_D0_TYPE;

// Define WMR_FFFF pkt types:
typedef enum
{
    WMR_FFFF_RAIN                   = 0x41,
    WMR_FFFF_TEMP                   = 0x42,
    WMR_FFFF_TEMPONLY               = 0x44,
    WMR_FFFF_PRESSURE               = 0x46,
    WMR_FFFF_UV                     = 0x47,
    WMR_FFFF_WIND                   = 0x48,
    WMR_FFFF_DATETIME               = 0x60
} WMR_FFFF_TYPE;


// parsing helper macros:
#define LO(byte)                (byte & 0x0f)
#define HI(byte)                ((byte & 0xf0) >> 4)
#define MHI(byte)               ((byte & 0x0f) << 4)
#define NUM(byte)               (10 * HI(byte) + LO(byte))
#define BIT(byte, bit)          ((byte & (1 << bit)) >> bit)


// define the readings collector
typedef struct
{
    float   temp[WMR_TEMP_SENSOR_COUNT];
    float   humidity[WMR_TEMP_SENSOR_COUNT];
    float   dewpoint[WMR_TEMP_SENSOR_COUNT];
    float   windDir;
    float   windAvgSpeed;
    float   windGustSpeed;
    float   pressure;
    int     UV;
    float   rain1h;
    float   rain24h;
    float   rainAccum;
    float   rainRate;
    uint8_t tendency;
} WMR_DATA;


// define the work area
typedef struct
{
    WMR_PROTOCOL_TYPE   protocol;
    int                 started;
    WMR_DATA            sensorData;
    uint8_t             readData[WMR_BUFFER_LENGTH];
    int                 readIndex;
    uint32_t            heartBeatCounter;
    uint32_t            lastDataRX;
    uint8_t             dataRXMask;
} WMR_WORK;


// define WMR-specific interface data here
typedef struct
{
    int             elevation;
    float           latitude;
    float           longitude;
    int             archiveInterval;
    WMR_DATA        wmrReadings;
    float           totalRain;              // to track cumulative changes
    int             outsideChannel;
    WV_ACCUM_ID     rainRateAccumulator;    // to compute rain rate
} WMR_IF_DATA;


// call once during initialization
extern int wmrInit (WVIEWD_WORK *work);

// do cleanup
extern void wmrExit (WVIEWD_WORK *work);

// read data from station:
extern void wmrReadData (WVIEWD_WORK *work, WMRUSB_MSG_DATA* msg);

// Enforce packet framing and pass to parse engine if a packet frame is complete:
extern int wmrProcessData (WVIEWD_WORK *work);

// get loop packet data:
extern void wmrGetReadings (WVIEWD_WORK *work);

#endif

