
package ias_system_call;

##############################################################################
#    NAME: ias_system_call.pm
#
# PURPOSE: To provide a routine that captures STDOUT and STDERR from executing
#          a command line program.
#
##############################################################################

# System Includes
use strict;
use warnings;
use POSIX;
use IPC::Open3;
use Symbol qw(gensym);
use IO::File;

use vars qw(@ISA @EXPORT @EXPORT_OK %EXPORT_TAGS $VERSION);
use Exporter;

# IAS Includes
use ias_const;

@ISA = qw(Exporter);
@EXPORT = qw(
              ias_system_call
            );

##############################################################################
#        NAME: ias_system_call
#
# DESCRIPTION: Execute a program and capture the STDOUT and STDERR in a hash.
#
#     RETURNS: hash: A hash of the results
#
#        NOTE: Any application being called cannot output "open3:" on the
#              first line of stdout because this code checks for a failure
#              indication from open3, which does contain that text.
#
#        NOTE: open3 returns error messages on stdout.
#
# HASH DESCRIPTION:
#          $var{command} - Specifies the command line that was executed.
#           $var{stdout} - If defined it holds an array of the STDOUT from the
#                          executed program.
#           $var{stderr} - If defined it holds an array of the STDERR from the
#                          executed program.
#           $var{status} - (SUCCESS or ERROR) The status from this
#                          ias_system_call routine.
#     $var{return_value} - (SUCCESS, FAILURE, or some other return value from
#                          the program)
#
##############################################################################
sub ias_system_call($)
{
    my $cmd = $_[0];

    my ($IAS_STDIN, $IAS_STDOUT, $IAS_STDERR);
    my %result_hash = ();
    my $pid = 0;

    # Setup the stdin
    $IAS_STDIN = gensym;

    # Default the hash values
    $result_hash{stdout} = undef;
    $result_hash{stderr} = undef;
    $result_hash{status} = ERROR;
    $result_hash{return_value} = FAILURE;

    # Specify this from the input
    $result_hash{command} = $cmd;

    # Try to execute the program
    $pid = open3($IAS_STDIN, $IAS_STDOUT, $IAS_STDERR, $cmd);

    # Read STDOUT while it is executing
    while (<$IAS_STDOUT>) { push(@{$result_hash{stdout}}, $_); }
    waitpid($pid, 0);

    # Copy STDOUT and STDERR into the hash components
    if (defined $IAS_STDOUT)
    {
        while (<$IAS_STDOUT>) { push(@{$result_hash{stdout}}, $_); }
    }
    if (defined $IAS_STDERR)
    {
        while (<$IAS_STDERR>) { push(@{$result_hash{stderr}}, $_); }
    }

    # Check for a successful program execution
    # Catch both the 127(error) and 128(core dump) values using 255
    if (!($? & 255))
    {
        # Default to a successfull execution of the program where the
        # return_value hash variable contains the return value of the
        # executed program
        $result_hash{return_value} = $? >> 8;
        $result_hash{status} = SUCCESS;

        # However if stdout has contents, we need to check if the open3
        # encountered an error
        if (defined $result_hash{stdout})
        {
            # Test for an open3 message and return failure values
            # open3 will return "open3:" at the beginning of a stdout line
            # We only need to test the first line
            if ($result_hash{stdout}[0] =~ /^open3:/)
            {
                $result_hash{return_value} = FAILURE;
                $result_hash{status} = ERROR;
            }
        }
    }
    # If the above "if" does not happen, then the defaulted values of ERROR
    # and FAILURE will be returned in the hash

    return %result_hash;
}

1;
