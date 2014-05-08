/*---------------------------------------------------------------------
 
 FILE NAME:
        wxt510config.c
 
 PURPOSE:
        Main entry point for the WXT-510 configuration utility.
 
 REVISION HISTORY:
    Date        Programmer  Revision    Function
    01/25/2006  M.S. Teel   0           Original
 
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
#include <wxt510Interface.h>

/*  ... global references
*/

/*  ... local memory
*/
// initial console wakeup number of attempts
#define WXT_INITIAL_WAKEUP_TRIES        10

static WVIEWD_WORK      wvWork;

#define WXT_NUMBER_BAUDS                8
static speed_t          BaudRates[WXT_NUMBER_BAUDS] =
{
    B19200,
    B9600,
    B4800,
    B2400,
    B1200,
    B38400,
    B57600,
    B115200
};

static char            *BaudNames[WXT_NUMBER_BAUDS] =
{
    "B19200",
    "B9600",
    "B4800",
    "B2400",
    "B1200",
    "B38400",
    "B57600",
    "B115200"
};

enum
{
    PARITY_NONE         = 0,
    PARITY_ODD,
    PARITY_EVEN,
    NUM_PARITIES
};

/*  ... methods
*/

static void serialPortConfig (int fd, speed_t baud, int parity, int stopBits, int size)
{
    struct termios  port;

    tcgetattr (fd, &port);

    cfsetispeed (&port, baud);
    cfsetospeed (&port, baud);

    // set parity
    if (parity == PARITY_NONE)
    {
        port.c_cflag &= ~PARENB;
    }
    else if (parity == PARITY_ODD)
    {
        port.c_cflag |= PARENB;
        port.c_cflag |= PARODD;
    }
    else
    {
        port.c_cflag |= PARENB;
        port.c_cflag &= ~PARODD;
    }

    if (stopBits == 1)
    {
        port.c_cflag &= ~CSTOPB;
    }
    else
    {
        port.c_cflag |= CSTOPB;
    }

    port.c_cflag &= ~CSIZE;
    if (size == 8)
    {
        port.c_cflag |= CS8;
    }
    else
    {
        port.c_cflag |= CS7;
    }

    port.c_cflag &= ~CRTSCTS;                   // turn OFF H/W flow control
    port.c_cflag |= (CREAD | CLOCAL);

    port.c_iflag &= ~(IXON | IXOFF | IXANY);    // turn off SW flow control

    port.c_iflag &= ~(INLCR | ICRNL);           // turn off other input magic

    port.c_oflag = 0;                           // NO output magic wanted

    port.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    tcsetattr (fd, TCSAFLUSH, &port);

    return;
}

static void portDTRToggle (int fd, int sec)
{
    struct termios  tty, old;

    tcgetattr (fd, &tty);
    tcgetattr (fd, &old);
    cfsetospeed (&tty, B0);
    cfsetispeed (&tty, B0);
    tcsetattr (fd, TCSANOW, &tty);
    if (sec > 0)
    {
        sleep (sec);
    }

    tcsetattr (fd, TCSANOW, &old);
}

static void USAGE (void)
{
    printf ("Usage: wxt510config <station_device> [enable_DTR]\n\n");
    printf ("    station_device     (mandatory) serial device the WXT-510 is connected to\n");
    printf ("                           FreeBSD:   /dev/cuaa0 - /dev/cuaa4\n");
    printf ("                           Linux:     /dev/ttyS0 - /dev/ttyS4\n");
    printf ("                           Linux USB: /dev/ttyUSB0\n");
    printf ("                           Ethernet:  host:port\n");
    printf ("    enable_DTR         (optional)  enable DTR toggle during autobaud (0 or 1)\n");
    printf ("                           default = 1 (enable DTR toggle)\n\n");
    return;
}

