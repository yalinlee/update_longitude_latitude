/*************************************************************************
NAME: ias_misc_get_timestamp

PURPOSE: Gets current time stamp

Algorithm References: None

RETURNS: SUCCESS -- successfully getting time
         ERROR -- error in getting time
**************************************************************************/
#include <time.h>
#include <string.h>
#include "ias_const.h"
#include "ias_miscellaneous.h"

int ias_misc_get_timestamp
(
    const char *p_format,    /* I: format of output timestamp */
    int stampsize,           /* I: size of timestamp for input */
    char *p_stamp            /* O: timestamp for output */
)
{
    time_t   ptime;               /* Time in seconds */

    ptime = time((time_t *) 0);

    /* Clear the string */
    memset(p_stamp, 0, stampsize);

    /* Format the date and time according to p_format specifications */
    if (strftime(p_stamp, stampsize, p_format, gmtime(&ptime) ) <= 0) 
        return ERROR;

    return SUCCESS;
}
