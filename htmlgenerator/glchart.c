/*---------------------------------------------------------------------------
 
  FILENAME:
        glchart.c
 
  PURPOSE:
        Provide the graphics lib chart utilities.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        09/05/03        M.S. Teel       0               Original
        01/02/05        M.S. Teel       1               Improve labeling and
                                                        chart limits calc
        05/15/08        Werner Krenn    2               Dual Units
 
  NOTES:
        
 
  LICENSE:
        Copyright (c) 2004, Mark S. Teel (mark@teel.ws)
  
        This source code is released for free distribution under the terms 
        of the GNU General Public License.
  
----------------------------------------------------------------------------*/

/*  ... System include files
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

/*  ... Library include files
*/
#include <sysdefs.h>
#include <radmsgLog.h>
#include "gd.h"
#include "gdfonts.h"
#include "gdfontt.h"
#include "gdfontmb.h"
#include "gdfontg.h"
#include "gdfontl.h"


/*  ... Local include files
*/
#include <glchart.h>


/*  ... global memory declarations
*/

/*  ... global memory referenced
*/

/*  ... static (local) memory declarations
*/
#define MAX_STEP_MULTIPLIER         9

static CHART nonReentrantChart;

static double stepSizeMultipliers[MAX_STEP_MULTIPLIER] =
    {
        2.0, 5.0, 10.0, 20.0, 50.0, 100.0, 200.0, 500.0, 1000.0
    };



static int createImage (CHART_ID id)
{
    if (id->height < (10 * gdFontMediumBold->h))
    {
        return -1;
    }

    if (id->im)
    {
        gdImageDestroy (id->im);
    }

    id->im = gdImageCreate (id->width, id->height);
    if (id->im == NULL)
    {
        return -1;
    }


    //  ... allocate our colors
    id->bgcolor = gdImageColorAllocateAlpha (id->im,
                  GLC_RED(id->bgcolor),
                  GLC_GREEN(id->bgcolor),
                  GLC_BLUE(id->bgcolor),
                  GLC_ALPHA(id->bgcolor));
    if (id->isTransparent)
    {
        gdImageColorTransparent (id->im, id->bgcolor);
        id->chartcolor = id->bgcolor;
        id->titleBGcolor = id->bgcolor;
    }
    else
    {
        id->chartcolor = gdImageColorAllocateAlpha (id->im,
                         GLC_RED(id->chartcolor),
                         GLC_GREEN(id->chartcolor),
                         GLC_BLUE(id->chartcolor),
                         GLC_ALPHA(id->chartcolor));
        id->titleBGcolor = gdImageColorAllocateAlpha (id->im,
                           GLC_RED(id->titleBGcolor),
                           GLC_GREEN(id->titleBGcolor),
                           GLC_BLUE(id->titleBGcolor),
                           GLC_ALPHA(id->titleBGcolor));
    }

    id->gridcolor = gdImageColorAllocateAlpha (id->im,
                    GLC_RED(id->gridcolor),
                    GLC_GREEN(id->gridcolor),
                    GLC_BLUE(id->gridcolor),
                    GLC_ALPHA(id->gridcolor));
    id->linecolor = gdImageColorAllocateAlpha (id->im,
                    GLC_RED(id->linecolor),
                    GLC_GREEN(id->linecolor),
                    GLC_BLUE(id->linecolor),
                    GLC_ALPHA(id->linecolor));
    id->titleFGcolor = gdImageColorAllocateAlpha (id->im,
                       GLC_RED(id->titleFGcolor),
                       GLC_GREEN(id->titleFGcolor),
                       GLC_BLUE(id->titleFGcolor),
                       GLC_ALPHA(id->titleFGcolor));
    id->textcolor = gdImageColorAllocateAlpha (id->im,
                    GLC_RED(id->textcolor),
                    GLC_GREEN(id->textcolor),
                    GLC_BLUE(id->textcolor),
                    GLC_ALPHA(id->textcolor));

    return 0;
}


