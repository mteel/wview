/*---------------------------------------------------------------------------

  FILENAME:
        twiProtocol.c

  PURPOSE:
        Provide protocol utilities for TWI station communication.

  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        12/14/2009      M.S. Teel       0               Original

  NOTES:

  LICENSE:
        Copyright (c) 2009, Mark S. Teel (mark@teel.ws)

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
#include <twiProtocol.h>

/*  ... global memory declarations
*/

/*  ... local memory
*/

static TWI_WORK             twiWork;



// read a line from the station
// returns TRUE if a line was read, FALSE otherwise
static int readLineFromStation (WVIEWD_WORK *work, char *store, int msTime)
{
    char        buffer[TWI_BYTE_LENGTH_MAX];
    uint64_t   readTime = radTimeGetMSSinceEpoch() + (uint64_t)msTime;
    int         retVal, timeToRead, done = FALSE, byteCount = 0;

    // we read until we see an "R" as a line terminator:
    while ((timeToRead = (uint32_t)(readTime - radTimeGetMSSinceEpoch())) > 0)
    {
        retVal = (*work->medium.read)(&work->medium, &buffer[byteCount], 1, timeToRead);
        if (retVal != 1)
        {
            radMsgLog (PRI_MEDIUM, "TWI: readLineFromStation: read failed!");
            (*work->medium.flush)(&work->medium, WV_QUEUE_INPUT);
            emailAlertSend(ALERT_TYPE_STATION_READ);
            return FALSE;
        }
        byteCount ++;

        if (buffer[byteCount-1] == '%')
        {
            // replace the pesky '%':
            buffer[byteCount-1] = 'P';
        }

        // are we done?
        if ((buffer[byteCount-2] == TWI_CR) && (buffer[byteCount-1] == TWI_LF))
        {
            // we are!
            done = TRUE;
            break;
        }
    }

    if (! done)
    {
        // ran out of time or empty line
        radMsgLog (PRI_MEDIUM, "TWI: readLineFromStation: timeout or empty line!");
        (*work->medium.flush)(&work->medium, WV_QUEUE_INPUT);
        emailAlertSend(ALERT_TYPE_STATION_READ);
        return FALSE;
    }

    // now lose the <CR> and <LF>
    byteCount -= 2;
    buffer[byteCount] = 0;
    wvstrncpy(store, buffer, TWI_BYTE_LENGTH_MAX);
    return TRUE;
}

// processes a data item:
// returns TRUE if the item is found, otherwise FALSE
static int processDataItem(PARSER_ID parser, int startIndex)
{
    char            *retStr, *tempPtr;

    // load up the fields properly
    retStr = parserGetNumber (parser, startIndex);
    if (retStr == NULL)
    {
        return FALSE;
    }

    // now we switch on the item index:
    switch (startIndex)
    {
        case TWI_DATA_WIND_DIR:
            twiWork.sensorData.windDir = wvutilsConvertWindStrToDegrees(retStr);
            if (twiWork.sensorData.windDir < 0)
            {
                return FALSE;
            }
            break;

        case TWI_DATA_WIND_SPD:
            tempPtr = strcasestr(retStr, "MPH");
            if (tempPtr == NULL)
            {
                return FALSE;
            }
            tempPtr[0] = 0;
            twiWork.sensorData.windSpeed = atof(retStr);
            break;

        case TWI_DATA_TEMP_AUX:
            tempPtr = strcasestr(retStr, "F");
            if (tempPtr == NULL)
            {
                return FALSE;
            }
            tempPtr[0] = 0;
            if (retStr[1] == '-')
            {
                retStr[1] = retStr[0];
                retStr[0] = '-';
            }
            twiWork.sensorData.auxTemp = atof(retStr);
            break;

        case TWI_DATA_TEMP_IN:
            tempPtr = strcasestr(retStr, "F");
            if (tempPtr == NULL)
            {
                return FALSE;
            }
            tempPtr[0] = 0;
            if (retStr[1] == '-')
            {
                retStr[1] = retStr[0];
                retStr[0] = '-';
            }
            twiWork.sensorData.inTemp = atof(retStr);
            break;

        case TWI_DATA_TEMP_OUT:
            tempPtr = strcasestr(retStr, "F");
            if (tempPtr == NULL)
            {
                return FALSE;
            }
            tempPtr[0] = 0;
            if (retStr[1] == '-')
            {
                retStr[1] = retStr[0];
                retStr[0] = '-';
            }
            twiWork.sensorData.outTemp = atof(retStr);
            break;

        case TWI_DATA_HUMIDITY:
            tempPtr = strcasestr(retStr, "P");
            if (tempPtr == NULL)
            {
                return FALSE;
            }
            tempPtr[0] = 0;
            twiWork.sensorData.humidity = atof(retStr);
            break;

        case TWI_DATA_BAROM:
            tempPtr = strcasestr(retStr, "F");
            if (tempPtr == NULL)
            {
                tempPtr = strcasestr(retStr, "R");
                if (tempPtr == NULL)
                {
                    tempPtr = strcasestr(retStr, "S");
                    if (tempPtr == NULL)
                    {
                        return FALSE;
                    }
                }
            }
            tempPtr[0] = 0;
            twiWork.sensorData.bp = atof(retStr);
            break;

        case TWI_DATA_RAIN_DAY:
            tempPtr = strcasestr(retStr, "D");
            if (tempPtr == NULL)
            {
                return FALSE;
            }
            tempPtr[0] = 0;
            twiWork.sensorData.dayrain = atof(retStr);
            break;

        case TWI_DATA_RAIN_MONTH:
            tempPtr = strcasestr(retStr, "M");
            if (tempPtr == NULL)
            {
                return FALSE;
            }
            tempPtr[0] = 0;
            twiWork.sensorData.monthrain = atof(retStr);
            break;

        case TWI_DATA_RAIN_RATE:
            tempPtr = strcasestr(retStr, "R");
            if (tempPtr == NULL)
            {
                return FALSE;
            }
            tempPtr[0] = 0;
            twiWork.sensorData.rainrate = atof(retStr);
            break;

        default:
        {
            return FALSE;
        }
    }

    return TRUE;
}

