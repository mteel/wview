/*---------------------------------------------------------------------
 
 FILE NAME:
        twiConfig.c
 
 PURPOSE:
        TWI autobaud utility.
 
 REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        12/14/2009      M.S. Teel       0               Original

  NOTES:

  LICENSE:
        Copyright (c) 2009, Mark S. Teel (mark@teel.ws)

        This source code is released for free distribution under the terms
        of the GNU General Public License.
 
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
#include <twiInterface.h>


/*  ... local memory
*/
#define TWI_NUMBER_BAUDS                7
static speed_t          BaudRates[TWI_NUMBER_BAUDS] =
{
    B19200,
    B2400,
    B9600,
    B4800,
    B300,
    B600,
    B1200
};

static char             *BaudNames[TWI_NUMBER_BAUDS] =
{
    "B19200",
    "B2400",
    "B9600",
    "B4800",
    "B300",
    "B600",
    "B1200"
};


/*  ... methods
*/

static void serialPortConfig (int fd, speed_t baud)
{
    struct termios  port;

    tcgetattr (fd, &port);

    cfsetispeed (&port, baud);
    cfsetospeed (&port, baud);

    // set parity
    port.c_cflag &= ~PARENB;

    port.c_cflag &= ~CSTOPB;

    port.c_cflag &= ~CSIZE;
    port.c_cflag |= CS8;

    port.c_cflag &= ~CRTSCTS;                   // turn OFF H/W flow control
    port.c_cflag |= (CREAD | CLOCAL);

    port.c_iflag &= ~(IXON | IXOFF | IXANY);    // turn off SW flow control

    port.c_iflag &= ~(INLCR | ICRNL);           // turn off other input magic

    port.c_oflag = 0;                           // NO output magic wanted

    port.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    tcsetattr (fd, TCSAFLUSH, &port);

    return;
}

static void portDTRToggle (int fd, int msec)
{
    struct termios  tty, old;

    tcgetattr (fd, &tty);
    tcgetattr (fd, &old);
    cfsetospeed (&tty, B0);
    cfsetispeed (&tty, B0);
    tcsetattr (fd, TCSANOW, &tty);
    if (msec > 0)
    {
        radUtilsSleep (msec);
    }

    tcsetattr (fd, TCSANOW, &old);
}


// Autobaud the TWI station:
// Returns the baudrate or ERROR:
int twiConfig(WVIEWD_WORK *pWork)
{
    int             baudIndex;

    radMsgLog (PRI_STATUS, "==> Detecting TWI communication settings:");

    // now autobaud the station:
    for (baudIndex = 0; baudIndex < TWI_NUMBER_BAUDS; baudIndex ++)
    {
        radMsgLog (PRI_STATUS, "==> Trying %s-8-N-1...", BaudNames[baudIndex]);
        // setup the port
        serialPortConfig(pWork->medium.fd, BaudRates[baudIndex]);

        tcflush (pWork->medium.fd, TCIFLUSH);
        tcflush (pWork->medium.fd, TCOFLUSH);

        // bump the DTR line so the station will not hang in certain scenarios
        if (pWork->stationToggleDTR)
        {
            portDTRToggle (pWork->medium.fd, 100);
        }

        // try to communicate:
        twiProtocolWriteLineToStation(pWork, "R", FALSE);

        radUtilsSleep (100);

        if (twiProtocolReadLine(pWork) == OK)
        {
            radMsgLog (PRI_STATUS, "==> Yes!");
            radMsgLog (PRI_STATUS, "==> Found TWI at %s-8-N-1", BaudNames[baudIndex]);
            return (int)BaudRates[baudIndex];
        }
        else
        {
            radMsgLog (PRI_STATUS, "==> No");
        }
    }

    radMsgLog (PRI_HIGH, "==> TWI autodetect failed!!");
    return ERROR;
}

