package ias_MessageIds;

############################################################################
#
# This file was originally based on the ALIAS file xdb_MessageIds.pm.
#
############################################################################

use strict;
use vars qw(@ISA @EXPORT @EXPORT_OK %EXPORT_TAGS $VERSION);
use Exporter;
@ISA = qw(Exporter);
@EXPORT = qw(
             IAS_MSG_WO_START
             IAS_MSG_WO_HALT
             IAS_MSG_WO_CONTINUE
             IAS_MSG_WO_ABORT
             IAS_MSG_WO_COMPLETE
             IAS_MSG_INGEST
             IAS_MSG_FTP
             IAS_MSG_INGESTDONE
             IAS_MSG_L0R_NOT_USED
             IAS_MSG_INGEST_STARTED
             IAS_MSG_RESTART_TASK
             IAS_MSG_STARTUP_COMPLETED
             IAS_MSG_EPHEMERIS_FILE_TOO_LONG
             IAS_MSG_EPHEMERIS_FILE_TOO_SHORT
             IAS_MSG_L0R_TIMED_OUT
             IAS_MSG_CONFIGURATION_MESSAGE
             IAS_MSG_SHUTDOWN_STARTED
             IAS_MSG_SHUTDOWN_COMPLETED
             IAS_MSG_WO_EXPIRE
             IAS_MSG_INPUT_DATA_ERROR
             IAS_MSG_INIT
             IAS_MSG_MAIN
             IAS_MSG_ABORTING
             IAS_MSG_DCO_TRANSFER_FAIL
             IAS_MSG_DCO_PRODUCT_TYPE_ERR
             IAS_MSG_DCO_START_WO_FAIL
             IAS_MSG_PWC_START_NEXT_SCRIPT
             IAS_MSG_PWC_MONITOR_SCRIPT
             IAS_MSG_PWS_START_NEXT_WO
             IAS_MSG_PWS_PROCESS_WO_TERM
             IAS_MSG_PWS_START_WO
             IAS_MSG_PSI_PROC_DIR
             IAS_MSG_PSI_REAPCHILD
             IAS_MSG_PSI_STARTPROCESS
             IAS_MSG_DRM_DISK_FIRST_ADVISORY
             IAS_MSG_DRM_DISK_SECOND_ADVISORY
             IAS_MSG_DRM_POLL_FOR_DELETE
             IAS_MSG_DRM_VERIFY_DB
             IAS_MSG_WARN_CREATE_DIRECTORY
             IAS_MSG_ERROR_CREATE_DIRECTORY
            );

############################################################################
# Message Id's for ias_db_PutEvent
############################################################################

# the 400 through 600 series error messages belong to IAS (not AIT) software

# status msgs
use constant IAS_MSG_WO_START                               => 401;
    # "Status: WO started"
use constant IAS_MSG_WO_HALT                                => 402;
    # "Status: WO (script) halted"
use constant IAS_MSG_WO_CONTINUE                            => 403;
    # "Status: WO (script) continued"
use constant IAS_MSG_WO_ABORT                               => 404;
    # "Status: WO aborted"
use constant IAS_MSG_WO_COMPLETE                            => 405;
    # "Status: WO completed"
use constant IAS_MSG_INGEST                                 => 406;
    # "Status: Ingested file from foreign host"
use constant IAS_MSG_FTP                                    => 407;
    # "Status: Transmitted file to foreign host"
use constant IAS_MSG_INGESTDONE                             => 408;
    # "Status: L0R Ingest done" */
use constant IAS_MSG_L0R_NOT_USED                           => 409;
    # "Status: L0R product not used by any WO"
use constant IAS_MSG_INGEST_STARTED                         => 411;
    # "Status: L0R Ingest started"
use constant IAS_MSG_RESTART_TASK                           => 412;
    # "Status: Restarting a task"
use constant IAS_MSG_STARTUP_COMPLETED                      => 413;
    # "Status: System startup completed"
use constant IAS_MSG_EPHEMERIS_FILE_TOO_LONG                => 414;
    # "Status: Ephemeris file is too long"
use constant IAS_MSG_EPHEMERIS_FILE_TOO_SHORT               => 415;
    # "Status: Ephemeris file is too short"
use constant IAS_MSG_L0R_TIMED_OUT                          => 416;
    # "Status: L0R not used:  L0R auto deleted"
use constant IAS_MSG_CONFIGURATION_MESSAGE                  => 417;
    # "Status: Configuration message"
use constant IAS_MSG_SHUTDOWN_STARTED                       => 418;
    # "Status: System shutdown started"
use constant IAS_MSG_SHUTDOWN_COMPLETED                     => 419;
    # "Status: System shutdown completed"
use constant IAS_MSG_WO_EXPIRE                              => 420;
    # "Status: WO expired"
use constant IAS_MSG_INPUT_DATA_ERROR                       => 422;
    # "Warning:  error processing input data"

# generic error msgs
use constant IAS_MSG_INIT                                   => 511;
    # "Error:  initializing task"
use constant IAS_MSG_MAIN                                   => 512;
    # "Error:  in task mainline processing"
use constant IAS_MSG_ABORTING                               => 513;
    # "Error:  task aborted"

# DCO error msgs
use constant IAS_MSG_DCO_TRANSFER_FAIL                      => 518;
    # "Error:  transferring product to delivery location"
use constant IAS_MSG_DCO_PRODUCT_TYPE_ERR                   => 519;
    # "Error:  getting module product type from code" 
use constant IAS_MSG_DCO_START_WO_FAIL                      => 520;
    # "Error:  starting work order with PCS"

# PWC error msgs
use constant IAS_MSG_PWC_START_NEXT_SCRIPT                  => 521;
    # "Error:  starting next WO script"
use constant IAS_MSG_PWC_MONITOR_SCRIPT                     => 522;
    # "Error:  processing WO script termination"

# PWS error msgs */
use constant IAS_MSG_PWS_START_NEXT_WO                      => 531;
    # "Error:  identifying next WO to start"
use constant IAS_MSG_PWS_PROCESS_WO_TERM                    => 532;
    # "Error:  processing PWC task termination"
use constant IAS_MSG_PWS_START_WO                           => 533;
    # "Error:  starting next WO"

# PSI error msgs
use constant IAS_MSG_PSI_PROC_DIR                           => 541;
    # "Error:  processing directive"
use constant IAS_MSG_PSI_REAPCHILD                          => 542;
    # "Error:  processing task termination"
use constant IAS_MSG_PSI_STARTPROCESS                       => 543;
    # "Error:  starting task"

# DRM msgs
use constant IAS_MSG_DRM_DISK_FIRST_ADVISORY                => 630;
    # "Warning:  disk usage exceeds first advisory limit"
use constant IAS_MSG_DRM_DISK_SECOND_ADVISORY               => 633;
    # "Error:  disk usage exceeds second advisory limit"
use constant IAS_MSG_DRM_POLL_FOR_DELETE                    => 631;
    # "Error:  deleting a Work Order"
use constant IAS_MSG_DRM_VERIFY_DB                          => 632;
    # "Error:  verifying files on disk vs. database"

# Warning msgs
use constant IAS_MSG_WARN_CREATE_DIRECTORY                  => 731;
    # "Warning:  failed to create directory"
# Failure msgs
use constant IAS_MSG_ERROR_CREATE_DIRECTORY                 => 732;
    # "Error:  failed to create directory"
1;
