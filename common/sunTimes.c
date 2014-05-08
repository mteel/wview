/*---------------------------------------------------------------------------
 
  FILENAME:
        sunTimes.c
 
  PURPOSE:
        Provide sunrise and sunset computation utilities.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        01/07/2006      M.S. Teel       0               Original
 
  NOTES:
        
 
  LICENSE:
        Copyright (c) 2006, Mark S. Teel (mark@teel.ws)
  
        This source code is released for free distribution under the terms 
        of the GNU General Public License.
  
----------------------------------------------------------------------------*/

//  ... System header files
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <termios.h>
#include <time.h>
#include <errno.h>
#include <math.h>

#include <sysdefs.h>


// A macro to compute the number of days elapsed since 1/1/2000
#define DAYS_THIS_MILLENIUM(y,m,d) \
    (367L*(y)-((7*((y)+(((m)+9)/12)))/4)+((275*(m))/9)+(d)-730530L)

// Some general conversions

#ifndef PI
#define PI              3.1415926535897932384
#endif

#define RAD_DEG         (180.0 / PI)
#define DEG_RAD         (PI / 180.0)

// trig functions in degrees
#define sind(x)         sin((x)*DEG_RAD)
#define cosd(x)         cos((x)*DEG_RAD)
#define tand(x)         tan((x)*DEG_RAD)

#define atand(x)        (RAD_DEG*atan(x))
#define asind(x)        (RAD_DEG*asin(x))
#define acosd(x)        (RAD_DEG*acos(x))
#define atan2d(y,x)     (RAD_DEG*atan2(y,x))


// This macro computes the length of the day, from sunrise to sunset.
// Sunrise/set is considered to occur when the Sun's upper limb is
// 35 arc minutes below the horizon (this accounts for the refraction
// of the Earth's atmosphere).
#define DAY_LENGTH(year,month,day,lon,lat)  \
                        dayLength(year, month, day, lon, lat, -35.0/60.0, 1)

// This macro computes the length of the day, including civil twilight.
// Civil twilight starts/ends when the Sun's center is 6 degrees below
// the horizon.
#define DAY_LENGTH_CIVIL(year,month,day,lon,lat)  \
                        dayLength(year, month, day, lon, lat, -6.0, 0)

// This macro computes the length of the day, incl. astronomical twilight.
// Astronomical twilight starts/ends when the Sun's center is 18 degrees
// below the horizon.
#define DAY_LENGTH_ASTRONOMICAL(year,month,day,lon,lat)  \
                        dayLength(year, month, day, lon, lat, -18.0, 0)


// This macro computes times for sunrise/sunset.
// Sunrise/set is considered to occur when the Sun's upper limb is
// 35 arc minutes below the horizon (this accounts for the refraction
// of the Earth's atmosphere).
#define SUN_RISE_SET(year,month,day,lon,lat,rise,set)  \
                        sunRiseSet(year, month, day, lon, lat, -35.0/60.0, 1, rise, set)

// This macro computes the start and end times of civil twilight.
// Civil twilight starts/ends when the Sun's center is 6 degrees below
// the horizon.
#define CIVIL_RISE_SET(year,month,day,lon,lat,start,end)  \
                        sunRiseSet(year, month, day, lon, lat, -6.0, 0, start, end)

// This macro computes the start and end times of astronomical twilight.
// Astronomical twilight starts/ends when the Sun's center is 18 degrees
// below the horizon.
#define ASTRO_RISE_SET(year,month,day,lon,lat,start,end)  \
                        sunRiseSet(year, month, day, lon, lat, -18.0, 0, start, end)


// This function reduces any angle to within the first revolution by subtracting
// or adding even multiples of 360.0 until the result is >= 0.0 and < 360.0
#define INV360          (1.0 / 360.0)
static double normalize (double x)
{
    return (x - 360.0 * floor(x * INV360));
}

static double normalize180 (double x)
{
    return (x - 360.0 * floor(x * INV360 + 0.5));
}

// Computes the Sun's ecliptic longitude and distance at an instant given in d,
// number of days since 1/1/2000. The Sun's ecliptic latitude is not computed,
// since it's always very near 0.
static void sunPosition (double d, double *lon, double *r)
{
    double M,         /* Mean anomaly of the Sun */
    w,         /* Mean longitude of perihelion */
    /* Note: Sun's mean longitude = M + w */
    e,         /* Eccentricity of Earth's orbit */
    E,         /* Eccentric anomaly */
    x, y,      /* x, y coordinates in orbit */
    v;         /* True anomaly */

    /* Compute mean elements */
    M = normalize (356.0470 + 0.9856002585 * d);
    w = 282.9404 + 4.70935E-5 * d;
    e = 0.016709 - 1.151E-9 * d;

    /* Compute true longitude and radius vector */
    E = M + e * RAD_DEG * sind(M) * ( 1.0 + e * cosd(M) );
    x = cosd(E) - e;
    y = sqrt(1.0 - e*e) * sind(E);
    *r = sqrt(x*x + y*y);                /* Solar distance */
    v = atan2d( y, x );                  /* True anomaly */
    *lon = v + w;                        /* True solar longitude */
    if (*lon >= 360.0)
        *lon -= 360.0;                   /* Make it 0..360 degrees */
}

