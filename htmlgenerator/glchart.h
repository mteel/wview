#ifndef INC_glcharth
#define INC_glcharth
/*---------------------------------------------------------------------------
 
  FILENAME:
        glchart.h
 
  PURPOSE:
        Provide the graphics lib chart definitions.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        09/05/03        M.S. Teel       0               Original
        05/15/08        Werner Krenn    1               dualUnits
        05/05/09        D. Pickett      2               Add scatter charts
 
  NOTES:
        
 
  LICENSE:
        Copyright (c) 2004, Mark S. Teel (mark@teel.ws)
  
        This source code is released for free distribution under the terms 
        of the GNU General Public License.
  
----------------------------------------------------------------------------*/

#include "gd.h"
#include <datadefs.h>



#define GLC_RED(x)                  ((x >> 24) & 0xFF)
#define GLC_GREEN(x)                ((x >> 16) & 0xFF)
#define GLC_BLUE(x)                 ((x >> 8 ) & 0xFF)
#define GLC_ALPHA(x)                ((x) & 0xFF)

#define GLC_DFLT_BG                 0xF0F0F000
#define GLC_DFLT_CHART              0x0000F000
#define GLC_DFLT_GRID               0xA0A00000
#define GLC_DFLT_LINE               0x80808000
#define GLC_DFLT_TITLEFG            0x00000000
#define GLC_DFLT_TITLEBG            0xE0E0E000
#define GLC_DFLT_TEXT               0x00000000

#define GLC_CONTENTS_OFFSET         4

#define MAX_GRAPH_POINTS            1024
#define MAX_NUMHASH                 64


typedef enum
{
   CHART_TYPE_LINE,
   CHART_TYPE_BAR,
   CHART_TYPE_SCATTER
} CHART_TYPE;

typedef struct
{
    gdImagePtr  im;
    CHART_TYPE  chartType;
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
    double      pointpixels;
    char        pointnames[MAX_GRAPH_POINTS][16];
    double      valueset[MAX_GRAPH_POINTS];
    int         bgcolor;
    int         chartcolor;
    int         gridcolor;
    int         linecolor;
    int         titleFGcolor;
    int         titleBGcolor;
    int         textcolor;
    int         isTransparent;
    int         isMetric;
    char        DualUnit[16];
} CHART, *CHART_ID;


//  ... API prototypes
//  ... NOTE: for the sake of optimization, these routines have been
//  ...       rendered "one chart at a time". This is how they were being
//  ...       used anyway, but each chartCreate call uses the same static 
//  ...       buffer, thus overwriting the previous contents

extern CHART_ID chartCreate 
(
    int         width, 
    int         height, 
    char        *title, 
    char        *units, 
    CHART_TYPE  chartType
);
extern void chartSetXScale (CHART_ID id, int decPlaces);
extern void chartSetXHashes (CHART_ID id, int numHashes);
extern void chartSetYScale (CHART_ID id, double min, double max, double step, int decPlaces);
extern void chartAddPoint (CHART_ID id, double value, char *name);
extern void chartSetDateTime (CHART_ID id, char *datetime);
extern void chartSetBGColor (CHART_ID id, int color);
extern void chartSetChartColor (CHART_ID id, int color);
extern void chartSetGridColor (CHART_ID id, int color);
extern void chartSetLineColor (CHART_ID id, int color);
extern void chartSetTitleColors (CHART_ID id, int fg, int bg);
extern void chartSetTextColor (CHART_ID id, int color);
extern void chartSetTransparency (CHART_ID id, int isTransparent);
extern void chartSetDualUnits (CHART_ID id);
extern int chartRender (CHART_ID id);
extern int chartSave (CHART_ID id, char *filename);
extern void chartDestroy (CHART_ID id);


#endif
