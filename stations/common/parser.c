/*---------------------------------------------------------------------------
 
  FILENAME:
        parser.c
 
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
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <termios.h>
#include <time.h>
#include <errno.h>
#include <math.h>

/*  ... Library include files
*/

/*  ... Local include files
*/
#include <parser.h>


/*  ... local memory
*/

static void emptyTokenList (RADLIST_ID list)
{
    NODE_PTR        node;

    for (node = radListRemoveFirst(list);
         node != NULL;
         node = radListRemoveFirst(list))
    {
        radBufferRls (node);
    }
}


// create a parsing object
PARSER_ID parserInit (char *inputString, char *delimiters)
{
    PARSER_ID       newId;
    PARSE_TOKEN     *node;
    char            *token;
    char            temp[_MAX_PATH];

    newId = (PARSER_ID)radBufferGet (sizeof(PARSE_DATA));
    if (newId == NULL)
    {
        return NULL;
    }

    memset (newId, 0, sizeof(PARSE_DATA));
    radListReset (&newId->tokens);

    // parse the input
    wvstrncpy (temp, inputString, _MAX_PATH);
    token = strtok (temp, delimiters);
    if (token == NULL)
    {
        radBufferRls (newId);
        return NULL;
    }

    while (token != NULL)
    {
        node = (PARSE_TOKEN *)radBufferGet (sizeof(PARSE_TOKEN));
        if (node == NULL)
        {
            emptyTokenList (&newId->tokens);
            radBufferRls (newId);
            return NULL;
        }

        wvstrncpy (node->value, token, sizeof(node->value));
        radListAddToEnd (&newId->tokens, (NODE_PTR)node);

        // get the next one
        token = strtok (NULL, delimiters);
    }

    return newId;
}

// destroy a parsing object
void parserExit (PARSER_ID id)
{
    emptyTokenList (&id->tokens);
    radBufferRls (id);
}

// get the first token
char *parserGetFirst (PARSER_ID id)
{
    id->current = (PARSE_TOKEN *)radListGetFirst (&id->tokens);
    return id->current->value;
}

// get the next token
char *parserGetNext (PARSER_ID id)
{
    id->current = (PARSE_TOKEN *)radListGetNext (&id->tokens, (NODE_PTR)id->current);
    return id->current->value;
}

// get the n'th token
char *parserGetNumber (PARSER_ID id, int n)
{
    PARSE_TOKEN     *node;
    int             count;

    for (count = 1, node = (PARSE_TOKEN *)radListGetFirst(&id->tokens);
         node != NULL;
         count ++, node = (PARSE_TOKEN *)radListGetNext(&id->tokens, (NODE_PTR)node))
    {
        if (count == n)
        {
            // return this one
            id->current = node;
            return id->current->value;
        }
    }

    // if here, we had no luck
    return NULL;
}

