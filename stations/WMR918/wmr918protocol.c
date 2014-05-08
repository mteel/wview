/*---------------------------------------------------------------------------
 
  FILENAME:
        wmr918protocol.c
 
  PURPOSE:
        Provide protocol utilities for WMR918 station communication.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        02/27/2008      M.S. Teel       0               Original
 
  NOTES:

  LICENSE:
        Copyright (c) 2008, Mark S. Teel (mark@teel.ws)
  
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
#include <wmr918protocol.h>

/*  ... global memory declarations
*/

/*  ... local memory
*/

static WMR918_WORK          wmr918Work;

static int                  wmr918GroupLength[16] =
{
    8, 13, 6, 6, 4, 10, 11, 0, 0, 0, 0, 0, 0, 0, 2, 6
};

static char*                WMRSensorNames[4] =
{
    "Wind",
    "Rain",
    "OutTemp",
    "InTemp"
};

static int readStationData (WVIEWD_WORK *work)
{
    int                 i, retVal, groupType, checkSum, channel;
    float               tempFloat;
    WMR918_IF_DATA*     ifWorkData = (WMR918_IF_DATA*)work->stationData;
    uint8_t             *pPacket = &wmr918Work.readData[2];

    // read the first three bytes -- 0xff, 0xff, <type>
    retVal = (*work->medium.read) (&work->medium, &wmr918Work.readData[0], 3, WMR918_READ_TIMEOUT);
    if (retVal != 3)
    {
        radMsgLog (PRI_MEDIUM, "readStationData: read header failed: %s", 
                   strerror(errno));
        emailAlertSend(ALERT_TYPE_STATION_READ);
        return ERROR;
    }

    while ((wmr918Work.readData[0] != 0xff) || 
           (wmr918Work.readData[1] != 0xff) || 
           (wmr918Work.readData[2] > 15))
    {
        wmr918Work.readData[0] = wmr918Work.readData[1];
        wmr918Work.readData[1] = wmr918Work.readData[2];
        retVal = (*work->medium.read) (&work->medium, &wmr918Work.readData[2], 1, WMR918_READ_TIMEOUT);
        if (retVal != 1)
        {
            radMsgLog (PRI_MEDIUM, "readStationData: read header X failed: %s", 
                       strerror(errno));
            emailAlertSend(ALERT_TYPE_STATION_READ);
            return ERROR;
        }
    }

    groupType = (int)wmr918Work.readData[2];

    // read remaining bytes of this type + checksum:
    retVal = (*work->medium.read) (&work->medium, 
                                   &wmr918Work.readData[3], 
                                   wmr918GroupLength[groupType], 
                                   WMR918_READ_TIMEOUT);
    if (retVal != wmr918GroupLength[groupType])
    {
        radMsgLog (PRI_MEDIUM, "readStationData: read payload failed: %s", 
                   strerror(errno));
        emailAlertSend(ALERT_TYPE_STATION_READ);
        return ERROR;
    }

    // Verify checksum:
    checkSum = 0;
    for (i = 0; i < wmr918GroupLength[groupType] + 2; i ++)
    {
        checkSum += wmr918Work.readData[i];
    }
    checkSum &= 0xFF;
    if (checkSum != wmr918Work.readData[wmr918GroupLength[groupType] + 2])
    {
        radMsgLog (PRI_MEDIUM, "readStationData: checksum mismatch: computed %2.2X, RX %2.2X", 
                   checkSum, wmr918Work.readData[wmr918GroupLength[groupType] + 2]);
        emailAlertSend(ALERT_TYPE_STATION_READ);
        return ERROR;
    }

    // Parse it for data:
    switch (groupType)
    {
        case WMR918GROUP0:
        {
            wmr918Work.dataRXMask |= WMR918_SENSOR_WIND;

            // Battery status:
            wmr918Work.sensorData.windBatteryStatus = HI(pPacket[1]);

            // Wind gust direction:
            tempFloat = (float)(NUM(pPacket[2]) + (100 * LO(pPacket[3])));
            wmr918Work.sensorData.maxWindDir = tempFloat;
            wmr918Work.sensorData.windDir = wmr918Work.sensorData.maxWindDir;

            // Gust speed:
            tempFloat = (float)(HI(pPacket[3]));
            tempFloat *= 0.1;
            tempFloat += (float)(NUM(pPacket[4]));
            tempFloat *= 2.237;                     // convert to mph
            wmr918Work.sensorData.maxWindSpeed = tempFloat;

            // Average speed:
            tempFloat = (float)(NUM(pPacket[5]));
            tempFloat *= 0.1;
            tempFloat += (float)(LO(pPacket[6]) * 10);
            tempFloat *= 2.237;                     // convert to mph
            wmr918Work.sensorData.windSpeed = tempFloat;
            break;
        }
        case WMR918GROUP1:
        {
            wmr918Work.dataRXMask |= WMR918_SENSOR_RAIN;

            // Battery status:
            wmr918Work.sensorData.rainBatteryStatus = HI(pPacket[1]);

            // Rain rate:
            tempFloat = (float)(NUM(pPacket[2]) + (100 * LO(pPacket[3])));
            tempFloat *= 0.03937;
            wmr918Work.sensorData.rainrate = tempFloat;

            // Rain total:
            tempFloat = (float)NUM(pPacket[4]);
            tempFloat += (float)(NUM(pPacket[5]) * 100);
            tempFloat *= 0.03937;
            wmr918Work.sensorData.rain = tempFloat;
            break;
        }
        case WMR918GROUP2:
        {
            // Up to 3 channels for WMR968, capture value
            channel = LO(pPacket[1]);

            // verify channel and set to use as index:
            if (channel != 0x4 && channel != 0x2 && channel != 0x1)
            {
                radMsgLog(PRI_MEDIUM, "readStationData: group2 data has invalid channel %d", channel);
                break;
            }

            if (channel == 0x4)
            {
                channel = 3;
            }

            if ((int)ifWorkData->outsideChannel == channel)
                wmr918Work.dataRXMask |= WMR918_SENSOR_OUT_TEMP;

            // Battery status:
            if ((int)ifWorkData->outsideChannel == channel)
                wmr918Work.sensorData.outTempBatteryStatus = HI(pPacket[1]) & 0x7;
            else if ((int)ifWorkData->outsideChannel < channel)
                wmr918Work.sensorData.extraBatteryStatus[channel - 1] = HI(pPacket[1]);
            else
                wmr918Work.sensorData.extraBatteryStatus[channel] = HI(pPacket[1]);

            // Temp:
            tempFloat = (float)NUM(pPacket[2]);
            tempFloat *= 0.1;
            tempFloat += (float)(LO(pPacket[3]) * 10);
            if (BIT(HI(pPacket[3]),3))
            {
                tempFloat *= -1.0;
            }
            tempFloat *= 9;
            tempFloat /= 5;
            tempFloat += 32;
            if ((int)ifWorkData->outsideChannel == channel)
                wmr918Work.sensorData.outTemp = tempFloat;
            else if ((int)ifWorkData->outsideChannel < channel)
                wmr918Work.sensorData.extraTemp[channel - 1] = tempFloat;
            else
                wmr918Work.sensorData.extraTemp[channel] = tempFloat;

            // Humidity:
            tempFloat = (float)NUM(pPacket[4]);
            if ((int)ifWorkData->outsideChannel == channel)
                wmr918Work.sensorData.outHumidity = tempFloat;
            else if ((int)ifWorkData->outsideChannel < channel)
                wmr918Work.sensorData.extraHumidity[channel - 1] = tempFloat;
            else
                wmr918Work.sensorData.extraHumidity[channel] = tempFloat;

            break;
        }
        case WMR918GROUP3:
        {
            // check if this is the primary outside temperature selection
            // if not then this sensor will become channel 0 extra sensor
            if ((int)ifWorkData->outsideChannel == 0)
                wmr918Work.dataRXMask |= WMR918_SENSOR_OUT_TEMP;

            // Battery status:
            if ((int)ifWorkData->outsideChannel == 0)
                wmr918Work.sensorData.outTempBatteryStatus = HI(pPacket[1]) & 0x7;
            else
                wmr918Work.sensorData.extraBatteryStatus[0] = HI(pPacket[1]);

            // Temp:
            tempFloat = (float)NUM(pPacket[2]);
            tempFloat *= 0.1;
            tempFloat += (float)(LO(pPacket[3]) * 10);
            if (BIT(HI(pPacket[3]),3))
            {
                tempFloat *= -1.0;
            }
            tempFloat *= 9;
            tempFloat /= 5;
            tempFloat += 32;
            if ((int)ifWorkData->outsideChannel == 0)
                wmr918Work.sensorData.outTemp = tempFloat;
            else
                wmr918Work.sensorData.extraTemp[0] = tempFloat;

            // Humidity:
            tempFloat = (float)NUM(pPacket[4]);
            if ((int)ifWorkData->outsideChannel == 0)
                wmr918Work.sensorData.outHumidity = tempFloat;
            else
                wmr918Work.sensorData.extraHumidity[0] = tempFloat;

            break;
        }
        case WMR918GROUP4:
        { 
	    int tmpChan;
	    //
	    // Channel is bit-position encoded in the low order of packet 1 range 1-3
	    //
	    tmpChan = LO(pPacket[1]) & 0x07;
	    if (tmpChan == 4)
	    {
	       tmpChan = 3;
	    }
	    // Sanity -- keeping same.
            if ((tmpChan <= 0) || (tmpChan > 4))
	    {
	       tmpChan = 1;
	    }
            // Temp:
            tempFloat = (float)NUM(pPacket[2]);
            tempFloat *= 0.1;
            tempFloat += (float)(LO(pPacket[3]) * 10);
            if (BIT(HI(pPacket[3]),3))
            {
                tempFloat *= -1.0;
            }
            tempFloat *= 9;
            tempFloat /= 5;
            tempFloat += 32;
	    //
	    // To support legacy pool sensors take channel 1 (note - channel selection on the
	    // pool sensor is by switch on some models.
	    //
	    if (tmpChan == 1)
	    {
	       wmr918Work.sensorData.poolTempBatteryStatus = HI (pPacket[1]) & 0x7;
               wmr918Work.sensorData.pool = tempFloat;
	    }
 	    wmr918Work.sensorData.extraTemp[tmpChan - 1] = tempFloat;
            break;
        }
        case WMR918GROUP5:
        {
            wmr918Work.dataRXMask |= WMR918_SENSOR_IN_TEMP;

            // Battery status:
            wmr918Work.sensorData.inTempBatteryStatus = HI(pPacket[1]);

            // Temp:
            tempFloat = (float)NUM(pPacket[2]);
            tempFloat *= 0.1;
            tempFloat += (float)(LO(pPacket[3]) * 10);
            if (BIT(HI(pPacket[3]),3))
            {
                tempFloat *= -1.0;
            }
            tempFloat *= 9;
            tempFloat /= 5;
            tempFloat += 32;
            wmr918Work.sensorData.inTemp = tempFloat;

            // Humidity:
            tempFloat = (float)NUM(pPacket[4]);
            wmr918Work.sensorData.inHumidity = tempFloat;

            // BP:
            tempFloat = (float)pPacket[6];
            tempFloat += 795;
            tempFloat *= 0.02953;
            wmr918Work.sensorData.pressure = tempFloat;
            break;
        }
        case WMR918GROUP6:
        {
            wmr918Work.dataRXMask |= WMR918_SENSOR_IN_TEMP;

            // Battery status:
            wmr918Work.sensorData.inTempBatteryStatus = HI(pPacket[1]);

            // Temp:
            tempFloat = (float)NUM(pPacket[2]);
            tempFloat *= 0.1;
            tempFloat += (float)(LO(pPacket[3]) * 10);
            if (BIT(HI(pPacket[3]),3))
            {
                tempFloat *= -1.0;
            }
            tempFloat *= 9;
            tempFloat /= 5;
            tempFloat += 32;
            wmr918Work.sensorData.inTemp = tempFloat;

            // Humidity:
            tempFloat = (float)NUM(pPacket[4]);
            wmr918Work.sensorData.inHumidity = tempFloat;

            // BP:
            tempFloat = (float)((int)pPacket[6] + (int)(BIT(LO(pPacket[7]),0) << 8));
            tempFloat += 600;
            tempFloat *= 0.02953;
            wmr918Work.sensorData.pressure = tempFloat;

            // Forecast:
            wmr918Work.sensorData.tendency = HI(pPacket[7]);

            break;
        }
        default:
        {
            break;
        }
    }

    return groupType;
}

