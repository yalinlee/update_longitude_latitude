package Logger;

###########################################################################
####  Documentation is provided in POD format.
###########################################################################

=head1 NAME

Logger - supply object methods for logging

B<Original Concept and Implementation:  IAS 8.0>

=head1 SYNOPSIS

    use Logger;

    my $var = new Logger;
    my $var = new Logger FILENAME;
    my $var = new Logger FILENAME, MODULENAME;

=head1 DESCRIPTION

B<I<C<Logger>>> provides a logging implementation using Perl.  B<I<C<Logger>>>
provides methods for logging messages to a log files using a standard log
format.

The messaging format is consistant with current IAS logging.

A locking mechanism is used that conforms to the current IAS locking of log
files.

Messages can be sent to STDOUT by using a FILENAME of "STDOUT" during the
creation of the B<I<C<Logger>>> object.

=head2 MESSAGE FORMAT

Here is the printf formatting string
B<S<C<%19s  %s  %7s  %-20s  %6s  %s %s\n>>> that is used
for packing the given information.

B<S<C<    %19s>>  -> The time stamp of the message in the following format.
B<S<C<YYYY-MM-DD HH:MM:SS>>>.

B<S<C<    %s>>    -> The module name. Used to help identify what is generating
the message.  Default('UNK')

B<S<C<    %7s>>   -> The process ID. Used to help identify what is generating the
message.  Default('UNK')

B<S<    %-20s> -> A filename (without the path) indicating where the message was
generated from.

B<S<C<    %6s>>   -> The line number where the message was generated.

B<S<C<    %s>>    -> The status/type of the message "INFO", "WARN", or "FATAL".

B<S<C<    %s>>    -> The message.

=cut

###########################################################################

use strict;

use Exporter;
our @ISA = qw{Exporter};
our @EXPORT = qw{set_default_log get_default_log
                 set_log_by_name get_log_by_name
                 IAS_LOG_LEVEL_DEBUG
                 IAS_LOG_LEVEL_INFO
                 IAS_LOG_LEVEL_WARN
                 IAS_LOG_LEVEL_ERROR
                 IAS_LOG_LEVEL_DISABLE};

# Include some functionality.
use IO::File;
use File::Basename;
use Fcntl;
use POSIX qw{:unistd_h};
use Time::localtime;

# Some variables private to the library.
# They are used to keep track of the known Logger objects.
my $defaultLogger = undef;
my %loggerObjects;

# Create a log message level the same as in the ias_logging.h
use constant {
    IAS_LOG_LEVEL_DEBUG => 0, 
    IAS_LOG_LEVEL_INFO => 1, 
    IAS_LOG_LEVEL_WARN => 2, 
    IAS_LOG_LEVEL_ERROR => 3, 
    IAS_LOG_LEVEL_DISABLE => 4, 
};
###########################################################################

=pod 

=head2 CONSTRUCTOR

=over 4

=item * new ( [FILENAME [,MODULENAME]] )

Creates a B<I<C<Logger>>> object to be used for generating log files.

=for html <p></p>

=over 4

=item I<Returns>

A newly created B<I<C<Logger>>> object.

=back

=back

=cut

###########################################################################
sub new(@)
{
    # Grab the number of parameters passed to the constructor.
    my $count = @_;

    # Grab the class designation off of the parameter list.
    my $class = shift;

    # Define some parameter variables.
    my $filename = "";
    my $moduleName = "";

    # Make sure a value was passed in for this parameter.
    if ($count > 1)
    {
        $filename = shift;
    }

    # Make sure a value was passed in for this parameter.
    if ($count > 2)
    {
        $moduleName = shift;
    }

    # Define a new self object.
    my $self = {
        FILENAME => "",
        MODULENAME => "UNK",
        FILEHANDLE => new IO::File,
        LOG_MSG_LEVEL => IAS_LOG_LEVEL_INFO,
        OPEN => 0
    };

    # Associate the object with the class.
    bless($self, $class);

    # Check to see if a empty filename was provided.
    if (!($filename eq ""))
    {
        # Call initialize on the filename.
        if (!$self->_initialize($filename))
        {
            return 0;
        }
    }

    # Check to see if a empty moduleName was provided.
    if (!($moduleName eq ""))
    {
        # Call setModuleName with the moduleName.
        if (!$self->set_module_name($moduleName))
        {
            return 0;
        }
    }

    # Check to see if we already have a default Logger object.
    # The first Logger object created is the default Logger object.
    if (!defined($defaultLogger))
    {
        $defaultLogger = $self;
    }

    # Return the created Logger object.
    return $self;
}

