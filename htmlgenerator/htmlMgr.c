/*---------------------------------------------------------------------------
 
  FILENAME:
        htmlMgr.c
 
  PURPOSE:
        Provide the wview html generator utilities.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        08/30/03        M.S. Teel       0               Original
 
  NOTES:
        
 
  LICENSE:
        Copyright (c) 2004, Mark S. Teel (mark@teel.ws)
  
        This source code is released for free distribution under the terms 
        of the GNU General Public License.
  
----------------------------------------------------------------------------*/

/*  ... System include files
*/
#include <termios.h>

/*  ... Library include files
*/
#include <radtimeUtils.h>

/*  ... Local include files
*/
#include <services.h>
#include <dbsqlite.h>
#include <html.h>
#include <htmlMgr.h>
#include <glbucket.h>
#include <glchart.h>
#include <images.h>
#include <images-user.h>


/*  ... global memory declarations
*/
char sampleLabels[MAX_DAILY_NUM_VALUES][8];
char sampleHourLabels[MAX_DAILY_NUM_VALUES][8];

/*  ... global memory referenced
*/

/*  ... static (local) memory declarations
*/
#define DEBUG_GENERATION            FALSE

static HTML_MGR     mgrWork;
static uint64_t     GenerateTime;



//  ... local utilities

static int readImageConfFile (HTML_MGR *mgr, char *filename, int isUser)
{
    HTML_IMG        *img;
    FILE            *file;
    int             i, done;
    char            *token;
    char            temp[1536];

    file = fopen (filename, "r");
    if (file == NULL)
    {
        radMsgLog (PRI_HIGH, "htmlmgrInit: %s does not exist!",
                   filename);
        return ERROR_ABORT;
    }

    while (fgets (temp, 1536, file) != NULL)
    {
        if (temp[0] == ' ' || temp[0] == '\n' || temp[0] == '#')
        {
            // comment or whitespace
            continue;
        }

        token = strtok (temp, " \t");
        if (token == NULL)
        {
            continue;
        }

        img = (HTML_IMG *)malloc (sizeof (*img));
        if (img == NULL)
        {
            for (img = (HTML_IMG *)radListRemoveFirst (&mgr->imgList);
                    img != NULL;
                    img = (HTML_IMG *)radListRemoveFirst (&mgr->imgList))
            {
                free (img);
            }

            fclose (file);
            return ERROR;
        }
        memset (img, 0, sizeof (*img));

        img->mgrWork    = mgr;
        
        
        //  ... if legacy image type is found, skip it
        if (!strncmp (token, "IMG_TYPE_", 9))
        {
            // get the next token
            token = strtok (NULL, " \t");
            if (token == NULL)
            {
                free (img);
                goto line_loop_end;
            }
        }

        //  ... do the file name
        wvstrncpy (img->fname, token, sizeof(img->fname));

        //  ... now the label
        token = strtok (NULL, " \t");
        if (token == NULL)
        {
            free (img);
            goto line_loop_end;
        }
        if (token[strlen(token)-1] == '"')
        {
            // we're done!
            strncpy (img->title, &token[1], strlen(token)-2);   // lose quotes
        }
        else
        {
            done = FALSE;
            wvstrncpy (img->title, &token[1], sizeof(img->title));
            while (!done)
            {
                token = strtok (NULL, " ");
                if (token == NULL)
                {
                    free(img);
                    goto line_loop_end;
                }
                else
                {
                    if (token[strlen(token)-1] == '"')
                    {
                        done = TRUE;
                        token[strlen(token)-1] = 0;
                    }
                    strcat (img->title, " ");
                    strcat (img->title, token);
                }
            }
        }

        //  ... now the units label (no spaces allowed)
        token = strtok (NULL, " \t");
        if (token == NULL)
        {
            free (img);
            goto line_loop_end;
        }
        wvstrncpy (img->units, token, sizeof(img->units));

        //  ... now the decimal places
        token = strtok (NULL, " \t");
        if (token == NULL)
        {
            free (img);
            goto line_loop_end;
        }
        img->decimalPlaces = atoi (token);

        //  ... finally, the generator index
        token = strtok (NULL, " \t\n");
        if (token == NULL)
        {
            free (img);
            goto line_loop_end;
        }
        i = atoi (token);
        if (isUser)
        {
            img->generator = user_generators[i];
        }
        else
        {
            img->generator = images_generators[i];
        }

        radListAddToEnd (&mgr->imgList, (NODE_PTR)img);

// Loop end label:
line_loop_end:
        continue;
    }

    fclose (file);
    return OK;
}

