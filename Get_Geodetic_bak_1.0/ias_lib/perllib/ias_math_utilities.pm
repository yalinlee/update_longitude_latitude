
package ias_math_utilities;

##############################################################################
#    NAME: ias_math_utilities.pm
#
# PURPOSE: To provide a collection of math routines.
#
##############################################################################

# System Includes
use strict;
use warnings;
use vars qw(@ISA @EXPORT @EXPORT_OK %EXPORT_TAGS $VERSION);
use Exporter;
use List::Util qw/sum/;

# IAS Includes
use ias_const;

@ISA = qw(Exporter);
@EXPORT = qw(
              ias_get_mean
              ias_get_standard_deviation
              ias_get_rmse
            );

##############################################################################
# Name: ias_get_mean
#
# Purpose: Calculate mean of an array
#
# Input: array of values
#
#    RETURNS:
#        SUCCESS or FAILURE
#        mean
##############################################################################
sub ias_get_mean
{
    my $mean;      # result we are looking for

    my (@sample) = @_;

    if (@sample)
    {
        $mean = sum(@sample) / @sample;
    }
    else # the sample is empty
    {
        my $msg = "Could not compute the mean because the sample is empty";
        print STDERR __FILE__ . ": $msg\n";
        return (FAILURE, undef);
    }

    return (SUCCESS, $mean);
}

##############################################################################
# Name: ias_get_standard_deviation
#
# Purpose: Calculate standard deviation of an array
#
#     Note: The Statistics module is not present.
#
# Input: array of values
#
#    RETURNS:
#        SUCCESS or FAILURE
#        standard deviation
##############################################################################
sub ias_get_standard_deviation
{
    my $standard_deviation;      # result we are looking for
    my $sum_of_squares = 0;      # sum of squared difference from the mean
    my $sample_index;            # index into sample
    my $number_samples;          # number of samples

    my ($mean, @sample) = @_;

    # get number of samples
    $number_samples = @sample;

    # if the sample is less than or equal to 1
    if ($number_samples <= 1)
    {
        my $msg = "Could not compute the standard deviation because the " .
           "sample size is invalid";
        print STDERR __FILE__ . ": $msg\n";
        return (FAILURE, undef);
    }

    # compute the sum of squares
    for ($sample_index = 0; $sample_index < $number_samples; $sample_index++)
    {
        $sum_of_squares += $sample[$sample_index] ** 2;
    }

    # subtract mean squared for each sample
    $sum_of_squares -= $number_samples * ($mean ** 2);

    # use the absolute value to avoid round off errors
    $standard_deviation = sqrt(abs($sum_of_squares) / ($number_samples - 1));

    return (SUCCESS, $standard_deviation);
}

##############################################################################
# Name: ias_get_rmse
#
# Purpose: Calculate root mean squared error of an array
#
# Input: array of values
#
#    RETURNS:
#        SUCCESS or FAILURE
#        root mean squared error
##############################################################################
sub ias_get_rmse
{
    my $rmse;                    # result we are looking for
    my $sum_of_squares = 0;      # sum of squared difference from the mean
    my $sample_index;            # index into sample
    my $number_samples;          # number of samples

    my (@sample) = @_;

    # get number of samples - for readability only
    $number_samples = @sample;

    # if the sample is less than or equal to 1
    if ($number_samples <= 0)
    {
        my $msg = "Could not compute the RMSE because the sample is empty";
        print STDERR __FILE__ . ": $msg\n";
        return (FAILURE, undef);
    }

    # compute the sum of squares
    for ($sample_index = 0; $sample_index < $number_samples; $sample_index++)
    {
        $sum_of_squares += $sample[$sample_index] ** 2;
    }

    # compute the RMSE
    $rmse = sqrt($sum_of_squares / $number_samples);

    return (SUCCESS, $rmse);
}

1;