# Private Internal Method.
# Initializes the Logger object.
# This opens the passed in filename for appending.
# The Logger object will hold the file open as long as the object exists.
# If the Logger object already has a log file open, it will be closed.
sub _initialize($)
{
    # Self object is passed as the first parameter.
    my $self = shift;

    # Grab the filename from the parameters.
    my $filename = shift;

    # Verify that the filename is not empty.
    if ($filename eq "")
    {
        return 0;
    }

    # Close the previous file if needed.
    if ($self->{OPEN})
    {
        $self->close_log();
    }

    # Initialize to the defaults.
    $self->{OPEN} = 0;

    if (not($filename eq "STDOUT"))
    {
        # Clear the umask.
        my $mode = 0000;
        my $umask = umask($mode);

        # Open the new log for appending.
        $mode = 0664;
        if (!$self->{FILEHANDLE}->open("$filename",
                                       O_CREAT|O_WRONLY|O_APPEND, $mode))
        {
            return 0;
        }

        # Reset the umask.
        umask($umask);

        # Set the autoflush on this file handle so that buffers are written
        # immediately.
        $self->{FILEHANDLE}->autoflush(1);
    }
    else
    {
        # Set autoflush for STDOUT.
        $| = 1
    }

    # Initialize these to the new logs status.
    $self->{FILENAME} = $filename;
    $self->{OPEN} = 1;

    # Initialize ias_log_msg_level
    my $log_level = $ENV{IAS_LOG_LEVEL};

    if (defined $log_level)
    {
        if (uc($log_level) eq "DEBUG")
        {
            $self->{LOG_MSG_LEVEL} = IAS_LOG_LEVEL_DEBUG;
        }
        elsif (uc($log_level) eq "INFO")
        {
            $self->{LOG_MSG_LEVEL} = IAS_LOG_LEVEL_INFO;
        } 
        elsif (uc($log_level) eq "WARN")
        {
            $self->{LOG_MSG_LEVEL} = IAS_LOG_LEVEL_WARN;
        }  
        elsif (uc($log_level) eq "ERROR")
        {
            $self->{LOG_MSG_LEVEL} = IAS_LOG_LEVEL_ERROR;
        } 
        else
        {
            return 0;
        }
    }
    else
    {
        $self->{LOG_MSG_LEVEL} = IAS_LOG_LEVEL_INFO;
    }

    # Return success.
    return 1;
}

###########################################################################

=pod 

=head2 PUBLIC METHODS

=cut

###########################################################################

###########################################################################

=pod 

=over 4

=item * set_default_log ( B<I<C<Logger>>><OBJECT> )

This method sets the internal default log variable to the passed in
B<I<C<Logger>>> object.

=back

=cut

###########################################################################
sub set_default_log($)
{
    # Set the default Logger object to the parameter passed in.
    $defaultLogger = shift;
}

###########################################################################

=pod 

=over 4

=item * B<I<C<Logger>>><OBJECT> get_default_log ( )

This method returns the internal variable which holds the default
B<I<C<Logger>>> object.

=for html <p></p>

=over 4

=item I<Returns>

The default B<I<C<Logger>>> object.

=back

=back

=cut

###########################################################################
sub get_default_log()
{
    # Return the default Logger object variable.
    return $defaultLogger;
}

###########################################################################

=pod 

=over 4

=item * set_log_by_name ( LOGNAME, B<I<C<Logger>>><OBJECT> )

This method adds the passed in B<I<C<Logger>>> to an internal hash of
B<I<C<Logger>>> objects.  B<get_log_by_name()> is used to return a saved
B<I<C<Logger>>> object.

=back

=cut

###########################################################################
sub set_log_by_name($$)
{
    # Grab the name from the parameters.
    my $name = shift;

    # Set the object from the passed in parameters.
    $loggerObjects{$name} = shift;
}

###########################################################################

=pod 

=over 4

=item * B<I<C<Logger>>><OBJECT> get_log_by_name ( LOGNAME )

This method returns the B<I<C<Logger>>> object associated with B<LOGNAME>.

=for html <p></p>

=over 4

=item I<Returns>

The B<I<C<Logger>>> object associated with the passed in name.

=back

=back

=cut

###########################################################################
sub get_log_by_name($)
{
    # Grab the name from the parameters.
    my $name = shift;

    # Return the Logger object associated with the passed in name.
    return $loggerObjects{$name};
}

###########################################################################

=pod 

=over 4

=item * BOOL set_module_name ( MODULENAME )

This method tells the B<I<C<Logger>>> object the B<MODULENAME> to output to
the log file.

