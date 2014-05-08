/*---------------------------------------------------------------------------
 
  FILENAME:
        procmonStates.c
 
  PURPOSE:
        Provide the wview PMON state machine.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        11/26/2007      M.S. Teel       0               Original
 
  NOTES:
        
 
  LICENSE:
        Copyright (c) 2007, Mark S. Teel (mark@teel.ws)
  
        This source code is released for free distribution under the terms 
        of the GNU General Public License.
  
----------------------------------------------------------------------------*/

//  System include files
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

//  Library include files
#include <radsystem.h>

//  Local include files
#include <procmon.h>


//  static (local) memory declarations
static int newProcessEntryPoint (void * pargs)
{
    PMON_PROCESS*       pProcess = (PMON_PROCESS*)pargs;
    char*               newargv[] = { NULL, NULL };
    char*               newenviron[] = { NULL };

    newargv[0] = pProcess->binFile;
    execve (pProcess->binFile, newargv, newenviron);
    perror("execve");       // execve() only returns on error
    return ERROR;
}


// Handlers:
int IdleStateHandler (int state, void* stimulus, void* userData)
{
    PMON_STIM*      stim = (PMON_STIM*)stimulus;

    if (stim->type == PMON_STIM_POLL)
    {
        return PMON_STATE_WAIT_RESP;
    }

    return state;
}

int WaitRespStateHandler (int state, void* stimulus, void* userData)
{
    PMON_STIM*          stim = (PMON_STIM*)stimulus;
    WVIEW_PMON_WORK*    pmonWork = (WVIEW_PMON_WORK*)userData;

    if (stim->type == PMON_STIM_TIMEOUT)
    {
        // Timeout!
        // This guy expired with no response!
        radMsgLog (PRI_HIGH, "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^");
        radMsgLog (PRI_HIGH, "PMON >>>>  %s did not respond to poll!",
                   pmonWork->process[stim->index].binFile);
        radMsgLog (PRI_HIGH, "PMON >>>>  OLDPID: %d", pmonWork->process[stim->index].pid);

        // Kill him:
        // first try to be civil:
        radMsgLog (PRI_HIGH, "PMON >>>>  SIGTERM: %s",
                   pmonWork->process[stim->index].binFile);
        if (kill (pmonWork->process[stim->index].pid, SIGTERM) != 0 && errno != ESRCH)
        {
            radMsgLog (PRI_HIGH, "PMON >>>>  SIGTERM %s failed: %s",
                       pmonWork->process[stim->index].binFile, strerror(errno));
            radMsgLog (PRI_HIGH, "PMON >>>>  Process Monitor shutting down:");
            radMsgLog (PRI_HIGH, "PMON >>>>  Please report this incident to wview Forums");
            radProcessSetExitFlag ();
            return PMON_STATE_IDLE;
        }

        // Allow time for the guy to exit:
        radMsgLog (PRI_HIGH, "PMON >>>>  waiting for %s to exit",
                   pmonWork->process[stim->index].binFile);
        pmonWork->process[stim->index].ticks = PMON_WAIT_EXIT_TIME;
        return PMON_STATE_WAIT_EXIT;
    }
    else if (stim->type == PMON_STIM_RESPONSE)
    {
        return PMON_STATE_IDLE;
    }

    return state;
}

int WaitExitStateHandler (int state, void* stimulus, void* userData)
{
    PMON_STIM*          stim = (PMON_STIM*)stimulus;
    WVIEW_PMON_WORK*    pmonWork = (WVIEW_PMON_WORK*)userData;
    struct stat         fileData;

    if (stim->type == PMON_STIM_TIMEOUT)
    {
        // Check to see if he did:
        if (stat (pmonWork->process[stim->index].pidFile, &fileData) == 0)
        {
            // He did NOT!
            // Get ugly:
            radMsgLog (PRI_HIGH, "PMON >>>>  SIGKILL: %s",
                       pmonWork->process[stim->index].binFile);
            if (kill (pmonWork->process[stim->index].pid, SIGKILL) != 0 && errno != ESRCH)
            {
                radMsgLog (PRI_HIGH, "PMON >>>>  KILL %s failed: %s",
                           pmonWork->process[stim->index].binFile, strerror(errno));
                radMsgLog (PRI_HIGH, "PMON >>>>  Process Monitor shutting down:");
                radMsgLog (PRI_HIGH, "PMON >>>>  Please report this incident to wview Forums");
                radProcessSetExitFlag ();
                return PMON_STATE_IDLE;
            }

            // Delete the pid file:
            radMsgLog (PRI_HIGH, "PMON >>>>  deleting: %s",
                       pmonWork->process[stim->index].pidFile);
            if (stat (pmonWork->process[stim->index].pidFile, &fileData) == 0)
            {
                unlink (pmonWork->process[stim->index].pidFile);
            }

            // Ok, cleanup his msgRouter connection:
            radMsgRouterProcessExit (pmonWork->process[stim->index].pid);
        }

        // Start the process again:
        radMsgLog (PRI_HIGH, "PMON >>>>  starting: %s",
                   pmonWork->process[stim->index].binFile);
        pmonWork->process[stim->index].pid = radStartProcess (newProcessEntryPoint, 
                                                              &pmonWork->process[stim->index]);
        if (pmonWork->process[stim->index].pid == ERROR)
        {
            radMsgLog (PRI_HIGH, "PMON >>>>  START %s failed: %s",
                       pmonWork->process[stim->index].binFile, strerror(errno));
            radMsgLog (PRI_HIGH, "PMON >>>>  Process Monitor shutting down:");
            radMsgLog (PRI_HIGH, "PMON >>>>  Please report this incident to wview Forums");
            radProcessSetExitFlag ();
            return PMON_STATE_IDLE;
        }
        else
        {
            pmonWork->process[stim->index].ticks = PMON_WAIT_START_TIME;
            return PMON_STATE_WAIT_START;
        }
    }

    return state;
}

int WaitStartStateHandler (int state, void* stimulus, void* userData)
{
    PMON_STIM*          stim = (PMON_STIM*)stimulus;
    WVIEW_PMON_WORK*    pmonWork = (WVIEW_PMON_WORK*)userData;

    if (stim->type == PMON_STIM_TIMEOUT)
    {
        pmonWork->process[stim->index].pid = pmonGetProcessPid (pmonWork->process[stim->index].pidFile);
        if (pmonWork->process[stim->index].pid == ERROR)
        {
            radMsgLog (PRI_HIGH, "PMON >>>>  GETPID %s failed: %s",
                   pmonWork->process[stim->index].binFile, strerror(errno));
            radMsgLog (PRI_HIGH, "PMON >>>>  Process Monitor shutting down:");
            radMsgLog (PRI_HIGH, "PMON >>>>  Please report this incident to wview Forums");
            radProcessSetExitFlag ();
        }
        else
        {
            statusIncrementStat(PMON_STATS_RESTARTS);
            radMsgLog (PRI_HIGH, "PMON >>>>  NEWPID: %d", pmonWork->process[stim->index].pid);
            radMsgLog (PRI_HIGH, "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^");
        }

        return PMON_STATE_IDLE;
    }

    return state;
}

