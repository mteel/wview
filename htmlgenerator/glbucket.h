#ifndef INC_glbucketh
#define INC_glbucketh
/*---------------------------------------------------------------------------
 
  FILENAME:
        glbucket.h
 
  PURPOSE:
        Provide the graphics lib bucket definitions.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        09/05/03        M.S. Teel       0               Original
 
  NOTES:
        
 
  LICENSE:
        Copyright (c) 2004, Mark S. Teel (mark@teel.ws)
  
        This source code is released for free distribution under the terms 
        of the GNU General Public License.
  
----------------------------------------------------------------------------*/

#include "gd.h"


#define GLB_RED(x)                  ((x >> 24) & 0xFF)
#define GLB_GREEN(x)                ((x >> 16) & 0xFF)
#define GLB_BLUE(x)                 ((x >> 8) & 0xFF)
#define GLB_ALPHA(x)                ((x) & 0xFF)

#define GLB_CONTENTS_OFFSET         4

#define GLB_HILOW_NONE              (double)(-1000.0)

#define GLB_DFLT_BG                 0xF0F0F0
#define GLB_DFLT_BUCKET             0x000000
#define GLB_DFLT_CONTENT            0x808080
#define GLB_DFLT_HIGH               0xFF0000
#define GLB_DFLT_LOW                0x0000FF
#define GLB_DFLT_TITLEFG            0x000000
#define GLB_DFLT_TITLEBG            0xE0E0E0
#define GLB_DFLT_TEXT               0x000000


typedef struct
{
    gdImagePtr  im;
    int         width;
    int         height;
    int         bucketWidth;
    int         imtx;
    int         imty;
    int         imbx;
    int         imby;
    double      pixelconstant;
    double      min;
    double      max;
    double      high;
    double      low;
    double      value;
    double      valueM;
    double      stepSize;
    int         decPlaces;
    char        title[64];
    char        units[64];
    char        datetime[64];
    int         bgcolor;
    int         bucketcolor;
    int         contentcolor;
    int         highcolor;
    int         lowcolor;
    int         titleFGcolor;
    int         titleBGcolor;
    int         textcolor;
    int         isTransparent;
    int         isDualUnits;
    int         isMetric;
} BUCKET, *BUCKET_ID;


//  ... API prototypes

extern BUCKET_ID bucketCreate (int width, int height, int bucketWidth, char *title);
extern void bucketSetScale (BUCKET_ID id, double min, double max, double step);
extern void bucketSetValues (BUCKET_ID id, double high, double low, double value);
extern void bucketSetUnits (BUCKET_ID id, char *units);
extern void bucketSetDecimalPlaces (BUCKET_ID id, int decimalPlaces);
extern void bucketSetDateTime (BUCKET_ID id, char *datetime);
extern void bucketSetBGColor (BUCKET_ID id, int color);
extern void bucketSetBucketColor (BUCKET_ID id, int color);
extern void bucketSetContentColor (BUCKET_ID id, int color);
extern void bucketSetHighLowColors (BUCKET_ID id, int hcolor, int lcolor);
extern void bucketSetTitleColors (BUCKET_ID id, int fg, int bg);
extern void bucketSetTextColor (BUCKET_ID id, int color);
extern void bucketSetTransparency (BUCKET_ID id, int isTransparent);
extern int bucketRender (BUCKET_ID id);
extern int bucketSave (BUCKET_ID id, char *filename);
extern void bucketDestroy (BUCKET_ID id);

#endif


