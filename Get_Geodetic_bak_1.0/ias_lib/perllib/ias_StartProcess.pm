package ias_StartProcess;

############################################################################
#
# This is derived from the ALIAS file xxx_StartProcess.pm. 
#
############################################################################

use strict;
use vars qw(@ISA @EXPORT $VERSION);
use Exporter;
@ISA = qw(Exporter);
@EXPORT = qw( ias_StartProcess );

############################################################################
# Name: ias_StartProcess
#
# Description: Starts a new process
#
# Parameters:
#   database handle
#   process name to start
#   array of arguments to the new process
#
# Returns:
#   -1 on error
#   pid of new process on success
############################################################################
sub ias_StartProcess($$@)
{
    my ($dbh, $task_name, @args) = @_;

    # fork a new process
    my $pid = fork();
    if ($pid == -1)
    {
        # fork failed
        return -1;
    }
    elsif ($pid == 0)
    {
        # fork succeeded and this is the child process

        # do not automatically destroy the database connection in the child
        # since it will close it in the parent too
        $dbh->{InactiveDestroy} = 1;

        # start the new process
        exec($task_name, @args);

        # should never get here
        exit(-1);
    }
    else
    {
        # parent process, so return the child pid
        return $pid;
    }
}

1;
