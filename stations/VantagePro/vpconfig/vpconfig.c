/*---------------------------------------------------------------------
 
 FILE NAME:
        vpconfig.c
 
 PURPOSE:
        Main entry point for the Vantage Pro configuration utility.
 
 REVISION HISTORY:
    Date        Programmer  Revision    Function
    04/13/2005  M.S. Teel   0           Original
 
 ASSUMPTIONS:
 None.
 
------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>

#include <datadefs.h>
#include <daemon.h>
#include <station.h>
#include <vproInterface.h>

/*  ... global references
*/

/*  ... local memory
*/
// initial console wakeup number of attempts
#define VPC_INITIAL_WAKEUP_TRIES        10

static WVIEWD_WORK      wvWork;


/*  ... methods
*/

static void dumpConfig (void)
{
    int         numTries = 0;
    int         channel;
    
    // initial wakeup of the VP console
    while ((vpifWakeupConsole (&wvWork) == ERROR) && 
           (numTries < VPC_INITIAL_WAKEUP_TRIES))
    {
        printf ("VP console WAKEUP failed - retry\n");
        radUtilsSleep (1000);
        numTries ++;
    }
    if (numTries == VPC_INITIAL_WAKEUP_TRIES)
    {
        printf ("VP console WAKEUP failed - aborting\n");
        return;
    }

    // get the archive interval
    if (vpconfigGetArchiveInterval (&wvWork) == ERROR)
    {
        printf ("vpconfigGetArchiveInterval failed - aborting\n");
        return;
    }
    
    // get the RXCheck string
    if (vpifGetRXCheck (&wvWork) == ERROR)
    {
        printf ("vpifGetRXCheck failed - aborting\n");
        return;
    }
    
    // get the version string
    if (vpconfigGetFWVersion (&wvWork) == ERROR)
    {
        printf ("vpconfigGetFWVersion failed - aborting\n");
        return;
    }
    
    // get the Lat and Long
    if (vpifGetLatandLong (&wvWork) == ERROR)
    {
        printf ("vpifGetLatandLong failed - aborting\n");
        return;
    }
    
    // get the Rain Season Start
    if (vpconfigGetRainSeasonStart (&wvWork) == ERROR)
    {
        printf ("vpconfigGetRainSeasonStart failed - aborting\n");
        return;
    }
    
    // get the station wind direction calibration
    if (vpconfigGetWindDirectionCal (&wvWork) == ERROR)
    {
        printf ("vpconfigGetWindDirectionCal failed - aborting\n");
        return;
    }
    
    // get the transmitter type and retransmitter settings
    if (vpconfigGetTransmitters (&wvWork) == ERROR)
    {
        printf ("vpconfigGetTransmitters failed - aborting\n");
        return;
    }
    
    // get the rain collector size
    if (vpconfigGetRainCollectorSize (&wvWork) == ERROR)
    {
        printf ("vpconfigRainCollectorSize failed - aborting\n");
        return;
    }
    
    // get the wind cup size
    if (vpconfigGetWindCupSize (&wvWork) == ERROR)
    {
        printf ("vpconfigGetWindCupSize failed - aborting\n");
        return;
    }
    
    printf ("\n");
    printf ("Firmware Version:        %s\n",
            ((VP_IF_DATA *)wvWork.stationData)->fwVersion);

    printf ("Station Location:        ");
    if (wvWork.latitude < 0)
        printf ("%.1f S, ", (float)(-wvWork.latitude)/10.0);
    else
        printf ("%.1f N, ", (float)wvWork.latitude/10.0);
    if (wvWork.longitude < 0)
        printf ("%.1f W, ", (float)(-wvWork.longitude)/10.0);
    else
        printf ("%.1f E, ", (float)wvWork.longitude/10.0);
    printf ("%d feet\n", (int)wvWork.elevation);
        
    printf ("Archive Interval:        %d minutes\n", 
            wvWork.archiveInterval);

    printf ("Rain Season Start Month: %d\n", 
            ((VP_IF_DATA *)wvWork.stationData)->rainSeasonStart);
    
    printf ("Wind direction calibration: %d degrees\n", 
            ((VP_IF_DATA *)wvWork.stationData)->windDirectionCal);
    
    for (channel = 0; channel < 8; channel++)
    {
        if (((VP_IF_DATA *)wvWork.stationData)->listenChannels & ((uint8_t) 1 << channel))
        {
            uint8_t sensorType = ((VP_IF_DATA *)wvWork.stationData)->transmitterType[channel*2] & 0xF;
            uint8_t humidityIndex = (((VP_IF_DATA *)wvWork.stationData)->transmitterType[channel*2 + 1] & (uint8_t) 0xF0) >> 4;
            uint8_t temperatureIndex = ((VP_IF_DATA *)wvWork.stationData)->transmitterType[channel*2 + 1] & 0xF;
            printf ("Listening on channel:    %d for ", channel + 1);
            switch ((VPRO_SENSOR_TYPES) sensorType)
            {
                case VPRO_SENSOR_ISS:
                    printf("ISS\n"); break;
                case VPRO_SENSOR_TEMP:
                    printf("temperature (ID %d)\n", temperatureIndex + 1); break;
                case VPRO_SENSOR_HUM:
                    printf("humidity (ID %d)\n", humidityIndex); break;
                case VPRO_SENSOR_TEMP_HUM:
                    printf("temperature (ID %d) and humidity (ID %d)\n", temperatureIndex + 1, humidityIndex); break;
                case VPRO_SENSOR_WIND:
                    printf("wireless anemometer\n"); break;
                case VPRO_SENSOR_RAIN:
                    printf("rain\n"); break;
                case VPRO_SENSOR_LEAF:
                    printf("leaf wetness\n"); break;
                case VPRO_SENSOR_SOIL:
                    printf("soil moisture\n"); break;
                case VPRO_SENSOR_LEAF_SOIL:
                    printf("leaf wetness and soil moisture\n"); break;
                case VPRO_SENSOR_SENSORLINK:
                    printf("SensorLink\n"); break;
                case VPRO_SENSOR_NONE:
                    // Should not happen; tx bit in listenChannels should be off.
                    printf("no sensor\n"); break;
                default:
                    printf("unknown sensor type 0x%02x\n", sensorType); break;
            }
        }
    }

    printf ("Retransmit channel:      "); 
    if (((VP_IF_DATA *)wvWork.stationData)->retransmitChannel == 0)
    {
        printf ("off\n"); 
    }
    else
    {
        printf("%d\n",
            ((VP_IF_DATA *)wvWork.stationData)->retransmitChannel);
    }
    
    printf ("Rain collector size:     ");
    switch (((VP_IF_DATA *)wvWork.stationData)->rainCollectorSize)
    {
        case VPRO_RAIN_COLLECTOR_0_01_IN:
          printf("0.01 in\n"); break;
        case VPRO_RAIN_COLLECTOR_0_2_MM:
          printf("0.2 mm\n"); break;
        case VPRO_RAIN_COLLECTOR_0_1_MM:
          printf("0.1 mm\n"); break;
        default:
          printf("unknown"); break;
    }
    
    printf ("Wind cup size:           ");
    switch (((VP_IF_DATA *)wvWork.stationData)->windCupSize)
    {
        case 0:
          printf("small\n"); break;
        case 1:
          printf("large\n"); break;
        default:
          printf("unknown"); break;
    }
    
    printf ("RX Check Stats:          %s recvd:missed:resyncs:good-pkts-in-a-row:CRC errors\n\n",
            ((VP_IF_DATA *)wvWork.stationData)->rxCheck);
    
    return;
}

