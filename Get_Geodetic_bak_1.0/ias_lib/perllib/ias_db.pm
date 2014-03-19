package ias_db;

##############################################################################
#
# This module is used to provide database utilities which are generic across
# the IAS/LPGS system.
#
##############################################################################

use strict;
use DBI;
use ias_const;
use ias_wo_const;
use ias_processing_system;

use vars qw(@ISA @EXPORT @EXPORT_OK %EXPORT_TAGS $VERSION);

use Exporter;
@ISA = qw(Exporter);
@EXPORT = qw(
                ias_db_GetDBConnectionInfo
                ias_db_ConnectToDB
                ias_db_DisconnectFromDB
                ias_db_GetIASConfig
                ias_db_PutEvent
                ias_db_update_wo_dir
                ias_db_CreateAnomaly
                ias_db_AnomalyExists
                ias_db_execute_sql
                $ias_db_error
                IAS_DB_COMMITEVENT
                IAS_DB_NOCOMMITEVENT
                IAS_DB_DISCONNECT_COMMIT
                IAS_DB_DISCONNECT_ROLLBACK
            );

# variable to hold an error message for the last error encountered 
use vars qw($ias_db_error);
$ias_db_error = "";

# constants for database actions
use constant IAS_DB_COMMITEVENT => 0;
use constant IAS_DB_NOCOMMITEVENT => 1;
use constant IAS_DB_DISCONNECT_COMMIT => 0;
use constant IAS_DB_DISCONNECT_ROLLBACK => 1;

##############################################################################
# Name: ias_db_GetDBConnectionInfo
#
# Description: Returns the username and password to use to connect to the
#   database.
#
# Parameters:
#   database schema
#
# Returns:
#   user_id of the database user
#   password for the user
##############################################################################
sub ias_db_GetDBConnectionInfo(@)
{
    my ($db_schema) = @_;

    # default to using the Oracle OS authentication method
    my $user_id = "/";
    my $password = "";

    my $db_con_str;

    # default to common database if schema not passed in
    if (not defined $db_schema)
    {
        $db_schema = "COMMON";
    }

    # Retrieve connection information for the specified schema.
    if (($db_schema eq "COMMON") && defined $ENV{IAS_DB_COM})
    {
        $db_con_str = $ENV{IAS_DB_COM};
    }
    elsif ($db_schema eq "L7_ETM" && defined $ENV{IAS_DB_L7_ETM})
    {
        $db_con_str = $ENV{IAS_DB_L7_ETM};
    }
    elsif ($db_schema eq "L5_TM" && defined $ENV{IAS_DB_L5_TM})
    {
        $db_con_str = $ENV{IAS_DB_L5_TM};
    }
    elsif ($db_schema eq "L4_TM" && defined $ENV{IAS_DB_L4_TM})
    {
        $db_con_str = $ENV{IAS_DB_L4_TM};
    }
    elsif ($db_schema eq "L8_OLITIRS" && defined $ENV{IAS_DB_L8_OLITIRS})
    {
        $db_con_str = $ENV{IAS_DB_L8_OLITIRS};
    }
    else
    {
        # can not establish a user name and password so return the default
        # Oracle OS authentication method
        return ($user_id, $password);
    }

    ($user_id, $password) = split '/', $db_con_str;

    return ($user_id, $password);
}

##############################################################################
# Name: ias_db_ConnectToDB
#
# Description: Connect the user to the database
#
# Parameters:
#   username - database user name
#   password - user's database password
#
# Returns:
#   a database handle if successful
#   undef if not successful
##############################################################################
sub ias_db_ConnectToDB($$)
{
    my ($username, $password) = @_;
    my $sid = $ENV{'ORACLE_SID'};

    # connect to the database, disabling autocommit since the applications will
    # control commiting.  Also, disable automatically printing errors since
    # the errors need to be sent to a log file.
    my $dbh = DBI->connect("DBI:Oracle:$sid", $username, $password,
                           {AutoCommit => 0, PrintError => 0});

    # if the database connection was not successful, issue an error message and
    # return undef
    if (!$dbh)
    {
        $ias_db_error = $DBI::errstr;
        return undef;
    }

    return $dbh;
}

