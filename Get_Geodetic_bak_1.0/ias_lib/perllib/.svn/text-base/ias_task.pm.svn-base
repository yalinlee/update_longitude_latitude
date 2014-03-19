
package ias_task;

##############################################################################
#    NAME: ias_task.pm
#
# PURPOSE: To provide a base class that implements the common functionality
#          required by every IAS task.
#          This class:
#              - Establishes the Logger object
#              - Provides the main processing loop
#              - Implements the signal handling for termination of the task
#
##############################################################################

# System Includes
use strict;
use warnings;

# IAS Includes
use Logger;
use ias_const;
use ias_processing_system;
use ias_db;

##############################################################################
#        NAME: new (public)
#
# DESCRIPTION: This creates an instance of the ias_task object to be used by
#              the calling program.
#
#  PARAMETERS: string: The name of the program to use for logging.
#              string: The name of the polling interval value to use from the
#                      system configuration.
#
#     RETURNS: reference: The created object.
#
##############################################################################
sub ias_task::new
{
    # Grab the class designation off of the parameter list
    my $class = shift;

    # Grab the number of parameters passed to the constructor
    my $parameter_count = @_;

    # Define some parameter variables
    my $program_name = "ias_task"; # Default to this so UNK isn't used
    my $polling_interval_name = undef;

    # Initialize the ias_processing_system module
    if (ias_init_system() != SUCCESS)
    {
        print STDERR __FILE__ . ":$ias_error_msg\n";
        return undef;
    }

    # Create a logger object for the class to use
    my $logger = Logger->new($ias_log_filename);
    $logger->set_module_name($program_name);

    # Make sure a value was passed in for this parameter
    if ($parameter_count > 0)
    {
        $program_name = shift;
    }
    else
    {
        my $msg = "Missing program_name parameter";
        $logger->fatal_msg(__FILE__, __LINE__, $msg);
        $logger->close_log();
        return undef;
    }

    # We know the real name of the program now, so set it
    $logger->set_module_name($program_name);

    # Make sure a value was passed in for this parameter
    if ($parameter_count > 1)
    {
        $polling_interval_name = shift;
    }
    else
    {
        my $msg = "Missing polling_interval_name parameter";
        $logger->fatal_msg(__FILE__, __LINE__, $msg);
        $logger->close_log();
        return undef;
    }

    # Establish the self contents
    my $self = {
        program_name => $program_name,
        termination_triggered => FALSE,
        database_handle => undef,
        config_parameters => undef,
        polling_interval_name => $polling_interval_name,
        polling_interval => 10,
        logger => $logger
    };

    bless($self, $class);

    # Setup signal handlers so they gracefully shutdown the system
    $SIG{'INT'}  = sub { $self->_signal_handler };
    $SIG{'TERM'} = sub { $self->_signal_handler };
    $SIG{'QUIT'} = sub { $self->_signal_handler };
    $SIG{'HUP'}  = 'IGNORE';

    return $self;
}

##############################################################################
#        NAME: database_connect (public)
#
# DESCRIPTION: Connect to the database using the defaults
#
#     RETURNS: status: SUCCESS or ERROR
#
##############################################################################
sub ias_task::database_connect
{
    # Self object is passed as the first parameter
    my $self = shift;

    # Get the username and password for the database
    my ($username, $password) = ias_db_GetDBConnectionInfo();

    # Connect to the database
    $self->{database_handle} = ias_db_ConnectToDB($username, $password);

    if (not $self->{database_handle})
    {
        $self->{logger}->fatal_msg(__FILE__, __LINE__, $ias_db_error);
        return ERROR;
    }

    # In case open2 or open3 calls are made, setting AutoInactiveDestroy will
    # prevent the database connection from being closed by any fork/exec
    # failures.  open2 and open3 cause exec and forks to happen.
    $self->{database_handle}->{AutoInactiveDestroy} = 1;

    # Read the configuration for the IAS
    $self->{config_parameters} = ias_db_GetIASConfig($self->{database_handle});
    if (not defined $self->{config_parameters})
    {
        my $msg = "Getting IASConfigInfo : $ias_db_error";
        $self->{logger}->fatal_msg(__FILE__, __LINE__, $msg);
        return ERROR;
    }

    # Only set this if the polling_interval_name is defined in the
    # configuration parameters
    if (defined $self->{config_parameters}->{$self->{polling_interval_name}})
    {
        $self->{polling_interval} =
            $self->{config_parameters}->{$self->{polling_interval_name}};
    }
    else
    {
        my $msg = "Undefined polling interval name";
        $self->{logger}->fatal_msg(__FILE__, __LINE__, $msg);
        return ERROR;
    }

    return SUCCESS;
}

##############################################################################
#        NAME: database_disconnect (public)
#
# DESCRIPTION: Disconnect from the database
#
#     RETURNS: status: SUCCESS or ERROR
#
##############################################################################
sub ias_task::database_disconnect
{
    # Self object is passed as the first parameter
    my $self = shift;

    # Disconnect from the database while we sleep
    if (not ias_db_DisconnectFromDB($self->{database_handle},
                                    IAS_DB_DISCONNECT_COMMIT))
    {
        my $msg = "Disconnecting from database : $ias_db_error";
        $self->{logger}->fatal_msg(__FILE__, __LINE__, $msg);
        return ERROR;
    }

    return SUCCESS;
}

##############################################################################
#        NAME: _process (private)
#
# DESCRIPTION: This provides the default processing implementation.  Only
#              ERROR is returned because it is required be re-implemented.
#
#     RETURNS: status: ERROR
#
##############################################################################
sub ias_task::_process
{
    # Self object is passed as the first parameter
    my $self = shift;

    my $msg = "Implementation Error: Please re-implement me.......";
    $self->{logger}->fatal_msg(__FILE__, __LINE__, $msg);

    return ERROR;
}

##############################################################################
#        NAME: run (public)
#
# DESCRIPTION: This provide the main daemon loop for processing the task.
#
#     RETURNS: status: SUCCESS or ERROR
#
##############################################################################
sub ias_task::run
{
    # Self object is passed as the first parameter
    my $self = shift;

    my $status = SUCCESS;

    $self->{logger}->info_msg(__FILE__, __LINE__, "Started Processing");

    do
    {
        # The re-implementation of _process provides for the processing
        # required by the task
        $status = $self->_process();

        sleep($self->{polling_interval});
    }
    while ((not $self->{termination_triggered}) && $status == SUCCESS);

    return $status;
}

##############################################################################
#        NAME: _signal_handler (private)
#
# DESCRIPTION: Signal hander for terminating the process.  It sets a flag to
#              gracefully exit the application.
#
#     RETURNS: Nothing
#
##############################################################################
sub ias_task::_signal_handler
{
    # Self object is passed as the first parameter
    my $self = shift;

    # Notify the log
    $self->{logger}->info_msg(__FILE__, __LINE__, "Termination signaled");

    # Set the variable for termination
    $self->{termination_triggered} = TRUE;
}

1;