static int setInterval (int value)
{
    if (value != 5 &&
        value != 10 &&
        value != 15 &&
        value != 30)
    {
        printf ("archive interval must be one of: 5, 10, 15, 30\n");
        return ERROR;
    }
    
    if (vpconfigSetInterval (&wvWork, value) == ERROR)
    {
        printf ("failed to set archive interval\n");
        return ERROR;
    }
    
    sleep (5);
    
    vpconfigClearArchiveMemory (&wvWork);
    
    return OK;
}



static void USAGE (void)
{
    printf ("Usage: vpconfig <station_device> <command> [cmnd_args]\n\n");
    printf ("    station_device     serial device the VP console is connected to:\n");
    printf ("                           FreeBSD:   /dev/cuaa0 - /dev/cuaa4\n");
    printf ("                           Linux:     /dev/ttyS0 - /dev/ttyS4\n");
    printf ("                           Linux USB: /dev/ttyUSB0\n");
    printf ("                           Ethernet:  host:port\n\n");
    printf ("    command            command to execute, one of:\n");
    printf ("                           show\n");
    printf ("                             - retrieve and display VP console config\n");
    printf ("                           cleararchive\n");
    printf ("                             - clear the archive memory without changing the interval\n");
    printf ("                           setinterval <interval in minutes>\n");
    printf ("                             - set the archive interval (this clears the archive memory)\n");
    printf ("                           setelevation <elevation in feet>\n");
    printf ("                             - set the station elevation (feet) - use this to adjust the barometer\n");
    printf ("                           setgain <0 for off, 1 for on>\n");
    printf ("                             - sets the gain of the radio receiver\n");
    printf ("                           setlatlong <latitude (negative for S)> <longitude (negative for W)>\n");
    printf ("                             - set the station latitude and longitude in tenths of a degree\n");
    printf ("                           setrainseasonstart <month (1-12)>\n");
    printf ("                             - set the month that the yearly rain total begins\n");
    printf ("                           setyeartodaterain <0.00 - 200.00>\n");
    printf ("                             - set the yearly rain total to date\n");
    printf ("                           setyeartodateET <0.00 - 200.00>\n");
    printf ("                             - set the yearly Evapotranspiration total to date\n");
    printf ("                           setwinddirectioncal <-360 - 360>\n");
    printf ("                             - set the station wind direction calibration\n");
    printf ("                           setsensor <1 - 8> <type>\n");
    printf ("                             - set type of sensor to listen to on the given channel\n");
    printf ("                               type is one of: iss temp hum temp_hum\n");
    printf ("                                               wind rain leaf soil leaf_soil\n");
    printf ("                                               sensorlink none\n");
    printf ("                               (this clears the archive memory)\n");
    printf ("                           setretransmit <0 - 8>\n");
    printf ("                             - set channel that the station uses to retransmit data (0 is off)\n");
    printf ("                               (this clears the archive memory)\n");
    printf ("                           setraincollectorsize <0.01in 0.2mm 0.1mm>\n");
    printf ("                             - set the amount of rain that tips the collector once\n");
    printf ("                           setwindcupsize <0 for small, 1 for large>\n");
    printf ("                             - set the size of the anemometer cup (1 is standard)\n");
    
    return;
}