##############################################################################
# Name: ias_db_DisconnectFromDB
#
# Description: Disconnect from the database
#
# Parameters:
#   database handle - database handle to disconnect from
#   disconnect command - IAS_DB_DISCONNECT_COMMIT or IAS_DB_DISCONNECT_ROLLBACK
#
# Returns:
#   0 on error
#   1 on success
##############################################################################
sub ias_db_DisconnectFromDB($$)
{
    my ($dbh, $command) = @_;
    my $command_status = 0;

    if ($command == IAS_DB_DISCONNECT_COMMIT)
    {
        # the command was to commit before disconnecting
        $command_status = $dbh->commit();
        $ias_db_error = $DBI::errstr if (!$command_status);
    }
    elsif ($command == IAS_DB_DISCONNECT_ROLLBACK)
    {
        # the command was to rollback before disconnecting
        $command_status = $dbh->rollback();
        $ias_db_error = $DBI::errstr if (!$command_status);
    }
    else
    {
        $ias_db_error = "ias_db_DisconnectFromDB parameter error";
        return 0;
    }

    # disconnect from the database
    my $status = $dbh->disconnect();
    # set the error message if not already set and an error happened on 
    # disconnect
    $ias_db_error = $DBI::errstr if ($command_status && !$status);

    # return 0 if an error happened
    return 0 if (!$command_status || !$status);

    return 1;
}