static void drawGrid (CHART_ID id)
{
    register int    i;
    int             xlabelwidth = 0, ylabelwidth = 0;
    double          units, step;
    double          minR, stepR;
    int             dataPoints, xnumhash, xhashindexlen;
    double          hashwidth, xhashwidth;
    int             numhash, x, y, temp1, temp2;
    char            text[MAX_NUMHASH][WVIEW_STRING1_SIZE], ylabelfmt[32];
    char            textR[MAX_NUMHASH][WVIEW_STRING1_SIZE], ylabelfmtR[32];

    //  build labels and calculate the maximum x-label width
    for (i = 0; i < id->numpoints; i ++)
    {
        if (strlen (id->pointnames[i]) > xlabelwidth)
            xlabelwidth = strlen (id->pointnames[i]);
    }

    //  calculate the maximum number of hash marks and the units per hash
    //  for the x-axis
    xhashwidth = (xlabelwidth*gdFontSmall->h)/2;
    dataPoints = id->numpoints - 1;

    // see if a suggestion was given
    if (id->xnumhashes != 0)
    {
        xnumhash = id->xnumhashes;
    }
    else
    {
        xnumhash =  (id->imbx - id->imtx)/xhashwidth;

        // try to normalize the number of hashmarks
        // try with the endpoint removed
        for (i = 1; i < dataPoints/2; i ++)
        {
            if (dataPoints % i != 0)
                continue;

            if (dataPoints/i <= xnumhash)
            {
                xnumhash = dataPoints/i;
                break;
            }
        }
    }

    if (strlen(id->DualUnit) > 0)
    {
        id->imbx -= (gdFontMediumBold->h);
        if ((strcmp(id->DualUnit,"inHg") == 0) || 
            (strcmp(id->DualUnit,"inches") == 0) || 
            (strcmp(id->DualUnit,"in/day") == 0) || 
            (strcmp(id->DualUnit,"cm/day") == 0) || 
            (strcmp(id->DualUnit,"in/hour") == 0) || 
            (strcmp(id->DualUnit,"in/week") == 0))
        {
            id->imbx -= 7;
        }
    }

    xhashwidth = (double)(id->imbx - id->imtx)/(double)xnumhash;
    id->pointpixels = (double)(id->imbx - id->imtx)/(double)dataPoints;
    xhashindexlen = dataPoints/xnumhash;


    //  calculate the maximum number of hash marks and the units per hash
    //  for the y-axis
    hashwidth = (gdFontSmall->h * 3)/2;

    numhash = (id->imby - id->imty)/hashwidth;
    if (numhash > MAX_NUMHASH)
    {
        numhash = MAX_NUMHASH;
    }

    units = (id->max - id->min)/id->ystepSize;
    step = id->ystepSize;
    i = 0;
    while ((int)units > numhash && i < MAX_STEP_MULTIPLIER)
    {
        step = id->ystepSize * stepSizeMultipliers[i];
        units = (id->max - id->min)/step;
        i ++;
    }

    id->ystepSize = step;
    numhash = (int)units;

    temp1 = (int)(id->max * 10000);
    temp2 = (int)((id->min + (id->ystepSize * numhash)) * 10000);
    temp2 ++;

    if (temp2 < temp1)
    {
        numhash ++;
    }

    hashwidth = (double)(id->imby - id->imty)/(double)numhash;
    id->max = id->min + (id->ystepSize * numhash);
    id->ypixelconstant = (double)(id->imby - id->imty)/(double)(id->max - id->min);
    numhash ++;
    if (numhash > 64)
        numhash = 64;

    //  build labels and calculate the maximum y-label width
    sprintf (ylabelfmt, "%%.%1.1df", id->ydecPlaces);
    for (i = 0; i < numhash; i ++)
    {
        sprintf (text[i], ylabelfmt, id->min + (i * id->ystepSize));

        // now fill the right hand array with alternate scale @Rcm
        if (strlen(id->DualUnit) > 0)
        {
            textR[i][0] = ' ';
            if (strcmp(id->DualUnit,"inHg") == 0)
            {  // if hPa then change to inHg value
                minR = wvutilsConvertHPAToINHG (id->min);
                stepR = wvutilsConvertHPAToINHG (id->ystepSize);
                sprintf (textR[i], "%.1f", minR + (i * stepR));
            }
            else if (strcmp(id->DualUnit,"hPa") == 0)
            {  // if inHg then change to hPa value
                minR = wvutilsConvertINHGToHPA (id->min);
                stepR = wvutilsConvertINHGToHPA (id->ystepSize);
                sprintf (textR[i], "%.0f", minR + (i * stepR));
            }
            else if (strcmp(id->DualUnit,"C") == 0)
            {
                minR = wvutilsConvertFToC (id->min);
                stepR = ((id->ystepSize) * (5.0/9.0));
                sprintf (textR[i], "%.1f", minR + (i * stepR));
            }
            else if (strcmp(id->DualUnit,"F") == 0)
            {
                minR = wvutilsConvertCToF (id->min);
                stepR = ((id->ystepSize) * (9.0/5.0));
                sprintf (textR[i], "%.1f", minR + (i * stepR));
            }
            else if ((strcmp(id->DualUnit,"mm") == 0) ||
                     (strcmp(id->DualUnit,"mm/h") == 0))
            {
                minR = wvutilsConvertINToMM (id->min);
                stepR = wvutilsConvertINToMM (id->ystepSize);
                if (wvutilsConvertINToMM (id->max) >= 100)
                    sprintf (textR[i], "%.0f", minR + (i * stepR));
                else
                    sprintf (textR[i], "%.1f", minR + (i * stepR));
            }
            else if ((strcmp(id->DualUnit,"inches") == 0) ||
                     (strcmp(id->DualUnit,"in/hour") == 0) ||
                     (strcmp(id->DualUnit,"in/hr") == 0))
            {
                minR = wvutilsConvertMMToIN (id->min);
                stepR = wvutilsConvertMMToIN (id->ystepSize);
                if (wvutilsConvertMMToIN (id->max) >= 100)
                    sprintf (textR[i], "%.1f", minR + (i * stepR));
                else
                    sprintf (textR[i], "%.2f", minR + (i * stepR));
            }
            else if (((strcmp(id->DualUnit,"inches") == 0) && (strcmp(id->units,"cm") == 0)) || 
                     (strcmp(id->DualUnit,"in/day") == 0) || (strcmp(id->DualUnit,"in/week") == 0))
            {
                minR = wvutilsConvertCMToIN (id->min);
                stepR = wvutilsConvertCMToIN (id->ystepSize);
                sprintf (textR[i], "%.1f", minR + (i * stepR));
            }
            else if (strcmp(id->DualUnit,"km/h") == 0)
            {
                if (!strcmp(id->units, "mph"))
                {
                    minR = wvutilsConvertMPHToKPH (id->min);
                    stepR = wvutilsConvertMPHToKPH (id->ystepSize);
                    sprintf (textR[i], "%.0f", minR + (i * stepR));
                }
                else if (!strcmp(id->units, "m/s"))
                {
                    minR = wvutilsConvertMPSToKPH (id->min);
                    stepR = wvutilsConvertMPSToKPH (id->ystepSize);
                    sprintf (textR[i], "%.0f", minR + (i * stepR));
                }
                else if (!strcmp(id->units, "knots"))
                {
                    minR = wvutilsConvertKnotsToKPH (id->min);
                    stepR = wvutilsConvertKnotsToKPH (id->ystepSize);
                    sprintf (textR[i], "%.0f", minR + (i * stepR));
                }
            }
            else if (strcmp(id->DualUnit,"mph") == 0)
            {
                if (!strcmp(id->units, "km/h"))
                {
                    minR = wvutilsConvertKPHToMPH (id->min);
                    stepR = wvutilsConvertKPHToMPH (id->ystepSize);
                    sprintf (textR[i], "%.0f", minR + (i * stepR));
                }
                else if (!strcmp(id->units, "m/s"))
                {
                    minR = wvutilsConvertMPSToMPH (id->min);
                    stepR = wvutilsConvertMPSToMPH (id->ystepSize);
                    sprintf (textR[i], "%.0f", minR + (i * stepR));
                }
                else if (!strcmp(id->units, "knots"))
                {
                    minR = wvutilsConvertKnotsToMPH (id->min);
                    stepR = wvutilsConvertKnotsToMPH (id->ystepSize);
                    sprintf (textR[i], "%.0f", minR + (i * stepR));
                }
            }
            else if (strcmp(id->DualUnit,"m/s") == 0)
            {
                if (!strcmp(id->units, "km/h"))
                {
                    minR = wvutilsConvertKPHToMPS (id->min);
                    stepR = wvutilsConvertKPHToMPS (id->ystepSize);
                    sprintf (textR[i], "%.0f", minR + (i * stepR));
                }
                else if (!strcmp(id->units, "mph"))
                {
                    minR = wvutilsConvertMPHToMPS (id->min);
                    stepR = wvutilsConvertMPHToMPS (id->ystepSize);
                    sprintf (textR[i], "%.0f", minR + (i * stepR));
                }
                else if (!strcmp(id->units, "knots"))
                {
                    minR = wvutilsConvertKnotsToMPS (id->min);
                    stepR = wvutilsConvertKnotsToMPS (id->ystepSize);
                    sprintf (textR[i], "%.0f", minR + (i * stepR));
                }
            }
            else if (strcmp(id->DualUnit,"knots") == 0)
            {
                if (!strcmp(id->units, "km/h"))
                {
                    minR = wvutilsConvertKPHToKnots (id->min);
                    stepR = wvutilsConvertKPHToKnots (id->ystepSize);
                    sprintf (textR[i], "%.0f", minR + (i * stepR));
                }
                else if (!strcmp(id->units, "mph"))
                {
                    minR = wvutilsConvertMPHToKnots (id->min);
                    stepR = wvutilsConvertMPHToKnots (id->ystepSize);
                    sprintf (textR[i], "%.0f", minR + (i * stepR));
                }
                else if (!strcmp(id->units, "m/s"))
                {
                    minR = wvutilsConvertMPSToKnots (id->min);
                    stepR = wvutilsConvertMPSToKnots (id->ystepSize);
                    sprintf (textR[i], "%.0f", minR + (i * stepR));
                }
            }
            // all else: eg '%' or watts/m^2 (hum/rad)
            // same value both sides
            else
            {
                wvstrncpy (textR[i], text[i], sizeof(textR[i]));
            }
            if (strlen (text[i]) > ylabelwidth)
                ylabelwidth = strlen (text[i])+2;
        }
        else
        {
            if (strlen (text[i]) > ylabelwidth)
                ylabelwidth = strlen (text[i]);
        }
    }

    //  now we know the chart area, draw it
    gdImageFilledRectangle (id->im,
                            id->imtx - GLC_CONTENTS_OFFSET,
                            id->imty - GLC_CONTENTS_OFFSET,
                            id->imbx + GLC_CONTENTS_OFFSET,
                            id->imby + GLC_CONTENTS_OFFSET,
                            id->chartcolor);

    gdImageRectangle (id->im,
                      id->imtx,
                      id->imty,
                      id->imbx,
                      id->imby,
                      id->gridcolor);

    //  draw the y-axis
    gdImageSetThickness (id->im, 1);
    for (i = 0; i < numhash; i ++)
    {
        y = id->imby - (i * hashwidth);

        gdImageLine (id->im,
                     id->imtx,
                     y,
                     id->imbx,
                     y,
                     id->gridcolor);

        y -= gdFontSmall->h/2;
        gdImageString (id->im,
                       gdFontSmall,
                       (id->imtx - 5) - (strlen(text[i]) * gdFontSmall->h/2),
                       y,
                       (uint8_t *)text[i],
                       id->textcolor);

        if (strlen(id->DualUnit) > 0)
        {
            // right side
            x = id->imbx + (GLC_CONTENTS_OFFSET - 2);

            gdImageString (id->im,
                           gdFontSmall,
                           x + gdFontSmall->h/2,
                           y,
                           (uint8_t *)textR[i],
                           id->textcolor);
        }
    }

    //  draw the x-axis
    gdImageSetThickness (id->im, 1);

    for (i = 0; i <= xnumhash; i ++)            // one more for right-most
    {
        int         dataPoint = i * xhashindexlen;

        x = id->imtx + (i * xhashwidth);

        gdImageLine (id->im,
                     x,
                     id->imty,
                     x,
                     id->imby,
                     id->gridcolor);

        if (strlen(id->pointnames[dataPoint]) < 2)
            x -= gdFontSmall->h/4;

        gdImageString (id->im,
                       gdFontSmall,
                       x - ((gdFontSmall->h/2) *
                            (strlen(id->pointnames[dataPoint])/2)),
                       (id->height - (3 * gdFontMediumBold->h)) + 4,
                       (uint8_t *)id->pointnames[dataPoint],
                       id->textcolor);
    }


    //  finally, draw date
    gdImageString (id->im,
                   gdFontSmall,
                   ((id->width - ((gdFontSmall->h/2) * strlen (id->datetime)))/2) - 2,
                   id->height - (gdFontMediumBold->h + 2),
                   (uint8_t *)id->datetime,
                   id->textcolor);
}

