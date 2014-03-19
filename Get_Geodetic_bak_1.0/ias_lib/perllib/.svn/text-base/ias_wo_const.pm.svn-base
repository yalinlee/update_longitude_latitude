package ias_wo_const;

##############################################################################
#
# This module is used to provide constants for the IAS/LPGS system.
#
##############################################################################

use strict;
use vars qw(@ISA @EXPORT @EXPORT_OK %EXPORT_TAGS $VERSION);
use Exporter;
@ISA = qw(Exporter);
@EXPORT = qw(
                DB_WO_L0R_NEW
                DB_WO_REQUEST_FAILED
                DB_WO_L0R_REQUESTED
                DB_WO_SUBSETTER_FAILED
                DB_WO_L0R_READY
                DB_WO_INGEST_FAILED
                DB_WO_PENDING
                DB_WO_RUNNING
                DB_WO_HALTED
                DB_WO_EVALUATE
                DB_WO_INACTIVE
                DB_WO_DIAGNOSTIC_COMPLETE
                DB_WO_READY_TO_SHIP
                DB_WO_TRANSFER_FAILED
                DB_WO_ABORT_WO
                DB_WO_COMPLETE_WO
                DB_WO_RELEASE_ABORT
                DB_WO_RELEASE_COMPLETE
                DB_WO_ABORTED
                DB_WO_COMPLETED
                DB_WO_SCRIPT_PENDING
                DB_WO_SCRIPT_RUNNING
                DB_WO_SCRIPT_SKIP
                DB_WO_SCRIPT_COMPLETE
                DB_WO_SCRIPT_STATUS_SUCCESS
                UI_WO_SCRIPT_STATUS_SUCCESS_STR
                DB_WO_SCRIPT_STATUS_FAILURE
                DB_L0R_DATA_STATUS_SUBSETTER
                DB_L0R_DATA_STATUS_LOCAL
                DB_REQUEST_TYPE_STANDARD
                DB_REQUEST_TYPE_DIAGNOSTIC
                DB_ANOMALY_NEW
                DB_ANOMALY_ANALYSIS
                DB_ANOMALY_APPROVED
                DB_ANOMALY_FAILED
                DB_ANOMALY_RUN_HELD
                DB_ANOMALY_RUN_PAUSED
                DB_ANOMALY_RUNNING
                DB_ANOMALY_INTERNAL
                DB_ANOMALY_ORIGIN_INTERNAL
            );

# -----------------------------------------------------------------------
# Work order states.
    # New work orders with a subsetter request start at this state.
use constant DB_WO_L0R_NEW     => "N";
use constant UI_WO_L0R_NEW_STR => "New";

    # Failed to request the scene/interval from the subsetter.
use constant DB_WO_REQUEST_FAILED     => "RF";
use constant UI_WO_REQUEST_FAILED_STR => "L0R Request Failed";

    # The L0R scene/interval has been requested from the subsetter.
use constant DB_WO_L0R_REQUESTED     => "DR";
use constant UI_WO_L0R_REQUESTED_STR => "L0R Requested";

    # The subsetter failed to procduce the L0R scene/interval.
use constant DB_WO_SUBSETTER_FAILED     => "SF";
use constant UI_WO_SUBSETTER_FAILED_STR => "Subsetter Failed";

    # Successfully subsetted.  Waiting for ingest data to process the L0R data.
use constant DB_WO_L0R_READY     => "LR";
use constant UI_WO_L0R_READY_STR => "L0R Ready";

    # Ingest data failed to process the L0Rp metadata.
use constant DB_WO_INGEST_FAILED     => "IF";
use constant UI_WO_INGEST_FAILED_STR => "Ingest Failed";

    # Work order is ready for processing.  New work orders using local data
    # start at this state.
use constant DB_WO_PENDING     => "P";
use constant UI_WO_PENDING_STR => "Pending";

    # Work order is currently running.
use constant DB_WO_RUNNING     => "R";
use constant UI_WO_RUNNING_STR => "Running";

    # Work order failed to process.
use constant DB_WO_HALTED     => "H";
use constant UI_WO_HALTED_STR => "Halted";

    # Work order completed.  Waiting for evaluation approval.
use constant DB_WO_EVALUATE     => "E";
use constant UI_WO_EVALUATE_STR => "Evaluate";

    # Inactive work order.  Anomaly order waiting for activation.
use constant DB_WO_INACTIVE     => "I";
use constant UI_WO_INACTIVE_STR => "Inactive WO";

    # Diagnostic work order completed.  Anomaly order waiting for approval.
use constant DB_WO_DIAGNOSTIC_COMPLETE     => "DC";
use constant UI_WO_DIAGNOSTIC_COMPLETE_STR => "Diagnostic WO Complete";

    # Work order completed. Ready for transfer product to deliver the product.
use constant DB_WO_READY_TO_SHIP     => "RS";
use constant UI_WO_READY_TO_SHIP_STR => "Ready To Ship";

    # Transfer product failed.
