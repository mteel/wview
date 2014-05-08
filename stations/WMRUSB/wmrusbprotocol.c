/*---------------------------------------------------------------------------

  FILENAME:
        wmrusbprotocol.c

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
#include <wmrusbprotocol.h>

/*  ... global memory declarations
*/

/*  ... local memory
*/
#ifdef WMR_COUNT_BYTES
int      UsbRawBytes;
int      StreamBytes;
int      PacketBytes;
int      ChecksumBytes;
int      StatCount;
int      UnknownPacketType;
#endif

static WMR_WORK             wmrWork;

static char*                WMRSensorNames[4] =
{
    "Wind",
    "Rain",
    "Temp",
    "Pressure"
};

/* utility functions */

static float d2temp (int a, int b)
{
    a &= 0xff;
    b &= 0xff;
    int t = (b << 8) | a;
    if (t & 0x8000)
    {
        t &= 0x7FFF;
        return -(t / 10.0);
    }
    else
    {
        return (t / 10.0);
    }
}

static int checksum (unsigned char *ptr, int len)
{
    int sum = 0;
    int a = ptr[len-2];
    int b = ptr[len-1];

    int compare = (b << 8) | a;
    len -= 2;
    while (len)
    {
        sum += *ptr++;
        --len;
    }

    return (sum == compare);

}

/* frame decoding functions */

static void decodeRain (unsigned char *ptr)
{
    if (ptr[0] != 0xFF && ptr[1] != 0xFF && ptr[6] != 0xFF && ptr[7] != 0xFF)
    {
        wmrWork.sensorData.rainRate  = 0.01 * ((int)(ptr[1]*255) + ptr[0]);
        wmrWork.sensorData.rain1h    = 0.01 * ((int)(ptr[3]*255) + ptr[2]);
        wmrWork.sensorData.rain24h   = 0.01 * ((int)(ptr[5]*255) + ptr[4]);
        wmrWork.sensorData.rainAccum = 0.01 * ((int)(ptr[7]*255) + ptr[6]);

        wmrWork.dataRXMask |= WMR_SENSOR_RAIN;

        if (!wmrWork.started)
        {
            radMsgLog (PRI_MEDIUM, "received RAIN packet...");
        }
#ifdef WMR_DEBUG
        radMsgLog(PRI_MEDIUM, "RAIN: Rate:%f, 1H:%f, 24H:%f, ACCUM:%f",
                  wmrWork.sensorData.rainRate,
                  wmrWork.sensorData.rain1h,
                  wmrWork.sensorData.rain24h,
                  wmrWork.sensorData.rainAccum);
        radMsgLogData(ptr, 8);
#endif
    }
}

static void decodeWind (unsigned char *ptr)
{
    float       avg, gust;

    if (ptr[0] != 0xFF && ptr[1] != 0xFF)
    {
        gust     = (float)((int)ptr[2] + ((int)(ptr[3] & 0x0F)*256));
        gust     /= 10;
        gust     = wvutilsConvertMPSToMPH(gust);

        avg      = (float)((int)ptr[4]*16) + ((int)ptr[3] >> 4);
        avg      /= 10;
        avg      = wvutilsConvertMPSToMPH(avg);

        // Sanity check:
        if ((gust < 0) || (gust > 250) || (avg < 0) || (avg > 250))
        {
            // packet is bogus:
            return;
        }

        wmrWork.sensorData.windGustSpeed    = gust;
        wmrWork.sensorData.windAvgSpeed     = avg;

        // wind direction unit is 22.5 degrees
        wmrWork.sensorData.windDir = (ptr[0] & 0x0F) * 22.5;

        wmrWork.dataRXMask |= WMR_SENSOR_WIND;

        if (!wmrWork.started)
        {
            radMsgLog (PRI_MEDIUM, "received WIND packet...");
        }
    }
}