static void drawTitle (CHART_ID id)
{
    gdImageFilledRectangle (id->im,
                            0, 0,
                            id->width, gdFontMediumBold->h,
                            id->titleBGcolor);

    if (strlen (id->DualUnit) > 0)
    {
        gdImageString (id->im,
                       gdFontMediumBold,
                       ((id->width - ((gdFontMediumBold->h/2) * strlen (id->title)))/2) - 3,
                       2,
                       (uint8_t *)id->title,
                       id->linecolor);

        gdImageString (id->im,
                       gdFontMediumBold,
                       id->width - (((gdFontMediumBold->h/2)+2) * strlen (id->DualUnit)),
                       0,
                       (uint8_t *)id->DualUnit,
                       id->titleFGcolor);

        gdImageString (id->im,
                       gdFontMediumBold,
                       2,
                       0,
                       (uint8_t *)id->units,
                       id->titleFGcolor);
    }
    else
    {
        gdImageString (id->im,
                       gdFontMediumBold,
                       2,
                       0,
                       (uint8_t *)id->title,
                       id->linecolor);

        gdImageString (id->im,
                       gdFontMediumBold,
                       id->width - (((gdFontMediumBold->h/2)+2) * strlen (id->units)),
                       0,
                       (uint8_t *)id->units,
                       id->titleFGcolor);
    }
}

