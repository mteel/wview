#ifndef INC_wh1080protocolh
#define INC_wh1080protocolh
/*---------------------------------------------------------------------------
 
  FILENAME:
        wh1080Protocol.h
 
  PURPOSE:
        Provide protocol utilities for WH1080 station communication.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        02/17/2011      M.S. Teel       0               Original
 
  NOTES:
        Parts of this implementation were inspired by the fowsr project
        (C) Arne-Jorgen Auberg (arne.jorgen.auberg@gmail.com) with hidapi
        mods by Bill Northcott.

        The WH1080 protocol is undocumented. The following was observed
        by sniffing the USB interface:

        A1 is a read command: 
        It is sent as A1XX XX20 A1XX XX20 where XXXX is the offset in the 
        memory map. The WH1080 responds with 4 8 byte blocks to make up a 
        32 byte read of address XXXX.

        A0 is a write command: 
        It is sent as A0XX XX20 A0XX XX20 where XXXX is the offset in the 
        memory map. It is followed by 4 8 byte chunks of data to be written 
        at the offset. The WH1080 acknowledges the write with an 8 byte 
        chunk: A5A5 A5A5.

        A2 is a one byte write command. 
        It is used as: A200 1A20 A2AA 0020 to indicate a data refresh.
        The WH1080 acknowledges the write with an 8 byte chunk: A5A5 A5A5.
 
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
#include <radsocket.h>

/*  ... Local include files
*/
#include <datadefs.h>
#include <dbsqlite.h>
#include <daemon.h>
#include <parser.h>
#include <sensor.h>
#include <usbhid.h>

/* WH1080  <vendorid, productid> */
#define WH1080_VENDOR_ID                0x1941
#define WH1080_PRODUCT_ID               0x8021


// Define the rain rate acuumulator period (minutes):
#define WH1080_RAIN_RATE_PERIOD         5

// Weather Station buffer parameters:
#define WH1080_RAIN_MAX             0x10000     // Wrap value for rain counter
#define WH1080_BUFFER_START         0x100       // Size of fixed block
                                                // start of buffer records
#define WH1080_BUFFER_CHUNK         0x20        // Size of chunk received over USB

// Weather Station record memory positions:
#define WH1080_DELAY                0   // Position of delay parameter
#define WH1080_HUMIDITY_IN          1   // Position of inside humidity parameter
#define WH1080_TEMPERATURE_IN       2   // Position of inside temperature parameter
#define WH1080_HUMIDITY_OUT         4   // Position of outside humidity parameter
#define WH1080_TEMPERATURE_OUT      5   // Position of outside temperature parameter
#define WH1080_ABS_PRESSURE         7   // Position of absolute pressure parameter
#define WH1080_WIND_AVE             9   // Position of wind direction parameter
#define WH1080_WIND_GUST            10  // Position of wind direction parameter
#define WH1080_WIND_DIR             12  // Position of wind direction parameter
#define WH1080_RAIN                 13  // Position of rain parameter
#define WH1080_STATUS               15  // Position of status parameter

// Control block offsets:
#define WH1080_SAMPLING_INTERVAL    16  // Position of sampling interval
#define WH1080_DATA_COUNT           27  // Position of data_count parameter
#define WH1080_CURRENT_POS          30  // Position of current_pos parameter

// Types for decoding raw weather station data.
//	ub 	unsigned byte
//	sb	signed byte
//	us unsigned short
//	ss	signed short
//	dt	date time  bcd  yymmddhhmm
//	tt	time bcd  hhmm
//	pb	status - bit 6 lost contact - bit 7 rain counter overflow
//	wa	wind average low bits puls lower bits of address +2
//	wg	wind gust low bits plus upper bits of address +1
//	wd  wind direction

enum ws_types {ub,sb,us,ss,dt,tt,pb,wa,wg,wd};

#define WH1080_NUM_SENSORS              11

// Define the decoder type structure:
typedef struct _decodeType
{
    int             pos;
    enum ws_types   ws_type;
    float           scale;
    float*          var;
} WH1080_DECODE_TYPE;

// define the readings collector
typedef struct
{
    float   delay;
    float   inhumidity;
    float   intemp;
    float   outhumidity;
    float   outtemp;
    float   pressure;
    float   windAvgSpeed;
    float   windGustSpeed;
    float   windDir;
    float   rain;
    float   status;
} WH1080_DATA;


// define the work area
typedef struct
{
    WH1080_DATA     sensorData;
    uint8_t         controlBlock[WH1080_BUFFER_CHUNK];
    uint8_t         recordBlock[WH1080_BUFFER_CHUNK];
    int             lastRecord;
} WH1080_WORK;


// define WH1080-specific interface data here
typedef struct
{
    int             elevation;
    float           latitude;
    float           longitude;
    int             archiveInterval;
    WH1080_DATA     wh1080Readings;
    float           totalRain;              // to track cumulative changes
    WV_ACCUM_ID     rainRateAccumulator;    // to compute rain rate
} WH1080_IF_DATA;


// call once during initialization
extern int wh1080Init (WVIEWD_WORK *work);

// do cleanup
extern void wh1080Exit (WVIEWD_WORK *work);

// read data from station:
extern void wh1080ReadData (WVIEWD_WORK *work);

// get loop packet data:
extern void wh1080GetReadings (WVIEWD_WORK *work);

#endif

