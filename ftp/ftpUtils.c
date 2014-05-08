/*---------------------------------------------------------------------------
 
  FILENAME:
        ftpUtils.c
 
  PURPOSE:
        Provide the ftp utilities.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        01/08/04        M.S. Teel       0               Original
 
  NOTES:
        
 
  LICENSE:
        Copyright (c) 2004, Mark S. Teel (mark@teel.ws)
  
        This source code is released for free distribution under the terms 
        of the GNU General Public License.
  
----------------------------------------------------------------------------*/

/*  ... System include files
*/
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <fnmatch.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <locale.h>
#include <langinfo.h>
#include <stdio.h>
#include <stdint.h>
#include <libgen.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

/*  ... Library include files
*/
#include <radsysutils.h>

/*  ... Local include files
*/
#include <ftpUtils.h>


/*  ... global memory declarations
*/

/*  ... global memory referenced
*/
extern void FTPDefaultSigHandler (int signum);

/*  ... static (local) memory declarations
*/
static char         FTPRule[FTP_MAX_PATH];
static char         curlError[CURL_ERROR_SIZE];


static int FilterFile(const struct dirent *dp)
{
    if (fnmatch (FTPRule, dp->d_name, FNM_FILE_NAME) == 0)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static int SendRule(FTP_ID id, FTP_RULE_ID rule, const char* markerFile)
{
    static char     FTPCmndLine[FTP_MAX_CMND_LENGTH];
    struct dirent   **namelist;
    struct stat     statbuf;
    int             index, logindex, numFiles, cmndLength, hdrLength, retVal = 0;
    char            logBfr[RADMSGLOG_MAX_LENGTH];
    time_t          lastUpdate;
    char            dirnameStr[FTP_MAX_PATH], basenameStr[FTP_MAX_PATH], loginStr[128];
    char            tempRule[_MAX_PATH];
    CURLcode        res;
    FILE*           sendFile;

    // dirname and basename may be destructive of the input string:
    wvstrncpy(tempRule, rule->src, FTP_MAX_PATH);
    wvstrncpy(dirnameStr, dirname(tempRule), FTP_MAX_PATH);
    wvstrncpy(tempRule, rule->src, FTP_MAX_PATH);
    wvstrncpy(basenameStr, basename(tempRule), FTP_MAX_PATH);
    wvstrncpy(FTPRule, basenameStr, FTP_MAX_PATH);

    numFiles = scandir(dirnameStr, &namelist, FilterFile, alphasort);
    if (numFiles <= 0)
    {
        radMsgLog (PRI_HIGH, "FTP-ERROR: No files match rule %s", rule->src);
        return ERROR;
    }

    cmndLength = 0;

    if (strlen(id->directory) == 0)
    {
        cmndLength += sprintf (&FTPCmndLine[cmndLength], "ftp://%s/",
                               id->host);
    }
    else
    {
        cmndLength += sprintf (&FTPCmndLine[cmndLength], "ftp://%s/%s/",
                               id->host, id->directory);
    }

    hdrLength = cmndLength;

    sprintf(loginStr, "%s:%s", id->user, id->pass);

    // Grab last transmission time marker:
    // wvutilsReadMarkerFile returns 0 if the file does not exist:
    lastUpdate = wvutilsReadMarkerFile(markerFile);

    // Loop through all files in the list:
    for (index = 0; index < numFiles; index ++)
    {
        // Skip meta files:
        if (! strncmp(namelist[index]->d_name, ".", 8) ||
            ! strncmp(namelist[index]->d_name, "..", 8))
        {
            free(namelist[index]);
            continue;
        }

        // Get entry's information:
        if (strncmp(dirnameStr, ".", 2) != 0)
        {
            sprintf(tempRule, "%s/%s", dirnameStr, namelist[index]->d_name);
        }
        else
        {
            // In this directory:
            sprintf(tempRule, "%s", namelist[index]->d_name);
        }

        memset(&statbuf, 0, sizeof(statbuf));
        if (stat(tempRule, &statbuf) == -1)
        {
            free(namelist[index]);
            continue;
        }

        if (statbuf.st_mtime <= lastUpdate)
        {
            // nothing new:
            wvutilsLogEvent(PRI_STATUS, "FTP-STATUS: %s is not new, skipping it",
                            tempRule);
            free(namelist[index]);
            continue;
        }

        // send it now:
        cmndLength = hdrLength;
        cmndLength += sprintf(&FTPCmndLine[cmndLength], "%s", tempRule);

        // Log the command before executing it:
        for (logindex = 0; logindex < cmndLength; logindex += RADMSGLOG_MAX_LENGTH)
        {
            wvstrncpy(logBfr, &FTPCmndLine[logindex], RADMSGLOG_MAX_LENGTH);
            if (strlen(logBfr) > 0)
            {
                wvutilsLogEvent(PRI_STATUS, "FTP-URL: %s", logBfr);
            }
        }

        // Execute the curl transaction:
        sendFile = fopen(tempRule, "rb");
        if (sendFile == NULL)
        {
            sprintf(logBfr, "FTP-ERROR: failed to open %s", tempRule);
            radMsgLog (PRI_HIGH, "%s", logBfr);
            statusUpdateMessage(logBfr);
            free(namelist[index]);
            continue;
        }

        // Set curl URL, login and FILE* each time through:
        curl_easy_setopt(id->curlHandle, CURLOPT_URL, FTPCmndLine);
        curl_easy_setopt(id->curlHandle, CURLOPT_USERPWD, loginStr);
        curl_easy_setopt(id->curlHandle, CURLOPT_READDATA, sendFile);

        res = curl_easy_perform(id->curlHandle);
        if (res != CURLE_OK)
        {
            sprintf(logBfr, "FTP-ERROR: curl_easy_perform failed: %s", curlError);
            radMsgLog (PRI_HIGH, "%s", logBfr);
            statusUpdateMessage(logBfr);
            statusIncrementStat(FTP_STATS_CONNECT_ERRORS);
        }
        else
        {
            wvutilsLogEvent(PRI_STATUS, "FTP-SUCCESS: %s", tempRule);
            retVal ++;
        }

        fclose(sendFile);
        free(namelist[index]);
    }

    free(namelist);
    return retVal;
}


// API:
int ftpUtilsInit (FTP_DATA* data)
{
    FTP_RULE_ID     rule;
    FTP_ID          newId;
    int             i;
    char            conftype[WVIEW_STRING1_SIZE];
    int             iValue;
    const char*     source;
    int             interval;
    FileStates      state = FS_HOST;

    newId = (FTP_ID)data;
    radListReset (&newId->rules);
    newId->expiry = 0;

    if (wvconfigInit(FALSE) == ERROR)
    {
        radMsgLog (PRI_CATASTROPHIC, "wvconfigInit failed!");
        return ERROR;
    }

    // Is the ftp daemon enabled?
    iValue = wvconfigGetBooleanValue(configItem_ENABLE_FTP);
    if (iValue == ERROR || iValue == 0)
    {
        wvconfigExit ();
        radMsgLog (PRI_STATUS, "ftp daemon disabled - exiting...");
        return ERROR_ABORT;
    }

    // get the wview verbosity setting
    if (wvutilsSetVerbosity (WV_VERBOSE_WVIEWFTPD) == ERROR)
    {
        radMsgLog (PRI_CATASTROPHIC, "wvutilsSetVerbosity failed!");
        wvconfigExit ();
        return ERROR_ABORT;
    }
    

    source = wvconfigGetStringValue(configItemFTP_HOST);
    if (source == NULL)
    {
        // No host defined - bail:
        radMsgLog (PRI_CATASTROPHIC, "wvconfigGetStringValue %s failed!",
                   configItemFTP_HOST);
        wvconfigExit();
        return ERROR;
    }
    wvstrncpy (newId->host, source, sizeof(newId->host));
    radMsgLog (PRI_STATUS, "FTP-CONFIG: using remote host: %s", newId->host);

    source = wvconfigGetStringValue(configItemFTP_USERNAME);
    if (source == NULL)
    {
        // No username defined - bail:
        radMsgLog (PRI_CATASTROPHIC, "wvconfigGetStringValue %s failed!",
                   configItemFTP_USERNAME);
        wvconfigExit();
        return ERROR;
    }
    wvstrncpy (newId->user, source, sizeof(newId->user));

    source = wvconfigGetStringValue(configItemFTP_PASSWD);
    if (source == NULL)
    {
        // No passwd defined - bail:
        radMsgLog (PRI_CATASTROPHIC, "wvconfigGetStringValue %s failed!",
                   configItemFTP_PASSWD);
        wvconfigExit();
        return ERROR;
    }
    wvstrncpy (newId->pass, source, sizeof(newId->pass));

    source = wvconfigGetStringValue(configItemFTP_REMOTE_DIRECTORY);
    if (source == NULL)
    {
        // No dir defined - bail:
        radMsgLog (PRI_CATASTROPHIC, "wvconfigGetStringValue %s failed!",
                   configItemFTP_REMOTE_DIRECTORY);
        wvconfigExit();
        return ERROR;
    }
    wvstrncpy (newId->directory, source, sizeof(newId->directory));
    radMsgLog (PRI_STATUS, "FTP-CONFIG: using remote directory: %s", newId->directory);

    // Get passive mode setting:
    iValue = wvconfigGetBooleanValue(configItemFTP_USE_PASSIVE);
    if (iValue == ERROR || iValue == 1)
    {
        // Default to passive mode:
        newId->IsPassive = TRUE;
        radMsgLog (PRI_STATUS, "FTP-CONFIG: using EPSV mode for transfers");
    }
    else
    {
        newId->IsPassive = FALSE;
        radMsgLog (PRI_STATUS, "FTP-CONFIG: not using EPSV mode for transfers");
    }

    // Get default interval:
    iValue = wvconfigGetINTValue(configItemFTP_INTERVAL);
    if (iValue == ERROR || iValue == 0)
    {
        // Default to 5 minutes:
        newId->interval = 5;
        radMsgLog (PRI_STATUS, "FTP-CONFIG: using default 5 minute interval for transfers");
    }
    else
    {
        newId->interval = iValue;
        radMsgLog (PRI_STATUS, "FTP-CONFIG: using %d minute interval for transfers", iValue);
    }

    for (i = 1; i <= FTP_MAX_RULES; i ++) 
    {
        rule = (FTP_RULE_ID)malloc (sizeof (*rule));
        if (rule == NULL)
        {
            for (rule = (FTP_RULE_ID)radListRemoveFirst (&newId->rules);
                 rule != NULL;
                 rule = (FTP_RULE_ID)radListRemoveFirst (&newId->rules))
            {
                free (rule);
            }

            return ERROR;
        }

        sprintf (conftype, "FTP_RULE_%1.1d_SOURCE", i);
        source = wvconfigGetStringValue(conftype);
        if (source == NULL)
        {
            // No source given - continue:
            radMsgLog (PRI_MEDIUM, "wvconfigGetStringValue %s failed!",
                       conftype);
            free (rule);
            continue;
        }

        if (strlen(source) == 0)
        {
            // No source or interval given - continue:
            free (rule);
            continue;
        }
        else
        {
            wvstrncpy (rule->src, source, sizeof(rule->src));
            radListAddToEnd (&newId->rules, (NODE_PTR)rule);
        }
    }

    wvconfigExit();

    curl_global_init(CURL_GLOBAL_ALL);


    // Create curl handle here for all files:
    newId->curlHandle = curl_easy_init();
    if (newId->curlHandle == NULL)
    {
        radMsgLog (PRI_HIGH, "FTP-ERROR: failed to initialize curl!");
        statusUpdateMessage("failed to initialize curl");
        return ERROR;
    }

    // Setup curl options that are not file specific:

    // Make sure libcurl does NOT use SIG_ALARM:
    curl_easy_setopt(newId->curlHandle, CURLOPT_NOSIGNAL, 1L);

    curl_easy_setopt(newId->curlHandle, CURLOPT_ERRORBUFFER, curlError);
    curl_easy_setopt(newId->curlHandle, CURLOPT_UPLOAD, OPTION_TRUE);
    curl_easy_setopt(newId->curlHandle, CURLOPT_TCP_NODELAY, OPTION_TRUE);
    if (newId->IsPassive)
    {
        curl_easy_setopt(newId->curlHandle, CURLOPT_FTP_USE_EPSV, OPTION_TRUE);
    }
    else
    {
        curl_easy_setopt(newId->curlHandle, CURLOPT_FTP_USE_EPSV, OPTION_FALSE);
    }

    // Set overall timeout for a file:
    curl_easy_setopt(newId->curlHandle, CURLOPT_TIMEOUT, 20L);

    // Set the connection timeout:
    curl_easy_setopt(newId->curlHandle, CURLOPT_CONNECTTIMEOUT, 15L);

    // Set server FTP response timeout:
    curl_easy_setopt(newId->curlHandle, CURLOPT_FTP_RESPONSE_TIMEOUT, 10L);

    // Set the duration in seconds for the curl handle's DNS cache before flush:
    // (7 days)
    curl_easy_setopt(newId->curlHandle, CURLOPT_DNS_CACHE_TIMEOUT, 604800L);

    curl_easy_setopt(newId->curlHandle, CURLOPT_NOPROGRESS, OPTION_TRUE);
    curl_easy_setopt(newId->curlHandle, CURLOPT_FTP_CREATE_MISSING_DIRS, OPTION_TRUE);

    radMsgLog (PRI_MEDIUM, "FTP: INIT: %d rules added",
               radListGetNumberOfNodes (&newId->rules));

    return OK;
}

int ftpUtilsSendFiles (FTP_ID id, char *workdir)
{
    FTP_RULE_ID     rule;
    int             retVal, numRules = 0, numFiles = 0;
    char            markerFName[_MAX_PATH];
    time_t          updateTime = time(NULL);

    sprintf (markerFName, "%s/%s", WVIEW_RUN_DIR, FTP_MARKER_FILE);

    if (id->expiry > updateTime)
    {
        // not our time yet:
        return 0;
    }

    if (chdir(workdir) == -1)
    {
        radMsgLog (PRI_HIGH, "FTP: chdir failed: %s", strerror(errno));
        return ERROR;
    }

    for (rule = (FTP_RULE_ID)radListGetFirst (&id->rules);
         rule != NULL;
         rule = (FTP_RULE_ID)radListGetNext (&id->rules, (NODE_PTR)rule))
    {
        if (strlen(rule->src) == 0)
        {
            // skip this one, it is empty:
            continue;
        }

        wvutilsLogEvent(PRI_STATUS, "FTP-RULE: checking for new %s", rule->src);

        retVal = SendRule(id, rule, markerFName);
        if (retVal > 0)
        {
            numFiles += retVal;
        }

        numRules ++;
    }

    // First time through?
    if (id->expiry == 0)
    {
        id->expiry = updateTime;
    }

    while (id->expiry <= updateTime)
    {
        id->expiry += (id->interval * 60);
    }

    if (numRules == 0)
    {
        // nothing to do!
        return 0;
    }


    // Update status:
    id->rulesSent += numRules;
    statusUpdateStat(FTP_STATS_RULES_SENT, id->rulesSent);

    // Mark the last update time:
    wvutilsWriteMarkerFile(markerFName, updateTime);

    wvutilsLogEvent(PRI_STATUS, "FTP-DONE: sent %d files", numFiles);
    return numRules;
}


void ftpUtilsExit (FTP_ID id)
{
    FTP_RULE_ID rule;

    curl_easy_cleanup(id->curlHandle);
    curl_global_cleanup();

    for (rule = (FTP_RULE_ID)radListRemoveFirst (&id->rules);
         rule != NULL;
         rule = (FTP_RULE_ID)radListRemoveFirst (&id->rules))
    {
        free (rule);
    }
}


