/*---------------------------------------------------------------------------
 
  FILENAME:
        glbucket.c
 
  PURPOSE:
        Provide the graphics lib utilities.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        09/05/03        M.S. Teel       0               Original
        02/03/07        Randy Miller    1               Add alternate units on right
                                                        side of bucket and at bottom
                                                        with label
        05/15/08        Werner Krenn    2               Add some metric units for dualUnits
 
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
#include <glbucket.h>


/*  ... global memory declarations
*/

/*  ... global memory referenced
*/

/*  ... static (local) memory declarations
*/
#define MAX_STEP_MULTIPLIER         9

static BUCKET nonReentrantBucket;

static double stepSizeMultipliers[MAX_STEP_MULTIPLIER] =
    {
        2.0, 5.0, 10.0, 20.0, 50.0, 100.0, 200.0, 500.0, 1000.0
    };


static int createImage (BUCKET_ID id)
{
    if (id->height < ( 4 * gdFontMediumBold->h))
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
                  GLB_RED(id->bgcolor),
                  GLB_GREEN(id->bgcolor),
                  GLB_BLUE(id->bgcolor),
                  GLB_ALPHA(id->bgcolor));

    if (id->isTransparent)
    {
        gdImageColorTransparent (id->im, id->bgcolor);
        id->titleBGcolor = id->bgcolor;
    }
    else
    {
        id->titleBGcolor = gdImageColorAllocateAlpha (id->im,
                           GLB_RED(id->titleBGcolor),
                           GLB_GREEN(id->titleBGcolor),
                           GLB_BLUE(id->titleBGcolor),
                           GLB_ALPHA(id->titleBGcolor));
    }

    id->bucketcolor = gdImageColorAllocateAlpha (id->im,
                      GLB_RED(id->bucketcolor),
                      GLB_GREEN(id->bucketcolor),
                      GLB_BLUE(id->bucketcolor),
                      GLB_ALPHA(id->bucketcolor));
    id->contentcolor = gdImageColorAllocateAlpha (id->im,
                       GLB_RED(id->contentcolor),
                       GLB_GREEN(id->contentcolor),
                       GLB_BLUE(id->contentcolor),
                       GLB_ALPHA(id->contentcolor));
    id->highcolor = gdImageColorAllocateAlpha (id->im,
                    GLB_RED(id->highcolor),
                    GLB_GREEN(id->highcolor),
                    GLB_BLUE(id->highcolor),
                    GLB_ALPHA(id->highcolor));
    id->lowcolor = gdImageColorAllocateAlpha (id->im,
                   GLB_RED(id->lowcolor),
                   GLB_GREEN(id->lowcolor),
                   GLB_BLUE(id->lowcolor),
                   GLB_ALPHA(id->lowcolor));
    id->titleFGcolor = gdImageColorAllocateAlpha (id->im,
                       GLB_RED(id->titleFGcolor),
                       GLB_GREEN(id->titleFGcolor),
                       GLB_BLUE(id->titleFGcolor),
                       GLB_ALPHA(id->titleFGcolor));
    id->textcolor = gdImageColorAllocateAlpha (id->im,
                    GLB_RED(id->textcolor),
                    GLB_GREEN(id->textcolor),
                    GLB_BLUE(id->textcolor),
                    GLB_ALPHA(id->textcolor));

    return 0;
}


