/******************************************************************************
FUNCTION: ias_geo_check_start_end_date

PURPOSE: See if the start or end date is invalid

RETURN VALUE:
Type = int
Value    Description
-----    -----------
ERROR    There is a problem with the dates
SUCCESS  The dates are acceptable

*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "ias_logging.h"
#include "ias_math.h"
#include "ias_const.h"
#include "ias_geo.h"
#include "ias_miscellaneous.h"

int ias_geo_check_start_end_date
(
    int isdate,     /* I: start date (YYYYMMDD) */
    int iedate      /* I: end date (YYYYMMDD) */
)
{
    double jdate_begin;   /* Julian date of CPF starting effective time */
    double jdate_end;     /* Julian date of CPF ending effective time */

    int    eDay         = iedate % 100;
    int    eMonth       = (iedate / 100) % 100;
    int    eYear        = iedate / 10000;

    int    sDay         = isdate % 100;
    int    sMonth       = (isdate / 100) % 100;
    int    sYear        = isdate / 10000;

    if (ias_misc_check_year_month_day(sYear, sMonth, sDay) == ERROR)
    {
        IAS_LOG_ERROR("Invalid start date: %d", isdate);
        return ERROR;
    }
    if (ias_misc_check_year_month_day(eYear, eMonth, eDay) == ERROR)
    {
        IAS_LOG_ERROR("Invalid end date: %d", iedate);
        return ERROR;
    }

    /* Obtain the julian dates for the effective dates */
    jdate_begin = ias_math_compute_full_julian_date(sYear, sMonth, sDay, 0);
    jdate_end   = ias_math_compute_full_julian_date(eYear, eMonth, eDay, 0);

    /* Make sure the end date is greater than or equal to the begin date */
    if (jdate_begin > jdate_end)
    {
        IAS_LOG_ERROR("Begin date greater than End date");
        return ERROR;
    }

    return SUCCESS; 
}