static int readHtmlTemplateFile (HTML_MGR *mgr, char *filename)
{
    HTML_TMPL       *html;
    FILE            *file;
    char            *token;
    char            temp[HTML_MAX_LINE_LENGTH];

    file = fopen (filename, "r");
    if (file == NULL)
    {
        radMsgLog (PRI_HIGH, "htmlmgrInit: %s does not exist!",
                   filename);
        return ERROR_ABORT;
    }

    while (fgets (temp, HTML_MAX_LINE_LENGTH, file) != NULL)
    {
        if (temp[0] == ' ' || temp[0] == '\n' || temp[0] == '#')
        {
            // comment or whitespace
            continue;
        }

        html = (HTML_TMPL *)malloc (sizeof (*html));
        if (html == NULL)
        {
            for (html = (HTML_TMPL *)radListRemoveFirst (&mgr->templateList);
                 html != NULL;
                 html = (HTML_TMPL *)radListRemoveFirst (&mgr->templateList))
            {
                free (html);
            }

            fclose (file);
            return ERROR;
        }

        // do the template file name
        token = strtok (temp, " \t\n");
        if (token == NULL)
        {
            free (html);
            continue;
        }
        wvstrncpy (html->fname, token, sizeof(html->fname));

        radListAddToEnd (&mgr->templateList, (NODE_PTR)html);
    }

    fclose (file);
    return OK;
}

static void cleanupForecastRules (HTML_MGR *mgr)
{
    int             i;
    
    for (i = 0; i <= HTML_MAX_FCAST_RULE; i ++)
    {
        if (mgr->ForecastRuleText[i] != NULL)
        {
            free (mgr->ForecastRuleText[i]);
            mgr->ForecastRuleText[i] = NULL;
        }
    }

    for (i = 1; i <= VP_FCAST_ICON_MAX; i ++)
    {
        if (mgr->ForecastIconFile[i] != NULL)
        {
            free (mgr->ForecastIconFile[i]);
            mgr->ForecastIconFile[i] = NULL;
        }
    }
    
    return;
}       

static int readForecastRuleConfigFile (HTML_MGR *mgr, char *filename)
{
    FILE            *file;
    char            *token;
    char            temp[HTML_MAX_LINE_LENGTH];
    int             iconNo = 1, ruleNo = 0;

    file = fopen (filename, "r");
    if (file == NULL)
    {
        return ERROR_ABORT;
    }

    //  read each line starting with "ICON", assigning to the corresponding 
    //  forecast icon
    while (fgets (temp, HTML_MAX_LINE_LENGTH-1, file) != NULL)
    {
        // does the line begin with "ICON"?
        if (strncmp (temp, "ICON", 4))
        {
            // nope
            continue;
        }
        
        // get the memory to store the string
        mgr->ForecastIconFile[iconNo] = (char *) malloc (FORECAST_ICON_FN_MAX);
        if (mgr->ForecastIconFile[iconNo] == NULL)
        {
            radMsgLog (PRI_HIGH, 
                       "readForecastRuleConfigFile: cannot allocate memory for forecast icon #%d!",
                       iconNo);
            cleanupForecastRules (mgr);
            fclose(file);
            return ERROR;
        }
        
        // skip the ICON keyword
        token = strtok (temp, " \t");
        if (token == NULL)
        {
            free (mgr->ForecastIconFile[iconNo]);
            mgr->ForecastIconFile[iconNo] = NULL;
            continue;
        }
        token = strtok (NULL, " \t\n");
        if (token == NULL)
        {
            free (mgr->ForecastIconFile[iconNo]);
            mgr->ForecastIconFile[iconNo] = NULL;
            continue;
        }

        wvstrncpy (mgr->ForecastIconFile[iconNo], 
                   token, 
                   FORECAST_ICON_FN_MAX);
        if (mgr->ForecastIconFile[iconNo][strlen(mgr->ForecastIconFile[iconNo]) - 1] == '\n')
            mgr->ForecastIconFile[iconNo][strlen(mgr->ForecastIconFile[iconNo]) - 1] = 0;
        
        if (++iconNo == (VP_FCAST_ICON_MAX + 1))
        {
            // we're done!
            break;
        }
    }

    
    // reset to the beginning of the file
    fseek (file, 0, SEEK_SET);
    

    // throw away all lines until we hit the keyword "RULES"
    while (fgets (temp, HTML_MAX_LINE_LENGTH-1, file) != NULL)
    {
        if (!strncmp (temp, "<RULES>", 7))
        {
            // begin the rule dance
            break;
        }
    }

    //  read each line, assigning to the corresponding forecast rule
    while (fgets (temp, HTML_MAX_LINE_LENGTH-1, file) != NULL)
    {
        // get the memory to store the string
        mgr->ForecastRuleText[ruleNo] = (char *) malloc (strlen(temp) + 1);
        if (mgr->ForecastRuleText[ruleNo] == NULL)
        {
            radMsgLog (PRI_HIGH, "readForecastRuleConfigFile: cannot allocate memory for forecast rule #%d!",
                       ruleNo);
            cleanupForecastRules (mgr);
            fclose(file);
            return ERROR;
        }
        
        // copy the string (we know temp is bounded and we allocated for its length):
        strcpy (mgr->ForecastRuleText[ruleNo], temp);
        if (mgr->ForecastRuleText[ruleNo][strlen(temp) - 1] == '\n')
            mgr->ForecastRuleText[ruleNo][strlen(temp) - 1] = 0;
        
        if (++ruleNo == (HTML_MAX_FCAST_RULE + 1))
        {
            // we're done!
            break;
        }
    }
    
    // did we find them all?
    if (ruleNo != (HTML_MAX_FCAST_RULE + 1))
    {
        // nope, let's fuss about it
        radMsgLog (PRI_MEDIUM, "readForecastRuleConfigFile: NOT all forecast rules are assigned in %s: "
                               "the last %d are missing and will be empty ...",
                   filename,
                   (HTML_MAX_FCAST_RULE + 1) - ruleNo);
    }
    if (iconNo != (VP_FCAST_ICON_MAX + 1))
    {
        // nope, let's fuss about it
        radMsgLog (PRI_MEDIUM, "readForecastRuleConfigFile: NOT all forecast icons are assigned in %s: "
                               "the last %d are missing and will be empty ...",
                   filename,
                   (VP_FCAST_ICON_MAX + 1) - iconNo);
    }

    radMsgLog (PRI_STATUS, "%d icon definitions, %d forecast rules found ...",
               iconNo - 1,
               ruleNo);

    fclose (file);
    return OK;
}

