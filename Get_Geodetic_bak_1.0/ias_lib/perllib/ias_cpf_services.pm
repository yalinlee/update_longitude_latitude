
package ias_cpf_services;

##############################################################################
#    NAME: ias_cpf_services.pm
#
# PURPOSE: To provide a collection of routines for retrieving information
#          from the CPF service.
#
##############################################################################

# System Includes
use strict;
use warnings;
use vars qw(@ISA @EXPORT @EXPORT_OK %EXPORT_TAGS $VERSION);
use Exporter;

use LWP::UserAgent;
use HTTP::Request;
use HTTP::Request::Common;

# IAS Includes
use ias_const;

@ISA = qw(Exporter);
@EXPORT = qw(
              ias_cpf_service_get_cpf_names_by_date
              ias_cpf_service_get_cpf_names_by_date_range
              ias_cpf_service_get_all_cpf_names_by_source
              ias_cpf_service_get_all_eval_cpf_names
              ias_cpf_service_get_all_prod_cpf_names
              IAS_CPF_SERVICE_NEARMATCH_TRUE
              IAS_CPF_SERVICE_NEARMATCH_FALSE
            );

use constant IAS_CPF_SERVICE_NEARMATCH_TRUE  => "true";
use constant IAS_CPF_SERVICE_NEARMATCH_FALSE => "false";

# The SOAP XML for retrieving a CPF filename using a date
use constant SOAP_CPF_GetAllCpfNamesByDate => <<SOAP_ENV;
<?xml version='1.0' encoding='UTF-8'?>
<soapenv:Envelope
  xmlns:soapenv="http://schemas.xmlsoap.org/soap/envelope/"
  xmlns:cpf="uri:gov/usgs/ias/services/cpf/soap/cpf-2b">
    <soapenv:Header/>
    <soapenv:Body>
        <cpf:getAllCpfNamesByDate>
            <getAllCpfByDateRequest>
                <acquisitionDate>%s</acquisitionDate>
                <status>ACTIVE</status>
                <nearMatch>%s</nearMatch>
            </getAllCpfByDateRequest>
        </cpf:getAllCpfNamesByDate>
    </soapenv:Body>
</soapenv:Envelope>
SOAP_ENV

# The SOAP XML for retrieving a list of CPF filenames using a source
use constant SOAP_CPF_GetAllCpfNames => <<SOAP_ENV;
<?xml version='1.0' encoding='UTF-8'?>
<soapenv:Envelope
  xmlns:soapenv="http://schemas.xmlsoap.org/soap/envelope/"
  xmlns:cpf="uri:gov/usgs/ias/services/cpf/soap/cpf-2b">
    <soapenv:Header/>
    <soapenv:Body>
        <cpf:getAllCpfNames>
            <source>%s</source>
        </cpf:getAllCpfNames>
    </soapenv:Body>
</soapenv:Envelope>
SOAP_ENV

# The SOAP XML for retrieving a list of CPF filenames using a date range
use constant SOAP_CPF_GetAllCpfNamesByDateRange => <<SOAP_ENV;
<?xml version='1.0' encoding='UTF-8'?>
<soapenv:Envelope
  xmlns:soapenv="http://schemas.xmlsoap.org/soap/envelope/"
  xmlns:cpf="uri:gov/usgs/ias/services/cpf/soap/cpf-2b">
    <soapenv:Header/>
    <soapenv:Body>
        <cpf:getAllCpfNamesByDateRange>
            <getAllCpfByDateRangeRequest>
                <startDate>%s</startDate>
                <endDate>%s</endDate>
                <status>ACTIVE</status>
            </getAllCpfByDateRangeRequest>
        </cpf:getAllCpfNamesByDateRange>
    </soapenv:Body>
</soapenv:Envelope>
SOAP_ENV


