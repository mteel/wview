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

#include "archiveFile.h"

static char     idCode[16] = {'W','D','A','T','5','.','0',0,0,0,0,0,0,0,5,0};

static int insertTimeValue (UCHAR *valarray, int index, USHORT value)
{
    int     fIndex;

    fIndex = (index/2) * 3;
    if ((index % 2) == 0)
    {
        valarray[fIndex] = value & 0xFF;
        valarray[fIndex+2] = (valarray[fIndex+2] & 0xF0) | ((value >> 8) & 0x0F);
    }
    else
    {
        valarray[fIndex+1] = value & 0xFF;
        valarray[fIndex+2] = (valarray[fIndex+2] & 0x0F) | ((value >> 4) & 0xF0);
    }

    return value;
}

// ***************************************************************************
// * Calculated Weather Data
// ***************************************************************************
//  calculate the heat index
static float CalculateHeatIndex (float temp, float humidity)
{
    double      T1, T2, T3, T4, T5, T6, T7, T8, T9, result;

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

    return (float)result;
}

//  calculate the wind chill
static float CalculateWindChill (float temp, float windspeed)
{
    double      T1, T2, T3, T4;
    double      result;

    if (temp >= 50.0 || windspeed <= 3.0)
    {
        return temp;
    }

    T1 = 35.74;
    T2 = 0.6215 * temp;
    T3 = -1.0 * (35.75 * pow (windspeed, 0.16));
    T4 = 0.4275 * temp * pow (windspeed, 0.16);
    result = T1 + T2 + T3 + T4;

    return (float)result;
}

//  calculate the dewpoint
static float CalculateDewpoint (float temp, float humidity)
{
    float       Tc  = (5.0/9.0)*(temp - 32.0);
    float       Es  = 6.11 * pow ((double)10.0, (double)(7.5 * (Tc/(237.7 + Tc))));
    float       E   = (humidity * Es)/100;
    float       Tdc = (-430.22 + 237.7 * log(E))/(-log(E)+19.08);
    return ((9.0/5.0) * Tdc + 32);
}


// Class API
CArchiveFile::CArchiveFile (char* FullPath, bool write)
{
    dayIndex    = 1;
    recordIndex = 0;
    Valid       = false;

    if (write)
    {
        IsWrite = true;
        filePtr = fopen (FullPath, "w");
        if (filePtr)
        {
            IsWriteStarted = false;
            Valid = true;
        }
    }
    else
    {
        IsWrite = false;
        filePtr = fopen (FullPath, "r");
        if (filePtr)
        {
            // Read the file header
            if (fread (&fileHdr, sizeof(fileHdr), 1, filePtr) != 1)
            {
                fclose (filePtr);
            }
            else
            {
                Valid = true;
            }
        }
    }
}

CArchiveFile::~CArchiveFile ()
{
    if (Valid)
    {
        fclose (filePtr);
        Valid = false;
    }
}

ArchiveRecord* CArchiveFile::GetNext (int* day)
{
    if (! Valid)
    {
        return 0;
    }

    for (int i = dayIndex; i < 32; i ++)
    {
        if (fileHdr.dayIndex[i].recordsInDay == 0)
        {
            // we're done with this day
            dayIndex ++;
            recordIndex = 0;
            continue;
        }

        if (recordIndex >= (fileHdr.dayIndex[i].recordsInDay - 2))
        {
            dayIndex ++;
            recordIndex = 0;
            continue;
        }

        //  goto the archive record
        if (fseek (filePtr,
                    sizeof(fileHdr) +
                        DBFILES_RECORD_SIZE * (fileHdr.dayIndex[i].startPos + 2 + recordIndex),
                    SEEK_SET)
            == -1)
        {
            fclose (filePtr);
            Valid = false;
            return 0;
        }

        // read the archive record
        memset (&arcRecord, 0, sizeof(arcRecord));
        if (fread (&arcRecord, sizeof(arcRecord), 1, filePtr) != 1)
        {
            fclose (filePtr);
            Valid = false;
            return 0;
        }

        // we have a winner
        recordIndex ++;
        *day = dayIndex;
        return &arcRecord;
    }

    // If here, we have exhausted the file
    fclose (filePtr);
    Valid = false;
    return 0;
}

