/*---------------------------------------------------------------------------
 
  FILENAME:
        wvutils.c
 
  PURPOSE:
        Provide some global utility API methods.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        05/29/2005      M.S. Teel       0               Original
 
  NOTES:
        
 
  LICENSE:
        Copyright (c) 2004, Mark S. Teel (mark@teel.ws)
  
        This source code is released for free distribution under the terms 
        of the GNU General Public License.
  
----------------------------------------------------------------------------*/

//  ... System header files
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <termios.h>
#include <time.h>
#include <errno.h>
#include <math.h>
#include <sys/stat.h>

#include <radsysdefs.h>
#include <radmsgLog.h>
#include <radbuffers.h>

//  ... Local header files
#include <sysdefs.h>
#include <services.h>
#include <beaufort.h>

#ifndef BUILD_UTILITIES
#include <wvconfig.h>
static uint16_t VerboseMask, DaemonMask;
#endif

static int      IsMetricRainMM = 1;

// Define a validity test macro:
#define WV_UTILS_VALID(x)       ((x < -300 || x > 300 || !isfinite(x)) ? FALSE : TRUE)


//  ... define methods here

#ifndef BUILD_UTILITIES
#ifndef _WXT510_CONFIG_ONLY
// ***************************************************************************
// * Output Controlled General Event Logging
// ***************************************************************************
int wvutilsSetVerbosity (uint16_t daemonBitMask)
{
    const char*     sValue;
    int             i;

    sValue = wvconfigGetStringValue(configItem_STATION_VERBOSE_MSGS);
    if (sValue == NULL)
    {
        return ERROR;
    }

    DaemonMask = daemonBitMask;

    // be backwards compatible
    if (strlen(sValue) < 8)
    {
        i = atoi (sValue);
        if (i)
            VerboseMask = WV_VERBOSE_ALL;
        else
            VerboseMask = 0x0000;
    }
    else
    {
        // process bit-by-bit
        VerboseMask = 0x0000;
        for (i = 0; i < 8; i ++)
        {
            if (sValue[i] == '1')
                VerboseMask |= (1 << (7-i));
        }
    }

    return OK;
}

int wvutilsToggleVerbosity (void)
{
    int     retVal;

    if (DaemonMask & VerboseMask)
    {
        retVal = 0;
        VerboseMask &= ~DaemonMask;
    }
    else
    {
        retVal = 1;
        VerboseMask |= DaemonMask;
    }

    return retVal;
}

void wvutilsLogEvent (int priority, char *format, ...)
{
    va_list     argList;
    char        temp1[RADMSGLOG_MAX_LENGTH+64];

    if ((DaemonMask & VerboseMask) == 0)
    {
        return;
    }
    else
    {
        // print the var arg stuff to the buffer
        va_start (argList, format);
        vsprintf (temp1, format, argList);
        va_end   (argList);
    
        radMsgLog (priority, temp1);
    }

    return;
}
#endif
#endif

// ***************************************************************************
// * Calculated Weather Data
// ***************************************************************************
//  calculate the heat index
float wvutilsCalculateHeatIndex (float temp, float humidity)
{
    double      T1, T2, T3, T4, T5, T6, T7, T8, T9, result;

    if (temp <= ARCHIVE_VALUE_NULL || humidity <= ARCHIVE_VALUE_NULL)
        return ARCHIVE_VALUE_NULL;

    if (temp < 75.0)
    {
        return temp;
    }

    T1 = -42.379;
    T2 = 2.04901523 * temp;
    T3 = 10.14333127 * humidity;
    T4 = -0.22475541 * temp * humidity;
    T5 = -6.83783E-3 * (temp * temp);
    T6 = -5.481717E-2 * (humidity * humidity);
    T7 = 1.22874E-3 * (temp * temp) * humidity;
    T8 = 8.5282E-4 * temp * (humidity * humidity);
    T9 = -1.99E-6 * (temp * temp) * (humidity * humidity);
    result = T1 + T2 + T3 + T4 + T5 + T6 + T7 + T8 + T9;

	if (! WV_UTILS_VALID(result))
	{
		return ARCHIVE_VALUE_NULL;
	}

    return (float)result;
}

