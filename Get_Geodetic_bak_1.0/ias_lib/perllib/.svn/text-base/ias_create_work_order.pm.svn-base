package ias_create_work_order;

############################################################################
#   NAME:  ias_create_work_order 
#
#   PURPOSE: The ias_create_work_order was developed to be called
#           by PCS Work Order Generator (PWG) to insert work orders into the
#           work order system through a command line interface
#           As a library routine this may be used to insert work
#           orders by other routines.
#
#   NOTE:   This code was adapted from ALIAS for LDCM use.  This is currently
#           not multi-sensor compatible.
#
############################################################################

use strict;
use vars qw(@ISA @EXPORT @EXPORT_OK %EXPORT_TAGS $VERSION);
use Exporter;       # export functions and variables to users namespaces.
@ISA = qw(Exporter);
@EXPORT = qw(
                ias_create_work_order
            );

use Logger;                 # log routines

use ias_const;              # constant definitions
use ias_wo_const;           # Assign work order constants
use ias_processing_system;  # assign processing system globals
use ias_db;                 # database

# declare some constants
use constant FOUND      =>  1;      # flag indicating found
use constant NOTFOUND   =>  0;      # flag indicating not found
# set debug to 1 to turn on messages and 0 to turn off messages
use constant DEBUG      =>  0;      # Debug flag
use constant PROG       =>  "ias_create_work_order";

###########################################################################
# Name: ias_get_work_order_parameters
#
# Description:  Retrieve the parameters from the database.
#
# Inputs:   (1) procedure ID (module ID in database)
#           (2) satellite/sensor database environment variable
#           (5) logger object
#           (6) database handle
#   
# Outputs:  (3) array of common default parameters
#           (4) array of script default parameters
#
# Returns:  ERROR
#           SUCCESS
#
###########################################################################
sub ias_get_work_order_parameters
{
    my ($procedure_id, $sat_sen_envt_var, 
        $com_parm_arr, $def_parm_arr, $logger, $dbh) = @_;

    my ($sth,             # db query statement handle
        $sql,             # message buffer
        $dbrow,           # array bucket counter
        $parm_name,       # name of current parameter
        $parm_value,      # value of current parameter
        $parm_occurrence, # number of times parameter occurs this function
        $sub_module_id,   # current function name
        $seq_nbr,         # order in which current function is executed
        $global           # flag indicating if parm is a global or not
       );

    # get the sensor specific instance of the database
    my $dbschema = $ENV{$sat_sen_envt_var};

    if (defined $dbschema)
    {
        $dbschema = substr ($dbschema, 0, index($dbschema, '/'));
    }
    else
    {
        $logger->fatal_msg(__FILE__,__LINE__,
            "Unable to retrieve database instance from $sat_sen_envt_var");
        $dbh->rollback;
        return ERROR;
    }

    # get the default global parameters
    $sql = qq{SELECT DISTINCT parm_name, parm_value, parm_occurrence 
              FROM schema.def_parms
              WHERE module_id = '$procedure_id' and global = 1};

    # replace the word schema with the actual sensor database schema name
    $sql =~ s/schema/$dbschema/g;

    $sth = $dbh->prepare($sql);
    if (!$sth)
    {
        $logger->fatal_msg(__FILE__,__LINE__,
                "Unable to prepare the global default parms select statement");
        $dbh->rollback;
        return ERROR;
    }

    if (! ($sth->execute()))
    {
        $logger->fatal_msg(__FILE__,__LINE__,
                "Unable to execute the global default parms select statement");
        $sth->finish;
        $dbh->rollback;
        return ERROR;
    }

    # initialize the counter
    $dbrow = 0;

    # fill the common parms array with global parameters
    while (($parm_name, $parm_value, $parm_occurrence) = $sth->fetchrow_array)
    {
        $$com_parm_arr[$dbrow] = [$parm_name, $parm_value, $parm_occurrence];
        $dbrow++;
    }

    # get the remaining default parms for the module ID
    $sql = qq{SELECT dp.sub_module_id, sm.seq_nbr, dp.parm_name,
                dp.parm_value, dp.parm_occurrence, dp.global
            FROM schema.sub_modules sm, schema.def_parms dp
            WHERE sm.module_id = dp.module_id
            AND sm.sub_module_id = dp.sub_module_id
            AND sm.module_id = '$procedure_id'
            ORDER BY sm.seq_nbr};

    # replace the word schema with the actual sensor database schema name
    $sql =~ s/schema/$dbschema/g;

    $sth = $dbh->prepare($sql);
    if (!$sth)
    {
        $logger->fatal_msg(__FILE__,__LINE__,
                "Unable to prepare the default parameters select statement");
        $dbh->rollback;
        return ERROR;
    }

    if (! ($sth->execute()))
    {
        $logger->fatal_msg(__FILE__,__LINE__,
                "Unable to execute the default parameters select statement");
        $sth->finish;
        $dbh->rollback;
        return ERROR;
    }

    # initialize the counter
    $dbrow = 0;

    # populate the default parameter array
    while (($sub_module_id, $seq_nbr, $parm_name, $parm_value, 
            $parm_occurrence, $global) = $sth->fetchrow_array)
    {
        $$def_parm_arr[$dbrow] = [$procedure_id, $sub_module_id, $seq_nbr, 
                              $parm_name, $parm_value, $parm_occurrence, 
                              $global];        
        $dbrow++;
    }

    return SUCCESS;
} # end of ias_get_work_order_parameters