static void decodeTemp (unsigned char *ptr)
{
    unsigned int    sensor = ptr[0] & 0x0F;
    float           humid, temp, dew;

    if (sensor < WMR_TEMP_SENSOR_COUNT)
    {
        humid = (float)ptr[3];
        temp  = wvutilsConvertCToF(d2temp(ptr[1], ptr[2]));
        dew   = wvutilsConvertCToF(d2temp(ptr[4], ptr[5]));

        // Sanity check the values (allow extra sensors to have bogus humidity):
        if (((sensor <= WMR_TEMP_SENSOR_OUT) && (humid > 100)) || (temp < -150) || (temp > 150))
        {
            // this packet is bogus:
            return;
        }

        wmrWork.sensorData.humidity[sensor] = humid;
        wmrWork.sensorData.temp[sensor]     = temp;
        wmrWork.sensorData.dewpoint[sensor] = dew;

        if (sensor == WMR_TEMP_SENSOR_OUT)
        {
            wmrWork.dataRXMask |= WMR_SENSOR_OUT_TEMP;

            if (!wmrWork.started)
            {
                radMsgLog (PRI_MEDIUM, "received TEMP packet...");
            }
        }
    }
}

static void decodeTempOnly (unsigned char *ptr)
{
    unsigned int    sensor = ptr[0] & 0x0F;
    float           temp;

    if (sensor < WMR_TEMP_SENSOR_COUNT)
    {
        temp  = wvutilsConvertCToF(d2temp(ptr[1], ptr[2]));

        // Sanity check the values:
        if ((temp < -150) || (temp > 150))
        {
            // this packet is bogus:
            return;
        }

        wmrWork.sensorData.temp[sensor]     = temp;
    }
}

static void decodePressure (unsigned char *ptr)
{
    float       pressure;

    // station provides pressure in hPa
    pressure = (((int)(ptr[1] & 0x0F)) << 8) + (int)ptr[0];
    pressure = wvutilsConvertHPAToINHG(pressure);

    // Sanity check it:
    if ((pressure < 20) || (pressure > 40))
    {
        // packet is bogus:
        return;
    }

    wmrWork.sensorData.pressure = pressure;
    wmrWork.dataRXMask |= WMR_SENSOR_PRESSURE;

    if (!wmrWork.started)
    {
        radMsgLog (PRI_MEDIUM, "received PRESSURE packet...");
    }
}

static void decodeUV (unsigned char *ptr)
{
    if (ptr[0] == 0xFF)
    {
        wmrWork.sensorData.UV = -1;
    }
    else
    {
        wmrWork.sensorData.UV = (int)(ptr[0] & 0x0F);
    }
}

static int IsFFFFPacketStart (uint8_t* value)
{
    if (value[0] == 0xFF && value[1] == 0xFF)
        return TRUE;
    else
        return FALSE;
}

static int IsD0PacketStart (uint8_t value)
{
    if (0xD2 <= value && value <= 0xD7)
        return TRUE;
    if (value == 0xD9)
        return TRUE;

    return FALSE;
}

static int IsPacketStart (uint8_t* pValue)
{
    if (wmrWork.protocol == WMR_PROTOCOL_FFFF)
    {
        return IsFFFFPacketStart(pValue);
    }
    else
    {
        return IsD0PacketStart(*pValue);
    }
}

static int getFFFFPktLength(uint8_t* bfr, int maxLen)
{
    uint8_t*    tempPtr = bfr;
    int         RetVal;
    int         LoopMax = ((maxLen <= 64) ? maxLen : 64);

    // Skip first two FF's.
    for (RetVal = 2; RetVal < LoopMax-1; RetVal ++)
    {
        if ((tempPtr[RetVal] == 0xFF) && (tempPtr[RetVal+1] == 0xFF))
        {
            return RetVal;
        }
    }

    // If here no end of packet found.
    return ((maxLen <= 64) ? 0 : -1);
}

static int checkD0PktLength(uint8_t type, uint8_t length)
{
    int     RetVal = TRUE;

    switch ((int)type)
    {
    case WMR_D0_HISTORY:
        if (((int)length < 49) || ((int)length > 112))
            RetVal = FALSE;
        break;
    case WMR_D0_RAIN:
        if ((int)length != 22)
            RetVal = FALSE;
        break;
    case WMR_D0_TEMP:
        if ((int)length != 16)
            RetVal = FALSE;
        break;
    case WMR_D0_PRESSURE:
        if ((int)length != 13)
            RetVal = FALSE;
        break;
    case WMR_D0_WIND:
        if ((int)length != 16)
            RetVal = FALSE;
        break;
    case WMR_D0_STATUS:
        if ((int)length != 8)
            RetVal = FALSE;
        break;
    case WMR_D0_UV:
        if ((int)length != 10)
            RetVal = FALSE;
        break;
    default:
        break;
    }

    return RetVal;
}

