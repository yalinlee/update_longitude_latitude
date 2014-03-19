
package ias_time;

##############################################################################
#    NAME: ias_time.pm
#
# PURPOSE: A time class for providing routines for manipulation and formatting
#          time data.
#
#    NOTE: This is not a complete class, because it only provides the methods
#          that are currently needed.
#
##############################################################################

# System Includes
use strict;
use warnings;
use Time::Local;

use vars qw(@ISA @EXPORT @EXPORT_OK %EXPORT_TAGS $VERSION);

use Exporter;

@ISA = qw(Exporter);
@EXPORT = qw(
              SQL_DATE_FORMAT
              SECONDS_IN_DAY
              IAS_TIME_LOCAL
              IAS_TIME_GMT
              IAS_TIME_FORMAT_CPF
              IAS_TIME_FORMAT_CPF_SERVICE
              IAS_TIME_FORMAT_DATABASE
              IAS_TIME_FORMAT_TIMESTAMP
            );

use constant SQL_DATE_FORMAT => "yyyy-mm-dd hh24:mi:ss";
use constant SECONDS_IN_DAY => 86400;
use constant IAS_TIME_LOCAL => 0;
use constant IAS_TIME_GMT => 1;

use constant IAS_TIME_FORMAT_CPF => 0;
use constant IAS_TIME_FORMAT_CPF_SERVICE => 1;
use constant IAS_TIME_FORMAT_DATABASE => 2;
use constant IAS_TIME_FORMAT_TIMESTAMP => 3;

# An array for converting a zero-based integer month value into the long name
my @month_strings = (
    "January",
    "February",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December"
);

# An array for converting a zero-based integer day value into the long name
my @week_strings = (
    "Sunday",
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday",
);

##############################################################################
#        NAME: new
#
# DESCRIPTION: Creates a new ias_time object using a provided time or it will
#              default to the current time.  The type of the time object is
#              defaulted to local time unless the user specifies GMT.
#
#  PARAMETERS: reference: An optional hash parameter containing the following
#                         elements
#                         - time_value: The time in seconds to initialize with
#                         -  time_type: The type of time object to create,
#                                       either IAS_TIME_LOCAL or IAS_TIME_GMT
#
#     RETURNS: reference: The ias_time object created
#
#    EXAMPLES:
#              To create a current local time object
#              - my $var = ias_time->new();
#              - my $var = ias_time->new({time_value => time,
#                                         time_type => IAS_TIME_LOCAL});
#
#              To create a current GMT time object
#              - my $var = ias_time->new({time_type => IAS_TIME_GMT});
#
##############################################################################
sub ias_time::new
{
    # Grab the number of parameters passed to the constructor
    my $parameter_count = @_;

    # Grab the class designation off of the parameter list
    my $class = shift;

    my $time_type = IAS_TIME_LOCAL;
    my $time_value = time();
    my $input_hash = ();

    # Determine if the user supplied input
    if ($parameter_count > 1)
    {
        $input_hash = shift;
    }

    # If the use provided input parameters
    # Figure out if they are ones we use and assign the values correctly
    if (defined $input_hash)
    {
        if (defined $input_hash->{"time_value"})
        {
            $time_value = $input_hash->{"time_value"};
        }

        if (defined $input_hash->{"time_type"})
        {
            $time_type = $input_hash->{"time_type"};
        }
    }

    # Specify the class variables
    my $self = {
        default_time_type => $time_type,
        time_value => $time_value,
    };

    bless($self, $class);

    return $self;
}