static void emptyWorkLists
(
    HTML_MGR_ID     id
)
{
    NODE_PTR        nptr;

    for (nptr = radListRemoveFirst (&id->imgList);
         nptr != NULL;
         nptr = radListRemoveFirst (&id->imgList))
    {
        free (nptr);
    }

    for (nptr = radListRemoveFirst (&id->templateList);
         nptr != NULL;
         nptr = radListRemoveFirst (&id->templateList))
    {
        free (nptr);
    }

    return;
}

//  ... API methods

HTML_MGR_ID htmlmgrInit
(
    char            *installPath,
    int             isMetricUnits,
    char            *imagePath,
    char            *htmlPath,
    int             arcInterval,
    int             isExtendedData,
    char            *name,
    char            *city,
    char            *state,
    int16_t         elevation,
    int16_t         latitude,
    int16_t         longitude,
    char            *mphaseIncrease,
    char            *mphaseDecrease,
    char            *mphaseFull,
    char            *radarURL,
    char            *forecastURL,
    char            *dateFormat,
    int             isDualUnits
)
{
    HTML_MGR_ID     newId;
    int             i, numNodes, numImages, numTemplates;
    char            confFilePath[256];

    newId = &mgrWork;
    memset (newId, 0, sizeof (mgrWork));

    radListReset (&newId->imgList);
    radListReset (&newId->templateList);
    
    newId->isMetricUnits    = isMetricUnits;
    newId->archiveInterval  = arcInterval;
    newId->isExtendedData   = isExtendedData;
    newId->imagePath        = imagePath;
    newId->htmlPath         = htmlPath;
    newId->stationElevation = elevation;
    newId->stationLatitude  = latitude;
    newId->stationLongitude = longitude;
    wvstrncpy (newId->stationName, name, sizeof(newId->stationName));
    wvstrncpy (newId->stationCity, city, sizeof(newId->stationCity));
    wvstrncpy (newId->stationState, state, sizeof(newId->stationState));
    wvstrncpy (newId->mphaseIncrease, mphaseIncrease, sizeof(newId->mphaseIncrease));
    wvstrncpy (newId->mphaseDecrease, mphaseDecrease, sizeof(newId->mphaseDecrease));
    wvstrncpy (newId->mphaseFull, mphaseFull, sizeof(newId->mphaseFull));
    wvstrncpy (newId->radarURL, radarURL, sizeof(newId->radarURL));
    wvstrncpy (newId->forecastURL, forecastURL, sizeof(newId->forecastURL));
    wvstrncpy (newId->dateFormat, dateFormat, sizeof(newId->dateFormat));
    newId->isDualUnits    = isDualUnits;
    
    //  ... initialize the newArchiveMask
    newId->newArchiveMask = NEW_ARCHIVE_ALL;

    
    //  ... first, the built-in generators
    sprintf (confFilePath, "%s/images.conf", installPath);
    if (readImageConfFile (newId, confFilePath, FALSE) != OK)
    {
        return NULL;
    }
    else
    {
        numNodes = radListGetNumberOfNodes (&newId->imgList);
        radMsgLog (PRI_STATUS, "htmlmgrInit: %d built-in image definitions added",
                   numNodes);
        numImages = numNodes;
    }

    //  ... next, the user-defined generators
    sprintf (confFilePath, "%s/images-user.conf", installPath);
    i = readImageConfFile (newId, confFilePath, TRUE);
    if (i == ERROR_ABORT)
    {
        radMsgLog (PRI_STATUS, "htmlmgrInit: no user image definitions found");
    }
    else if (i == ERROR)
    {
        return NULL;
    }
    else
    {
        numImages = radListGetNumberOfNodes (&newId->imgList);
        radMsgLog (PRI_STATUS, "htmlmgrInit: %d user image definitions added",
                   radListGetNumberOfNodes (&newId->imgList) - numNodes);
    }

    //  ... now initialize our html template list
    sprintf (confFilePath, "%s/html-templates.conf", installPath);
    if (readHtmlTemplateFile (newId, confFilePath) != OK)
    {
        return NULL;
    }
    else
    {
        numTemplates = radListGetNumberOfNodes (&newId->templateList);
        radMsgLog (PRI_STATUS, "htmlmgrInit: %d templates added", numTemplates);
    }

    //  ... now initialize our forecast rule text list
    sprintf (confFilePath, "%s/forecast.conf", installPath);
    if (readForecastRuleConfigFile (newId, confFilePath) != OK)
    {
        radMsgLog (PRI_STATUS, "htmlmgrInit: forecast html tags are disabled - %s not found...",
                   confFilePath);
    }

    //  ... initialize the sample label array
    htmlmgrSetSampleLabels(newId);

    //  ... initialize our dial palette to conserve CPU cycles
    if (htmlGenPngDialInit (newId->imagePath) == ERROR)
    {
        radMsgLog (PRI_HIGH, "htmlmgrInit: htmlGenPngDialInit failed!");
        return NULL;
    }

    statusUpdateStat(HTML_STATS_IMAGES_DEFINED, numImages);
    statusUpdateStat(HTML_STATS_TEMPLATES_DEFINED, numTemplates);

    return newId;
}