static void getSunRAandDec (double d, double *RA, double *dec, double *r)
{
    double lon, obl_ecl, x, y, z;

    /* Compute Sun's ecliptical coordinates */
    sunPosition (d, &lon, r);

    /* Compute ecliptic rectangular coordinates (z=0) */
    x = *r * cosd(lon);
    y = *r * sind(lon);

    /* Compute obliquity of ecliptic (inclination of Earth's axis) */
    obl_ecl = 23.4393 - 3.563E-7 * d;

    /* Convert to equatorial rectangular coordinates - x is uchanged */
    z = y * sind(obl_ecl);
    y = y * cosd(obl_ecl);

    /* Convert to spherical coordinates */
    *RA = atan2d( y, x );
    *dec = atan2d( z, sqrt(x*x + y*y) );
}


// This function computes GMST0, the Greenwhich Mean Sidereal Time at UTC. GMST
// is then the sidereal time at Greenwich at any time of the day.
static double GMST0 (double d)
{
    double sidtim0;

    sidtim0 = normalize ((180.0 + 356.0470 + 282.9404) +
                         (0.9856002585 + 4.70935E-5) * d);
    return sidtim0;
}


/* Note: year,month,date = calendar date, 1801-2099 only.             */
/*       Eastern longitude positive, Western longitude negative       */
/*       Northern latitude positive, Southern latitude negative       */
/*       The longitude value IS critical in this function!            */
/*       altit = the altitude which the Sun should cross              */
/*               Set to -35/60 degrees for rise/set, -6 degrees       */
/*               for civil, -12 degrees for nautical and -18          */
/*               degrees for astronomical twilight.                   */
/*         upper_limb: non-zero -> upper limb, zero -> center         */
/*               Set to non-zero (e.g. 1) when computing rise/set     */
/*               times, and to zero when computing start/end of       */
/*               twilight.                                            */
/*        *rise = where to store the rise time                        */
/*        *set  = where to store the set  time                        */
/*                Both times are relative to the specified altitude,  */
/*                and thus this function can be used to comupte       */
/*                various twilight times, as well as rise/set times   */
/* Return value:  0 = sun rises/sets this day, times stored at        */
/*                    *trise and *tset.                               */
/*               +1 = sun above the specified "horizon" 24 hours.     */
/*                    *trise set to time when the sun is at south,    */
/*                    minus 12 hours while *tset is set to the south  */
/*                    time plus 12 hours. "Day" length = 24 hours     */
/*               -1 = sun is below the specified "horizon" 24 hours   */
/*                    "Day" length = 0 hours, *trise and *tset are    */
/*                    both set to the time when the sun is at south.  */
/*                                                                    */
static int sunRiseSet
(
    int     year,
    int     month,
    int     day,
    double  lon,
    double  lat,
    double  altit,
    int     upper_limb,
    double  *trise,
    double  *tset
)
{
    double  d,          /* Days since 2000 Jan 0.0 (negative before) */
            sr,         /* Solar distance, astronomical units */
            sRA,        /* Sun's Right Ascension */
            sdec,       /* Sun's declination */
            sradius,    /* Sun's apparent radius */
            t,          /* Diurnal arc */
            tsouth,     /* Time when Sun is at south */
            sidtime,    /* Local sidereal time */
            cost;

    int     rc = 0;     /* Return code from function - usually 0 */

    /* Compute d of 12h local mean solar time */
    d = DAYS_THIS_MILLENIUM(year,month,day) + 0.5 - lon/360.0;

    /* Compute local sideral time of this moment */
    sidtime = normalize (GMST0(d) + 180.0 + lon);

    /* Compute Sun's RA + Decl at this moment */
    getSunRAandDec (d, &sRA, &sdec, &sr);

    /* Compute time when Sun is at south - in hours UT */
    tsouth = 12.0 - normalize180 (sidtime - sRA)/15.0;

    /* Compute the Sun's apparent radius, degrees */
    sradius = 0.2666 / sr;

    /* Do correction to upper limb, if necessary */
    if (upper_limb)
        altit -= sradius;

    /* Compute the diurnal arc that the Sun traverses to reach */
    /* the specified altitide altit: */
    cost = (sind(altit) - sind(lat) * sind(sdec)) / (cosd(lat) * cosd(sdec));
    if (cost >= 1.0)
    {
        rc = -1;
        t = 0.0;                /* Sun always below altit */
    }
    else if (cost <= -1.0)
    {
        rc = +1;
        t = 12.0;               /* Sun always above altit */
    }
    else
    {
        t = acosd(cost)/15.0;   /* The diurnal arc, hours */
    }

    /* Store rise and set times - in hours UTC */
    *trise = tsouth - t;
    *tset  = tsouth + t;

    return rc;
}