##############################################################################
# Name: ias_db_GetIASConfig
#
# Description: Returns a hash where each element is a column from the sys_parms
#   table or environment variables.
#
# Parameters:
#   database handle - database connection handle
#
# Returns:
#   reference to a hash with configuration items on success
#   undef on error
##############################################################################
sub ias_db_GetIASConfig($)
{
    my ($dbh) = @_;

    # initialize the hash to return
    my %config = ();

    # query the sys_parms table for the configuration items there
    my $query;
    if ($isLANDSAT)
    {
        $query = "SELECT
            CHECK_DISK_INTERVAL,
            DELETION_INTERVAL,
            DDM_DAN_POLLING_INTERVAL,
            DDM_DES_POLLING_INTERVAL,
            DDM_FT_POLLING_INTERVAL,
            DISK_FIRST_ADVISORY_LIMIT,
            DISK_SECOND_ADVISORY_LIMIT,
            EST_AVG_WO_SIZE,
            MAX_CONCURRENT_WO,
            MAX_FT_ATTEMPTS,
            PSI_POLLING_INTERVAL,
            RESOURCE_MONITOR_INTERVAL,
            SERVER_HOSTNAME,
            SUBSETTER_INTERFACE_STATE,
            WORK_ORDER_SCHEDULER_INTERVAL
            FROM sys_parms";
    }
    else # isLDCM
    {
        $query = "SELECT 
            DELETION_INTERVAL,
            DISK_FIRST_ADVISORY_LIMIT,
            DISK_SECOND_ADVISORY_LIMIT,
            EST_AVG_WO_SIZE,
            MAX_CONCURRENT_WO,
            PSI_POLLING_INTERVAL,
            RESOURCE_MONITOR_INTERVAL,
            SERVER_HOSTNAME,
            WORK_ORDER_SCHEDULER_INTERVAL,
            DCO_POLLING_INTERVAL,
            DSI_POLLING_INTERVAL,
            SUBSETTER_INTERFACE_STATE,
            WORK_ORDER_EXPIRE_AGE";
        if ($isIAS) 
        {
            $query .= ", CEM_POLLING_INTERVAL";
        }
        $query .= " FROM sys_parms";
    }

    # prepare and execute the query
    my $sth = $dbh->prepare($query);
    if (!$sth)
    {
        $ias_db_error = $DBI::errstr;
        return undef;
    }
    my $status = $sth->execute();
    if (!$status)
    {
        $ias_db_error = $DBI::errstr;
        return undef;
    }

    # copy the data returned by the query to the hash that will be returned
    # to the calling routine
    my $success = 0;
    while (my $db_row_ref = $sth->fetchrow_hashref())
    {
        $success = 1;
        $config{DeletionInterval} = $db_row_ref->{DELETION_INTERVAL};
        $config{DiskFirstAdvisoryLimit} 
                = $db_row_ref->{DISK_FIRST_ADVISORY_LIMIT};
        $config{DiskSecondAdvisoryLimit} 
                = $db_row_ref->{DISK_SECOND_ADVISORY_LIMIT};
        $config{EstimateAverageWOSize} = $db_row_ref->{EST_AVG_WO_SIZE};
        $config{MaxConcurrentWO} = $db_row_ref->{MAX_CONCURRENT_WO};
        $config{PSIPollingInterval} = $db_row_ref->{PSI_POLLING_INTERVAL};
        $config{ResourceMonitorInterval} 
                = $db_row_ref->{RESOURCE_MONITOR_INTERVAL};
        $config{ServerHostname} = $db_row_ref->{SERVER_HOSTNAME};
        $config{SubsetterInterfaceFlag} 
                = $db_row_ref->{SUBSETTER_INTERFACE_STATE};
        $config{WorkOrderSchedulerInterval} 
                = $db_row_ref->{WORK_ORDER_SCHEDULER_INTERVAL};

        # System specific items.
        if ($isLANDSAT)
        {
            $config{CheckDiskInterval} = $db_row_ref->{CHECK_DISK_INTERVAL};
            $config{MaxFTAttempts} = $db_row_ref->{MAX_FT_ATTEMPTS};
            $config{DDM_DAN_PollingInterval} =
                $db_row_ref->{DDM_DAN_POLLING_INTERVAL};
            $config{DDM_DES_PollingInterval} =
                $db_row_ref->{DDM_DES_POLLING_INTERVAL};
            $config{DDM_FT_PollingInterval} =
                $db_row_ref->{DDM_FT_POLLING_INTERVAL};
        }

        if ($isLDCM)
        {
            $config{DCOPollingInterval} = $db_row_ref->{DCO_POLLING_INTERVAL};
            $config{DSIPollingInterval} = $db_row_ref->{DSI_POLLING_INTERVAL};
            $config{WorkOrderExpireAge} = $db_row_ref->{WORK_ORDER_EXPIRE_AGE};
            if ($isIAS)
            {
                $config{CEMPollingInterval} = 
                    $db_row_ref->{CEM_POLLING_INTERVAL};
            }
        }
    }
    $sth->finish();

    # if no records were read by the query, return an error
    if ($success == 0)
    {
        $ias_db_error = "Error: no data found in sys_parms table";
        return undef;
    }

    # verify directory related enviroment variable for both Landsat and LDCM
    if (not defined $ENV{IAS_DATA_DIR})
    {
        $ias_db_error = "Error: environment variables not set up properly";
        return undef;
    }
    $config{TarPathName} = $ENV{IAS_DATA_DIR} . "/temp/tar/";
    $config{ScriptPathName} = $ENV{IAS_BIN} . "/";

    # verify the directory related environment variables are defined
    if ($isLANDSAT) # IAS
    {
        # verify the directory related environment variables are defined
        if (not defined $ENV{IAS_WODATA})
        {
            $ias_db_error = "Error: environment variables not set up properly";
            return undef;
        }

        # set up the config hash with the directories
        $config{TestScriptPathName} = $ENV{IAS_SYS_DIR} . "/test_script/";
        $config{WOPathName} = $ENV{IAS_WODATA} . "/";
    }
    else # LDCM
    {
        # verify the directory related environment variables are defined
        if ((not defined $ENV{IAS_SYSTEM_ID})
            || (not defined $ENV{IAS_WO}))
        {
            $ias_db_error = "Error: environment variables not set up properly";
            return undef;
        }

        # set up the config hash with the directories
        $config{TestScriptPathName} = $ENV{IAS_DATA_DIR} . "/test_script/";
        $config{WOPathName} = $ENV{IAS_WO} . "/";
    }

    # Also set up the config hash with the IAS system id.  Only
    # select the first character, and make sure it's upper case.
    $config{SystemId} = $ENV{IAS_SYSTEM_ID};
    $config{SystemId} =~ s/(\w)\w*/\U$1/;

    # return a reference to the hash of configuration items
    return \%config;
}

