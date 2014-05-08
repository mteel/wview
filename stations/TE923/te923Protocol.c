/*---------------------------------------------------------------------------
 
  FILENAME:
        te923Protocol.c
 
  PURPOSE:
        Provide protocol utilities for TE923 station communication.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        03/03/2011      M.S. Teel       0               Original
 
  NOTES:
        Parts of this implementation were inspired by the te923con project
        (C) Sebastian John (http://te923.fukz.org).
 
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
#include <te923Protocol.h>

/*  ... global memory declarations
*/

/*  ... local memory
*/

static TE923_WORK           te923Work;
static WV_ACCUM_ID          te923_12HourTempAvg;
static time_t               lastAvgTime;


// Local methods:

static uint8_t bcdDecode(uint8_t byte)
{
    return ((int)((byte & 0xF0) >> 4) * 10 + (int)(byte & 0x0F));
}

static int readBlock (WVIEWD_WORK *work, int offset, uint8_t* buffer)
{
    // Read data at offset 'offset':
    // After sending the read command, the device will send back 32 bytes data within 100ms.
    // If not, then it means the command has not been received correctly.

    char     rqstBuffer[8];
    uint8_t  crc;
    int      i, retVal, bytes, count = 0;

    rqstBuffer[0] = 0x05;
    rqstBuffer[1] = 0xAF;
    rqstBuffer[4] = offset/0x10000;
    rqstBuffer[3] = (offset - (rqstBuffer[4] * 0x10000))/0x100;
    rqstBuffer[2] = offset - (rqstBuffer[4] * 0x10000) - (rqstBuffer[3] * 0x100);
    rqstBuffer[5] = (rqstBuffer[1] ^ rqstBuffer[2] ^ rqstBuffer[3] ^ rqstBuffer[4]);
    rqstBuffer[6] = 0xAF;
    rqstBuffer[7] = 0xFE;

    // Request read of 33-byte chunk from offset:
    retVal = (*(work->medium.usbhidWrite))(&work->medium, rqstBuffer, 8);
    if (retVal != 8)
    {
        radMsgLog (PRI_HIGH, "TE923: write data request failed!");
        return ERROR;
    }

    radUtilsSleep(100);

    count = (*(work->medium.usbhidReadSpecial))(&work->medium, buffer, TE923_BUFLEN - 1, 5000);

    // Short buffer?
    if (count < (TE923_BUFLEN - 1))
    {
        return -2;          // Short buffer RX
    }

    // Check the CRC:
    crc = 0x00;
    for (i = 0; i <= 32; i ++)
    {
        crc = crc ^ buffer[i];
    }

    if (crc != buffer[33])
    {
        return -3;          // Bad CRC
    }

    // Check leading byte:
    if (buffer[0] != 0x5a)
    {
        return -4;          // Bad data
    }

    return OK;
}

