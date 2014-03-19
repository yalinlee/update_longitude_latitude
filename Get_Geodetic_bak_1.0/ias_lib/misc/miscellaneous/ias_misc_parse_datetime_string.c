/*************************************************************************
 NAME:                   ias_misc_parse_datetime_string

 PURPOSE:  Parses a date/time string into its constituent components

 RETURNS:  Integer status code of SUCCESS or ERROR

 NOTES:    The date/time string is expected to be consistent with the
           accepted format(s) supported by the IAS, i.e. either

           YYYY[-]mm[-]dd[tT]HH[:]MM[:]SS     (CPF/BPF-format)

           YYYY[:]ddd[:]HH[:]MM[:]SS.SSSSSSS  (L0R-format)

           If other formats need to be considered, this will need to
           be modified.
***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>          /* Error code returned from strtol() and strtod() */
#include "ias_const.h"
#include "ias_logging.h"
#include "ias_math.h"
#include "ias_miscellaneous.h"
#include "ias_structures.h"
#include "ias_types.h"

#define MIN_MONTH 1
#define MAX_MONTH 12
#define MIN_DAY 1
#define MAX_DAY 365
#define MAX_DAY_LEAP 366
#define MIN_TIME 0
#define MAX_HOUR 23 
#define MAX_MINUTE 59
#define MAX_SECOND 60 /* The check is for less than this value */
#define MAX_SECONDS_DIGITS 20

static int is_string_numeric
(
    char *input_string
)
{
    if (strspn(input_string, "0123456789") == strlen(input_string))
        return TRUE;    
    else
        return FALSE;
}