=for html <p></p>

=over 4

=item I<Returns>

=over 4

=item * TRUE

If the module name was successfully set.

=item * FALSE

If the module name was an empty string.

=back

=back

=back

=cut

###########################################################################
sub set_module_name($)
{
    # Self object is passed as the first parameter.
    my $self = shift;

    # Grab the module name from the parameters.
    my $moduleName = shift;

    # Verify that the name is not empty.
    if ($moduleName eq "")
    {
        return 0;
    }

    # Set the module name in the self object.
    $self->{MODULENAME} = $moduleName;

    # Return success.
    return 1;
}

###########################################################################

=pod 

=over 4

=item * BOOL set_log_msg_level ( LOG_MSG_LEVEL )

This method tells the B<I<C<Logger>>> object the B<LOG_MSG_LEVEL> to output to
the log file.

=for html <p></p>

=over 4

=item I<Returns>

=over 4

=item * TRUE

If the log_msg_level was successfully set.

=item * FALSE

If the log_msg_level was an empty string.

=back

=back

=back

=cut

###########################################################################
sub set_log_msg_level($)
{
    # Self object is passed as the first parameter.
    my $self = shift;

    # Grab the $log_msg_level from the parameters.
    my $log_msg_level = shift;

    # Verify that the $log_msg_level is not empty.
    if ($log_msg_level eq "")
    {
        return 0;
    }

    # Set the LOG_MSG_LEVEL in the self object.
    $self->{LOG_MSG_LEVEL} = $log_msg_level;

    # Return success.
    return 1;
}

###########################################################################

=pod 

=over 4

=item * BOOL open_log ( FILENAME )

This method will initialize the B<I<C<Logger>>> object to use the passed in
B<FILENAME>.

=for html <p></p>

=over 4

=item I<Returns>

=over 4

=item * TRUE

If successfully initialized.

=item * FALSE

If failed to initialize.

=back

=back

=back

=cut

###########################################################################
sub open_log($)
{
    # Self object is passed as the first parameter.
    my $self = shift;

    # Grab the filename from the parameters.
    my $filename = shift;

    # Make sure a filename was provided.
    if ($filename eq "")
    {
        return 0;
    }

    # Call initialize on the filemname.
    if (!$self->_initialize($filename))
    {
        return 0;
    }

    # Return success.
    return 1;
}

###########################################################################

=pod 

=over 4

=item * close_log ( )

This method will close the file handle associated with the B<I<C<Logger>>>
object and re-initialize internal variables.

=back

=cut

###########################################################################
sub close_log()
{
    # Self object is passed as the first parameter.
    my $self = shift;

    if (not($self->{FILENAME} eq "STDOUT"))
    {
        # Check to see if the log is open before closing it.
        if ($self->{OPEN})
        {
            $self->{FILEHANDLE}->close();
        }
    }
    # Else there is nothing to close for STDOUT

    # Re-Initialize to the default values.
    $self->{FILENAME} = "STDOUT";
    $self->{OPEN} = 0;
}

###########################################################################

=pod 

=over 4

=item * BOOL info_msg ( [__FILE__, __LINE__,] MESSAGETEXT [, MODULENAME] )

This method is used to output a informational message to a log file.  The
__FILE__ and __LINE__ parameters may be omitted, and Logger will make use
of Perl's builtin caller function to determine calling file and line number.

=for html <p></p>

=over 4

=item I<Returns>

=over 4

=item * TRUE

If the message was successfully written to the log file.

=item * FALSE

If the message was not successfully written to the log file.

=back

=back

=back

=cut

###########################################################################
sub info_msg($$$;$)
{
    # Self object is passed as the first parameter.
    my $self = shift;

    # Determine the number of parameters that were passed.
    my $count = @_;

    # Default the message to an empty string.
    my $msg = "";

    # if $self->{LOG_MSG_LEVEL} is high enough, output the log message 
    if ($self->{LOG_MSG_LEVEL} <= IAS_LOG_LEVEL_INFO)
    {
        if ($count == 4 )
        {
            # Caller provided file, line, msg, and module
            #                           Status  File   Line   Msg    Module
            $msg = $self->_pack_msg_info("INFO", shift, shift, shift, shift);
        }
        elsif ($count == 3 )
        {
            # Caller provided file, line, and msg
            #                           Status  File   Line   Msg
            $msg = $self->_pack_msg_info("INFO", shift, shift, shift);
        }
        elsif ($count == 2)
        {
            # Get file and line information of caller
            my ($package, $file, $line) = caller;

            # Caller provided msg and module
            #                           Status  File   Line   Msg     Module
            $msg = $self->_pack_msg_info("INFO", $file, $line, shift, shift);
        }
        elsif ($count == 1)
        {
            # Get file and line information of caller
            my ($package, $file, $line) = caller;

            # Caller provided msg
            #                           Status  File   Line   Msg
            $msg = $self->_pack_msg_info("INFO", $file, $line, shift);
        }

        # Attempt to write the message.
        return $self->_write_msg($msg);
    }
}

