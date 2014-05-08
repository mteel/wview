/*---------------------------------------------------------------------------
 
  FILENAME:
        wh1080Protocol.c
 
  PURPOSE:
        Provide protocol utilities for WH1080 station communication.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        02/17/2011      M.S. Teel       0               Original
 
  NOTES:
        Parts of this implementation were inspired by the fowsr project
        (C) Arne-Jorgen Auberg (arne.jorgen.auberg@gmail.com) with hidapi
        mods by Bill Northcott.
 
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
#include <sysdefs.h>
#include <daemon.h>
#include <station.h>
#include <wh1080Protocol.h>

/*  ... global memory declarations
*/

/*  ... local memory
*/

static WH1080_WORK          wh1080Work;

// Define the position, decode type, conversion factor and the storage variable
// for the station data retrieved via USB:
static WH1080_DECODE_TYPE   decodeVals[] = 
{
    {0, ub,  1.0, &wh1080Work.sensorData.delay},        // Minutes since last stored reading (1:240)
    {1, ub,  1.0, &wh1080Work.sensorData.inhumidity},   // Indoor relative humidity % (1:99), 0xFF means invalid
    {2, ss,  0.1, &wh1080Work.sensorData.intemp},       // Indoor temp; Multiply by 0.1 to get C (-40:+60), 0xFFFF means invalid
    {4, ub,  1.0, &wh1080Work.sensorData.outhumidity},  // Outdoor relative humidity % (1:99), 0xFF means invalid
    {5, ss,  0.1, &wh1080Work.sensorData.outtemp},      // Outdoor temp; Multiply by 0.1 to get C (-40:+60) , 0xFFFF means invalid
    {7, us,  0.1, &wh1080Work.sensorData.pressure},     // Pressure; Multiply by 0.1 to get hPa (920:1080), 0xFFFF means invalid
    {9, wa,  0.1, &wh1080Work.sensorData.windAvgSpeed}, // Avg Wind; Multiply by 0.1 to get m/s (0:50), 0xFF means invalid
    {10, wg,  0.1, &wh1080Work.sensorData.windGustSpeed}, // Gust Wind; Multiply by 0.1 to get m/s (0:50), 0xFF means invalid
    {12, wd,  1.0, &wh1080Work.sensorData.windDir},     // Wind Dir; Multiply by 22.5 (0-15), 7th bit indicates invalid data
    {13, us,  0.3, &wh1080Work.sensorData.rain},        // Rain; Multiply by 0.33 to get mm
    {15, pb,  1.0, &wh1080Work.sensorData.status}       // Status; 6th bit indicates loss of contact with sensors, 7th bit indicates rainfall overflow
};

// Local methods:

static uint16_t getUSHORT(char* raw)
{
    unsigned char lo = (unsigned char)raw[0];
    unsigned char hi = (unsigned char)raw[1];
    return lo + (hi * 256);
}

static int16_t getSHORT(char* raw)
{
    unsigned char lo = (unsigned char)raw[0];
    unsigned char hi = (unsigned char)raw[1];
    uint16_t us = lo + (hi * 256);
    if (us >= 0x8000)                       // Test for sign bit
        return -(us - 0x8000);              // Negative value
    else
        return us;                          // Positive value
}

static uint8_t bcdDecode(uint8_t byte)
{
    uint8_t   lo = byte & 0x0F;
    uint8_t   hi = byte / 16;

    return (lo + (hi * 10));
}