int htmlmgrReReadImageFiles
(
    HTML_MGR_ID     id,
    char            *installPath
)
{
    int             i, numNodes;
    char            confFilePath[256];

    // cleanup the old definitions
    cleanupForecastRules (id);
    emptyWorkLists (id);
    radListReset (&id->imgList);
    radListReset (&id->templateList);
    
    //  re-read the built-in generators
    sprintf (confFilePath, "%s/images.conf", installPath);
    if (readImageConfFile (id, confFilePath, FALSE) != OK)
    {
        return ERROR;
    }
    else
    {
        numNodes = radListGetNumberOfNodes (&id->imgList);
        radMsgLog (PRI_STATUS, "htmlmgrReReadImageFiles: %d built-in image definitions added",
                   numNodes);
    }

    //  re-read the user-defined generators
    sprintf (confFilePath, "%s/images-user.conf", installPath);
    i = readImageConfFile (id, confFilePath, TRUE);
    if (i == ERROR_ABORT)
    {
        radMsgLog (PRI_STATUS, "htmlmgrReReadImageFiles: no user image definitions found");
    }
    else if (i == ERROR)
    {
        return ERROR;
    }
    else
    {
        radMsgLog (PRI_STATUS, "htmlmgrReReadImageFiles: %d user image definitions added",
                   radListGetNumberOfNodes (&id->imgList) - numNodes);
    }

    //  reinitialize our html template list
    sprintf (confFilePath, "%s/html-templates.conf", installPath);
    if (readHtmlTemplateFile (id, confFilePath) != OK)
    {
        return ERROR;
    }
    else
    {
        radMsgLog (PRI_STATUS, "htmlmgrReReadImageFiles: %d templates added",
                   radListGetNumberOfNodes (&id->templateList));
    }

    //  reinitialize our forecast rule text list
    sprintf (confFilePath, "%s/forecast.conf", installPath);
    if (readForecastRuleConfigFile (id, confFilePath) != OK)
    {
        radMsgLog (PRI_STATUS, "htmlmgrReReadImageFiles: forecast html tags are disabled - %s not found...",
                   confFilePath);
    }

    return OK;
}

void htmlmgrExit
(
    HTML_MGR_ID     id
)
{
    cleanupForecastRules (id);

    emptyWorkLists (id);

    return;
}

