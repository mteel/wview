#ifndef INC_beauforth
#define INC_beauforth
/*---------------------------------------------------------------------------
 
  FILENAME:
        beaufort.h
 
  PURPOSE:
        Provide the beaufort wind scale definitions.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        04/16/07        M.S. Teel       0               Original
 
  NOTES:
        
 
  LICENSE:
        Copyright (c) 2007, Mark S. Teel (mark@teel.ws)
  
        This source code is released for free distribution under the terms 
        of the GNU General Public License.
  
----------------------------------------------------------------------------*/

//  Define the Beaufort scale thresholds (use <= comparison)
enum BeaufortScaleValues
{
    Beaufort_Calm               = 0,
    Beaufort_LightAir           = 3,
    Beaufort_LightBreeze        = 7,
    Beaufort_GentleBreeze       = 12,
    Beaufort_ModerateBreeze     = 18,
    Beaufort_FreshBreeze        = 24,
    Beaufort_StrongBreeze       = 31,
    Beaufort_NearGale           = 38,
    Beaufort_Gale               = 46,
    Beaufort_SevereGale         = 54,
    Beaufort_Storm              = 63,
    Beaufort_ViolentStorm       = 72,
    Beaufort_Hurricane          = 83
};

#endif