static int decodeSensor(char* raw, enum ws_types ws_type, float scale, float* var)
{
    float           fresult;
    uint16_t        usTemp;

    switch (ws_type)
    {
    case ub:
        if ((unsigned char)raw[0] == 0xFF)
        {
            // Deal with humidity < 10% problem by hard-coding to 9:
            fresult = 9;
        }
        else
        {
            fresult = (unsigned char)raw[0] * scale;
        }
        break;
    case us:
        usTemp = getUSHORT(raw);
        if (usTemp == 0xFFFF)
        {
            return ERROR;
        }
        fresult = usTemp * scale;
        break;
    case ss:
        if (((unsigned char)raw[0] == 0xFF) && ((unsigned char)raw[1] == 0xFF))
        {
            return ERROR;
        }
        fresult = getSHORT(raw) * scale;
        break;
    case pb:
        fresult = (unsigned char)raw[0];
        break;
    case wa:
        // wind average - 12 bits split across a byte and a nibble
        if (((unsigned char)raw[0] == 0xFF) && (((unsigned char)raw[2] & 0x0F) == 0x0F))
        {
            return ERROR;
        }
        fresult = (unsigned char)raw[0] + (((unsigned char)raw[2] & 0x0F) * 256);
        fresult = fresult * scale;
        break;
    case wg:
        // wind gust - 12 bits split across a byte and a nibble
        if (((unsigned char)raw[0] == 0xFF) && (((unsigned char)raw[1] & 0xF0) == 0xF0))
        {
            return ERROR;
        }
        fresult = (unsigned char)raw[0] + (((unsigned char)raw[1] & 0xF0) * 16);
        fresult = fresult * scale;
        break;
    case wd:
        if (((unsigned char)raw[0] & 0x80) == 0x80)
        {
            return ERROR;
        }
        fresult = (unsigned char)raw[0] * scale;
        fresult *= 22.5;
        break;
    default:
        fresult = ARCHIVE_VALUE_NULL;
        break;
    }

    *var = fresult;
    return OK;
}

// Expects the medium to already be open:
static int readBlock (WVIEWD_WORK *work, int offset, uint8_t* buffer)
{
    // Read 32 bytes data at offset 'offset':
    // After sending the read command, the device will send back 32 bytes data within 100ms.
    // If not, then it means the command has not been received correctly.
    // Read repeatedly until the record stabilizes (two consecutive reads are equal).

    uint8_t  rqstBuffer[8], oldBuffer[32], newBuffer[32];
    int      retVal, readDone = FALSE, firstTime = TRUE;

    rqstBuffer[0] = 0xA1;                   // READ COMMAND
    rqstBuffer[1] = (char)(offset / 256);   // READ ADDRESS HIGH
    rqstBuffer[2] = (char)(offset & 0xFF);  // READ ADDRESS LOW
    rqstBuffer[3] = 0x20;                   // END MARK
    rqstBuffer[4] = 0xA1;                   // READ COMMAND
    rqstBuffer[5] = (char)(offset / 256);   // READ ADDRESS HIGH
    rqstBuffer[6] = (char)(offset & 0xFF);  // READ ADDRESS LOW
    rqstBuffer[7] = 0x20;                   // END MARK

    // Read any pending data on USB bus and discard before starting:
    (*(work->medium.usbhidRead))(&work->medium, newBuffer, 32, 500);

    // Read until consecutive reads produce an identical buffer:
    while ((!work->exiting) && (!readDone))
    {
        // Request read of 32-byte chunk from offset:
        retVal = (*(work->medium.usbhidWrite))(&work->medium, rqstBuffer, 8);
        if (retVal != 8)
        {
            radMsgLog (PRI_HIGH, "WH1080: write data request failed!");
            return ERROR;
        }
    
        // Read 32-byte chunk and place in buffer:
        retVal = (*(work->medium.usbhidRead))(&work->medium, newBuffer, 32, 1000);
        if (retVal != 32)
        {
            radMsgLog (PRI_HIGH, "WH1080: read data block failed!");
            return ERROR;
        }

        // Compare to previous result:
        if (!firstTime)
        {
            if (memcmp(oldBuffer, newBuffer, 32) == 0)
            {
                // They match and we are done:
                memcpy(buffer, newBuffer, 32);
                readDone = TRUE;
            }
            else
            {
                radMsgLog (PRI_STATUS, "WH1080: readBlock buffer still changing");
            }
        }
        else
        {
            firstTime = FALSE;
        }

        memcpy(oldBuffer, newBuffer, 32);
    }

    return OK;
}

