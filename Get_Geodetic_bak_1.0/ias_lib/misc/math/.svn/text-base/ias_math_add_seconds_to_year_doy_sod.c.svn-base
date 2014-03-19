/******************************************************************************
Name:     ias_math_add_seconds_to_year_doy_sod

Purpose: Given a year, day of year, and seconds of day;
         add more seconds to the date;
         and calculate new year, day of year, and seconds.
         Seconds can be more than a day.

Returns: nothing

******************************************************************************/
#include "ias_const.h"
#include "ias_math.h"

void ias_math_add_seconds_to_year_doy_sod
(
    double seconds,     /* I: Seconds to add to date given */
    double *date        /* IO: Year, DOY, SOD */
)
{
    double sod = date[2];
    int doy = date[1];
    int year = date[0];

    /* add seconds to current date */
    sod += seconds;

    /* check if we've moved to the next day */
    while (sod >= IAS_SEC_PER_DAY)
    {
        int days_in_year = IAS_DAYS_IN_YEAR;
        if (ias_math_is_leap_year((int)year))
            days_in_year = IAS_DAYS_IN_LEAP_YEAR;

        /* decrease the seconds by a day */
        sod -= IAS_SEC_PER_DAY;

        /* advance the day to the next one */
        doy++;

        /* check if we could possibly be at the end of the year */
        if (doy > days_in_year)
        {
            doy -= days_in_year;
            year++;
        }
    }
    /* Check to see if moved to previous day */
    while (sod < 0.0)
    {
        /* move backward one day to get sod to be >= 0 */
        sod += IAS_SEC_PER_DAY;

        /* move to the previous day */
        doy--;

        /* if the day has turned negative, roll back the year */
        if (doy < 1)
        {
            int days_in_year = IAS_DAYS_IN_YEAR;
            year--;

            if (ias_math_is_leap_year((int)year))
                days_in_year = IAS_DAYS_IN_LEAP_YEAR;

            doy += days_in_year;
        }
    }

    /* set the resulting year/doy/sod in the time parameter */
    date[0] = year;
    date[1] = doy;
    date[2] = sod;
}
