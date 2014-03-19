package ias_date_utils;

###############################################################################
#
# Purpose: Collection of utilities to provide date functions.
#
#     ias_modified_julian_date:
#         Calculate a modified julian date from day, month, and year.
#
#     ias_is_leap:
#         Determine if a specified year is a leap year.
#
#     ias_year_doy_to_year_month_day:
#         Convert a year, and day of year to year, month, and day values.
#
###############################################################################

use strict;
use ias_const;
use Logger;
use Time::Local;
use vars qw(@ISA @EXPORT @EXPORT_OK %EXPORT_TAGS $VERSION);
use Exporter;
@ISA = qw(Exporter);
@EXPORT = qw(
                ias_modified_julian_date
                ias_is_leap
                ias_year_doy_to_year_month_day
                cpf_date_to_seconds
                j2000_days_to_seconds
            );

###############################################################################
#
# Name: ias_modified_julian_date
#
# Purpose: To calculate modified julian date of a given day, month, year.
#
# Returns: The modified julian date of a given day, month, year.
#
# Parameters:
#    $d - day
#    $m - month
#    $y - year
#
############################################################################### 
sub ias_modified_julian_date($$$$)
{
    my ($day, $month, $year, $error_ref) = @_;

    my $mjd;

    # check if date info is reasonable
    # check year 
    if ($year < 1970 || $year > 2050)
    {
        $$error_ref = "Filename calendar year invalid: $year.";
        return ERROR;
    }

    # month
    if ($month < 1 || $month > 12)
    {
        $$error_ref = "Filename month invalid: $month.";
        return ERROR;
    }

    # validate day of month for months with 31 days
    if (($month == 1 || $month == 3 || $month == 5 || $month == 7 
        || $month == 8 || $month == 10 || $month == 12) 
        && ($day < 1 || $day > 31))
    {
        $$error_ref = "Filename day of month invalid for month with 31 days.";
        return ERROR;
    }

    # validate day of month for months with 30 days
    if (($month == 4 || $month == 6 || $month == 9 || $month == 11)
        && ($day < 1 || $day > 30))
    {
        $$error_ref = "Filename day of month invalid for month with 30 days.";
        return ERROR;
    }

    # validate month/day of february and leap year
    if ($month == 2)
    {
       if (ias_is_leap($year))
       {
            if ($day < 1 || $day > 29)
            {
                $$error_ref = "Filename day of month invalid for month of "
                    . "February leap year.";
               return ERROR;
            }
       }
       elsif ($day < 1 || $day > 28)
       {
              $$error_ref = "Filename day of month invalid for month of "
                . "February year.";
              return ERROR;
       }
    }

    $mjd = int(1461 * ($year + int(($month - 14) / 12) + 4800) / 4) 
           + int(367 * ($month - 12 * int(($month - 14) / 12) - 2) / 12) 
           - int(3 * int(($year + int(($month - 14) / 12) + 4900) / 100) / 4) 
           + $day - 32075 - 2400000.5;

    return int($mjd);
}

###############################################################################
# 
# Name: ias_is_leap
#
# Purpose: Check leap year status return 1 if leap year found
#
# Returns: TRUE if year is a leap year
#          FALSE if year is not a leap year
#
# Parameters: I: An integer representing the year under test
#
###############################################################################
sub ias_is_leap($)
{
  my($year) = @_;
  
  if ($year % 4) 
  {
      return FALSE;
  }
  if (!($year % 100) && (!($year % 400)))
  {
        return TRUE;
  }
  else 
  {
     return TRUE;
  }
}

###############################################################################
#
# Name: ias_year_doy_to_year_month_day
#
# Purpose: Convert a (year, day of year) to (year, month, and day of month).
#
# Parameters: integer: Year
#             integer: Day of year
#
# Returns: integer: Year
#          integer: Month
#          integer: Day of month
#
###############################################################################
sub ias_year_doy_to_year_month_day($$)
{
    # Force the input to integers so we do not have any fractional values
    my $year = int($_[0]);
    my $day_of_year = int($_[1]);

    # Number of days for each month January -- December
    my @days_in_month = (31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31);

    # Adjust the February number of days for leap year
    if (ias_is_leap($year))
    {
        $days_in_month[1] = 29;
    }

    # Subtract off months until the day_of_year is smaller than the number of
    # days in the month, making it day_of_month.  month is the zero-based
    # value for the month
    my $month = 0;
    while ($day_of_year > $days_in_month[$month])
    {
        $day_of_year -= $days_in_month[$month];
        $month++;
    }
    my $day_of_month = $day_of_year;

    # Adjust for end of month condition
    if ($day_of_month == 0)
    {
        $day_of_month = 1;
        $month++;
    }

    # Return the year, month, and day of month values
    # month is converted from zero-based to one-based
    return ($year, $month + 1, $day_of_month);
}

###############################################################################
#
# Name: cpf_date_to_seconds
#
# Purpose: Convert a CPF format date string to seconds
#
# Parameters: string: Date string
#
# Returns: integer: Seconds
#
###############################################################################
sub cpf_date_to_seconds($)
{
    my $in_string = $_[0];

    # cpf date format YYYY-MM-DDTHH:MM:SSZ

    my @fields = undef;

    @fields = split('T', $in_string);

    my $date_string = $fields[0];
    my $time_string = $fields[1];

    @fields = split('-', $date_string);
    my $year_string = $fields[0];
    my $month_string = $fields[1];
    my $day_string = $fields[2];

    @fields = split(':', $time_string);
    my $hours_string = $fields[0];
    my $minutes_string = $fields[1];
    my $seconds_string = $fields[2];


    my $time = timelocal($seconds_string, $minutes_string, $hours_string,
                            $day_string, $month_string - 1, $year_string);

    return $time;
}

###############################################################################
#
# Name: j2000_days_to_seconds
#
# Purpose: Convert a j2000 day to seconds
#
# Parameters: string: Date string
#
# Returns: integer: Seconds
#
###############################################################################
sub j2000_days_to_seconds($)
{
    my $j2000_days = $_[0];
    my $pre_2000_seconds = timegm(0, 0, 0, 1, 0, 2000);
    my $seconds_since_2000 = $j2000_days * 86400;
    return ($seconds_since_2000 + $pre_2000_seconds);
}