// Expects the medium to already be open:
static int writeBlock (WVIEWD_WORK *work, int offset, uint8_t* buffer)
{
    // Write 32 bytes data at offset 'offset':

    char     rqstBuffer[8];
    int      retVal;

    rqstBuffer[0] = 0xA0;                   // WRITE COMMAND
    rqstBuffer[1] = (char)(offset / 256);   // WRITE ADDRESS HIGH
    rqstBuffer[2] = (char)(offset & 0xFF);  // WRITE ADDRESS LOW
    rqstBuffer[3] = 0x20;                   // END MARK
    rqstBuffer[4] = 0xA0;                   // WRITE COMMAND
    rqstBuffer[5] = (char)(offset / 256);   // WRITE ADDRESS HIGH
    rqstBuffer[6] = (char)(offset & 0xFF);  // WRITE ADDRESS LOW
    rqstBuffer[7] = 0x20;                   // END MARK

    // Request write of 32-byte chunk from offset:
    retVal = (*(work->medium.usbhidWrite))(&work->medium, rqstBuffer, 8);
    if (retVal != 8)
    {
        radMsgLog (PRI_HIGH, "WH1080: write data request failed!");
        return ERROR;
    }

    // Write 32-byte chunk:
    retVal = (*(work->medium.usbhidWrite))(&work->medium, buffer, 32);
    if (retVal != 32)
    {
        radMsgLog (PRI_HIGH, "WH1080: write data block failed!");
        return ERROR;
    }

    // Read 8-byte ACK:
    retVal = (*(work->medium.usbhidRead))(&work->medium, buffer, 8, 1000);
    if (retVal != 8)
    {
        radMsgLog (PRI_HIGH, "WH1080: read data ACK failed!");
        return ERROR;
    }

    return OK;
}

// Expects the medium to already be open:
static int writeDataRefresh (WVIEWD_WORK *work)
{
    uint8_t  rqstBuffer[8], readBuffer[8];
    int      retVal;

    rqstBuffer[0] = 0xA2;                   // One byte write command
    rqstBuffer[1] = 0;
    rqstBuffer[2] = 0x1A;
    rqstBuffer[3] = 0x20;
    rqstBuffer[4] = 0xA2;
    rqstBuffer[5] = 0xAA;
    rqstBuffer[6] = 0;
    rqstBuffer[7] = 0x20;

    retVal = (*(work->medium.usbhidWrite))(&work->medium, rqstBuffer, 8);
    if (retVal != 8)
    {
        radMsgLog (PRI_HIGH, "WH1080: write data ACK failed!");
        return ERROR;
    }

    // Read 8-byte ACK:
    retVal = (*(work->medium.usbhidRead))(&work->medium, readBuffer, 8, 1000);
    if (retVal != 8)
    {
        radMsgLog (PRI_HIGH, "WH1080: read data ACK failed!");
        return ERROR;
    }

    return OK;
}

// Expects the medium to already be open:
static int readFixedBlock(WVIEWD_WORK *work, uint8_t* block)
{
    // Read fixed block:
    if (readBlock(work, 0, block) == ERROR)
    {
        radMsgLog (PRI_HIGH, "WH1080: readFixedBlock readBlock failed");
        return ERROR;
    }

    // Check for valid magic numbers:
    // This is hardly an exhaustive list and I can find no definitive 
    // documentation that lists all possible values; further, I suspect it is
    // more of a header than a magic number...
    if ((block[0] == 0x55) ||
        (block[0] == 0xFF) ||
        (block[0] == 0x01) ||
        ((block[0] == 0x00) && (block[1] == 0x1E)) ||
        ((block[0] == 0x00) && (block[1] == 0x01)))
    {
        return OK;
    }
    else
    {
        radMsgLog (PRI_HIGH, "WH1080: readFixedBlock bad magic number %2.2X %2.2X",
                   (int)block[0], (int)block[1]);
        radMsgLog(PRI_HIGH, 
                  "WH1080: You may want to clear the memory on the station "
                  "console to remove any invalid records or data...");
        return ERROR_ABORT;
    }
}

// Expects the medium to already be open:
static int writeFixedBlock(WVIEWD_WORK *work, uint8_t* block)
{
    // Set for valid data:
    block[0] = 0x55;
    block[1] = 0xAA;

    if (writeBlock(work, 0, block) == ERROR)
    {
        return ERROR;
    }

    if (writeDataRefresh(work) == ERROR)
    {
        return ERROR;
    }

    return OK;
}