//  calculate the wind chill
float wvutilsCalculateWindChill (float temp, float windspeed)
{
    double      T1, T2, T3, T4;
    double      result;

    if (temp <= ARCHIVE_VALUE_NULL || windspeed <= ARCHIVE_VALUE_NULL) 
        return ARCHIVE_VALUE_NULL;

    if (temp >= 50.0 || windspeed <= 3.0)
    {
        return temp;
    }

    T1 = 35.74;
    T2 = 0.6215 * temp;
    T3 = -1.0 * (35.75 * pow (windspeed, 0.16));
    T4 = 0.4275 * temp * pow (windspeed, 0.16);
    result = T1 + T2 + T3 + T4;

	if (! WV_UTILS_VALID(result))
	{
		return ARCHIVE_VALUE_NULL;
	}

    return (float)result;
}

//  calculate the dewpoint
float wvutilsCalculateDewpoint (float temp, float humidity)
{
    float       Tc, Es, E, Tdc;
    float       result;

    if (temp <= ARCHIVE_VALUE_NULL || humidity <= ARCHIVE_VALUE_NULL) 
        return ARCHIVE_VALUE_NULL;

    Tc  = (5.0/9.0)*(temp - 32.0);
    Es  = 6.11 * pow ((double)10.0, (double)(7.5 * (Tc/(237.7 + Tc))));
    E   = (humidity * Es)/100;
    Tdc = (-430.22 + 237.7 * log(E))/(-log(E)+19.08);
    result = ((9.0/5.0) * Tdc) + 32;

	if (! WV_UTILS_VALID(result))
	{
		return ARCHIVE_VALUE_NULL;
	}

    return result;
}

//  Calculate the "e ^ (-mgh/RT)" term for pressure conversions:
static double calculatePressureTerm (float tempF, float elevationFT)
{
    double      exponent;
    double      elevMeters = (double)wvutilsConvertFeetToMeters(elevationFT);
    double      tempKelvin = (double)wvutilsConvertFToC(tempF) + 273.15;

    // e ^ -elevMeters/(tempK * 29.263)
    exponent = (-elevMeters);

    // degrees Kelvin (T)
    exponent /= (tempKelvin * 29.263);

    // e ^ (-mgh/RT)
    exponent = exp(exponent);

    return exponent;
}

//  calculate sea level pressure from station pressure
float wvutilsConvertSPToSLP (float SP, float tempF, float elevationFT)
{
    double      SLP, PT;

    // Formula used: SLP = SP / PressureTerm
    // compute SLP:
    PT = calculatePressureTerm (tempF, elevationFT);
    if (PT != 0.0)
        SLP = SP / PT;
    else
        SLP = 0.0;

    return (float)SLP;
}

//  calculate station pressure from sea level pressure
float wvutilsConvertSLPToSP (float SLP, float tempF, float elevationFT)
{
    double      SP, PT;

    // Formula used: SP = SLP * PressureTerm
    // compute PressureTerm:
    PT = calculatePressureTerm (tempF, elevationFT);
    SP = SLP * PT;

    return (float)SP;
}

//  calculate altimeter pressure from station pressure
float wvutilsConvertSPToAltimeter (float SPInches, float elevationFT)
{
    double      magicEXP            = 0.190284;
    double      inverseMagicEXP     = 1 / magicEXP;
    double      elevMeters          = (double)wvutilsConvertFeetToMeters(elevationFT);
    double      stationPressureMB   = (double)wvutilsConvertINHGToHPA(SPInches);
    double      constantTerm;
    double      variableTerm;
    double      tempdouble;
    double      altimeter;

    // Formula used: http://www.wrh.noaa.gov/slc/projects/wxcalc/formulas/altimeterSetting.pdf
    
    // calculate the constant term
    constantTerm =  pow (1013.25, magicEXP);
    constantTerm *= 0.0065;
    constantTerm /= 288;

    // calculate the variable term
    tempdouble = stationPressureMB - 0.3;
    tempdouble = pow (tempdouble, magicEXP);
    variableTerm = elevMeters / tempdouble;

    // compute main term
    tempdouble = constantTerm * variableTerm;
    tempdouble += 1;
    tempdouble = pow (tempdouble, inverseMagicEXP);

    // compute altimeter
    altimeter = stationPressureMB - 0.3;
    altimeter *= tempdouble;

    // finally, convert back to inches
    altimeter *= 0.0295299;
    
    return (float)altimeter;
}

// ***************************************************************************
// * Converters
// ***************************************************************************

// Configurable wind units:
static HTML_WUNITS  WVU_WindUnits;
void wvutilsSetWindUnits(HTML_WUNITS units)
{
    WVU_WindUnits = units;
}