static int readStationData (WVIEWD_WORK *work)
{
    TE923_DATA*     sensors = &te923Work.sensorData;
    int             ret, i, offset, isInvalid;
    float*          pTemp;
    int             adr = 0x020001;
    uint8_t         buf[TE923_BUFLEN];

    if ((*(work->medium.usbhidInit))(&work->medium) != OK)
    {
        return ERROR;
    }

    for (i = 0; i < TE923_MAX_RETRIES; i ++)
    {
        if (readBlock(work, adr, buf) == OK)
        {
            break;
        }
        else
        {
            radUtilsSleep(250);
        }
    }

    (*(work->medium.usbhidExit))(&work->medium);

    if (i == TE923_MAX_RETRIES)
    {
        return ERROR;
    }

    // Loose the header byte:
    memmove(buf, buf + 1, TE923_BUFLEN-1);

    // decode temperature and humidity from all sensors:
    for (i = 0; i <= TE923_MAX_CHANNELS; i ++)
    {
        offset = i * 3;
        isInvalid = FALSE;

        if (bcdDecode(buf[0+offset] & 0x0F) > 9)
        {
            if (i == 0)
            {
                sensors->intemp = ARCHIVE_VALUE_NULL;
            }
            else
            {
                sensors->outtemp[i-1] = ARCHIVE_VALUE_NULL;
            }
            if (((buf[0+offset] & 0x0F) == 0x0C) || ((buf[0+offset] & 0x0F) == 0x0B))
            {
                isInvalid = TRUE;
            }
        }
        else
        {
            if (((buf[1+offset] & 0x40) != 0x40) && i > 0)
            {
                sensors->outtemp[i-1] = ARCHIVE_VALUE_NULL;
                isInvalid = TRUE;
            }
            else
            {
                if (i == 0)
                {
                    pTemp = &sensors->intemp;
                }
                else
                {
                    pTemp = &sensors->outtemp[i-1];
                }
            
                *pTemp = (bcdDecode(buf[0+offset]) / 10.0) + (bcdDecode(buf[1+offset] & 0x0F) * 10.0);
                if ((buf[1+offset] & 0x20) == 0x20)
                {
                    *pTemp += 0.05;
                }
                if ((buf[1+offset] & 0x80) != 0x80)
                {
                    *pTemp *= -1;
                }
                *pTemp = wvutilsConvertCToF(*pTemp);
            }
        }

        if (isInvalid || (bcdDecode(buf[2+offset] & 0x0F) > 9))
        {
            if (i == 0)
            {
                sensors->inhumidity = ARCHIVE_VALUE_NULL;
            }
            else
            {
                sensors->outhumidity[i-1] = ARCHIVE_VALUE_NULL;
            }
        }
        else
        {
            if (i == 0)
            {
                sensors->inhumidity = bcdDecode(buf[2+offset]);
            }
            else
            {
                sensors->outhumidity[i-1] = bcdDecode(buf[2+offset]);
            }
        }
    }

    // decode value from UV sensor:
    if ((buf[18] == 0xAA) && (buf[19] == 0x0A))
    {
        sensors->UV = ARCHIVE_VALUE_NULL;
    }
    else if ((bcdDecode(buf[18]) > 99) || (bcdDecode(buf[19]) > 99))
    {
        sensors->UV = ARCHIVE_VALUE_NULL;
    }
    else
    {
        sensors->UV = bcdDecode(buf[18] & 0x0F) / 10.0 +
                      bcdDecode((buf[18] & 0xF0) >> 4) +
                      bcdDecode(buf[19] & 0x0F) * 10.0;
    }

    // decode pressure:
    if ((buf[21] & 0xF0) == 0xF0)
    {
        sensors->pressure = ARCHIVE_VALUE_NULL;
    }
    else
    {
        sensors->pressure = (float)(buf[21] * 0x100 + buf[20]) * 0.0625;
        sensors->pressure = wvutilsConvertHPAToINHG(sensors->pressure);
    }

    // decode windgust:
    isInvalid = FALSE;
    if ((bcdDecode(buf[25] & 0xF0) > 90) || (bcdDecode(buf[25] & 0x0F) > 9))
    {
        sensors->windGustSpeed = ARCHIVE_VALUE_NULL;
        isInvalid = TRUE;
    }
    else
    {
        offset = 0;
        if ((buf[26] & 0x10) == 0x10)
        {
            offset = 100;
        }
        sensors->windGustSpeed = (bcdDecode(buf[25]) / 10.0) + 
                                 (bcdDecode(buf[26] & 0x0F) * 10.0) + 
                                 offset;
    }

    // decode windspeed:
    if (isInvalid || (bcdDecode( buf[27] & 0xF0) > 90) || (bcdDecode(buf[27] & 0x0F) > 9))
    {
        sensors->windAvgSpeed = ARCHIVE_VALUE_NULL;
        isInvalid = TRUE;
    }
    else
    {
        offset = 0;
        if ((buf[28] & 0x10) == 0x10)
        {
            offset = 100;
        }
        sensors->windAvgSpeed = (bcdDecode(buf[27]) / 10.0) + 
                                (bcdDecode(buf[28] & 0x0F) * 10.0) + 
                                offset;
    }

    // decode wind direction:
    if (isInvalid)
    {
        sensors->windDir = ARCHIVE_VALUE_NULL;
    }
    else
    {
        sensors->windDir = (float)(buf[29] & 0x0F);
        sensors->windDir *= 22.5;
    }

    // decode rain counter:
    sensors->rain = (float)(buf[31] * 0x100 + buf[30]);
    // TE923 manual says resolution == 0.03 inches, not 1 millimeter (0.039").
    // But console is not divisible by 0.03 after a few tips. Each
    // bucket tip is actually about 1/36" or 0.27777777" (0.705556 mm).
    sensors->rain /= 36;

    // Add our outside temp to the 12-hour accumulator:
    if ((sensors->outtemp[0] != ARCHIVE_VALUE_NULL) &&
        ((time(NULL) - lastAvgTime) >= (60*work->archiveInterval)))
    {
        sensorAccumAddSample(te923_12HourTempAvg, time(NULL), sensors->outtemp[0]);
        lastAvgTime = time(NULL);
    }

    return OK;
}

