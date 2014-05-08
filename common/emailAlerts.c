/*---------------------------------------------------------------------------
 
  FILENAME:
        emailAlerts.c
 
  PURPOSE:
        Provide the alert email API methods.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        03/07/2009       M.S. Teel       0               Original
 
  NOTES:
        
 
  LICENSE:
        Copyright (c) 2009, Mark S. Teel (mark@teel.ws)
  
        This source code is released for free distribution under the terms 
        of the GNU General Public License.
  
----------------------------------------------------------------------------*/

//  ... System header files
#include <radmsgLog.h>


//  ... Local header files
#include <emailAlerts.h>

//  ... Local memory:

static ALERT_INFO       alertInfo[ALERT_TYPE_MAX] =
{
    { "wview Test Email", "If you received this wview email alerts are working." },
    { "wview File System IO Alert: r/w error", "Failed to read/write file(s) - system disk full or corrupt!" },
    { "wview Station Interface Alert: Wakeup", "Vantage Pro wakeup failed - check cabling, device or interface!" },
    { "wview Station Interface Alert: Loop Error", "Loop data retrieval failed - current conditions may not be updating!" },
    { "wview Station Interface Alert: Archive Error", "Archive data retrieval failed - archive table and graphs may not be updating!" },
    { "wview Station Interface Alert: Interface Failure", "Station device interface failed - check driver, device or interface!" },
    { "wview Station Interface Alert: Corrupt Data", "Bogus or corrupt station data received - check station or cabling" },
    { "wview Station Interface Alert: Flatline Values", "Station data not changing - check station or interface" }
};

static time_t           lastAlertTime[ALERT_TYPE_MAX] =
{
    0,
    0,
    0
};


//  ... define methods here

int emailAlertSend (EmailAlertTypes type)
{
    WVIEW_MSG_ALERT     alert;

    if ((time(NULL) - lastAlertTime[type]) < ALERT_NOTIFY_INTERVAL)
    {
        // Abate:
        return OK;
    }

    alert.alertType = type;
    lastAlertTime[type] = time(NULL);
    radMsgRouterMessageSend (WVIEW_MSG_TYPE_ALERT, &alert, sizeof(alert));
    return OK;
}

char* emailAlertGetSubject (EmailAlertTypes type)
{
    if (type < ALERT_TYPE_MAX)
        return alertInfo[type].subject;
    else
        return NULL;
}

char* emailAlertGetBody (EmailAlertTypes type)
{
    if (type < ALERT_TYPE_MAX)
        return alertInfo[type].body;
    else
        return NULL;
}


