/******************************************************************************
NAME: ias_math_init_leap_seconds
      ias_math_get_leap_seconds
      ias_math_convert_year_doy_sod_to_j2000_seconds
      ias_math_convert_j2000_seconds_to_year_doy_sod

PURPOSE: Package of routines to convert between J2000 TAI time (in seconds) and
         UTC year, day of year, and seconds of day date/time notation.

NOTES:
    - UTC time has leap seconds "inserted" periodically to correct for the
      slowing rotation of the Earth.  Seconds since TAI J2000 is a continuous
      count of seconds since J2000 without any leap seconds.  So, when doing
      conversions between UTC and seconds from J2000, the correct number of
      leap seconds needs to be applied.  This initialization routine
      initializes the number of leap seconds once for this module under the
      assumption that the UTC <-> seconds from J2000 conversions that are
      done will not cross a leap second.  This is a safe assumption for
      most applications of this module.  The init routine is allowed to be
      called more than once, but never with a different time unless the
      clear routine is called first.
    - Another way of looking at the relationship between UTC and TAI is that 
      since UTC has extra seconds inserted relative to TAI, a clock based on
      TAI time is ahead of a UTC clock.
    - Assumes imaging will not occur during a leap second

******************************************************************************/
#include <math.h>
#include "ias_math.h"
#include "ias_const.h"
#include "ias_logging.h"

/* Define the J2000 TAI epoch in terms of UTC */
#define J2_EPOCH_YEAR  2000
#define J2_EPOCH_DOY   1
#define J2_EPOCH_SOD   43135.816

/* Global static variable to save the initialized number of leap seconds to
   correct for. Calculated once, used in all routines as converting from one
   time system to another. */
static int leap_seconds_adjustment = 0;
static int leap_init_flag = 0;

/* Local routine to support the external routines provided */
static int get_leap_seconds_at_year_doy_sod
(
    int year,               /* I: Year to calculate leap seconds */
    int doy,                /* I: Day of year in calculation */
    double sod,             /* I: Seconds of day */
    int time_is_tai,        /* I: Flag that indicates the reference time is in
                                  TAI and therefore needs to have leap seconds
                                  applied to use the leap second table that is
                                  in UTC */
    const IAS_MATH_LEAP_SECONDS_DATA *cpf_leap_seconds,
                            /* I: Leap seconds info from CPF */
    int *num_leap_seconds   /* O: Num relevant leap seconds from CPF info */
);