/* Note: year,month,date = calendar date, 1801-2099 only.             */
/*       Eastern longitude positive, Western longitude negative       */
/*       Northern latitude positive, Southern latitude negative       */
/*       The longitude value is not critical. Set it to the correct   */
/*       longitude if you're picky, otherwise set to to, say, 0.0     */
/*       The latitude however IS critical - be sure to get it correct */
/*       altit = the altitude which the Sun should cross              */
/*               Set to -35/60 degrees for rise/set, -6 degrees       */
/*               for civil, -12 degrees for nautical and -18          */
/*               degrees for astronomical twilight.                   */
/*         upper_limb: non-zero -> upper limb, zero -> center         */
/*               Set to non-zero (e.g. 1) when computing day length   */
/*               and to zero when computing day+twilight length.      */
static double dayLength
(
    int     year,
    int     month,
    int     day,
    double  lon,
    double  lat,
    double  altit,
    int     upper_limb
)
{
    double  d,  /* Days since 2000 Jan 0.0 (negative before) */
            obl_ecl,    /* Obliquity (inclination) of Earth's axis */
            sr,         /* Solar distance, astronomical units */
            slon,       /* True solar longitude */
            sin_sdecl,  /* Sine of Sun's declination */
            cos_sdecl,  /* Cosine of Sun's declination */
            sradius,    /* Sun's apparent radius */
            t,          /* Diurnal arc */
            cost;

    d = DAYS_THIS_MILLENIUM(year,month,day) + 0.5 - lon/360.0;

    /* Compute obliquity of ecliptic (inclination of Earth's axis) */
    obl_ecl = 23.4393 - 3.563E-7 * d;

    /* Compute Sun's position */
    sunPosition (d, &slon, &sr);

    /* Compute sine and cosine of Sun's declination */
    sin_sdecl = sind(obl_ecl) * sind(slon);
    cos_sdecl = sqrt(1.0 - sin_sdecl * sin_sdecl);

    /* Compute the Sun's apparent radius, degrees */
    sradius = 0.2666 / sr;

    /* Do correction to upper limb, if necessary */
    if (upper_limb)
        altit -= sradius;

    /* Compute the diurnal arc that the Sun traverses to reach */
    /* the specified altitide altit: */
    cost = (sind(altit) - sind(lat) * sin_sdecl) /
           (cosd(lat) * cos_sdecl);
    if (cost >= 1.0)
        t = 0.0;                      // Sun always below altit
    else if (cost <= -1.0)
        t = 24.0;                     // Sun always above altit
    else
        t = (2.0/15.0) * acosd(cost); // The diurnal arc, hours

    return t;
}

static int16_t packAndGMTAlign (double value)
{
    int16_t     retVal;
    int         temp;
    time_t      nowtime = time (NULL);
    struct tm   bknTime;
    long        gmtMinsEast;

    localtime_r (&nowtime, &bknTime);

#ifdef HAVE_STRUCT_TM_TM_ZONE
    gmtMinsEast = bknTime.tm_gmtoff/60;
#else
    gmtMinsEast = -(timezone/60);
#endif

    value *= 60;
    temp = (int)value;
    temp += gmtMinsEast;
    temp += 1;
    retVal = ((temp/60)*100) + (temp%60);
    return retVal;
}


// API functions
// all times returned as packed time (hour*100 + minutes)
// lat and long are floats representing degrees:
//   N  => +
//   S  => -
//   E  => +
//   W  => -

