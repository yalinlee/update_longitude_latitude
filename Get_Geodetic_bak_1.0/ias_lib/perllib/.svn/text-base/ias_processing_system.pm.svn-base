package ias_processing_system;

##############################################################################
#
# This module is used to provide functionality which is generic across the
# IAS/LPGS system.
#
##############################################################################

use strict;
use ias_const;
use Sys::Hostname;
use vars qw(@ISA @EXPORT @EXPORT_OK %EXPORT_TAGS $VERSION);
use Exporter;
@ISA = qw(Exporter);
@EXPORT = qw(
                ias_init_system
                ias_validate_environment
                ias_get_host_specific_log_filename
                $isInitialized
                $isIAS
                $isLPGS
                $isLANDSAT
                $isLDCM
                $ias_error_msg
                $ias_log_filename
            );

use vars qw(
                $isIAS
                $isLPGS
                $isLANDSAT
                $isLDCM
                $ias_error_msg
                $ias_log_filename
           );

my $isInitialized = FALSE;
##############################################################################
#
# Name: ias_init_system
#
# Description: Initializes module variables to define the current processing
#              system.  It also initializes the filename used for the system
#              log.
#
# Outputs:      $ias_error_msg, $ias_log_filename
#
# Returns: SUCCESS or ERROR
#
##############################################################################
sub ias_init_system()
{
    # Start with them all initialized to FALSE.
    $isIAS = FALSE;
    $isLPGS = FALSE;
    $isLANDSAT = FALSE;
    $isLDCM = FALSE;
    $isInitialized = FALSE;

    if(defined $ENV{PROCESSING_SYSTEM})
    {
        my $processing_system = $ENV{PROCESSING_SYSTEM};

        # IAS or LPGS
        if($processing_system =~ /^IAS_./)
        {
            $isIAS = TRUE;
        }
        elsif($processing_system =~ /^LPGS_./)
        {
            $isLPGS = TRUE;
        }
        else
        {
            $ias_error_msg = "ERROR: Invalid PROCESSING_SYSTEM " .
                        "environment variable.  Unknown system\n";
            return ERROR;
        }

        # LANDSAT or LDCM
        if($processing_system =~ /._LANDSAT$/)
        {
            $isLANDSAT = TRUE;
        }
        elsif($processing_system =~ /._LDCM$/)
        {
            $isLDCM = TRUE;
        }
        else
        {
            $ias_error_msg = "ERROR: Invalid PROCESSING_SYSTEM " .
                        "environment variable.  Unknown sensor.\n";

            return ERROR;
        }
    }
    else
    {
        $ias_error_msg = "ERROR: PROCESSING_SYSTEM environment variable " .
                                "not set.\n";

        return ERROR;
    }

    # Setup the log filename to use.
    # Default to the "IAS-log" name in the IAS_LOG directory path
    if(defined $ENV{IAS_LOG})
    {
        $ias_log_filename = $ENV{IAS_LOG}; # This defines the path.

        # Now add the filename to the path.
        if($isIAS)
        {
            $ias_log_filename .= "/IAS-Log";
        }
        else
        {
            $ias_log_filename .= "/LPGS-Log";
        }
    }
    else
    {
        $ias_error_msg = "ERROR: IAS_LOG environment variable not set.\n";

        return ERROR;
    }

    $isInitialized = TRUE;
    return SUCCESS;
}

###########################################################################
# Name: ias_get_host_specific_log_filename
#
# Description:
#       This gets the host specific log filename used by the various pcs
# scripts (including start_PWS, stop_PWC, stop_PWS, PWS, PWC).
#
# Returns:
#       error_message ('', if there isn't an error), log_filename for host
#
###########################################################################
sub ias_get_host_specific_log_filename()
{
    my $error_msg = '';
    if (!$isInitialized)
    {
        # Initialize the ias_processing_system module
        if (ias_init_system() != SUCCESS)
        {
            $error_msg = $ias_error_msg;
            chomp ($error_msg);
            return ($error_msg, '');
        }
        else
        {
            $error_msg = 'Warning: ias_init_system should be called prior' .
            " to this routine.";
        }
    }
    my $log_filename = $ias_log_filename;
    $log_filename .= "-" . hostname if ($isLPGS);
    return ($error_msg, $log_filename);
}

###########################################################################
# Name: ias_validate_environment
#
# Description:
#       Make sure environment variables are set.
#
# Inputs:
#       Array reference to the list of environment variables
#
# Returns:
#       number of errors encountered
#
# Outputs:
#       prints error messages to $ias_error_msg
###########################################################################
sub ias_validate_environment
{
    my $varsList = $_[0]; # List of environment variables
    my $env_errors = 0;   # Number of errors encountered in the list

    # Make sure each needed variable is defined
    foreach (@$varsList) {
        unless ($ENV{$_}) {
            $env_errors++;
            $ias_error_msg = "ERROR: Missing environment variable: $_\n";
        }
    }

    # Return the number of errors encountered
    return $env_errors;
}


1;
