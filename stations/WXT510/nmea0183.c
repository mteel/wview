/*---------------------------------------------------------------------------

  FILENAME:
        nmea0183.c

  PURPOSE:
        Provide protocol utilities for NMEA 0183 station communication.

  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        01/14/2006      M.S. Teel       0               Original
        03/23/2008	    W. Krenn	    1               modified rain/hail/heating

  NOTES:

  LICENSE:
        Copyright (c) 2006, Mark S. Teel (mark@teel.ws)

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
#include <nmea0183.h>

/*  ... global memory declarations
*/

/*  ... local memory
*/

#define NMEA_XDUCER_MAX_TYPES       4

static NMEA0183_WORK        nmeaWork;

static char                 *TransducerTypes[NMEA_XDUCER_MAX_TYPES] =
{
    "WIND",
    "THP",
    "RAIN",
    "SUPV"
};


// compute NMEA checksum and generate the checksum string
static char *generateChecksum (char *command)
{
    static char     returnBuffer[4];
    unsigned char   highnibble, lownibble, sum = 0;
    int             i, length = strlen(command);

    for (i = 1; i < length; i ++)               // skip the '$' character
    {
        sum ^= command[i];
    }

    highnibble = (sum >> 4) & 0x0F;
    lownibble = sum & 0x0F;
    returnBuffer[0] = '*';
    returnBuffer[1] = ((highnibble < 0xA) ? ('0' + highnibble) : (('A' - 0xA) + highnibble));
    returnBuffer[2] = ((lownibble < 0xA) ? ('0' + lownibble) : (('A' - 0xA) + lownibble));
    returnBuffer[3] = 0;

    return returnBuffer;
}

