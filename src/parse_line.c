#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "struct_def.h"



// parse single line of text file into relevant fields
LINE parse_line(char* line, char* delim)
{

    LINE ls;

    int i=0;
    const char* tok;
    for (tok = strtok(line, delim);
            tok && *tok;
            tok = strtok(NULL, delim))
    {      
        switch(i)
        {
            case 0:
                strcpy(ls.ip, tok);
                break;
            case 1:
                strcpy(ls.date, tok);
                break;
            case 2:
                strcpy(ls.time, tok);
                break;
        }
        i++;
    }
    return ls;
}