void htmlmgrSetSampleLabels(HTML_MGR_ID id)
{
    int         i;
    time_t      startTime, ntime = time(NULL);
    struct tm   tmtime;
    int         daySamples;

    // initialize the sample label array:
    localtime_r(&ntime, &tmtime);
    daySamples = id->dayStart;

    startTime = ntime - WV_SECONDS_IN_DAY;
    localtime_r(&startTime, &tmtime);
    tmtime.tm_sec   = 0;
    tmtime.tm_hour  = (daySamples * id->archiveInterval) / 60;
    tmtime.tm_min   = (daySamples * id->archiveInterval) % 60;
    startTime = mktime(&tmtime);

    for (i = 0; i < DAILY_NUM_VALUES(id) - 1; i ++)
    {
        if (daySamples >= DAILY_NUM_VALUES(id) - 1)
            daySamples = 0;

        localtime_r(&startTime, &tmtime);
        sprintf (sampleLabels[daySamples], "%d:%2.2d", tmtime.tm_hour, tmtime.tm_min);
        sprintf (sampleHourLabels[daySamples], "%d:00", tmtime.tm_hour);

        startTime += (60*id->archiveInterval);
        daySamples ++;
    }

    // Do the current time (always store it in the last slot):
    localtime_r(&startTime, &tmtime);
    sprintf (sampleLabels[DAILY_NUM_VALUES(id) - 1], "%d:%2.2d", tmtime.tm_hour, tmtime.tm_min);
    startTime += WV_SECONDS_IN_HOUR;
    localtime_r(&startTime, &tmtime);
    sprintf (sampleHourLabels[DAILY_NUM_VALUES(id) - 1], "%d:00", tmtime.tm_hour);
}

int htmlmgrBPTrendInit
(
    HTML_MGR_ID         id,
    int                 timerIntervalMINs
)
{
    int                 i;
    
    //  ... initialize the barometric pressure trend array
    for (i = 0; i < BP_MAX_VALUES; i ++)
    {
        id->baromTrendValues[i] = 0;
    }
    
    id->baromTrendNumValues = 0;
    id->baromTrendIndex = 0;
    id->baromTrendIndexMax = 60/timerIntervalMINs;      // samples per hour
    id->baromTrendIndexMax *= 4;                        // 4 hours
    id->baromTrendIndicator = 1;                        // steady
    
    return OK;
}


static int computeBPTrend
(
    HTML_MGR_ID         id
)
{
    register int        i;
    register float      avg, sum = 0;
    float               currVal = id->loopStore.barometer;
    
    //  ... compute the barometric pressure trend
    //  ... we just use a circular queue since the order of the values
    //  ... doesn't really matter, and it is more efficient
    
    //  ... save the new value
    id->baromTrendValues[id->baromTrendIndex] = currVal;
    
    //  ... do we need to wrap the index?
    if (++ id->baromTrendIndex >= id->baromTrendIndexMax)
        id->baromTrendIndex = 0;
    
    //  ... manage the number of values
    if (id->baromTrendNumValues < id->baromTrendIndexMax)
        id->baromTrendNumValues ++;
    
    //  ... compute the average over all values
    for (i = 0; i < id->baromTrendNumValues; i ++)
    {
        sum += id->baromTrendValues[i];
    }
    avg = sum/id->baromTrendNumValues;
    
    if (currVal < avg)
        id->baromTrendIndicator = 0;
    else if (currVal > avg)
        id->baromTrendIndicator = 2;
    else
        id->baromTrendIndicator = 1;
    
    return OK;
}

static int newProcessEntryPoint (void * pargs)
{
    char*               binFile = (char*)pargs;

    return (system(binFile));
}