/*************************************************************************
Name: ias_math_init_leap_seconds

Purpose: Initializes leap seconds at a point in time, provided in total
    seconds since J2000 epoch.

Returns:
    SUCCESS if leap seconds successfully calculated and initialized
    ERROR if unsuccessful at determining leap seconds and initializing

**************************************************************************/
int ias_math_init_leap_seconds
(
    double j2secs,  /* I: seconds since J2000 to use for initializing the
                          number of leap seconds */
    const IAS_MATH_LEAP_SECONDS_DATA *cpf_leap_seconds
                    /* I: Leap seconds info from CPF */
)
{
    double time[3]; /* For conversion of total seconds to year, doy, sod */
    double sod;     /* Seconds Of Day */
    int year;
    int doy;        /* Day Of Year */
    int j2_epoch_leap_secs;
    int curr_leap_secs;
    int status;
    int leap_seconds_set_before = 0;
    int saved_adjustment = leap_seconds_adjustment;

    /* Set a flag if this is a call to init where init has been done before. */
    if (leap_init_flag)
        leap_seconds_set_before = 1;

    /* Compute the number of leap seconds at the epoch time */
    year = J2_EPOCH_YEAR;
    doy = J2_EPOCH_DOY;
    sod = J2_EPOCH_SOD;
    status = get_leap_seconds_at_year_doy_sod(year, doy, sod, 0,
                cpf_leap_seconds, &j2_epoch_leap_secs);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Initializing leap seconds. Computing leap seconds "
                "at epoch time");
        return ERROR;
    }

    /* Compute the number of leap seconds at the given time */
    /* Set the leap seconds initialized flag early, so the error condition
       in the convert routine is not triggered.  Also zero out the current
       leap seconds adjustment so it doesn't affect the results if this is
       called multiple times. */
    leap_init_flag = 1;
    leap_seconds_adjustment = 0;
    status = ias_math_convert_j2000_seconds_to_year_doy_sod(j2secs, time);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Initializing leap seconds. Converting given J2000 "
                "seconds to date format");
        /* Unset the flag since initialization did not complete successfully */
        leap_init_flag = 0;
        return ERROR;
    }

    /* restore the original adjustment */
    leap_seconds_adjustment = saved_adjustment;

    /* Finish computation of leap seconds at the given time.  Note that this
       time is TAI and calls the routine with that indication. */
    year = (int)(time[0] + 0.5);
    doy = (int)(time[1] + 0.5);
    sod = time[2];
    status = get_leap_seconds_at_year_doy_sod(year, doy, sod, 1,
                cpf_leap_seconds, &curr_leap_secs);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Initializing leap seconds. Computing leap seconds "
                "at given time");
        /* Unset the flag since initialization did not complete successfully */
        leap_init_flag = 0;
        return ERROR;
    }

    /* If previously initialized, confirm that this new time being initialized
       will result in the same leap seconds as previously initialized to */
    if (leap_seconds_set_before)
    {
        if ((curr_leap_secs - j2_epoch_leap_secs) != leap_seconds_adjustment)
        {
            IAS_LOG_ERROR("Subsequent leap seconds initialization resulted in "
                    "different leaps seconds adjustment than previously "
                    "determined");
            return ERROR;
        }

        /* Otherwise, we're done */
        return SUCCESS;
    }

    leap_seconds_adjustment = curr_leap_secs - j2_epoch_leap_secs;

    IAS_LOG_DEBUG("Leap seconds adjustment found is %d",
            leap_seconds_adjustment);

    return SUCCESS;
}

/*************************************************************************
Name: ias_math_init_leap_seconds_from_UTC_time

Purpose: Initializes leap seconds using a UTC time.

Returns:
    SUCCESS if leap seconds successfully calculated and initialized
    ERROR if unsuccessful at determining leap seconds and initializing

**************************************************************************/
int ias_math_init_leap_seconds_from_UTC_time
(
    const double *ref_time, /* I: Year, day of year, seconds of day to use to
                                  intialize the leap seconds */
    const IAS_MATH_LEAP_SECONDS_DATA *cpf_leap_seconds
                            /* I: Leap seconds info from CPF */
)
{
    double sod;     /* Seconds Of Day */
    int year;
    int doy;        /* Day Of Year */
    int j2_epoch_leap_secs;
    int curr_leap_secs;
    int status;
    int leap_seconds_set_before = 0;

    /* Set a flag if this is a call to init where init has been done before. */
    if (leap_init_flag)
        leap_seconds_set_before = 1;

    /* Compute the number of leap seconds at the epoch time */
    year = J2_EPOCH_YEAR;
    doy = J2_EPOCH_DOY;
    sod = J2_EPOCH_SOD;
    status = get_leap_seconds_at_year_doy_sod(year, doy, sod, 0,
                cpf_leap_seconds, &j2_epoch_leap_secs);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Initializing leap seconds. Computing leap seconds "
                "at epoch time");
        return ERROR;
    }


    /* Get the leap seconds at the reference time */
    year = (int)(ref_time[0] + 0.5);
    doy = (int)(ref_time[1] + 0.5);
    sod = ref_time[2];
    status = get_leap_seconds_at_year_doy_sod(year, doy, sod, 0,
                cpf_leap_seconds, &curr_leap_secs);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Initializing leap seconds. Computing leap seconds "
                "at reference year %d, day %d, seconds %f", year, doy, sod);
        /* Unset the flag since initialization did not complete successfully */
        leap_init_flag = 0;
        return ERROR;
    }

    /* If previously initialized, confirm that this new time being initialized
       will result in the same leap seconds as previously initialized to */
    if (leap_seconds_set_before)
    {
        if ((curr_leap_secs - j2_epoch_leap_secs) != leap_seconds_adjustment)
        {
            IAS_LOG_ERROR("Subsequent leap seconds initialization resulted in "
                    "different leaps seconds adjustment than previously "
                    "determined");
            return ERROR;
        }

        /* Otherwise, we're done */
        return SUCCESS;
    }

    leap_init_flag = 1;

    leap_seconds_adjustment = curr_leap_secs - j2_epoch_leap_secs;

    IAS_LOG_DEBUG("Leap seconds adjustment found is %d",
            leap_seconds_adjustment);

    return SUCCESS;
}