/*  ... THE entry point
*/
int main (int argc, char *argv[])
{
    int         temp, temp1;
    float       tempf;
    char        *tptr, host[128], port[16];
    ULONG       (*radsysFptr) (uint8_t systemID);
    
    if (argc < 3)
    {
        USAGE ();
        exit (1);
    }
    
    // make sure we pull in radSystem.o when we link:
    radsysFptr = radSystemGetUpTimeSEC;
    
    memset (&wvWork, 0, sizeof (wvWork));
    
    // do we need to parse an ethernet string here?
    if (argv[1][0] != '/')
    {
        // guess so, did they give us host and port?
        tptr = strstr (argv[1], ":");
        if (tptr == NULL)
        {
            printf ("vpconfig: invalid interface %s given - ", argv[1]);
            printf ("not device and not host:port format - aborting!\n");
            exit (1);
        }
        
        strcpy (wvWork.stationInterface, "ethernet");
        strncpy (wvWork.stationHost, argv[1], tptr - argv[1]);
        wvWork.stationPort = atoi (tptr + 1);
    }
    else
    {
        // normal device specification
        strcpy (wvWork.stationInterface, "serial");
        wvstrncpy (wvWork.stationDevice, argv[1], sizeof(wvWork.stationDevice));
    }   


    // initialize the station abstraction
    if (stationInit (&wvWork, NULL) == ERROR)
    {
        printf ("stationInit failed\n");
        exit (1);
    }


    // process command
    if (!strcmp (argv[2], "show"))
    {
        // dump configuration
        dumpConfig ();
    }
    else if (!strcmp (argv[2], "setinterval"))
    {
        // set the archive interval
        if (argc < 4)
        {
            printf ("no archive interval specified\n");
            (*(wvWork.medium.exit)) (&wvWork.medium);
            exit (1);
        }
        else
        {
            if (setInterval (atoi (argv[3])) == ERROR)
            {
                printf ("set failed!\n");
                (*(wvWork.medium.exit)) (&wvWork.medium);
                exit (1);
            }
            else
            {
                printf ("archive memory cleared and interval set to %d minutes\n",
                        atoi (argv[3]));
            }
        }
    }
    else if (!strcmp (argv[2], "cleararchive"))
    {
        // clear archive memory
        vpifWakeupConsole (&wvWork);
        if (vpconfigClearArchiveMemory (&wvWork) == ERROR)
        {
            printf ("failed to clear archive memory\n");
            (*(wvWork.medium.exit)) (&wvWork.medium);
            exit (1);
        }
        else
        {
            printf ("archive memory cleared\n");
        }
    }
    else if (!strcmp (argv[2], "setelevation"))
    {
        // set the station elevation
        if (argc < 4)
        {
            printf ("no elevation specified\n");
            (*(wvWork.medium.exit)) (&wvWork.medium);
            exit (1);
        }
        else
        {
            vpifWakeupConsole (&wvWork);
            if (vpconfigSetElevation (&wvWork, atoi (argv[3])) == ERROR)
            {
                printf ("set elevation failed!\n");
                (*(wvWork.medium.exit)) (&wvWork.medium);
                exit (1);
            }
            else
            {
                printf ("elevation set to %d feet\n", atoi (argv[3]));
            }
        }
    }
    else if (!strcmp (argv[2], "setgain"))
    {
        // set the station receiver gain
        if (argc < 4)
        {
            printf ("no gain flag specified\n");
            (*(wvWork.medium.exit)) (&wvWork.medium);
            exit (1);
        }
        else
        {
            temp = atoi (argv[3]);
            if (temp != 0)
                temp = 1;
            vpifWakeupConsole (&wvWork);
            if (vpconfigSetGain (&wvWork, temp) == ERROR)
            {
                printf ("set gain failed!\n");
                printf ("gain cannot be set on VP 2 (is this a VP 2?)\n");
                (*(wvWork.medium.exit)) (&wvWork.medium);
                exit (1);
            }
            else
            {
                printf ("gain turned %s\n", ((temp) ? "on" : "off"));
            }
        }
    }
    else if (!strcmp (argv[2], "setlatlong"))
    {
        // set the station latitude and longitude
        if (argc < 5)
        {
            printf ("latitude and/or longitude value missing\n");
            (*(wvWork.medium.exit)) (&wvWork.medium);
            exit (1);
        }
        else
        {
            temp = atoi (argv[3]);
            temp1 = atoi (argv[4]);
            if (temp < -900 || temp > 900 || temp1 < -1799 || temp1 > 1800)
            {
                printf ("invalid latitude or longitude given\n");
                (*(wvWork.medium.exit)) (&wvWork.medium);
                exit (1);
            }
            else
            {
                vpifWakeupConsole (&wvWork);
                if (vpconfigSetLatandLong (&wvWork, temp, temp1) == ERROR)
                {
                    printf ("set latlong failed!\n");
                    (*(wvWork.medium.exit)) (&wvWork.medium);
                    exit (1);
                }
                else
                {
                    printf ("latitude set to %.1f, longitude set to %.1f\n", 
                            (float)temp/10.0, (float)temp1/10.0);
                }
            }
        }
    }
    else if (!strcmp (argv[2], "setrainseasonstart"))
    {
        // set the station rain season start month
        if (argc < 4)
        {
            printf ("no start month specified\n");
            (*(wvWork.medium.exit)) (&wvWork.medium);
            exit (1);
        }
        else
        {
            temp = atoi (argv[3]);
            if (temp < 1 || temp > 12)
            {
                printf ("invalid start month given\n");
                (*(wvWork.medium.exit)) (&wvWork.medium);
                exit (1);
            }
            else
            {
                vpifWakeupConsole (&wvWork);
                if (vpconfigSetRainSeasonStart (&wvWork, temp) == ERROR)
                {
                    printf ("set rain season start month failed!\n");
                    (*(wvWork.medium.exit)) (&wvWork.medium);
                    exit (1);
                }
                else
                {
                    printf ("rain season start month set to %d\n", temp);
                }
            }
        }
    }
    else if (!strcmp (argv[2], "setyeartodaterain"))
    {
        // set the station rain year-to-date
        if (argc < 4)
        {
            printf ("no rain amount specified\n");
            (*(wvWork.medium.exit)) (&wvWork.medium);
            exit (1);
        }
        else
        {
            tempf = (float)atof (argv[3]);
            if (tempf < 0.0 || tempf > 200.0)
            {
                printf ("invalid rain amount given\n");
                (*(wvWork.medium.exit)) (&wvWork.medium);
                exit (1);
            }
            else
            {
                vpifWakeupConsole (&wvWork);
                if (vpconfigSetRainYearToDate (&wvWork, tempf) == ERROR)
                {
                    printf ("set rain year-to-date failed!\n");
                    (*(wvWork.medium.exit)) (&wvWork.medium);
                    exit (1);
                }
                else
                {
                    printf ("rain year-to-date set to %.2f\n", tempf);
                }
            }
        }
    }
    else if (!strcmp (argv[2], "setyeartodateET"))
    {
        // set the station ET year-to-date
        if (argc < 4)
        {
            printf ("no ET value specified\n");
            (*(wvWork.medium.exit)) (&wvWork.medium);
            exit (1);
        }
        else
        {
            tempf = (float)atof (argv[3]);
            if (tempf < 0.0 || tempf > 200.0)
            {
                printf ("invalid ET value given\n");
                (*(wvWork.medium.exit)) (&wvWork.medium);
                exit (1);
            }
            else
            {
                vpifWakeupConsole (&wvWork);
                if (vpconfigSetETYearToDate (&wvWork, tempf) == ERROR)
                {
                    printf ("set ET year-to-date failed!\n");
                    (*(wvWork.medium.exit)) (&wvWork.medium);
                    exit (1);
                }
                else
                {
                    printf ("ET year-to-date set to %.2f\n", tempf);
                }
            }
        }
    }
    else if (!strcmp (argv[2], "setwinddirectioncal"))
    {
        // set the station wind direction calibration
        if (argc < 4)
        {
            printf ("no wind calibration value specified\n");
            (*(wvWork.medium.exit)) (&wvWork.medium);
            exit (1);
        }
        else
        {
            temp = atoi (argv[3]);
            if (temp < -360 || temp > 360)
            {
                printf ("invalid wind direction calibration value given\n");
                (*(wvWork.medium.exit)) (&wvWork.medium);
                exit (1);
            }
            else
            {
                vpifWakeupConsole (&wvWork);
                if (vpconfigSetWindDirectionCal (&wvWork, temp) == ERROR)
                {
                    printf ("set wind direction calibration failed!\n");
                    (*(wvWork.medium.exit)) (&wvWork.medium);
                    exit (1);
                }
                else
                {
                    printf ("Wind direction calibration set to %d\n", temp);
                }
            }
        }
    }
    else if (!strcmp (argv[2], "setsensor"))
    {
        // Configure a channel to listen for a sensor.
        if (argc < 5)
        {
            printf ("not enough arguments; need channel number and station type\n");
            printf ("type is one of: iss temp hum temp_hum wind rain leaf soil leaf_soil sensorlink none\n");
            (*(wvWork.medium.exit)) (&wvWork.medium);
            exit (1);
        }
        else
        {
            temp = atoi (argv[3]);
            if (temp < 1 || temp > 8)
            {
                printf ("invalid channel given (1 to 8)\n");
                (*(wvWork.medium.exit)) (&wvWork.medium);
                exit (1);
            }
            else
            {
                if (!strcmp (argv[4], "iss"))
                {
                    temp1 = VPRO_SENSOR_ISS;
                }
                else if (!strcmp (argv[4], "temp"))
                {
                    temp1 = VPRO_SENSOR_TEMP;
                }
                else if (!strcmp (argv[4], "hum"))
                {
                    temp1 = VPRO_SENSOR_HUM;
                }
                else if (!strcmp (argv[4], "temp_hum"))
                {
                    temp1 = VPRO_SENSOR_TEMP_HUM;
                }
                else if (!strcmp (argv[4], "wind"))
                {
                    temp1 = VPRO_SENSOR_WIND;
                }
                else if (!strcmp (argv[4], "rain"))
                {
                    temp1 = VPRO_SENSOR_RAIN;
                }
                else if (!strcmp (argv[4], "leaf"))
                {
                    temp1 = VPRO_SENSOR_LEAF;
                }
                else if (!strcmp (argv[4], "soil"))
                {
                    temp1 = VPRO_SENSOR_SOIL;
                }
                else if (!strcmp (argv[4], "leaf_soil"))
                {
                    temp1 = VPRO_SENSOR_LEAF_SOIL;
                }
                else if (!strcmp (argv[4], "sensorlink"))
                {
                    temp1 = VPRO_SENSOR_SENSORLINK;
                }
                else if (!strcmp (argv[4], "none"))
                {
                    temp1 = VPRO_SENSOR_NONE;
                }
                else
                {
                    printf ("Unrecognized sensor type %s\n", argv[4]);
                    printf ("type is one of: iss temp hum temp_hum wind rain leaf soil leaf_soil sensorlink none\n");
                    (*(wvWork.medium.exit)) (&wvWork.medium);
                    exit (1);
                }
                vpifWakeupConsole (&wvWork);
                if (vpconfigSetSensor (&wvWork, temp, (VPRO_SENSOR_TYPES) temp1) == ERROR)
                {
                    printf ("set sensor type failed!\n");
                    (*(wvWork.medium.exit)) (&wvWork.medium);
                    exit (1);
                }
                else
                {
                    printf ("Sensor on channel %d set to type %s\n", temp, argv[4]);
                }
            }
        }
    }
    else if (!strcmp (argv[2], "setretransmit"))
    {
        // Configure the channel to retransmit on
        if (argc < 4)
        {
            printf ("no channel specified (0 turns off retransmission)\n");
            (*(wvWork.medium.exit)) (&wvWork.medium);
            exit (1);
        }
        else
        {
            temp = atoi (argv[3]);
            if (temp < 0 || temp > 8)
            {
                printf ("invalid channel given (1 to 8; 0 turns off retransmission)\n");
                (*(wvWork.medium.exit)) (&wvWork.medium);
                exit (1);
            }
            else
            {
                vpifWakeupConsole (&wvWork);
                if (vpconfigSetRetransmitChannel (&wvWork, temp) == ERROR)
                {
                    printf ("set retransmission channel failed!\n");
                    (*(wvWork.medium.exit)) (&wvWork.medium);
                    exit (1);
                }
                else
                {
                    if (temp == 0)
                    {
                        printf ("Retransmission disabled\n");
                    }
                    else
                    {
                        printf ("Retransmitting on channel %d\n", temp);
                    }
                }
            }
        }
    }
    else if (!strcmp (argv[2], "setraincollectorsize"))
    {
        // Configure the amount of rain that tips the rain collector
        if (argc < 4)
        {
            printf ("not enough arguments; need rain collector size\n");
            printf ("size is one of: 0.01in 0.2mm 0.1mm\n");
            (*(wvWork.medium.exit)) (&wvWork.medium);
            exit (1);
        }
        else
        {
            if (!strcmp (argv[3], "0.01in"))
            {
                temp = VPRO_RAIN_COLLECTOR_0_01_IN;
            }
            else if (!strcmp (argv[3], "0.2mm"))
            {
                temp = VPRO_RAIN_COLLECTOR_0_2_MM;
            }
            else if (!strcmp (argv[3], "0.1mm"))
            {
                temp = VPRO_RAIN_COLLECTOR_0_1_MM;
            }
            else
            {
                printf ("Unrecognized rain collector type %s\n", argv[3]);
                printf ("size is one of: 0.01in 0.2mm 0.1mm\n");
                (*(wvWork.medium.exit)) (&wvWork.medium);
                exit (1);
            }
            vpifWakeupConsole (&wvWork);
            if (vpconfigSetRainCollectorSize (&wvWork, (VPRO_RAIN_COLLECTOR_SIZE) temp) == ERROR)
            {
                printf ("set rain collector size failed!\n");
                (*(wvWork.medium.exit)) (&wvWork.medium);
                exit (1);
            }
            else
            {
                printf ("Rain collector set to %s\n", argv[3]);
            }
        }
    }
    else if (!strcmp (argv[2], "setwindcupsize"))
    {
        // set the wind cup size
        if (argc < 4)
        {
            printf ("no wind cup size specified (0 = small, 1 = large)\n");
            (*(wvWork.medium.exit)) (&wvWork.medium);
            exit (1);
        }
        else
        {
            temp = atoi (argv[3]);
            if (temp != 0)
                temp = 1;
            vpifWakeupConsole (&wvWork);
            if (vpconfigSetWindCupSize (&wvWork, temp) == ERROR)
            {
                printf ("set wind cup size failed!\n");
                (*(wvWork.medium.exit)) (&wvWork.medium);
                exit (1);
            }
            else
            {
                printf ("wind cup size set to %s\n", ((temp) ? "large" : "small"));
            }
        }
    }
    else
    {
        printf ("unknown command %s given!\n", argv[2]);
        (*(wvWork.medium.exit)) (&wvWork.medium);
        exit (1);
    }

    (*(wvWork.medium.exit)) (&wvWork.medium);
    exit (0);
}

// Retrieve exit status:
int wviewdIsExiting(void)
{
    return wvWork.exiting;
}