##############################################################################
# Name: ias_db_PutEvent
#
# Description: Inserts an Event into the Events table.
#
# Parameters:
#   database handle - database connection handle
#   program name - name of program putting the event into the database
#   message id - message id number for the event
#   work order id - work order id for the event
#   script id - script id for the event
#   event comment - comment for the event
#   command - IAS_DB_COMMITEVENT or IAS_DB_NOCOMMITEVENT
#   
#
# Returns:
#   1 on success, 0 on error
##############################################################################
sub ias_db_PutEvent($$$$$$$)
{
    my ($dbh, $program_name, $message_id, $work_order_id, $script_id, 
        $event_comment, $command) = @_;

    my $product_request_id = undef;

    # replace all ' in the event command with ''.  This is for the database
    # call.
    $event_comment =~ s/'/''/g;
    
    # Get product_request_id form work orders table if there is a work order
    # id and we are running LPGS
    if (!$isInitialized)
    {
        # Initialize the ias_processing_system module
        if (ias_init_system() != SUCCESS)
        {
            $ias_db_error = $ias_error_msg;
            return 0;
        }
    }

    if ($isLPGS && defined $work_order_id && ($work_order_id ne ""))
    {
        my $query = "SELECT PRODUCT_REQUEST_ID FROM work_orders WHERE"
            . " WORK_ORDER_ID='$work_order_id'";

        # prepare and execute the query
        # if this fails we still want to go on and insert the event without
        # the product request id, so don't return an error
        my $sth = $dbh->prepare($query);
        if (!$sth)
        {
            $ias_db_error = $DBI::errstr;
        }
        else
        {
            if (!$sth->execute())
            {
                $ias_db_error = $DBI::errstr;
            }
            else
            {
                while (my $db_row_ref = $sth->fetchrow_hashref())
                {
                    $product_request_id = $db_row_ref->{PRODUCT_REQUEST_ID};
                }
            }
            $sth->finish();
        }
    }

    $product_request_id = "" if (not defined $product_request_id);

    my $system_id = $ENV{IAS_SYSTEM_ID};

    my $sql = "INSERT INTO Events
                 (SEQ_NBR, PROGRAM_ID, EVENT_DATE, MESSAGE_ID, 
                  WORK_ORDER_ID, SCRIPT_ID, EVENT_COMMENT, 
                  PRODUCT_REQUEST_ID, SYSTEM_ID)
               VALUES
                 (EVENT_SEQ.NEXTVAL, '$program_name', SYSDATE, '$message_id', 
                  '$work_order_id', '$script_id', '$event_comment', 
                  '$product_request_id', '$system_id')";

    # perform the insert
    my $sth = $dbh->prepare($sql);
    if (!$sth)
    {
        $ias_db_error = $DBI::errstr;
        return 0;
    }
    if (!$sth->execute())
    {
        $ias_db_error = $DBI::errstr;
        return 0;
    }
    $sth->finish();

    # commit the event if requested
    if ($command == IAS_DB_COMMITEVENT)
    {
        if (!$dbh->commit())
        {
            $ias_db_error = $DBI::errstr;
            return 0;
        }
    }

    return 1;
}

##############################################################################
# Name: ias_db_update_wo_dir
#
# Description: Update the work order directory in the database.
#
# Parameters:
#     dbh --------- The database handle to use.
#     wo_id ------- The work order id to update in the work_orders table.
#     wo_dir ------ The work order directory to set in the work_orders table.
#     error_ref --- The reference to a string for returning errors.
#
# Outputs:
#     error_ref --- An error message or empty string.
#
# Returns: SUCCESS or ERROR
#
##############################################################################
sub ias_db_update_wo_dir($$$$)
{
    my $dbh = $_[0];       # Input
    my $wo_id = $_[1];     # Input
    my $wo_dir = $_[2];    # Input
    my $error_ref = $_[3]; # Output

    my $sql = "update work_orders set wo_path = '$wo_dir' 
                where work_order_id = '$wo_id'";

    my $rows = $dbh->do($sql);

    if (not defined $rows)
    {
        $$error_ref = $DBI::errstr;
        return ERROR;
    }
    elsif ($rows != 1)
    {
        $$error_ref = "Updating the work order directory for $wo_id";
        return ERROR;
    }

    return SUCCESS;
}

