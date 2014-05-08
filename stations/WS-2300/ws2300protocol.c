/*---------------------------------------------------------------------------
 
  FILENAME:
        ws2300protocol.c
 
  PURPOSE:
        Provide protocol utilities for WS-2300 station communication.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        02/27/2008      M.S. Teel       0               Original
        02/14/2010      MS Teel         1               Add user contributed
                                                        sensor multiple read
                                                        data check
 
  NOTES:
        Portions of the WS-2300 interface code was inspired by the rw2300 
        library, open2300 Version 1.10, Copyright 2003-2005, Kenneth Lavrsen.

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
#include <ws2300protocol.h>

/*  ... global memory declarations
*/

/*  ... local memory
*/

static WS2300_WORK          ws2300Work;

static WS_SENSOR_INFO       sensorInfo[WS_SENSOR_MAX] =
{
    { 0x346, 2 },
    { 0x373, 2 },
    { 0x3FB, 1 },
    { 0x419, 1 },
    { 0x527, 3 },
    { 0x5D8, 3 },
    { 0x4D2, 3 },
};


static void encodeAddress (int address_in, uint8_t* address_out)
{
    int             i = 0;
    int             adrbytes = 4;
    uint8_t         nibble;

    for (i = 0; i < adrbytes; i ++)
    {
        nibble = (address_in >> (4 * (3 - i))) & 0x0F;
        address_out[i] = (uint8_t) (0x82 + (nibble * 4));
    }

    return;
}

static void encodeData
(
    int             number, 
    uint8_t         encode_constant,
    uint8_t*        data_in, 
    uint8_t*        data_out
)
{
    int             i = 0;

    for (i = 0; i < number; i ++)
    {
        data_out[i] = (uint8_t) (encode_constant + (data_in[i] * 4));
    }

    return;
}

static uint8_t encodeLength (int number)
{
    int             coded_number;

    coded_number = (uint8_t) (0xC2 + number * 4);
    if (coded_number > 0xfe)
        coded_number = 0xfe;

    return coded_number;
}

/*******************************************************************************
 * checksumSequence calculates the checksum for the first 4
 * commands sent to WS2300.
 ******************************************************************************/
static uint8_t checksumSequence (uint8_t* command, int sequence)
{
    int             response;

    response = sequence * 16 + ((*command) - 0x82) / 4;
    return (uint8_t)response;
}

/*******************************************************************************
 * checksumLast calculates the checksum for the last command
 * which is sent just before data is received from WS2300
 ******************************************************************************/
static uint8_t checksumLast (int number)
{
    int             response;

    response = 0x30 + number;
    return response;
}


/*******************************************************************************
 * checksumDataRX calculates the checksum for the data bytes received
 * from the WS2300
 ******************************************************************************/
static uint8_t checksumDataRX (uint8_t* data, int number)
{
    int             checksum = 0;
    int             i;

    for (i = 0; i < number; i ++)
    {
        checksum += data[i];
    }

    checksum &= 0xFF;
    return (uint8_t) checksum;
}

/*******************************************************************************
 * resetStation WS2300 by sending command 06
 ******************************************************************************/
static void resetStation (WVIEWD_WORK* work)
{
    uint8_t     command = 0x06;
    uint8_t     answer;
    int         i;

    for (i = 0; i < 3; i ++)
    {
        // Discard any garbage in the buffers
        tcflush (work->medium.fd, TCIFLUSH);
        tcflush (work->medium.fd, TCOFLUSH);

        (*work->medium.write) (&work->medium, &command, 1);
        (*work->medium.read) (&work->medium, &answer, 1, WS2300_READ_TIMEOUT);

        if (answer == 0x02)
        {
            return;
        }

        // we sleep longer and longer for each retry
        radUtilsSleep (50 * (i+1));
    }
}