int ias_misc_parse_datetime_string
(
    const char *datetime_string,            /* I: Date/time string */
    IAS_DATETIME_FORMAT_TYPE format_type,   /* I: Either L0R-format or
                                                  CPF/BPF/RLUT-format */
    IAS_DATETIME *time                      /* I/O: Decomposed date/time
                                                    data */
)
{
    char year_string[5];                    /* Date/time substrings */
    char month_string[3];
    char day_of_month_string[3];
    char day_of_year_string[4];
    char hour_string[3];
    char minute_string[3];
    char second_string[MAX_SECONDS_DIGITS]; /* Supports fractional seconds */
    long month_days[13] =                   /* Days in each month */
        { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    long year;                              /* Date/time values */
    long month;
    long day_of_month;
    long day_of_year;
    long hour;
    long minute;
    double second;

    int hour_offset;                        /* Offsets to time components
                                               and component separator
                                               characters */
    int day_of_year_offset;
    int day_of_year_separator_offset;
    int month_offset;
    int month_separator_offset;
    int day_of_month_offset;
    int day_of_month_separator_offset;
    int hour_separator_offset;
    int minute_offset;
    int minute_separator_offset;
    int second_offset;
    int seconds_length;
    int status;

    /* Do we have a valid format ID */
    if ((format_type != IAS_DATETIME_L0R_FORMAT)
            && (format_type != IAS_DATETIME_CPF_FORMAT))
    {
        IAS_LOG_ERROR("Invalid date/time string format identifier");
        return ERROR;
    }

    /* Some sanity checks on the input date/time string to make sure it's
       in an expected format. Make sure only numbers are in the number
       fields, and there's only one 'valid' separator character separating
       them.  Again, the valid formats for a date/time string input to this
       routine are:

       YYYY[sep]mm[sep]dd[sep]HH[sep]MM[sep]SS.SSSSS, or

       YYYY[:]ddd[:]HH[:]MM[:]SS.SSSSS

       The year/month/day data can be separated by either a '-' or a ':'.
       The full date can be separated from the full time by a ':' or a 'T'.
       The hour/minute/second data can only be separated by a ':'.*/
    memset(second_string, 0, sizeof(second_string));

    /* The year format is the same in both cases and is always 4 digits */
    if ((strspn(datetime_string, "0123456789") != 4))
    {
        IAS_LOG_ERROR("Malformed year in date/time string %s", datetime_string);
        return ERROR;
    }
    strncpy(year_string, datetime_string, sizeof(year_string));
    year_string[4] = '\0';

    if (strspn(&datetime_string[4], "-:") != 1)
    {
        IAS_LOG_ERROR("Invalid separator %c in date/time string %s",
                datetime_string[4], datetime_string);
        return ERROR;
    }

    /* After the year, account for the differences. For CPF/BPF-format
       strings, parse the the day and the month. For L0R-format
       strings, parse the day of the year. */
    if (format_type == IAS_DATETIME_L0R_FORMAT)
    {
        /* Here we parse out just day of year; expect 3 digits at most */
        day_of_year_offset = 5;
        day_of_year_separator_offset = strcspn(&datetime_string[5], "-:")
            + day_of_year_offset;
        if (((day_of_year_separator_offset - day_of_year_offset) == 0)
                || ((day_of_year_separator_offset - day_of_year_offset) > 3))
        {
            IAS_LOG_ERROR("Malformed day-of-year in date/time string %s",
                    datetime_string);
            return ERROR;
        }
        strncpy(day_of_year_string, &datetime_string[day_of_year_offset], 
                sizeof(day_of_year_string));
        day_of_year_string[day_of_year_separator_offset - day_of_year_offset]
            = '\0';

        if (is_string_numeric(day_of_year_string) == FALSE)
        {
            IAS_LOG_ERROR("Malformed day-of-year in date/time string %s",
                    datetime_string);
            return ERROR;
        }

        hour_offset = day_of_year_separator_offset + 1;
    }
    else
    {
        /* Parse out month and then day of month */
        /* Parse month; expect 2 digits at most */
        month_offset = 5;
        month_separator_offset = strcspn(&datetime_string[month_offset], "-:")
            + month_offset;
        if (((month_separator_offset - month_offset) == 0)
                || ((month_separator_offset - month_offset) > 2))
        {
            IAS_LOG_ERROR("Malformed month in date/time string %s",
                    datetime_string);
            return ERROR;
        }
        strncpy(month_string, &datetime_string[month_offset], 
                sizeof(month_string));
        month_string[month_separator_offset - month_offset] = '\0';

        if (is_string_numeric(month_string) == FALSE)
        {
            IAS_LOG_ERROR("Malformed month in date/time string %s",
                    datetime_string);
            return ERROR;
        }

        /* Parse day of month; expect 2 digits at most */
        day_of_month_offset = month_separator_offset + 1;

        day_of_month_separator_offset
            = strcspn(&datetime_string[day_of_month_offset], ":Tt")
            + day_of_month_offset;

        if (((day_of_month_separator_offset - day_of_month_offset) == 0)
                || ((day_of_month_separator_offset - day_of_month_offset) > 2))
        {
            IAS_LOG_ERROR("Malformed day of month in date/time string %s",
                    datetime_string);
            return ERROR;
        }

        strncpy(day_of_month_string, &datetime_string[day_of_month_offset], 
                sizeof(day_of_month_string));
        day_of_month_string[day_of_month_separator_offset - day_of_month_offset]
            = '\0';

        if (is_string_numeric(day_of_month_string) == FALSE)
        {
            IAS_LOG_ERROR("Malformed day of month in date/time string %s",
                    datetime_string);
            return ERROR;
        }

        if (strspn(&datetime_string[day_of_month_separator_offset], ":Tt") != 1)
        {
            IAS_LOG_ERROR("Invalid separator %c in date/time string %s",
                    datetime_string[10], datetime_string);
            return ERROR;
        }
        hour_offset = day_of_month_separator_offset + 1;
    }

    /* Now that we have the time component offsets determined for the
       specified format type, parse the time components */
    /* Parse hour; expect two digits at most */
    if ((strspn(&datetime_string[hour_offset], "0123456789") == 0)
            || (strspn(&datetime_string[hour_offset], "0123456789") > 2))
    {
        IAS_LOG_ERROR("Malformed hour in date/time string %s", datetime_string);
        return ERROR;
    }

    hour_separator_offset = strcspn(&datetime_string[hour_offset], ":")
        + hour_offset;

    strncpy(hour_string, &datetime_string[hour_offset], sizeof(hour_string));
    hour_string[hour_separator_offset - hour_offset] = '\0';
    if (is_string_numeric(hour_string) == FALSE)
    {
        IAS_LOG_ERROR("Malformed hour in date/time string %s", datetime_string);
        return ERROR;
    }

    if (strspn(&datetime_string[hour_separator_offset], ":") != 1)
    {
        IAS_LOG_ERROR("Invalid separator %c in date/time string %s",
                datetime_string[hour_separator_offset], datetime_string);
        return ERROR;
    }

    /* Parse minutes; expect two digits at most */
    minute_offset = hour_separator_offset + 1;
    minute_separator_offset = strcspn(&datetime_string[minute_offset], ":")
        + minute_offset;
    if ((strspn(&datetime_string[minute_offset], "0123456789") == 0)
            || (strspn(&datetime_string[minute_offset], "0123456789") > 2))
    {
        IAS_LOG_ERROR("Malformed minute in date/time string %s",
                datetime_string);
        return ERROR;
    }

    strncpy(minute_string, &datetime_string[minute_offset],
            sizeof(minute_string));
    minute_string[minute_separator_offset - minute_offset] = '\0';

    if (is_string_numeric(minute_string) == FALSE)
    {
        IAS_LOG_ERROR("Malformed minute in date/time string %s",
                datetime_string);
        return ERROR;
    }

    if (strspn(&datetime_string[minute_separator_offset], ":") != 1)
    {
        IAS_LOG_ERROR("Invalid separator %c in date/time string %s",
                datetime_string[hour_separator_offset], datetime_string);
        return ERROR;
    }

    /* Parse seconds */
    second_offset = minute_separator_offset + 1;
    seconds_length = strspn(&datetime_string[second_offset], "0123456789.");

    if ((seconds_length != 0) && (seconds_length < MAX_SECONDS_DIGITS))
    {
        strncpy(second_string, &datetime_string[second_offset], 
                sizeof(second_string));
        second_string[seconds_length] = '\0';
    }
    else
    {
        IAS_LOG_ERROR("Malformed second in date/time string %s",
                datetime_string);
        return ERROR;
    }

    if (strspn(second_string, "0123456789.") != strlen(second_string))
    {
        IAS_LOG_ERROR("Malformed second in date/time string %s",
                datetime_string);
        return ERROR;
    }

    /* The whole string was in a valid format, so convert the date/time
       substrings to numbers and store them. After each call, check to
       see whether the conversion produced a valid number. Start with
       the date information */
    errno = 0;
    year = strtol(year_string, (char **)NULL, 10);
    if ((errno == EINVAL) || (errno == ERANGE))
    {
        IAS_LOG_ERROR("Converting year to integer value");
        return ERROR;
    }
    time->year = (int)year;

    errno = 0;
    if (format_type == IAS_DATETIME_L0R_FORMAT)
    {
        day_of_year = strtol(day_of_year_string, (char **)NULL, 10);
        if ((errno == EINVAL) || (errno == ERANGE))
        {
            IAS_LOG_ERROR("Converting day of year to integer value");
            return ERROR;
        }

        /* Check the day of year range */
        if ( ias_math_is_leap_year(year) )
        {
            if ( day_of_year < MIN_DAY || day_of_year > MAX_DAY_LEAP )
            {
                IAS_LOG_ERROR("Day of year out of range - leap year");
                return ERROR;
            }
        }
        else
        {
            if ( day_of_year < MIN_DAY || day_of_year > MAX_DAY )
            {
                IAS_LOG_ERROR("Day of year out of range");
                return ERROR;
            }
        }

        time->day_of_year = (int)day_of_year;

        /* Get the corresponding month and day of the month and fill these in */
        status = ias_math_convert_doy_to_month_day(time->day_of_year,
                time->year, &(time->month), &(time->day_of_month));
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Retrieving month and day-of-month information "
                    "from corresponding day-of-year");
            return ERROR;
        }
    }
    else
    {
        month = strtol(month_string, (char **)NULL, 10);

        /* Check the month range */
        if ( month < MIN_MONTH || month > MAX_MONTH )
        {
            IAS_LOG_ERROR("Month out of range");
            return ERROR;
        }

        day_of_month = strtol(day_of_month_string, (char **)NULL, 10);
        if ((errno == EINVAL) || (errno == ERANGE))
        {
            IAS_LOG_ERROR("Converting month and/or day of month to integer "
                    "values");
            return ERROR;
        }

        /* Set number of days in Feb. for leap year */
        if ( ias_math_is_leap_year(year) )
           month_days[2] = 29;
        /* Check the day of month range */
        if ( day_of_month < MIN_MONTH || day_of_month > month_days[month] )
        {
            IAS_LOG_ERROR("Day of month out of range");
            return ERROR;
        }

        time->month = (int)month;
        time->day_of_month = (int)day_of_month;

        /* Get the corresponding day-of-year and fill this in */
        status = ias_math_convert_month_day_to_doy(time->month,
                time->day_of_month, time->year, &(time->day_of_year));
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Retrieving day-of-year information from "
                    "corresponding month and day-of-month");
            return ERROR;
        }
    }

    /* Now convert the time information */
    errno = 0;
    hour = strtol(hour_string, (char **)NULL, 10);

    /* Check the hour range */
    if ( hour < MIN_TIME || hour > MAX_HOUR )
    {
        IAS_LOG_ERROR("Hour out of range");
        return ERROR;
    }

    minute = strtol(minute_string, (char **)NULL, 10);
    if ((errno == EINVAL) || (errno == ERANGE))
    {
        IAS_LOG_ERROR("Converting hour and/or minute to integer value ");
        return ERROR;
    }

    /* Check the minute range */
    if ( minute < MIN_TIME || minute > MAX_MINUTE )
    {
        IAS_LOG_ERROR("Minute out of range");
        return ERROR;
    }

    time->hour = (int)hour;
    time->minute = (int)minute;

    /* Convert the seconds component, and check if an error code was set */
    errno = 0;
    second = strtod(second_string, (char **)NULL);
    if ((errno == EINVAL) || (errno == ERANGE))
    {
        IAS_LOG_ERROR("Converting second to floating point value");
        return ERROR;
    }

    /* Check the second range */
    if ( second < MIN_TIME || second >= MAX_SECOND )
    {
        IAS_LOG_ERROR("Second out of range");
        return ERROR;
    }

    time->second = second;

    /* Done */
    return SUCCESS;
}
