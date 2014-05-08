#ifndef INC_glmulticharth
#define INC_glmulticharth
/*---------------------------------------------------------------------------
 
  FILENAME:
        glmultichart.h
 
  PURPOSE:
        Provide the graphics lib miltiple plot chart definitions.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        02/06/05        M.S. Teel       0               Original
 
  NOTES:
        
 
  LICENSE:
        Copyright (c) 2004, Mark S. Teel (mark@teel.ws)
  
        This source code is released for free distribution under the terms 
        of the GNU General Public License.
  
----------------------------------------------------------------------------*/

#include "glchart.h"

#define MC_MAX_DATA_SETS            4


typedef struct
{
    char        legend[16];
    int         lineColor;
    double      valueset[MAX_GRAPH_POINTS];
} MC_DATASET;

typedef struct
{
    gdImagePtr  im;
    int         isBarChart;
    int         width;
    int         height;
    char        title[64];
    char        units[16];
    char        datetime[64];
    int         xdecPlaces;
    int         xnumhashes;
    int         ydecPlaces;
    double      ystepSize;
    double      ypixelconstant;
    double      min;
    double      max;
    int         imtx;
    int         imty;
    int         imbx;
    int         imby;
    double      xmax;
    int         numpoints;
    int         numdatasets;
    MC_DATASET  dataset[MC_MAX_DATA_SETS];
    char        pointnames[MAX_GRAPH_POINTS][16];
    double      pointpixels;
    int         bgcolor;
    int         chartcolor;
    int         gridcolor;
    int         titleFGcolor;
    int         titleBGcolor;
    int         textcolor;
    int         isTransparent;
    int         isMetric;
    char        DualUnit[16];
} MULTICHART, *MULTICHART_ID;


//  ... API prototypes
//  ... NOTE: for the sake of optimization, these routines have been
//  ...       rendered "one chart at a time". This is how they were being
//  ...       used anyway, but each multiChartCreate call uses the same static 
//  ...       buffer, thus overwriting the previous contents

extern MULTICHART_ID multiChartCreate
(
    int                     width, 
    int                     height, 
    char                    *units,
    int                     numDataSets, 
    char                    *legends[]
);
extern void multiChartSetXScale (MULTICHART_ID id, int decPlaces);
extern void multiChartSetXHashes (MULTICHART_ID id, int numHashes);
extern void multiChartSetYScale (MULTICHART_ID id, double min, double max, double step, int decPlaces);
extern void multiChartAddPoint (MULTICHART_ID id, double *values, char *name);
extern void multiChartSetDateTime (MULTICHART_ID id, char *datetime);
extern void multiChartSetBGColor (MULTICHART_ID id, int color);
extern void multiChartSetChartColor (MULTICHART_ID id, int color);
extern void multiChartSetGridColor (MULTICHART_ID id, int color);
extern void multiChartSetLineColor (MULTICHART_ID id, int dataset, int color);
extern void multiChartSetTitleColors (MULTICHART_ID id, int fg, int bg);
extern void multiChartSetTextColor (MULTICHART_ID id, int color);
extern void multiChartSetTransparency (MULTICHART_ID id, int isTransparent);
extern int multiChartRender (MULTICHART_ID id);
extern int multiChartSave (MULTICHART_ID id, char *filename);
extern void multiChartDestroy (MULTICHART_ID id);


#endif