###########################################################################

=pod 

=over 4

=item * BOOL warn_msg ( [__FILE__, __LINE__,] MESSAGETEXT [, MODULENAME] )

This method is used to output a warning message to a log file. The
__FILE__ and __LINE__ parameters may be omitted, and Logger will make use
of Perl's builtin caller function to determine calling file and line number.

=for html <p></p>

=over 4

=item I<Returns>

=over 4

=item * TRUE

If the message was successfully written to the log file.

=item * FALSE

If the message was not successfully written to the log file.

=back

=back

=back

=cut

###########################################################################
sub warn_msg($$$;$)
{
    # Self object is passed as the first parameter.
    my $self = shift;

    # Determine the number of parameters that were passed.
    my $count = @_;

    # Default the message to an empty string.
    my $msg = "";

    # if $self->{LOG_MSG_LEVEL} is high enough, output the log message 
    if ($self->{LOG_MSG_LEVEL} <= IAS_LOG_LEVEL_WARN)
    {
        if ($count == 4 )
        {
            # Caller provided file, line, msg, and module
            #                           Status  File   Line   Msg    Module
            $msg = $self->_pack_msg_info("WARN", shift, shift, shift, shift);
        }
        elsif ($count == 3 )
        {
            # Caller provided file, line, and msg
            #                           Status  File   Line   Msg
            $msg = $self->_pack_msg_info("WARN", shift, shift, shift);
        }
        elsif ($count == 2)
        {
            # Get file and line information of caller
            my ($package, $file, $line) = caller;

            # Caller provided msg and module
            #                           Status  File   Line   Msg     Module
            $msg = $self->_pack_msg_info("WARN", $file, $line, shift, shift);
        }
        elsif ($count == 1)
        {
            # Get file and line information of caller
            my ($package, $file, $line) = caller;

            # Caller provided msg
            #                           Status  File   Line   Msg
            $msg = $self->_pack_msg_info("WARN", $file, $line, shift);
        }

        # Attempt to write the message.
        return $self->_write_msg($msg);
    }
}

###########################################################################

=pod 

=over 4

=item * BOOL debug_msg ( [__FILE__, __LINE__,] MESSAGETEXT [, MODULENAME] )

This method is used to output a warning message to a log file. The
__FILE__ and __LINE__ parameters may be omitted, and Logger will make use
of Perl's builtin caller function to determine calling file and line number.

=for html <p></p>

=over 4

=item I<Returns>

=over 4

=item * TRUE

If the message was successfully written to the log file.

=item * FALSE

If the message was not successfully written to the log file.

=back

=back

=back

=cut

###########################################################################
sub debug_msg($$$;$)
{
    # Self object is passed as the first parameter.
    my $self = shift;

    # Determine the number of parameters that were passed.
    my $count = @_;

    # Default the message to an empty string.
    my $msg = "";

    # if $self->{LOG_MSG_LEVEL} is high enough, output the log message 
    if ($self->{LOG_MSG_LEVEL} <= IAS_LOG_LEVEL_DEBUG)
    {
        if ($count == 4 )
        {
            # Caller provided file, line, msg, and module
            #                           Status  File   Line   Msg    Module
            $msg = $self->_pack_msg_info("DEBUG", shift, shift, shift, shift);
        }
        elsif ($count == 3 )
        {
            # Caller provided file, line, and msg
            #                           Status  File   Line   Msg
            $msg = $self->_pack_msg_info("DEBUG", shift, shift, shift);
        }
        elsif ($count == 2)
        {
            # Get file and line information of caller
            my ($package, $file, $line) = caller;

            # Caller provided msg and module
            #                           Status  File   Line   Msg     Module
            $msg = $self->_pack_msg_info("DEBUG", $file, $line, shift, shift);
        }
        elsif ($count == 1)
        {
            # Get file and line information of caller
            my ($package, $file, $line) = caller;

            # Caller provided msg
            #                           Status  File   Line   Msg
            $msg = $self->_pack_msg_info("DEBUG", $file, $line, shift);
        }

        # Attempt to write the message.
        return $self->_write_msg($msg);
    }
}

###########################################################################