###########################################################################
# Name: parse_manual_params
#
# Description:  Parse the parameters array to manually override defaults
#
# Inputs:   (1) array of manual parameter definitions to use
#           (2) logger object
#   
# Outputs:  (3) array of script parameter values
#           (4) array of script parameter flags
#           (5) array of script parameter list info for setting lists
#
# Returns:  ERROR
#           SUCCESS
#
###########################################################################
sub parse_manual_params
{
    my ($parms, $logger, $script_parms_value, $script_parms_used,
            $parm_list_occurrences_to_unset) = @_;

    foreach (@{$parms})
    {
        # use .* for value since a variety of inputs can be provided (numeric,
        # strings, filenames with extensions, lists, etc) and also allows to
        # name a parameter and purposely not specify a value, providing a
        # mechanism to get the parameter to ultimately be NULL in the DB
        if (m/(\w+):(\w+)=(.*)/)
        {
            my $script_name = uc $1;
            my $parm_name = uc $2;
            my $parm_occurrence = 1;
            my $parm_value = $3;
            my @list_values = ();

            # if parameter occurrence (for params with lists) is present,
            # split that from the parameter name
            if ($parm_name =~ /(\w+)_ELEM(\d+)/)
            {
                $parm_name = $1;
                $parm_occurrence = $2;
            }

            # if parameter value given is a list, use that for the multiple
            # occurrences of the parameter - unset all occurrences after
            # given values too
            if ($parm_value =~ /\((.*)\)/)
            {
                # split up the list components
                @list_values = split ',', $1;
                my $list_parm_occurrence = 1;
                foreach my $list_item (@list_values)
                {
                    if ($list_item eq '')
                    {
                        # Make sure no blank list items were found (this
                        # would be due to two commas together in the input
                        # list values given for the parameter), as this is
                        # not known to be needed for any parameters, so is
                        # checked for and treated as an error
                        $logger->fatal_msg(__FILE__, __LINE__,
                         "Script parameter "
                         . "$script_name:$parm_name\_ELEM$list_parm_occurrence "
                         . "not valid");
                        return ERROR;
                    }

                    # save the parameter value into the structure and set a
                    # flag that can later be used to warn for parameters
                    # provided that weren't used
                    $$script_parms_value{$script_name}{$parm_name}
                        {$list_parm_occurrence} = $list_item;
                    $$script_parms_used{$script_name}{$parm_name}
                        {$list_parm_occurrence} = 0;

                    $list_parm_occurrence++;
                }

                # set flag for the next list_parm_occurrence in case there
                # were default values after those given for this parameter
                # list, so later when overriding the values, we can unset the
                # remaining list elements that were left out of provided list
                $$parm_list_occurrences_to_unset{$script_name}{$parm_name}
                    = $list_parm_occurrence;
            }
            else
            {
                # save the parameter value into the structure and set a flag
                # that can later be used to warn for parameters provided that
                # weren't used
                $$script_parms_value{$script_name}{$parm_name}{$parm_occurrence}
                    = $parm_value;
                $$script_parms_used{$script_name}{$parm_name}{$parm_occurrence}
                    = 0;
            }
        }
        else
        {
            # The parameter definition string did not match expected format
            $logger->fatal_msg(__FILE__, __LINE__,
                    "Invalid parameter definition ($_) does not match format "
                    . "'SCRIPT_NAME:PARAM_NAME=value'");
            return ERROR;
        }
    }

    # debugging to check the parameters that were provided manually
    if (DEBUG)
    {
        foreach my $script (keys %{$script_parms_value})
        {
            print "Parameters for script $script:\n";
            foreach my $parm (keys %{$$script_parms_value{$script}})
            {
                foreach my $occur (sort keys
                        %{$$script_parms_value{$script}{$parm}})
                {
                    print "  $parm\[$occur\] = "
                        . $$script_parms_value{$script}{$parm}{$occur} . "\n";
                }
                if (exists $$parm_list_occurrences_to_unset{$script}{$parm})
                {
                    print "need to unset any remaining list items for "
                        . "$script:$parm starting at list element occurrence "
                        . "$$parm_list_occurrences_to_unset{$script}{$parm}\n";
                }
            }
        }
    }

    return SUCCESS;
} # end parse_manual_params