// Returns:
// OK - if new record retrieved
// ERROR - if there was an interface error
// ERROR_ABORT - if there is no new record (WH1080 generates new records at 
//               best once a minute)
static int readStationData (WVIEWD_WORK *work)
{
    WH1080_IF_DATA*     ifWorkData = (WH1080_IF_DATA*)work->stationData;
    int                 currentPosition, readPosition, index, retVal;

    if ((*(work->medium.usbhidInit))(&work->medium) != OK)
    {
        return ERROR;
    }

    // Read the WH1080 fixed block:
    retVal = readFixedBlock(work, &wh1080Work.controlBlock[0]);
    if (retVal == ERROR_ABORT)
    {
        // Try again later (bad magic number):
        (*(work->medium.usbhidExit))(&work->medium);
        return ERROR_ABORT;
    }
    else if (retVal == ERROR)
    {
        // USB interface error:
        (*(work->medium.usbhidExit))(&work->medium);
        return ERROR;
    }

    // Get the current record position; the WH1080 reports the record it is 
    // building, thus if it changes we need the prior just finished record:
    currentPosition = (int)getUSHORT(&wh1080Work.controlBlock[WH1080_CURRENT_POS]);

    // Make sure the index is aligned on 16-byte boundary:
    if ((currentPosition % 16) != 0)
    {
        // bogus, try again later:
        (*(work->medium.usbhidExit))(&work->medium);
        return ERROR_ABORT;
    }

    // Is this the first time?
    if (wh1080Work.lastRecord == -1)
    {
        // Yes.
        wh1080Work.lastRecord = currentPosition;
        (*(work->medium.usbhidExit))(&work->medium);
        return ERROR_ABORT;
    }

    // Is there a new record?
    if (currentPosition == wh1080Work.lastRecord)
    {
        // No, wait till it is finished.
        (*(work->medium.usbhidExit))(&work->medium);
        return ERROR_ABORT;
    }

    // Read last record that is now complete:
    if (readBlock(work, wh1080Work.lastRecord, &wh1080Work.recordBlock[0]) == ERROR)
    {
        radMsgLog (PRI_HIGH, "WH1080: read data block at index %d failed!",
                   wh1080Work.lastRecord);
        (*(work->medium.usbhidExit))(&work->medium);
        return ERROR;
    }

    (*(work->medium.usbhidExit))(&work->medium);

    readPosition = wh1080Work.lastRecord;
    wh1080Work.lastRecord = currentPosition;

//radMsgLogData(wh1080Work.recordBlock, 32);

    // Is the record valid? Check for unpopulated record or no sensor data
    // received status bit:
    if ((wh1080Work.recordBlock[WH1080_STATUS] & 0x40) != 0)
    {
        // No!
        radMsgLog (PRI_HIGH, 
                   "WH1080: data block at index %d has bad status, ignoring the record",
                   readPosition);
        return ERROR_ABORT;
    }

    // Parse the data received:
    for (index = 0; index < WH1080_NUM_SENSORS; index ++)
    {
        if (decodeSensor(&wh1080Work.recordBlock[decodeVals[index].pos],
                         decodeVals[index].ws_type,
                         decodeVals[index].scale,
                         decodeVals[index].var)
            != OK)
        {
            // Bad sensor data, abort this cycle:
            radMsgLog (PRI_HIGH, 
                       "WH1080: data block at index %d has bad sensor value, ignoring the record",
                       readPosition);
            return ERROR_ABORT;
        }
    }

    // Convert to Imperial units:
    wh1080Work.sensorData.intemp        = wvutilsConvertCToF(wh1080Work.sensorData.intemp);
    wh1080Work.sensorData.outtemp       = wvutilsConvertCToF(wh1080Work.sensorData.outtemp);
    wh1080Work.sensorData.pressure      = wvutilsConvertHPAToINHG(wh1080Work.sensorData.pressure);
    wh1080Work.sensorData.windAvgSpeed  = wvutilsConvertMPSToMPH(wh1080Work.sensorData.windAvgSpeed);
    wh1080Work.sensorData.windGustSpeed = wvutilsConvertMPSToMPH(wh1080Work.sensorData.windGustSpeed);
    wh1080Work.sensorData.rain          = wvutilsConvertMMToIN(wh1080Work.sensorData.rain);

    return OK;
}


