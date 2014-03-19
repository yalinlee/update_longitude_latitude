/******************************************************************************
NAME: ias_math_convert_doy_to_month_day

PURPOSE: Converts day of year to month/day for a specific year.

RETURNS: ERROR or SUCCESS

******************************************************************************/
#include "ias_const.h"
#include "ias_logging.h"
#include "ias_math.h"

int ias_math_convert_doy_to_month_day
(
    int doy,    /* I: Day of Year */  
    int year,   /* I: Year  */
    int *mon,   /* O: Month */
    int *day    /* O: Day of month */
)
{
    static const int noleap[IAS_MAX_MONTH] = { 0, 31, 59, 90, 120, 151, 181,
                                  212, 243, 273, 304, 334 };
    static const int leap[IAS_MAX_MONTH] = { 0, 31, 60, 91, 121, 152, 182, 213,
                                  244, 274, 305, 335 };
    int n;

    if (ias_math_is_leap_year(year))
    {
        if (doy > IAS_DAYS_IN_LEAP_YEAR || doy < 1)
        {
            IAS_LOG_ERROR("Day of year incorrect");
            return ERROR;
        }
        for (n = 0 ; n < IAS_MAX_MONTH ; n++)
        {
            if (leap[n] >= doy)
                break;
        }
        n--;
        *day = doy - leap[n];
        *mon = n + 1;
    }
    else
    {
        if (doy > IAS_DAYS_IN_YEAR || doy < 1)
        {
            IAS_LOG_ERROR ("Day of year incorrect");
            return ERROR;
        }
        for (n = 0 ; n < IAS_MAX_MONTH ; n++)
        {
            if (noleap[n] >= doy)
                break;
        }
        n--;
        *day = doy - noleap[n];
        *mon = n + 1;
    }

    return SUCCESS;
}
