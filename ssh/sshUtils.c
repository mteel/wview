/*---------------------------------------------------------------------------
 
  FILENAME:
        sshUtils.c
 
  PURPOSE:
        Provide the ssh utilities.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        05/28/2005      M.S. Teel       0               Original
 
  NOTES:
        
 
  LICENSE:
        Copyright (c) 2004, Mark S. Teel (mark@teel.ws)
  
        This source code is released for free distribution under the terms 
        of the GNU General Public License.
  
----------------------------------------------------------------------------*/

/*  ... System include files
*/
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/*  ... Library include files
*/
#include <radsysutils.h>

/*  ... Local include files
*/
#include <sshUtils.h>


/*  ... global memory declarations
*/

/*  ... global memory referenced
*/
extern void SSHDefaultSigHandler (int signum);

/*  ... static (local) memory declarations
*/
static char     *rsyncPaths[] =
{
    "/usr/bin/rsync",
    "/usr/local/bin/rsync",
    "/usr/sbin/rsync",
    "/usr/local/sbin/rsync",
    NULL
};


int sshUtilsInit (SSH_DATA* data)
{
    SSH_RULE_ID     rule;
    SSH_ID          newId;
    int             i, iValue;
    const char*     sValue;
    char            conftype[64], temp[_MAX_PATH];
    struct stat     fileData;

    // first, let's find rsync - if it isn't there, error out
    for (i = 0; rsyncPaths[i] != NULL; i ++)
    {
        if (stat (rsyncPaths[i], &fileData) == 0)
        {
            // found him!
            wvstrncpy (temp, rsyncPaths[i], _MAX_PATH);
            break;
        }
    }

    if (rsyncPaths[i] == NULL)
    {
        // we no findey, rsync must not be installed!
        radMsgLog (PRI_HIGH, 
                   "sshUtilsInit: cannot locate rsync - is it installed?");
        return ERROR;
    }
    
    // OK, we can proceed
    newId = (SSH_ID)data;

    memset (newId, 0, sizeof (*newId));
    radListReset (&newId->rules);
    wvstrncpy (newId->rsyncPath, temp, sizeof(newId->rsyncPath));

    if (wvconfigInit(FALSE) == ERROR)
    {
        radMsgLog (PRI_CATASTROPHIC, "wvconfigInit failed!\n");
        return ERROR;
    }

    // Is the ssh daemon enabled?
    iValue = wvconfigGetBooleanValue(configItem_ENABLE_SSH);
    if (iValue == ERROR || iValue == 0)
    {
        wvconfigExit ();
        radMsgLog (PRI_STATUS, "ssh daemon disabled - exiting...");
        return ERROR_ABORT;
    }

    // get the wview verbosity setting
    if (wvutilsSetVerbosity (WV_VERBOSE_WVIEWSSHD) == ERROR)
    {
        wvconfigExit ();
        radMsgLog (PRI_CATASTROPHIC, "wvutilsSetVerbosity failed!");
        return ERROR_ABORT;
    }
    

    for (i = 1; i <= SSH_MAX_RULES; i ++)
    {
        // allocate a rule:
        rule = (SSH_RULE_ID) malloc (sizeof (*rule));
        if (rule == NULL)
        {
            for (rule = (SSH_RULE_ID)radListRemoveFirst (&newId->rules);
                 rule != NULL;
                 rule = (SSH_RULE_ID)radListRemoveFirst (&newId->rules))
            {
                free (rule);
            }

            return ERROR;
        }
        memset(rule, 0, sizeof(*rule));

        sprintf (conftype, "SSH_%1.1d_INTERVAL", i);
        rule->interval = wvconfigGetINTValue(conftype);
        if (rule->interval <= 0)
        {
            // Incomplete - continue:
            free (rule);
            continue;
        }
        
        sprintf (conftype, "SSH_%1.1d_SOURCE", i);
        sValue = wvconfigGetStringValue(conftype);
        if (sValue == NULL || strlen(sValue) == 0)
        {
            // Incomplete - continue:
            free (rule);
            continue;
        }
        wvstrncpy (rule->src, sValue, sizeof(rule->src));

        sprintf (conftype, "SSH_%1.1d_HOST", i);
        sValue = wvconfigGetStringValue(conftype);
        if (sValue == NULL || strlen(sValue) == 0)
        {
            // Incomplete - continue:
            free (rule);
            continue;
        }
        wvstrncpy (rule->host, sValue, sizeof(rule->host));

        // Get the ssh port number:
        sprintf (conftype, "SSH_%1.1d_PORT", i);
        iValue = wvconfigGetINTValue(conftype);
        if (iValue == ERROR)
        {
            rule->sshPort = SSH_PORT_DEFAULT;
            radMsgLog (PRI_STATUS, "SSH: RULE %d: using default ssh port 22", i);
        }
        else
        {
            rule->sshPort = iValue;
            radMsgLog (PRI_STATUS, "SSH: RULE %d: using ssh port %d", i, iValue);
        }

        // Get the SSH username:
        sprintf (conftype, "SSH_%1.1d_USERNAME", i);
        sValue = wvconfigGetStringValue(conftype);
        if (sValue == NULL)
        {
            // No host defined, set to empty:
            radMsgLog (PRI_STATUS, "SSH: RULE %d: using empty SSH login", i);
            rule->sshUser[0] = 0;
        }
        else
        {
            wvstrncpy (rule->sshUser, sValue, sizeof(rule->sshUser));
            radMsgLog (PRI_STATUS, "SSH: RULE %d: using SSH login %s", i, sValue);
        }

        // Get the SSH timeout:
        sprintf (conftype, "SSH_%1.1d_TIMEOUT", i);
        iValue = wvconfigGetINTValue(conftype);
        if (iValue == ERROR)
        {
            // No timeout defined, set to default:
            radMsgLog (PRI_STATUS, "SSH: RULE %d: using default timeout of 120", i);
            rule->timeout = 120;
        }
        else
        {
            rule->timeout = iValue;
            radMsgLog (PRI_STATUS, "SSH: RULE %d: using SSH timeout %d", i, iValue);
        }

        sprintf (conftype, "SSH_%1.1d_DESTINATION", i);
        sValue = wvconfigGetStringValue(conftype);
        if (sValue == NULL || strlen(sValue) == 0)
        {
            // Incomplete - continue:
            free (rule);
            continue;
        }
        wvstrncpy (rule->dest, sValue, sizeof(rule->dest));

        // Cause a transfer at the first opportunity:
        rule->currentCount = 0;

        radMsgLog (PRI_STATUS, "SSH: RULE %d: updating %s ==> %s:%s every %d minutes",
                   i, rule->src, rule->host, rule->dest, rule->interval);

        radListAddToEnd (&newId->rules, (NODE_PTR)rule);
    }

    wvconfigExit();
    return OK;
}