static void storeLoopPkt (WVIEWD_WORK *work, LOOP_PKT *dest, WMR918_DATA *src)
{
    float               tempfloat;
    WMR918_IF_DATA*     ifWorkData = (WMR918_IF_DATA*)work->stationData;
    time_t              nowTime = time(NULL);

    // Clear optional data:
    stationClearLoopData(work);


    // WMR918 produces station pressure
    if (10 < src->pressure && src->pressure < 50)
    {
        dest->stationPressure               = src->pressure;

        // Apply calibration here so the computed values reflect it:
        dest->stationPressure *= work->calMPressure;
        dest->stationPressure += work->calCPressure;

        if (-150 < src->outTemp && src->outTemp < 150)
        {
            // compute sea-level pressure (BP)
            dest->barometer                 = wvutilsConvertSPToSLP (dest->stationPressure,
                                                                     src->outTemp,
                                                                     (float)ifWorkData->elevation);
        }

        // calculate altimeter
        dest->altimeter                     = wvutilsConvertSPToAltimeter (dest->stationPressure,
                                                                         (float)ifWorkData->elevation);
    }

    if (-150 < src->outTemp && src->outTemp < 150)
        dest->outTemp                       = src->outTemp;

    if (0 <= src->outHumidity && src->outHumidity <= 100)
    {
        tempfloat = src->outHumidity;
        tempfloat += 0.5;
        dest->outHumidity                   = (uint16_t)tempfloat;
    }

    if (0 <= src->windSpeed && src->windSpeed <= 250)
    {
        tempfloat = src->windSpeed;
        dest->windSpeedF                    = tempfloat;
    }

    if (0 <= src->windDir && src->windDir <= 360)
    {
        tempfloat = src->windDir;
        tempfloat += 0.5;
        dest->windDir                       = (uint16_t)tempfloat;
    }

    if (0 <= src->maxWindSpeed && src->maxWindSpeed <= 250)
    {
        tempfloat = src->maxWindSpeed;
        dest->windGustF                     = tempfloat;
    }

    if (0 <= src->maxWindDir && src->maxWindDir <= 360)
    {
        tempfloat = src->maxWindDir;
        tempfloat += 0.5;
        dest->windGustDir                   = (uint16_t)tempfloat;
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

        // Compute rain rate - the WMR918 rain rate is poor...
        // Update the rain accumulator:
        sensorAccumAddSample (ifWorkData->rainRateAccumulator, nowTime, dest->sampleRain);
        dest->rainRate                      = sensorAccumGetTotal (ifWorkData->rainRateAccumulator);
        dest->rainRate                      *= (60/WMR918_RAIN_RATE_PERIOD);
    }
    else
    {
        dest->sampleRain = 0;
        sensorAccumAddSample (ifWorkData->rainRateAccumulator, nowTime, dest->sampleRain);
        dest->rainRate                      = sensorAccumGetTotal (ifWorkData->rainRateAccumulator);
        dest->rainRate                      *= (60/WMR918_RAIN_RATE_PERIOD);
    }        

    dest->inTemp                        = src->inTemp;
    tempfloat = src->inHumidity;
    tempfloat += 0.5;
    dest->inHumidity                    = (uint16_t)tempfloat;

    dest->extraTemp[0]                  = (float)src->extraTemp[0];
    tempfloat = src->extraHumidity[0];
    tempfloat += 0.5;
    dest->extraHumidity[0]              = (uint16_t)tempfloat;

    dest->extraTemp[1]                  = (float)src->extraTemp[1];
    tempfloat = src->extraHumidity[1];
    tempfloat += 0.5;
    dest->extraHumidity[1]              = (uint16_t)tempfloat;

    dest->extraTemp[2]                  = (float)src->extraTemp[2];

    // WMR918 specific:
    tempfloat = src->extraHumidity[2];
    tempfloat += 0.5;
    dest->wmr918Humid3                  = (uint16_t)tempfloat;

    dest->wmr918Pool                    = src->pool;

    dest->wmr918WindBatteryStatus       = src->windBatteryStatus;
    dest->wmr918RainBatteryStatus       = src->rainBatteryStatus;
    dest->wmr918OutTempBatteryStatus    = src->outTempBatteryStatus;
    dest->wmr918InTempBatteryStatus     = src->inTempBatteryStatus;
    dest->wmr918poolTempBatteryStatus   = src->poolTempBatteryStatus;
    dest->wmr918extra1BatteryStatus     = src->extraBatteryStatus[0];
    dest->wmr918extra2BatteryStatus     = src->extraBatteryStatus[1];
    dest->wmr918extra3BatteryStatus     = src->extraBatteryStatus[2];
    dest->wmr918Tendency                = src->tendency;

    return;
}


