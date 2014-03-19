
package ias_odl;

##############################################################################
#    NAME: ias_odl.pm
#
# PURPOSE: This module currently only provides a simple means for extracting
#          parameter values from an ODL file, instead of having specialized
#          code throughout the IAS Perl code.  It is intended that this module
#          would expand and perform other ODL functions as they are needed and
#          implemented.
#
#   NOTES: This is by no means complete.  It assumes valid ODL and no nested
#          groups.  It also loads the whole ODL file into memory.
#
##############################################################################

# System Includes
use strict;
use warnings;
use vars qw(@ISA @EXPORT @EXPORT_OK %EXPORT_TAGS $VERSION);
use Exporter;

# IAS Includes
use ias_const;

@ISA = qw(Exporter);
@EXPORT = qw(
              ODL_TYPE_VALUE
              ODL_TYPE_QUOTED_VALUE
              ODL_TYPE_VALUE_LIST
              ODL_TYPE_QUOTED_VALUE_LIST
            );

# These constants are currently only assigned, they are not used beyond that.
# It seemed simple enough to determine them and assign them.
use constant ODL_TYPE_VALUE => 0;
use constant ODL_TYPE_QUOTED_VALUE => 1;
use constant ODL_TYPE_VALUE_LIST => 2;
use constant ODL_TYPE_QUOTED_VALUE_LIST => 3;

##############################################################################
#        NAME: new
#
# DESCRIPTION: Creates a new ias_odl object.
#
#     RETURNS: reference: The ias_odl object created
#
##############################################################################
sub ias_odl::new
{
    # Grab the class designation off of the parameter list
    my $class = shift;

    my $self = {
        filename => undef,
        contents => undef,
        error_string => ""
    };

    bless($self, $class);

    return $self;
}

