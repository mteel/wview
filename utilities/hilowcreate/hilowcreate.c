/*---------------------------------------------------------------------
 
 FILE NAME:
        hilowcreate.c
 
 PURPOSE:
        wview archive file convertor utility: SQLite3 database TO Davis WLK.
 
 REVISION HISTORY:
    Date        Programmer  Revision    Function
    03/01/2009  M.S. Teel   0           Original
 
 ASSUMPTIONS:
 None.
 
------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>

#include <sysdefs.h>
#include <datadefs.h>
#include <dbsqlite.h>


static void USAGE (void)
{
    printf ("Usage: hilowcreate <source_directory>\n\n");
    printf ("    Create wview HILOW data in <source_directory>/wview-hilow.sdb\n");
    printf ("    using archive records in <source_directory>/wview-archive.sdb\n\n");
    printf ("Note: If wview-hilow.sdb already exists in <source_directory>, it will be overwritten;\n");
    printf ("      <source_directory> cannot be the live wview archive, copy wview-archive.sdb to a work directory.\n\n");
    return;
}


// Create HILOW DB:
static void CreateHiLowDatabase (char *srcDir)
{
    time_t              timeStamp, diffTime, retVal, startTime = time(NULL);
    struct tm           buildTime;
    ARCHIVE_PKT         sqlitePkt;
    int                 inserts = 0, errors = 0;
    char                cmnd[_MAX_PATH];

    dbsqliteArchiveSetPath(srcDir);

    // ... Initialize the archive database interface:
    if (dbsqliteArchiveInit() == ERROR)
    {
        printf("dbsqliteArchiveInit failed");
        return;
    }

    // Remove any old copies:
    sprintf(cmnd, "rm -f %s/wview-hilow.sdb", srcDir);
    if (system(cmnd) != 0)
    {
        printf("system error:%s:%s\n", cmnd, strerror(errno));
    }

    if (dbsqliteHiLowInit(TRUE) == ERROR)
    {
        printf("dbsqliteHiLowInit failed");
        dbsqliteArchiveExit();
        return;
    }

    dbsqliteHiLowExit();
    dbsqliteArchiveExit();

    // Output results:
    printf ("%s/wview-hilow.sdb created successfully!\n", srcDir);
    printf ("Now copy %s/wview-hilow.sdb to %s/archive and restart wview 5.1.0 or newer...\n",
            srcDir, WV_RUN_DIR);
    return;
}

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char *argv[])
{
    char            *SourceDir;
    struct stat     fileData;
    char            tempPath[_MAX_PATH];
    
    if (argc < 2)
    {
        USAGE ();
        return ERROR;
    }

    SourceDir = argv[1];
    sprintf(tempPath, "%s/archive", WV_RUN_DIR);

    // sanity check the arguments
    if (stat(SourceDir, &fileData) != 0)
    {
        printf ("Source directory %s does not exist!\n", SourceDir);
        return ERROR;
    }
    else if (!(fileData.st_mode & S_IFDIR))
    {
        printf ("Source directory %s is not a directory!\n", SourceDir);
        return ERROR;
    }
    else if (! strcmp(SourceDir, tempPath))
    {
        printf ("ERROR: Source directory cannot be the active archive directory %s\n",
                tempPath);
        printf ("Copy your wview-archive.sdb database to a working directory\n");
        printf ("and use that as the source directory.\n");
        printf ("wview will add the new records created while this process is running\n");
        printf ("the next time it is started with the new HILOW database.\n");
        return ERROR;
    }

    // OK, args appear to be good, create HILOW data:
    printf("Creating HILOW database...(this takes ~ 30 seconds to 30 minutes per month of data according to your server platform)...\n");
    CreateHiLowDatabase (SourceDir);

    exit (0);
}

