/*---------------------------------------------------------------------------
 
  FILENAME:
        glmultichart.c
 
  PURPOSE:
        Provide the graphics lib multiple plot chart utilities.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        02/06/05        M.S. Teel       0               Original
 
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
#include "glmultichart.h"


/*  ... global memory declarations
*/

/*  ... global memory referenced
*/

/*  ... static (local) memory declarations
*/
#define MAX_STEP_MULTIPLIER         9

static MULTICHART nonReentrantMultiChart;

static double stepSizeMultipliers[MAX_STEP_MULTIPLIER] =
    {
        2.0, 5.0, 10.0, 20.0, 50.0, 100.0, 200.0, 500.0, 1000.0
    };



static int createImage (MULTICHART_ID id)
{
    int         i;

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

    for (i = 0; i < id->numdatasets; i ++)
    {
        id->dataset[i].lineColor =
            gdImageColorAllocateAlpha (id->im,
                                       GLC_RED(id->dataset[i].lineColor),
                                       GLC_GREEN(id->dataset[i].lineColor),
                                       GLC_BLUE(id->dataset[i].lineColor),
                                       GLC_ALPHA(id->dataset[i].lineColor));
    }

    return 0;
}


static void drawGrid (MULTICHART_ID id)
{
    register int    i;
    int             xlabelwidth = 0, ylabelwidth = 0;
    double          units, step;
    double          minR, stepR;
    int             dataPoints, xnumhash, xhashindexlen;
    double          hashwidth, xhashwidth;
    int             numhash, x, y;
    char            text[MAX_NUMHASH][WVIEW_STRING1_SIZE], ylabelfmt[32];
    char            textR[MAX_NUMHASH][WVIEW_STRING1_SIZE], ylabelfmtR[32];

    //  ... calculate the maximum number of hash marks and the units per hash
    //  ... for the x-axis
    //  build labels and calculate the maximum x-label width
    for (i = 0; i < id->numpoints; i ++)
    {
        if (strlen (id->pointnames[i]) > xlabelwidth)
            xlabelwidth = strlen (id->pointnames[i]);
    }

    xhashwidth = (xlabelwidth*gdFontSmall->h)/2;
    dataPoints = id->numpoints - 1;


    if (strlen (id->DualUnit) > 0)
    {
        // ((gdFontSmall->h/2) * strlen (id->DualUnit)))/2) - 2
        id->imbx -= (gdFontMediumBold->h);
        if ((strcmp(id->DualUnit,"inches") == 0) || 
            (strcmp(id->DualUnit,"knots") == 0) || 
            (strcmp(id->DualUnit,"m/s") == 0))
            id->imbx -= 7;
    }


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

    xhashwidth = (double)(id->imbx - id->imtx)/(double)xnumhash;
    id->pointpixels = (double)(id->imbx - id->imtx)/(double)dataPoints;
    xhashindexlen = dataPoints/xnumhash;


    //  ... calculate the maximum number of hash marks and the units per hash
    //  ... for the y-axis
    hashwidth = (gdFontSmall->h * 3)/2;
    numhash = (id->imby - id->imty)/hashwidth;
    if (numhash > 64)
        numhash = 64;

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
    if ((id->min + (id->ystepSize * numhash)) < id->max)
    {
        numhash ++;
    }

    hashwidth = (double)(id->imby - id->imty)/(double)numhash;
    id->max = id->min + (id->ystepSize * numhash);
    id->ypixelconstant = (double)(id->imby - id->imty)/(double)(id->max - id->min);
    numhash ++;
    if (numhash > MAX_NUMHASH)
    {
        numhash = MAX_NUMHASH;
    }

    //  build labels and calculate the maximum y-label width
    sprintf (ylabelfmt, "%%.%1.1df", id->ydecPlaces);
    for (i = 0; i < numhash; i ++)
    {
        sprintf (text[i], ylabelfmt, id->min + (i * id->ystepSize));

        // now fill the right hand array with alternate scale @Rcm
        if (strlen(id->DualUnit) > 0)
        {
            textR[i][0] = ' ';
            if (strcmp(id->DualUnit,"C") == 0)
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
            else if (strcmp(id->DualUnit,"mm") == 0)
            {
                minR = wvutilsConvertINToMM (id->min);
                stepR = wvutilsConvertINToMM (id->ystepSize);
                if (wvutilsConvertINToMM (id->max) >= 100)
                    sprintf (textR[i], "%.0f", minR + (i * stepR));
                else
                    sprintf (textR[i], "%.1f", minR + (i * stepR));
            }
            else if (strcmp(id->DualUnit,"inches") == 0)
            {
                minR = wvutilsConvertMMToIN (id->min);
                stepR = wvutilsConvertMMToIN (id->ystepSize);
                if (wvutilsConvertMMToIN (id->max) >= 100)
                    sprintf (textR[i], "%.0f", minR + (i * stepR));
                else
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
            else //all else: eg '%' or watts/m^2 (hum/ rad)
            {    // same value both sides
                minR = id->min * 25.4;
                stepR = id->ystepSize * 25.4;
                sprintf (textR[i], "%s", text[i]);
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

    //  ... now we know the chart area, draw it
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

    //  ... draw the y-axis
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

        if (strlen(id->DualUnit) > 0 )
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

    //  ... draw the x-axis
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


    //  ... finally, draw date
    gdImageString (id->im,
                   gdFontSmall,
                   ((id->width - ((gdFontSmall->h/2) * strlen (id->datetime)))/2) - 2,
                   id->height - (gdFontMediumBold->h + 2),
                   (uint8_t *)id->datetime,
                   id->textcolor);
}

static void drawTitle (MULTICHART_ID id)
{
    int         i, totalLen = 0;
    char        xtitle[WVIEW_STRING1_SIZE];

    gdImageFilledRectangle (id->im,
                            0, 0,
                            id->width, gdFontMediumBold->h,
                            id->titleBGcolor);

    if (strlen(id->DualUnit) != 0)
    {
        for (i = 0; i < id->numdatasets; i ++)
        {
            totalLen += 2;
            totalLen += (((gdFontMediumBold->h/2)+2) * strlen (id->dataset[i].legend));
        }
        totalLen = (id->width - totalLen) - 3;
        totalLen /= 2;

        for (i = 0; i < id->numdatasets; i ++)
        {
            totalLen += 2;
            gdImageString (id->im,
                           gdFontMediumBold,
                           totalLen,
                           0,
                           (uint8_t *)id->dataset[i].legend,
                           id->dataset[i].lineColor);

            totalLen += (((gdFontMediumBold->h/2)+2) * strlen (id->dataset[i].legend));
        }

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
        for (i = 0; i < id->numdatasets; i ++)
        {
            totalLen += 2;
            gdImageString (id->im,
                           gdFontMediumBold,
                           totalLen,
                           0,
                           (uint8_t *)id->dataset[i].legend,
                           id->dataset[i].lineColor);

            totalLen += (((gdFontMediumBold->h/2)+2) * strlen (id->dataset[i].legend));
        }

        gdImageString (id->im,
                       gdFontMediumBold,
                       id->width - (((gdFontMediumBold->h/2)+2) * strlen (id->title)),
                       0,
                       (uint8_t *)id->title,
                       id->titleFGcolor);

    }
}

static void drawLines (MULTICHART_ID id)
{
    register int    i, j;
    int             x1, x2, y1, y2;

    gdImageSetThickness (id->im, 2);

    for (i = 0; i < id->numpoints - 1; i ++)
    {
        for (j = 0; j < id->numdatasets; j ++)
        {
            if (id->dataset[j].valueset[i] <= ARCHIVE_VALUE_NULL ||
                id->dataset[j].valueset[i+1] <= ARCHIVE_VALUE_NULL)
            {
                continue;
            }

            x1 = id->imtx + (int)((double)i * id->pointpixels);
            x2 = id->imtx + (int)((double)(i+1) * id->pointpixels);
            y1 = id->imby - (int)(id->ypixelconstant * (id->dataset[j].valueset[i] - id->min));
            y2 = id->imby - (int)(id->ypixelconstant * (id->dataset[j].valueset[i+1] - id->min));
            gdImageLine (id->im,
                         x1,
                         y1,
                         x2,
                         y2,
                         id->dataset[j].lineColor);
        }
    }
}


static void normalizeMinMax (MULTICHART_ID id, double newMIN, double newMAX)
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
}



/*  ... API methods
*/

MULTICHART_ID multiChartCreate
(
    int                     width,
    int                     height,
    char                    *units,
    int                     numDataSets,
    char                    *legends[]
)
{
    int                     i;
    register MULTICHART_ID  newId;

    newId = &nonReentrantMultiChart;

    memset (newId, 0, sizeof (*newId));

    if (numDataSets > MC_MAX_DATA_SETS)
        numDataSets = MC_MAX_DATA_SETS;
    newId->numdatasets = numDataSets;

    for (i = 0; i < numDataSets; i ++)
    {
        wvstrncpy (newId->dataset[i].legend, legends[i], sizeof(newId->dataset[i].legend));
    }

    newId->width = width;
    newId->height = height;
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
    newId->titleFGcolor = GLC_DFLT_TITLEFG;
    newId->titleBGcolor = GLC_DFLT_TITLEBG;
    newId->textcolor    = GLC_DFLT_TEXT;

    return newId;
}

void multiChartSetXScale (MULTICHART_ID id, int decPlaces)
{
    id->xdecPlaces = decPlaces;

    return;
}

void multiChartSetXHashes (MULTICHART_ID id, int numHashes)
{
    id->xnumhashes = numHashes;

    return;
}

void multiChartSetYScale (MULTICHART_ID id, double min, double max, double step, int decPlaces)
{
    id->min = min;
    id->max = max;
    id->ystepSize = step;
    id->ydecPlaces = decPlaces;

    normalizeMinMax (id, min, max);
    return;
}

void multiChartAddPoint (MULTICHART_ID id, double *values, char *name)
{
    int             i;

    if (id->numpoints >= MAX_GRAPH_POINTS)
    {
        return;
    }

    for (i = 0; i < id->numdatasets; i ++)
    {
        id->dataset[i].valueset[id->numpoints] = values[i];

        if (values[i] > ARCHIVE_VALUE_NULL)
        {
            if (values[i] < id->min)
                normalizeMinMax (id, values[i], id->max);
            else if (values[i] > id->max)
                normalizeMinMax (id, id->min, values[i]);
        }
    }

    wvstrncpy (id->pointnames[id->numpoints], name, sizeof(id->pointnames[id->numpoints]));
    id->numpoints ++;

    return;
}

void multiChartSetDateTime (MULTICHART_ID id, char *datetime)
{
    wvstrncpy (id->datetime, datetime, sizeof(id->datetime));
    return;
}

void multiChartSetBGColor (MULTICHART_ID id, int color)
{
    id->bgcolor = color;
    return;
}

void multiChartSetChartColor (MULTICHART_ID id, int color)
{
    id->chartcolor = color;
    return;
}

void multiChartSetGridColor (MULTICHART_ID id, int color)
{
    id->gridcolor = color;
    return;
}

void multiChartSetLineColor (MULTICHART_ID id, int dataset, int color)
{
    if (dataset >= MC_MAX_DATA_SETS)
        return;

    id->dataset[dataset].lineColor = color;
    return;
}

void multiChartSetTitleColors (MULTICHART_ID id, int fg, int bg)
{
    id->titleFGcolor = fg;
    id->titleBGcolor = bg;
    return;
}

void multiChartSetTextColor (MULTICHART_ID id, int color)
{
    id->textcolor = color;
    return;
}

void multiChartSetTransparency (MULTICHART_ID id, int isTransparent)
{
    id->isTransparent = isTransparent;
    return;
}

void multiChartSetDualUnits (MULTICHART_ID id)
{
    htmlutilsSetDualUnits (id->isMetric, id->units, id->DualUnit);
    return;
}

int multiChartRender (MULTICHART_ID id)
{
    if (createImage (id) == -1)
    {
        return -1;
    }

    drawTitle (id);
    drawGrid (id);
    drawLines (id);

    return 0;
}

int multiChartSave (MULTICHART_ID id, char *name)
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

void multiChartDestroy (MULTICHART_ID id)
{
    if (id->im)
        gdImageDestroy (id->im);

    return;
}

