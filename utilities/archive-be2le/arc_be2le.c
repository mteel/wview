/*---------------------------------------------------------------------
 
 FILE NAME:
        arc_be2le.c
 
 PURPOSE:
        wview archive file convertor utility: Little Endian to Big Endian.
 
 REVISION HISTORY:
    Date        Programmer  Revision    Function
    02/26/2006  M.S. Teel   0           Original
 
 ASSUMPTIONS:
 None.
 
------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>

#include <sysdefs.h>
#include <datadefs.h>
#include <dbfiles.h>
#include "wvutilities.h"


static void USAGE (void)
{
    printf ("Usage: arc-be2le <source_directory> <dest_directory>\n\n");
    printf ("    Convert wview archive data in <source_directory> from big endian\n");
    printf ("    to little endian then store the result in <dest_directory>\n\n");
    return;
}

int main (int argc, char *argv[])
{
    char            *SourceDir, *DestDir;
    struct stat     fileData;
    
    if (argc < 3)
    {
        USAGE ();
        return ERROR;
    }

    SourceDir = argv[1];
    DestDir = argv[2];

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
    if (stat (DestDir, &fileData) != 0)
    {
        printf ("Destination directory %s does not exist, creating...\n", DestDir);
        if (mkdir(DestDir, 0755) != 0)
        {
            printf ("Destination directory %s could not be created: %s\n", 
                    DestDir, strerror(errno));
            return -1;
        }
    }

    // OK, args appear to be good, convert files (set 'doLE2BE' to 0)
    wvuConvertWLKFiles (SourceDir, DestDir, 0);

    exit (0);
}