static void drawBucket (BUCKET_ID id)
{
    int     i, ylabelwidth = 0;
    double  units, step, hashwidth;
    double minR, stepR;
    int     numhash, x, y;
    char    text[64][32];
    char    textR[64][32];
    char ylabelfmt[24];
    char ylabelfmtR[24];

    /* Calculate the maximum number of hash marks and the units per hash */
    hashwidth = (5 * gdFontSmall->h)/4;
    numhash = (id->imby - id->imty)/hashwidth;
    if (numhash > 64)
        numhash = 64;

    units = (id->max - id->min)/id->stepSize;
    step = id->stepSize;
    i = 0;
    while ((int)units > numhash && i < MAX_STEP_MULTIPLIER)
    {
        step = id->stepSize * stepSizeMultipliers[i];
        units = (id->max - id->min)/step;
        i ++;
    }

    id->stepSize = step;
    numhash = (int)units;
    if ((id->min + (id->stepSize * numhash)) < id->max)
    {
        numhash ++;
    }

    if (numhash == 0)
    {
        hashwidth = 0.0;
        id->max = id->min;
    }
    else
    {
        hashwidth = (double)(id->imby - id->imty)/(double)numhash;
        id->max = id->min + (id->stepSize * numhash);
    }

    if (id->max - id->min == 0)
        id->pixelconstant = 0;
    else
        id->pixelconstant = ((double)(id->imby - id->imty))/(id->max - id->min);

    if (numhash)
        numhash ++;
    if (numhash > 64)
        numhash = 64;

    // bucket scales
    //  build labels and calculate the maximum label width
    sprintf (ylabelfmt, "%%.%1.1df", id->decPlaces-1);
    for (i = 0; i < numhash; i ++)
    {
        sprintf (text[i], ylabelfmt, id->min + (i * id->stepSize));
        // now fill the right hand array with alternate scale @Rcm
        if ( id->isDualUnits )
        {
            textR[i][0] = ' ';
            if (strcmp(id->units,"hPa") == 0)
            {
                minR = wvutilsConvertHPAToINHG (id->min);
                stepR = wvutilsConvertHPAToINHG (id->stepSize);
                sprintf (textR[i], "%.2f", minR + (i * stepR));
            }
            else if (strcmp(id->units,"inHg") == 0)
            {
                minR = wvutilsConvertINHGToHPA (id->min);
                stepR = wvutilsConvertINHGToHPA (id->stepSize);
                sprintf (textR[i], "%.0f", minR + (i * stepR));
            }
            else if (strcmp(id->units,"F") == 0)
            {
                minR = wvutilsConvertFToC (id->min);
                stepR = ((id->stepSize) * (5.0/9.0));
                sprintf (textR[i], "%.1f", minR + (i * stepR));
            }
            else if (strcmp(id->units,"C") == 0)
            {
                minR = wvutilsConvertCToF (id->min);
                stepR = ((id->stepSize) * (9.0/5.0));
                sprintf (textR[i], "%.1f", minR + (i * stepR));
            }
            else if ((strcmp(id->units,"inches") == 0) || 
                     (strcmp(id->units,"in/hour") == 0) || 
                     (strcmp(id->units,"in/hr") == 0))
            {
                minR = wvutilsConvertINToMM (id->min);
                stepR = wvutilsConvertINToMM (id->stepSize);
                if (wvutilsConvertINToMM (id->max) >= 100)
                    sprintf (textR[i], "%.0f", minR + (i * stepR));
                else
                    sprintf (textR[i], "%.1f", minR + (i * stepR));
            }
            else if ((strcmp(id->units,"mm") == 0) ||
                     (strcmp(id->units,"mm/hr") == 0) || 
                     (strcmp(id->units,"mm/hour") == 0) || 
                     (strcmp(id->units,"mm/h") == 0))
            {
                minR = wvutilsConvertMMToIN (id->min);
                stepR = wvutilsConvertMMToIN (id->stepSize);
                if (wvutilsConvertMMToIN (id->max) >= 100)
                    sprintf (textR[i], "%.0f", minR + (i * stepR));
                else
                    sprintf (textR[i], "%.1f", minR + (i * stepR));
            }
            else if (strcmp(id->units,"cm") == 0)
            {
                minR = wvutilsConvertCMToIN (id->min);
                stepR = wvutilsConvertCMToIN (id->stepSize);
                sprintf (textR[i], "%.1f", minR + (i * stepR));
                if (wvutilsConvertCMToIN (id->max) >= 100)
                    sprintf (textR[i], "%.0f", minR + (i * stepR));
                else
                    sprintf (textR[i], "%.1f", minR + (i * stepR));
            }

            else
            {
                // all else: eg '%' or watts/m^2 (hum/ rad)
                // same value both sides
                minR = id->min * 25.4;
                stepR = id->stepSize * 25.4;
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

    //  ... now we know the bucket area, draw it
    gdImageFilledRectangle (id->im,
                            id->imtx - GLB_CONTENTS_OFFSET,
                            id->imty,
                            id->imbx + GLB_CONTENTS_OFFSET,
                            id->imby + GLB_CONTENTS_OFFSET,
                            id->bucketcolor);

    //  ... open up the contents area
    gdImageFilledRectangle (id->im,
                            id->imtx, id->imty - 1,
                            id->imbx, id->imby - 1,
                            id->bgcolor);

    //  ... draw the lips
    gdImageSetThickness (id->im, GLB_CONTENTS_OFFSET);
    gdImageLine (id->im, id->imtx-1, id->imty,
                 id->imtx-8, id->imty, id->bucketcolor);
    gdImageLine (id->im, id->imbx+1, id->imty,
                 id->imbx+8, id->imty, id->bucketcolor);

    //  ... draw the ticks and labels
    gdImageSetThickness (id->im, 2);
    for (i = 0; i < numhash; i ++)
    {
        // left side
        x = id->imtx - (GLB_CONTENTS_OFFSET - 1);
        y = (id->imby - (i * hashwidth)) + 1;

        gdImageLine (id->im,
                     x,
                     y,
                     x - 3,
                     y,
                     id->bucketcolor);

        y -= gdFontSmall->h/2;
        gdImageString (id->im,
                       gdFontSmall,
                       (x - 5) - (strlen(text[i]) * gdFontSmall->h/2),
                       y,
                       (uint8_t *)text[i],
                       id->textcolor);

        // right side
        x = id->imbx + (GLB_CONTENTS_OFFSET - 1);
        y = (id->imby - (i * hashwidth)) + 1;

        gdImageLine (id->im,
                     x,
                     y,
                     x + 3,
                     y,
                     id->bucketcolor);
        if ( id->isDualUnits )
        {
            y -= gdFontSmall->h/2;
            gdImageString (id->im,
                           gdFontSmall,
                           x + 4 + gdFontSmall->h/2,
                           y,
                           (uint8_t *)textR[i],
                           id->textcolor);
        }
        else
        {
            y -= gdFontSmall->h/2;
            gdImageString (id->im,
                           gdFontSmall,
                           x + 4 + gdFontSmall->h/2,
                           y,
                           (uint8_t *)text[i],
                           id->textcolor);
        }
    }

    //  ... finally, draw units and date
    sprintf (ylabelfmt, "%%.%1.1df %%s", id->decPlaces); // generate the format string with a variable number of decimal places
    sprintf (text[0], ylabelfmt, id-> value, id->units); // build the string with readig and units

    gdImageString (id->im,
                   gdFontMediumBold,
                   ((id->width - ((gdFontMediumBold->h/2) * (1+strlen (text[0]))))/2),
                   (id->height - (gdFontMediumBold->h*3)) + 4,
                   (uint8_t *)text[0],
                   id->textcolor);

    // -- Bottom of bucket display --
    // now next line down, draw the alt units @rcm
    if ( id->isDualUnits)
    {
        if (strcmp(id->units,"F") == 0)
        {
            id->valueM = wvutilsConvertFToC (id->value);
            sprintf (text[0], "%.1f C", id->valueM);
        }
        else if (strcmp(id->units,"C") == 0)
        {
            id->valueM = wvutilsConvertCToF (id->value);
            sprintf (text[0], "%.0f F", id->valueM);
        }
        else if (strcmp(id->units,"inches") == 0)
        {
            id->valueM = wvutilsConvertINToMM (id->value);
            sprintf (text[0], "%.1f mm", id->valueM);
        }
        else if ((strcmp(id->units,"in/hour") == 0) || (strcmp(id->units,"in/hr") == 0))
        {
            id->valueM = wvutilsConvertINToMM (id->value);
            sprintf (text[0], "%.1f mm/h", id->valueM);
        }
        else if ((strcmp(id->units,"mm/hr") == 0) || 
                 (strcmp(id->units,"mm/hour") == 0) || 
                 (strcmp(id->units,"mm/h") == 0))
        {
            id->valueM = wvutilsConvertMMToIN (id->value);
            sprintf (text[0], "%.2f in/hr", id->valueM);
        }
        else if (strcmp(id->units,"hPa") == 0)
        {
            id->valueM  = wvutilsConvertHPAToINHG (id->value);
            sprintf (text[0], "%2.3f inHg", id->valueM);
        }
        else if (strcmp(id->units,"inHg") == 0)
        {
            id->valueM  = wvutilsConvertINHGToHPA (id->value);
            sprintf (text[0], "%4.0f hPa", id->valueM);
        }
        else if (strcmp(id->units,"mm") == 0)
        {
            id->valueM = wvutilsConvertMMToIN (id->value);
            sprintf (text[0], "%.2f inches", id->valueM);
        }
        else if (strcmp(id->units,"cm") == 0)
        {
            id->valueM = wvutilsConvertCMToIN (id->value);
            sprintf (text[0], "%.2f inches", id->valueM);
        }
        else
        {
            // no conversion no second display
            id->valueM = id->value;
            text[0][0] = 0;
        }
        gdImageString (id->im,
                       gdFontMediumBold,
                       ((id->width - ((gdFontMediumBold->h/2) * (1+strlen (text[0]))))/2),
                       (id->height - (gdFontMediumBold->h*3)) + 15,
                       (uint8_t *)text[0],
                       id->textcolor);
    }
    gdImageString (id->im,
                   gdFontSmall,
                   ((id->width - ((gdFontSmall->h/2) * strlen (id->datetime)))/2) - 2,
                   id->height - (gdFontMediumBold->h - 0),
                   (uint8_t *)id->datetime,
                   id->textcolor);
}

static void drawTitle (BUCKET_ID id)
{
    gdImageFilledRectangle (id->im,
                            0, 0,
                            id->width, gdFontMediumBold->h,
                            id->titleBGcolor);

    gdImageString (id->im,
                   gdFontMediumBold,
                   ((id->width - ((gdFontMediumBold->h/2) * strlen (id->title)))/2) - 3,
                   0,
                   (uint8_t *)id->title,
                   id->titleFGcolor);
}

static void drawContents (BUCKET_ID id)
{
    int     y;

    y = (int)(id->pixelconstant * (id->value - id->min));

    //  ... draw the contents area
    gdImageFilledRectangle (id->im,
                            id->imtx,
                            id->imby - y,
                            id->imbx,
                            id->imby,
                            id->contentcolor);

    //  ... draw high and low
    if (id->high != GLB_HILOW_NONE)
    {
        y = id->pixelconstant * (id->high - id->min);
        gdImageLine (id->im,
                     id->imtx,
                     id->imby - y,
                     id->imbx,
                     id->imby - y,
                     id->highcolor);
    }

    if (id->low != GLB_HILOW_NONE)
    {
        y = id->pixelconstant * (id->low - id->min);
        gdImageLine (id->im,
                     id->imtx,
                     id->imby - y,
                     id->imbx,
                     id->imby - y,
                     id->lowcolor);
    }
}


/*  ... methods
*/

BUCKET_ID bucketCreate (int width, int height, int bucketWidth, char *title)
{
    BUCKET_ID       newId;

    if ((5 * gdFontSmall->h) + (2 * GLB_CONTENTS_OFFSET) + bucketWidth > width)
    {
        return NULL;
    }

    newId = (BUCKET_ID)&nonReentrantBucket;

    memset (newId, 0, sizeof (*newId));

    newId->width = width;
    newId->height = height;
    newId->bucketWidth = bucketWidth;
    wvstrncpy (newId->title, title, sizeof(newId->title));

    //  ... now set the contents coords
    newId->imtx = (newId->width - newId->bucketWidth)/2;
    newId->imbx = (newId->width - 1) - newId->imtx;
    newId->imty = 2 * gdFontMediumBold->h;
    newId->imby = newId->height - (3 * gdFontMediumBold->h);

    newId->decPlaces    = 0;
    newId->bgcolor      = GLB_DFLT_BG;
    newId->bucketcolor  = GLB_DFLT_BUCKET;
    newId->contentcolor = GLB_DFLT_CONTENT;
    newId->highcolor    = GLB_DFLT_HIGH;
    newId->lowcolor     = GLB_DFLT_LOW;
    newId->titleFGcolor = GLB_DFLT_TITLEFG;
    newId->titleBGcolor = GLB_DFLT_TITLEBG;
    newId->textcolor    = GLB_DFLT_TEXT;

    return newId;
}

void bucketSetScale (BUCKET_ID id, double min, double max, double step)
{
    id->min = min;
    id->max = max;
    id->stepSize = step;

    return;
}

void bucketSetValues (BUCKET_ID id, double high, double low, double value)
{
    id->high = high;
    id->low = low;
    id->value = value;

    if ((id->high != GLB_HILOW_NONE) && (id->high > id->max))
        id->max = id->high;

    if ((id->low != GLB_HILOW_NONE) && (id->low < id->min))
        id->min = id->low;

    if (value < id->min)
        id->min = value - (0.1 * value);

    if (value > id->max)
        id->max = value + (0.1 * value);

    return;
}

void  bucketSetUnits (BUCKET_ID id, char *units)
{
    wvstrncpy (id->units, units, sizeof(id->units));
    return;
}

void bucketSetDecimalPlaces (BUCKET_ID id, int decPlaces)
{
    id->decPlaces = decPlaces;
    return;
}

void bucketSetDateTime (BUCKET_ID id, char *datetime)
{
    wvstrncpy (id->datetime, datetime, sizeof(id->datetime));
    return;
}

void bucketSetBGColor (BUCKET_ID id, int color)
{
    id->bgcolor = color;
    return;
}

void bucketSetBucketColor (BUCKET_ID id, int color)
{
    id->bucketcolor = color;
    return;
}

void bucketSetContentColor (BUCKET_ID id, int color)
{
    id->contentcolor = color;
    return;
}

void bucketSetHighLowColors (BUCKET_ID id, int hcolor, int lcolor)
{
    id->highcolor = hcolor;
    id->lowcolor = lcolor;
    return;
}

void bucketSetTitleColors (BUCKET_ID id, int fg, int bg)
{
    id->titleFGcolor = fg;
    id->titleBGcolor = bg;
    return;
}

void bucketSetTextColor (BUCKET_ID id, int color)
{
    id->textcolor = color;
    return;
}

void bucketSetTransparency (BUCKET_ID id, int isTransparent)
{
    id->isTransparent = isTransparent;
    return;
}

void bucketSetDualUnits (BUCKET_ID id, int isDualUnits)
{
    id->isDualUnits = isDualUnits;
    return;
}

int bucketRender (BUCKET_ID id)
{
    if (createImage (id) == -1)
    {
        return -1;
    }

    drawTitle (id);
    drawBucket (id);
    drawContents (id);

    return 0;
}

int bucketSave (BUCKET_ID id, char *name)
{
    FILE    *out;

    if ((out = fopen (name, "wb")) != NULL)
    {
        gdImagePng (id->im, out);
        fflush (out);
        fclose (out);
        return 0;
    }
    else
    {
        return -1;
    }
}

void bucketDestroy (BUCKET_ID id)
{
    if (id->im)
        gdImageDestroy (id->im);

    return;
}