static void drawLine (CHART_ID id)
{
    register int    i;
    int             x1, x2, y1, y2;

    gdImageSetThickness (id->im, 2);
    for (i = 0; i < id->numpoints - 1; i ++)
    {
        if (id->valueset[i] <= ARCHIVE_VALUE_NULL || id->valueset[i+1] <= ARCHIVE_VALUE_NULL)
        {
            continue;
        }

        x1 = id->imtx + (int)((double)i * id->pointpixels);
        x2 = id->imtx + (int)((double)(i+1) * id->pointpixels);
        y1 = id->imby - (int)(id->ypixelconstant * (id->valueset[i] - id->min));
        y2 = id->imby - (int)(id->ypixelconstant * (id->valueset[i+1] - id->min));
        gdImageLine (id->im,
                     x1,
                     y1,
                     x2,
                     y2,
                     id->linecolor);
    }
}


static void drawBars (CHART_ID id)
{
    register int    i;
    int             x1, x2, y1, y2;

    for (i = 0; i < id->numpoints - 1; i ++)
    {
        if (id->valueset[i] <= ARCHIVE_VALUE_NULL)
        {
            continue;
        }

        x1 = id->imtx + (int)((double)i * id->pointpixels);
        x2 = id->imtx + (int)((double)(i+1) * id->pointpixels);
        x1 += 1;
        x2 -= 1;
        y1 = id->imby - (int)(id->ypixelconstant * (id->valueset[i] - id->min));
        y2 = id->imby;
        gdImageFilledRectangle (id->im,
                                x1, y1,
                                x2, y2,
                                id->linecolor);
    }
}