##############################################################################
#        NAME: load_odl
#
# DESCRIPTION: Read the specified file and load the internal contents
#              attribute with a hash representing the file contents.
#
#     RETURNS: status: SUCCESS or ERROR
#
##############################################################################
sub ias_odl::load_odl($)
{
    # Self object is passed as the first parameter
    my $self = shift;

    # Grab the filename from the input parameter
    my $filename = shift;

    # Wipeout any current contents
    $self->{contents} = {};

    # Open the file for reading
    if (not open(ODL, "<$filename"))
    {
        $self->{error_string} = "Failed to open file for reading: $!";
        return ERROR;
    }

    # Assign the default group name
    my $group_name = "ROOT";

    # Process through the ODL file
    while (<ODL>)
    {
        my $line = $_;
        $line =~ s/[\r\n]+/\n/g; # Fix DOS formatted files/lines
        chomp($line);

        # Search for a GROUP line to assign the group name
        if ($line =~ /^\s*GROUP\s*=\s*(.+)\s*$/)
        {
            $group_name = $1;
            next; # Skip to next line
        }

        # Search for a END_GROUP line to assign the group name back to ROOT
        if ($line =~ /^\s*END_GROUP\s*=\s*$group_name\s*$/)
        {
            $group_name = "ROOT";
            next; # Skip to next line
        }

        # Search for a list line first because it requires special processing
        # and the following search would be a false hit
        if ($line =~ /^\s*(\w+)\s*=\s*\((.+)\s*$/
            or $line =~ /^\s*(\w+)\s*=\s*\(\s*$/)
        {
            my $parameter_name = $1;
            my $value = "";

            if (defined $2)
            {
                $value = $2;
            }

            # Process until we reach the terminating paren
            # NOTE - This while assumes valid ODL syntax
            while (not $value =~ /.+\)\s*$/)
            {
                $value .= <ODL>;
                $value =~ s/[\r\n]+/\n/g; # Fix DOS formatted files/lines
                chomp($value);
            }

            # Remove the trailing paren and remove any extra spaces
            $value =~ s/\)//g;
            $value =~ s/\s+/ /g;

            # Initially classify the parameter as a value list
            $self->{contents}{$group_name}{$parameter_name}{type} =
                ODL_TYPE_VALUE_LIST;

            # If we have any double quotes remove them and classify the
            # parameter as a quoted value list
            if ($value =~ /\"/)
            {
                $value =~ s/\"//g;
                $self->{contents}{$group_name}{$parameter_name}{type} =
                    ODL_TYPE_QUOTED_VALUE_LIST;
            }

            # Assign the value to the hash
            $self->{contents}{$group_name}{$parameter_name}{value} = $value;

            next; # Skip to next line
        }

        if ($line =~ /^\s*(\w+)\s*=\s*(.+)\s*$/)
        {
            my $parameter_name = $1;
            my $value = $2;

            # Initially classify the parameter as a value
            $self->{contents}{$group_name}{$parameter_name}{type} =
                ODL_TYPE_VALUE;

            # If we have double quotes remove them and classify the parameter
            # as a quoted value
            if ($value =~ /\"(.+)\"/)
            {
                $self->{contents}{$group_name}{$parameter_name}{type} =
                    ODL_TYPE_QUOTED_VALUE;
                $value = $1;
            }

            # Assign the value to the hash
            $self->{contents}{$group_name}{$parameter_name}{value} = $value;

            next; # Skip to next line
        }
    }

    close(ODL);

    return SUCCESS;
}

##############################################################################
#        NAME: get_parameter_value
#
# DESCRIPTION: To return the specified ODL value back to the caller.
#
#  PARAMETERS: string: The full ODL path to the parameter using periods to
#                      separate the group and parameter elements.
#
#     RETURNS: string: The value for the specified parameter or undef
#
##############################################################################
sub ias_odl::get_parameter_value($)
{
    # Self object is passed as the first parameter
    my $self = shift;

    # The parameter to find
    my $parameter_path = shift;

    # Split the path into the group and parameter values
    $parameter_path =~ /^(.+)\.(.+)$/;

    my $group_name = $1;
    my $parameter_name = $2;

    if (not defined $self->{contents}
        or not defined $self->{contents}{$group_name}
        or not defined $self->{contents}{$group_name}{$parameter_name})
    {
        $self->{error_string} = "Missing parameter [$parameter_path]";
        return undef;
    }

    # Return the value back to the caller
    return $self->{contents}{$group_name}{$parameter_name}{value};
}

##############################################################################
#        NAME: set_parameter_value
#
# DESCRIPTION: To set the specified ODL parameter to the provided value.
#
#  PARAMETERS: string: The full ODL path to the parameter using periods to
#                      separate the group and parameter elements.
#              string: The value to assign.
#
#     RETURNS: status: SUCCESS or ERROR
#
##############################################################################
sub ias_odl::set_parameter_value($$)
{
    # Self object is passed as the first parameter
    my $self = shift;

    # The parameter to find
    my $parameter_path = shift;
    my $parameter_value = shift;

    # Split the path into the group and parameter values
    $parameter_path =~ /^(.+)\.(.+)$/;

    my $group_name = $1;
    my $parameter_name = $2;

    if (not defined $self->{contents}
        or not defined $self->{contents}{$group_name}
        or not defined $self->{contents}{$group_name}{$parameter_name})
    {
        $self->{error_string} = "Missing parameter [$parameter_path]";
        return ERROR;
    }

    # Set the value
    $self->{contents}{$group_name}{$parameter_name}{value} = $parameter_value;

    return SUCCESS;
}

##############################################################################
#        NAME: groups
#
# DESCRIPTION: To return a list of the groups contained in the ODL.
#
#     RETURNS: list: The list of all the groups in the ODL or undef
#
##############################################################################
sub ias_odl::groups()
{
    # Self object is passed as the first parameter
    my $self = shift;

    if (not defined $self->{contents})
    {
        $self->{error_string} = "No ODL contents available";
        return undef;
    }

    return keys(%{$self->{contents}});
}

##############################################################################
#        NAME: parameters
#
# DESCRIPTION: To return a list of the parameters contained in the specified
#              ODL group.
#
#  PARAMETERS: string: The group name for the parameters to get.
#
#     RETURNS: list: The list of all the parameters in the group or undef
#
##############################################################################
sub ias_odl::parameters($)
{
    # Self object is passed as the first parameter
    my $self = shift;

    # The group to use
    my $group_name = shift;

    if (not defined $self->{contents}
        or not defined $self->{contents}{$group_name})
    {
        $self->{error_string} = "No ODL contents available for group ["
            . $group_name . "]";
        return undef;
    }

    return keys(%{$self->{contents}{$group_name}});
}

1;