char* wvutilsGetWindUnitLabel(void)
{
    static char     W_Units_Label[16];

    switch (WVU_WindUnits)
    {
    case HTML_WINDUNITS_MPH:
        strncpy(W_Units_Label, "mph", 3);
        break;
    case HTML_WINDUNITS_MS:
        strncpy(W_Units_Label, "m/s", 3);
        break;
    case HTML_WINDUNITS_KNOTS:
        strncpy(W_Units_Label, "knots", 5);
        break;
    case HTML_WINDUNITS_KMH:
        strncpy(W_Units_Label, "km/h", 4);
        break;
    }

    return W_Units_Label;
}

float wvutilsGetWindSpeed(float mph)
{
    float   RetVal = -1;

    switch (WVU_WindUnits)
    {
    case HTML_WINDUNITS_MPH:
        RetVal = mph;
        break;
    case HTML_WINDUNITS_MS:
        RetVal = wvutilsConvertMPHToMPS(mph);
        break;
    case HTML_WINDUNITS_KNOTS:
        RetVal = wvutilsConvertMPHToKnots(mph);
        break;
    case HTML_WINDUNITS_KMH:
        RetVal = wvutilsConvertMPHToKPH(mph);
        break;
    }

    return RetVal;
}

float wvutilsGetWindSpeedMetric(float kmh)
{
    float   RetVal = -1;

    switch (WVU_WindUnits)
    {
    case HTML_WINDUNITS_MPH:
        RetVal = wvutilsConvertKPHToMPH(kmh);
        break;
    case HTML_WINDUNITS_MS:
        RetVal = wvutilsConvertKPHToMPS(kmh);
        break;
    case HTML_WINDUNITS_KNOTS:
        RetVal = wvutilsConvertKPHToKnots(kmh);
        break;
    case HTML_WINDUNITS_KMH:
        RetVal = kmh;
        break;
    }

    return RetVal;
}

void wvutilsSetRainIsMM(int setValue)
{
    IsMetricRainMM = setValue;
}

int wvutilsGetRainIsMM(void)
{
    return IsMetricRainMM;
}

float wvutilsConvertFToC (float fahrenValue)
{
    float      retVal;
    
    if (fahrenValue <= ARCHIVE_VALUE_NULL) 
        return ARCHIVE_VALUE_NULL;

    retVal = fahrenValue - 32.0;
    retVal *= 5.0;
    retVal /= 9.0;
    
    return retVal;
}

float wvutilsConvertCToF (float celsiusValue)
{
    float      retVal;
    
    if (celsiusValue <= ARCHIVE_VALUE_NULL) 
        return ARCHIVE_VALUE_NULL;
    
    retVal = celsiusValue * 9.0;
    retVal /= 5.0;
    retVal += 32.0;
    
    return retVal;
}

float wvutilsConvertDeltaFToC (float fahrenValue)
{
    float      retVal;
    
    if (fahrenValue <= ARCHIVE_VALUE_NULL) 
        return ARCHIVE_VALUE_NULL;
    
    retVal = fahrenValue * 5.0;
    retVal /= 9.0;
    
    return retVal;
}

float wvutilsConvertINHGToHPA (float inches)
{
    float      retVal;
    
    if (inches <= ARCHIVE_VALUE_NULL) 
        return ARCHIVE_VALUE_NULL;
    
    retVal = inches / 0.0295299;
    
    return retVal;
}

float wvutilsConvertHPAToINHG (float mb)
{
    float      retVal;
    
    if (mb <= ARCHIVE_VALUE_NULL) 
        return ARCHIVE_VALUE_NULL;
    
    retVal = mb * 0.0295299;
    
    return retVal;
}

float wvutilsConvertINToCM (float inches)
{
    float      retVal;
    
    if (inches <= ARCHIVE_VALUE_NULL) 
        return ARCHIVE_VALUE_NULL;
    
    retVal = inches * 2.54;
    
    return retVal;
}

float wvutilsConvertCMToIN (float cm)
{
    float      retVal;
    
    if (cm <= ARCHIVE_VALUE_NULL) 
        return ARCHIVE_VALUE_NULL;
    
    retVal = cm / 2.54;
    
    return retVal;
}

float wvutilsConvertINToMM (float inches)
{
    float      retVal;
    
    if (inches <= ARCHIVE_VALUE_NULL) 
        return ARCHIVE_VALUE_NULL;
    
    retVal = inches * 25.4;
    
    return retVal;
}

float wvutilsConvertMMToIN (float mm)
{
    float      retVal;
    
    if (mm <= ARCHIVE_VALUE_NULL) 
        return ARCHIVE_VALUE_NULL;
    
    retVal = mm / 25.4;
    
    return retVal;
}

float wvutilsConvertRainINToMetric (float inches)
{
    if (IsMetricRainMM)
    {
        return wvutilsConvertINToMM(inches);
    }
    else
    {
        return wvutilsConvertINToCM(inches);
    }
}

