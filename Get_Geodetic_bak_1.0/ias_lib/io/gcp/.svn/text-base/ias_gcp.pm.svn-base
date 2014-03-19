package ias_gcp;

use strict;
use warnings;
use Logger;
use ias_const;                           # defines SUCCESS and ERROR

our (@ISA, @EXPORT_OK);
BEGIN
{
    require Exporter;
    @ISA = qw(Exporter);
    @EXPORT_OK = qw(ias_gcp_write_gcplib);  # symbols to export on request
}


#############################################################################
# 
# NAME:           ias_gcp_write_gcplib
# 
# PURPOSE:        The GCP Retrieval Module Detail Design describes a Perl script
#                 that acquires GCP data. This Perl module writes the GCP 
#                 metadata to disk. 
# 
# RETURNS:        1 on success
#                 0 on error
# 
# 
#############################################################################
sub ias_gcp_write_gcplib($$$)
{
    my $error_log = shift;                 # Logger error log
    my $gcp_filename = shift;              # GCP data file name
    my $gcp_data_reference = shift;        # Array reference

    # Verify gcp_data_reference is defined
    if (!defined($gcp_filename))
    {
        my $error = "gcp_data_reference is not defined";
        $error_log->fatal_msg(__FILE__, __LINE__, $error);
        return ERROR;
    }

    # Verify filename is not blank and is defined
    if (($gcp_filename eq "") || (!defined($gcp_filename)))
    {
        my $error = "Empty Filename";
        $error_log->fatal_msg(__FILE__, __LINE__, $error);
        return ERROR;
    }

    my @gcp_data = @{$gcp_data_reference}; # Array of hashes containing GCP data

    # Verify gcp_data array of hashes has at least one record (hash)
    my $total = $#gcp_data + 1;
    if ($total == 0)
    {
        my $warn = "No GCPLIB records";
        $error_log->warn_msg(__FILE__, __LINE__, $warn);
    }

    # Open the file
    my $status = open OUT, ">$gcp_filename";
    if (!defined($status))
    {
        my $error = "Opening $gcp_filename";
        $error_log->fatal_msg(__FILE__, __LINE__, $error);
        return ERROR;
    }

    # Print the header 
    $status = print OUT "# Unique Ground Control Point ID - integer\n"
        . "# Name of the chip image - string\n"
        . "# Reference line - double\n"
        . "# Reference sample - double\n"
        . "# True latitude of reference point in degrees - double\n"
        . "# True longitude of reference point in degrees - double\n"
        . "# Projection x value of reference point in meters - double\n"
        . "# Projection y value of reference point in meters - double\n"
        . "# Elevation at the Ground Control Point in meters - double\n"
        . "# Pixel size x value in meters - double\n"
        . "# Pixel size y value in meters - double\n"
        . "# Chip size in lines - double\n"
        . "# Chip size in samples - double\n"
        . "# Ground Control Point source - string\n"
        . "# Ground Control Point type - string\n"
        . "# Chip projection - string\n"
        . "# Projection UTM zone - integer\n"
        . "# Acquisition date - string in form mm-dd-yyyy\n"
        . "# Absolute or relative flag - string, should be ABS or REL\n"
        . "# Image chip data type - string\n";
    if ($status == 0)
    {
        my $error = "Writing header comment to $gcp_filename";
        $error_log->fatal_msg(__FILE__, __LINE__, $error);
        close OUT;
        return ERROR;
    }

    # Print the data start marker
    $status = print OUT "BEGIN\n";
    if ($status == 0)
    {
        my $error = "Writing BEGIN to $gcp_filename";
        $error_log->fatal_msg(__FILE__, __LINE__, $error);
        close OUT;
        return ERROR;
    }

    # Print the number of GCPs
    $status = print OUT "$total\n";
    if ($status == 0)
    {
        my $error = "Writing number of GCPs to $gcp_filename";
        $error_log->fatal_msg(__FILE__, __LINE__, $error);
        close OUT;
        return ERROR;
    }

    for my $gcp_record ( @gcp_data )
    {
        my ($point_id, $chip_name, $reference_line, $reference_sample,
                $latitude, $longitude, $projection_x, $projection_y, $elevation,
                $pixel_size_x, $pixel_size_y, $chip_size_lines, 
                $chip_size_samples, $source, $chip_type, $projection, $zone, 
                $date, $abs_rel_flag, $chip_data_type);

        # Get values out of each hash
        $point_id               = $gcp_record->{'point_id'};
        $chip_name              = $gcp_record->{'chip_name'};
        $reference_line         = $gcp_record->{'reference_line'};
        $reference_sample       = $gcp_record->{'reference_sample'};
        $latitude               = $gcp_record->{'latitude'};
        $longitude              = $gcp_record->{'longitude'};
        $projection_x           = $gcp_record->{'projection_x'};
        $projection_y           = $gcp_record->{'projection_y'};
        $elevation              = $gcp_record->{'elevation'};
        $pixel_size_x           = $gcp_record->{'pixel_size_x'};
        $pixel_size_y           = $gcp_record->{'pixel_size_y'};
        $chip_size_lines        = $gcp_record->{'chip_size_lines'};
        $chip_size_samples      = $gcp_record->{'chip_size_samples'};
        $source                 = $gcp_record->{'source'};
        $chip_type              = $gcp_record->{'chip_type'};
        $projection             = $gcp_record->{'projection'};
        $zone                   = $gcp_record->{'zone'};
        $date                   = $gcp_record->{'date'};
        $abs_rel_flag           = $gcp_record->{'abs_rel_flag'};
        $chip_data_type         = $gcp_record->{'chip_data_type'};

        # Verify these variables have the correct type of values
        if ($point_id !~ /^\d+$/) # should be int
        {
            my $error = "point_id: invalid value $point_id";
            $error_log->fatal_msg(__FILE__, __LINE__, $error);
            close OUT;
            return ERROR;
        }
        if ($chip_name !~ /^\w+$/) # should be string
        {
            my $error = "chip_name: invalid value $chip_name";
            $error_log->fatal_msg(__FILE__, __LINE__, $error);
            close OUT;
            return ERROR;
        }
        if ($reference_line !~ /^-?\d+\.?\d*$/) # should be double
        {
            my $error = "reference_line: invalid value $reference_line";
            $error_log->fatal_msg(__FILE__, __LINE__, $error);
            close OUT;
            return ERROR;
        }
        if ($reference_sample !~ /^-?\d+\.?\d*$/) # should be double
        {
            my $error = "reference_sample: invalid value $reference_sample";
            $error_log->fatal_msg(__FILE__, __LINE__, $error);
            close OUT;
            return ERROR;
        }
        if ($latitude !~ /^-?\d+\.\d+$/) # should be double
        {
            my $error = "latitude: invalid value $latitude";
            $error_log->fatal_msg(__FILE__, __LINE__, $error);
            close OUT;
            return ERROR;
        }
        if ($longitude !~ /^-?\d+\.\d+$/) # should be double
        {
            my $error = "longitude: invalid value $longitude";
            $error_log->fatal_msg(__FILE__, __LINE__, $error);
            close OUT;
            return ERROR;
        }
        if ($projection_y !~ /^-?\d+\.?\d*$/) # should be double
        {
            my $error = "projection_y: invalid value $projection_y";
            $error_log->fatal_msg(__FILE__, __LINE__, $error);
            close OUT;
            return ERROR;
        }
        if ($projection_x !~ /^-?\d+\.?\d*$/) # should be double
        {
            my $error = "projection_x: invalid value $projection_x";
            $error_log->fatal_msg(__FILE__, __LINE__, $error);
            close OUT;
            return ERROR;
        }
        if ($elevation !~ /^-?\d+\.?\d*$/) # should be double
        {
            my $error = "elevation: invalid value $elevation";
            $error_log->fatal_msg(__FILE__, __LINE__, $error);
            close OUT;
            return ERROR;
        }
        if ($pixel_size_x !~ /^-?\d+\.?\d*$/) # should be double
        {
            my $error = "pixel_size_x: invalid value $pixel_size_x";
            $error_log->fatal_msg(__FILE__, __LINE__, $error);
            close OUT;
            return ERROR;
        }
        if ($pixel_size_y !~ /^-?\d+\.?\d*$/) # should be double
        {
            my $error = "pixel_size_y: invalid value $pixel_size_y";
            $error_log->fatal_msg(__FILE__, __LINE__, $error);
            close OUT;
            return ERROR;
        }
        if ($chip_size_lines !~ /^-?\d+\.?\d*$/) # should be double
        {
            my $error = "chip_size_lines: invalid value $chip_size_lines";
            $error_log->fatal_msg(__FILE__, __LINE__, $error);
            close OUT;
            return ERROR;
        }
        if ($chip_size_samples !~ /^-?\d+\.?\d*$/) # should be double
        {
            my $error = "chip_size_samples: invalid value $chip_size_samples";
            $error_log->fatal_msg(__FILE__, __LINE__, $error);
            close OUT;
            return ERROR;
        }
        if ($source !~ /^\w+\+?$/) # should be string 
        {
            my $error = "source: invalid value $source";
            $error_log->fatal_msg(__FILE__, __LINE__, $error);
            close OUT;
            return ERROR;
        }
        if ($chip_type !~ /^\w+$/) # should be string
        {
            my $error = "chip_type: invalid value $chip_type";
            $error_log->fatal_msg(__FILE__, __LINE__, $error);
            close OUT;
            return ERROR;
        }
        if ($projection !~ /^\w+$/) # should be string
        {
            my $error = "projection: invalid value $projection";
            $error_log->fatal_msg(__FILE__, __LINE__, $error);
            close OUT;
            return ERROR;
        }
        if ($zone !~ /^\d+$/) # should be int
        {
            my $error = "zone: invalid value $zone";
            $error_log->fatal_msg(__FILE__, __LINE__, $error);
            close OUT;
            return ERROR;
        }
        if ($date !~ /^\d{2}-\d{2}-\d{4}$/) # should be date: MM-DD-YYYY
        {
            my $error = "date: invalid value $date";
            $error_log->fatal_msg(__FILE__, __LINE__, $error);
            close OUT;
            return ERROR;
        }

        if ($abs_rel_flag !~ /^(ABS|REL)$/) # Only ABS and REL are valid
        {
            my $error = "abs_rel_flag: invalid value $abs_rel_flag";
            $error_log->fatal_msg(__FILE__, __LINE__, $error);
            close OUT;
            return ERROR;
        }

        if ($chip_data_type !~ /^[\w\*]+$/) # should be string, can have '*'
        {
            my $error = "chip_data_type: invalid value $chip_data_type";
            $error_log->fatal_msg(__FILE__, __LINE__, $error);
            close OUT;
            return ERROR;
        }

        # Print out the values to the file
        my $status = printf OUT "%d %s %lf %lf %lf %lf %lf %lf %lf %lf %lf " .
            "%lf %lf %s %s %s %d %s %s %s\n", $point_id, $chip_name,
            $reference_line, $reference_sample, $latitude, $longitude,
            $projection_y, $projection_x, $elevation, $pixel_size_x,
            $pixel_size_y, $chip_size_lines, $chip_size_samples, $source, 
            $chip_type, $projection, $zone, $date, $abs_rel_flag, 
            $chip_data_type;
        if ($status == 0)
        {
            my $error = "Writing values to $gcp_filename";
            $error_log->fatal_msg(__FILE__, __LINE__, $error);
            close OUT;
            return ERROR;
        }
    }
    close OUT;

    return SUCCESS;
}
