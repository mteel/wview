/*---------------------------------------------------------------------------
 
  FILENAME:
        windAverage.c
 
  PURPOSE:
        Provide the consensus wind averaging API methods.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        1/31/2004       M.S. Teel       0               Original
 
  NOTES:
        
 
  LICENSE:
        Copyright (c) 2004, Mark S. Teel (mark@teel.ws)
  
        This source code is released for free distribution under the terms 
        of the GNU General Public License.
  
----------------------------------------------------------------------------*/

//  ... System header files
#include <radmsgLog.h>


//  ... Local header files
#include <windAverage.h>



//  ... define methods here

void windAverageReset (WAVG_ID id)
{
    memset (id, 0, sizeof (*id));
}

//  ... add a data point (wind observation) to the data set
void windAverageAddValue (WAVG_ID id, int value)
{
    if (value < 0)
        value = 0;

    value += (WAVG_BIN_SIZE/2);
    value /= WAVG_BIN_SIZE;
    value %= WAVG_NUM_BINS;

    id->bins[value] ++;

    return;
}

//  ... add a set of wind direction bins:
void windAverageAddBins (WAVG_ID id, int* bins)
{
    int     i;

    for (i = 0; i < WAVG_NUM_BINS; i ++)
    {
        id->bins[i] += bins[i];
    }
}

//  ... use consensus averaging to compute the average wind direction
int windAverageCompute (WAVG_ID id)
{
    int         i, j, retVal, maxIndex = 0;
    uint64_t    sum, maxSum = 0;

    //  ... first, fill in the "wrap-around" bins
    for (i = WAVG_NUM_BINS; i < WAVG_TOTAL_BINS; i ++)
    {
        id->bins[i] = id->bins[i - WAVG_NUM_BINS];
    }

    //  ... now, find our best consensus
    for (i = 0; i < WAVG_NUM_BINS; i ++)
    {
        sum = 0;
        for (j = 0; j <= WAVG_CONSENSUS_BINS; j ++)
        {
            sum += id->bins[i + j];
        }

        if (sum > maxSum)
        {
            maxSum = sum;
            maxIndex = i;
        }
    }

    if (maxSum == 0)
    {
        return ARCHIVE_VALUE_NULL;
    }

    //  ... now we know where the best "id->consBins" is,
    //  ... compute the weighted average value
    sum = 0;
    for (i = 0; i <= WAVG_CONSENSUS_BINS; i ++)
    {
        sum += (i * id->bins[maxIndex+i]);
    }
    sum *= WAVG_INTERVAL;
    sum /= maxSum;
    sum += (maxIndex * WAVG_INTERVAL);

    retVal = (int)sum;
    retVal %= 720;
    retVal /= 2;

    return retVal;
}