###########################################################################
# Name: parse_script_options
#
# Description:  Parse the script options array to extract pause and skip 
#               options to manually override their default values.
#
# Inputs:   (1) array of manual script options to use
#           (2) logger object
#   
# Outputs:  NONE
#
# Returns:  (1) status of parsing operation
#           (2) hash array of script pause flags 
#           (3) hash array of script states
#
###########################################################################
sub parse_script_options
{
    my ($script_options, $logger) = @_;
    my $err_count = 0;

    # options extracted are returned as two associative arrays.
    my %script_pause_flags = ();
    my %script_states = ();

    foreach (@{$script_options})
    {
        # script option format is simply SCRIPT_NAME:option
        # (current options are SKIP or PAUSE)
        if (m/(\w+):(SKIP|PAUSE)$/)
        {
            my $script_name = uc $1;
            my $option = uc $2;
            
            if ($option eq 'SKIP')
            {
                $script_states{$script_name} = 'K';
            } 
            elsif ($option eq 'PAUSE')
            {
                $script_pause_flags{$script_name} = 1;
            }
            else
            {
                # The option did not match expected values
                $logger->fatal_msg(__FILE__, __LINE__,
                        "Invalid script option ($_) does not match expected "
                        . "values (SKIP or PAUSE)");
                $err_count++;
            }
        }
        else
        {
            # The parameter definition string did not match expected format
            $logger->fatal_msg(__FILE__, __LINE__,
                    "Invalid script option ($_) does not match format "
                    . "'SCRIPT_NAME:(SKIP|PAUSE)'");
            $err_count++;
        }
    }

    # if any of the script options were invalid, stop
    if ($err_count > 0)
    {
        $logger->fatal_msg(__FILE__, __LINE__,
               "Creation of work order halted due to invalid script option(s)");
        return ( ERROR, undef, undef );
    }

    if (DEBUG)
    {
        my $script_name;
        my $state;
        my $pause;
        while (($script_name,$state) = each(%script_states))
        {
            print "  script_state: $script_name=$state\n";
        }
        while (($script_name,$pause) = each(%script_pause_flags))
        {
            print "  script_pause_flag: $script_name=$pause\n";
        }
    }
    return (SUCCESS, \%script_pause_flags,\%script_states);

} # end parse_script_options