static int readStationStatus(WVIEWD_WORK *work) 
{
    int             i, ret;
    int             adr = 0x00004C;
    uint8_t         buf[TE923_BUFLEN];
    TE923_DATA*     sensors = &te923Work.sensorData;

    if ((*(work->medium.usbhidInit))(&work->medium) != OK)
    {
        return ERROR;
    }

    for (i = 0; i < TE923_MAX_RETRIES; i ++)
    {
        if (readBlock(work, adr, buf) == OK)
        {
            break;
        }
        else
        {
            radUtilsSleep(250);
        }
    }

    (*(work->medium.usbhidExit))(&work->medium);

    if (i == TE923_MAX_RETRIES)
    {
        return ERROR;
    }

    if (buf[0] != 0x5A)
    {
        return ERROR;
    }

    if ((buf[1] & 0x80) == 0x80)
        sensors->statusRain = TRUE;
    else
        sensors->statusRain = FALSE;

    if ((buf[1] & 0x40) == 0x40)
        sensors->statusWind = TRUE;
    else
        sensors->statusWind = FALSE;

    if ((buf[1] & 0x20) == 0x20)
        sensors->statusUV = TRUE;
    else
        sensors->statusUV = FALSE;

    if ((buf[1] & 0x10) == 0x10)
        sensors->statusSensor[4] = TRUE;
    else
        sensors->statusSensor[4] = FALSE;

    if ((buf[1] & 0x08) == 0x08)
        sensors->statusSensor[3] = TRUE;
    else
        sensors->statusSensor[3] = FALSE;

    if ((buf[1] & 0x04) == 0x04)
        sensors->statusSensor[2] = TRUE;
    else
        sensors->statusSensor[2] = FALSE;

    if ((buf[1] & 0x02) == 0x02)
        sensors->statusSensor[1] = TRUE;
    else
        sensors->statusSensor[1] = FALSE;

    if ((buf[1] & 0x01) == 0x01)
        sensors->statusSensor[0] = TRUE;
    else
        sensors->statusSensor[0] = FALSE;

    return OK;
}

//  Calculate the "e ^ (-mgh/RT)" term for pressure conversions:
static double calcPressureTerm(float tempF, float elevationFT)
{
    double      exponent;
    double      elevMeters = (double)wvutilsConvertFeetToMeters(elevationFT);
    double      tempKelvin = (double)wvutilsConvertFToC(tempF) + 273.15;

    // e ^ -elevMeters/(tempK * 29.263)
    exponent = (-elevMeters);

    // degrees Kelvin (T)
    exponent /= (tempKelvin * 29.263);

    // e ^ (-mgh/RT)
    exponent = exp(exponent);

    return exponent;
}

//  calculate station pressure from sea level pressure (using 12-hour temp avg):
static float te923ConvertSLPToSP(float SLP, float tempF, float elevationFT)
{
    double      SP, PT;

    // Formula used: SP = SLP * PressureTerm
    // compute PressureTerm:
    PT = calcPressureTerm (tempF, elevationFT);
    SP = SLP * PT;

    return (float)SP;
}