static void drawScatter (CHART_ID id)
{
    register int    i;
    int             x1, y1;

    gdImageSetThickness (id->im, 2);
    for (i = 0; i < id->numpoints; i ++)
    {
        if (id->valueset[i] <= ARCHIVE_VALUE_NULL)
        {
            continue;
        }

        x1 = id->imtx + (int)((double)i * id->pointpixels);
        y1 = id->imby - (int)(id->ypixelconstant * (id->valueset[i] - id->min));
        gdImageFilledEllipse (id->im,
                     x1,
                     y1,
                     2,
                     2,
                     id->linecolor);
    }
}


static void normalizeMinMax (CHART_ID id, double newMIN, double newMAX)
{
    register double     temp, factor = 1.0;
    register int        i;

    // compute factor based on number of decimal places defined
    // (avoid use of pow function)
    for (i = 0; i < id->ydecPlaces; i ++)
    {
        factor *= 10.0;
    }

    // determine chart min
    id->min = newMIN;
    id->min *= factor;

    // make sure we don't lop off decimal places "below" since we are
    // computing a MIN here
    temp = (int)id->min;
    if (temp - id->min > 0)
        temp -= 1.0;
    id->min = temp;
    while (((int)id->min % 5) != 0)
    {
        id->min -= 1.0;
    }
    id->min /= factor;

    // determine chart max
    id->max = newMAX;
    id->max *= factor;
    id->max *= 10;
    id->max = (int)id->max;
    id->max /= 10;

    // make sure we don't lop off decimal places "above" since we are
    // computing a MAX here
    temp = (int)id->max;
    if (id->max - temp > 0)
        temp += 1.0;
    id->max = temp;
    while (((int)id->max % 5) != 0)
    {
        id->max += 1.0;
    }
    id->max /= factor;

    return;
}



/*  ... API methods
*/

