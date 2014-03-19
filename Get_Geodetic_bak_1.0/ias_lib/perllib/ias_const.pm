package ias_const;

##############################################################################
#
# This module is used to provide constants for the IAS/LPGS system.
#
##############################################################################

use strict;
use vars qw(@ISA @EXPORT @EXPORT_OK %EXPORT_TAGS $VERSION);
use Exporter;
@ISA = qw(Exporter);
@EXPORT = qw(
                TRUE
                FALSE
                SUCCESS
                FAILURE
                ERROR
                DB_SUCCESS
            );

# Common processing and error constants.
use constant TRUE    => 1;
use constant FALSE   => 0;
use constant SUCCESS => 0;
use constant FAILURE => 1;
use constant ERROR   => -1;

# The following is used by Landsat specific code.
use constant DB_SUCCESS => 'S';

1;
