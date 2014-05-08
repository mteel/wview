#ifndef INC_parserh
#define INC_parserh
/*---------------------------------------------------------------------------
 
  FILENAME:
        parser.h
 
  PURPOSE:
        Provide ASCII parsing utilities.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        01/15/2006      M.S. Teel       0               Original
 
  NOTES:
 
  LICENSE:
        Copyright (c) 2006, Mark S. Teel (mark@teel.ws)
  
        This source code is released for free distribution under the terms 
        of the GNU General Public License.
  
----------------------------------------------------------------------------*/

/*  ... System include files
*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <string.h>

/*  ... Library include files
*/
#include <radsysdefs.h>
#include <radbuffers.h>
#include <radtimeUtils.h>
#include <radlist.h>

/*  ... Local include files
*/
#include <sysdefs.h>


// define a token node
typedef struct
{
    NODE            node;
    char            value[WVIEW_STRING1_SIZE];
} PARSE_TOKEN;

// parse ID
typedef struct
{
    RADLIST         tokens;
    PARSE_TOKEN     *current;

} PARSE_DATA, *PARSER_ID;


// function prototypes

// create a parsing object
extern PARSER_ID parserInit (char *inputString, char *delimiters);

// get the first token
extern char *parserGetFirst (PARSER_ID id);

// get the next token
extern char *parserGetNext (PARSER_ID id);

// get the n'th token (starts with 1); resets the current pointer
extern char *parserGetNumber (PARSER_ID id, int n);

// destroy a parsing object
extern void parserExit (PARSER_ID id);

#endif