static int startDataRequest (WVIEWD_WORK* work, int startFlag)
{
    int         retVal;

    if (startFlag || ws2300Work.currentSensor == WS_SENSOR_MAX)
    {
        ws2300Work.currentSensor = WS_SENSOR_IN_TEMP;
        ws2300Work.numTries = 0;
        ws2300Work.numGood = 0;
    }

    memset (ws2300Work.commandData, 0, WS2300_BUFFER_LENGTH);
    memset (ws2300Work.readData, 0, WS2300_BUFFER_LENGTH);

    // First 4 bytes are populated with converted address
    encodeAddress (sensorInfo[ws2300Work.currentSensor].address, ws2300Work.commandData);

    // Last populate the 5th byte with the converted number of bytes
    ws2300Work.commandData[4] = encodeLength (sensorInfo[ws2300Work.currentSensor].bytes);

    retVal = (*work->medium.write) (&work->medium, &ws2300Work.commandData[0], 1);
    if (retVal != 1)
    {
        radMsgLog (PRI_MEDIUM, "startDataRequest: write address 0 failed: %s", 
                   strerror(errno));
        emailAlertSend(ALERT_TYPE_STATION_LOOP);
        return ERROR;
    }

    return OK;
}

static void RetryCurrentSensor(WVIEWD_WORK* work)
{
    radUtilsSleep(50);

    // reset the station:
    resetStation(work);

    // Start the sensor gathering:
    if (startDataRequest(work, FALSE) == ERROR)
    {
        radMsgLog (PRI_HIGH, "RetryCurrentSensor: startDataRequest failed!");
        return;
    }
    
    radProcessTimerStart (work->ifTimer, WS_RESPONSE_TIMEOUT);
}

static float computeRainRate (WVIEWD_WORK *work, float sampleRain)
{
    int         sampleSECS = work->cdataInterval / 1000;
    int         multiplier;
    float       rainRate;

    multiplier = WV_SECONDS_IN_HOUR / sampleSECS;
    rainRate = sampleRain * (float)multiplier;
    return rainRate;
}

static void storeLoopPkt (WVIEWD_WORK *work, LOOP_PKT *dest, WS2300_DATA *src)
{
    float               tempfloat;
    WS2300_IF_DATA*     ifWorkData = (WS2300_IF_DATA*)work->stationData;
    time_t              nowTime = time (NULL);

    // Clear optional data:
    stationClearLoopData(work);


    // WS-2300 produces station pressure
    if ((10 < src->pressure && src->pressure < 50) &&
        (-150 < src->outTemp && src->outTemp < 150))
    {
        dest->stationPressure               = src->pressure;
    
        // Apply calibration here so the computed values reflect it:
        dest->stationPressure *= work->calMPressure;
        dest->stationPressure += work->calCPressure;
    
        // compute sea-level pressure (BP)
        tempfloat = wvutilsConvertSPToSLP (dest->stationPressure,
                                         src->outTemp,
                                         (float)ifWorkData->elevation);
        dest->barometer                     = tempfloat;
    
        // calculate altimeter
        tempfloat = wvutilsConvertSPToAltimeter (dest->stationPressure,
                                               (float)ifWorkData->elevation);
        dest->altimeter                     = tempfloat;
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

        // Compute rain rate - the WS-2300 does not provide it!
        // Update the rain accumulator:
        sensorAccumAddSample (ifWorkData->rainRateAccumulator, nowTime, dest->sampleRain);
        dest->rainRate                      = sensorAccumGetTotal (ifWorkData->rainRateAccumulator);
        dest->rainRate                      *= (60/WS2300_RAIN_RATE_PERIOD);
    }
    else
    {
        dest->sampleRain = 0;
        sensorAccumAddSample (ifWorkData->rainRateAccumulator, nowTime, dest->sampleRain);
        dest->rainRate                      = sensorAccumGetTotal (ifWorkData->rainRateAccumulator);
        dest->rainRate                      *= (60/WS2300_RAIN_RATE_PERIOD);
    }

    dest->inTemp                        = src->inTemp;
    tempfloat = src->inHumidity;
    tempfloat += 0.5;
    dest->inHumidity                    = (uint16_t)tempfloat;

    return;
}


////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////  A P I  /////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