##############################################################################
#        NAME: get_time_elements
#
# DESCRIPTION: Returns the individual time elements converted to 1-based
#              values and 4-digit year
#
#  PARAMETERS: integer: An optional value specifying wether or not to return
#                       GMT or local time values
#                       IAS_TIME_LOCAL or IAS_TIME_GMT
#
#     RETURNS: integer: seconds value or undef on error
#              integer: minutes value or undef on error
#              integer: hours value or undef on error
#              integer: day of month value or undef on error
#              integer: month value or undef on error
#              integer: year value or undef on error
#              integer: day of week value or undef on error
#              integer: day of year value or undef on error
#              integer: is it daylight savings time or undef on error
#
##############################################################################
sub ias_time::get_time_elements
{
    # Grab the number of parameters passed to the constructor
    my $parameter_count = @_;

    # Self object is passed as the first parameter.
    my $self = shift;

    my $time_type = $self->{default_time_type};
    # Grab the type from the parameter instead of the default type.
    if ($parameter_count > 1)
    {
        $time_type = shift;
    }

    # Define the time elements
    my ($second, $minute, $hour,
        $day_of_month, $month, $year,
        $day_of_week, $day_of_year, $is_daylight_savings_time);

    # Extract the time elements from the time methods or return undef for
    # time types not implemented
    if ($time_type == IAS_TIME_LOCAL)
    {
       ($second, $minute, $hour,
        $day_of_month, $month, $year,
        $day_of_week, $day_of_year,
        $is_daylight_savings_time) = localtime($self->{time_value});
    }
    elsif ($time_type == IAS_TIME_GMT)
    {
       ($second, $minute, $hour,
        $day_of_month, $month, $year,
        $day_of_week, $day_of_year,
        $is_daylight_savings_time) = gmtime($self->{time_value});
    }
    else
    {
        return (undef, undef, undef, undef, undef, undef, undef, undef, undef);
    }

    # convert year to 4 digit value
    $year = $year + 1900;
    # convert month to 1 based
    $month = $month + 1;
    # convert day of week to 1 based
    $day_of_week = $day_of_week + 1;
    # convert day of year to 1 based
    $day_of_year = $day_of_year + 1;

    return ($second, $minute, $hour,
            $day_of_month, $month, $year,
            $day_of_week, $day_of_year, $is_daylight_savings_time);
}