int sshUtilsSendFiles (SSH_ID id, char *workdir)
{
    SSH_RULE_ID     rule;
    int             done, numRules = 0;
    char            rsync[_MAX_PATH];
    char            cmndLine[2048];
    int             index, cmndLength;
    FILE*           fp;

    // loop through all of our rules here
    for (rule = (SSH_RULE_ID)radListGetFirst (&id->rules);
         rule != NULL;
         rule = (SSH_RULE_ID)radListGetNext (&id->rules, (NODE_PTR)rule))
    {
        // our time to run?
        if (rule->currentCount > 0)
        {
            rule->currentCount --;
            continue;
        }
        
        // Build the command:
        cmndLength = 0;
        cmndLength += sprintf(&cmndLine[cmndLength], "%s ", id->rsyncPath);
        cmndLength += sprintf(&cmndLine[cmndLength], "-azL --timeout=%d ", rule->timeout);
        if (strlen(rule->sshUser) > 0)
        {
            cmndLength += sprintf(&cmndLine[cmndLength], "--rsh=\'ssh -p %d -l %s\' ",
                                  rule->sshPort, rule->sshUser);
        }
        else
        {
            cmndLength += sprintf(&cmndLine[cmndLength], "--rsh=\'ssh -p %d\' ",
                                  rule->sshPort);
        }
        cmndLength += sprintf(&cmndLine[cmndLength], "%s/ ", rule->src);
        cmndLength += sprintf(&cmndLine[cmndLength], "%s:%s", rule->host, rule->dest);

        // Log the monster before executing it:
        for (index = 0; index < cmndLength; index += RADMSGLOG_MAX_LENGTH)
        {
            strncpy(rsync, &cmndLine[index], RADMSGLOG_MAX_LENGTH);
            if (strlen(rsync) > 0)
            {
                wvutilsLogEvent(PRI_STATUS, "SSH-COMMAND: %s", rsync);
            }
        }

        if (chdir(workdir) == -1)
        {
            radMsgLog (PRI_HIGH, "SSH: chdir failed: %s", strerror(errno));
            return ERROR;
        }
    
        // Release the SIGCHLD signal so popen can do its thing:
        radProcessSignalRelease(SIGCHLD);

        // Use popen so we can retrieve the rsync command output:
        fp = popen(cmndLine, "r");
        if (fp == NULL)
        {
            radMsgLog (PRI_HIGH, "SSH: popen failed: %s", strerror(errno));
            radProcessSignalCatch (SIGCHLD, SSHDefaultSigHandler);
            return ERROR;
        }

        done = FALSE;
        while (! done)
        {
            if (fgets(rsync, _MAX_PATH, fp) == NULL)
            {
                if (ferror(fp) && errno != EINTR)
                {
                    radMsgLog (PRI_HIGH, "SSH: IO error for popen stream: %s",
                               strerror(errno));
                }

                done = TRUE;
            }
            else
            {
                // Log if verbosity is ON:
                wvutilsLogEvent(PRI_STATUS, "SSH-OUTPUT: %s", rsync);
            }
        }

        if (pclose(fp) == -1)
        {
            radMsgLog (PRI_HIGH, "SSH: pclose failed: %s", strerror(errno));
            radProcessSignalCatch (SIGCHLD, SSHDefaultSigHandler);
            return ERROR;
        }

        // Restore the SIGCHLD signal:
        radProcessSignalCatch (SIGCHLD, SSHDefaultSigHandler);

        rule->currentCount = rule->interval - 1;
        wvutilsLogEvent (PRI_STATUS, "SSH-RULE: %s ==> %s:%s", 
                         rule->src, rule->host, rule->dest);

        numRules ++;
    }
    
    id->rulesSent += numRules;
    statusUpdateStat(SSH_STATS_RULES_SENT, id->rulesSent);
    return OK;
}


void sshUtilsExit (SSH_ID id)
{
    SSH_RULE_ID rule;

    for (rule = (SSH_RULE_ID)radListRemoveFirst (&id->rules);
         rule != NULL;
         rule = (SSH_RULE_ID)radListRemoveFirst (&id->rules))
    {
        free (rule);
    }
}