// returns OK or ERROR
int twiProtocolReadLine (WVIEWD_WORK *work)
{
    char        buffer[TWI_BYTE_LENGTH_MAX];
    int         argIndex;
    PARSER_ID   parser;

    // read a line from the station
    if (! readLineFromStation(work, buffer, TWI_RESPONSE_TIMEOUT))
    {
        radMsgLog(PRI_HIGH, "TWI: twiProtocolReadLine: readLineFromStation failed!");
        return ERROR;
    }

//radMsgLog(PRI_STATUS,"$$$$==> TWI: RX: %s", buffer);

    // now parse this mother!
    parser = parserInit(buffer, TWI_DELIMITERS);
    if (parser == NULL)
    {
        radMsgLog (PRI_HIGH, "TWI: twiProtocolReadLine: parserInit failed!");
        return ERROR;
    }

    // now break it up by data item:
    // skip time and date:
    for (argIndex = TWI_DATA_WIND_DIR; argIndex <= TWI_DATA_RAIN_RATE; argIndex ++) 
    {
        if (processDataItem(parser, argIndex) == FALSE)
        {
            radMsgLog(PRI_HIGH, "TWI: twiProtocolReadLine: parse of data item %d failed!",
                      argIndex);
            parserExit(parser);
            return ERROR;
        }
    }

    parserExit(parser);
    return OK;
}



///////////////////////////////////////////////////////////////////////////
///////////////////////////////  A P I  ///////////////////////////////////
int twiProtocolInit (WVIEWD_WORK *work)
{
    memset (&twiWork, 0, sizeof(twiWork));

    return OK;
}

int twiProtocolPostInit (WVIEWD_WORK *work)
{
    return OK;
}

void twiProtocolExit (WVIEWD_WORK *work)
{
    // nothing to clean up...
    return;
}

int twiProtocolGetReadings (WVIEWD_WORK *work, TWI_DATA *store)
{
    // request a data update with rainrate:
    if (twiProtocolWriteLineToStation (work, "R", FALSE) == ERROR)
    {
        radMsgLog (PRI_MEDIUM, "TWI: twiProtocolGetReadings: twiProtocolWriteLineToStation failed!");
        return ERROR;
    }

    radUtilsSleep(50);

    // read the data:
    if (twiProtocolReadLine(work) == ERROR)
    {
        radMsgLog (PRI_MEDIUM, "TWI: twiProtocolGetReadings: twiProtocolReadLine failed!");
        return ERROR;
    }

    // copy the results to the destination
    *store = twiWork.sensorData;

    return OK;
}

// send a properly formatted line to the station:
int twiProtocolWriteLineToStation
(
    WVIEWD_WORK     *work,
    char            *strToSend,
    int             flushRXBuffer
)
{
    char        temp[TWI_BYTE_LENGTH_MAX];
    int         length;

    sprintf (temp, "%s", strToSend);
    length = strlen(temp);
    if ((*work->medium.write)(&work->medium, temp, length) != length)
    {
        radMsgLog (PRI_MEDIUM, "TWI: twiProtocolWriteLineToStation: write error!");
        emailAlertSend(ALERT_TYPE_STATION_DEVICE);
        return ERROR;
    }

    if (flushRXBuffer)
    {
        radUtilsSleep (50);
        (*work->medium.flush)(&work->medium, WV_QUEUE_INPUT);
    }

    return OK;
}