###########################################################################
# Name: insert_params_for_wo
#
# Description:  Insert the parameters for the work order from the arrays
#
# Inputs:   (1) default parameter definitions from DEF_PARMS
#           (2) work order ID
#           (3) array of script parameter values
#           (4) array of script parameter flags
#           (5) array of script parameter list info for setting lists
#           (6) database handle
#           (7) logger object
#           (8) hash of script pause flags
#           (9) hash of script states
#   
# Returns:  ERROR
#           SUCCESS
#
###########################################################################
sub insert_params_for_wo
{
    my ($script_parm_arr, $wo_id, $script_parms_value, $script_parms_used,
            $parm_list_occurrences_to_unset, $dbh, $logger, 
            $script_pause_flags, $script_states) = @_;

    my $index;
    my $sql;
    my %script_pause_flags = %$script_pause_flags;
    my %script_states = %$script_states;
    my %script_pause_flags_used = ();
    my %script_states_used = ();

    # initialize this scalar to empty string so we will insert the first sub
    # module into the work order scripts table, if the name changes in the
    # order message we will pick it up and create a record for that one also.
    my $sub_module = '';

    # loop through the array and insert the sub_module and associated 
    # parameters & values into the work order scripts table.
    for ($index = 0; $index <= $#{$script_parm_arr}; $index++)
    {
        my $common = 'COMMON';
        my $param_name;
        my $param_value;
        my $param_occurrence;

        if ($sub_module ne $$script_parm_arr[$index][1])
        {
            $sub_module = $$script_parm_arr[$index][1];

            # Flag if a pause is needed between sub-modules
            my $pause_flag = 0;

            if (exists $script_pause_flags{$sub_module})
            {
                $pause_flag = $script_pause_flags{$sub_module};
                $script_pause_flags_used{$sub_module} = 1;
            }

            # State of sub-module (could be SKIP)
            my $state = 'P';
            if (exists $script_states{$sub_module})
            {
                $state = $script_states{$sub_module};
                $script_states_used{$sub_module} = 1;
            }

            # we got a new sub module so insert it
            $sql = qq{INSERT INTO wo_scripts
                                 (work_order_id, script_id, 
                                  pause_flag, seq_nbr, state)
                    VALUES ('$wo_id', '$sub_module',
                            $pause_flag, $$script_parm_arr[$index][2], 
                            '$state')};
            if (DEBUG)
            {
                print "SQL:  $sql\n";
            }
             
            if (not $dbh->do($sql))
            {
                $logger->fatal_msg(__FILE__,__LINE__,
                        "Error inserting work order script: $DBI::errstr");
                return ERROR;
            }
        }

        # These were the defaults from the database of the common default
        # parms and a list of default parms for the given procedure ID
        $param_name = $$script_parm_arr[$index][3];
        $param_value = $$script_parm_arr[$index][4];
        $param_occurrence = $$script_parm_arr[$index][5];

        # check for common parameters (global).
        # if the current parameter is a global parameter and we got a manual
        # override value for it, then set it
        if ($$script_parm_arr[$index][6] == 1)
        {
            if (exists $$script_parms_value{$common}{$param_name}
                    {$param_occurrence})
            {
                $param_value = $$script_parms_value{$common}{$param_name}
                    {$param_occurrence};

                # set flag that this manual input for the parameter was used
                $$script_parms_used{$common}{$param_name}{$param_occurrence}
                    = 1;
            }

            # now unset remaining items of common list parameters, when needed
            if (exists $$parm_list_occurrences_to_unset{$common}{$param_name}
                    and $param_occurrence
                    >= $$parm_list_occurrences_to_unset{$common}{$param_name})
            {
                $param_value = '';
            }
        }

        # override default value (or global previously set) if
        # script-specific value was provided
        if (exists $$script_parms_value{$sub_module}{$param_name}
                {$param_occurrence})
        {
            $param_value = $$script_parms_value{$sub_module}{$param_name}
                {$param_occurrence};

            # set flag that this manual input for the parameter was used
            $$script_parms_used{$sub_module}{$param_name}{$param_occurrence}
                = 1;
        }

        # when a list was provided for a parameter, the n number of elements
        # given in the list are assigned to the first n occurrences of the
        # parameter when overriding the defaults. Any remaining occurrences of
        # the parameter that were not given a value in the provided list are
        # unset, as determined by earlier parsing of the list. The remaining
        # occurrences not set results in those getting set to NULL in the DB.
        if (exists $$parm_list_occurrences_to_unset{$sub_module}{$param_name}
                and $param_occurrence
                >= $$parm_list_occurrences_to_unset{$sub_module}{$param_name})
        {
            $param_value = '';
        }

        # the parameter's value will be NULL in the DB if not assigned 
        if (not defined($param_value))
        {
            $param_value = '';
        }

        # write parameters to the work order parameters table
        $sql = qq{INSERT INTO wo_parms (work_order_id, script_id, 
                                parm_name, parm_value, parm_occurrence)
                       VALUES ('$wo_id', '$sub_module',
                               '$param_name', '$param_value', 
                               $param_occurrence)};

        if (DEBUG)
        {
            print "SQL:  $sql\n";
        }

        if (not $dbh->do($sql))
        {
            $logger->fatal_msg(__FILE__,__LINE__,
                    "Error inserting work order parameter: $DBI::errstr");
            return ERROR;
        }
    } # end of for loop

    # error when parameters were provided to override defaults, but not
    # used due to not matching any script/param expected - this is an invalid
    # definition of the parameters for the work order, so no reason to run
    # the order to find out when it's done that a parameters was mis-typed or
    # something and the work order would need to be re-run.
    my $parm_errs = 0;
    foreach my $script (keys %{$script_parms_used})
    {
        foreach my $parm (keys %{$$script_parms_used{$script}})
        {
            foreach my $occur (sort keys %{$$script_parms_used{$script}{$parm}})
            {
                if (not $$script_parms_used{$script}{$parm}{$occur})
                {
                    $logger->fatal_msg(__FILE__, __LINE__,
                            "Script parameter $script:$parm\_ELEM$occur not "
                            . "valid");
                    $parm_errs++;
                }
            }
        }
    }

    # perform a similar check on the script options.
    my $script_name;
    my $state;
    my $pause;
    while (($script_name,$state) = each(%script_states))
    {
        if (not $script_states_used{$script_name})
        {
            $logger->fatal_msg(__FILE__, __LINE__,
                    "Script $script_name not valid to SKIP");
            $parm_errs++;
        }
    }
    while (($script_name,$pause) = each(%script_pause_flags))
    {
        if (not $script_pause_flags_used{$script_name})
        {
            $logger->fatal_msg(__FILE__, __LINE__,
                    "Script $script_name not valid to PAUSE");
            $parm_errs++;
        }
    }

    # if any of the parameters or options were invalid, stop
    if ($parm_errs > 0)
    {
        $logger->fatal_msg(__FILE__, __LINE__,
                "Creation of work order halted due to invalid parameter(s)");
        return ERROR;
    }

    return SUCCESS;
} # end insert_params_for_wo

###########################################################################
# Name: insert_work_order
#
# Description:
#       Builds a valid work order from parameters provided and 
#       inserts it into the work order table schema for processing.
#
# Inputs:
#       Logger
#       Procedure ID
#       Scene ID
#       Order Id
#       Order Unit Number
#       L0R Data Path
#       Script Parm Overrides
#       Priority Flag
#       Out DB Schema
#       Work Order Comment
#       Common Database Connection
#       Caltest ID 
#       Requester
#       System Id
#       Satellite
#       Sensor
#       Array of Common Default Parameter Values
#       Array of Script Default Parameter Values
#       Array of Script Options
#
# Returns:
#       Errors     Undefined
#       Success    Work Order Id
#
# Outputs:
#       Prints error messages to the log file
#       Valid work order inserted into work order tables
#
# Notes:
#       The script_override_parameters are to be specified in this format:
#       SCRIPT_NAME:PARM_NAME=value
#       * script and parameter names use all uppercase
#       * define common/global parameters through the use of 'script' name
#         "COMMON"
#       * file names must contain the fully qualified path and file name
#       * specifying a parameter name with absence of a 'value' has the effect
#         of setting the parameter value to NULL in DB
#       * use '_ELEM#' (1-based) appended to parameter name to set the value
#         of a specific element for list parameters
#       * lists can be given for a parameter with (x,y,z) format
#          > either all list elements must be specified or
#          > a subset of list elements can be specified as the first 'n'
#            elements of the list; if all possible list values are not given,
#            tailing elements of the list not specified are considered to have
#            been left blank (set to NULL in DB); e.g. (10,11) given for a
#            BAND_LIST parameter has the effect of setting band list element
#            1 being '10', element 2 being '11', and all remaining 9 elements
#            of the band list not specified are set to NULL in DB
#       * the number of elements for a given parameter can vary in definition
#         across procedures and cannot be increased by providing more elements
#         than defined for each procedure
#       * invalid parameter definitions cause fatal error
#       * Examples:
#         DGC:GCP_TYPE=GLS
#         L8RPS:GENERATE_REPORT=1
#         L8RPS:BAND_LIST_ELEM11=
#         L8RPS:BAND_LIST=(1,2,3,4)
#         COMMON:STORE_CHARACTERIZATION_IN_DB=0
#         L8RPS:ALGORITHMS_ELEM16=reflectance_conversion_SKIP
###########################################################################
sub insert_work_order
{
    my ($logger, $procedure_id, $scene_id, $order_id, $unit_num,
            $l0r_data_path, $script_override_params, $wopriority,
            $out_db_schema_input, $wo_comment_input, $dbh, $caltest_id,
            $requester, $sys_id, $satellite, $sensor, $com_parm_arr,
            $script_parm_arr, $script_options ) = @_;

    # set defaults for the work order
    my $archived = 0;       # Work order not archived
    my $deleted = 0;        # Work order not deleted
    my $request_type = DB_REQUEST_TYPE_STANDARD; # Standard request type
    my $state = DB_WO_L0R_NEW; # L0R New state
    my $wo_comment = 'Manual Work Order'; # Manual comment
    my $out_db_schema = 'P';# Default output schema to Production
    my $wo_priority = 1;    # Default priority
    my $l0r_data_status = undef;  # Default status
    my $product_request_id = undef;
    my @script_override_params = ();
    if (defined @{$script_override_params})
    {
        @script_override_params = @{$script_override_params};
    }
    my @script_options = ();
    if (defined @{$script_options})
    {
        @script_options = @{$script_options};
    }
    # declare some more local vars
    my $param_value;
    my $sql;
    my $wo_seq;
    my $wo_id;
    my $index;
    my %script_parms_value = ();
    my %script_parms_used = ();
    my %parm_list_occurrences_to_unset = ();
    my %script_pause_flags = ();
    my %script_states = ();

    # verify that the scene ID is not NULL.  Scene ID is required.
    if (!$scene_id)
    {
        $logger->fatal_msg(__FILE__,__LINE__,
            "The required parameter scene_id is not included with arguments");
        $dbh->rollback;
        return undef;
    }

    # if the input comment parameter is not null, replace the generic message
    if ($wo_comment_input)
    {
        $wo_comment = $wo_comment_input;
    }

    # if the work order priority is not null, replace the default
    if ($wopriority)
    {
        $wo_priority = $wopriority;
    }

    # if the input work order database schema is not null, replace the 
    # default
    if ($out_db_schema_input)
    {
        $out_db_schema = $out_db_schema_input;
    }

    # if the caltest order ID is null, initialize it
    if (!$caltest_id)
    {
        $caltest_id = "";
    }

    # get the next sequence number for the work order
    $sql = qq{SELECT wo_seq.NEXTVAL FROM dual};

    if (! ($wo_seq = $dbh->selectrow_array($sql)))
    {
        $logger->fatal_msg(__FILE__,__LINE__,
                "Unable to get next work order ID from the Database");
        $dbh->rollback;
        return undef;
    }
    # set the actual work order ID
    $wo_id = sprintf("%s%s", $sys_id, $wo_seq);

    $logger->info_msg(__FILE__,__LINE__,
            "Inserting manual work order $wo_id");

    # initialize the product_request_id
    # set parameters if not defined
    # If one is undefined, then both are undefined or should be.
    if ((not defined $order_id) || (not defined $unit_num))
    {
        # Set these correctly for manually entered orders and orders not
        # from the TRAM.
        $order_id = '';
        $unit_num = '';
        # product_request_id has to be set to something for LPGS so just use
        # the work order ID, since it also has to be unique.
        # We don't care if it is set in IAS, because we do not use the
        # product_request_id column in IAS.
        $product_request_id = $wo_id;
    }
    else
    {
        if ($unit_num =~ /^[0-9]{1,5}$/)
        {
            # define the product_request_id
            $product_request_id = sprintf("%s\_%05d", $order_id, $unit_num);
        }
        else
        {
            $logger->fatal_msg(__FILE__, __LINE__, "Unit nbr ($unit_num) "
                    . "is a non-numeric value");
            return undef;
        }
    }

    # set l0r_data_path and data status if defined
    if (defined $l0r_data_path)
    {
        # set status to local
        $l0r_data_status = DB_L0R_DATA_STATUS_LOCAL;
        $state = DB_WO_PENDING;
    }
    else
    {
        $l0r_data_path = "";
        $l0r_data_status = DB_L0R_DATA_STATUS_SUBSETTER;
        $state = DB_WO_L0R_NEW;
    }

    # insert the work order
    if ($isIAS)
    {
    $sql = qq{INSERT INTO work_orders
               (work_order_id, archived, date_entered, deleted,
                priority, requester, request_type, state, act_compl_date,
                date_l0R_requested, ephem_type,
                req_start_date, customer_order_nbr, customer_unit_nbr,
                procedure_id, product_request_id,
                cpf_override, ephem_file, wo_comment, wo_path, cpf_name,
                satellite, sensor, scene_id, l0r_data_path, caltest_orders_id,
                l0r_data_status, output_db_schema)
            VALUES
               ('$wo_id', $archived, sysdate, $deleted, 
                $wo_priority, '$requester', '$request_type', '$state', NULL, 
                NULL, NULL,
                sysdate, '$order_id', '$unit_num',
                '$procedure_id', '$product_request_id', 
                NULL, NULL, '$wo_comment', NULL, NULL,
                '$satellite', '$sensor', '$scene_id','$l0r_data_path',
                '$caltest_id', '$l0r_data_status', '$out_db_schema')};
    }
    else # isLPGS - there is no caltest_orders_id
    {
    $sql = qq{INSERT INTO work_orders
               (work_order_id, archived, date_entered, deleted,
                priority, requester, request_type, state, act_compl_date,
                date_l0R_requested, ephem_type,
                req_start_date, customer_order_nbr, customer_unit_nbr,
                procedure_id, product_request_id,
                cpf_override, ephem_file, wo_comment, wo_path, cpf_name,
                satellite, sensor, scene_id, l0r_data_path, 
                l0r_data_status, output_db_schema)
            VALUES
               ('$wo_id', $archived, sysdate, $deleted, 
                $wo_priority, '$requester', '$request_type', '$state', NULL, 
                NULL, NULL,
                sysdate, '$order_id', '$unit_num',
                '$procedure_id', '$product_request_id', 
                NULL, NULL, '$wo_comment', NULL, NULL,
                '$satellite', '$sensor', '$scene_id','$l0r_data_path',
                '$l0r_data_status', '$out_db_schema')};
    }

    if (DEBUG)
    {
        print "SQL: $sql\n";
    }

    # perform the work order insert
    if (! $dbh->do($sql))
    {
        $logger->fatal_msg(__FILE__,__LINE__,
                "Unable to insert into the work orders table");
        $dbh->rollback;
        return undef;
    }

    # insert the common parameters here.
    for ($index = 0; $index <= $#{$com_parm_arr}; $index++)
    {
        if (defined $$com_parm_arr[$index][1])
        {
            $param_value = $$com_parm_arr[$index][1];
        }
        else
        {
            $param_value = "";
        }

        $sql = qq{INSERT INTO wo_common_parms 
                             (work_order_id, parm_name, 
                              parm_value, parm_occurrence)
                  VALUES ('$wo_id', '$$com_parm_arr[$index][0]', 
                          '$param_value', $$com_parm_arr[$index][2])};
        if (DEBUG)
        {
            print "SQL: $sql\n";
        }

        if (! $dbh->do($sql))
        {
            $logger->fatal_msg(__FILE__,__LINE__,
                    "Unable to insert into the WO Common Parms table");
            $dbh->rollback;
            return undef;
        }

    } # end of for count of com_parm_arr

    # If manual input was provided to override default parameters,
    # process them out of the input array and prepare them for use
    if (@script_override_params)
    {
        if (parse_manual_params(\@script_override_params, $logger,
                    \%script_parms_value, \%script_parms_used,
                    \%parm_list_occurrences_to_unset) != SUCCESS)
        {
            $logger->fatal_msg(__FILE__, __LINE__,
                    "Failure parsing manually provided parameters");
            $dbh->rollback;
            return undef;
        }
    }
   
    # If manual script options were provided to override defaults,
    # process them out of the input array and prepare them for use
    if (@script_options)
    {
        my ( $status, $script_pause_flags, $script_states ) = 
               parse_script_options(\@script_options, $logger);
        if ($status != SUCCESS)
        {
            $logger->fatal_msg(__FILE__, __LINE__,
                    "Failure parsing manually provided script options");
            $dbh->rollback;
            return undef;
        }
        %script_pause_flags = %$script_pause_flags;
        %script_states = %$script_states;
    }
   
    # Set up the parameters for the work order in the database using the
    # defaults from the database and overriding the defaults where user
    # input was provided for specific script parameters
    if (insert_params_for_wo($script_parm_arr, $wo_id, \%script_parms_value,
                \%script_parms_used, \%parm_list_occurrences_to_unset, $dbh,
                $logger, \%script_pause_flags, \%script_states) != SUCCESS)
    {
        $logger->fatal_msg(__FILE__, __LINE__,
                "Error inserting work order parameters in database");
        $dbh->rollback;
        return undef;
    }

    # commit the work order record to the database
    if (!$dbh->commit)
    {
        # try rolling back
        $dbh->rollback();
        $logger->fatal_msg(__FILE__,__LINE__,
                "Error committing work order: $DBI::errstr");
        return undef;
    }
    
    return ($wo_id);
} # end of insert_work_order

############################################################################
# Name: ias_create_work_order 
#
# Description: Insert a work order into the database.
#
# Inputs:
#   Required:
#       Logger
#       Procedure ID
#       Scene ID
#   Optional:
#       Order ID
#       Order Unit Number
#       L0R Data Path
#       Script Parm Overrides
#       Priority Flag
#       Out DB Schema
#       Work Order Comment
#       Common Database Connection
#       Caltest ID
#       Requester
#
# Returns:
#       Work order ID or undef
#       On success, the work order database is updated with a work order.
#
############################################################################
sub ias_create_work_order($$$%)
{
    my $logger = shift;
    my $procedure_id = shift;
    my $scene_id = shift;
    my %args = @_;

    ##########################################################################
    # Variable declarations
    ##########################################################################

    # assign optional parameters
    my $order_id = $args{'order_id'};
    my $unit_number = $args{'unit_number'};
    my $l0r_data_path = $args{'l0r_data_path'};
    my @script_override_params = ();
    if (defined $args{'parms'})
    {
        @script_override_params = @{$args{'parms'}};
    }
    my @script_options = ();
    if (defined $args{'script_options'})
    {
        @script_options = @{$args{'script_options'}};
    }
    my $wo_priority = $args{'wo_priority'};
    my $output_db_schema = $args{'out_db_schema'};
    my $wo_comment = $args{'wo_comment'};
    my $dbhcommon = $args{'dbhcommon'};
    my $caltest_id = $args{'caltest_id'};
    my $requester = $args{'requester'};

    my $sql;                # SQL statement
    my $rtn;                # Subroutine return value
    my $count;              # Count to ensure procedure is valid - should be (1)

    my $sys_id;             # System ID
    my @script_parm_arr;    # Array to contain default script parms
    my @com_parm_arr;       # Array to contain default common parms 

    my $ias_error_msg;

    if (DEBUG)
    {
        print "PROCEDURE ID: $procedure_id\n";
        print "SCENE ID: $scene_id\n";
        print "ORDER ID: $order_id\n" if ($order_id);
        print "UNIT NUMBER: $unit_number\n" if ($unit_number);
        print "L0R DATA PATH: $l0r_data_path\n";
        my $n = 1;
        foreach (@script_override_params)
        {
            print "PARM$n: $_\n";
            $n++;
        }
        print "PRIORITY: $wo_priority\n" if ($wo_priority);
        print "OUT DB SCHEMA: $output_db_schema\n" if ($output_db_schema);
        print "COMMENT: $wo_comment\n";
        print "DB HANDLER: $dbhcommon\n" if ($dbhcommon);
        print "CALTEST ID: $caltest_id\n" if ($caltest_id);
        print "REQUESTER: $requester\n" if ($requester);
    }

    # test the scene ID name, verify that the satellite name and sensor are
    # identifiable
    my $sat_name;
    my $sensor;
    my $sat_sen_envt_var;
    if (!defined $scene_id)
    {
        $logger->fatal_msg(__FILE__, __LINE__,
                "Required parameter Scene ID is not defined.");
        return undef;
    }
    else
    {
        # get the satellite name.  This is a stub for other satellites/sensors.
        # the first and third character make up the satellite
        $sat_name = join("",(substr($scene_id, 0, 1), substr($scene_id, 2, 1)));

        # the second character represents the sensor
        if ($sat_name =~ m/^L8$/)
        {
            # fill in the work orders table with the OLITIRS sensor
            $sensor = "OLITIRS";
        }
        # this else condition should not be reached as all satellites/sensors
        # that are not L8 are not supported in this routine
        else
        {
            $logger->fatal_msg(__FILE__, __LINE__,
                    "Invalid Scene ID.  Satellite $sat_name is not supported.");
            return undef;
        }

        # Set up the sensor-specific environment variable name
        $sat_sen_envt_var = "IAS_DB_${sat_name}_${sensor}";
    }

    # define list of environment variables needed to run the application
    my @vars_list = qw (
            ORACLE_SID
            TWO_TASK
            IAS_DB_COM
            );

    # verify that the environment variables are defined
    if (ias_validate_environment(\@vars_list))
    {
        $logger->fatal_msg(__FILE__, __LINE__, $ias_error_msg);
        return undef;
    }

    # connect to the common database if it isn't already
    my $dbhflag = FOUND;

    if (!$dbhcommon)
    {
        $dbhflag = NOTFOUND;
        # get the username and password for the common database
        my ($user_id, $password) = ias_db_GetDBConnectionInfo();

        # connect to the database
        $dbhcommon = ias_db_ConnectToDB($user_id, $password);
        if (!$dbhcommon)
        {
            $logger->fatal_msg(__FILE__, __LINE__,
                    "Error connecting to LDCM database");
            return undef;
        }
    }   # end of dbhcommon connect

    # check if the Procedure ID is defined and if it is valid
    if (!defined $procedure_id)
    {
        $logger->fatal_msg(__FILE__, __LINE__,
                "Required parameter Procedure ID is not defined.");
        if (!$dbhflag) { $dbhcommon->disconnect; }
        return undef;
    }
    else
    {
        # get the sensor/satellite specific instance.  
        my $dbschema = $ENV{$sat_sen_envt_var};
        
        if (defined $dbschema)
        {
            # new string will include everything in the original string 
            # before the / symbol
            $dbschema = substr ($dbschema, 0, index($dbschema, '/'));
        }
        else
        {
            $logger->fatal_msg(__FILE__, __LINE__,
                "Unable to retrieve database instance from $sat_sen_envt_var");
            if (!$dbhflag) { $dbhcommon->disconnect; }
            return undef;
        }

        # verify the procedure name given is valid
        $sql = qq{SELECT count(*) FROM schema.modules
            WHERE module_id = '$procedure_id' and mod_type='PROCEDURE'};

        $sql =~ s/schema/$dbschema/g;
        
        $count = $dbhcommon->selectrow_array($sql);
        if (!defined($count) || $count < 1)
        {
            $logger->fatal_msg(__FILE__, __LINE__,
                    "Unable to validate input procedure name $procedure_id");
            if (!$dbhflag) { $dbhcommon->disconnect; }
            return undef;
        }
    }

    # this routine queries the database to get a list of
    # common default parms and a list of default parms for the given
    # procedure ID
    $rtn = &ias_get_work_order_parameters($procedure_id, $sat_sen_envt_var,
            \@com_parm_arr, \@script_parm_arr, $logger, $dbhcommon);

    # the log file is updated in routine so disconnect database connection 
    # and return error
    if ($rtn != SUCCESS)
    {
        if (!$dbhflag) { $dbhcommon->disconnect; }
        return undef;
    }

    # get the user name of the person requesting the work order if not
    # specified on the command line
    if (!defined $requester)
    {
        $requester = getpwuid($<);
        if (!defined $requester)
        {
            $logger->fatal_msg(__FILE__, __LINE__,
                    "USER not set, unable to get UNIX user name");
            if (!$dbhflag) { $dbhcommon->disconnect; }
            return undef;
        }
    }

    # get the system ID, IAS (I) or LPGS (L)
    $sys_id = $ENV{IAS_SYSTEM_ID};

    if (!defined $sys_id)
    {
        $logger->fatal_msg(__FILE__, __LINE__,
                "IAS_SYSTEM_ID not set, unable to get system ID");
        if (!$dbhflag) { $dbhcommon->disconnect; }
        return undef;
    }

    # capitalize and get the first letter
    $sys_id =~ s/(\w)\w*/\U$1/;

    # call to enter work order into the database
    my $wo_id = insert_work_order($logger, $procedure_id, $scene_id,
            $order_id, $unit_number, $l0r_data_path, \@script_override_params,
            $wo_priority, $output_db_schema, $wo_comment, $dbhcommon,
            $caltest_id, $requester, $sys_id, $sat_name, $sensor,
            \@com_parm_arr, \@script_parm_arr, \@script_options);

    # if an error occurred from insert_work_order, return error
    if (!defined($wo_id))
    {
        if (!$dbhflag) { $dbhcommon->disconnect; }
        return undef;
    }

    # commit the work order record to the database
    if (DEBUG)
    {
        $dbhcommon->rollback;
    }
    else
    {
        if (!$dbhcommon->commit)
        {
            $logger->fatal_msg(__FILE__, __LINE__,
                    "Work Order record commit failed");
            if (!$dbhflag) { $dbhcommon->disconnect; }
            return undef;
        }
    }

    # close the database if it was not included in arguments
    if (!$dbhflag)
    {
        $dbhcommon->disconnect;
    }

    return $wo_id;
}

# standard 1 at end of perl package.
1;