static void storeLoopPkt (WVIEWD_WORK *work, LOOP_PKT *dest, WH1080_DATA *src)
{
    float               tempfloat;
    WH1080_IF_DATA*     ifWorkData = (WH1080_IF_DATA*)work->stationData;
    time_t              nowTime = time(NULL);

    // Clear optional data:
    stationClearLoopData(work);

    if ((10 < src->pressure && src->pressure < 50) &&
        (-150 < src->outtemp && src->outtemp < 150))
    {
        // WH1080 produces station pressure
        dest->stationPressure               = src->pressure;
    
        // Apply calibration here so the computed values reflect it:
        dest->stationPressure *= work->calMPressure;
        dest->stationPressure += work->calCPressure;
    
        // compute sea-level pressure (BP)
        tempfloat = wvutilsConvertSPToSLP(dest->stationPressure,
                                          src->outtemp,
                                          (float)ifWorkData->elevation);
        dest->barometer                     = tempfloat;
    
        // calculate altimeter
        tempfloat = wvutilsConvertSPToAltimeter(dest->stationPressure,
                                                (float)ifWorkData->elevation);
        dest->altimeter                     = tempfloat;
    }

    if (-150 < src->outtemp && src->outtemp < 150)
    {
        dest->outTemp  = src->outtemp;
    }

    if (0 <= src->outhumidity && src->outhumidity <= 100)
    {
        tempfloat = src->outhumidity;
        tempfloat += 0.5;
        dest->outHumidity  = (uint16_t)tempfloat;
    }

    if (0 <= src->windAvgSpeed && src->windAvgSpeed <= 250)
    {
        tempfloat = src->windAvgSpeed;
        dest->windSpeedF  = tempfloat;
    }

    if (0 <= src->windDir && src->windDir <= 360)
    {
        tempfloat = src->windDir;
        tempfloat += 0.5;
        dest->windDir        = (uint16_t)tempfloat;
        dest->windGustDir    = (uint16_t)tempfloat;
    }

    if (0 <= src->windGustSpeed && src->windGustSpeed <= 250)
    {
        tempfloat = src->windGustSpeed;
        dest->windGustF      = tempfloat;
    }

    if (0 <= src->rain)
    {
        if (!work->runningFlag)
        {
            // just starting, so start with whatever the station reports:
            ifWorkData->totalRain = src->rain;
            dest->sampleRain = 0;
        }
        else
        {
            // process the rain accumulator
            if (src->rain - ifWorkData->totalRain >= 0)
            {
                dest->sampleRain = src->rain - ifWorkData->totalRain;
                ifWorkData->totalRain = src->rain;
            }
            else
            {
                // we had a counter reset...
                dest->sampleRain = src->rain;
                dest->sampleRain += (WH1080_RAIN_MAX - ifWorkData->totalRain);
                ifWorkData->totalRain = src->rain;
            }
        }

        if (dest->sampleRain > 2)
        {
            // Not possible, filter it out:
            dest->sampleRain = 0;
        }

        // Update the rain accumulator:
        sensorAccumAddSample (ifWorkData->rainRateAccumulator, nowTime, dest->sampleRain);
        dest->rainRate    = sensorAccumGetTotal (ifWorkData->rainRateAccumulator);
        dest->rainRate   *= (60/WH1080_RAIN_RATE_PERIOD);
    }
    else
    {
        dest->sampleRain = 0;
        sensorAccumAddSample (ifWorkData->rainRateAccumulator, nowTime, dest->sampleRain);
        dest->rainRate                      = sensorAccumGetTotal (ifWorkData->rainRateAccumulator);
        dest->rainRate                      *= (60/WH1080_RAIN_RATE_PERIOD);
    }

    dest->inTemp                        = src->intemp;
    tempfloat = src->inhumidity;
    tempfloat += 0.5;
    dest->inHumidity                    = (uint16_t)tempfloat;

    return;
}


////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////  A P I  /////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