////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////  A P I  /////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

int wmr918Init (WVIEWD_WORK *work)
{
    WMR918_IF_DATA*     ifWorkData = (WMR918_IF_DATA*)work->stationData;
    fd_set              rfds;
    struct timeval      tv;
    int                 i, length, retVal;
    time_t              nowTime = time(NULL) - (WV_SECONDS_IN_HOUR/(60/WMR918_RAIN_RATE_PERIOD));
    ARCHIVE_PKT         recordStore;
    char                outString[128];

    memset (&wmr918Work, 0, sizeof(wmr918Work));

    // Create the rain accumulator (WMR918_RAIN_RATE_PERIOD minute age) 
    // so we can compute rain rate:
    ifWorkData->rainRateAccumulator = sensorAccumInit(WMR918_RAIN_RATE_PERIOD);

    // Populate the accumulator with the last WMR918_RAIN_RATE_PERIOD minutes:
    while ((nowTime = dbsqliteArchiveGetNextRecord(nowTime, &recordStore)) != ERROR)
    {
        sensorAccumAddSample(ifWorkData->rainRateAccumulator, 
                             recordStore.dateTime, 
                             recordStore.value[DATA_INDEX_rain]);
    }

    radMsgLog (PRI_MEDIUM, 
        "wmr918Init: waiting for first sensor packets (this may take some time):");
    while ((wmr918Work.dataRXMask != WMR918_SENSOR_ALL) && (! work->exiting))
    {
        // Log what we are waiting for:
        length = 0;
        for (i = 0; i < 4; i ++)
        {
            if (! (wmr918Work.dataRXMask & (1 << i)))
            {
                length += sprintf(&outString[length], "%s ", WMRSensorNames[i]);
            }
        }
        radMsgLog (PRI_MEDIUM, "wmr918Init: waiting for sensors: %s", outString);

        tv.tv_sec  = 2;
        tv.tv_usec = 0;

        FD_ZERO(&rfds);
        FD_SET(work->medium.fd, &rfds);
        if (select (work->medium.fd + 1, &rfds, NULL, NULL, &tv) > 0)
        {
            retVal = readStationData (work);
            switch (retVal)
            {
                case WMR918GROUP0:
                    radMsgLog (PRI_MEDIUM, "received WIND packet...");
                    break;
                case WMR918GROUP1:
                    radMsgLog (PRI_MEDIUM, "received RAIN packet...");
                    break;
                case WMR918GROUP2:
                    radMsgLog (PRI_MEDIUM, "received EXTRA TEMP packet...");
                    break;
                case WMR918GROUP3:
                    radMsgLog (PRI_MEDIUM, "received OUT TEMP packet...");
                    break;
                case WMR918GROUP4:
                    radMsgLog (PRI_MEDIUM, "received POOL TEMP packet...");
                    break;
                case WMR918GROUP5:
                case WMR918GROUP6:
                    radMsgLog (PRI_MEDIUM, "received IN TEMP packet...");
                    break;
                default:
                    break;
            }
        }
    }

    if (! work->exiting)
    {
        radMsgLog (PRI_MEDIUM, "wmr918Init: first sensor packets received.");
    }

    // populate the LOOP structure:
    ifWorkData->wmr918Readings = wmr918Work.sensorData;
    storeLoopPkt (work, &work->loopPkt, &ifWorkData->wmr918Readings);

    // we must indicate successful completion here -
    // even though we are synchronous, the daemon wants to see this event
    radProcessEventsSend (NULL, STATION_INIT_COMPLETE_EVENT, 0);

    return OK;
}

void wmr918Exit (WVIEWD_WORK *work)
{
    WMR918_IF_DATA*     ifWorkData = (WMR918_IF_DATA*)work->stationData;

    return;
}

void wmr918ReadData (WVIEWD_WORK *work)
{
    // process received packets:
    readStationData (work);
}

void wmr918GetReadings (WVIEWD_WORK *work)
{
    WMR918_IF_DATA*     ifWorkData = (WMR918_IF_DATA*)work->stationData;

    // populate the LOOP structure:
    ifWorkData->wmr918Readings = wmr918Work.sensorData;
    storeLoopPkt (work, &work->loopPkt, &ifWorkData->wmr918Readings);

    // indicate the LOOP packet is done
    radProcessEventsSend (NULL, STATION_LOOP_COMPLETE_EVENT, 0);
}