int htmlmgrGenerate
(
    HTML_MGR_ID         id
)
{
    register HTML_IMG   *img;
    int                 retVal, imgs = 0, htmls = 0;
    char                temp[256];
    struct stat         fileData;

    GenerateTime = radTimeGetMSSinceEpoch ();

#if __DEBUG_BUFFERS
    radMsgLog (PRI_STATUS, "DBG BFRS: HTML BEGIN: %u of %u available",
               buffersGetAvailable (),
               buffersGetTotal ());
#endif

    //  ... compute the Barometric Pressure trend
    computeBPTrend (id);

#if DEBUG_GENERATION
    radMsgLog (PRI_MEDIUM, "GENERATE: images");
#endif

    //  ... generate the weather images
    for (img = (HTML_IMG *)radListGetFirst (&id->imgList);
         img != NULL;
         img = (HTML_IMG *)radListGetNext (&id->imgList, (NODE_PTR)img))
    {
        retVal = (*img->generator) (img);
        if (retVal == OK)
        {
            imgs ++;
        }
        else if (retVal != ERROR_ABORT)
        {
            sprintf (temp, "%s/%s", id->imagePath, img->fname);
            radMsgLog (PRI_HIGH, "%s generation failed - must be local to the wview server!",
                       temp);
            radMsgLog (PRI_HIGH, "Otherwise you may be including data in "
                                 "images.conf for which you do not have sensors?!?");
        }
    }

    //  ... clear the archiveAvailable flag (must be after generator loop)
    id->newArchiveMask = 0;

#if DEBUG_GENERATION
    radMsgLog (PRI_MEDIUM, "GENERATE: pre-generate script");
#endif

    // If the wview pre-generation script exists, run it now
    sprintf (temp, "%s/%s", WVIEW_CONFIG_DIR, HTML_PRE_GEN_SCRIPT);
    if (stat (temp, &fileData) == 0)
    {
        // File exists, run it
        radStartProcess (newProcessEntryPoint, temp);
    }

#if DEBUG_GENERATION
    radMsgLog (PRI_MEDIUM, "GENERATE: templates");
#endif

    //  ... now generate the HTML
    if ((htmls = htmlgenOutputFiles(id, GenerateTime)) == ERROR)
    {
        return ERROR;
    }

    wvutilsLogEvent(PRI_STATUS, "Generated: %u ms: %d images, %d template files",
                    (uint32_t)(radTimeGetMSSinceEpoch() - GenerateTime), imgs, htmls);

    id->imagesGenerated += imgs;
    id->templatesGenerated += htmls;
    statusUpdateStat(HTML_STATS_IMAGES_GENERATED, id->imagesGenerated);
    statusUpdateStat(HTML_STATS_TEMPLATES_GENERATED, id->templatesGenerated);

#if __DEBUG_BUFFERS
    radMsgLog (PRI_STATUS, "DBG BFRS: HTML END: %u of %u available",
               buffersGetAvailable (),
               buffersGetTotal ());
#endif

#if DEBUG_GENERATION
    radMsgLog (PRI_MEDIUM, "GENERATE: post-generate script");
#endif

    // Finally, if the wview post-generation script exists, run it now
    sprintf (temp, "%s/%s", WVIEW_CONFIG_DIR, HTML_POST_GEN_SCRIPT);
    if (stat (temp, &fileData) == 0)
    {
        // File exists, run it
        radStartProcess (newProcessEntryPoint, temp);
    }

#if DEBUG_GENERATION
    radMsgLog (PRI_MEDIUM, "GENERATE: DONE");
#endif

    return OK;
}