##############################################################################
#        NAME: ias_cpf_service_cpf_request
#
# DESCRIPTION: Actual call to the CPF service
#
#  PARAMETERS: string: The SOAP XML to send to the service
#
#     RETURNS: status: SUCCESS or ERROR
#              string: undefined or the response
#
##############################################################################
sub ias_cpf_service_cpf_request($)
{
    # Grab the input parameters value
    my $soap_xml = $_[0];

    my $ias_services = $ENV{IAS_SERVICES};
    if ((not defined $ias_services) or ($ias_services eq ""))
    {
        my $msg = "Missing environment variable IAS_SERVICES";
        print STDERR __FILE__ . ": $msg\n";
        return(ERROR, undef);
    }

    # Create the user agent
    my $user_agent = LWP::UserAgent->new();

    # Build the request
    my $service_end_point = $ias_services . "/dataservices/CpfService";
    my $request = HTTP::Request->new(POST => $service_end_point);
    $request->header(SOAPAction =>
                     '"uri:gov/usgs/ias/services/cpf/soap/cpf-2b"');
    $request->content($soap_xml);
    $request->content_type("text/xml; charset=utf-8");

    # Submit the request
    my $response = $user_agent->request($request);

    if ($response->code != 200)
    {
        my $msg = "IAS CPF service error : " . $response->error_as_HTML;
        print STDERR __FILE__ . ": $msg\n";
        return(ERROR, undef);
    }

    return(SUCCESS, $response);
}

##############################################################################
#        NAME: ias_cpf_service_get_cpf_names_by_date
#
# DESCRIPTION: Calls the CPF service to get a list of CPF names by date
#
#  PARAMETERS: string: The date to search for
#                      Example - "2001-10-03T12:20:20Z"
#              string: Wether or not to use near match
#                      Example - IAS_CPF_SERVICE_NEARMATCH_TRUE
#                             or IAS_CPF_SERVICE_NEARMATCH_FALSE
#
#     RETURNS: status: SUCCESS or ERROR
#              string: undefined or the response
#
##############################################################################
sub ias_cpf_service_get_cpf_names_by_date($$)
{
    # Grab the input parameters value
    my $start_date = $_[0];
    my $near_match = $_[1];

    my $soap_xml = sprintf(SOAP_CPF_GetAllCpfNamesByDate,
        $start_date, $near_match);

    my ($status, $response) = ias_cpf_service_cpf_request(
        $soap_xml);

    if ($status != SUCCESS)
    {
        return(ERROR, undef);
    }

    # Get the response and remove a bunch of the garbage from the output
    # Also format to the output we want
    my $response_str = $response->as_string();
    if ($response_str =~ /<cpf>(.*)<\/cpf>/)
    {
        $response_str = $1;
        $response_str =~ s/<cpf>//g;
        $response_str =~ s/<\/cpf>//g;
        $response_str =~ s/<filename>//g;
        $response_str =~ s/<\/filename>/|/g;
        $response_str =~ s/<version>//g;
        $response_str =~ s/<\/version>/|/g;
        $response_str =~ s/<status>//g;
        $response_str =~ s/<\/status>/\n/g;
        # Remove the version since we have it in the CPF name
        $response_str =~ s/\|(\d*)\|/\|/g;
        $response_str =~ s/\n$//g;
    }
    else
    {
        return(ERROR, undef);
    }

    return(SUCCESS, $response_str);
}