// packedRise = packedSet = -1     =>  the sun never sets
// packedRise = packedSet = -2     =>  the sun never rises
// otherwise the valid times in packed format
void sunTimesGetSunRiseSet
(
    int         year,
    int         month,
    int         day,
    float       latitude,
    float       longitude,
    int         type,               // RS_TYPE_SUN, RS_TYPE_CIVIL, RS_TYPE_ASTRO
    int16_t     *packedRise,
    int16_t     *packedSet
)
{
    int         rv;
    double      rise, set;

    switch (type)
    {
        case RS_TYPE_SUN:
            rv = SUN_RISE_SET(year, month, day, (double)longitude, 
                              (double)latitude, &rise, &set);
            break;
        case RS_TYPE_CIVIL:
            rv = CIVIL_RISE_SET(year, month, day, (double)longitude, 
                                (double)latitude, &rise, &set);
            break;
        case RS_TYPE_ASTRO:
            rv = ASTRO_RISE_SET(year, month, day, (double)longitude, 
                                (double)latitude, &rise, &set);
            break;
        case RS_TYPE_MIDDAY:
            rv = SUN_RISE_SET(year, month, day, (double)longitude, 
                              (double)latitude, &rise, &set);
            break;
        default:
            rv = -1;
            break;
    }
    if (rv == 1)
    {
        *packedRise = *packedSet = -1;
    }
    else if (rv == -1)
    {
        *packedRise = *packedSet = -2;
    }
    else
    {
        if (type == RS_TYPE_MIDDAY)
        {
            *packedRise = packAndGMTAlign ((rise+set)/2.0);
        }
        else
        {
            *packedRise = packAndGMTAlign (rise);
            *packedSet = packAndGMTAlign (set);
        }
    }

    return;
}

// Get day length in packed format
int16_t sunTimesGetDayLength
(
    int         year,
    int         month,
    int         day,
    float       latitude,
    float       longitude
)
{
    int         rv;
    double      daylen;
    int16_t     packedLength;

    daylen = DAY_LENGTH(year, month, day, (double)longitude, (double)latitude);
    daylen *= 60;
    daylen += 1;
    rv = (int)daylen;
    packedLength = ((rv/60) * 100) + (rv % 60);
    return packedLength;
}

// Debug application:
#if 0
int main (void)
{
    int     year, month, day;
    double  lon, lat;
    int16_t packedLen, packedRise, packedSet;

    printf ("Longitude (+ is east) and latitude (+ is north): ");
    scanf ("%lf %lf", &lon, &lat);

    for ( ; ; )
    {
        printf ("Input date ( yyyy mm dd ) (ctrl-C exits): ");
        scanf ("%d %d %d", &year, &month, &day);

        packedLen = sunTimesGetDayLength (year, month, day, (float)lat, (float)lon);
        printf ("==> Day length: %2.2d:%2.2d hours\n\n", packedLen/100, packedLen%100);

        sunTimesGetSunRiseSet (year, month, day, (float)lat, (float)lon, 
                               RS_TYPE_MIDDAY, &packedRise, &packedSet);
        if (packedRise == -1)
        {
            printf ("Sun never sets!\n");
        }
        else if (packedRise == -2)
        {
            printf ("Sun never rises!\n");
        }
        else
        {
            printf ("==> Midday: %2.2d:%2.2d Local Time\n\n",
                    packedRise/100, packedRise%100);
        }

        sunTimesGetSunRiseSet (year, month, day, (float)lat, (float)lon, 
                               RS_TYPE_SUN, &packedRise, &packedSet);
        if (packedRise == -1)
        {
            printf ("Sun never sets!\n");
        }
        else if (packedRise == -2)
        {
            printf ("Sun never rises!\n");
        }
        else
        {
            printf ("==> Sunrise: %2.2d:%2.2d, Sunset: %2.2d:%2.2d Local Time\n",
                    packedRise/100, packedRise%100, packedSet/100, packedSet%100);
        }

        sunTimesGetSunRiseSet (year, month, day, (float)lat, (float)lon, 
                               RS_TYPE_CIVIL, &packedRise, &packedSet);
        if (packedRise == -1)
        {
            printf ("Sun never falls below civil twilight!\n");
        }
        else if (packedRise == -2)
        {
            printf ("Sun never rises above civil twilight!\n");
        }
        else
        {
            printf ("==> Civil Twilight Begins: %2.2d:%2.2d, Civil Twilight Ends: %2.2d:%2.2d Local Time\n",
                    packedRise/100, packedRise%100, packedSet/100, packedSet%100);
        }

        sunTimesGetSunRiseSet (year, month, day, (float)lat, (float)lon, 
                               RS_TYPE_ASTRO, &packedRise, &packedSet);
        if (packedRise == -1)
        {
            printf ("Sun never falls below astronomical twilight!\n");
        }
        else if (packedRise == -2)
        {
            printf ("Sun never rises above astronomical twilight!\n");
        }
        else
        {
            printf ("==> Astronomical Twilight Begins: %2.2d:%2.2d, Astronomical Twilight Ends: %2.2d:%2.2d Local Time\n",
                    packedRise/100, packedRise%100, packedSet/100, packedSet%100);
        }
    }
}
#endif