bool CArchiveFile::PutNext (ArchiveRecord* record, int day)
{
    float               chill, dew, heat;
    int                 dayRecs;

    if (! Valid)
    {
        return false;
    }

    chill = CalculateWindChill ((float)record->outsideTemp/10, (float)record->windSpeed/10);
    dew = CalculateDewpoint ((float)record->outsideTemp/10, (float)record->outsideHum/10);
    heat = CalculateHeatIndex ((float)record->outsideTemp/10, (float)record->outsideHum/10);

    if (! IsWriteStarted)
    {
        // Create an empty header
        memset (&fileHdr, 0, sizeof (fileHdr));
        memcpy (fileHdr.idCode, idCode, sizeof (idCode));
        IsWriteStarted = true;
    }

    if (fileHdr.dayIndex[day].recordsInDay == 0)
    {
        // Create the day's summary record
        memset (&sumRecord, 0, sizeof (sumRecord));
        sumRecord.dataSpan = record->archiveInterval;
        sumRecord.hiOutTemp = record->hiOutsideTemp;
        insertTimeValue (sumRecord.timeValues1,
                         High_Outside_Temperature,
                         record->packedTime);
        sumRecord.lowOutTemp = record->lowOutsideTemp;
        insertTimeValue (sumRecord.timeValues1,
                         Low_Outside_Temperature,
                         record->packedTime);
        sumRecord.hiInTemp = record->insideTemp;
        insertTimeValue (sumRecord.timeValues1,
                         High_Inside_Temperature,
                         record->packedTime);
        sumRecord.lowInTemp = record->insideTemp;
        insertTimeValue (sumRecord.timeValues1,
                         Low_Inside_Temperature,
                         record->packedTime);
        sumRecord.avgOutTemp = record->outsideTemp;
        sumRecord.avgInTemp = record->insideTemp;
        sumRecord.hiChill = (int)(chill*10);
        insertTimeValue (sumRecord.timeValues1,
                         High_Wind_Chill,
                         record->packedTime);
        sumRecord.lowChill = sumRecord.hiChill;
        insertTimeValue (sumRecord.timeValues1,
                         Low_Wind_Chill,
                         record->packedTime);
        sumRecord.hiDew = (int)(dew*10);
        insertTimeValue (sumRecord.timeValues1,
                         High_Dew_Point,
                         record->packedTime);
        sumRecord.lowDew = sumRecord.hiDew;
        insertTimeValue (sumRecord.timeValues1,
                         Low_Dew_Point,
                         record->packedTime);
        sumRecord.avgChill = sumRecord.hiChill;
        sumRecord.avgDew = sumRecord.hiDew;
        sumRecord.hiOutHum = record->outsideHum;
        insertTimeValue (sumRecord.timeValues1,
                         High_Outside_Humidity,
                         record->packedTime);
        sumRecord.lowOutHum = record->outsideHum;
        insertTimeValue (sumRecord.timeValues1,
                         Low_Outside_Humidity,
                         record->packedTime);
        sumRecord.hiInHum = record->insideHum;
        insertTimeValue (sumRecord.timeValues1,
                         High_Inside_Humidity,
                         record->packedTime);
        sumRecord.lowInHum = record->insideHum;
        insertTimeValue (sumRecord.timeValues1,
                         Low_Inside_Humidity,
                         record->packedTime);
        sumRecord.avgOutHum = sumRecord.hiOutHum;
        sumRecord.hiBar = record->barometer;
        insertTimeValue (sumRecord.timeValues1,
                         High_Barometer,
                         record->packedTime);
        sumRecord.lowBar = record->barometer;
        insertTimeValue (sumRecord.timeValues1,
                         Low_Barometer,
                         record->packedTime);
        sumRecord.avgBar = record->barometer;
        sumRecord.hiSpeed = record->hiWindSpeed;
        insertTimeValue (sumRecord.timeValues1,
                         High_Wind_Speed,
                         record->packedTime);
        sumRecord.avgSpeed = record->windSpeed;
        sumRecord.dailyWindRunTotal = 0x8000;
        sumRecord.hi10MinSpeed = 0x8000;
        sumRecord.dirHiSpeed = record->hiWindDirection;
        sumRecord.hi10MinDir = 0xff;
        sumRecord.dailyRainTotal = (record->rain & 0x0FFF) * 10;
        sumRecord.hiRainRate = record->hiRainRate;
        insertTimeValue (sumRecord.timeValues1,
                         High_Rain_Rate,
                         record->packedTime);
        sumRecord.dailyUVDose = 0x8000;
        sumRecord.hiUV = record->hiUV;
        sumRecord.dataType2 = 3;
        sumRecord.todaysWeather = 0;
        sumRecord.numWindPackets = record->numWindSamples;
        sumRecord.hiSolar = record->hisolarRad;
        sumRecord.dailySolarEnergy = 0x8000;
        sumRecord.minSunlight = 0x8000;
        sumRecord.dailyETTotal = record->ET;
        sumRecord.hiHeat = (int)(heat*10);
        insertTimeValue (sumRecord.timeValues1,
                         High_Outside_Heat_Index,
                         record->packedTime);
        sumRecord.lowHeat = sumRecord.hiHeat;
        insertTimeValue (sumRecord.timeValues1,
                         Low_Outside_Heat_Index,
                         record->packedTime);
        sumRecord.avgHeat = sumRecord.hiHeat;
    }

    if (sumRecord.dataType1 == 0)
    {
        fileHdr.dayIndex[day].startPos = fileHdr.totalRecords;
        fileHdr.totalRecords += 3;
        fileHdr.dayIndex[day].recordsInDay += 3;
        sumRecord.dataSpan = record->archiveInterval;
    }
    else
    {
        // roll in this archive record's data
        fileHdr.totalRecords += 1;
        fileHdr.dayIndex[day].recordsInDay += 1;
        sumRecord.dataSpan += record->archiveInterval;
        dayRecs = fileHdr.dayIndex[day].recordsInDay - 2;

        if (record->hiOutsideTemp > sumRecord.hiOutTemp)
        {
            sumRecord.hiOutTemp = record->hiOutsideTemp;
            insertTimeValue (sumRecord.timeValues1,
                             High_Outside_Temperature,
                             record->packedTime);
        }
        if (record->lowOutsideTemp < sumRecord.lowOutTemp)
        {
            sumRecord.lowOutTemp = record->lowOutsideTemp;
            insertTimeValue (sumRecord.timeValues1,
                             Low_Outside_Temperature,
                             record->packedTime);
        }
        if (record->insideTemp > sumRecord.hiInTemp)
        {
            sumRecord.hiInTemp = record->insideTemp;
            insertTimeValue (sumRecord.timeValues1,
                             High_Inside_Temperature,
                             record->packedTime);
        }
        if (record->insideTemp < sumRecord.lowInTemp)
        {
            sumRecord.lowInTemp = record->insideTemp;
            insertTimeValue (sumRecord.timeValues1,
                             Low_Inside_Temperature,
                             record->packedTime);
        }
        sumRecord.avgOutTemp = (((dayRecs-1)*sumRecord.avgOutTemp)+record->outsideTemp)/dayRecs;
        sumRecord.avgInTemp = (((dayRecs-1)*sumRecord.avgInTemp)+record->insideTemp)/dayRecs;
        if ((int)(chill*10) > sumRecord.hiChill)
        {
            sumRecord.hiChill = (int)(chill*10);
            insertTimeValue (sumRecord.timeValues1,
                             High_Wind_Chill,
                             record->packedTime);
        }
        if ((int)(chill*10) < sumRecord.lowChill)
        {
            sumRecord.lowChill = (int)(chill*10);
            insertTimeValue (sumRecord.timeValues1,
                             Low_Wind_Chill,
                             record->packedTime);
        }
        if ((int)(dew*10) > sumRecord.hiDew)
        {
            sumRecord.hiDew = (int)(dew*10);
            insertTimeValue (sumRecord.timeValues1,
                             High_Dew_Point,
                             record->packedTime);
        }
        if ((int)(dew*10) < sumRecord.lowDew)
        {
            sumRecord.lowDew = (int)(dew*10);
            insertTimeValue (sumRecord.timeValues1,
                             Low_Dew_Point,
                             record->packedTime);
        }
        sumRecord.avgChill = (((dayRecs-1)*sumRecord.avgChill)+(int)(chill*10))/dayRecs;
        sumRecord.avgDew = (((dayRecs-1)*sumRecord.avgDew)+(int)(dew*10))/dayRecs;
        if (record->outsideHum > sumRecord.hiOutHum)
        {
            sumRecord.hiOutHum = record->outsideHum;
            insertTimeValue (sumRecord.timeValues1,
                             High_Outside_Humidity,
                             record->packedTime);
        }
        if (record->outsideHum < sumRecord.lowOutHum)
        {
            sumRecord.lowOutHum = record->outsideHum;
            insertTimeValue (sumRecord.timeValues1,
                             Low_Outside_Humidity,
                             record->packedTime);
        }
        if (record->insideHum > sumRecord.hiInHum)
        {
            sumRecord.hiInHum = record->insideHum;
            insertTimeValue (sumRecord.timeValues1,
                             High_Inside_Humidity,
                             record->packedTime);
        }
        if (record->insideHum < sumRecord.lowInHum)
        {
            sumRecord.lowInHum = record->insideHum;
            insertTimeValue (sumRecord.timeValues1,
                             Low_Inside_Humidity,
                             record->packedTime);
        }
        sumRecord.avgOutHum = (((dayRecs-1)*sumRecord.avgOutHum)+record->outsideHum)/dayRecs;
        if (record->barometer > sumRecord.hiBar)
        {
            sumRecord.hiBar = record->barometer;
            insertTimeValue (sumRecord.timeValues1,
                             High_Barometer,
                             record->packedTime);
        }
        if (record->barometer < sumRecord.lowBar)
        {
            sumRecord.lowBar = record->barometer;
            insertTimeValue (sumRecord.timeValues1,
                             Low_Barometer,
                             record->packedTime);
        }
        sumRecord.avgBar = (((dayRecs-1)*sumRecord.avgBar)+record->barometer)/dayRecs;
        if (record->hiUV > sumRecord.hiUV)
        {
            sumRecord.hiUV = record->hiUV;
            insertTimeValue (sumRecord.timeValues1,
                             High_UV,
                             record->packedTime);
        }
        if (record->hisolarRad > sumRecord.hiSolar)
        {
            sumRecord.hiSolar = record->hisolarRad;
            insertTimeValue (sumRecord.timeValues2,
                             High_Solar_Rad,
                             record->packedTime);
        }
        if (record->hiWindSpeed > sumRecord.hiSpeed)
        {
            sumRecord.hiSpeed = record->hiWindSpeed;
            insertTimeValue (sumRecord.timeValues1,
                             High_Wind_Speed,
                             record->packedTime);
        }
        sumRecord.avgSpeed = (((dayRecs-1)*sumRecord.avgSpeed)+record->windSpeed)/dayRecs;
        sumRecord.dailyRainTotal += (record->rain & 0x0FFF);
        if (record->hiRainRate > sumRecord.hiRainRate)
        {
            sumRecord.hiRainRate = record->hiRainRate;
            insertTimeValue (sumRecord.timeValues1,
                             High_Rain_Rate,
                             record->packedTime);
        }
        if ((int)(heat*10) > sumRecord.hiHeat)
        {
            sumRecord.hiHeat = (int)(heat*10);
            insertTimeValue (sumRecord.timeValues2,
                             High_Outside_Heat_Index,
                             record->packedTime);
        }
        if ((int)(heat*10) < sumRecord.lowHeat)
        {
            sumRecord.lowHeat = (int)(heat*10);
            insertTimeValue (sumRecord.timeValues2,
                             Low_Outside_Heat_Index,
                             record->packedTime);
        }
        sumRecord.avgHeat = (((dayRecs-1)*sumRecord.avgHeat)+(int)(heat*10))/dayRecs;
    }

    // write out the hdr and the new records
    if (fseek (filePtr, 0, SEEK_SET) == -1)
    {
        fclose (filePtr);
        Valid = false;
        return false;
    }

    if (fwrite (&fileHdr, sizeof(fileHdr), 1, filePtr) != 1)
    {
        fclose (filePtr);
        Valid = false;
        return false;
    }

    if (fseek (filePtr,
               sizeof(fileHdr) + (DBFILES_RECORD_SIZE * fileHdr.dayIndex[day].startPos),
               SEEK_SET)
            == -1)
    {
        fclose (filePtr);
        Valid = false;
        return false;
    }

    sumRecord.dataType1 = 2;
    if (fwrite (&sumRecord, sizeof(sumRecord), 1, filePtr) != 1)
    {
        fclose (filePtr);
        Valid = false;
        return false;
    }

    if (fseek (filePtr, 0, SEEK_END) == -1)
    {
        fclose (filePtr);
        Valid = false;
        return false;
    }
    if (fwrite (record, sizeof(*record), 1, filePtr) != 1)
    {
        fclose (filePtr);
        Valid = false;
        return false;
    }

    return true;
}