##############################################################################
#        NAME: get_string_format
#
# DESCRIPTION: Returns the individual time element
#
#  PARAMETERS: reference: An optional hash parameter containing the following
#                         elements
#                         - string_format: The format of the string to return
#                                          either IAS_TIME_FORMAT_CPF,
#                                          or IAS_TIME_FORMAT_CPF_SERVICE,
#                                          or IAS_TIME_FORMAT_DATABASE,
#                                          or IAS_TIME_FORMAT_TIMESTAMP
#                         -     time_type: The type of time object to use,
#                                          either IAS_TIME_LOCAL or
#                                          IAS_TIME_GMT
#
#     RETURNS: string: The time formatted to the requested string format or
#                      undef
#
#    EXAMPLES:
#              To get a timestamp formatted time string
#              - my $str = $time_var->get_string_format();
#              - my $str = $time_var->new({string_format =>
#                                              IAS_TIME_FORMAT_TIMESTAMP,
#                                          time_type => IAS_TIME_LOCAL});
#
#              To get a CPF formatted time string in GMT
#              - my $str = $time_var->new({string_format =>
#                                              IAS_TIME_FORMAT_CPF,
#                                          time_type => IAS_TIME_GMT});
#
##############################################################################
sub ias_time::get_string_format
{
    # Grab the number of parameters passed to the constructor
    my $parameter_count = @_;

    # Self object is passed as the first parameter.
    my $self = shift;

    # Default these
    my $string_format = IAS_TIME_FORMAT_TIMESTAMP;
    my $time_type = $self->{default_time_type};
    my $input_hash = ();

    # Determine if the user supplied input
    if ($parameter_count > 1)
    {
        $input_hash = shift;
    }

    # If the use provided input parameters
    # Figure out if they are ones we use and assign the values correctly
    if (defined $input_hash)
    {
        if (defined $input_hash->{"string_format"})
        {
            $string_format = $input_hash->{"string_format"};
        }

        if (defined $input_hash->{"time_type"})
        {
            $time_type = $input_hash->{"time_type"};
        }
    }

    # Get the time elements
    my ($second, $minute, $hour,
        $day_of_month, $month, $year,
        $day_of_week, $day_of_year, $is_daylight_savings_time) =
        $self->get_time_elements($time_type);

    # Check if get_time_values failed
    if (not defined $second)
    {
        return undef;
    }

    # Convert to the format
    my $time_string = "";
    if ($string_format == IAS_TIME_FORMAT_CPF)
    {
        #  Format: "yyyy-mm-ddThh24:mi:ss"
        # Example: "2012-04-16T08:31:45"
        $time_string .= sprintf("%04d", $year);
        $time_string .= "-";
        $time_string .= sprintf("%02d", $month);
        $time_string .= "-";
        $time_string .= sprintf("%02d", $day_of_month);
        $time_string .= "T";
        $time_string .= sprintf("%02d", $hour);
        $time_string .= ":";
        $time_string .= sprintf("%02d", $minute);
        $time_string .= ":";
        $time_string .= sprintf("%02d", $second);
    }
    elsif ($string_format == IAS_TIME_FORMAT_CPF_SERVICE)
    {
        #  Format: "yyyy-mm-ddThh24:mi:ssZ"
        # Example: "2012-04-16T08:31:45Z"
        $time_string .= sprintf("%04d", $year);
        $time_string .= "-";
        $time_string .= sprintf("%02d", $month);
        $time_string .= "-";
        $time_string .= sprintf("%02d", $day_of_month);
        $time_string .= "T";
        $time_string .= sprintf("%02d", $hour);
        $time_string .= ":";
        $time_string .= sprintf("%02d", $minute);
        $time_string .= ":";
        $time_string .= sprintf("%02d", $second);
        $time_string .= "Z";
    }
    elsif ($string_format == IAS_TIME_FORMAT_DATABASE)
    {
        #  Format: "yyyy-mm-dd hh24:mi:ss"
        # Example: "2012-04-16 08:31:45"
        $time_string .= sprintf("%04d", $year);
        $time_string .= "-";
        $time_string .= sprintf("%02d", $month);
        $time_string .= "-";
        $time_string .= sprintf("%02d", $day_of_month);
        $time_string .= " ";
        $time_string .= sprintf("%02d", $hour);
        $time_string .= ":";
        $time_string .= sprintf("%02d", $minute);
        $time_string .= ":";
        $time_string .= sprintf("%02d", $second);
    }
    elsif ($string_format == IAS_TIME_FORMAT_TIMESTAMP)
    {
        #  Format: "yyyymmddhh24miss"
        # Example: "20120416083145"
        $time_string .= sprintf("%04d", $year);
        $time_string .= sprintf("%02d", $month);
        $time_string .= sprintf("%02d", $day_of_month);
        $time_string .= sprintf("%02d", $hour);
        $time_string .= sprintf("%02d", $minute);
        $time_string .= sprintf("%02d", $second);
    }
    else
    {
        return undef;
    }

    return $time_string;
}

##############################################################################
#        NAME: diff
#
# DESCRIPTION: Provides the difference between the time associated
#              with the ias_time object and a provided time.
#
#  PARAMETERS: integer: year
#              integer: month
#              integer: day
#              integer: hour
#              integer: minute
#              integer: second
#
#     RETURNS: integer: The time difference in seconds
#
##############################################################################
sub ias_time::diff($$$$$$)
{
    # Self object is passed as the first parameter.
    my $self = shift;

    my ($year, $month, $day, $hour, $minute, $second) = @_;
    
    my $input_time = 0;
    if ($self->{default_time_type} == IAS_TIME_LOCAL)
    {
        $input_time = timelocal($second, $minute, $hour,
                                $day, $month - 1, $year);
    }
    else
    {
        $input_time = timegm($second, $minute, $hour,
                             $day, $month - 1, $year);
    }

    return ($self->{time_value} - $input_time);
}

1;