float wvutilsConvertRainMetricToIN (float inches)
{
    if (IsMetricRainMM)
    {
        return wvutilsConvertMMToIN(inches);
    }
    else
    {
        return wvutilsConvertCMToIN(inches);
    }
}

float wvutilsConvertMilesToKilometers (float miles)
{
    float      retVal;
    
    if (miles <= ARCHIVE_VALUE_NULL) 
        return ARCHIVE_VALUE_NULL;
    
    retVal = miles * 1.609;
    
    return retVal;
}

float wvutilsConvertMPHToKPH (float mph)
{
    float      retVal;

    if (mph <= ARCHIVE_VALUE_NULL)
        return ARCHIVE_VALUE_NULL;

    retVal = 1.609344 * mph;

    return retVal;
}

float wvutilsConvertKPHToMPH (float kph)
{
    float      retVal;

    if (kph <= ARCHIVE_VALUE_NULL)
        return ARCHIVE_VALUE_NULL;

    retVal = kph / 1.609344;

    return retVal;
}

float wvutilsConvertKPHToMPS (float kph)
{
    float      retVal;

    if (kph <= ARCHIVE_VALUE_NULL)
        return ARCHIVE_VALUE_NULL;

    retVal = kph * 0.2777697;

    return retVal;
}

float wvutilsConvertKPHToKnots (float kph)
{
    float      retVal;

    if (kph <= ARCHIVE_VALUE_NULL)
        return ARCHIVE_VALUE_NULL;

    retVal = kph * 0.5399417;

    return retVal;
}
float wvutilsConvertMPHToMPS (float mph)
{
    float      retVal;

    if (mph <= ARCHIVE_VALUE_NULL)
        return ARCHIVE_VALUE_NULL;

    retVal = 0.447027 * mph;

    return retVal;
}

float wvutilsConvertMPHToKnots (float mph)
{
    float      retVal;

    if (mph <= ARCHIVE_VALUE_NULL)
        return ARCHIVE_VALUE_NULL;

    retVal = 0.8689762 * mph;

    return retVal;
}

float wvutilsConvertMPSToKPH (float mps)
{
    float      retVal;
    
    if (mps <= ARCHIVE_VALUE_NULL) 
        return ARCHIVE_VALUE_NULL;
    
    retVal = mps / 3.6;
    
    return retVal;
}

float wvutilsConvertMPSToMPH (float mps)
{
    float      retVal;
    
    if (mps <= ARCHIVE_VALUE_NULL) 
        return ARCHIVE_VALUE_NULL;
    
    retVal = mps * 2.2369;
    
    return retVal;
}

float wvutilsConvertMPSToKnots (float mps)
{
    float      retVal;

    if (mps <= ARCHIVE_VALUE_NULL)
        return ARCHIVE_VALUE_NULL;

    retVal = mps * 1.943759;

    return retVal;
}

float wvutilsConvertKnotsToKPH (float mps)
{
    float      retVal;
    
    if (mps <= ARCHIVE_VALUE_NULL) 
        return ARCHIVE_VALUE_NULL;
    
    retVal = mps / 1.852;
    
    return retVal;
}

float wvutilsConvertKnotsToMPS (float mps)
{
    float      retVal;

    if (mps <= ARCHIVE_VALUE_NULL)
        return ARCHIVE_VALUE_NULL;

    retVal = mps * 0.388445;

    return retVal;
}

float wvutilsConvertKnotsToMPH (float knots)
{
    float      retVal;
    
    if (knots <= ARCHIVE_VALUE_NULL)
        return ARCHIVE_VALUE_NULL;
    
    retVal = knots * 1.150779;

    return retVal;
}

float wvutilsConvertKilometersToMiles (float km)
{
    float      retVal;
    
    if (km <= ARCHIVE_VALUE_NULL) 
        return ARCHIVE_VALUE_NULL;
    
    retVal = km / 1.609;
    
    return retVal;
}

float wvutilsConvertFeetToMeters (float feet)
{
    float      retVal;
    
    if (feet <= ARCHIVE_VALUE_NULL) 
        return ARCHIVE_VALUE_NULL;
    
    retVal = feet * 0.3048;
    
    return retVal;
}