/*************************************************************************
Name: ias_math_clear_leap_seconds

Purpose: Clears a previous initialization of the number of leap seconds in
    case an application wants to use this with widely different times.
    Originally, this was intended to just be used within the context of a
    single collection that should never cross a leap second event.  However,
    a use was found for it that spans collections, so this ability was added.

Returns: nothing

**************************************************************************/
void ias_math_clear_leap_seconds()
{
    leap_seconds_adjustment = 0;
    leap_init_flag = 0;
}

/*************************************************************************
Name: ias_math_get_leap_second_adjustment

Purpose: Accessor function to get the leap second adjustment being applied
    since J2000.

Returns:
    SUCCESS if leap seconds initialized and set to return parameter
    ERROR if leap seconds have not been initialized yet

Notes:
    Returns, through a parameter, number of leap seconds, if initialized

**************************************************************************/
int ias_math_get_leap_second_adjustment
(
    int *seconds    /* O: the leap seconds adjustment previously initialized */
)
{
    /* Return ERROR if not initialized */
    if (!leap_init_flag)
    {
        IAS_LOG_ERROR("Leap seconds not initialized");
        *seconds = 0;
        return ERROR;
    }
    else
        *seconds = leap_seconds_adjustment;

    return SUCCESS;
}

/*************************************************************************
Name: ias_math_convert_year_doy_sod_to_j2000_seconds

Purpose: Converts a UTC date/time value in calendar year, day of year, seconds
    of day format to a value of TAI seconds since J2000.

Returns:
    SUCCESS
    ERROR

Notes:
    - This routine applies the number of leaps seconds to the conversion
      that are determined by the time passed to the init routine.

**************************************************************************/
int ias_math_convert_year_doy_sod_to_j2000_seconds
(
    const double *time, /* I: Year, day of year, seconds of day to convert to
                              J2000 epoch time */
    double *j2secs      /* O: TAI seconds from J2000 */
)
{
    int year_iterator;
    int calendar_year;
    int days_from_j2000;
    double total_secs;
    double secs_from_begin_calendar_year;

    /* Error if the leap seconds have not been initialized yet. */
    if (!leap_init_flag)
    {
        IAS_LOG_ERROR("Leap seconds not initialized");
        return ERROR;
    }

    /* Round the year to the nearest whole year in case it contains a
       fractional portion */
    calendar_year = (int)(time[0] + 0.5);
    secs_from_begin_calendar_year = (time[1] - 1.0) * IAS_SEC_PER_DAY + time[2]
        - ((double)(J2_EPOCH_DOY - 1) * IAS_SEC_PER_DAY + J2_EPOCH_SOD)
        + leap_seconds_adjustment;

    /* Count the number of days in the years from J2000 epoch year to
       the given year */
    days_from_j2000 = 0;
    year_iterator = J2_EPOCH_YEAR;
    while (year_iterator < calendar_year)
    {
        /* Year of reference is J2000 epoch (2000). This case is for
           conversions of dates after the J2000 epoch (normal). */
        if (ias_math_is_leap_year(year_iterator))
            days_from_j2000 += IAS_DAYS_IN_LEAP_YEAR;
        else
            days_from_j2000 += IAS_DAYS_IN_YEAR;
        year_iterator++;
    }
    while (year_iterator > calendar_year)
    {
        /* Year of reference is J2000 epoch (2000). This case is for
           conversions of dates before the J2000 epoch (usually CalVal and
           testing only). */
        if (ias_math_is_leap_year(calendar_year))
            days_from_j2000 -= IAS_DAYS_IN_LEAP_YEAR;
        else
            days_from_j2000 -= IAS_DAYS_IN_YEAR;
        calendar_year++;
    }

    /* Calculate the total seconds from J2000 as the seconds in each day since
       J2000 plus the seconds calculated previously since beginning of current
       year */
    total_secs = secs_from_begin_calendar_year
        + (double)(days_from_j2000 * IAS_SEC_PER_DAY);

    *j2secs = total_secs;

    return SUCCESS;
}