int ws2300Init (WVIEWD_WORK *work)
{
    WS2300_IF_DATA*     ifWorkData = (WS2300_IF_DATA*)work->stationData;
    time_t              nowTime = time (NULL) - (WV_SECONDS_IN_HOUR/(60/WS2300_RAIN_RATE_PERIOD));
    ARCHIVE_PKT         recordStore;
    float               tempRain;

    memset (&ws2300Work, 0, sizeof(ws2300Work));

    // Create the rain accumulator (WS2300_RAIN_RATE_PERIOD minute age) 
    // so we can compute rain rate:
    ifWorkData->rainRateAccumulator = sensorAccumInit (WS2300_RAIN_RATE_PERIOD);

    // Populate the accumulator with the last WS2300_RAIN_RATE_PERIOD minutes:
    while ((nowTime = dbsqliteArchiveGetNextRecord(nowTime, &recordStore)) != ERROR)
    {
        sensorAccumAddSample(ifWorkData->rainRateAccumulator, 
                             recordStore.dateTime, 
                             recordStore.value[DATA_INDEX_rain]);
    }

    // Reset the station:
    resetStation (work);
    radUtilsSleep (50);
    resetStation (work);

    return OK;
}

void ws2300Exit (WVIEWD_WORK *work)
{
    WS2300_IF_DATA*     ifWorkData = (WS2300_IF_DATA*)work->stationData;

    // Clean up the rain accumulator:
    sensorAccumExit (ifWorkData->rainRateAccumulator);

    return;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////       STATE HANDLERS       ////////////////////////////
////////////////////////////////////////////////////////////////////////////////

int wsStartProcState (int state, void *stimulus, void *data)
{
    STIM                *stim = (STIM *)stimulus;
    WVIEWD_WORK         *work = (WVIEWD_WORK *)data;
    WS2300_IF_DATA*     ifWorkData = (WS2300_IF_DATA*)work->stationData;

    switch (stim->type)
    {
    case STIM_DUMMY:
        //  ... this one starts this state machine

        // set the work archive interval now
        work->archiveInterval = ifWorkData->archiveInterval;
    
        // sanity check the archive interval against the most recent record
        if (stationVerifyArchiveInterval (work) == ERROR)
        {
            // bad magic!
            radMsgLog (PRI_HIGH, "stationInit: stationVerifyArchiveInterval failed!");
            radMsgLog (PRI_HIGH, "You must either move old /var/wview/archive files out of the way -or-");
            radMsgLog (PRI_HIGH, "fix the wview.conf setting...");
            (*(work->medium.exit)) (&work->medium);
            return WS_STATE_ERROR;
        }
        else
        {
            radMsgLog (PRI_STATUS, "station archive interval: %d minutes",
                       work->archiveInterval);
        }
    
        ifWorkData->totalRain = 0;
    
        // initialize the station interface
        if (ws2300Init (work) == ERROR)
        {
            radMsgLog (PRI_HIGH, "stationInit: ws2300Init failed!");
            (*(work->medium.exit)) (&work->medium);
            return WS_STATE_ERROR;
        }
    
        // Start the first sensor gathering:
        if (startDataRequest (work, TRUE) == ERROR)
        {
            radMsgLog (PRI_HIGH, "stationInit: startDataRequest failed!");
            (*(work->medium.exit)) (&work->medium);
            return WS_STATE_ERROR;
        }
        
        radProcessTimerStart (work->ifTimer, WS_RESPONSE_TIMEOUT);
        return WS_STATE_ADRS0_ACK;
    }

    return state;
}

int wsRunState (int state, void *stimulus, void *data)
{
    STIM                *stim = (STIM *)stimulus;
    WVIEWD_WORK         *work = (WVIEWD_WORK *)data;

    switch (stim->type)
    {
    case WS_STIM_READINGS:
        // Start the sensor gathering:
        if (startDataRequest (work, TRUE) == ERROR)
        {
            radMsgLog (PRI_HIGH, "wsRunState: startDataRequest failed!");
            return WS_STATE_ERROR;
        }

        radProcessTimerStart (work->ifTimer, WS_RESPONSE_TIMEOUT);
        return WS_STATE_ADRS0_ACK;
    }

    return state;
}

int wsAdrs0State (int state, void *stimulus, void *data)
{
    STIM                *stim = (STIM *)stimulus;
    WVIEWD_WORK         *work = (WVIEWD_WORK *)data;
    int                 retVal;
    uint8_t             readData;

    switch (stim->type)
    {
    case STIM_IO:
        radProcessTimerStop (work->ifTimer);

        retVal = (*work->medium.read) (&work->medium, &readData, 1, WS2300_READ_TIMEOUT);
        if (retVal != 1)
        {
            radMsgLog (PRI_MEDIUM, "wsAdrs0State: read address 0 failed: %s", 
                       strerror(errno));
            RetryCurrentSensor(work);
            return WS_STATE_ADRS0_ACK;
        }
        if (readData != checksumSequence (&ws2300Work.commandData[0], 0))
        {
            // start this sensor again:
            RetryCurrentSensor(work);
            return WS_STATE_ADRS0_ACK;
        }

        // All good, go to next byte
        retVal = (*work->medium.write) (&work->medium, &ws2300Work.commandData[1], 1);
        if (retVal != 1)
        {
            radMsgLog (PRI_MEDIUM, "wsAdrs0State: write address 1 failed: %s", 
                       strerror(errno));
            return WS_STATE_ERROR;
        }

        radProcessTimerStart (work->ifTimer, WS_RESPONSE_TIMEOUT);
        return WS_STATE_ADRS1_ACK;

    case STIM_TIMER:
        // IF timer expiry - try again:
        RetryCurrentSensor(work);
        return WS_STATE_ADRS0_ACK;
    }

    return state;
}

int wsAdrs1State (int state, void *stimulus, void *data)
{
    STIM                *stim = (STIM *)stimulus;
    WVIEWD_WORK         *work = (WVIEWD_WORK *)data;
    int                 retVal;
    uint8_t             readData;

    switch (stim->type)
    {
    case STIM_IO:
        radProcessTimerStop (work->ifTimer);

        retVal = (*work->medium.read) (&work->medium, &readData, 1, WS2300_READ_TIMEOUT);
        if (retVal != 1)
        {
            radMsgLog (PRI_MEDIUM, "wsAdrs1State: read address 1 failed: %s", 
                       strerror(errno));
            RetryCurrentSensor(work);
            return WS_STATE_ADRS0_ACK;
        }
        if (readData != checksumSequence (&ws2300Work.commandData[1], 1))
        {
            // start this sensor again:
            RetryCurrentSensor(work);
            return WS_STATE_ADRS0_ACK;
        }

        // All good, go to next byte
        retVal = (*work->medium.write) (&work->medium, &ws2300Work.commandData[2], 1);
        if (retVal != 1)
        {
            radMsgLog (PRI_MEDIUM, "wsAdrs1State: write address 2 failed: %s", 
                       strerror(errno));
            return WS_STATE_ERROR;
        }

        radProcessTimerStart (work->ifTimer, WS_RESPONSE_TIMEOUT);
        return WS_STATE_ADRS2_ACK;

    case STIM_TIMER:
        // IF timer expiry - try again:
        RetryCurrentSensor(work);
        return WS_STATE_ADRS0_ACK;
    }

    return state;
}

int wsAdrs2State (int state, void *stimulus, void *data)
{
    STIM                *stim = (STIM *)stimulus;
    WVIEWD_WORK         *work = (WVIEWD_WORK *)data;
    int                 retVal;
    uint8_t             readData;

    switch (stim->type)
    {
    case STIM_IO:
        radProcessTimerStop (work->ifTimer);

        retVal = (*work->medium.read) (&work->medium, &readData, 1, WS2300_READ_TIMEOUT);
        if (retVal != 1)
        {
            radMsgLog (PRI_MEDIUM, "wsAdrs2State: read address 2 failed: %s", 
                       strerror(errno));
            RetryCurrentSensor(work);
            return WS_STATE_ADRS0_ACK;
        }
        if (readData != checksumSequence (&ws2300Work.commandData[2], 2))
        {
            // start this sensor again:
            RetryCurrentSensor(work);
            return WS_STATE_ADRS0_ACK;
        }

        // All good, go to next byte
        retVal = (*work->medium.write) (&work->medium, &ws2300Work.commandData[3], 1);
        if (retVal != 1)
        {
            radMsgLog (PRI_MEDIUM, "wsAdrs2State: write address 3 failed: %s", 
                       strerror(errno));
            return WS_STATE_ERROR;
        }

        radProcessTimerStart (work->ifTimer, WS_RESPONSE_TIMEOUT);
        return WS_STATE_ADRS3_ACK;

    case STIM_TIMER:
        // IF timer expiry - try again:
        RetryCurrentSensor(work);
        return WS_STATE_ADRS0_ACK;
    }

    return state;
}

int wsAdrs3State (int state, void *stimulus, void *data)
{
    STIM                *stim = (STIM *)stimulus;
    WVIEWD_WORK         *work = (WVIEWD_WORK *)data;
    int                 retVal;
    uint8_t             readData;

    switch (stim->type)
    {
    case STIM_IO:
        radProcessTimerStop (work->ifTimer);

        retVal = (*work->medium.read) (&work->medium, &readData, 1, WS2300_READ_TIMEOUT);
        if (retVal != 1)
        {
            radMsgLog (PRI_MEDIUM, "wsAdrs3State: read address 0 failed: %s", 
                       strerror(errno));
            RetryCurrentSensor(work);
            return WS_STATE_ADRS0_ACK;
        }
        if (readData != checksumSequence (&ws2300Work.commandData[3], 3))
        {
            // start this sensor again:
            RetryCurrentSensor(work);
            return WS_STATE_ADRS0_ACK;
        }

        // All good, go to next byte (NumBytes)
        retVal = (*work->medium.write) (&work->medium, &ws2300Work.commandData[4], 1);
        if (retVal != 1)
        {
            radMsgLog (PRI_MEDIUM, "wsAdrs3State: write NumBytes failed: %s", 
                       strerror(errno));
            return WS_STATE_ERROR;
        }

        radProcessTimerStart (work->ifTimer, WS_RESPONSE_TIMEOUT);
        return WS_STATE_NUMBYTES_ACK;

    case STIM_TIMER:
        // IF timer expiry - try again:
        RetryCurrentSensor(work);
        return WS_STATE_ADRS0_ACK;
    }

    return state;
}

int wsNumBytesState (int state, void *stimulus, void *data)
{
    STIM                *stim = (STIM *)stimulus;
    WVIEWD_WORK         *work = (WVIEWD_WORK *)data;
    int                 retVal;
    uint8_t             readData;
    WS2300_IF_DATA*     ifWorkData = (WS2300_IF_DATA*)work->stationData;
    float               tempFloat = 0;
    int                 IsGoodValue;


    switch (stim->type)
    {
    case STIM_IO:
        radProcessTimerStop (work->ifTimer);

        // read the NumBytes ACK
        retVal = (*work->medium.read) (&work->medium, &readData, 1, WS2300_READ_TIMEOUT);
        if (retVal != 1)
        {
            radMsgLog (PRI_MEDIUM, "wsNumBytesState: read NumBytes ACK failed: %s", 
                       strerror(errno));
            RetryCurrentSensor(work);
            return WS_STATE_ADRS0_ACK;
        }

        if (readData != checksumLast (sensorInfo[ws2300Work.currentSensor].bytes))
        {
            // start this sensor again:
            RetryCurrentSensor(work);
            return WS_STATE_ADRS0_ACK;
        }

        // read the data bytes (and CRC):
        retVal = (*work->medium.read) (&work->medium, 
                                       &ws2300Work.readData[0], 
                                       sensorInfo[ws2300Work.currentSensor].bytes + 1, 
                                       WS2300_READ_TIMEOUT);
        if (retVal != sensorInfo[ws2300Work.currentSensor].bytes + 1)
        {
            // start this sensor again:
            RetryCurrentSensor(work);
            return WS_STATE_ADRS0_ACK;
        }

        // verify checksum:
        readData = ws2300Work.readData[sensorInfo[ws2300Work.currentSensor].bytes];
        if (readData != checksumDataRX (&ws2300Work.readData[0], sensorInfo[ws2300Work.currentSensor].bytes))
        {
            // start this sensor again:
            RetryCurrentSensor(work);
            return WS_STATE_ADRS0_ACK;
        }

        // OK, finally, we have data from the station -
        // switch on the sensor type to store it:
        switch (ws2300Work.currentSensor)
        {
            case WS_SENSOR_IN_TEMP:
                ws2300Work.sensorData.inTemp = ((((ws2300Work.readData[1] >> 4) * 10 + 
                                                (ws2300Work.readData[1] & 0xF) +
                                                (ws2300Work.readData[0] >> 4) / 10.0 + 
                                                (ws2300Work.readData[0] & 0xF) / 100.0) - 
                                                30.0) * 9 / 5 + 32);
                tempFloat = ws2300Work.sensorData.inTemp;
                break;
                
            case WS_SENSOR_OUT_TEMP:
                ws2300Work.sensorData.outTemp = ((((ws2300Work.readData[1] >> 4) * 10 + 
                                                (ws2300Work.readData[1] & 0xF) +
                                                (ws2300Work.readData[0] >> 4) / 10.0 + 
                                                (ws2300Work.readData[0] & 0xF) / 100.0) - 
                                                30.0) * 9 / 5 + 32);
                tempFloat = ws2300Work.sensorData.outTemp;
                break;
                
            case WS_SENSOR_IN_HUMIDITY:
                ws2300Work.sensorData.inHumidity = ((ws2300Work.readData[0] >> 4) * 10 + 
                                                    (ws2300Work.readData[0] & 0xF));
                tempFloat = ws2300Work.sensorData.inHumidity;
                break;
                
            case WS_SENSOR_OUT_HUMIDITY:
                ws2300Work.sensorData.outHumidity = ((ws2300Work.readData[0] >> 4) * 10 + 
                                                     (ws2300Work.readData[0] & 0xF));
                tempFloat = ws2300Work.sensorData.outHumidity;
                break;
                
            case WS_SENSOR_WIND:
                if ((ws2300Work.readData[0] != 0x00) ||
                    ((ws2300Work.readData[1] == 0xFF) && (((ws2300Work.readData[2] & 0xF) == 0) || 
                    ((ws2300Work.readData[2] & 0xF) == 1))))
                {
                    // Skip wind this time
                    break;
                }
                else
                {
                    ws2300Work.sensorData.windDir       = (ws2300Work.readData[2] >> 4) * 22.5;
                    ws2300Work.sensorData.windSpeed     = ((((ws2300Work.readData[2] & 0xF) << 8) + 
                                                           (ws2300Work.readData[1])) / 10.0 * 2.23693629);
                    ws2300Work.sensorData.maxWindDir    = ws2300Work.sensorData.windDir;
                    ws2300Work.sensorData.maxWindSpeed  = ws2300Work.sensorData.windSpeed;
                    tempFloat = ws2300Work.sensorData.windSpeed;
                }
                break;
                
            case WS_SENSOR_PRESSURE:
                ws2300Work.sensorData.pressure = (((ws2300Work.readData[2] & 0xF) * 1000 + 
                                                  (ws2300Work.readData[1] >> 4) * 100 +
                                                  (ws2300Work.readData[1] & 0xF) * 10 + 
                                                  (ws2300Work.readData[0] >> 4) +
                                                  (ws2300Work.readData[0] & 0xF) / 10.0) / 33.8638864);
                tempFloat = ws2300Work.sensorData.pressure;
                break;

            case WS_SENSOR_RAIN:
                ws2300Work.sensorData.rain = (((ws2300Work.readData[2] >> 4) * 1000 + 
                                              (ws2300Work.readData[2] & 0xF) * 100 +
                                              (ws2300Work.readData[1] >> 4) * 10 + 
                                              (ws2300Work.readData[1] & 0xF) + 
                                              (ws2300Work.readData[0] >> 4) /
                                              10.0 + (ws2300Work.readData[0] & 0xF) / 100.0) / 25.4);
                tempFloat = ws2300Work.sensorData.rain;
                break;
        }

        // Read each sensor at least 3 times to omit wild WS-2300 readings;
        // Contributed by a WS-2300 user:
        IsGoodValue = FALSE;
        if (ws2300Work.numTries > 0)
        {
            // We read each sensor 3 or more times, until we get 3 values
            // that are close to each other:
            switch (ws2300Work.currentSensor)
            {
                case WS_SENSOR_RAIN:
                    // Rain must be +/- 0.02 inches:
                    if ((tempFloat >= (ws2300Work.lastAttempt - 0.02)) && 
                        (tempFloat <= (ws2300Work.lastAttempt + 0.02))) 
                    {
                        // Looks good
                        IsGoodValue = 1;
                    }
                    break;

                case WS_SENSOR_PRESSURE:
                    // Pressure must be +/- 0.2 inHg:
                    if ((tempFloat >= (ws2300Work.lastAttempt - 0.2)) && 
                        (tempFloat <= (ws2300Work.lastAttempt + 0.2))) 
                    {
                        // Looks good
                        IsGoodValue = 1;
                    }
                    break;

                case WS_SENSOR_WIND:
                    // Wind is trickier as it can fluctuate more than other
                    // weather data and we don't want to "prune" valid wind
                    // events; Wind must be +/- 2 MPH:
                    if ((tempFloat >= (ws2300Work.lastAttempt - 2)) && 
                        (tempFloat <= (ws2300Work.lastAttempt + 2))) 
                    {
                        // Looks good
                        IsGoodValue = 1;
                    }
                    break;

                default:
                    // Everything else is +/- 1 unit (temp and humidity) 
                    // to be a good value match:
                    if ((tempFloat >= (ws2300Work.lastAttempt - 1.0)) && 
                        (tempFloat <= (ws2300Work.lastAttempt + 1.0)))
                    {
                        // Looks good
                        IsGoodValue = 1;
                    }
                    break;
            }

            ws2300Work.lastAttempt = tempFloat;

            if (IsGoodValue)
            {
                ws2300Work.numGood ++;
            }
            else
            {
                ws2300Work.numGood = 0;
            }
        }

        if ((ws2300Work.numGood < WS2300_NUM_GOOD_REQUIRED) && 
            (++ws2300Work.numTries < WS2300_MAX_RETRIES))
        {
            // Read the same sensor again and see what we get:
            if (startDataRequest(work, FALSE) == ERROR)
            {
                radMsgLog (PRI_HIGH, "wsNumBytesState: startDataRequestX failed!");
                return WS_STATE_ERROR;
            }
 
            radProcessTimerStart(work->ifTimer, WS_RESPONSE_TIMEOUT);
            return WS_STATE_ADRS0_ACK;
        }
 

        // Bump the sensor type:
        ws2300Work.numTries = 0;
        ws2300Work.numGood = 0;
        ws2300Work.currentSensor ++;
        if (ws2300Work.currentSensor >= WS_SENSOR_MAX)
        {
            // We are done!
            // populate the LOOP structure:
            ifWorkData->ws2300Readings = ws2300Work.sensorData;
            storeLoopPkt (work, &work->loopPkt, &ifWorkData->ws2300Readings);

            // check to see if this was the first time through:
            if (!work->runningFlag)
            {
                // we must indicate successful completion here -
                // the daemon wants to see this event:
                radProcessEventsSend (NULL, STATION_INIT_COMPLETE_EVENT, 0);
            }
            else
            {
                // indicate the LOOP packet is done:
                radProcessEventsSend (NULL, STATION_LOOP_COMPLETE_EVENT, 0);
            }

            return WS_STATE_RUN;
        }
        else
        {
            // start the next sensor:
            if (startDataRequest (work, FALSE) == ERROR)
            {
                radMsgLog (PRI_HIGH, "wsNumBytesState: startDataRequestX failed!");
                return WS_STATE_ERROR;
            }
            
            radProcessTimerStart (work->ifTimer, WS_RESPONSE_TIMEOUT);
            return WS_STATE_ADRS0_ACK;
        }
        return state;

    case STIM_TIMER:
        // IF timer expiry - try again:
        RetryCurrentSensor(work);
        return WS_STATE_ADRS0_ACK;
    }

    return state;
}

int wsErrorState (int state, void *stimulus, void *data)
{
    STIM                *stim = (STIM *)stimulus;
    WVIEWD_WORK         *work = (WVIEWD_WORK *)data;

    switch (stim->type)
    {
    default:
        radMsgLog (PRI_HIGH, "wsErrorState: invalid stimulus!");
        break;
    }

    return state;
}