static void shiftUpReadBuffer(int numToShift)
{
    int     i;

    if (numToShift > wmrWork.readIndex)
    {
        numToShift = wmrWork.readIndex;
    }

    for (i = 0; (i + numToShift) < wmrWork.readIndex; i ++)
    {
        wmrWork.readData[i] = wmrWork.readData[i + numToShift];
    }

    wmrWork.readIndex -= numToShift;
}

static int parseStationData(int length)
{
    uint8_t   *ptr = &wmrWork.readData[0];

#ifdef WMR_DEBUG
    radMsgLog(PRI_MEDIUM, "WMRDBG: parse packet");
    radMsgLogData(&wmrWork.readData[0], length);
#endif
    if (wmrWork.protocol == WMR_PROTOCOL_FFFF)
    {
        // Check checksum:
        if (!checksum(ptr+2, length-2))
        {
            // Bad checksum:
#ifdef WMR_DEBUG
            radMsgLog(PRI_MEDIUM, "RX packet checksum error: discarding %d bytes", length);
            radMsgLogData(&wmrWork.readData[0], length);
#endif
            return ERROR;
        }

        switch ((int)ptr[3])
        {
        case WMR_FFFF_RAIN:
            decodeRain(ptr+4);
            break;
        case WMR_FFFF_TEMP:
            decodeTemp(ptr+4);
            break;
        case WMR_FFFF_TEMPONLY:
            decodeTempOnly(ptr+4);
            break;
        case WMR_FFFF_PRESSURE:
            decodePressure(ptr+4);
            break;
        case WMR_FFFF_WIND:
            decodeWind(ptr+4);
            break;
        case WMR_FFFF_UV:
            decodeUV(ptr+5);
            break;
        default:
#ifdef WMR_COUNT_BYTES
            UnknownPacketType ++;
#endif
            break;
        }
    }
    else
    {
        // Check checksum:
        if (!checksum(ptr, length))
        {
            // Bad checksum:
            radMsgLog(PRI_MEDIUM, "packet checksum error");
            radMsgLogData(&wmrWork.readData[0], length);
            return ERROR;
        }

        switch ((int)ptr[0])
        {
        case WMR_D0_RAIN:
            decodeRain(ptr+7);
            break;
        case WMR_D0_TEMP:
            decodeTemp(ptr+7);
            break;
        case WMR_D0_PRESSURE:
            decodePressure(ptr+7);
            break;
        case WMR_D0_WIND:
            decodeWind(ptr+7);
            break;
        case WMR_D0_STATUS:
            //decodeStatus(ptr+7);
            break;
        case WMR_D0_UV:
            decodeUV(ptr+7);
            break;
        default:
#ifdef WMR_COUNT_BYTES
            UnknownPacketType ++;
#endif
            break;
        }
    }

    return OK;
}

