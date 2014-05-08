/*---------------------------------------------------------------------------
 
  FILENAME:
        status.c
 
  PURPOSE:
        Provide the status reporting API methods.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        12/27/2009      M.S. Teel       0               Original
 
  NOTES:
        
 
  LICENSE:
        Copyright (c) 2009, Mark S. Teel (mark@teel.ws)
  
        This source code is released for free distribution under the terms 
        of the GNU General Public License.
  
----------------------------------------------------------------------------*/

//  ... System header files
#include <radmsgLog.h>


//  ... Local header files
#include <status.h>

//  ... Local memory:

static STATUS_INFO          ProcessStatus;


static int WriteStatusFile(void)
{
    FILE*       statfile;
    int         index;

    statfile = fopen (ProcessStatus.filePath, "w");
    if (statfile == NULL)
    {
        radMsgLog (PRI_HIGH, "status file create failed!");
        return ERROR;
    }

    fprintf (statfile, "status = %d\n", ProcessStatus.status);
    if (strlen(ProcessStatus.lastMessage) > 0)
    {
        fprintf (statfile, "message = \"%s\"\n", ProcessStatus.lastMessage);
    }

    for (index = 0; index < STATUS_STATS_MAX; index ++)
    {
        if (ProcessStatus.stat[index] == -1 || 
            ProcessStatus.statLabel[index][0] == 0)
        {
            continue;
        }

        fprintf (statfile, "desc%d = \"%s\"\n", index, ProcessStatus.statLabel[index]);
        fprintf (statfile, "stat%d = %d\n", index, ProcessStatus.stat[index]);
    }

    fclose (statfile);
    return OK;
}

//  ... API methods:

//  ... initialize the status log:
int statusInit(const char* filePath, char* statLabel[4])
{
    int             index;
    char            temp[256];
    struct stat     fileData;

    // create our run directory if it is not there:
    sprintf (temp, "%s", "/var/run");
    if (stat (temp, &fileData) != 0)
    {
        if (mkdir (temp, 0755) != 0)
        {
            radMsgLog (PRI_CATASTROPHIC,
                       "Cannot create base run directory: %s - aborting!",
                       temp);
            return ERROR;
        }
    }
    sprintf (temp, "%s", WVIEW_STATUS_DIRECTORY);
    if (stat (temp, &fileData) != 0)
    {
        if (mkdir (temp, 0755) != 0)
        {
            radMsgLog (PRI_CATASTROPHIC,
                       "Cannot create status directory: %s - aborting!",
                       temp);
            return ERROR;
        }
    }

    memset(&ProcessStatus, 0, sizeof(ProcessStatus));
    wvstrncpy(ProcessStatus.filePath, filePath, _MAX_PATH);

    for (index = 0; index < STATUS_STATS_MAX; index ++)
    {
        if (statLabel == NULL ||
            statLabel[index] == NULL || statLabel[index][0] == 0)
        {
            // skip this one:
            ProcessStatus.stat[index] = -1;
            ProcessStatus.statLabel[index][0] = 0;
        }

        ProcessStatus.stat[index] = 0;
        wvstrncpy(ProcessStatus.statLabel[index], statLabel[index], 64);
    }

    return OK;
}

//  ... send a status update:
int statusUpdate(STATUS_TYPE status)
{
    ProcessStatus.status = status;
    WriteStatusFile();
    return OK;
}

//  ... send a status update:
int statusUpdateMessage(const char* message)
{
    wvstrncpy(ProcessStatus.lastMessage, message, _MAX_PATH);
    WriteStatusFile();
    return OK;
}

//  ... send a status update:
int statusUpdateStat(int index, int value)
{
    if (0 > index || index >= STATUS_STATS_MAX ||
        ProcessStatus.stat[index] == -1)
    {
        return ERROR;
    }

    ProcessStatus.stat[index] = value;
    WriteStatusFile();
    return OK;
}

//  ... send a status update:
int statusIncrementStat(int index)
{
    if (0 > index || index >= STATUS_STATS_MAX ||
        ProcessStatus.stat[index] == -1)
    {
        return ERROR;
    }

    ProcessStatus.stat[index] ++;
    WriteStatusFile();
    return OK;
}

//  ... send a status update:
// Do not allow the value to be negative:
int statusDecrementStat(int index)
{
    if (0 > index || index >= STATUS_STATS_MAX ||
        ProcessStatus.stat[index] <= 0)
    {
        return ERROR;
    }

    ProcessStatus.stat[index] --;
    WriteStatusFile();
    return OK;
}

