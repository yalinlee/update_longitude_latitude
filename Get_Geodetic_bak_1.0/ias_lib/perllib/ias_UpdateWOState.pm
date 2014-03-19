package ias_UpdateWOState;

############################################################################
#   NAME:  ias_UpdateWOState
#
#   PURPOSE: Updates the work order status in the database.
#
############################################################################

use strict;
use vars qw(@ISA @EXPORT @EXPORT_OK %EXPORT_TAGS $VERSION);
use Exporter;       # export functions and variables to users namespaces.
@ISA = qw(Exporter);
@EXPORT = qw(
                ias_UpdateWOState
            );

use ias_const;              # constant definitions
use ias_wo_const;           # Assign work order constants

############################################################################
# Name: ias_UpdateWOState
#
# Description: Updates the status of a work order in the database
#
# Parameters:
#   database handle
#   work order id
#   work order state
#   error message reference for returning error messages
#
# Returns:
#   FALSE indicates that the status was not updated
#   TRUE indicats that the status was sucessfully updated
#
############################################################################
sub ias_UpdateWOState($$$$)
{
    my ($dbh, $wo_id, $state, $error_ref) = @_;

    my $sql = undef;
    # build the sql statement to update the state of the work order in the 
    # work orders table (but only if the state isn't already aborted)
    # We set the DATE_UPDATED field if the state gets set to evaluate or
    # halted because this field only applies to those states.
    if ($state eq DB_WO_EVALUATE || $state eq DB_WO_HALTED)
    {
       $sql = "UPDATE work_orders "
            . "   SET state = '$state', "
            . "       date_updated = SYSDATE "
            . " WHERE work_order_id = '$wo_id' "
            . "   AND state != '". DB_WO_ABORTED . "'";
    }
    else
    {
       $sql = "UPDATE work_orders "
            . "   SET state = '$state' "
            . " WHERE work_order_id = '$wo_id' "
            . "   AND state != '" . DB_WO_ABORTED . "'";
    }

    my $rows = $dbh->do($sql);
    if (not defined $rows)
    {
        $$error_ref = $DBI::errstr;
        return FALSE;
    }
    elsif ($rows != 1)
    {
        $$error_ref = "Error updating the work order state for " . $wo_id;
        return FALSE;
    }

    return TRUE;
}