static void storeLoopPkt (WVIEWD_WORK *work, LOOP_PKT *dest, WMR_DATA *src)
{
    float               tempfloat;
    WMR_IF_DATA*        ifWorkData = (WMR_IF_DATA*)work->stationData;
    time_t              nowTime = time(NULL);
    int                 i;

    // Clear optional data:
    stationClearLoopData(work);

    if ((10 < src->pressure && src->pressure < 50) &&
            (-150 < src->temp[WMR_TEMP_SENSOR_OUT] && src->temp[WMR_TEMP_SENSOR_OUT] < 150))
    {
        // wmr has Station Pressure:
        dest->stationPressure  = src->pressure;

        // Apply calibration here so the computed values reflect it:
        dest->stationPressure *= work->calMPressure;
        dest->stationPressure += work->calCPressure;

        // compute sea-level pressure (BP)
        tempfloat = wvutilsConvertSPToSLP(dest->stationPressure,
                                          src->temp[WMR_TEMP_SENSOR_OUT],
                                          (float)ifWorkData->elevation);
        dest->barometer                     = tempfloat;

        // calculate altimeter
        tempfloat = wvutilsConvertSPToAltimeter(dest->stationPressure,
                                                (float)ifWorkData->elevation);
        dest->altimeter                     = tempfloat;
    }

    if (-150 < src->temp[WMR_TEMP_SENSOR_OUT] &&
            src->temp[WMR_TEMP_SENSOR_OUT] < 150)
    {
        dest->outTemp  = src->temp[WMR_TEMP_SENSOR_OUT];
    }

    if (0 <= src->humidity[WMR_TEMP_SENSOR_OUT] &&
            src->humidity[WMR_TEMP_SENSOR_OUT] <= 100)
    {
        tempfloat = src->humidity[WMR_TEMP_SENSOR_OUT];
        tempfloat += 0.5;
        dest->outHumidity  = (uint16_t)tempfloat;
    }

    if (0 <= src->windAvgSpeed && src->windAvgSpeed <= 250)
    {
        tempfloat = src->windAvgSpeed;
        dest->windSpeedF     = tempfloat;
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
        if (dest->windGustF < dest->windSpeedF)
        {
            dest->windGustF = dest->windSpeedF;
        }
    }

    if (0 <= src->rainAccum)
    {
        if (ifWorkData->totalRain < 0)
        {
            // just starting, so start with whatever the station reports:
            radMsgLog(PRI_STATUS, "storeLoopPacket: initializing rain accumulator...");
            ifWorkData->totalRain = src->rainAccum;
            dest->sampleRain = 0;
        }
        else
        {
            // process the rain accumulator; check for crazy values:
            if ((src->rainAccum - ifWorkData->totalRain) >= 0)
            {
                if ((src->rainAccum - ifWorkData->totalRain) < 2)
                {
                    dest->sampleRain = src->rainAccum - ifWorkData->totalRain;
                    ifWorkData->totalRain = src->rainAccum;
                }
                else
                {
                    // Value too large, ignore it.
                    dest->sampleRain = 0;
                }
            }
            else
            {
                // we had a counter reset...
                dest->sampleRain = src->rainAccum;
                ifWorkData->totalRain = src->rainAccum;
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
        dest->rainRate   *= (60/WMR_RAIN_RATE_PERIOD);
    }
    else
    {
        dest->sampleRain = 0;
    }

    dest->inTemp                        = src->temp[WMR_TEMP_SENSOR_IN];
    tempfloat                           = src->humidity[WMR_TEMP_SENSOR_IN];
    tempfloat += 0.5;
    dest->inHumidity                    = (uint16_t)tempfloat;

    dest->UV                            = src->UV;

    // Do the extras:
    for (i = 0; i < WMR_TEMP_SENSOR_COUNT - 2; i ++)
    {
        dest->extraTemp[i]      = src->temp[i+2];
        dest->extraHumidity[i]  = src->humidity[i+2];
    }

    return;
}

static int sendAck(WVIEWD_WORK *work)
{
    unsigned char       buf[32];

    memcpy(buf, "\x00\x00\x00\x00\x00\x00\x00\x00", 8);
    (*(work->medium.usbhidWrite))(&work->medium, buf, 0x08);
    (*(work->medium.usbhidWrite))(&work->medium, buf, 0x08);

    return OK;
}

static int sendHeartbeat(WVIEWD_WORK *work)
{
    unsigned char       buf[32];

    // send the heartbeat message so the console will stream live data:
    memcpy(buf, "\x07\xd0\x00\x00\x00\x00\x00\x00", 8);
    (*(work->medium.usbhidWrite))(&work->medium, buf, 0x08);
    (*(work->medium.usbhidWrite))(&work->medium, buf, 0x08);
    memcpy(buf, "\x01\x00\x00\x00\x00\x00\x00\x00", 8);
    (*(work->medium.usbhidWrite))(&work->medium, buf, 0x08);
    (*(work->medium.usbhidWrite))(&work->medium, buf, 0x08);

    return OK;
}

static int sendReset(WVIEWD_WORK *work)
{
    unsigned char       buf[32];

    radMsgLog (PRI_MEDIUM, "wmr: Sending reset to console...");

    // Send a reset:
    memcpy(buf, "\x20\x00\x08\x01\x08\x00\x00\x00", 8);
    (*(work->medium.usbhidWrite))(&work->medium, buf, 0x08);
    (*(work->medium.usbhidWrite))(&work->medium, buf, 0x08);

    radUtilsSleep(100);

    // send the heartbeat message so the console will stream live data:
    sendHeartbeat(work);

    return OK;
}

// Read raw USB data and buffer it for later processing:
// Only used before the reader thread has been started.
static void readDataDirect (WVIEWD_WORK *work)
{
    int     retVal, length;
    uint8_t buf[8];

    if ((radTimeGetSECSinceEpoch() - wmrWork.lastDataRX) >= 60)
    {
        // It has been too long since the last valid data packet was received,
        // send a RESET:
        sendReset(work);
        wmrWork.lastDataRX = radTimeGetSECSinceEpoch();
    }
    else if ((radTimeGetSECSinceEpoch() - wmrWork.heartBeatCounter) >= WMR_HEARTBEAT_INTERVAL)
    {
        // send the heartbeat message so the console will keep streaming live data:
        sendHeartbeat(work);
        wmrWork.heartBeatCounter = radTimeGetSECSinceEpoch();
        radUtilsSleep(10);
    }

    // Read on the USB interface:
    while (wmrWork.readIndex < WMR_BUFFER_LENGTH)
    {
        retVal = (*(work->medium.usbhidRead))(&work->medium, buf, 8, WMR_READ_WAIT);
        if (retVal == 8)
        {
            sendAck(work);
            // first octet is a length field:
            length = buf[0];
            if ((length < 8) && ((wmrWork.readIndex + length) < WMR_BUFFER_LENGTH))
            {
                memcpy(&wmrWork.readData[wmrWork.readIndex], buf+1, length);
                wmrWork.readIndex += length;
#ifdef WMR_DEBUG
                radMsgLog(PRI_MEDIUM, "WMRDBG: USB RX %d", length);
                radMsgLogData(&wmrWork.readData[0], wmrWork.readIndex);
#endif
            }

            wmrWork.lastDataRX = radTimeGetSECSinceEpoch();
        }
        else if (retVal == ERROR)
        {
            radMsgLog (PRI_HIGH, "readDataDirect: read error...");
            return;
        }
        else
        {
            // Try again later:
            return;
        }
    }

    return;
}

// Dynamically figure out if we are talking to a D0-D9 (WMR200) station or
// a FFFF 00XX (WMR88A/WMR100N) station so we know how to decode the packets:
static int detectStationProtocol (WVIEWD_WORK *work)
{
    int                 index;

    radMsgLog(PRI_MEDIUM, "wmrInit: Auto-detecting protocol...");

    wmrWork.readIndex = 0;

    // Read on the USB interface for a while:
    while (wmrWork.readIndex < 32)
    {
        readDataDirect(work);
        radUtilsSleep(250);
    }

    // OK, now we have some data to examine:
    // First look for FFFF bytes, this is definitive (?):
    wmrWork.protocol = WMR_PROTOCOL_UNKNOWN;
    for (index = 0; index < 31; index ++)
    {
        if (IsFFFFPacketStart(&wmrWork.readData[index]))
        {
            wmrWork.protocol = WMR_PROTOCOL_FFFF;
            radMsgLog (PRI_MEDIUM, "wmrInit: found old FFFF framed protocol");
            break;
        }
    }

    if (wmrWork.protocol == WMR_PROTOCOL_UNKNOWN)
    {
        // Assume D0:
        wmrWork.protocol = WMR_PROTOCOL_D0;
        radMsgLog (PRI_MEDIUM, "wmrInit: found D0-D9 framed protocol");
    }

    // Initialize the data RX time:
    wmrWork.lastDataRX = radTimeGetSECSinceEpoch();

    return OK;
}

// The reader thread:
static void ReaderThread(RAD_THREAD_ID threadId, void* threadData)
{
    int                 retVal, length, RestartNeeded;
    WMRUSB_MSG_DATA     msg;
    uint8_t             buf[8];
    WVIEWD_WORK*        work = (WVIEWD_WORK*)threadData;

    radMsgLog (PRI_STATUS, "wmr: read thread started...");

#ifdef WMR_COUNT_BYTES
    UsbRawBytes = StreamBytes = PacketBytes = ChecksumBytes = StatCount = 0;
#endif

    // Recreate medium blocking:
    if (usbhidMediumInit (&work->medium, WMR_VENDOR_ID, WMR_PRODUCT_ID, FALSE, TRUE) == ERROR)
    {
        radMsgLog (PRI_HIGH, "ReaderThread: USB MediumInit failed");
        return;
    }

    // Main loop:
    while (!radthreadShouldExit(threadId))
    {
        // Open the interface.
        if ((*(work->medium.usbhidInit))(&work->medium) != OK)
        {
            radMsgLog (PRI_HIGH, "wmr: read thread failed to open HID device!");
            radUtilsSleep(WMR_REESTABLISH_SLEEP);
            continue;
        }

        // Read on the USB interface:
        RestartNeeded = FALSE;
        msg.length = 0;
        while (!RestartNeeded && !radthreadShouldExit(threadId))
        {
            if ((radTimeGetSECSinceEpoch() - wmrWork.lastDataRX) >= 60)
            {
                // It has been too long since the last valid data packet was received,
                // send a RESET:
                sendReset(work);

                // Restart to bump the HID interface.
                RestartNeeded = TRUE;

                wmrWork.lastDataRX = radTimeGetSECSinceEpoch();
            }
            else if ((radTimeGetSECSinceEpoch() - wmrWork.heartBeatCounter) >= WMR_HEARTBEAT_INTERVAL)
            {
                // send the heartbeat message so the console will keep streaming live data:
                sendHeartbeat(work);
                wmrWork.heartBeatCounter = radTimeGetSECSinceEpoch();
                radUtilsSleep(10);
            }
            else
            {
                // Read on the interface:
                // We are using blocking IO.
                retVal = (*(work->medium.usbhidRead))(&work->medium, buf, 8, 0);
                if (retVal == 8)
                {
                    sendAck(work);

                    // first octet is a length field:
                    length = buf[0];
                    if (length < 8)
                    {
#ifdef WMR_COUNT_BYTES
                        UsbRawBytes += length;
#ifdef WMR_DUMP_RAW_USB
                        radMsgLog(PRI_HIGH, "USBRAW RX: %d: %2.2X%2.2X%2.2X%2.2X%2.2X%2.2X%2.2X",
                                  (int)buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7]);
#endif
#endif
                        memcpy(&msg.data[msg.length], buf+1, length);
                        msg.length += length;

                        if (msg.length > 8)
                        {
                            // Send to our consumer:
                            radMsgRouterMessageSend(WVIEW_MSG_TYPE_STATION_DATA, &msg, sizeof(msg));
                            msg.length = 0;
                        }
                    }

                    wmrWork.lastDataRX = radTimeGetSECSinceEpoch();
                }
                else if (retVal == ERROR)
                {
                    radMsgLog (PRI_HIGH, "wmr: read error...");
                    RestartNeeded = TRUE;
                }
            }
        }

        // Close the interface.
        (*(work->medium.usbhidExit))(&work->medium);
    }

    radMsgLog (PRI_STATUS, "wmr: read thread exiting...");
    return;
}

////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////  A P I  /////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

int wmrInit (WVIEWD_WORK *work)
{
    WMR_IF_DATA*        ifWorkData = (WMR_IF_DATA*)work->stationData;
    unsigned char       buf[32];
    char                outString[128];
    int                 i, length, printCounter;
    time_t              nowTime = time(NULL) - (WV_SECONDS_IN_HOUR/(60/WMR_RAIN_RATE_PERIOD));
    ARCHIVE_PKT         recordStore;

    memset (&wmrWork, 0, sizeof(wmrWork));
    wmrWork.sensorData.rainAccum = -1;
    ifWorkData->totalRain = -1;

    // Create the rain accumulator (WMR_RAIN_RATE_PERIOD minute age)
    // so we can compute rain rate:
    ifWorkData->rainRateAccumulator = sensorAccumInit(WMR_RAIN_RATE_PERIOD);

    // Populate the accumulator with the last WMR_RAIN_RATE_PERIOD minutes:
    while ((nowTime = dbsqliteArchiveGetNextRecord(nowTime, &recordStore)) != ERROR)
    {
        sensorAccumAddSample(ifWorkData->rainRateAccumulator,
                             recordStore.dateTime,
                             recordStore.value[DATA_INDEX_rain]);
    }

    // Open non-blocking:
    if (usbhidMediumInit (&work->medium, WMR_VENDOR_ID, WMR_PRODUCT_ID, FALSE, FALSE) == ERROR)
    {
        radMsgLog (PRI_HIGH, "stationInit: USB MediumInit failed");
        return ERROR;
    }

    // Initialize the USB interface:
    if ((*(work->medium.usbhidInit))(&work->medium) != OK)
    {
        return ERROR;
    }

    // Send a reset:
    sendReset(work);

    // Toss any previously received data:
    wmrWork.readIndex = 0;

    radUtilsSleep(10);

    // Initialize the data RX time:
    wmrWork.lastDataRX = radTimeGetSECSinceEpoch();

    // Autodetect FFFF or D0-DF framing:
    detectStationProtocol(work);

    // Get initial readings:
    radMsgLog (PRI_MEDIUM,
               "wmrInit: waiting for first sensor packets (this may take some time):");
    radMsgLog (PRI_MEDIUM,
               "wview requires one packet from each sensor suite (except rain) before it can complete initialization.");
    radMsgLog (PRI_MEDIUM,
               "If one of your sensors is out of range or malfunctioning, wview will not complete initialization.");
    printCounter = 10;
    while ((wmrWork.dataRXMask < WMR_SENSOR_ALL) && (!work->exiting))
    {
        if (++printCounter >= 10)
        {
            // Log what we are waiting for:
            length = 0;
            for (i = 0; i < 4; i ++)
            {
                if (!(wmrWork.dataRXMask & (1 << i)))
                {
                    length += sprintf(&outString[length], "%s ", WMRSensorNames[i]);
                }
            }
            radMsgLog (PRI_MEDIUM, "wmrInit: waiting for sensors: %s", outString);
            printCounter = 0;
        }

        readDataDirect(work);
        wmrProcessData(work);
        radUtilsSleep(WMR_PROCESS_TIME_INTERVAL);
    }

    wmrWork.dataRXMask = 0;

    // Close the USB interface. The reader thread will reopen it.
    (*(work->medium.usbhidExit))(&work->medium);

    if (!work->exiting)
    {
        radMsgLog (PRI_MEDIUM, "wmrInit: first sensor packets received.");
    }

    // Reset the USB buffer.
    wmrWork.readIndex = 0;

    // Create the USB reader thread:
    work->threadId = radthreadCreate(ReaderThread, work);
    if (work->threadId == NULL)
    {
        radMsgLog (PRI_HIGH, "wmrInit: radthreadCreate failed!");
        return ERROR;
    }

    wmrWork.started = TRUE;

    // Start our IF timer:
    radProcessTimerStart (work->ifTimer, WMR_PROCESS_TIME_INTERVAL);

    // populate the LOOP structure:
    ifWorkData->wmrReadings = wmrWork.sensorData;
    storeLoopPkt (work, &work->loopPkt, &ifWorkData->wmrReadings);

    // we must indicate successful completion here -
    // even though we are synchronous, the daemon wants to see this event
    radProcessEventsSend (NULL, STATION_INIT_COMPLETE_EVENT, 0);

    return OK;
}

void wmrExit (WVIEWD_WORK *work)
{
    radthreadWaitExit(work->threadId);
    radUtilsSleep(100);
    radMsgLog (PRI_MEDIUM, "wmrExit: read thread stopped.");
    return;
}

// Read raw USB data and buffer it for later processing:
void wmrReadData (WVIEWD_WORK *work, WMRUSB_MSG_DATA* msg)
{
    if ((wmrWork.readIndex + msg->length) < WMR_BUFFER_LENGTH)
    {
        memcpy(&wmrWork.readData[wmrWork.readIndex], msg->data, msg->length);
        wmrWork.readIndex += msg->length;

#ifdef WMR_DEBUG
        radMsgLog(PRI_MEDIUM, "WMRDBG: USB RX");
        radMsgLogData(&wmrWork.readData[0], wmrWork.readIndex);
#endif
#ifdef WMR_COUNT_BYTES
        StreamBytes += msg->length;
#endif
    }

    return;
}

// Enforce packet framing and pass to parse engine if a packet frame is complete:
int wmrProcessData (WVIEWD_WORK *work)
{
    int     pktLength, rval, index = 0;
    int     retVal = FALSE;

    // First, hunt for a packet start sequence (0xFFFF or 0xD2-0xD9):
    while ((index < wmrWork.readIndex - 1) && !IsPacketStart(&wmrWork.readData[index]))
    {
        index ++;
    }

    // Do we need to toss junk at the front of the buffer?
    if (index > 0)
    {

#ifdef WMR_DEBUG
        radMsgLog(PRI_MEDIUM, "WMRDBG: Frame: Tossing %d bytes", index);
        radMsgLogData(&wmrWork.readData[0], index);
#endif

        // Lose the rubbish:
        shiftUpReadBuffer(index);
    }

    if (wmrWork.protocol == WMR_PROTOCOL_FFFF)
    {
        // Do we have a packet start and type?
        if (wmrWork.readIndex >= 4)
        {
            // Get the packet length and see if we have a complete packet:
            pktLength = getFFFFPktLength(wmrWork.readData, wmrWork.readIndex);

            if (pktLength > 4)
            {
                // We have a completion, process it:
                rval = parseStationData(pktLength);
                retVal = TRUE;
                rval = rval;

#ifdef WMR_COUNT_BYTES
                if (rval != ERROR)
                {
                    PacketBytes += pktLength;
                    radMsgLog(PRI_MEDIUM, "PKT RX: %d", pktLength);
                }
                else
                {
                    ChecksumBytes += pktLength;
                }
#endif

                // Delete it:
                shiftUpReadBuffer(pktLength);
            }
            else if (pktLength != 0)
            {
                // Shift up two, something's amiss.
                shiftUpReadBuffer(2);
            }
        }
    }
    else  // wmrWork.protocol == WMR_PROTOCOL_D0
    {
        // Do we have a packet start and length?
        if (wmrWork.readIndex >= 2)
        {
            // Get the packet length and see if we have a complete packet:
            pktLength = wmrWork.readData[1];

            if (pktLength < 2)
            {
                // Invalid:
                shiftUpReadBuffer(1);
            }
            // Sanity check the length:
            else if (!checkD0PktLength(wmrWork.readData[0], wmrWork.readData[1]))
            {
                // Invalid:
                shiftUpReadBuffer(1);
            }
            else if (pktLength <= wmrWork.readIndex)
            {
                // We have a completion, process it:
                parseStationData(pktLength);
                retVal = TRUE;

#ifdef WMR_COUNT_BYTES
PacketBytes += pktLength;
#endif

                // Delete it:
                shiftUpReadBuffer(pktLength);
            }
        }
    }

    return retVal;
}

void wmrGetReadings (WVIEWD_WORK *work)
{
    WMR_IF_DATA*  ifWorkData = (WMR_IF_DATA*)work->stationData;

    // populate the LOOP structure:
    if (wmrWork.dataRXMask != 0)
    {
        ifWorkData->wmrReadings = wmrWork.sensorData;
        storeLoopPkt (work, &work->loopPkt, &ifWorkData->wmrReadings);
        wmrWork.dataRXMask = 0;

        // indicate the LOOP packet is done
        radProcessEventsSend (NULL, STATION_LOOP_COMPLETE_EVENT, 0);
    }
}