int wh1080Init (WVIEWD_WORK *work)
{
    WH1080_IF_DATA*     ifWorkData = (WH1080_IF_DATA*)work->stationData;
    fd_set              rfds;
    struct timeval      tv;
    int                 ret;
    time_t              nowTime = time(NULL) - (WV_SECONDS_IN_HOUR/(60/WH1080_RAIN_RATE_PERIOD));
    ARCHIVE_PKT         recordStore;
    unsigned char       controlBlock[WH1080_BUFFER_CHUNK];

    memset (&wh1080Work, 0, sizeof(wh1080Work));

    // Create the rain accumulator (WH1080_RAIN_RATE_PERIOD minute age)
    // so we can compute rain rate:
    ifWorkData->rainRateAccumulator = sensorAccumInit(WH1080_RAIN_RATE_PERIOD);

    // Populate the accumulator with the last WH1080_RAIN_RATE_PERIOD minutes:
    while ((nowTime = dbsqliteArchiveGetNextRecord(nowTime, &recordStore)) != ERROR)
    {
        sensorAccumAddSample(ifWorkData->rainRateAccumulator,
                             recordStore.dateTime,
                             recordStore.value[DATA_INDEX_rain]);
    }

    if ((*(work->medium.usbhidInit))(&work->medium) != OK)
    {
        return ERROR;
    }

    // Set the station to log data once per minute:
    while ((!work->exiting) && (readFixedBlock(work, controlBlock) != OK))
    {
        radMsgLog (PRI_HIGH, "WH1080: Initial fixed block read failed");
        (*(work->medium.usbhidExit))(&work->medium);
        radUtilsSleep(5000);
        (*(work->medium.usbhidInit))(&work->medium);
        radMsgLog (PRI_HIGH, "WH1080: Retrying initial fixed block read");
    }

    // For some reason the WH1080 wants the IF closed between a read and a write:
    (*(work->medium.usbhidExit))(&work->medium);
    if (work->exiting)
    {
        return ERROR;
    }

    controlBlock[WH1080_SAMPLING_INTERVAL] = 1;
    (*(work->medium.usbhidInit))(&work->medium);

    if (writeFixedBlock(work, controlBlock) == ERROR)
    {
        (*(work->medium.usbhidExit))(&work->medium);
        return ERROR;
    }

    radUtilsSleep(2000);

    (*(work->medium.usbhidExit))(&work->medium);

    radUtilsSleep(1000);

    wh1080Work.lastRecord = -1;

    // populate the LOOP structure:
    radMsgLog (PRI_HIGH, "Waiting for the next weather record to be ready "
                         "in the console to populate initial wview sensor readings "
                         "(this could take some time);");
    radMsgLog (PRI_HIGH, "While waiting be sure you are receiving all sensors on the console;");
    radMsgLog (PRI_HIGH, "if not, you may need to relocate the sensors or the console.");
    while ((!work->exiting) && (readStationData(work) != OK))
    {
        radUtilsSleep(1000);
    }
    if (work->exiting)
    {
        return ERROR;
    }

    ifWorkData->wh1080Readings = wh1080Work.sensorData;
    storeLoopPkt (work, &work->loopPkt, &ifWorkData->wh1080Readings);

    // we must indicate successful completion here -
    // even though we are synchronous, the daemon wants to see this event:
    radProcessEventsSend (NULL, STATION_INIT_COMPLETE_EVENT, 0);

    return OK;
}

void wh1080Exit (WVIEWD_WORK *work)
{
    return;
}

void wh1080ReadData (WVIEWD_WORK *work)
{
    return;
}

void wh1080GetReadings (WVIEWD_WORK *work)
{
    WH1080_IF_DATA*  ifWorkData = (WH1080_IF_DATA*)work->stationData;

    if (readStationData(work) == OK)
    {
        // populate the LOOP structure:
        ifWorkData->wh1080Readings = wh1080Work.sensorData;
        storeLoopPkt (work, &work->loopPkt, &ifWorkData->wh1080Readings);

        // indicate the LOOP packet is done
        radProcessEventsSend (NULL, STATION_LOOP_COMPLETE_EVENT, 0);
    }
}

