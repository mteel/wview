/*---------------------------------------------------------------------------

  FILENAME:
        htmlUtils.c

  PURPOSE:
        Provide the wview html generator utilities.

  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        08/30/03        M.S. Teel       0               Original

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

/*  ... Local include files
*/
#include <services.h>
#include <html.h>


/*  ... global memory declarations
*/

/*  ... global memory referenced
*/

/*  ... static (local) memory declarations
*/

void htmlutilsSetDualUnits (int isMetric, const char *units, char *dualUnits)
{
    if ((strcmp(units,"degrees") == 0) || (strcmp(units,"Grad") == 0))
    {
        if (isMetric)
        {
            strcpy (dualUnits, "F");
        }
        else
        {
            strcpy (dualUnits, "C");
        }
    }
    else if ((strcmp(units,"F") == 0) || 
             (strcmp(units,"degF") == 0) || 
             (strcmp(units,"deg F") == 0))
    {    // must be in degF
        strcpy (dualUnits, "C");
    }
    else if ((strcmp(units,"C") == 0) || 
             (strcmp(units,"degC") == 0) || 
             (strcmp(units,"deg C") == 0))
    {    // must be in degC
        strcpy (dualUnits, "F");
    }
    else if (strcmp(units,"inches") == 0)
    {  // if inches then change to mm value
        strcpy (dualUnits, "mm");
    }
    else if (strcmp(units,"in/hour") == 0)
    {  // if inches then change to mm
        strcpy (dualUnits, "mm/h");
    }
    else if ((strcmp(units,"mm/hr") == 0) || 
             (strcmp(units,"mm/hour") == 0) || 
             (strcmp(units,"mm/h") == 0))
    { // if in/hr then change to mm/hr value
        strcpy (dualUnits, "in/hour");
    }
    else if ((strcmp(units,"cm/hr") == 0) ||
             (strcmp(units,"cm/hour") == 0) || 
             (strcmp(units,"cm/h") == 0))
    { // if in/hr then change to mm/hr value
        strcpy (dualUnits, "in/hour");
    }
    else if ((strcmp(units,"cm/day") == 0) || (strcmp(units,"cm/Tag") == 0))
    {
        strcpy (dualUnits, "in/day");
    }
    else if ((strcmp(units,"cm/week") == 0) || (strcmp(units,"cm/Woche") == 0))
    {
        strcpy (dualUnits, "in/week");
    }
    else if ((strcmp(units,"in/day") == 0) || (strcmp(units,"inch/day") == 0))
    {
        strcpy (dualUnits, "cm/day");
    }
    else if ((strcmp(units,"in/week") == 0) || (strcmp(units,"inch/week") == 0))
    {
        strcpy (dualUnits, "cm/week");
    }
    else if ((strcmp(units,"hPa") == 0) || (strcmp(units,"mb") == 0))
    {   // if hPa then change to inHg value
        strcpy (dualUnits, "inHg");
    }
    else if (strcmp(units,"inHg") == 0)
    {   // if inHg then change to hPa(mb) value
        strcpy (dualUnits, "hPa");
    }
    else if (strcmp(units,"mm") == 0)
    {  // if mm then change to inches value
        strcpy (dualUnits, "inches");
    }
    else if (strcmp(units,"cm") == 0)
    {  // if cm then change to inches value
        strcpy (dualUnits, "inches");
    }
    else if (strcmp(units,"mph") == 0)
    {
        strcpy (dualUnits, "km/h");
    }
    else if (strcmp(units,"km/h") == 0)
    {
        strcpy (dualUnits, "mph");
    }
    else if (strcmp(units,"m/s") == 0)
    {
        strcpy (dualUnits, "mph");
    }
    else if (strcmp(units,"knots") == 0)
    {
        strcpy (dualUnits, "mph");
    }
    else
    { // no conversion no second display
        strcpy (dualUnits, "");
    }
    return;
}