// read archive database to initialize our historical arrays:
int htmlmgrHistoryInit (HTML_MGR_ID id)
{
    HISTORY_DATA    data;
    time_t          ntime, baseTime, arcTime;
    struct tm       locTime;
    int             i, j, retVal, saveHour;

    // Compute when last archive record should have been:
    arcTime = time(NULL);
    localtime_r(&arcTime, &locTime);
    locTime.tm_min = ((locTime.tm_min/id->archiveInterval)*id->archiveInterval);
    locTime.tm_sec = 0;
    arcTime = mktime(&locTime);

    // first, figure out our start label indexes and populate the history arrays
    // for each of day, week, month and year:

    // do the samples in the last day:
    id->dayStart = wvutilsGetDayStartTime(id->archiveInterval);

    // update the sample label array:
    htmlmgrSetSampleLabels(id);

    baseTime = arcTime;
    saveHour = -1;

    for (i = 0; i < DAILY_NUM_VALUES(id); i ++)
    {
        ntime = baseTime;
        ntime -= (WV_SECONDS_IN_DAY - (i*SECONDS_IN_INTERVAL(id->archiveInterval)));

        retVal = dbsqliteArchiveGetAverages (id->isMetricUnits,
                                             id->archiveInterval,
                                             &data,
                                             ntime, 
                                             1);

        if (retVal <= 0)
        {
            for (j = 0; j < DATA_INDEX_MAX(id->isExtendedData); j ++)
            {
                id->dayValues[j][i] = ARCHIVE_VALUE_NULL;
            }
        }
        else
        {
            for (j = 0; j < DATA_INDEX_MAX(id->isExtendedData); j ++)
            {
                if (data.values[j] <= ARCHIVE_VALUE_NULL || data.samples[j] == 0)
                {
                    id->dayValues[j][i] = ARCHIVE_VALUE_NULL;
                }
                else if (j == DATA_INDEX_rain || j == DATA_INDEX_windDir || j == DATA_INDEX_ET)
                {
                    id->dayValues[j][i] = data.values[j];
                }
                else
                {
                    id->dayValues[j][i] = data.values[j]/data.samples[j];
                }
            }
        }

        if (((i % 50) == 0) || ((i + 1) == DAILY_NUM_VALUES(id)))
        {
            radMsgLog(PRI_STATUS, "htmlHistoryInit: DAY: samples=%d", i);
        }
    }

    //  do the hours in the last week:
    id->weekStartTime_T = wvutilsGetWeekStartTime (id->archiveInterval);
    ntime = id->weekStartTime_T;

    for (i = 0; i < WEEKLY_NUM_VALUES; i ++)
    {
        retVal = dbsqliteArchiveGetAverages (id->isMetricUnits,
                                             id->archiveInterval,
                                             &data,
                                             ntime,
                                             WV_SECONDS_IN_HOUR/SECONDS_IN_INTERVAL(id->archiveInterval));

        if (retVal <= 0)
        {
            for (j = 0; j < DATA_INDEX_MAX(id->isExtendedData); j ++)
            {
                id->weekValues[j][i] = ARCHIVE_VALUE_NULL;
            }
        }
        else
        {
            for (j = 0; j < DATA_INDEX_MAX(id->isExtendedData); j ++)
            {
                if (data.values[j] <= ARCHIVE_VALUE_NULL || data.samples[j] == 0)
                {
                    id->weekValues[j][i] = ARCHIVE_VALUE_NULL;
                }
                else if (j == DATA_INDEX_rain || j == DATA_INDEX_windDir || j == DATA_INDEX_ET)
                {
                    id->weekValues[j][i] = data.values[j];
                }
                else
                {
                    id->weekValues[j][i] = data.values[j]/data.samples[j];
                }
            }
        }

        if (((i % 50) == 0) || ((i + 1) == WEEKLY_NUM_VALUES))
        {
            radMsgLog(PRI_STATUS, "htmlHistoryInit: WEEK: samples=%d", i);
        }

        ntime += WV_SECONDS_IN_HOUR;
    }

    //  do the hours in the last month:
    id->monthStartTime_T = wvutilsGetMonthStartTime (id->archiveInterval);
    ntime = id->monthStartTime_T;

    for (i = 0; i < MONTHLY_NUM_VALUES; i ++)
    {
        retVal = dbsqliteArchiveGetAverages (id->isMetricUnits,
                                             id->archiveInterval,
                                             &data,
                                             ntime,
                                             WV_SECONDS_IN_HOUR/SECONDS_IN_INTERVAL(id->archiveInterval));

        if (retVal <= 0)
        {
            for (j = 0; j < DATA_INDEX_MAX(id->isExtendedData); j ++)
            {
                id->monthValues[j][i] = ARCHIVE_VALUE_NULL;
            }
        }
        else
        {
            for (j = 0; j < DATA_INDEX_MAX(id->isExtendedData); j ++)
            {
                if (data.values[j] <= ARCHIVE_VALUE_NULL || data.samples[j] == 0)
                {
                    id->monthValues[j][i] = ARCHIVE_VALUE_NULL;
                }
                else if (j == DATA_INDEX_rain || j == DATA_INDEX_windDir || j == DATA_INDEX_ET)
                {
                    id->monthValues[j][i] = data.values[j];
                }
                else
                {
                    id->monthValues[j][i] = data.values[j]/data.samples[j];
                }
            }
        }

        if (((i % 50) == 0) || ((i + 1) == MONTHLY_NUM_VALUES))
        {
            radMsgLog(PRI_STATUS, "htmlHistoryInit: MONTH: samples=%d", i);
        }

        ntime += WV_SECONDS_IN_HOUR;
    }

    //  do the days in the last year (not including today):
    dbsqliteHistoryInit();
    dbsqliteHistoryPragmaSet("synchronous", "off");

    ntime = wvutilsGetYearStartTime (id->archiveInterval);

    id->yearStartTime_T = ntime;

    for (i = 0; i < YEARLY_NUM_VALUES; i ++)
    {
        data.startTime = ntime;
        if (dbsqliteHistoryGetDay(ntime, &data) == ERROR)
        {
            retVal = dbsqliteArchiveGetAverages(id->isMetricUnits,
                                                id->archiveInterval,
                                                &data,
                                                ntime,
                                                WV_SECONDS_IN_DAY/SECONDS_IN_INTERVAL(id->archiveInterval));

            if (retVal <= 0)
            {
                for (j = 0; j < DATA_INDEX_MAX(id->isExtendedData); j ++)
                {
                    data.values[j] = ARCHIVE_VALUE_NULL;
                }
            }
            else
            {
                // Add to the database:
                radMsgLog(PRI_STATUS, "htmlHistoryInit: storing day history for %s",
                          ctime(&ntime));
                dbsqliteHistoryInsertDay(&data);
            }
        }

        for (j = 0; j < DATA_INDEX_MAX(id->isExtendedData); j ++)
        {
            if (data.values[j] <= ARCHIVE_VALUE_NULL || data.samples[j] == 0)
            {
                id->yearValues[j][i] = ARCHIVE_VALUE_NULL;
            }
            else if (j == DATA_INDEX_rain || j == DATA_INDEX_windDir || j == DATA_INDEX_ET)
            {
                id->yearValues[j][i] = data.values[j];
            }
            else
            {
                id->yearValues[j][i] = data.values[j]/data.samples[j];
            }
        }

        if (((i % 50) == 0) || ((i + 1) == YEARLY_NUM_VALUES))
        {
            radMsgLog(PRI_STATUS, "htmlHistoryInit: YEAR: samples=%d", i);
        }

        ntime += WV_SECONDS_IN_DAY;
    }

    dbsqliteHistoryPragmaSet("synchronous", "normal");
    return OK;
}

