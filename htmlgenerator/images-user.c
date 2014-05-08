/*---------------------------------------------------------------------------

  FILENAME:
        images-user.c

  PURPOSE:
        Provide the user-defined wview image generator utilities.

  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        01/20/04        M.S. Teel       0               Original

  NOTES:
        

  LICENSE:
        Copyright (c) 2004, Mark S. Teel (mark@teel.ws)
  
        This source code is released for free distribution under the terms 
        of the GNU General Public License.
  
----------------------------------------------------------------------------*/

/*  ... System include files
*/
#include <termios.h>

/*  ... Library include files
*/
#include <radtimeUtils.h>

/*  ... Local include files
*/
#include <services.h>
#include <html.h>
#include <htmlMgr.h>
#include <htmlGenerate.h>
#include <glbucket.h>
#include <images.h>


/*  ... global memory declarations
*/

/*  ... global memory referenced
*/
extern char sampleLabels[MAX_DAILY_NUM_VALUES-1][8];

/*  ... (local) memory declarations
*/
static char *labels[MONTHLY_NUM_VALUES];

static char *monthLabels[12] =
{
    "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"
};


//  ... !!!  Define the image generator engines  !!! ...
//  ....................................................

static int generateInTemp (HTML_IMG *img)
{
    char        temp[256];
    
    sprintf (temp, "%s/%s", img->mgrWork->imagePath, img->fname);
    
    htmlGenPngBucket (temp,
                      img->mgrWork->isMetricUnits,
                      img->mgrWork->loopStore.inTemp,
                      sensorGetLow(&img->mgrWork->hilowStore.sensor[STF_DAY][SENSOR_INTEMP]),
                      sensorGetHigh(&img->mgrWork->hilowStore.sensor[STF_DAY][SENSOR_INTEMP]),
                      img->title,
                      img->units,
                      img->decimalPlaces,
                      0.0,
                      99.0,
                      img->mgrWork->dateFormat,
                      img->mgrWork->isDualUnits);
    
    return OK;
}


    
//  ... this is the user-defined generator jump table;
//  ... Indexes from images-user.conf are used to index into this table -
//  ... Thus, order is VERY important...
//  ... Add custom generators here and call them out by index in 
//  ... images-user.conf
int (*user_generators[]) (HTML_IMG *img) = 
{
    generateInTemp,         // 0
    NULL
};