=pod 

=over 4

=item * BOOL fatal_msg ( [__FILE__, __LINE__,] MESSAGETEXT [, MODULENAME] )

This method is used to output a fatal message to a log file. The
__FILE__ and __LINE__ parameters may be omitted, and Logger will make use
of Perl's builtin caller function to determine calling file and line number.

=for html <p></p>

=over 4

=item I<Returns>

=over 4

=item * TRUE

If the message was successfully written to the log file.

=item * FALSE

If the message was not successfully written to the log file.

=back

=back

=back

=cut

###########################################################################
sub fatal_msg($$$;$)
{
    # Self object is passed as the first parameter.
    my $self = shift;

    # Determine the number of parameters that were passed.
    my $count = @_;

    # Default the message to an empty string.
    my $msg = "";
    
    # if $self->{LOG_MSG_LEVEL} is high enough, output the log message 
    if ($self->{LOG_MSG_LEVEL} <= IAS_LOG_LEVEL_ERROR)
    {
        if ($count == 4 )
        {
            # Caller provided file, line, msg, and module
            #                           Status   File   Line   Msg    Module
            $msg = $self->_pack_msg_info("FATAL", shift, shift, shift, shift);
        }
        elsif ($count == 3 )
        {
            # Caller provided file, line, and msg
            #                           Status   File   Line   Msg
            $msg = $self->_pack_msg_info("FATAL", shift, shift, shift);
        }
        elsif ($count == 2)
        {
            # Get file and line information of caller
            my ($package, $file, $line) = caller;

            # Caller provided msg and module
            #                           Status  File   Line   Msg     Module
            $msg = $self->_pack_msg_info("FATAL", $file, $line, shift, shift);
        }
        elsif ($count == 1)
        {
            # Get file and line information of caller
            my ($package, $file, $line) = caller;

            # Caller provided msg
            #                           Status  File   Line   Msg
            $msg = $self->_pack_msg_info("FATAL", $file, $line, shift);
        }
 
        # Attempt to write the message.
        return $self->_write_msg($msg);
    }
}

# Private Internal Method.
# This will pack the debug information into a string and add the timestamp.
sub _pack_msg_info($$$$;$)
{
    my $self = shift; # Object is passed as the first parameter.

    my $count = @_;

    my $status = shift;
    my $file = shift;
    my $line = shift;
    my $msg = shift;
    my $module = "";

    if ($count == 5)
    {
        $module = shift;
    }

    # Grab the current time in a tm struct..
    my $tm = localtime();

    # Remove any leading path from the filename.
    $file = basename($file, "");

    # Create the timestamp string.
    my $timeStr = sprintf("%04d-%02d-%02d %02d:%02d:%02d",
                            $tm->year + 1900,
                            $tm->mon + 1,
                            $tm->mday,
                            $tm->hour, $tm->min,
                            $tm->sec);

    if ($module eq "")
    {
        $module = $self->{MODULENAME};
    }

    my $return = sprintf("%19s  %s  %7s %-20s  %6s  %s %s\n", $timeStr,
       	$module, $$, $file, $line, $status, $msg);

    return $return;
}

# Private Internal Method.
# This is the code that will write the message.
sub _write_msg($)
{
    my $self = shift; # Object is passed as the first parameter.

    my $msg = shift;
 

    # Verify the file is open.
    if ($self->{OPEN} == 0)
    {
        return 0;
    }

    if (not($self->{FILENAME} eq "STDOUT"))
    {
        # The print will fail if this is not done. (Must be a perl bug.)
        my $fh = $self->{FILEHANDLE};

        my $status;   # status returned from print
        my $lock;     # lock structure for fcntl (see man page for fcntl)

        # Pack the flock structure. This structure is documented in the
        # fcntl man page. We use l! for the off_t type so it will work with
        # both 32 and 64 bit builds.
        $lock = pack('s s l! l! l', F_WRLCK, SEEK_SET, 0, 0, 0);

        # Lock the file.  This will block for any other locks on the file.
        fcntl($fh,  F_SETLKW, $lock) or return 0;  # Lock Exclusive

        # Actually output the message to the logfile.
        # Save status because we always want to unlock the file.
        $status = print($fh "$msg");

        # unlock the file.
        $lock = pack('s s l! l! l', F_UNLCK, SEEK_SET, 0, 0, 0);
        fcntl($fh, F_SETLKW, $lock) or return 0;  # Unlock
 
        # return error if print failed
        if (!$status)
        {
            return 0;
        }
    }
    else # Print to STDOUT
    {
        print "$msg";
    }

    return 1;
}

1;