//  Determine if it is day or night
//  Returns TRUE or FALSE
int wvutilsIsDayTime (int16_t sunrise, int16_t sunset)
{
    time_t          currTime;
    struct tm       locTime;
    int             sunriseHour, sunriseMinute;
    int             sunsetHour, sunsetMinute;
    int             currentHour, currentMinute;
    int             isInverted = FALSE;

    // is it always day or always night?
    if (sunrise == -1)
    {
        return TRUE;
    }
    else if (sunrise == -2)
    {
        return FALSE;
    }

    // get the current time
    currTime = time (NULL);
    localtime_r (&currTime, &locTime);
    
    sunriseHour     = EXTRACT_PACKED_HOUR(sunrise);
    sunriseMinute   = EXTRACT_PACKED_MINUTE(sunrise);
    sunsetHour      = EXTRACT_PACKED_HOUR(sunset);
    sunsetMinute    = EXTRACT_PACKED_MINUTE(sunset);
    currentHour     = locTime.tm_hour;
    currentMinute   = locTime.tm_min;
    
    // determine if this is an inverted scenario (Set < Rise)
    if (sunriseHour > sunsetHour)
    {
        isInverted = TRUE;
    }
    else if (sunriseHour == sunsetHour && sunriseMinute > sunsetMinute)
    {
        isInverted = TRUE;
    }

    // walk through scenarios
    if (isInverted)
    {
        if (currentHour < sunsetHour)
        {
            return TRUE;
        }
        else if (currentHour == sunsetHour && currentMinute < sunsetMinute)
        {
            return TRUE;
        }
        else
        {
            if (currentHour < sunriseHour)
            {
                return FALSE;
            }
            else if (currentHour == sunriseHour && currentMinute < sunriseMinute)
            {
                return FALSE;
            }
            else
            {
                return TRUE;
            }
        }
    }
    else
    {
        // normal scenario
        if (currentHour < sunriseHour)
        {
            return FALSE;
        }
        else if (currentHour == sunriseHour && currentMinute < sunriseMinute)
        {
            return FALSE;
        }
        else
        {
            if (currentHour < sunsetHour)
            {
                return TRUE;
            }
            else if (currentHour == sunsetHour && currentMinute < sunsetMinute)
            {
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }
    }
}

// Air Density Calculation
static float getVaporPressure (float tempC)
{
    float       E;

    E = 6.1078 * powf (10, (7.5 * tempC / (237.3 + tempC)));

    return E;
}

static float calcDensity (float abspressmb, float e, float tc)
{
    float       Rv = 461.4964;
    float       Rd = 287.0531;
    float       tk = tc + 273.15;
    float       pv = e * 100;
    float       pd = (abspressmb - e) * 100;
    float       d = (pv/(Rv*tk)) + (pd/(Rd*tk));
    return d;
}

float wvutilsCalculateAirDensity (float tempF, float bp, float dp)
{
    float       tempC = wvutilsConvertFToC (tempF);
    float       bpMB = wvutilsConvertINHGToHPA (bp);
    float       dewpoint = wvutilsConvertFToC (dp);
    float       emb = getVaporPressure (dewpoint);
    float       density = calcDensity (bpMB, emb, tempC);

    return density;
}

time_t wvutilsPackedTimeToTimeT (uint16_t packedDate, uint16_t packedTime)
{
    time_t      timeT;
    struct tm   locTime;

    // convert everything to epoch time
    memset (&locTime, 0, sizeof(locTime));
    locTime.tm_year  = EXTRACT_PACKED_YEAR(packedDate) - 1900;
    locTime.tm_mon   = EXTRACT_PACKED_MONTH(packedDate) - 1;
    locTime.tm_mday  = EXTRACT_PACKED_DAY(packedDate);
    locTime.tm_hour  = EXTRACT_PACKED_HOUR(packedTime);
    locTime.tm_min   = EXTRACT_PACKED_MINUTE(packedTime);
    locTime.tm_isdst = -1;
    timeT = mktime (&locTime);
    return timeT;
}

// calculate the time difference in packed date/time format
// does not consider leap years
// returns the delta in minutes
int wvutilsCalculatePackedTimeDelta
(
    uint16_t    newDate,
    uint16_t    newTime,
    uint16_t    oldDate,
    uint16_t    oldTime
)
{
    int         diffMinutes;
    time_t      oldTimeT, newTimeT;

    // convert everything to epoch time
    oldTimeT = wvutilsPackedTimeToTimeT (oldDate, oldTime);
    newTimeT = wvutilsPackedTimeToTimeT (newDate, newTime);

    diffMinutes = (int)(newTimeT - oldTimeT);
    diffMinutes /= 60;
    return diffMinutes;
}    

// increment a packed time value by the given minutes, rolls over at 24:00
uint16_t wvutilsIncrementPackedTime (uint16_t pTime, int minutes)
{
    int         tempMin, tempHour;
    uint16_t    retVal;

    tempMin = EXTRACT_PACKED_MINUTE(pTime) + minutes;
    tempHour = EXTRACT_PACKED_HOUR(pTime);

    if (tempMin >= 60)
    {
        tempHour += tempMin/60;
        tempMin %= 60;
    }

    if (tempHour > 24 || (tempHour == 24 && tempMin > 0))
    {
        // allow 24:00, but nothing greater
        tempHour %= 24;
    }

    retVal = (tempHour * 100) + tempMin;
    return retVal;
}
    
// produce a float string fixing the truncation annoyance
char *wvutilsPrintFloat (float value, int decPlaces)
{
    static char     flbuffer[32];
    char            format[16];
    int             intTemp;

    intTemp = (int)(value * pow (10, (decPlaces+1)));
    if ((intTemp % 10) > 4)
        intTemp += 5;
    intTemp /= 10;
    sprintf (format, "%s.%1.1d%s", "%", decPlaces, "f");
    sprintf (flbuffer, format, (float)intTemp/(pow (10, decPlaces)));
    return flbuffer;
}

char *wvutilsConvertToBeaufortScale (int windSpeed)
{
    static char         beaufortBfr[32];

    if (windSpeed == Beaufort_Calm)
    {
        sprintf (beaufortBfr, "Calm");
    }
    else if (windSpeed <= Beaufort_LightAir)
    {
        sprintf (beaufortBfr, "Light Air");
    }
    else if (windSpeed <= Beaufort_LightBreeze)
    {
        sprintf (beaufortBfr, "Light Breeze");
    }
    else if (windSpeed <= Beaufort_GentleBreeze)
    {
        sprintf (beaufortBfr, "Gentle Breeze");
    }
    else if (windSpeed <= Beaufort_ModerateBreeze)
    {
        sprintf (beaufortBfr, "Moderate Breeze");
    }
    else if (windSpeed <= Beaufort_FreshBreeze)
    {
        sprintf (beaufortBfr, "Fresh Breeze");
    }
    else if (windSpeed <= Beaufort_StrongBreeze)
    {
        sprintf (beaufortBfr, "Strong Breeze");
    }
    else if (windSpeed <= Beaufort_NearGale)
    {
        sprintf (beaufortBfr, "Near Gale");
    }
    else if (windSpeed <= Beaufort_Gale)
    {
        sprintf (beaufortBfr, "Gale");
    }
    else if (windSpeed <= Beaufort_SevereGale)
    {
        sprintf (beaufortBfr, "Severe Gale");
    }
    else if (windSpeed <= Beaufort_Storm)
    {
        sprintf (beaufortBfr, "Storm");
    }
    else if (windSpeed <= Beaufort_ViolentStorm)
    {
        sprintf (beaufortBfr, "Violent Storm");
    }
    else // if (windSpeed <= Beaufort_Hurricane)
    {
        sprintf (beaufortBfr, "Hurricane");
    }
   
    return beaufortBfr;
}

void wvutilsSendPMONPollResponse(int mask, PMON_PROCESS_TYPES process)
{
    WVIEW_MSG_POLL_RESPONSE     response;

    if (! PMON_PROCESS_ISSET(mask,process))
    {
        return;
    }

    response.pid = getpid();
    radMsgRouterMessageSend (WVIEW_MSG_TYPE_POLL_RESPONSE, &response, sizeof(response));
    return;
}

int wvutilsGetDayStartTime(int archiveInterval)
{
    time_t          ntime = time(NULL);
    struct tm       locTime;
    int             retVal;

    localtime_r(&ntime, &locTime);
    locTime.tm_min = ((locTime.tm_min/archiveInterval)*archiveInterval);
    locTime.tm_sec = 0;
    ntime = mktime(&locTime);
    ntime -= WV_SECONDS_IN_DAY;
    localtime_r(&ntime, &locTime);
    retVal = ((60 * locTime.tm_hour) + locTime.tm_min)/archiveInterval;
    return retVal;
}

time_t wvutilsGetWeekStartTime(int archiveInterval)
{
    time_t          ntime = time(NULL);
    struct tm       locTime;

    ntime -= WV_SECONDS_IN_WEEK;
    ntime -= WV_SECONDS_IN_HOUR;
    localtime_r (&ntime, &locTime);
    locTime.tm_min = 0;
    ntime = mktime (&locTime);
    return ntime;
}

time_t wvutilsGetMonthStartTime (int archiveInterval)
{
    time_t          ntime = time(NULL);
    struct tm       locTime;

    ntime -= WV_SECONDS_IN_MONTH;
    ntime -= WV_SECONDS_IN_HOUR;
    localtime_r (&ntime, &locTime);
    locTime.tm_min = 0;
    ntime = mktime (&locTime);
    return ntime;
}

time_t wvutilsGetYearStartTime (int archiveInterval)
{
    time_t          ntime = time(NULL);
    struct tm       locTime;

    ntime -= WV_SECONDS_IN_YEAR;
    ntime -= SECONDS_IN_INTERVAL(archiveInterval);
    localtime_r (&ntime, &locTime);
    locTime.tm_hour = 0;
    locTime.tm_min  = archiveInterval;
    locTime.tm_sec  = 0;
    locTime.tm_isdst = -1;
    ntime = mktime (&locTime);
    return ntime;
}

int wvutilsGetYear (time_t ntime)
{
    struct tm       locTime;
    localtime_r (&ntime, &locTime);
    return (locTime.tm_year + 1900);
}

int wvutilsGetMonth (time_t ntime)
{
    struct tm       locTime;
    localtime_r (&ntime, &locTime);
    return (locTime.tm_mon + 1);
}

int wvutilsGetDay (time_t ntime)
{
    struct tm       locTime;
    localtime_r (&ntime, &locTime);
    return (locTime.tm_mday);
}

int wvutilsGetHour (time_t ntime)
{
    struct tm       locTime;
    localtime_r (&ntime, &locTime);
    return (locTime.tm_hour);
}

int wvutilsGetMin (time_t ntime)
{
    struct tm       locTime;
    localtime_r (&ntime, &locTime);
    return (locTime.tm_min);
}

int wvutilsGetSec (time_t ntime)
{
    struct tm       locTime;
    localtime_r (&ntime, &locTime);
    return (locTime.tm_sec);
}

int wvutilsTimeIsToday(time_t checkTime)
{
    time_t          ntime = time(NULL);
    struct tm       nowTime, testTime;

    localtime_r (&ntime, &nowTime);
    localtime_r (&checkTime, &testTime);

    if (nowTime.tm_year == testTime.tm_year)
    {
        if (nowTime.tm_mon == testTime.tm_mon)
        {
            if (nowTime.tm_mday == testTime.tm_mday)
            {
                return TRUE;
            }
        }
        
    }
    return FALSE;
}

char* wvutilsGetArchivePath(void)
{
    static char     dbFilePath[128];

    sprintf (dbFilePath, "%s/archive", WVIEW_RUN_DIR);
    return dbFilePath;
}

char* wvutilsGetConfigPath(void)
{
    static char     dbFilePath[128];

    sprintf (dbFilePath, "%s", WV_CONFIG_DIR);
    return dbFilePath;
}

int wvutilsWriteMarkerFile(const char* filePath, time_t marker)
{
    FILE*       pFile;

    pFile = fopen(filePath, "w");
    if (pFile == NULL)
    {
        return ERROR;
    }

    fprintf (pFile, "%u", (uint32_t)marker);
    fclose (pFile);
    return OK;
}

time_t wvutilsReadMarkerFile(const char* filePath)
{
    FILE*       pFile;
    char        tempBfr[32];
    uint32_t    retVal;

    pFile = fopen(filePath, "r");
    if (pFile == NULL)
    {
        return (time_t)0;
    }

    if (fgets(tempBfr, sizeof(tempBfr), pFile) == NULL)
    {
        fclose(pFile);
        return (time_t)0;
    }

    retVal = strtol(tempBfr, NULL, 10);
    if (retVal <= 0)
    {
        fclose(pFile);
        return (time_t)0;
    }

    fclose (pFile);
    return (time_t)retVal;
}

// Define a SIGCHLD handler to wait for child processes to exit:
// Should only be called from process signal handler.
void wvutilsWaitForChildren(void)
{
    // Wait for any remaining processes:
    while (waitpid(-1, NULL, WNOHANG) > 0);

    return;
}

// Define a NULL-terminating strncpy:
int wvstrncpy(char *d, const char *s, size_t bufsize)
{
	size_t     len;
	size_t     ret;
	
	if (!d || !s) 
        return 0;
	len = strlen(s);
	ret = len;
	if (bufsize <= 0) 
        return 0;
	if (len >= bufsize) 
        len = bufsize-1;
	memcpy(d, s, len);
	d[len] = 0;
	
	return ret;
}

void wvstrtrim(char *d)
{
    char* end = d + strlen(d);

    // find the last non-whitespace character:
    while ((end != d) && isspace( *(end-1) ))
    {
        --end;
    }

    *end = '\0';
    return;
}

static int      lastDSTState;
int wvutilsDetectDSTInit(void)
{
    time_t              ntime;
    struct tm           tmtime;

    ntime = time (NULL);
    localtime_r (&ntime, &tmtime);
    lastDSTState = tmtime.tm_isdst;
    return 0;
}

// returns:
// 0  => no change
// -1 => fall back from DST to non-DST
// 1  => spring forward from non-DST to DST
int wvutilsDetectDSTChange(void)
{
    time_t              ntime;
    struct tm           tmtime;
    int                 retVal;

    ntime = time (NULL);
    localtime_r (&ntime, &tmtime);
    
    if (tmtime.tm_isdst != lastDSTState)
    {
        if (lastDSTState != 0)
        {
            // fall back
            retVal = WVUTILS_DST_FALL_BACK;
        }
        else
        {
            // spring forward
            retVal = WVUTILS_DST_SPRING_FORWARD;
        }

        lastDSTState = tmtime.tm_isdst;
        return retVal;
    }
    else
    {
        return WVUTILS_DST_NO_CHANGE;
    }
}

// Assign a degree value to received string representation for wind direction:
// Returns degree equivalent or -1 if ERROR:
static char*        directionLabels[16] =
{
    "N",
    "NNE",
    "NE",
    "ENE",
    "E",
    "ESE",
    "SE",
    "SSE",
    "S",
    "SSW",
    "SW",
    "WSW",
    "W",
    "WNW",
    "NW",
    "NNW"
};
int wvutilsConvertWindStrToDegrees(const char* windStr)
{
    int     i;
    float   tempVal = -1;

    for (i = 0; i < 16; i ++)
    {
        if (! strcmp(windStr, directionLabels[i]))
        {
            // found him:
            tempVal = 22.5 * (float)i;
        }
    }

    if (tempVal < 0)
    {
        return ERROR;
    }

    return (int)tempVal;
}

char* wvutilsCreateCWOPVersion(char* wviewStr)
{
    static char     CWOPStr[24];
    int             index;

    memset(CWOPStr, 0, 24);
    for (index = 0; index < 23 && index < strlen(wviewStr); index ++)
    {
        if (wviewStr[index] == ' ' || wviewStr[index] == '.')
        {
            CWOPStr[index] = '_';
        }
        else
        {
            CWOPStr[index] = wviewStr[index];
        }
    }

    return CWOPStr;
}

// Calculate the apparent temperature:
// Australian Bureau of Meteorology http://reg.bom.gov.au/info/thermal_stress/#atapproximation [^]
// Robert G. Steadman. 1994: Norms of apparent temperature in Australia.
float wvutilsCalculateApparentTemp(float temp, float windspeed, float humidity)
{
    double AT,Ta,e,ws;
    double result;
    
    Ta = wvutilsConvertFToC(temp);
    e = humidity / 100.0 * 6.105 * exp(17.27 * Ta/( 237.7 + Ta));
    ws = wvutilsConvertMPHToMPS(windspeed);
    AT = Ta + 0.33 * e - 0.70 * ws - 4.00;
    result = wvutilsConvertCToF(AT);
    
    return (float)result;
}

// Calculate the wet bulb temperature:
// Based on NOAA Java Script http://www.srh.noaa.gov/epz/?n=wxcalc_rh
float wvutilsCalculateWetBulbTemp( float temp, float humidity, float pressure)
{
    float   tempC = wvutilsConvertFToC(temp);
    float   pressureMB = wvutilsConvertINHGToHPA(pressure);
    float   Ewguess, Eguess;
    float   Es = 6.112 * exp(17.67 * tempC / (tempC + 243.5));
    float   Twguess = 0.0;
    float   incr = 10.0;
    int     previoussign = 1;
    int     cursign;
    float   Edifference = 1;
    float   E2 = Es * (humidity/100.0);

    while (fabs(Edifference) > 0.05)
    {
        Ewguess = 6.112 * exp((17.67 * Twguess) / (Twguess + 243.5));
        Eguess = Ewguess - pressureMB * (tempC - Twguess) * 0.00066 * (1.0 + (0.00115 * Twguess));
        Edifference = E2 - Eguess;
        if (Edifference == 0)
        {
            break;
        }
        else
        {
            if (Edifference < 0)
                cursign = -1;
            else cursign = 1;

            if (cursign != previoussign)
            {
                previoussign = cursign;
                incr = incr/10;
            }
        }
        Twguess = Twguess + incr * previoussign;
    }

    return wvutilsConvertCToF(Twguess);
}