/*************************************************************************
Name: ias_math_convert_j2000_seconds_to_year_doy_sod

Purpose: Converts a value of total seconds from J2000 (TAI) to calendar year,
    day of year, and seconds of day (UTC). Assumes the total seconds from
    J2000 contains the leap seconds, so will remove those, as appropriate for
    the new format converting to.

Returns:
    nothing

Notes:
    - This routine applies the number of leaps seconds to the conversion
      that are determined by the time passed to the init routine.

**************************************************************************/
int ias_math_convert_j2000_seconds_to_year_doy_sod
(
    double j2secs,  /* I: TAI seconds since J2000 to convert to Year, day of
                       year, seconds of day */
    double *time    /* O: UTC Year, day of year, seconds of day output array */
)
{
    double j2_epoch_total_secs;
    int year;
    int full_days_from_j2000;
    int days_remaining;
    int days_in_year;

    /* Error if leap seconds have not been initialized. */
    if (!leap_init_flag)
    {
        IAS_LOG_ERROR("Leap seconds not initialized");
        return ERROR;
    }

    /* initialize the year to the j2000 base year */
    year = J2_EPOCH_YEAR;

    /* J2000 seconds from the satellite is based on J2000 epoch being noon on
       Jan 1, 2000. For UTC time, need to know total seconds since midnight on
       Jan 1, 2000. */
    /* When called from ias_math_init_leap_seconds,
       leap_seconds_adjustment is 0 */
    j2_epoch_total_secs = j2secs
        + ((double)(J2_EPOCH_DOY - 1) * IAS_SEC_PER_DAY + J2_EPOCH_SOD)
        - leap_seconds_adjustment;

    /* find out how many complete days are in our seconds since j2000 */
    full_days_from_j2000 = (int)floor(j2_epoch_total_secs / IAS_SEC_PER_DAY);

    /* calculate seconds of day by taking out our total days worth of seconds
       from original seconds since j2000 passed in */
    time[2] = j2_epoch_total_secs
        - (double)(full_days_from_j2000 * IAS_SEC_PER_DAY);

    /* total days of the j2000 epoch year */
    if (ias_math_is_leap_year(year))
        days_in_year = IAS_DAYS_IN_LEAP_YEAR;
    else
        days_in_year = IAS_DAYS_IN_YEAR;

    /* Now going to count up how many years of days we have */
    days_remaining = full_days_from_j2000;
    while (days_remaining >= days_in_year)
    {
        /* Year of reference is J2000 epoch (2000). This case is for
           conversions of dates after the J2000 epoch (normal). */
        days_remaining -= days_in_year;
        year++;
        if (ias_math_is_leap_year(year))
            days_in_year = IAS_DAYS_IN_LEAP_YEAR;
        else
            days_in_year = IAS_DAYS_IN_YEAR;
    }
    while (days_remaining < 0)
    {
        /* Year of reference is J2000 epoch (2000). This case is for
           conversions of dates before the J2000 epoch (usually CalVal and
           testing only). */
        year--;
        if (ias_math_is_leap_year(year))
            days_in_year = IAS_DAYS_IN_LEAP_YEAR;
        else
            days_in_year = IAS_DAYS_IN_YEAR;
        days_remaining += days_in_year;
    }

    /* Save the final year and day of current year */
    time[0] = (double)year;
    time[1] = (double)(days_remaining + 1);

    return SUCCESS;
}

