/*---------------------------------------------------------------------------
 
  FILENAME:
        stormRain.c
 
  PURPOSE:
        Provide utilities to compute "storm" rain and start date.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        01/08/2006      M.S. Teel       0               Original
 
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

/*  ... Local include files
*/
#include <stormRain.h>

/*  ... global memory declarations
*/

/*  ... local memory
*/
static STORMRAIN_WORK       stormWork;


static void arcRecordCallback(ARCHIVE_PKT* rec, void* data)
{
    float           rate, rainfall;
    int             diffMins;
    STORMRAIN_WORK* sWork = (STORMRAIN_WORK*)data;

    rainfall = (float)rec->value[DATA_INDEX_rain];
    if (rainfall <= ARCHIVE_VALUE_NULL)
    {
        // NULL rain in this record:
        return;
    }
    rate = (float)rec->value[DATA_INDEX_rainRate];
    if (rate <= ARCHIVE_VALUE_NULL)
    {
        // NULL rate in this record:
        return;
    }

    if (!sWork->inStorm)
    {
        if (rate >= sWork->startTrigger)
        {
            // we have a start time
            sWork->rain = rainfall;
            sWork->startTime = sWork->lastRainTime = rec->dateTime;
            sWork->inStorm = TRUE;
        }
    }
    else
    {
        if (rainfall != 0)
        {
            // update the rain total, etc.
            sWork->rain += rainfall;
            sWork->lastRainTime = rec->dateTime;
        }
        else
        {
            // see if the storm is over
            diffMins = (rec->dateTime - sWork->lastRainTime)/60;
            if (diffMins >= (60*sWork->idleHours))
            {
                // storm is over...
                sWork->rain = 0;
                sWork->startTime = sWork->lastRainTime = 0;
                sWork->inStorm = FALSE;
            }
        }
    }
}

void stormRainInit (float startTrigger, int idleHours)
{
    time_t          start, end;
    ARCHIVE_PKT     arcRecord;
    char            select[256];

    memset (&stormWork, 0, sizeof(stormWork));
    stormWork.startTrigger = startTrigger;
    stormWork.idleHours = idleHours;

    sprintf(select, "rain,rainRate");

    // get our search start time/date
    start = end = time(NULL);
    start -= WV_SECONDS_IN_WEEK;                    // go back one week

    dbsqliteArchiveExecutePerRecord(arcRecordCallback,  
                                    (void*)&stormWork, 
                                    start, 
                                    end, 
                                    select);

    return;
}

void stormRainUpdate (float rainRate, float rainFall)
{
    time_t          ntime = time(NULL);
    int             diffMins;

    if (!stormWork.inStorm)
    {
        if (rainRate >= stormWork.startTrigger)
        {
            // we have a start time
            stormWork.rain = rainFall;
            stormWork.startTime = stormWork.lastRainTime = ntime;
            stormWork.inStorm = TRUE;
        }
    }
    else
    {
        if (rainFall != 0)
        {
            // update the rain total, etc.
            stormWork.rain += rainFall;
            stormWork.lastRainTime = ntime;
        }
        else
        {
            // see if the storm is over
            diffMins = (int)(ntime - stormWork.lastRainTime);
            diffMins /= 60;
            if (diffMins >= (60*stormWork.idleHours))
            {
                // storm is over...
                stormWork.rain = 0;
                stormWork.startTime = stormWork.lastRainTime = (time_t)0;
                stormWork.inStorm = FALSE;
            }
        }
    }

    return;
}

time_t stormRainGetStartTimeT (void)
{
    return stormWork.startTime;
}

float stormRainGet (void)
{
    return stormWork.rain;
}