static void storeLoopPkt (WVIEWD_WORK *work, LOOP_PKT *dest, TE923_DATA *src)
{
    float               tempfloat;
    TE923_IF_DATA*      ifWorkData = (TE923_IF_DATA*)work->stationData;
    time_t              nowTime = time(NULL);
    int                 i;

    // Clear optional data:
    stationClearLoopData(work);

    if ((10 < src->pressure && src->pressure < 50) &&
            (-150 < src->outtemp[0] && src->outtemp[0] < 150))
    {
        // TE923 produces sea level pressure (SLP):
        dest->barometer = src->pressure;

        // Apply calibration here so the computed values reflect it:
        dest->barometer *= work->calMBarometer;
        dest->barometer += work->calCBarometer;

        // calculate station pressure:
        dest->stationPressure = te923ConvertSLPToSP(dest->barometer, 
                                                    sensorAccumGetAverage(te923_12HourTempAvg), 
                                                    (float)ifWorkData->elevation);

        // now calculate altimeter:
        dest->altimeter = wvutilsConvertSPToAltimeter(dest->stationPressure,
                                                      (float)ifWorkData->elevation);
    }

    if (-150 < src->outtemp[0] && src->outtemp[0] < 150)
    {
        dest->outTemp  = src->outtemp[0];
    }

    if (0 <= src->outhumidity[0] && src->outhumidity[0] <= 100)
    {
        tempfloat = src->outhumidity[0];
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
        dest->rainRate   *= (60/TE923_RAIN_RATE_PERIOD);
    }
    else
    {
        dest->sampleRain = 0;
        sensorAccumAddSample (ifWorkData->rainRateAccumulator, nowTime, dest->sampleRain);
        dest->rainRate    = sensorAccumGetTotal (ifWorkData->rainRateAccumulator);
        dest->rainRate   *= (60/TE923_RAIN_RATE_PERIOD);
    }

    dest->inTemp                        = src->intemp;
    tempfloat = src->inhumidity;
    tempfloat += 0.5;
    dest->inHumidity                    = (uint16_t)tempfloat;
    dest->UV                            = src->UV;

    // Do the extras:
    for (i = 1; i < TE923_MAX_CHANNELS; i ++)
    {
        dest->extraTemp[i-1]                = src->outtemp[i];
        dest->extraHumidity[i-1]            = src->outhumidity[i];
        dest->extraTempBatteryStatus[i-1]   = src->statusSensor[i];
    }

    dest->uvBatteryStatus               = src->statusUV;
    dest->windBatteryStatus             = src->statusWind;
    dest->rainBatteryStatus             = src->statusRain;

    return;
}


////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////  A P I  /////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

int te923Init (WVIEWD_WORK *work)
{
    TE923_IF_DATA*     ifWorkData = (TE923_IF_DATA*)work->stationData;
    fd_set              rfds;
    struct timeval      tv;
    int                 ret;
    time_t              nowTime = time(NULL) - (WV_SECONDS_IN_HOUR/(60/TE923_RAIN_RATE_PERIOD));
    ARCHIVE_PKT         recordStore;

    memset (&te923Work, 0, sizeof(te923Work));

    // Create the rain accumulator (TE923_RAIN_RATE_PERIOD minute age)
    // so we can compute rain rate:
    ifWorkData->rainRateAccumulator = sensorAccumInit(TE923_RAIN_RATE_PERIOD);

    // Populate the accumulator with the last TE923_RAIN_RATE_PERIOD minutes:
    while ((nowTime = dbsqliteArchiveGetNextRecord(nowTime, &recordStore)) != ERROR)
    {
        sensorAccumAddSample(ifWorkData->rainRateAccumulator,
                             recordStore.dateTime,
                             recordStore.value[DATA_INDEX_rain]);
    }

    // Initialize the 12-hour temp accumulator (before the first loop cycle):
    te923_12HourTempAvg = sensorAccumInit(60 * 12);

    // populate the LOOP structure:
    if ((readStationData(work) == ERROR) || (readStationStatus(work) == ERROR))
    {
        radMsgLog (PRI_HIGH, "Failed to read initial station data:");
        radMsgLog (PRI_HIGH, "Is your station receiving all sensors on the console?");
        radMsgLog (PRI_HIGH, "If not, you may need to relocate the sensors or the console.");
        return ERROR;
    }

    ifWorkData->te923Readings = te923Work.sensorData;

    // Load data for the last 12 hours:
    nowTime = time(NULL) - (WV_SECONDS_IN_HOUR * 12);
    while ((nowTime = dbsqliteArchiveGetNextRecord(nowTime, &recordStore)) != ERROR)
    {
        sensorAccumAddSample(te923_12HourTempAvg, 
                             recordStore.dateTime, 
                             recordStore.value[DATA_INDEX_outTemp]);
    }

    storeLoopPkt(work, &work->loopPkt, &ifWorkData->te923Readings);

    // we must indicate successful completion here -
    // even though we are synchronous, the daemon wants to see this event
    radProcessEventsSend (NULL, STATION_INIT_COMPLETE_EVENT, 0);

    return OK;
}

void te923Exit (WVIEWD_WORK *work)
{
    return;
}

void te923GetReadings (WVIEWD_WORK *work)
{
    TE923_IF_DATA*  ifWorkData = (TE923_IF_DATA*)work->stationData;

    if ((readStationData(work) == OK) && (readStationStatus(work) == OK))
    {
        // populate the LOOP structure:
        ifWorkData->te923Readings = te923Work.sensorData;
        storeLoopPkt (work, &work->loopPkt, &ifWorkData->te923Readings);

        // indicate the LOOP packet is done
        radProcessEventsSend (NULL, STATION_LOOP_COMPLETE_EVENT, 0);
    }
}

