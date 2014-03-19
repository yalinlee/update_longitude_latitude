/*****************************************************************************

NAME: ias_cpf_convert_3dmonth_to_number

PURPOSE: Convert a 3digit ascii month ('Jan', ..., 'Dec') to its numerical
         value (1-12).
        
RETURN VALUE: SUCCESS or ERROR

*****************************************************************************/

#include <string.h>
#include "local_defines.h"

int ias_cpf_convert_3digit_month_to_number
(
    char *ascii_3digit_month, /* I: the 3digit ascii month */
    int *number               /* O: the numerical value of the month */
)
{
    /* Since this code is currently only used for leap seconds in the CPF
       library and the leap seconds are currently only in January or July,
       the following code checks those months first. */
    if (strncmp(ascii_3digit_month, "Jan", 3) == 0)
    {
        *number = 1;
    }
    else if (strncmp(ascii_3digit_month, "Jul", 3) == 0)
    {
        *number = 7;
    }
    else if (strncmp(ascii_3digit_month, "Feb", 3) == 0)
    {
        *number = 2;
    }
    else if (strncmp(ascii_3digit_month, "Mar", 3) == 0)
    {
        *number = 3;
    }
    else if (strncmp(ascii_3digit_month, "Apr", 3) == 0)
    {
        *number = 4;
    }
    else if (strncmp(ascii_3digit_month, "May", 3) == 0)
    {
        *number = 5;
    }
    else if (strncmp(ascii_3digit_month, "Jun", 3) == 0)
    {
        *number = 6;
    }
    else if (strncmp(ascii_3digit_month, "Aug", 3) == 0)
    {
        *number = 8;
    }
    else if (strncmp(ascii_3digit_month, "Sep", 3) == 0)
    {
        *number = 9;
    }
    else if (strncmp(ascii_3digit_month, "Oct", 3) == 0)
    {
        *number = 10;
    }
    else if (strncmp(ascii_3digit_month, "Nov", 3) == 0)
    {
        *number = 11;
    }
    else if (strncmp(ascii_3digit_month, "Dec", 3) == 0)
    {
        *number = 12;
    }
    else
    {
        IAS_LOG_ERROR("ASCII to digit month converstion failed: %s,",
            ascii_3digit_month);
        return ERROR;
    }

    return SUCCESS;
}