// verify the checksum of a received line -
// (will truncate the checksum string from the input string)
static int verifyChecksum (char *inLine)
{
    int         length = strlen(inLine);
    char        inCS[8];

    if (length < 4)
    {
        // short string!
        return FALSE;
    }

    // the incoming line MUST be terminated with the checksum string
    strncpy (inCS, &inLine[length-3], sizeof(inCS));
    inLine[length-3] = 0;
    if (!strcmp(inCS, generateChecksum(inLine)))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

// read a line from the station
// returns TRUE if a line was read, FALSE otherwise
static int readLineFromStation (WVIEWD_WORK *work, char *store, int msTime)
{
    char        buffer[NMEA_BYTE_LENGTH_MAX];
    uint64_t   readTime = radTimeGetMSSinceEpoch() + (uint64_t)msTime;
    int         retVal, timeToRead, done = FALSE, byteCount = 0;

    // we read until we see a <CR><LF> as a line terminator
    while ((timeToRead = (uint32_t)(readTime - radTimeGetMSSinceEpoch())) > 0)
    {
        retVal = (*work->medium.read) (&work->medium, &buffer[byteCount], 1, timeToRead);
        if (retVal != 1)
        {
            radMsgLog (PRI_MEDIUM, "NMEA: readLineFromStation: read failed!");
            (*work->medium.flush) (&work->medium, WV_QUEUE_INPUT);
            emailAlertSend(ALERT_TYPE_STATION_READ);
            return FALSE;
        }
        byteCount ++;

        // are we done?
        if ((byteCount >= 2) &&
            (buffer[byteCount-2] == NMEA_CR) &&
            (buffer[byteCount-1] == NMEA_LF))
        {
            // we are!
            done = TRUE;
            break;
        }
    }

    if (!done || byteCount <= 2)
    {
        // ran out of time or empty line
        radMsgLog (PRI_MEDIUM, "NMEA: readLineFromStation: timeout or empty line!");
        (*work->medium.flush) (&work->medium, WV_QUEUE_INPUT);
        emailAlertSend(ALERT_TYPE_STATION_READ);
        return FALSE;
    }

    // now lose the <CR> and <LF>
    byteCount -= 2;
    buffer[byteCount] = 0;
    wvstrncpy (store, buffer, NMEA_BYTE_LENGTH_MAX);
    return TRUE;
}

// processes a transducer field set
// returns TRUE if the XDUCER is found, otherwise FALSE
static int processTransducer (PARSER_ID parser, int startIndex)
{
    char            type;
    int             id;
    float           value;
    char            units;
    char            *retStr;

    // load up the fields properly
    retStr = parserGetNumber (parser, startIndex);
    if (retStr == NULL)
    {
        return FALSE;
    }
    type = retStr[0];
    retStr = parserGetNumber (parser, startIndex+1);
    if (retStr == NULL)
    {
        return FALSE;
    }
    value = (float)atof (retStr);
    retStr = parserGetNumber (parser, startIndex+2);
    if (retStr == NULL)
    {
        return FALSE;
    }
    units = retStr[0];
    retStr = parserGetNumber (parser, startIndex+3);
    if (retStr == NULL)
    {
        return FALSE;
    }
    id = atoi (retStr);

    // now we switch on the type field
    switch (type)
    {
        case 'C':
        {
            switch (id)
            {
                case 0:
                    // TEMP - air
                    if (units != 'F')
                    {
                        break;
                    }
                    nmeaWork.sensorData.temperature = value;
                    nmeaWork.sensorStatus |= NMEA_DONE_TEMP;
                    break;
                case 1:
                    // TEMP - internal
                    break;
                case 2:
                    // TEMP - heating
                    nmeaWork.sensorData.heatingTemp = value;
                    nmeaWork.sensorStatus |= NMEA_DONE_HEAT_TEMP;
                    break;
            }

            break;
        }
        case 'A':
        {
            switch (id)
            {
                case 0:
                    // DIRECTION - min
                    break;
                case 1:
                    // DIRECTION - avg
                    if (units != 'D')
                    {
                        break;
                    }
                    nmeaWork.sensorData.windDir = value;
                    nmeaWork.sensorStatus |= NMEA_DONE_WINDDIR;
                    break;
                case 2:
                    // DIRECTION - max
                    if (units != 'D')
                    {
                        break;
                    }
                    nmeaWork.sensorData.maxWindDir = value;
                    nmeaWork.sensorStatus |= NMEA_DONE_MAXWINDDIR;
                    break;
            }

            break;
        }
        case 'S':
        {
            switch (id)
            {
                case 0:
                    // SPEED - min
                    break;
                case 1:
                    // SPEED - avg
                    if (units != 'S')
                    {
                        break;
                    }
                    nmeaWork.sensorData.windSpeed = value;
                    nmeaWork.sensorStatus |= NMEA_DONE_WINDSPD;
                    break;
                case 2:
                    // SPEED - max
                    if (units != 'S')
                    {
                        break;
                    }
                    nmeaWork.sensorData.maxWindSpeed = value;
                    nmeaWork.sensorStatus |= NMEA_DONE_MAXWINDSPD;
                    break;
            }

            break;
        }
        case 'P':
        {
            // pressure is always ID 0
            if (units != 'I')
            {
                break;
            }
            nmeaWork.sensorData.pressure = value;
            nmeaWork.sensorStatus |= NMEA_DONE_PRESSURE;
            break;
        }
        case 'H':
        {
            // humidity is always ID 0
            if (units != 'P')
            {
                break;
            }
            nmeaWork.sensorData.humidity = value;
            nmeaWork.sensorStatus |= NMEA_DONE_HUMIDITY;
            break;
        }
        case 'V':
        {
            switch (id)
            {
                case 0:
                    // RAIN
                    if (units != 'I')
                    {
                        break;
                    }
                    nmeaWork.sensorData.rain = value;
                    nmeaWork.sensorStatus |= NMEA_DONE_RAIN;
                    break;
                case 1:
                    // HAIL
                    nmeaWork.sensorData.hail = value;
                    nmeaWork.sensorStatus |= NMEA_DONE_HAIL;
                    break;
            }

            break;
        }
        case 'Z':
        {
            switch (id)
            {
                case 0:
                    // DURATION - rain
                    nmeaWork.sensorData.rainduration = value;
                    // nmeaWork.sensorStatus |= NMEA_DONE_RAIN;
                    break;
                case 1:
                    nmeaWork.sensorData.hailduration = value;
                    // nmeaWork.sensorStatus |= NMEA_DONE_RAIN;
                    // DURATION - hail
                    break;
            }

            break;
        }
        case 'R':
        {
            switch (id)
            {
                case 0:
                    // RATE - rain
                    if (units != 'I')
                    {
                        break;
                    }
                    nmeaWork.sensorData.rainrate = value;
                    nmeaWork.sensorStatus |= NMEA_DONE_RAINRATE;
                    break;
                case 1:
                    // RATE - hail
                    nmeaWork.sensorData.hailrate = value;
                    nmeaWork.sensorStatus |= NMEA_DONE_HAILRATE;
                    break;
                case 2:
                    // Peak RATE - rain
                    nmeaWork.sensorData.rainpeakrate = value;
                    // nmeaWork.sensorStatus |= NMEA_DONE_RAINRATE;
                    break;
                case 3:
                    // Peak RATE - hail
                    nmeaWork.sensorData.hailpeakrate = value;
                    // nmeaWork.sensorStatus |= NMEA_DONE_HAILRATE;
                    break;
            }

            break;
        }
        case 'U':
        {
            switch (id)
            {
                case 0:
                    // VOLTAGE - supply
                    nmeaWork.sensorData.supplyVoltage = value;
                    nmeaWork.sensorStatus |= NMEA_DONE_SUP_VOLT;
                    break;
                case 1:
                    // VOLTAGE - heating
                    nmeaWork.sensorData.heatingVoltage = value;
                    nmeaWork.sensorStatus |= NMEA_DONE_HEAT_VOLT;
                    break;
                case 2:
                    // VOLTAGE - reference
                    nmeaWork.sensorData.referenceVoltage = value;
                    nmeaWork.sensorStatus |= NMEA_DONE_REF_VOLT;
                    break;
            }

            break;
        }
        default:
        {
            break;
        }
    }

    return TRUE;
}

// returns the number of transducers processed or ERROR
static int readSensorLine (WVIEWD_WORK *work)
{
    char        buffer[NMEA_BYTE_LENGTH_MAX];
    int         retVal, argIndex, done;
    PARSER_ID   parser;

    // read a line from the station
    if (!readLineFromStation (work, buffer, NMEA_RESPONSE_TIMEOUT))
    {
        return ERROR;
    }

    // check the checksum integrity, this will truncate it as well
    if (!verifyChecksum(buffer))
    {
        // corruption?
        radMsgLog (PRI_MEDIUM, "NMEA: readSensorLine: verifyChecksum failed!");
        emailAlertSend(ALERT_TYPE_STATION_READ);
        return ERROR;
    }

//radMsgLog(PRI_STATUS,"DBG: RX: %s", buffer);

    // now parse this mother!
    parser = parserInit (buffer, NMEA_DELIMITERS);
    if (parser == NULL)
    {
        radMsgLog (PRI_MEDIUM, "NMEA: readSensorLine: parserInit failed!");
        return ERROR;
    }

    // make sure it is the right kind of monkey...
    if (strcmp (parserGetFirst(parser), NMEA_WIXDR_ID))
    {
        // nope!
        radMsgLog (PRI_MEDIUM, "NMEA: readSensorLine: NOT a WIXDR response!");
        parserExit (parser);
        return ERROR;
    }

    // now break it up by transducer
    done = FALSE;
    retVal = 0;
    argIndex = 2;
    while (!done)
    {
        if (processTransducer(parser, argIndex) == FALSE)
        {
            done = TRUE;
            continue;
        }

        retVal += 1;
        argIndex += 4;                  // there are 4 fields per transducer
    }

    parserExit (parser);
    return retVal;
}



///////////////////////////////////////////////////////////////////////////
///////////////////////////////  A P I  ///////////////////////////////////
int nmea0183Init (WVIEWD_WORK *work)
{
    char            buffer[64];

    memset (&nmeaWork, 0, sizeof(nmeaWork));

    /////// Wakeup/Clear the WXT-510 ///////
    // send a <CR><LF>
    buffer[0] = 0;
    if (nmea0183WriteLineToStation (work, buffer, NULL, FALSE, TRUE) == ERROR)
    {
        radMsgLog (PRI_MEDIUM, "NMEA: nmea0183Init: W/C failed!");
        return ERROR;
    }
    radUtilsSleep (50);

    // send a ? command
    sprintf (buffer, "?");
    if (nmea0183WriteLineToStation (work, buffer, NULL, FALSE, TRUE) == ERROR)
    {
        radMsgLog (PRI_MEDIUM, "NMEA: nmea0183Init: %s failed!", buffer);
        return ERROR;
    }
    radUtilsSleep (50);


    /////// Setup the WXT-510 to send sensor data in the format we want ///////
    // Note: These setup commands are not NMEA 0183, rather WXT-510 ASCII
    //       commands - there are no NMEA equivalents...

    // --- Comm Settings ---
    // set the device address to 0, RS232, NMEA polled, inter-message delay 20ms
    sprintf (buffer, "0XU,A=0,M=Q,C=2,L=20");
    if (nmea0183WriteLineToStation (work, buffer, buffer, FALSE, FALSE) == ERROR)
    {
        radMsgLog (PRI_MEDIUM, "NMEA: nmea0183Init: %s failed!", buffer);
        return ERROR;
    }
    radUtilsSleep (500);

    // set error messages OFF
    sprintf (buffer, "0SU,S=N");
    if (nmea0183WriteLineToStation (work, buffer, buffer, FALSE, FALSE) == ERROR)
    {
        radMsgLog (PRI_MEDIUM, "NMEA: nmea0183Init: %s failed!", buffer);
        return ERROR;
    }
    radUtilsSleep (50);


    // send a reset command
    sprintf (buffer, "0XZ");
    if (nmea0183WriteLineToStation (work, buffer, NULL, FALSE, TRUE) == ERROR)
    {
        radMsgLog (PRI_MEDIUM, "NMEA: nmea0183Init: %s failed!", buffer);
        return ERROR;
    }

    // wait to let the WXT-510 reset
    radUtilsSleep (500);

    // send a ? command
    sprintf (buffer, "?");
    if (nmea0183WriteLineToStation (work, buffer, "0", FALSE, FALSE) == ERROR)
    {
        radMsgLog (PRI_MEDIUM, "NMEA: nmea0183Init: %s failed!", buffer);
        return ERROR;
    }
    radUtilsSleep (50);


    // --- Wind sensors ---
    // turn on average and max direction and speed
    sprintf (buffer, "0WU,R=0110110001101100");
    if (nmea0183WriteLineToStation (work, buffer, "0WU,R=01101100&01101100", FALSE, FALSE)
        == ERROR)
    {
        radMsgLog (PRI_MEDIUM, "NMEA: nmea0183Init: %s failed!", buffer);
        return ERROR;
    }
    radUtilsSleep (25);

    // set update and averaging interval to 2 seconds for initial readings
    sprintf (buffer, "0WU,I=2,A=2");
    if (nmea0183WriteLineToStation (work, buffer, buffer, FALSE, FALSE) == ERROR)
    {
        radMsgLog (PRI_MEDIUM, "NMEA: nmea0183Init: %s failed!", buffer);
        return ERROR;
    }
    radUtilsSleep (25);

    // set units to MPH, direction correction to 0, response format type T
    sprintf (buffer, "0WU,U=S,D=0,N=T");
    if (nmea0183WriteLineToStation (work, buffer, buffer, FALSE, FALSE) == ERROR)
    {
        radMsgLog (PRI_MEDIUM, "NMEA: nmea0183Init: %s failed!", buffer);
        return ERROR;
    }
    radUtilsSleep (25);

    // --- THP sensors ---
    // turn on air pressure, temperature and humidity
    sprintf (buffer, "0TU,R=1101000011010000");
    if (nmea0183WriteLineToStation (work, buffer, "0TU,R=11010000&11010000", FALSE, FALSE)
        == ERROR)
    {
        radMsgLog (PRI_MEDIUM, "NMEA: nmea0183Init: %s failed!", buffer);
        return ERROR;
    }
    radUtilsSleep (25);

    // set pressure units to in/Hg, temperature units to Fahrenheit
    sprintf (buffer, "0TU,P=I,T=F");
    if (nmea0183WriteLineToStation (work, buffer, buffer, FALSE, FALSE) == ERROR)
    {
        radMsgLog (PRI_MEDIUM, "NMEA: nmea0183Init: %s failed!", buffer);
        return ERROR;
    }
    radUtilsSleep (25);

    // --- Precip sensors ---
    // turn on rain/hail amount and intensity
    // sprintf (buffer, "0RU,R=1011010010110100");
    // 0RU,R=11111111&11111111,I=60,U=I,S=I,M=T,Z=M
    // if (nmea0183WriteLineToStation (work, buffer, "0RU,R=10110100&10110100", FALSE, FALSE)

    sprintf (buffer, "0RU,R=1111111111111111");
    if (nmea0183WriteLineToStation (work, buffer, "0RU,R=11111111&11111111", FALSE, FALSE)
        == ERROR)
    {
        radMsgLog (PRI_MEDIUM, "NMEA: nmea0183Init: %s failed!", buffer);
        return ERROR;
    }
    radUtilsSleep (25);

    // set units to imperial, manual reset of the counters
    // sprintf (buffer, "0RU,U=I,S=I,Z=M");
    // NMEA: nmea0183WriteLineToStation: Expect:0RU,I=60,U=I,M=T,S=I,Z=M, Recv:0RU,I=60,U=I,S=I,M=T,Z=M
    sprintf (buffer, "0RU,I=60,U=I,S=I,M=T,Z=M");
    if (nmea0183WriteLineToStation (work, buffer, buffer, FALSE, FALSE) == ERROR)
    {
        radMsgLog (PRI_MEDIUM, "NMEA: nmea0183Init: %s failed!", buffer);
        return ERROR;
    }
    radUtilsSleep (25);

    // --- Supervisor message ---
    // turn on everything
    sprintf (buffer, "0SU,R=1111000011110000");
    if (nmea0183WriteLineToStation (work, buffer, "0SU,R=11110000&11110000", FALSE, FALSE)
        == ERROR)
    {
        radMsgLog (PRI_MEDIUM, "NMEA: nmea0183Init: %s failed!", buffer);
        return ERROR;
    }
    radUtilsSleep (25);

    // set heating control ON
    sprintf (buffer, "0SU,H=Y");
    if (nmea0183WriteLineToStation (work, buffer, buffer, FALSE, FALSE) == ERROR)
    {
        radMsgLog (PRI_MEDIUM, "NMEA: nmea0183Init: %s failed!", buffer);
        return ERROR;
    }

    // wait here for the first wind cycle to finish, etc.
    radMsgLog (PRI_STATUS, "NMEA: nmea0183Init: waiting 10 seconds to allow the station to settle...");
    radUtilsSleep (10000);

    return OK;
}

int nmea0183PostInit (WVIEWD_WORK *work)
{
    char            buffer[64];

    // set wind averaging interval to one minute
    sprintf (buffer, "0WU,I=60,A=60");
    if (nmea0183WriteLineToStation (work, buffer, buffer, FALSE, FALSE) == ERROR)
    {
        radMsgLog (PRI_MEDIUM, "NMEA: nmea0183PostInit: %s failed!", buffer);
        return ERROR;
    }
    radUtilsSleep (25);

    return OK;
}

void nmea0183Exit (WVIEWD_WORK *work)
{
    // nothing to clean up...
    return;
}

int nmea0183GetReadings (WVIEWD_WORK *work, NMEA0183_DATA *store)
{
    char            buffer[64];
    int             i;

    // request a sensor update
    sprintf (buffer, "$--WIQ,XDR");
    if (nmea0183WriteLineToStation (work, buffer, NULL, TRUE, FALSE) == ERROR)
    {
        radMsgLog (PRI_MEDIUM, "NMEA: nmea0183GetReadings: nmea0183WriteLineToStation failed!");
        return ERROR;
    }

    // clear the status bits so we know when we have received everything
    nmeaWork.sensorStatus = 0;

    // read the 4 sensor output lines:
    // wind, THP, precip, supervisor
    for (i = 0; i < NMEA_XDUCER_MAX_TYPES; i ++)
    {
        if (readSensorLine(work) == ERROR)
        {
            radMsgLog (PRI_MEDIUM, "NMEA: nmea0183GetReadings: readSensorLine %s failed!",
                       TransducerTypes[i]);
            return ERROR;
        }
    }

    // did we get everything?
    if (nmeaWork.sensorStatus < NMEA_DONE_ALL)
    {
        radMsgLog (PRI_MEDIUM, "NMEA: nmea0183GetReadings: 0x%4.4X: not all sensors updated...",
                   nmeaWork.sensorStatus);
        return ERROR;
    }

    // copy the results to the destination
    *store = nmeaWork.sensorData;

    return OK;
}

// send a properly formatted line to the station, optionally checking the response
int nmea0183WriteLineToStation
(
    WVIEWD_WORK     *work,
    char            *strToSend,
    char            *expectedResp,
    int             generateCS,
    int             flushRXBuffer
)
{
    char        temp[NMEA_BYTE_LENGTH_MAX];
    int         length;

    if (generateCS)
    {
        sprintf (temp, "%s%s%c%c",
                 strToSend,
                 generateChecksum(strToSend),
                 NMEA_CR,
                 NMEA_LF);
    }
    else
    {
        sprintf (temp, "%s%c%c",
                 strToSend,
                 NMEA_CR,
                 NMEA_LF);
    }

    length = strlen(temp);
    if ((*work->medium.write) (&work->medium, temp, length) != length)
    {
        radMsgLog (PRI_MEDIUM, "NMEA: nmea0183WriteLineToStation: write error!");
        emailAlertSend(ALERT_TYPE_STATION_DEVICE);
        return ERROR;
    }

    // need to check the response?
    if (expectedResp != NULL)
    {
        // read a line from the station
        if (!readLineFromStation (work, temp, NMEA_RESPONSE_TIMEOUT))
        {
            return ERROR;
        }

        if (generateCS)
        {
            // verify the checksum (and truncate it)
            if (!verifyChecksum(temp))
            {
                // corruption?
                radMsgLog (PRI_MEDIUM, "NMEA: nmea0183WriteLineToStation: verifyChecksum failed!");
                emailAlertSend(ALERT_TYPE_STATION_READ);
                return ERROR;
            }
        }

        // do they match?
        if (!strcmp(expectedResp, temp))
        {
            return OK;
        }
        else
        {
            radMsgLog (PRI_MEDIUM, "NMEA: nmea0183WriteLineToStation: expected response mismatch!");
            radMsgLog (PRI_MEDIUM, "NMEA: nmea0183WriteLineToStation: Expect:%s, Recv:%s",
                       expectedResp, temp);
            emailAlertSend(ALERT_TYPE_STATION_READ);
            return ERROR;
        }
    }
    else if (flushRXBuffer)
    {
        radUtilsSleep (50);
        (*work->medium.flush) (&work->medium, WV_QUEUE_INPUT);
    }

    return OK;
}

int nmea0183ResetAccumulators (WVIEWD_WORK *work)
{
    char            buffer[64];

    // reset rain accumulators
    sprintf (buffer, "0XZRU");
    if (nmea0183WriteLineToStation (work, buffer, NULL, FALSE, TRUE) == ERROR)
    {
        radMsgLog (PRI_MEDIUM, "NMEA: nmea0183ResetAccumulators: %s failed!", buffer);
        return ERROR;
    }
    radUtilsSleep (25);

    return OK;
}