/*  ... THE entry point
*/
int main (int argc, char *argv[])
{
    int         baudIndex, parityIndex, stopIndex, sizeIndex, dtrEnable = TRUE;
    char        *tptr, host[128], port[16], buffer[WVIEW_MAX_PATH];
    ULONG       (*radsysFptr) (uint8_t systemID);
    
    if (argc < 2)
    {
        USAGE ();
        exit (1);
    }

    // make sure we pull in radSystem.o when we link
    radsysFptr = radSystemGetUpTimeSEC;

    memset (&wvWork, 0, sizeof (wvWork));

    // do we need to parse an ethernet string here?
    if (argv[1][0] != '/')
    {
        // guess so, did they give us host and port?
        tptr = strstr (argv[1], ":");
        if (tptr == NULL)
        {
            printf ("wxt510config: invalid interface %s given - ", argv[1]);
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

    if (argc > 2)
    {
        // Grab the DTR toggle setting:
        if (!strcmp(argv[2], "0"))
        {
            dtrEnable = FALSE;
        }
    }

    // initialize the station abstraction
    if (stationInit (&wvWork, NULL) == ERROR)
    {
        printf ("stationInit failed\n");
        exit (1);
    }

    printf ("Detecting current WXT-510 communication settings...\n");

    // now autobaud the station
    for (baudIndex = 0; baudIndex < WXT_NUMBER_BAUDS; baudIndex ++)
    {
        for (parityIndex = PARITY_NONE; parityIndex < NUM_PARITIES; parityIndex ++)
        {
            for (stopIndex = 1; stopIndex <= 2; stopIndex ++)
            {
                for (sizeIndex = 8; sizeIndex >= 7; sizeIndex --)
                {
                    printf ("Trying %s-%d-%c-%d...",
                            BaudNames[baudIndex], 
                            sizeIndex,
                            ((parityIndex == 0) ? 'N' : ((parityIndex == 1) ? 'O' : 'E')),
                            stopIndex);

                    // setup the port
                    serialPortConfig (wvWork.medium.fd, 
                                      BaudRates[baudIndex], 
                                      parityIndex, 
                                      stopIndex,
                                      sizeIndex);

                    tcflush (wvWork.medium.fd, TCIFLUSH);
                    tcflush (wvWork.medium.fd, TCOFLUSH);

                    if (dtrEnable)
                    {
                        // bump the DTR line so the station will not hang in certain scenarios
                        portDTRToggle (wvWork.medium.fd, 1);
                    }

                    // try to communicate - send a CR/LF then ? command
                    buffer[0] = 0;
                    nmea0183WriteLineToStation (&wvWork, buffer, NULL, FALSE, TRUE);
                    radUtilsSleep (50);
                    sprintf (buffer, "0XU,A=0");
                    nmea0183WriteLineToStation (&wvWork, buffer, NULL, FALSE, TRUE);
                    sprintf (buffer, "?");
                    if (nmea0183WriteLineToStation (&wvWork, buffer, "0", FALSE, FALSE) == OK)
                    {
                        printf ("Yes!\n");
                        printf ("Found WXT-510 at %s-%d-%c-%d\n", 
                                BaudNames[baudIndex], 
                                sizeIndex,
                                ((parityIndex == 0) ? 'N' : ((parityIndex == 1) ? 'O' : 'E')),
                                stopIndex);
                        printf ("Setting to B19200-8-N-1...\n");

                        // set 19200 8-N-1
                        sprintf (buffer, "0XU,B=19200,D=8,P=N,S=1");
                        if (nmea0183WriteLineToStation (&wvWork, buffer, buffer, FALSE, FALSE) == ERROR)
                        {
                            printf ("Writing %s failed!\n", buffer);
                            return ERROR;
                        }

                        // send a reset command
                        sprintf (buffer, "0XZ");
                        if (nmea0183WriteLineToStation (&wvWork, buffer, NULL, FALSE, TRUE) == ERROR)
                        {
                            printf ("Writing %s failed!\n", buffer);
                            return ERROR;
                        }
                        radUtilsSleep (100);

                        // now verify we got it done
                        serialPortConfig (wvWork.medium.fd, B19200, PARITY_NONE, 1, 8);

                        // send a <CR><LF>
                        buffer[0] = 0;
                        if (nmea0183WriteLineToStation (&wvWork, buffer, NULL, FALSE, TRUE) == ERROR)
                        {
                            printf ("CR/LF failed!\n");
                            return ERROR;
                        }
                        radUtilsSleep (50);

                        sprintf (buffer, "?");
                        if (nmea0183WriteLineToStation (&wvWork, buffer, "0", FALSE, FALSE) == OK)
                        {
                            printf ("WXT-510 set to B19200-8-N-1 - config complete!\n");
                            (*(wvWork.medium.exit)) (&wvWork.medium);
                            exit (0);
                        }
                        else
                        {
                            printf ("WXT-510 set to B19200-8-N-1 failed!!\n");
                            (*(wvWork.medium.exit)) (&wvWork.medium);
                            exit (1);
                        }
                    }
                    else
                    {
                        printf ("No\n");
                    }
                }
            }
        }
    }

    printf ("WXT-510 autodetect failed!!\n");
    (*(wvWork.medium.exit)) (&wvWork.medium);
    exit (2);
}

// Retrieve exit status:
int wviewdIsExiting(void)
{
    return wvWork.exiting;
}

