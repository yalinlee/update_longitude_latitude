/******************************************************************************
NAME:          ias_misc_convert_to_lowercase

PURPOSE:        
ias_misc_convert_to_lowercase converts all the upper case characters in a 
string to lower case.

RETURN VALUE:
Type = char *
Value    Description
-----    -----------
pointer  Returns a pointer to the string so it can immediately be used as
         a parameter to another function (such as strcmp)

NOTES:
The original string is converted in place.
******************************************************************************/
#include <ctype.h>              /* tolower prototype */
#include "ias_miscellaneous.h"

char *ias_misc_convert_to_lowercase 
(
    char *string_ptr  /* I/O: pointer to string to convert */
)
{
    char *c_ptr = string_ptr;

    while (*c_ptr != '\0')
    {
        *c_ptr = tolower(*c_ptr);
        c_ptr++;
    }

    return string_ptr;
}