CHART_ID chartCreate
(
    int                 width,
    int                 height,
    char                *title,
    char                *units,
    CHART_TYPE          chartType
)
{
    register CHART_ID   newId;

    newId = (CHART_ID)&nonReentrantChart;

    memset (newId, 0, sizeof (*newId));

    newId->chartType = chartType;
    newId->width = width;
    newId->height = height;
    wvstrncpy (newId->title, title, sizeof(newId->title));
    wvstrncpy (newId->units, units, sizeof(newId->units));

    //  ... now set the contents coords
    newId->imtx = 5 * gdFontSmall->h/2;
    //  ... SSM: move the right edge of the graph 5 pixels left to make
    //  ...      room for last label
    newId->imbx = (newId->width - 1) - gdFontMediumBold->h - 5;
    newId->imty = 2 * gdFontMediumBold->h;
    newId->imby = newId->height - (3 * gdFontMediumBold->h);

    newId->bgcolor      = GLC_DFLT_BG;
    newId->chartcolor   = GLC_DFLT_CHART;
    newId->gridcolor    = GLC_DFLT_GRID;
    newId->linecolor    = GLC_DFLT_LINE;
    newId->titleFGcolor = GLC_DFLT_TITLEFG;
    newId->titleBGcolor = GLC_DFLT_TITLEBG;
    newId->textcolor    = GLC_DFLT_TEXT;

    return newId;
}

void chartSetXScale (CHART_ID id, int decPlaces)
{
    id->xdecPlaces = decPlaces;

    return;
}

void chartSetXHashes (CHART_ID id, int numHashes)
{
    id->xnumhashes = numHashes;

    return;
}


void chartSetDualUnits (CHART_ID id)
{
    htmlutilsSetDualUnits (id->isMetric, id->units, id->DualUnit);
    return;
}


void chartSetYScale (CHART_ID id, double min, double max, double step, int decPlaces)
{
    id->min = min;
    id->max = max;
    id->ystepSize = step;
    id->ydecPlaces = decPlaces;

    normalizeMinMax (id, min, max);
    return;
}

void chartAddPoint (CHART_ID id, double value, char *name)
{
    if (id->numpoints >= MAX_GRAPH_POINTS)
    {
        return;
    }

    id->valueset[id->numpoints] = value;
    wvstrncpy (id->pointnames[id->numpoints], name, 16);
    id->numpoints ++;

    if (value <= ARCHIVE_VALUE_NULL)
    {
        return;
    }

    if (value < id->min)
        normalizeMinMax (id, value, id->max);
    else if (value > id->max)
        normalizeMinMax (id, id->min, value);

    return;
}

void chartSetDateTime (CHART_ID id, char *datetime)
{
    wvstrncpy (id->datetime, datetime, sizeof(id->datetime));
    return;
}

void chartSetBGColor (CHART_ID id, int color)
{
    id->bgcolor = color;
    return;
}

void chartSetChartColor (CHART_ID id, int color)
{
    id->chartcolor = color;
    return;
}

void chartSetGridColor (CHART_ID id, int color)
{
    id->gridcolor = color;
    return;
}

void chartSetLineColor (CHART_ID id, int color)
{
    id->linecolor = color;
    return;
}

void chartSetTitleColors (CHART_ID id, int fg, int bg)
{
    id->titleFGcolor = fg;
    id->titleBGcolor = bg;
    return;
}

void chartSetTextColor (CHART_ID id, int color)
{
    id->textcolor = color;
    return;
}

void chartSetTransparency (CHART_ID id, int isTransparent)
{
    id->isTransparent = isTransparent;
    return;
}

int chartRender (CHART_ID id)
{
    if (createImage (id) == -1)
    {
        return -1;
    }

    drawTitle (id);
    drawGrid (id);

    switch (id->chartType)
    {
        case CHART_TYPE_BAR:
            drawBars(id);
            break;
        case CHART_TYPE_LINE:
            drawLine(id);
            break;
        case CHART_TYPE_SCATTER:
            drawScatter(id);
            break;
    }

    return 0;
}

int chartSave (CHART_ID id, char *name)
{
    FILE    *out;

    if ((out = fopen (name, "wb")) != NULL)
    {
        gdImagePng (id->im, out);
        fclose (out);
        return 0;
    }
    else
    {
        return -1;
    }
}

void chartDestroy (CHART_ID id)
{
    if (id->im)
        gdImageDestroy (id->im);

    return;
}