use constant DB_WO_TRANSFER_FAILED     => "TF";
use constant UI_WO_TRANSFER_FAILED_STR => "Transfer Failed";

    # Work order aborted. Notify TRAM
use constant DB_WO_ABORT_WO     => "AW";
use constant UI_WO_ABORT_WO_STR => "Abort WO";

    # Work order completed. Notify TRAM
use constant DB_WO_COMPLETE_WO     => "CW";
use constant UI_WO_COMPLETE_WO_STR => "Complete WO";

    # Work order aborted.  Ready for subsetter to release resources
use constant DB_WO_RELEASE_ABORT     => "RA";
use constant UI_WO_RELEASE_ABORT_STR => "Release L0R Abort";

    # Work order completed.  Ready for subsetter to release resources
use constant DB_WO_RELEASE_COMPLETE     => "RC";
use constant UI_WO_RELEASE_COMPLETE_STR => "Release L0R Complete";

    # Work order aborted. - Final state
use constant DB_WO_ABORTED     => "A";
use constant UI_WO_ABORTED_STR => "WO Aborted";

    # Work order completed. - Final state
use constant DB_WO_COMPLETED     => "C";
use constant UI_WO_COMPLETED_STR => "WO Completed";

# -----------------------------------------------------------------------
# Work Order Script states
    # Work order script is waiting to be ran. - Pending
use constant DB_WO_SCRIPT_PENDING     => "P";
use constant UI_WO_SCRIPT_PENDING_STR => "Pending";

    # Work order script is running.
use constant DB_WO_SCRIPT_RUNNING     => "R";
use constant UI_WO_SCRIPT_RUNNING_STR => "Running";

    # Work order script will be skipped. - Final State
use constant DB_WO_SCRIPT_SKIP     => "K";
use constant UI_WO_SCRIPT_SKIP_STR => "Skip";

    # Work order script has completed. - Final State
use constant DB_WO_SCRIPT_COMPLETE => "C";
use constant UI_WO_SCRIPT_COMPLETE_STR => "Complete";

# -----------------------------------------------------------------------
# Work order script exit status.
    # Script ran successfully.
use constant DB_WO_SCRIPT_STATUS_SUCCESS => "S";
use constant UI_WO_SCRIPT_STATUS_SUCCESS_STR => "Success";

    # Script failed with errors.
use constant DB_WO_SCRIPT_STATUS_FAILURE => "F";
use constant UI_WO_SCRIPT_STATUS_FAILURE_STR => "Failure";

# -----------------------------------------------------------------------
# Work order L0R data source.
    # L0R input data came from the subsetter.
use constant DB_L0R_DATA_STATUS_SUBSETTER     => 0;
use constant UI_L0R_DATA_STATUS_SUBSETTER_STR => "Subsetter";

    # L0R input data came from a user provided disk path.
use constant DB_L0R_DATA_STATUS_LOCAL     => 1;
use constant UI_L0R_DATA_STATUS_LOCAL_STR => "Local";

# -----------------------------------------------------------------------
# Work order request types.
    # Diagnostic reqeust type.
use constant DB_REQUEST_TYPE_STANDARD     => "S";
use constant UI_REQUEST_TYPE_STANDARD_STR => "Standard";

    # Diagnostic reqeust type.
use constant DB_REQUEST_TYPE_DIAGNOSTIC     => "D";
use constant UI_REQUEST_TYPE_DIAGNOSTIC_STR => "Diagnostic";

# -----------------------------------------------------------------------
# Anomaly states.
    # New anomaly.
use constant DB_ANOMALY_NEW     => 1;
use constant UI_ANOMALY_NEW_STR => "New";

    # Not actually used but in the UI.
use constant DB_ANOMALY_ANALYSIS     => 2;
use constant UI_ANOMALY_ANALYSIS_STR => "Analysis";

    # Anomaly work order completed and approved.
use constant DB_ANOMALY_APPROVED     => 3;
use constant UI_ANOMALY_APPROVED_STR => "Closed-Approved";

    # Anomaly work order completed and rejected.
use constant DB_ANOMALY_FAILED     => 4;
use constant UI_ANOMALY_FAILED_STR => "Closed-Failed";

    # A new work order created for anomaly but has not been started yet.
use constant DB_ANOMALY_RUN_HELD     => 5;
use constant UI_ANOMALY_RUN_HELD_STR => "Run Held";

    # A work order for the anomaly has finished processing and is ready for
    # operator analysis.
use constant DB_ANOMALY_RUN_PAUSED     => 6;
use constant UI_ANOMALY_RUN_PAUSED_STR => "Run Paused";

    # A work order for the anomaly is running.
use constant DB_ANOMALY_RUNNING     => 7;
use constant UI_ANOMALY_RUNNING_STR => "Running";

# -----------------------------------------------------------------------
# Anomaly Origin.
    # Anomaly origin internal.
use constant DB_ANOMALY_ORIGIN_INTERNAL     => 1;
use constant UI_ANOMALY_ORIGIN_INTERNAL_STR => "Internal";

1;
