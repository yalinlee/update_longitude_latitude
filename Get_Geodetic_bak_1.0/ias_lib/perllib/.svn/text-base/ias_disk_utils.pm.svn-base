package ias_disk_utils;

##############################################################################
#
# This module is used to provide disk utilities which are generic across the
# IAS/LPGS system.
#
##############################################################################

use strict;
use ias_const;
use File::Path;
use vars qw(@ISA @EXPORT @EXPORT_OK %EXPORT_TAGS $VERSION);
use Exporter;
@ISA = qw(Exporter);
@EXPORT = qw(
                ias_get_disk_usage
                ias_create_work_order_directory
            );

##############################################################################
# Name: ias_get_disk_usage
#
# Description: Return the output of a UNIX df -TP command for the requested
#              directory.
#
# Parameters:
#     dir --------- The directory to get the disk information for.
#     error_ref --- The reference to a string for returning errors.
#
# Outputs:
#     error_ref ----- An error message or empty string.
#
# Returns:
#     on error ---- undef,
#     on success -- A hash reference for the usage information.
##############################################################################
sub ias_get_disk_usage($$)
{
    my ($dir, $error_ref) = @_;

    # use /bin/df to get information about the diskusage for the specified
    # directory
    my $status = open(PROCPIPE, "/bin/df -TP $dir 2>&1 |");
    if (!$status)
    {
        $$error_ref = "Error opening pipe in ias_get_disk_usage";
        return undef;
    }

    my $header_line = <PROCPIPE>;
    my $result_line = <PROCPIPE>;

    if (not defined $result_line)
    {
        # if the path doesn't exist, then $header_line will contain an
        # error message from the /bin/df call
        $$error_ref = "Error finding directory $header_line";
        return undef;
    }

    chomp($result_line);

    my @fields = split(' ', $result_line);
    my %entry = ();

    # save the results to a hash that is returned
    $entry{FileSystem} = $fields[0];
    $entry{Type} = $fields[1];
    $entry{Block} = $fields[2];
    $entry{Used} = $fields[3];
    $entry{Available} = $fields[4];
    $entry{PercentUsed} = $fields[5];
    $entry{MountedOn} = $fields[6];

    if (!close(PROCPIPE))
    {
        $$error_ref = "Error closing pipe in ias_get_disk_usage";
        return undef;
    }

    return \%entry;
}

##############################################################################
#
# Name: ias_create_work_order_directory
#
# Description: This routine is used to create a work order directory for the
#              processing system.
#
# Parameters:
#     wo_id --------- The work order id to create a directory for.  This
#                     parameter will also be used as the directory name.
#     wo_path ------- The path to the directory where the work order directory
#                     will be created.  This path is also used to get the disk
#                     statistics.
#     avg_wo_size --- The average size of a work order to detect if the disk
#                     has enough space.
#     wo_dir -------- The reference to a string for returning the path to and
#                     including the work order directory.
#     error_ref ----- The reference to a string for returning errors.
#
# Outputs:
#     wo_dir -------- The reference to a string for returning the path to and
#                     including the work order directory.
#     error_ref ----- An error message or empty string.
#
# Returns: SUCCESS or ERROR
#
##############################################################################
sub ias_create_work_order_directory($$$$$)
{
    my $wo_id = $_[0];       # Input
    my $wo_path = $_[1];     # Input
    my $avg_wo_size = $_[2]; # Input
    my $wo_dir = $_[3];      # Output
    my $error_ref = $_[4];   # Output

    my $save_dir = "";
    my $error_msg = ""; # for any errors from ias_get_disk_usage

    # initialize the returned work order directory string to an empty string
    $$wo_dir = "";
    # initialize the returned error string to an empty string
    $$error_ref = "";

    # get the diskusage for the work order's directory
    my $disk_usage = ias_get_disk_usage($wo_path, \$error_msg);
    if (not defined $disk_usage)
    {
        $$error_ref = "Failed to get disk usage for $wo_path: $error_msg";

        return ERROR;
    }

    # check to see if there is enough diskspace and error if not enough
    my $avail_disk = $disk_usage->{Available} / 1024;
    if ($avail_disk > $avg_wo_size)
    {
        $$wo_dir = $wo_path;
    }
    else
    {
        $$error_ref = "Out of work order disk space: The data disk doesn't " .
                      "have enough space for an average wo size";

        return ERROR;
    }

    # available disk has been identified, build the work order directory

    # build the paths to create

    # verify the work order directory has a trailing '/'.
    if (not ($$wo_dir =~ /\/$/))
    {
        $$wo_dir .= '/'; # it didn't so add it
    }

    $$wo_dir .= $wo_id;
    $save_dir = $$wo_dir . "/save";

    my $current_mask = umask(0000); # save and set the umask

    # create the toplevel work order directory and the save directory
    eval { mkpath([$$wo_dir, $save_dir], 0, 0775) };

    if ($@)
    {
        $$error_ref = "Creating directories $@";

        umask($current_mask); # restore the umask

        return ERROR;
    }

    umask($current_mask); # restore the umask

    return SUCCESS;
}

1;
