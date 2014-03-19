
package ias_cpf_const;

##############################################################################
#    NAME: ias_cpf_const.pm
#
# PURPOSE: To provide a collection of constants for accessing CPF parameters.
#
##############################################################################

# System Includes
use strict;
use warnings;
use vars qw(@ISA @EXPORT @EXPORT_OK %EXPORT_TAGS $VERSION);
use Exporter;

@ISA = qw(Exporter);
@EXPORT = qw(
              IAS_CPF_Effective_Date_Begin
              IAS_CPF_Effective_Date_End
              IAS_CPF_Alignment_Matrix
            );

use constant IAS_CPF_Effective_Date_Begin =>
    "FILE_ATTRIBUTES.Effective_Date_Begin";
use constant IAS_CPF_Effective_Date_End =>
    "FILE_ATTRIBUTES.Effective_Date_End";

use constant IAS_CPF_Alignment_Matrix => 
    "ATTITUDE_PARAMETERS.ACS_to_OLI_Rotation_Matrix";

1;
