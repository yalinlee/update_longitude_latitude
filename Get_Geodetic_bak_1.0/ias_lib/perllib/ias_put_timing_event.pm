package ias_put_timing_event;
##############################################################################
#
# Name: ias_put_timing_event
#
# Description: Logs an event to the WORK_ORDER_LOG database table
#
# Parameters: work order id, event, component
#
# Returns: SUCCESS or ERROR;
#
# NOTE: The caller is responsible for committing the database update.  This
#       module is typically called as part of a database transaction, in 
#       which case the commit for the transaction can be used.
#
##############################################################################

use Exporter;
@ISA = qw(Exporter);
@EXPORT = qw(
                ias_put_timing_event
            );

use strict;
use DBI;
use ias_const;
use ias_db;
use ias_processing_system;

sub ias_put_timing_event($$$$)
{
    my ($dbh, $work_order_id, $event, $component) = @_;

    my $product_request_id = undef;
    my $scene_id = undef;

    # Get product_request_id from work orders table if there is a work order
    # id and we are running LPGS
    if (!$isInitialized)
    {
        # Initialize the ias_processing_system module
        if (ias_init_system() != SUCCESS)
        {
            $ias_db_error = $ias_error_msg;
            return ERROR;
        }
    }

    if (!$isLPGS)
    {
        $ias_db_error = "ias_put_timing_event called on a non-LPGS system. " .
            "Only LPGS can send events to the WORK_ORDER_LOG DB table.";
        return ERROR;
    }

    if (defined $work_order_id && ($work_order_id ne ""))
    {
        my $query = "SELECT PRODUCT_REQUEST_ID, SCENE_ID FROM work_orders"
            . " WHERE WORK_ORDER_ID='$work_order_id'";

        # prepare and execute the query
        # if this fails we still want to go on and insert the event without
        # the product request id, so don't return an error
        my $sth = $dbh->prepare($query);
        if ($sth)
        {
            if (!$sth->execute())
            {
                $ias_db_error = $DBI::errstr;
            }
            else
            {
                my $db_row_ref = $sth->fetchrow_hashref();
                $product_request_id = $db_row_ref->{PRODUCT_REQUEST_ID};
                $scene_id = $db_row_ref->{SCENE_ID};
            }
            $sth->finish();
        }
    }

    $product_request_id = "" if (not defined $product_request_id);
    $scene_id = "" if (not defined $scene_id);

    my $sql = "INSERT INTO work_order_log (WORK_ORDER_LOG_ID, WORK_ORDER_ID,"
              . " STATUS, COMPONENT, DATE_STATUSED, PRODUCT_REQUEST_ID,"
              . " SCENE_ID) VALUES"
              . " (WORK_ORDER_LOG_SEQ.NEXTVAL, '$work_order_id', '$event',"
              . " '$component', SYSDATE, '$product_request_id', '$scene_id')";

    my $sth = $dbh->prepare($sql);
    if (!$sth)
    {
        $ias_db_error = $DBI::errstr;
        return ERROR;
    }

    if (!$sth->execute())
    {
        $ias_db_error = $DBI::errstr;
        return ERROR;
    }

    $sth->finish();

    return SUCCESS;

}
1;
