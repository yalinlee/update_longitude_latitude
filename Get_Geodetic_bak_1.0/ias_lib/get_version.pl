#!/usr/bin/perl -w
#------------------------------------------------------------------------------
# Name: get_version.pl
#
# Description:	Generates a version string for IAS builds, incorporating
# relevant SVN information into it.
#
#------------------------------------------------------------------------------
use strict;
use Getopt::Long;
use constant IAS_SOFTWARE_VERSION_LENGTH => 10;

# Forward declarations
sub ShowUsage($);
sub ProcessOptions();

# Option variables
my $quote = '';
my $help = '';
my $no_newline = '';

# Main variables
my $wcp; # Your SVN Working copy path
my $svnurl; # Head URL as given by svn info
my $svnrevision; # SVN revision number
my $version = ''; # This is our tagged version.
my $revision;
my $destination = '';
my $isModified = ''; # Whether there are pending changes under the working path.
my $output = '';

ProcessOptions();
# SVN Working copy path is an optional parameter to this routine.
if ($#ARGV == -1)
{
    $wcp = '.'; # Use the current directory, if it wasn't passed.
}
else
{
    $wcp = $ARGV[0];
}

# Avoid "svn info" message about "not a working copy" if it's exported
$svnrevision = `svnversion -n -c $wcp`;
if ($svnrevision ne 'exported')
{
    # Parse the URL given by svn info to see if this is 'tagged' as a type of
    # 'release'.
    $svnurl = `svn info $wcp | grep "^URL: "`;
    chomp $svnurl;
}

if (!$svnurl)
{
    # If it wasn't there, try to get the HeadURL using SVN keyword substitution.
    my $head_url = '$HeadURL: svn://edclxs131/tags/oli_ias/LPGS_2_2_1_OPS/ias_lib/get_version.pl $';
    if ($head_url =~ m!\$HeadURL: (\S+) \$!)
    {
        $svnurl = $1;
    }
}

# See if the url 'matches' a 'release' directory format, e.g. IAS_1_0_1_FQT
if ($svnurl =~
    /(tags|branches)\/oli_ias\/[A-Za-z]{3,4}_(\d{1,3}_\d{1,3}_\d{1,3})(_[A-Za-z]+)?/)
{
    ($version, $destination) = ($2, $3);
    # Replace underscores with "." for the major.minor.dot release format.
    $version =~ s/_/./g;
    # Remove underscore in the output destination.
    $destination =~s/_//g; # Reserved for future use.
}
# Now grab the revision information for the SVN working path.
if ($svnrevision eq 'exported')
{
    # Get the Revision using SVN keyword substitution.
    my $revision_keyword = '$Revision: 31333 $';

    if ($revision_keyword =~ m!\$Revision: (\S+) \$!)
    {
        $revision = $1;
    }
    else
    {
        $revision = 'Unknown';
    }
}
else
{
    $revision = $svnrevision; # Make a copy to "format" with.
    if ($revision =~ /^(\d+):?(\d+)?(M?S?P?)$/)
    {
        if ($2)
        {
            # It's unfortunate there simply won't be enough room to put both  
            # the starting and ending revision numbers in.  We use only the 
            # end of the revision range.
            if ($3)
            {
                # Include the status flags
                $isModified = 1;
                $revision = $2 . $3 . $ENV{USER};
            }
            else
            {
                $revision = $2 . $ENV{USER};
            }
        }
        elsif ($3)
        {
            # Modified, switched and/or sparsely populated.
            $isModified = 1;
            $revision = $1 . $3 . $ENV{USER};
        }
        else
        {
            # Singular revision number (this would be the normal flow)
        }
    }
    else
    {
        # Unexpected format... (let the revision value pass through, as is)
    }
}

if (($version) && (!$isModified))
{
    $output = $version;
}
else
{
    $output = $revision;
}
# Trim it down to 10 characters, if need be...
if (length $output > IAS_SOFTWARE_VERSION_LENGTH)
{
    $output = substr ($output, 0, IAS_SOFTWARE_VERSION_LENGTH);
}

# Ready for output...
print '"' if ($quote);
print $output;
print '"' if ($quote);
print "\n" if (!$no_newline);
exit(0);
#------------------------------------------------------------------------------
# Name: ProcessOptions()
#
# Description:  Using Getopt::Long, this routine parses the argument list,
# flagging any options found in the parameter list.  If an invalid option is
# given or the help option is requested, a usage message is shown and the
# program exits immediately.
#------------------------------------------------------------------------------
sub ProcessOptions()
{
    my $result = GetOptions(
        "quote|q" => \$quote,
        "help|h" => \$help,
        "no-newline|n" => \$no_newline);
    ShowUsage(1) if (!$result);
    ShowUsage(0) if $help;
}
#------------------------------------------------------------------------------
# Name: ShowUsage($)
#
# Description: Shows the help or usage directives for the program.
#
# Exits with parameter value.
#------------------------------------------------------------------------------
sub ShowUsage($)
{
    my $exitvalue = shift; # I: Return code
    print << "EOF";
Utilizing SVN, this program (get_version.pl) creates a version for IAS builds.
Options:
  --no-newline (-n)
        Omits the trailing newline from the output.
  --quote (-q)
        Add doublequotes to the output.
  --help (-h)
        Prints a help summary.
EOF
    exit($exitvalue);
}