int htmlmgrAddSampleValue (HTML_MGR_ID id, HISTORY_DATA *data, int numIntervals)
{
    register int    i, j;

    // check for data gap
    while (numIntervals > 1)
    {
        // we have apparently missed some archive records - add empty data in 
        // the "gap"
        
        // do the normal new record stuff first
        for (i = 0; i < DATA_INDEX_MAX(id->isExtendedData); i ++)
        {
            for (j = 0; j < DAILY_NUM_VALUES(id) - 1; j ++)
            {
                id->dayValues[i][j] = id->dayValues[i][j+1];
            }
        }

        // populate history data with ARCHIVE_VALUE_NULL
        for (i = 0; i < DATA_INDEX_MAX(id->isExtendedData); i ++)
        {
            id->dayValues[i][DAILY_NUM_VALUES(id)-1] = ARCHIVE_VALUE_NULL;
        }

        // decrement interval count
        numIntervals --;
    }

    // now add the new record data
    for (i = 0; i < DATA_INDEX_MAX(id->isExtendedData); i ++)
    {
        for (j = 0; j < DAILY_NUM_VALUES(id) - 1; j ++)
        {
            id->dayValues[i][j] = id->dayValues[i][j+1];
        }
    }

    for (i = 0; i < DATA_INDEX_MAX(id->isExtendedData); i ++)
    {
        id->dayValues[i][DAILY_NUM_VALUES(id)-1] = data->values[i];

        if (data->values[i] > ARCHIVE_VALUE_NULL && data->samples[i] != 0 &&
            i != DATA_INDEX_rain && i != DATA_INDEX_windDir && i != DATA_INDEX_ET)
        {
            id->dayValues[i][DAILY_NUM_VALUES(id)-1] /= data->samples[i];
        }
    }

    // Compute the new day start interval:
    id->dayStart = wvutilsGetDayStartTime(id->archiveInterval);

    // update the sample label array:
    htmlmgrSetSampleLabels(id);

    return OK;
}

int htmlmgrAddHourValue (HTML_MGR_ID id, HISTORY_DATA *data)
{
    register int    i, j;

    for (i = 0; i < DATA_INDEX_MAX(id->isExtendedData); i ++)
    {
        for (j = 0; j < WEEKLY_NUM_VALUES-1; j ++)
        {
            id->weekValues[i][j] = id->weekValues[i][j+1];
        }
        for (j = 0; j < MONTHLY_NUM_VALUES-1; j ++)
        {
            id->monthValues[i][j] = id->monthValues[i][j+1];
        }
    }

    id->weekStartTime_T = wvutilsGetWeekStartTime (id->archiveInterval);
    id->monthStartTime_T = wvutilsGetMonthStartTime (id->archiveInterval);

    for (i = 0; i < DATA_INDEX_MAX(id->isExtendedData); i ++)
    {
        id->weekValues[i][WEEKLY_NUM_VALUES-1] = data->values[i];
        id->monthValues[i][MONTHLY_NUM_VALUES-1] = data->values[i];

        if (data->values[i] > ARCHIVE_VALUE_NULL && data->samples[i] != 0  &&
            i != DATA_INDEX_rain && i != DATA_INDEX_windDir && i != DATA_INDEX_ET)
        {
            id->weekValues[i][WEEKLY_NUM_VALUES-1] /= data->samples[i];
            id->monthValues[i][MONTHLY_NUM_VALUES-1] /= data->samples[i];
        }
    }

    return OK;
}

int htmlmgrAddDayValue (HTML_MGR_ID id, HISTORY_DATA *data)
{
    register int    i, j;

    for (i = 0; i < DATA_INDEX_MAX(id->isExtendedData); i ++)
    {
        for (j = 0; j < YEARLY_NUM_VALUES-1; j ++)
        {
            id->yearValues[i][j] = id->yearValues[i][j+1];
        }
    }

    id->yearStartTime_T += WV_SECONDS_IN_DAY;

    for (i = 0; i < DATA_INDEX_MAX(id->isExtendedData); i ++)
    {
        id->yearValues[i][YEARLY_NUM_VALUES-1] = data->values[i];

        if (data->values[i] > ARCHIVE_VALUE_NULL && data->samples[i] != 0  &&
            i != DATA_INDEX_rain && i != DATA_INDEX_windDir && i != DATA_INDEX_ET)
        {
            id->yearValues[i][YEARLY_NUM_VALUES-1] /= data->samples[i];
        }
    }

    return OK;
}
