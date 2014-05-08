/***************************************************************************
 *   Copyright (C) 2007 by Mark Teel   *
 *   mteel@teelworks.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#pragma once

// System include files
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include <sys/stat.h>

// Library include files
#include "afedit.h"

#define DBFILES_RECORD_SIZE         88

enum DailySummaryTimeIndexes
{
    High_Outside_Temperature        = 0,
    Low_Outside_Temperature         = 1,
    High_Inside_Temperature         = 2,
    Low_Inside_Temperature          = 3,
    High_Wind_Chill                 = 4,
    Low_Wind_Chill                  = 5,
    High_Dew_Point                  = 6,
    Low_Dew_Point                   = 7,
    High_Outside_Humidity           = 8,
    Low_Outside_Humidity            = 9,
    High_Inside_Humidity            = 10,
    Low_Inside_Humidity             = 11,
    High_Barometer                  = 12,
    Low_Barometer                   = 13,
    High_Wind_Speed                 = 14,
    High_Average_Wind_Speed         = 15,
    High_Rain_Rate                  = 16,
    High_UV                         = 17,

    High_Solar_Rad                  = 0,
    High_Outside_Heat_Index         = 1,
    Low_Outside_Heat_Index          = 2,
    High_Outside_THSW_Index         = 3,
    Low_Outside_THSW_Index          = 4,
    High_Outside_THW_Index          = 5,
    Low_Outside_THW_Index           = 6,
    High_Outside_Wet_Bulb_Temp      = 7,
    Low_Outside_Wet_Bulb_Temp       = 8
};


//  ... this describes each day in an archive file
typedef struct
{
    int16_t     recordsInDay;       // includes the daily summary record (counts as 2)
    int         startPos;           // The index (starting at 0) of the daily summary record
}__attribute__ ((packed)) DayIndex;

//  ... Header for each archive file
typedef struct
{
    char        idCode [16];        // = {'W','D','A','T','5','.','0',0,0,0,0,0,0,0,5,0}
    int         totalRecords;
    DayIndex    dayIndex [32];      // index records for each day. Index 0 is not used
    // (i.e. the 1'st is at index 1, not index 0)
}__attribute__ ((packed)) HeaderBlock;


//  ... After the Header are a series of 88-byte records with one of the following
//  ... formats.  Note that each day will begin with a daily summary record.

//  ... Daily Summary Record (176 bytes)
typedef struct
{
    uint8_t dataType1;
    uint8_t reserved1;              // this will cause rest of fields to start on an even address

    int16_t dataSpan;               // total # minutes accounted for by physical recs for this day
    int16_t hiOutTemp, lowOutTemp;  // tenths of a degree F
    int16_t hiInTemp, lowInTemp;    // tenths of a degree F
    int16_t avgOutTemp, avgInTemp;  // tenths of a degree F (integrated over the day)
    int16_t hiChill, lowChill;      // tenths of a degree F
    int16_t hiDew, lowDew;          // tenths of a degree F
    int16_t avgChill, avgDew;       // tenths of a degree F
    int16_t hiOutHum, lowOutHum;    // tenths of a percent
    int16_t hiInHum, lowInHum;      // tenths of a percent
    int16_t avgOutHum;              // tenths of a percent
    int16_t hiBar, lowBar;          // thousanths of an inch Hg
    int16_t avgBar;                 // thousanths of an inch Hg
    int16_t hiSpeed, avgSpeed;      // tenths of an MPH
    int16_t dailyWindRunTotal;      // 1/10'th of an mile
    int16_t hi10MinSpeed;           // the highest average wind speed record
    uint8_t dirHiSpeed, hi10MinDir; // direction code (0-15, 255)
    int16_t dailyRainTotal;         // 1/1000'th of an inch
    int16_t hiRainRate;             // 1/100'th inch/hr ???
    int16_t dailyUVDose;            // 1/10'th of a standard MED
    uint8_t hiUV;                   // tenth of a UV Index
    uint8_t timeValues1[27];        // space for 18 time values (see below)

    uint8_t dataType2;              // start of 2nd summary record
    uint8_t reserved2;              // this will cause rest of fields to start on an even address

    // this field is not used now.
    uint16_t todaysWeather;          // bitmapped weather conditions (Fog, T-Storm, hurricane, etc)

    int16_t numWindPackets;         // # of valid packets containing wind data,
    // this is used to indicate reception quality
    int16_t hiSolar;                // Watts per meter squared
    int16_t dailySolarEnergy;       // 1/10'th Ly
    int16_t minSunlight;            // number of accumulated minutes where the avg solar rad > 150
    int16_t dailyETTotal;           // 1/1000'th of an inch
    int16_t hiHeat, lowHeat;        // tenths of a degree F
    int16_t avgHeat;                // tenths of a degree F
    int16_t hiTHSW, lowTHSW;        // tenths of a degree F
    int16_t hiTHW, lowTHW;          // tenths of a degree F

    int16_t integratedHeatDD65;     // integrated Heating Degree Days (65F threshold)
    // tenths of a degree F - Day

    // Wet bulb values are not calculated
    int16_t hiWetBulb, lowWetBulb;  // tenths of a degree F
    int16_t avgWetBulb;             // tenths of a degree F

    uint8_t dirBins[24];            // space for 16 direction bins
    // (Used to calculate monthly dominent Dir)

    uint8_t timeValues2[15];        // space for 10 time values (see below)

    int16_t integratedCoolDD65;     // integrated Cooling Degree Days (65F threshold)
    // tenths of a degree F - Day
    uint8_t reserved3[11];
}__attribute__ ((packed)) DailySummaryRecord;

//  ... standard archive record (88 bytes)
typedef struct
{
    uint8_t dataType;
    uint8_t archiveInterval;        // number of minutes in the archive
    uint8_t iconFlags;              // Icon associated with this record, plus Edit flags
    uint8_t moreFlags;              // Tx Id, etc.

    int16_t packedTime;             // minutes past midnight of the end of the archive period
    int16_t outsideTemp;            // tenths of a degree F
    int16_t hiOutsideTemp;          // tenths of a degree F
    int16_t lowOutsideTemp;         // tenths of a degree F
    int16_t insideTemp;             // tenths of a degree F
    int16_t barometer;              // thousanths of an inch Hg
    int16_t outsideHum;             // tenths of a percent
    int16_t insideHum;              // tenths of a percent
    uint16_t rain;                   // number of clicks + rain collector type code
    int16_t hiRainRate;             // clicks per hour
    int16_t windSpeed;              // tenths of an MPH
    int16_t hiWindSpeed;            // tenths of an MPH
    uint8_t windDirection;          // direction code (0-15, 255)
    uint8_t hiWindDirection;        // direction code (0-15, 255)
    int16_t numWindSamples;         // number of valid ISS packets containing wind data
                                    // this is a good indication of reception
    int16_t solarRad, hisolarRad;   // Watts per meter squared
    uint8_t UV, hiUV;               // tenth of a UV Index

    uint8_t leafTemp[4];            // (whole degrees F) + 90

    int16_t newSensors[7];          // reserved for future use
    uint8_t forecast;               // forecast code during the archive interval

    uint8_t ET;                     // thousandths of an inch

    uint8_t soilTemp[6];            // (whole degrees F) + 90
    uint8_t soilMoisture[6];        // centibars of dryness
    uint8_t leafWetness[4];         // Leaf Wetness code (0-15, 255)
    uint8_t extraTemp[7];           // (whole degrees F) + 90
    uint8_t extraHum[7];            // whole percent
}__attribute__ ((packed)) ArchiveRecord;


// API
class CArchiveFile
{
public:
    // Open the file and set the record pointer to the first record
    CArchiveFile (char* FullPath, bool write = false);
    ~CArchiveFile ();

    bool                IsValid () { return Valid; }

    // If reading...
    ArchiveRecord*      GetNext (int* day);

    // If writing...
    bool                PutNext (ArchiveRecord* record, int day);

private:
    bool                Valid;
    bool                IsWrite;
    bool                IsWriteStarted;
    
    FILE*               filePtr;
    HeaderBlock         fileHdr;
    DailySummaryRecord  sumRecord;

    int                 dayIndex;
    int                 recordIndex;
    ArchiveRecord       arcRecord;

};

