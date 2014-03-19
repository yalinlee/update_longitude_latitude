
package ias_mail_call;

##############################################################################
#    NAME: ias_mail_call.pm
#
# PURPOSE: To provide a routine that sends a message using mail.
#
##############################################################################

# System Includes
use strict;
use warnings;
use POSIX;

use vars qw(@ISA @EXPORT @EXPORT_OK %EXPORT_TAGS $VERSION);
use Exporter;

# IAS Includes
use ias_const;
use ias_system_call;

@ISA = qw(Exporter);
@EXPORT = qw(
              ias_mail_call
            );

##############################################################################
#        NAME: ias_mail_call
#
# DESCRIPTION: Send an email to a list of users.
#
#  PARAMETERS: string: The subject of the message
#              string: The contents of the message
#              string: A comma seperated list of the email addresses to send
#                      the message to
#
#     RETURNS: status: SUCCESS or ERROR
#              string: An error message or undef
#
##############################################################################
sub ias_mail_call($$$)
{
    my $subject = $_[0];
    my $message = $_[1];
    my $recipients = $_[2];

    my $cmd = "echo '" . $message . "' | mail -s '" . $subject . "' "
        . $recipients;

    my %result_hash = ias_system_call($cmd);
    if ($result_hash{status} != SUCCESS
        || $result_hash{return_value} != SUCCESS)
    {
        my $msg = "Failed to execute mail command [" . $cmd . "]";
        return (ERROR, $msg);
    }

    return (SUCCESS, undef);
}

1;