/*************************************************************************
Name:  get_leap_seconds_at_year_doy_sod

Purpose: For a given year and day of year, spins through the leap seconds
    from the CPF and adds up the count of how many of them occurred prior to
    and up to (including) the current year and day of year.

Returns:
    Returns, through a parameter, the total leap seconds found.

Notes:
    - This is based this on an assumption that the leap seconds occur at
      midnight of the day provided (so no need for the seconds of day info)

    - The time_is_tai parameter indicates that the provided reference 
      year/doy/sod is in TAI and needs to be adjusted for the leap seconds
      found since the leap second times are UTC.  Note that this assumes the
      leap seconds in the CPF are from oldest to newest.

**************************************************************************/
static int get_leap_seconds_at_year_doy_sod
(
    int year,               /* I: Year to calculate leap seconds */
    int doy,                /* I: Day of year in calculation */
    double sod,             /* I: Seconds of day */
    int time_is_tai,        /* I: Flag that indicates the reference time is in
                                  TAI and therefore needs to have leap seconds
                                  applied to use the leap second table that is
                                  in UTC */
    const IAS_MATH_LEAP_SECONDS_DATA *cpf_leap_seconds,
                            /* I: Leap seconds info from CPF */
    int *num_leap_seconds   /* O: Num relevant leap seconds from CPF info */
)
{
    int count;
    int local_leap_seconds = 0;
    int calendar_month;
    int calendar_day_of_month;
    int status;

    /* Convert the year, doy, format provided to year, month, day format
       to compare to year, month, day format leap seconds are recorded by */
    status = ias_math_convert_doy_to_month_day(doy, year, &calendar_month,
                    &calendar_day_of_month);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Converting year and day of year to calendar date");
        return ERROR;
    }

    /* Check each of the leap seconds listed in the CPF */
    for (count = 0; count < cpf_leap_seconds->leap_seconds_count; count++)
    {
        int adjustment = 0;
        if (cpf_leap_seconds->leap_years[count] < year)
        {
            adjustment = cpf_leap_seconds->num_leap_seconds[count];
        }
        else if (cpf_leap_seconds->leap_years[count] == year)
        {
            if (cpf_leap_seconds->leap_months[count] < calendar_month)
            {
                adjustment = cpf_leap_seconds->num_leap_seconds[count];
            }
            else if (cpf_leap_seconds->leap_months[count] == calendar_month
                    && cpf_leap_seconds->leap_days[count]
                    <= calendar_day_of_month)
            {
                adjustment = cpf_leap_seconds->num_leap_seconds[count];
            }
        }

        /* if a leap second was found, adjust the input time if it is TAI */
        if ((adjustment != 0) && time_is_tai)
        {
            double date[3];
            date[0] = year;
            date[1] = doy;
            date[2] = sod;
            ias_math_add_seconds_to_year_doy_sod(-adjustment, date);
            sod = date[2];

            /* if the doy changed, recalculate the calendar month */
            if (date[1] != doy)
            {
                year = (int)(date[0] + 0.5);
                doy = (int)(date[1] + 0.5);

                status = ias_math_convert_doy_to_month_day(doy, year,
                                &calendar_month, &calendar_day_of_month);
                if (status != SUCCESS)
                {
                    IAS_LOG_ERROR(
                        "Converting year and day of year to calendar date");
                    return ERROR;
                }
            }
        }

        local_leap_seconds += adjustment;
    }

    *num_leap_seconds = local_leap_seconds;

     return SUCCESS;
}
