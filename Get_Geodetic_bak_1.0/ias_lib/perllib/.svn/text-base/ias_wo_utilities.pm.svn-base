
package ias_wo_utilities;

##############################################################################
#    NAME: ias_wo_utilities.pm
#
# PURPOSE: To provide a collection of routines for retrieving information
#          using the work order ID.
#
##############################################################################

# System Includes
use strict;
use warnings;
use vars qw(@ISA @EXPORT @EXPORT_OK %EXPORT_TAGS $VERSION);
use Exporter;
use File::Basename;

# IAS Includes
use ias_const;
use ias_db;
use ias_date_utils;
use ias_system_call;
use ias_cpf_services;

@ISA = qw(Exporter);
@EXPORT = qw(
              ias_wo_get_used_cpf_name
              ias_wo_get_radiometric_wo_common_id
            );

##############################################################################
#        NAME: ias_wo_get_used_cpf_name
#
# DESCRIPTION: Get the name of the CPF used by the work order from the
#              radiometric_work_order_common or geometric_work_order_common
#              tables.
#
#  PARAMETERS: reference: The sensor specific database handle to use
#                 string: The work order ID to use
#
#     RETURNS: status: SUCCESS or ERROR
#              string: The CPF filename or undefined
#
##############################################################################
sub ias_wo_get_used_cpf_name($$)
{
    # Grab the input parameters value
    my $database_handle = $_[0];
    my $work_order_id = $_[1];

    my $cpf_filename = undef;

    my $sql_template = "SELECT cpf_name"
                     . " FROM %s"
                     . " WHERE work_order_id = '" . $work_order_id . "'";

    # Try to get the CPF filename from the radiometric common table first
    my $sql = sprintf($sql_template, "radiometric_work_order_common");

    my $sth = ias_db_execute_sql($database_handle, $sql);
    if (not defined $sth)
    {
        my $msg = "Failed to retrieve the CPF filename from DB : "
            . $ias_db_error;
        print STDERR __FILE__ . ": $msg ";
        return (ERROR, undef);
    }

    # Get the value from the database return
    if (my $db_row_ref = $sth->fetchrow_hashref())
    {
        $cpf_filename = $db_row_ref->{'CPF_NAME'};
    }

    $sth->finish();

    # If not found in radiometric common table try the geometric common table
    if (not defined $cpf_filename)
    {
        $sql = sprintf($sql_template, "geometric_work_order_common");

        $sth = ias_db_execute_sql($database_handle, $sql);
        if (not defined $sth)
        {
            my $msg = "Failed to retrieve the CPF filename from DB : "
                . $ias_db_error;
            print STDERR __FILE__ . ": $msg ";
            return (ERROR, undef);
        }

        # Get the value from the database return
        if (my $db_row_ref = $sth->fetchrow_hashref())
        {
            $cpf_filename = $db_row_ref->{'CPF_NAME'};
        }

        $sth->finish();
    }

    # Since we could not find a value return ERROR and undef
    if (not defined $cpf_filename)
    {
        return (ERROR, undef);
    }

    return (SUCCESS, $cpf_filename);
}


##############################################################################
#        NAME: ias_wo_get_radiometric_wo_common_id
#
# DESCRIPTION: Retrieve the radiometric work order common ID from the
#              database for the provided work order ID
#
#  PARAMETERS: reference: The database handle to use
#                 string: The work order ID to use
#
#     RETURNS:  status: SUCCESS or ERROR
#              integer: radiometric wo common ID
##############################################################################
sub ias_wo_get_radiometric_wo_common_id($$)
{
    # Grab the input parameters value
    my $database_handle = $_[0];
    my $work_order_id = $_[1];

    my $rad_wo_common_id = undef;

    # Get the ID
    my $sql = "SELECT radiometric_wo_common_id"
            . " FROM radiometric_work_order_common"
            . " WHERE work_order_id = '" . $work_order_id . "'";

    my $sth = ias_db_execute_sql($database_handle, $sql);
    if (not defined $sth)
    {
        my $msg = "Failed to to execute database query for radiometric " .
            "work order common ID :" . $ias_db_error;
        print STDERR __FILE__ . ": $msg ";
        return (ERROR, undef);
    }

    # Get the value from the database return
    if (my $db_row_ref = $sth->fetchrow_hashref())
    {
        $rad_wo_common_id = $db_row_ref->{'RADIOMETRIC_WO_COMMON_ID'};
    }

    $sth->finish();

    # If not specified we need to figure out the default name
    if (not defined $rad_wo_common_id)
    {
        my $msg = "Failed to retrieve radiometric work order common ID : "
            . $ias_db_error;
        print STDERR __FILE__ . ": $msg ";
        return (ERROR, undef);
    }

    return (SUCCESS, $rad_wo_common_id);
}


1;