##############################################################################
#        NAME: ias_cpf_service_get_cpf_names_by_date_range
#
# DESCRIPTION: Calls the CPF service to get a list of all the CPF names and
#              statuses for a date range
#
#  PARAMETERS: string: The start date to search for
#                      Example - "2001-10-01T00:00:00Z"
#              string: The end date to search for
#                      Example - "2001-10-31T23:59:59Z"
#
#     RETURNS: status: SUCCESS or ERROR
#              string: undefined or the response
#
##############################################################################
sub ias_cpf_service_get_cpf_names_by_date_range($$)
{
    # Grab the input parameters value
    my $start_date = $_[0];
    my $end_date = $_[1];

    my $soap_xml = sprintf(SOAP_CPF_GetAllCpfNamesByDateRange,
        $start_date, $end_date);

    my ($status, $response) = ias_cpf_service_cpf_request(
        $soap_xml);

    if ($status != SUCCESS)
    {
        return(ERROR, undef);
    }

    # Get the response and remove a bunch of the garbage from the output
    # Also format to the output we want
    my $response_str = $response->as_string();
    if ($response_str =~ /<cpf>(.*)<\/cpf>/)
    {
        $response_str = $1;
        $response_str =~ s/<cpf>//g;
        $response_str =~ s/<\/cpf>//g;
        $response_str =~ s/<filename>//g;
        $response_str =~ s/<\/filename>/|/g;
        $response_str =~ s/<version>//g;
        $response_str =~ s/<\/version>/|/g;
        $response_str =~ s/<status>//g;
        $response_str =~ s/<\/status>/\n/g;
        # Remove the version since we have it in the CPF name
        $response_str =~ s/\|(\d*)\|/\|/g;
        $response_str =~ s/\n$//g;
    }
    else
    {
        return(ERROR, undef);
    }

    return(SUCCESS, $response_str);
}

##############################################################################
#        NAME: ias_cpf_service_get_all_cpf_names_by_source
#
# DESCRIPTION: Call the CPF service to get a list of all the CPF names and
#              statuses for a specific source
#
#  PARAMETERS: string: The source of the CPFs to get the names from
#                      Example - "PROD" Production CPFs
#                      Example - "EVAL" Evaluation CPFs
#
#     RETURNS: status: SUCCESS or ERROR
#              string: undefined or the response
#
##############################################################################
sub ias_cpf_service_get_all_cpf_names_by_source($)
{
    # Grab the input parameters value

    my $type = $_[0];

    my $soap_xml = sprintf(SOAP_CPF_GetAllCpfNames, $type);

    my ($status, $response) = ias_cpf_service_cpf_request(
        $soap_xml);

    if ($status != SUCCESS)
    {
        return(ERROR, undef);
    }

    # Get the response and remove a bunch of the garbage from the output
    # Also format to the output we want
    my $response_str = $response->as_string();
    if ($response_str =~ /<return>(.*)<\/return>/)
    {
        $response_str = $1;
        $response_str =~ s/<return>//g;
        $response_str =~ s/<\/return>//g;
        $response_str =~ s/<filename>//g;
        $response_str =~ s/<\/filename>/|/g;
        $response_str =~ s/<version>//g;
        $response_str =~ s/<\/version>/|/g;
        $response_str =~ s/<status>//g;
        $response_str =~ s/<\/status>/\n/g;
        # Remove the version since we have it in the CPF name
        $response_str =~ s/\|(\d*)\|/\|/g;
        $response_str =~ s/\n$//g;
    }
    else
    {
        return(ERROR, undef);
    }

    return(SUCCESS, $response_str);
}

##############################################################################
#        NAME: ias_cpf_service_get_all_eval_cpf_names
#
# DESCRIPTION: A convenience routine to get all of the evaluation CPF names
#              and statuses
#
#     RETURNS: status: SUCCESS or ERROR
#              string: undefined or the response
#
##############################################################################
sub ias_cpf_service_get_all_eval_cpf_names()
{
    my ($status, $response) =
        ias_cpf_service_get_all_cpf_names_by_source("EVAL");

    return($status, $response);
}

##############################################################################
#        NAME: ias_cpf_service_get_all_prod_cpf_names
#
# DESCRIPTION: A convenience routine to get all of the production CPF names and
#              and statuses
#
#     RETURNS: status: SUCCESS or ERROR
#              string: undefined or the response
#
##############################################################################
sub ias_cpf_service_get_all_prod_cpf_names()
{
    my ($status, $response) =
        ias_cpf_service_get_all_cpf_names_by_source("PROD");

    return($status, $response);
}

1;
