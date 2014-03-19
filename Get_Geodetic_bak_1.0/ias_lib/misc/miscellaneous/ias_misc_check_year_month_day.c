/******************************************************************************
NAME:    ias_misc_check_year_month_day

PURPOSE: See if the given date is valid (including leap year consideration)

RETURN VALUE:
Type = int
Value    Description
-----    -----------
ERROR    Date was invalid
SUCCESS  Valid date
*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "ias_logging.h"
#include "ias_const.h"
#include "ias_math.h"
#include "ias_miscellaneous.h"

int ias_misc_check_year_month_day
(
    int year,     /* I: Year (YYYY) */
    int month,    /* I: Month (MM) */
    int day       /* I: Day (DD) */
)
{
    const int days_per_month[12]  = {31,28,31,30,31,30,31,31,30,31,30,31};
    const int min_day             = 1; /* Smallest day # in month */
    int       max_day;                 /* Largest day # in month */

    /* Validate the date */
    if (year < IAS_MIN_YEAR || year > IAS_MAX_YEAR)
    {
        IAS_LOG_ERROR("Invalid year (%d)", year);
        return ERROR;
    }

    /* Validate the month */
    if (month < IAS_MIN_MONTH || month > IAS_MAX_MONTH)
    {
        IAS_LOG_ERROR("Invalid month (%d)", month);
        return ERROR;
    }

    /* Determine the number of days in the month */
    max_day = days_per_month[month - 1];
    if ((month == 2) && ias_math_is_leap_year(year))
        max_day++;

    /* Validate the day */
    if (day < min_day || day > max_day)
    {
        IAS_LOG_ERROR("Invalid day (%d)", day);
        return ERROR;
    }
    return SUCCESS; 
}