##############################################################################
# Name: ias_db_CreateAnomaly
#
# Description: Create an anomaly record in the anomalies table
#
# Parameters:
#     dbh --------- The database handle to use.
#     wo_id ------- The work order id.
#     pr_id ------- The product request id.
#     title ------- The anomaly title
#     error_ref --- The reference to a string for returning errors.
#
# Outputs:
#     error_ref --- An error message or empty string.
#
# Returns:
#   ERROR or Anomaly_Id
##############################################################################
sub ias_db_CreateAnomaly($$$$$)
{
    my ($dbh, $wo_id, $pr_id, $title, $error_ref) = @_;

    my $anomaly_id = ERROR;
    $$error_ref = '';

    # Get the next available anomaly Id from dbseq_anomaly_id
    my $sth = $dbh->prepare("SELECT anomalies_seq.nextval FROM DUAL");
    if (!$sth)
    {
        $$error_ref = "Error preparing anomaly_id sql: $DBI::errstr";
        return ERROR;
    }
    
    if (!$sth->execute())
    {
        $$error_ref = "Error executing anomaly_id sql: $DBI::errstr";
        $sth->finish();
        return ERROR;
    }

    # Get the anomaly_id. If no row was found, return an ERROR
    if (!(($anomaly_id) = $sth->fetchrow_array()))
    {
        $$error_ref = "No row found for $wo_id";
        $sth->finish();
        return ERROR;
    }

    $sth->finish();

    # Create the anomaly record
    my $anomaly_state = DB_ANOMALY_NEW;
    my $sql = "INSERT INTO anomalies "
            . " (anomaly_id, product_request_id, current_wo_id, date_entered, "
            . "    origin, state, title) "
            . "VALUES "
            . " ($anomaly_id, '$pr_id', '$wo_id', sysdate, "
            . "    " . DB_ANOMALY_ORIGIN_INTERNAL
            . ", $anomaly_state, '$title')";

    my $rows = $dbh->do($sql);

    if (not defined $rows)
    {
        $$error_ref = "Error inserting anomaly record";
        return ERROR;
    }

    return $anomaly_id;
}

##############################################################################
# Name: ias_db_AnomalyExists
#
# Description: Check if an anomaly exists for given product request id.
#
# Parameters:
#     dbh --------- The database handle to use.
#     pr_id ------- The product request id.
#     error_ref --- The reference to a string for returning errors.
#
# Outputs:
#     error_ref --- An error message or empty string.
#
# Returns:
#   TRUE, FALSE, or ERROR
##############################################################################
sub ias_db_AnomalyExists($$$)
{
    my ($dbh, $pr_id, $error_ref) = @_;

    my $found = TRUE;
    $$error_ref = '';

    my $sql = "SELECT anomaly_id "
            . "  FROM anomalies "
            . " WHERE product_request_id = '$pr_id'";

    my $sth = $dbh->prepare($sql);
    if (!$sth)
    {
        $$error_ref = "Error preparing anomaly exists sql: $DBI::errstr";
        return ERROR;
    }

    if (!$sth->execute())
    {
        $$error_ref = "Error executing anomaly exists sql: $DBI::errstr";
        $sth->finish();
        return ERROR;
    }

    # If no row was returned, an anomaly doesn't exist for this product request
    if (!$sth->fetchrow_array())
    {
        $found = FALSE;
    }
    
    $sth->finish();
    
    return $found;
}

##############################################################################
#
# Name: ias_db_execute_sql
#
# Description: Execute an sql statement and return the statement handle.
#
# Parameters: The database handle to use, and the sql to execute.
#
# Returns: The statement handle - defined or not defined.
#
##############################################################################
sub ias_db_execute_sql($$)
{
    my $dbh = $_[0];
    my $sql = $_[1];

    my $sth = $dbh->prepare($sql);
    if (!$sth)
    {
        $ias_db_error = "SQL Prepare Error :: " . $DBI::errstr;
        return undef;
    }
    elsif (!$sth->execute())
    {
        $ias_db_error = "SQL Execute Error :: " . $DBI::errstr;
        return undef;
    }

    return $sth;
}

1;
