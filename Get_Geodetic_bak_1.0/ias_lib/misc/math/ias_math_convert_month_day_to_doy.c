/******************************************************************************
NAME: ias_math_convert_month_day_to_doy      

PURPOSE: Converts month/day for a specific year to day of year.

RETURN VALUE: ERROR or SUCCESS

******************************************************************************/
#include "ias_logging.h"
#include "ias_math.h"

/* Calculate day of year given year, month, and day of month */
int ias_math_convert_month_day_to_doy
(
    int month,  /* I: Month */
    int day,    /* I: Day of month */
    int year,   /* I: Year */
    int *doy    /* O: Day of year */
)
{
    /* Days in month for non-leap years */
    static const int noleap[12] 
        = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    /* Days in month for leap years */
    static const int leap[12]
        = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int i; /* Counter */
    int doy_sum;

    /* Check to make sure month entered is OK */
    if ((month < 1) || (month > 12))
    {
        IAS_LOG_ERROR ("Invalid month requested: %d", month);
        return ERROR;
    }

    /* Calculate day of year */
    doy_sum = 0;
    if (ias_math_is_leap_year(year))
    {
        for (i = 0; i < month - 1; i++)
            doy_sum += leap[i];
    }
    else
    {
        for (i = 0; i < month - 1; i++)
            doy_sum += noleap[i];
    }
    doy_sum += day;

    *doy = doy_sum;

    return SUCCESS;
}
