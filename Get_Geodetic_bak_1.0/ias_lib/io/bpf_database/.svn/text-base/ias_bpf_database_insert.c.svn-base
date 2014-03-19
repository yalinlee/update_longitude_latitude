/******************************************************************************
NAME: ias_bpf_database

PURPOSE:        
These functions provide the ability to insert BPF data into the database
and update the status of a BPF.  Note that no BPF query is provided since
that capability is handled by the BPF service.

NOTES:
none

ALGORITHM REFERENCES:
none

******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "ias_bpf.h"                    /* BPF File IO routines */
#include "ias_bpf_database.h"
#include "ias_bpf_database_local.h"
#include "ias_types.h"
#include "ias_const.h"
#include "ias_structures.h"
#include "ias_logging.h"
#include "ias_miscellaneous.h"
#include "ias_db_query.h"
#include "ias_db_insert.h"
#include "ias_db_stored_proc.h"
#include "ias_satellite_attributes.h"

/* Define some default values in case they aren't supplied by the caller */
#define SPACECRAFT "Landsat_8"
#define DESCRIPTION "Automatically Generated"

#define INVALID_BPF_ID -1
#define INVALID_GROUP_ID -1


/* Array size to account for each SCA's A0_Coefficient parameter along with
   the other detector-specific parameters */
#define MAX_ARRAY_SIZE     (IAS_MAX_NSCAS * (IAS_BPF_PAN_MAX_NDET + 1))

/* Local routines for managing a sorted list of parameter names & IDs */
static int parameter_name_compare
(
    const void *arg1, 
    const void *arg2
);

static int insert_new_parameter
(
    struct ias_db_connection *conn, 
    char *parameter_name
);


/****************************************************************************
* Name: ias_bpf_db_insert
*
* Description: Inserts the contents of a BPF structure into the database
* using a series of stored procedures and straight inserts.  The BPF in the
* database will be set to the status provided by the caller in the
* FILE_ATTRIBUTES.status field.
*
* Returns: SUCCESS if the BPF was inserted and activated successfully
*          ERROR if the BPF was not inserted successfully.  If an error occurs,
*            the BPF will be left in the database in an 'INACTIVE' status --
*            it cannot be rolled back because the BPF stored procedures
*            perform commits.
*****************************************************************************/
int ias_bpf_db_insert
(
    struct ias_db_connection *conn,     /* I: Open db connection */
    IAS_BPF *bpf                /* I/O: populated BPF structure, may be
                                   updated during validation */
)
{
    int status;

    /* First do some error checking on the BPF data */
    if (ias_bpf_db_validate_contents(bpf) != SUCCESS) 
    {
        IAS_LOG_ERROR("Invalid BPF data provided");
        return ERROR;
    }

    /* Start a transaction to insert the whole BPF together */
    status = ias_db_start_transaction(conn);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Creating transaction for BPF insert");
        return ERROR;
    }

    /* -------------------------------------------------------*/
    /* Step 1, start the BPF by inserting into the main table */
    int bpf_id = ias_bpf_db_start(conn, bpf);
    if (bpf_id == INVALID_BPF_ID)
    {
        IAS_LOG_ERROR("Writing BPF to the database: Cannot create main record");
        ias_db_rollback_transaction(conn); /* ignore errors */
        return ERROR;
    }

    /* -----------------------------------------------------------*/
    /* Step 2, create groups and insert parameters for the groups */
    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    /*         Group 1, File Attributes                           */

    status = ias_bpf_db_insert_file_attributes(conn, bpf_id, bpf);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Writing BPF to the database: Cannot insert file "
            "atttributes");
        ias_db_rollback_transaction(conn); /* ignore errors */
        return ERROR;
    }

    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    /*         Group 2, Orbit Parameters                          */
    status = ias_bpf_db_insert_orbit_parameters(conn, bpf_id, bpf);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Writing BPF to the database: Cannot insert "
            "orbit parameters");
        ias_db_rollback_transaction(conn); /* ignore errors */
        return ERROR;
    }

    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    /*         Group 3, Bias Model                                */
    status = ias_bpf_db_insert_bias_model(conn, bpf_id, bpf);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Writing BPF to the database: Cannot insert bias model");
        ias_db_rollback_transaction(conn); /* ignore errors */
        return ERROR;
    }

    /* -------------------------------------------------------*/
    /* All done, complete the transaction */
    status = ias_db_commit_transaction(conn);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Unable to commit all BPF data to the database");
        return ERROR;
    }

    return SUCCESS;

} /* end of ias_bpf_db_insert */

/****************************************************************************
* Name: ias_bpf_db_insert_file_attributes
*
* Description: Inserts the FILE_ATTRIBUTES group and the parameters that belong
* to it.
*
* Returns: SUCCESS or ERROR
*****************************************************************************/
int ias_bpf_db_insert_file_attributes
(
    struct ias_db_connection *conn,     /* I: Open db connection */
    int bpf_id,                         /* I: ID of BPF being written */
    IAS_BPF *bpf                        /* I: BPF data */
)
{
    const struct IAS_BPF_FILE_ATTRIBUTES *file_attributes = NULL;
                                        /* File attributes to write to
                                           DB */
    int record_index;                   /* looping variable */
    int status;                         /* database operation status */
    int group_id;                       /* database ID of the group that gets 
                                           created */
    int table_size;                     /* number of items in the stored proc 
                                           table */
    const int num_records = 9;          /* number of parameters in this group */
    int return_ids[num_records];        /* return ID for insert of each 
                                           parameter in the group */
    int parameter_order[num_records];   /* order of each parameter within the 
                                           group */
    char empty[2] = "";                 /* empty string placeholder */
    IAS_DB_NULL_TYPE null_flag;         /* flag indicating a null value */
    IAS_DB_NULL_TYPE nullarr[num_records]; /* null indicators for unused stored 
                                           proc parameters */
    /* names and values for each parameter within the group */
    char parameter_names[num_records][PARAMETER_NAME_LENGTH];
    char parameter_values[num_records][PARAMETER_VALUE_LENGTH];

    /* Get the FILE_ATTRIBUTES group data */
    file_attributes = ias_bpf_get_file_attributes(bpf);
    if (file_attributes == NULL)
    {
        IAS_LOG_ERROR("Cannot retrieve FILE_ATTRIBUTES group data");
        return ERROR;
    }

    /* Create FILE_ATTRIBUTES as the 1st Group */
    group_id = ias_bpf_db_create_group(conn, bpf_id,
        (char *)file_attributes->file_name, "FILE_ATTRIBUTES", 1);
    if (group_id == INVALID_GROUP_ID)
    {
        IAS_LOG_ERROR("Unable to create FILE_ATTRIBUTES group");
        return ERROR;
    }

    /* Set up variables to insert the parameter values */
    memset(parameter_names, 0, sizeof(parameter_names));
    memset(parameter_values, 0, sizeof(parameter_values));

    /* The stored procedure takes up to 7 values per parameter.
     * Metadata parameters in the FILE_ATTRIBUTES group only have 1 value 
     * per parameter.
     * Set up an array of null indicators for the others. */
    null_flag = ias_db_get_null_data_flag();
    for (record_index = 0; record_index < num_records; record_index++)
    {
        parameter_order[record_index] = record_index + 1;
        nullarr[record_index] = null_flag;
    }

   IAS_DB_STORED_PROC_TABLE metadata_parameter_table[] =
    {
        {NULL, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_INT, 
            &bpf_id, sizeof(bpf_id), NULL},
        {NULL, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_INT, 
            &group_id, sizeof(group_id), NULL},
        {NULL, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_STRING_ARRAY, 
            parameter_names, PARAMETER_NAME_LENGTH, NULL},
        {NULL, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_INT_ARRAY, 
            parameter_order, sizeof(parameter_order[0]), NULL},
        {NULL, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_STRING_ARRAY, 
            parameter_values, PARAMETER_VALUE_LENGTH, NULL},
        /* the following stored proc parameters are null */
        {NULL, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_STRING, 
            empty, sizeof(empty), nullarr},
        {NULL, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_STRING, 
            empty, sizeof(empty), nullarr},
        {NULL, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_STRING, 
            empty, sizeof(empty), nullarr},
        {NULL, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_STRING, 
            empty, sizeof(empty), nullarr},
        {NULL, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_STRING, 
            empty, sizeof(empty), nullarr},
        {NULL, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_STRING, 
            empty, sizeof(empty), nullarr},
        /* the final stored proc parameter is the returned ID */
        {NULL, IAS_DB_PARAMETER_MODE_OUTPUT, IAS_DB_FIELD_INT_ARRAY, 
            return_ids, sizeof(return_ids[0]), NULL}
    };
    table_size  = sizeof(metadata_parameter_table) /
        sizeof(IAS_DB_STORED_PROC_TABLE);

    /* Set up parameter names and values */
    strncpy(parameter_names[0], "Spacecraft_Name",      PARAMETER_NAME_LENGTH);
    strncpy(parameter_names[1], "Sensor_Name",          PARAMETER_NAME_LENGTH);
    strncpy(parameter_names[2], "Effective_Date_Begin", PARAMETER_NAME_LENGTH);
    strncpy(parameter_names[3], "Effective_Date_End",   PARAMETER_NAME_LENGTH);
    strncpy(parameter_names[4], "Baseline_Date",        PARAMETER_NAME_LENGTH);
    strncpy(parameter_names[5], "Description",          PARAMETER_NAME_LENGTH);
    strncpy(parameter_names[6], "File_Name",            PARAMETER_NAME_LENGTH);
    strncpy(parameter_names[7], "File_Source",          PARAMETER_NAME_LENGTH);
    strncpy(parameter_names[8], "Version",              PARAMETER_NAME_LENGTH);

    strncpy(parameter_values[0], file_attributes->spacecraft_name, 
        PARAMETER_VALUE_LENGTH);
    strncpy(parameter_values[1], file_attributes->sensor_name,
        PARAMETER_VALUE_LENGTH);
    strncpy(parameter_values[2], file_attributes->effective_date_begin, 
        PARAMETER_VALUE_LENGTH);
    strncpy(parameter_values[3], file_attributes->effective_date_end, 
        PARAMETER_VALUE_LENGTH);
    strncpy(parameter_values[4], file_attributes->baseline_date, 
        PARAMETER_VALUE_LENGTH);
    strncpy(parameter_values[5], file_attributes->description, 
        PARAMETER_VALUE_LENGTH);
    strncpy(parameter_values[6], file_attributes->file_name, 
        PARAMETER_VALUE_LENGTH);
    strncpy(parameter_values[7], file_attributes->file_source, 
        PARAMETER_VALUE_LENGTH);
    snprintf(parameter_values[8], PARAMETER_VALUE_LENGTH, "%d", 
        file_attributes->version);

    /* Execute to insert parameters for the group: FILE_ATTRIBUTES */
    status = ias_db_stored_proc_using_table(conn, 
        "process_bpf_pkg.proc_create_bpf_parameter",
        metadata_parameter_table, table_size, num_records);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Unable to create parameters in FILE_ATTRIBUTES group");
        return ERROR;
    }

    /* Each stored proc return ID should be 1 (success) */
    for (record_index = 0; record_index < num_records; record_index++)
    {
        if (return_ids[record_index] != 1)
        {
            IAS_LOG_ERROR("Unable to create parameters in FILE_ATTRIBUTES "
                "group record %d", record_index + 1);
            return ERROR;
        }
    }

    return SUCCESS;

} /* end of ias_bpf_db_insert_file_attributes */

/****************************************************************************
* Name: ias_bpf_db_insert_orbit_parameters
*
* Description: Inserts the ORBIT_PARAMETERS group and the parameters that go 
* with it
*
* Returns: SUCCESS or ERROR
*****************************************************************************/
int ias_bpf_db_insert_orbit_parameters
(
    struct ias_db_connection *conn,     /* I: Open db connection */
    int bpf_id,                         /* I: BPF being written */
    IAS_BPF *bpf                        /* I: BPF data structure */
)
{
    const struct IAS_BPF_FILE_ATTRIBUTES *file_attributes = NULL;
                                        /* File attributes data */
    const struct IAS_BPF_ORBIT_PARAMETERS *orbit_parameters = NULL;
                                        /* Orbit parameters data */

    int record_index;                   /* looping variable */
    int status;                         /* database operation status */
    int table_size;                     /* number of items in the stored proc 
                                           table */
    int group_id;                       /* database ID of the group that gets 
                                           created */
    const int num_records = 1;          /* number of parameters in this group */
    int return_ids[num_records];        /* return ID for insert of each 
                                           parameter in the group */
    int parameter_order[num_records];   /* order of each parameter within the 
                                           group */
    char empty[2] = "";                 /* empty string placeholder */
    IAS_DB_NULL_TYPE null_flag;         /* flag indicating a null value */
    IAS_DB_NULL_TYPE nullarr[num_records];/* null indicators for unused stored 
                                           proc parameters */
    /* names and values for each parameter within the group */
    char parameter_names[num_records][PARAMETER_NAME_LENGTH];
    char parameter_values[num_records][PARAMETER_VALUE_LENGTH];


    /* Get the FILE_ATTRIBUTES group data */
    file_attributes = ias_bpf_get_file_attributes(bpf);
    if (file_attributes == NULL)
    {
        IAS_LOG_ERROR("Cannot retrieve FILE_ATTRIBUTES group data");
        return ERROR;
    }

    /* Get the ORBIT_PARAMETERS group data */
    orbit_parameters = ias_bpf_get_orbit_parameters(bpf);
    if (orbit_parameters == NULL)
    {
        IAS_LOG_ERROR("Cannot retrieve ORBIT_PARAMETERS group data");
        return ERROR;
    }

    /* Create ORBIT_PARAMETERS as the 2nd Group */
    group_id = ias_bpf_db_create_group(conn, bpf_id,
        (char *)file_attributes->file_name, "ORBIT_PARAMETERS", 2);
    if (group_id == INVALID_GROUP_ID)
    {
        IAS_LOG_ERROR("Unable to create ORBIT_PARAMETERS group");
        return ERROR;
    }

    /* Set up variables to insert the parameter values */
    memset(parameter_names, 0, sizeof(parameter_names));
    memset(parameter_values, 0, sizeof(parameter_values));

    /* The stored procedure takes up to 7 values per parameter.
     * Metadata parameters in the ORBIT_PARAMETERS group only have 1 value 
     * per parameter.
     * Set up an array of null indicators for the others. */
    null_flag = ias_db_get_null_data_flag();
    for (record_index = 0; record_index < num_records; record_index++)
    {
        parameter_order[record_index] = record_index + 1;
        nullarr[record_index] = null_flag;
    }

    IAS_DB_STORED_PROC_TABLE metadata_parameter_table[] =
    {
        {NULL, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_INT, 
            &bpf_id, sizeof(bpf_id), NULL},
        {NULL, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_INT, 
            &group_id, sizeof(group_id), NULL},
        {NULL, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_STRING_ARRAY, 
            parameter_names, PARAMETER_NAME_LENGTH, NULL},
        {NULL, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_INT_ARRAY, 
            parameter_order, sizeof(parameter_order[0]), NULL},
        {NULL, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_STRING_ARRAY, 
            parameter_values, PARAMETER_VALUE_LENGTH, NULL},
        /* the following stored proc parameters are null */
        {NULL, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_STRING, 
            empty, sizeof(empty), nullarr},
        {NULL, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_STRING, 
            empty, sizeof(empty), nullarr},
        {NULL, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_STRING, 
            empty, sizeof(empty), nullarr},
        {NULL, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_STRING, 
            empty, sizeof(empty), nullarr},
        {NULL, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_STRING, 
            empty, sizeof(empty), nullarr},
        {NULL, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_STRING, 
            empty, sizeof(empty), nullarr},
        /* the final stored proc parameter is the returned ID */
        {NULL, IAS_DB_PARAMETER_MODE_OUTPUT, IAS_DB_FIELD_INT_ARRAY, 
            return_ids, sizeof(return_ids[0]), NULL}
    };
    table_size  = sizeof(metadata_parameter_table) /
        sizeof(IAS_DB_STORED_PROC_TABLE);

    /* Set up parameter names and values */
    strncpy(parameter_names[0], "Orbit_Number", PARAMETER_NAME_LENGTH);

    snprintf(parameter_values[0], PARAMETER_VALUE_LENGTH, "%d", 
        orbit_parameters->begin_orbit_number);

    /* Execute to insert parameters for the group: ORBIT_PARAMETERS */
    status = ias_db_stored_proc_using_table(conn, 
        "process_bpf_pkg.proc_create_bpf_parameter",
        metadata_parameter_table, table_size, num_records);
    if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Unable to create parameters in ORBIT_PARAMETERS "
                "group");
            return ERROR;
        }

    /* Each stored proc return ID should be 1 (success) */
    for (record_index = 0; record_index < num_records; record_index++)
    {
        if (return_ids[record_index] != 1)
        {
            IAS_LOG_ERROR("Unable to create parameters in ORBIT_PARAMETERS "
                "group record %d", record_index + 1);
            return ERROR;
        }
    }

    return SUCCESS;

} /* end of ias_bpf_db_insert_orbit_parameters */

/****************************************************************************
* Name: ias_bpf_db_insert_bias_model
*
* Description: Inserts the BIAS_MODEL group and the parameters that go 
* with it.  
*
* NOTES: The bias model group contains over 80,000 parameters.
* For speed, this routine uses insert statements rather than
* stored procedures.  When doing it this way, the row inserted must have the ID
* of the parameter (when using the stored procedure, it looks up the ID for 
* you).  So first we get the IDs for all the parameter names in the 
* BPF_PARAMETER table.   These are stored in a sorted array, so the ID
* can quickly be looked up when inserting into the parameter value table.
* If a parameter name doesn't exist in the BPF_PARAMETER table, a new ID is 
* generated and it is added to the table.
*
* Returns: SUCCESS or ERROR
*****************************************************************************/
int ias_bpf_db_insert_bias_model
(
    struct ias_db_connection *conn,     /* I: Open db connection */
    const int bpf_id,                   /* I: ID of BPF being written */
    IAS_BPF *bpf                        /* I: BPF data structure */
)
{
    const struct IAS_BPF_FILE_ATTRIBUTES *file_attributes = NULL;
                                        /* FILE_ATTRIBUTES group data */
    const struct IAS_BPF_BIAS_MODEL      *bias_model = NULL;
                                        /* BAND_BIAS_MODEL group data */

    IAS_SENSOR_ID sensor_id;            /* ID number of the sensor sourcing
                                           the BPF parameters */

    int band_list[IAS_MAX_NBANDS];      /* OLI / TIRS normal band list */
    int index;                          /* Band loop counter */
    int number_of_bands;                /* Total number of bands for the
                                           given sensor */
    int p;                              /* initialization looping variable,
                                           for each parameter being inserted */
    int r;                              /* initialization looping variable,
                                           for each row being inserted */
    int row_index;                      /* looping variable for rows inserted 
                                           together */
    int coeff_set_index;                /* index for set of coefficients */
    int band_index;                     /* 0-based index corresponding to
                                           current band number */
    int sca_index;                      /* sca, detector loopers */
    int det_index;
    int max_coeff_set;                  /* PAN band has 2 sets of
                                           coefficients, all others have
                                           1 */
    int max_sca;                        /* sca, detector loop limits */
    int max_det; 
    int parms_per_sca;                  /* number of parameters per SCA group */
    int sca_start_index;                /* the starting row index for a SCA */
    int bandno;                         /* actual band number */
    int status;                         /* database operation status */
    int null_flag;                      /* flag indicating a null value */
    int non_null_flag;                  /* flag indicating a non-null value */
    int group_index = 3;                /* order of this group in the BPF, 
                                           always starts as the 3rd group */
    int group_id;                       /* database ID of the group that gets 
                                           created */
    int num_parameters;                 /* number of parameter names/IDs in the
                                           database */
    int table_size;                     /* size of database insert table */
    char parameter_name[PARAMETER_NAME_LENGTH]; /* single parameter name */
    char group_name_prefix[2][40];      /* group names have a band-specific 
                                           prefix */
    char group_name[60];                /* name of the group for a band/SCA */
    int place_in_group = 0;             /* order of a parameter within the 
                                           group */
    struct PARAMETER_NAME_ID *parameter_found;  /* indicates the parameter 
                                                   exists in the db already */
    int parameter_id_struct_size;

    int group_ids[MAX_ARRAY_SIZE];               /* group ID */
    int parameter_ids[MAX_ARRAY_SIZE];           /* parameter ID */
    int parameter_order[MAX_ARRAY_SIZE];         /* place of this
                                                    parameter in the
                                                    group */

    /* arrays of individual parameter values */
    double valarr[IAS_BPF_MAX_MODEL_PARAM_VALUES][MAX_ARRAY_SIZE];

    /* arrays of null indicators  */
    IAS_DB_NULL_TYPE nullarr[IAS_BPF_MAX_MODEL_PARAM_VALUES][MAX_ARRAY_SIZE];

    /* Determine whether this is an OLI or TIRS BPF we're dealing with. */
    sensor_id = ias_bpf_get_sensor_id(bpf);
    if (sensor_id == IAS_INVALID_SENSOR_ID)
    {
        IAS_LOG_ERROR("Retrieving sensor ID from BPF sensor name");
        return ERROR;
    }

    /* Get the sensor-specific band list. */
    status = ias_sat_attr_get_sensor_band_numbers(sensor_id, IAS_NORMAL_BAND,
        0, band_list, IAS_MAX_NBANDS, &number_of_bands);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Retrieving sensor-specific band list");
        return ERROR;
    }

    /* Get the maximum number of SCAs per band. */
    max_sca = ias_sat_attr_get_sensor_sca_count(sensor_id);
    if (max_sca == ERROR)
    {
        IAS_LOG_ERROR("Retrieving number of available SCAs for the "
            "specified sensor");
        return ERROR;
    }

    /* Get the BIAS_MODEL group data */
    bias_model = ias_bpf_get_bias_model(bpf);
    if (bias_model == NULL)
    {
        IAS_LOG_ERROR("Cannot retrieve BIAS_MODEL group data");
        return ERROR;
    }

    /* Get the file attributes. */
    file_attributes = ias_bpf_get_file_attributes(bpf);
    if (file_attributes == NULL)
    {
        IAS_LOG_ERROR("Retrieving FILE_ATTRIBUTES information");
        return ERROR;
    }

    /* Preload the BPF Parameter IDs to use when inserting parameter values */
    struct PARAMETER_NAME_ID *parameter_name_id_list = 
        ias_bpf_db_get_parameter_ids(conn, &num_parameters);
    if (parameter_name_id_list == NULL)
    {
        IAS_LOG_ERROR("Unable to retrieve BIAS_MODEL parameter IDs");
        return ERROR;
    }
    parameter_id_struct_size = sizeof(struct PARAMETER_NAME_ID);

    IAS_LOG_DEBUG("Retrieved %d parameter IDs", num_parameters);

    /* Set up and bind variables for inserting the bias model parameters */
    /* The table allows up to 4 values for each parameter (row) inserted */
    IAS_DB_INSERT_TABLE parameter_table[] =
    {
        IAS_DB_TABLE_INT_ARRAY("BPF_GROUP_ID", group_ids),
        IAS_DB_TABLE_INT_ARRAY("BPF_PARAMETER_ID", parameter_ids),
        IAS_DB_TABLE_INT_ARRAY("PARAMETER_ORDER", parameter_order),
        IAS_DB_TABLE_DOUBLE_ARRAY_WITH_NULLS("VALUE1", valarr[0], nullarr[0]),
        IAS_DB_TABLE_DOUBLE_ARRAY_WITH_NULLS("VALUE2", valarr[1], nullarr[1]),
        IAS_DB_TABLE_DOUBLE_ARRAY_WITH_NULLS("VALUE3", valarr[2], nullarr[2]),
        IAS_DB_TABLE_DOUBLE_ARRAY_WITH_NULLS("VALUE4", valarr[3], nullarr[3])
    };
    table_size  = sizeof(parameter_table) / sizeof(IAS_DB_INSERT_TABLE);

    /* Add sanity check on the table, to make sure we are writing all the
     * model parameters */
    if ((table_size - 3) < IAS_BPF_MAX_MODEL_PARAM_VALUES)
    {
        IAS_LOG_ERROR("BPF database insert table does not have the expected "
            "number of values (%d)", IAS_BPF_MAX_MODEL_PARAM_VALUES);
        return ERROR;
    }

    /* set up null & non-null flags */
    /* non-null is indicated by parameter length, in this case a double */
    non_null_flag = sizeof(valarr[0][0]); 
    null_flag = ias_db_get_null_data_flag();

    /* For each band, arrange the data into the proper bound variables      */
    /*   and then insert all at once                                        */
    /* Parameters are grouped by SCA:                                       */
    /* VNIR bias model - 4 values/detector and one A0 coeff per SCA;        */
    /*                   bands 1-5 scas 1-14 det 1-494                      */
    /* SWIR bias model - 4 values/detector and one A0 coeff per SCA;        */
    /*                   bands 6,7,9 scas 1-14 det 1-494                    */
    /* PAN  bias model - 4 values/detector and one A0 coeff per SCA, grouped*/
    /*                   separately for odd & even frames;                  */
    /*                   band 8 scas 1-14 det 1-988                         */
    /* THERMAL bias model - 2 values/detector, bands 10, 11 scas 1-3        */
    /*                      det 1-640                                       */
    for (index = 0; index < number_of_bands; index++)
    {
        /* Get the band number and corresponding band index */
        bandno = band_list[index];
        band_index = ias_sat_attr_convert_band_number_to_index(bandno);
        if (band_index == ERROR)
        {
            IAS_LOG_ERROR("Invalid band number %d", bandno);
            return ERROR;
        }

        /* Set up band-specific things */
        if (bias_model[band_index].spectral_type == IAS_SPECTRAL_THERMAL)
        {
            max_det = IAS_BPF_THERMAL_MAX_NDET;
            parms_per_sca = max_det;
            max_coeff_set = 1;

            sprintf(group_name_prefix[0], "BIAS_MODEL_B%02d", bandno);

            /* Fill in the NULL indicators for all possible parameter
               values.  Each SCA has max_det parameters; there is
               (currently) no A0_coefficient parameter.  */
            for (p = 0; p < IAS_BPF_MAX_MODEL_PARAM_VALUES; p++)
            {
                for (sca_index = 0; sca_index < max_sca; sca_index++)
                {
                    sca_start_index = sca_index * parms_per_sca;

                    if (p < IAS_BPF_THERMAL_VALUES)
                    {
                        for (r = 0; r < max_det; r++)
                            nullarr[p][sca_start_index + r] = non_null_flag;
                    }
                    else
                    {
                        for (r = 0; r < max_det; r++)
                        {
                            nullarr[p][sca_start_index + r] = null_flag;
                        }
                    }
                }
            }

            /* NULL out all other spaces */
            for (p = 0; p < IAS_BPF_MAX_MODEL_PARAM_VALUES; p++)
            {
                for (r = max_sca * parms_per_sca; r < MAX_ARRAY_SIZE; r++)
                {
                    nullarr[p][r] = null_flag;
                }
            }
        }
        else if (bias_model[band_index].spectral_type == IAS_SPECTRAL_PAN)
        {
            max_det = IAS_BPF_PAN_MAX_NDET;
            parms_per_sca = max_det + 1;
            max_coeff_set = 2;
            sprintf(group_name_prefix[0], "BIAS_MODEL_EVEN_B%02d",
                bandno);
            sprintf(group_name_prefix[1], "BIAS_MODEL_ODD_B%02d",
                bandno);

            /* Fill in the null indicators for all possible parameter values
             *   Each SCA has max_det + 1 parameters
             *   And each parameter will have either 
             *     IAS_BPF_PAN_VALUES (for the DXXX parameters) or 
             *     IAS_BPF_A0_values (for the A0_Coefficient parameter) values 
             */
            for (p = 0; p < IAS_BPF_MAX_MODEL_PARAM_VALUES; p++)
            {
                for (sca_index = 0; sca_index < max_sca; sca_index++)
                {
                    sca_start_index = sca_index * parms_per_sca;

                    /* How many values in each DXXX parameter in the PAN band */
                    if (p < IAS_BPF_PAN_VALUES) 
                        /* non-null for the values in the DXXX parameters */
                        for (r = 0; r < max_det; r++)
                            nullarr[p][sca_start_index + r] = non_null_flag;
                    else
                        /* set null for parameters not populated */
                        for (r = 0; r < max_det; r++)
                            nullarr[p][sca_start_index + r] = null_flag;

                    /* How many values in the single A0_Coefficient parameter */
                    if (p < IAS_BPF_A0_VALUES) 
                        /* non-null populated values in the A0 parameter */
                        nullarr[p][sca_start_index + max_det] = non_null_flag;
                    else
                        /* null for non-populated values */
                        nullarr[p][sca_start_index + max_det] = null_flag;
                }
            }
        }
        else if (bias_model[band_index].spectral_type == IAS_SPECTRAL_SWIR)
        {
            max_det = IAS_BPF_MS_MAX_NDET;
            parms_per_sca = max_det + 1;
            max_coeff_set = 1;

            sprintf(group_name_prefix[0], "BIAS_MODEL_B%02d", bandno);

            /* Fill in the null indicators for all possible parameter values
             *   Each SCA has max_det + 1 parameters
             *   And each parameter will have either 
             *     IAS_BPF_SWIR_VALUES (for the DXXX parameters) or 
             *     IAS_BPF_A0_values (for the A0_Coefficient parameter) values 
             */
            for (p = 0; p < IAS_BPF_MAX_MODEL_PARAM_VALUES; p++)
            {
                for (sca_index = 0; sca_index < max_sca; sca_index++)
                {
                    sca_start_index = sca_index * parms_per_sca;

                    if (p < IAS_BPF_SWIR_VALUES)
                        for (r = 0; r < max_det; r++)
                            nullarr[p][sca_start_index + r] = non_null_flag;
                    else
                        for (r = 0; r < max_det; r++)
                            nullarr[p][sca_start_index + r] = null_flag;

                    if (p < IAS_BPF_A0_VALUES) 
                        nullarr[p][sca_start_index + max_det] = non_null_flag;
                    else
                        nullarr[p][sca_start_index + max_det] = null_flag;
                }
            }

            /* NULL out all other spaces */
            for (p = 0; p < IAS_BPF_MAX_MODEL_PARAM_VALUES; p++)
            {
                for (r = max_sca * parms_per_sca; r < MAX_ARRAY_SIZE; r++)
                {
                    nullarr[p][r] = null_flag;
                }
            }
        }
        else /* VNIR bands */
        {
            max_det = IAS_BPF_MS_MAX_NDET;
            parms_per_sca = max_det + 1;
            max_coeff_set = 1;

            sprintf(group_name_prefix[0], "BIAS_MODEL_B%02d", bandno);

            /* Fill in the null indicators for all possible parameter values
             *   Each SCA has max_det + 1 parameters
             *   And each parameter will have either 
             *     IAS_BPF_VNIR_VALUES (for the DXXX parameters) or 
             *     IAS_BPF_A0_values (for the A0_Coefficient parameter) values 
             */

            for (p = 0; p < IAS_BPF_MAX_MODEL_PARAM_VALUES; p++)
            {
                for (sca_index = 0; sca_index < max_sca; sca_index++)
                {
                    sca_start_index = sca_index * parms_per_sca;

                    if (p < IAS_BPF_VNIR_VALUES)
                        for (r = 0; r < max_det; r++)
                            nullarr[p][sca_start_index + r] = non_null_flag;
                    else
                        for (r = 0; r < max_det; r++)
                            nullarr[p][sca_start_index + r] = null_flag;

                    if (p < IAS_BPF_A0_VALUES) 
                        nullarr[p][sca_start_index + max_det] = non_null_flag;
                    else
                        nullarr[p][sca_start_index + max_det] = null_flag;
                }
            }

            /* NULL out all other spaces */
            for (p = 0; p < IAS_BPF_MAX_MODEL_PARAM_VALUES; p++)
            {
                for (r = max_sca * parms_per_sca; r < MAX_ARRAY_SIZE; r++)
                {
                    nullarr[p][r] = null_flag;
                }
            }
        }

        for (coeff_set_index = 0; coeff_set_index < max_coeff_set; 
             coeff_set_index++) 
        {
            row_index = 0;
            for (sca_index = 0; sca_index < max_sca; sca_index++)
            {
                /* Create per-Band/SCA BIAS_MODEL Groups 3 - 142 */
                sprintf(group_name, "%s_SCA%02d",
                    group_name_prefix[coeff_set_index], (sca_index + 1));

                group_id = ias_bpf_db_create_group(conn, bpf_id, 
                   (char *)file_attributes->file_name, group_name,
                   group_index++);
                if (group_id == INVALID_GROUP_ID)
                {
                    free(parameter_name_id_list);
                    IAS_LOG_ERROR("Unable to create %s group", group_name);
                    return ERROR;
                }

                /* Do all the DXXX parameters in this SCA */
                for (det_index = 0; det_index < max_det; det_index++)
                {
                    /* Set up the Group and Parameter IDs for this entry */
                    group_ids[row_index]  = group_id;

                    sprintf(parameter_name, "D%03d", (det_index + 1));

                    /* Locate the ID for this parameter, or 
                     * create it if not found */
                    parameter_found = bsearch(parameter_name, 
                        parameter_name_id_list, num_parameters, 
                        parameter_id_struct_size,
                        &parameter_name_compare);
                    if (parameter_found)
                    {
                        parameter_ids[row_index] 
                            = parameter_found->parameter_id;
                    }
                    else 
                    {
                        parameter_ids[row_index] = insert_new_parameter(conn,
                            parameter_name);
                        if (parameter_ids[row_index] == ERROR)
                        {
                            IAS_LOG_ERROR("Could not find parameter ID for "
                                "%s", parameter_name);
                            free(parameter_name_id_list);
                            return ERROR;
                        }
                    }

                    /* Set in the proper detector-specific coefficients */
                    if (bias_model[band_index].spectral_type
                        == IAS_SPECTRAL_THERMAL)
                    {
                        for (p = 0; p < IAS_BPF_THERMAL_VALUES; p++)
                        {
                            valarr[p][row_index] = bias_model[band_index]
                                .thermal->det_params[sca_index][det_index][p];
                        }
                    }
                    else if (bias_model[band_index].spectral_type
                            == IAS_SPECTRAL_PAN)
                    {
                        if (coeff_set_index == 0)
                        {
                            for (p = 0; p < IAS_BPF_PAN_VALUES; p++)
                                valarr[p][row_index] = bias_model[band_index]
                                .pan->det_params_even[sca_index][det_index][p];
                        }
                        else
                        {
                            for (p = 0; p < IAS_BPF_PAN_VALUES; p++)
                                valarr[p][row_index] = bias_model[band_index]
                                .pan->det_params_odd[sca_index][det_index][p];
                        }
                    }
                    else if (bias_model[band_index].spectral_type
                                 == IAS_SPECTRAL_SWIR)
                    {
                        for (p = 0; p < IAS_BPF_SWIR_VALUES; p++)
                            valarr[p][row_index] = bias_model[band_index]
                                .swir->det_params[sca_index][det_index][p];
                    }
                    else /* VNIR bands */
                    {
                        for (p = 0; p < IAS_BPF_VNIR_VALUES; p++)
                            valarr[p][row_index] = bias_model[band_index]
                                .vnir->det_params[sca_index][det_index][p];
                    }
                    parameter_order[row_index] = place_in_group++;

                    /* Increment the row counter */
                    row_index++;
                }

                /* Now do the a0_coefficient parameter for this SCA if this
                   is an OLI BPF.  */
                if (sensor_id == IAS_OLI)
                {
                    group_ids[row_index] = group_id;
                    sprintf(parameter_name, "A0_Coefficient");

                    parameter_found = bsearch(parameter_name, 
                        parameter_name_id_list, num_parameters, 
                        parameter_id_struct_size,
                        &parameter_name_compare);
                    if (parameter_found)
                    {
                        parameter_ids[row_index] =
                            parameter_found->parameter_id;
                    }
                    else 
                    {
                        parameter_ids[row_index] = insert_new_parameter(conn, 
                            parameter_name);
                        if (parameter_ids[row_index] == ERROR)
                        {
                            IAS_LOG_ERROR("Could not find parameter ID for "
                                "%s", parameter_name);
                            free(parameter_name_id_list);
                            return ERROR;
                        }
                    }

                    /* Set in the proper coefficients */
                    if (bias_model[band_index].spectral_type
                        == IAS_SPECTRAL_PAN)
                    {
                        if (coeff_set_index == 0)
                        {
                            valarr[0][row_index] = bias_model[band_index]
                                .pan->a0_coefficient_even[sca_index];
                        }
                        else
                        {
                            valarr[0][row_index] = bias_model[band_index]
                                .pan->a0_coefficient_odd[sca_index];
                        }
                    }
                    else if (bias_model[band_index].spectral_type
                                 == IAS_SPECTRAL_SWIR)
                    {
                        valarr[0][row_index] = bias_model[band_index]
                            .swir->a0_coefficient[sca_index];
                    }
                    else /* VNIR bands */
                    {
                        valarr[0][row_index] = bias_model[band_index]
                            .vnir->a0_coefficient[sca_index];
                    }

                    parameter_order[row_index] = place_in_group++;

                    /* Increment the row counter */
                    row_index++;
                }
            }

            /* Do the actual insert */
            if (sensor_id == IAS_TIRS)
            {
                status = ias_db_insert_using_table(conn,
                    "BPF_PARAMETER_VALUES_DATA", parameter_table,
                    table_size, (max_sca * max_det));
            }
            else
            {
                status = ias_db_insert_using_table(conn, 
                    "BPF_PARAMETER_VALUES_DATA", parameter_table, table_size,
                    (max_sca * (max_det + 1)));
            }
            if (status != SUCCESS)
            {
                IAS_LOG_ERROR("Inserting bias model parameters for band %d",
                    bandno);
                free(parameter_name_id_list);
                return ERROR;
            }

        } /* coefficient set loop */

    } /* band loop */

    free(parameter_name_id_list);
    return SUCCESS;

} /* end of ias_bpf_db_insert_bias_model */

/****************************************************************************
* Name: ias_bpf_db_start
*
* Description: Begins the process of writing BPF data to the database.
*   The record in the main table is written with BPF metadata.
*
* Returns: the record ID of the new BPF created
*          INVALID_BPF_ID if an error occurred

* Notes:   In this routine, the shorter sensor name must be written
#          to the BPF_MAIN table.
*****************************************************************************/
int ias_bpf_db_start
(
    struct ias_db_connection *conn,       /* I: database connection */
    IAS_BPF *bpf                          /* I: BPF information */
)
{
    IAS_SENSOR_ID sensor_id;              /* Sensor ID number */
    char db_sensor_id_string[IAS_BPF_SENSOR_NAME_LENGTH];
    int bpf_id = INVALID_BPF_ID;          /* ID returned by the initiation 
                                             procedure */
    int table_size;                       /* # of entries in the stored proc 
                                             parameter table */
    int status;                           /* db library return status */
    char default_bpf_status[] = "ACTIVE"; /* status defaults to ACTIVE */

    const struct IAS_BPF_FILE_ATTRIBUTES *file_attributes = NULL;
    const struct IAS_BPF_ORBIT_PARAMETERS *orbit_parameters = NULL;

    /* Set the correct database sensor name. */
    sensor_id = ias_bpf_get_sensor_id(bpf);
    if (sensor_id == IAS_INVALID_SENSOR_ID)
    {
        IAS_LOG_ERROR("Determining sensor ID number from sensor name "
            "information");
        return ERROR;
    }
    if (sensor_id == IAS_OLI)
    {
        sprintf(db_sensor_id_string, "OLI");
    }
    else
    {
        sprintf(db_sensor_id_string, "TIRS");
    }

    /* Get the file attributes and orbit parameters information */
    file_attributes = ias_bpf_get_file_attributes(bpf);
    orbit_parameters = ias_bpf_get_orbit_parameters(bpf);

    if ((file_attributes == NULL) || (orbit_parameters == NULL))
    {
        IAS_LOG_ERROR("Cannot retrieve BPF file attributes and/or "
                      "orbit parameters groups");
        return ERROR;
    }

    /* Order is important!  Follow the parameter order defined in the
       'proc_insert_bpf_main' procedure.  */
    IAS_DB_STORED_PROC_TABLE main_table[] =
    {
        {NULL, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_STRING, 
            (char *)file_attributes->file_name, 
            sizeof(file_attributes->file_name), NULL},
        {NULL, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_STRING, 
            (char *)file_attributes->file_source, 
            sizeof(file_attributes->file_source), NULL},
        {NULL, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_STRING, 
            (char *)file_attributes->description, 
            sizeof(file_attributes->description), NULL},
        {NULL, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_INT, 
            (int *)&file_attributes->version, 
            sizeof(file_attributes->version), NULL},
        {NULL, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_INT, 
            (int *)&orbit_parameters->begin_orbit_number,
            sizeof(orbit_parameters->begin_orbit_number), NULL},
        {NULL, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_STRING,
            db_sensor_id_string, sizeof(db_sensor_id_string), NULL},
        {NULL, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_DATETIME, 
            &file_attributes->effective_date_begin,
            sizeof(file_attributes->effective_date_begin), NULL},
        {NULL, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_DATETIME, 
            &file_attributes->effective_date_end,
            sizeof(file_attributes->effective_date_end), NULL},
        {NULL, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_STRING, 
            default_bpf_status,
            sizeof(default_bpf_status), NULL},
        {NULL, IAS_DB_PARAMETER_MODE_OUTPUT, IAS_DB_FIELD_INT, 
            &bpf_id, sizeof(bpf_id), NULL}
    };

    table_size  = sizeof(main_table) / sizeof(IAS_DB_STORED_PROC_TABLE);
    status = ias_db_stored_proc_using_table(conn, 
        "process_bpf_pkg.proc_insert_bpf_main",
        main_table, table_size, 1);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Unable to create BPF %s in the database",
            file_attributes->file_name);
        return INVALID_BPF_ID;
    }

    IAS_LOG_INFO("Creating BPF %s as ID %d", 
        file_attributes->file_name, bpf_id);
    return bpf_id;
}

/****************************************************************************
* Name: ias_bpf_db_create_group
*
* Description: Uses a stored procedure to create a group attached to the given 
*   BPF to be used to insert parameters into.  
*
* Returns: ID of the group created
*          INVALID_GROUP_ID on error
*****************************************************************************/
int ias_bpf_db_create_group
(
    struct ias_db_connection *conn,     /* I: database connection */
    int bpf_id,                   /* I: ID of BPF being written */
    char *bpf_file_name,          /* I: Name of BPF being written */
    char *group_name,             /* I: Name of group to create */
    int group_order               /* I: Order of this group in the BPF */
)
{
    int group_id = INVALID_GROUP_ID;    /* ID of the group that is created */
    const int num_records = 1;          /* # of records inserted at a time*/
    int table_size;                     /* # of items in the insert table */
    int status;                         /* database operation status */

    IAS_DB_STORED_PROC_TABLE group_table[] =
    {
        {NULL, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_INT, 
            &bpf_id, sizeof(bpf_id), NULL},
        {NULL, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_STRING, 
            bpf_file_name, strlen(bpf_file_name), NULL},
        {NULL, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_STRING_ARRAY, 
            group_name, strlen(group_name), NULL},
        {NULL, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_INT_ARRAY, 
            &group_order, sizeof(group_order), NULL},
        {NULL, IAS_DB_PARAMETER_MODE_OUTPUT, IAS_DB_FIELD_INT_ARRAY, 
            &group_id, sizeof(group_id), NULL}
    };

    table_size  = sizeof(group_table) / sizeof(IAS_DB_STORED_PROC_TABLE);
    status = ias_db_stored_proc_using_table(conn, 
        "process_bpf_pkg.proc_create_bpf_group",
        group_table, table_size, num_records);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Unable to create group %s for BPF %d in the database",
            group_name, bpf_id);
        return INVALID_GROUP_ID;
    }

    return group_id;
}

/****************************************************************************
* Name: ias_bpf_db_get_parameter_ids
*
* Description: Gets a sorted array of parameter names and IDs, to have them
*   available to look up IDs (using bsearch) when inserting the bias model 
*   parameter values
*
* Returns: array of parameter_name_id structures
*          NULL if error
*****************************************************************************/
struct PARAMETER_NAME_ID *ias_bpf_db_get_parameter_ids
(
    struct ias_db_connection *conn,     /* I: database connection */
    int *num_parameters                 /* O: number of parameters found */
)
{
    int i;                              /* looping variable */
    int status;                         /* database operation status */
    int table_size;                     /* number of entries in table */
    int *bias_parameter_ids;            /* parameter IDs read from the db */
    char *bias_parameter_names;         /* parameter names read from the db */

    /* Get the number of rows in BPF_PARAMETER so we know how many items
     * to allocate space for */
    *num_parameters = ias_db_get_count(conn, "BPF_PARAMETER", "1", "1");
    if (*num_parameters < 1)
        return NULL;

    /* Allocate temporary space to keep all the parameter names & IDs 
     * that come from the database */
    bias_parameter_ids = malloc(*num_parameters * sizeof(*bias_parameter_ids));
    if (bias_parameter_ids == NULL)
    {
        IAS_LOG_ERROR("Allocating memory to retrieve BPF parameter IDs");
        return NULL;
    }
    bias_parameter_names = malloc(*num_parameters * PARAMETER_NAME_LENGTH *
        sizeof(*bias_parameter_names));
    if (bias_parameter_names == NULL)
    {
        free(bias_parameter_ids);
        IAS_LOG_ERROR("Allocating memory to retrieve BPF parameter IDs");
        return NULL;
    }
    memset(bias_parameter_names, 0, *num_parameters * PARAMETER_NAME_LENGTH 
        * sizeof(*bias_parameter_names));

    /* Query everything out of BPF_PARAMETER table, sorted by parameter name */
    IAS_DB_QUERY_TABLE parm_query_table[] =
    {
        {"bpf_parameter_id", bias_parameter_ids, sizeof(bias_parameter_ids[0]), 
            IAS_QUERY_INT_ARRAY, *num_parameters },
        {"name", bias_parameter_names, PARAMETER_NAME_LENGTH, 
            IAS_QUERY_STRING_ARRAY,  *num_parameters}
    };
    table_size  = sizeof(parm_query_table) / sizeof(IAS_DB_QUERY_TABLE);
    status = ias_db_query_using_table(conn, "BPF_PARAMETER", parm_query_table,
        table_size, "1=1 order by name", *num_parameters);
    if (status != SUCCESS)
    {
        free(bias_parameter_ids);
        free(bias_parameter_names);
        IAS_LOG_ERROR("Retrieving BPF parameter IDs from the database");
        return NULL;
    }

    /* Build a sorted array of parameter names & id structures */
    struct PARAMETER_NAME_ID *parameter_name_id_list 
        = malloc(*num_parameters * sizeof(*parameter_name_id_list));
    if (parameter_name_id_list == NULL)
    {
        free(bias_parameter_ids);
        free(bias_parameter_names);
        IAS_LOG_ERROR("Allocating memory to retrieve BPF parameter IDs");
        return NULL;
    }
    for (i = 0; i < *num_parameters; i++)
    {
        strncpy(parameter_name_id_list[i].parameter_name,
            &bias_parameter_names[i*PARAMETER_NAME_LENGTH],
            PARAMETER_NAME_LENGTH);
        parameter_name_id_list[i].parameter_id = bias_parameter_ids[i];
    }
    free(bias_parameter_ids);
    free(bias_parameter_names);

    return parameter_name_id_list;
}


/***************************************************************************
* Name: parameter_name_compare
*
* Description:  Comparator function used to search the parameter name - ID array
*    This function is passed to the bsearch algorithm to use in lookup of
*    a parameter ID for a given parameter name
*
* Returns: an integer less than, equal to, or greater than zero if the key 
*   value (arg1) is less than, a match, or greater than the name in the 
*   array member in arg2.
*****************************************************************************/
static int parameter_name_compare
(
    const void *arg1,  /* key value being sought, a string in this case */
    const void *arg2   /* array member to compare to */
)
{
    /* this shouldn't happen here, but check inputs just to be sure... */
    if (arg1 == NULL || arg2 == NULL)
        return -1; /* indicates no match */

    /* arg1 is the key, a string */
    char *key = (char *)arg1;

    /* arg2 is an array member, a parameter_name_id struct */
    struct PARAMETER_NAME_ID *mem = (struct PARAMETER_NAME_ID *)arg2;

    return(strncmp(key, mem->parameter_name, PARAMETER_NAME_LENGTH));
}


/****************************************************************************
* Name: insert_new_parameter
*
* Description:  If a BPF parameter name isn't in the BPF_PARAMETER table, it 
*  needs to be added.  Use a stored procedure to get back the ID of the 
*  newly-inserted parameter.
*
* Returns: the new parameter ID if the parameter was inserted
*          ERROR if the parameter couldn't be inserted
*****************************************************************************/
static int insert_new_parameter
(
    struct ias_db_connection *conn,     /* I: database connection */
    char *parameter_name                /* I: name of parameter to add */
)
{
    int new_parameter_id = ERROR;       /* ID of the new parameter entry */
    int table_size;                     /* number of items in the table */
    int status;                         /* database operation status */

    /* Set up a table to call the parameter creation stored procedure 
     * for this single parameter */
    IAS_DB_STORED_PROC_TABLE parameter_table[] =
    {
        {NULL, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_STRING, 
            parameter_name, strlen(parameter_name), NULL},
        {NULL, IAS_DB_PARAMETER_MODE_OUTPUT, IAS_DB_FIELD_INT, 
            &new_parameter_id, sizeof(new_parameter_id), NULL}
    };

    table_size  = sizeof(parameter_table) / sizeof(IAS_DB_STORED_PROC_TABLE);

    status = ias_db_stored_proc_using_table(conn, 
        "process_bpf_pkg.proc_create_bpf_parameter_name",
        parameter_table, table_size, 1);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Unable to create BPF parameter entry for %s", 
            parameter_name);
        return ERROR;
    }

    return new_parameter_id;
}

/****************************************************************************
* Name: ias_bpf_db_validate_contents
*
* Description: Performs various validations on the contents of the BPF data 
* structure before attempting to write to the database.  If certain fields 
* are not filled in, they will be set by this routine also:
*         file_attributes.file_name
*         file_attributes.file_source
*         file_attributes.description
*         file_attributes.baseline_date
*         file_attributes.sensor_name
*         file_attributes.spacecraft_name
*
* Returns: SUCCESS if the BPF data is valid
*          ERROR if the BPF data is not valid (log messages indicate the 
*                problem)
*
* Notes:   It is recommended this routine be called before attempting
*          to write a BPF data structure's contents to the database.
*****************************************************************************/
int ias_bpf_db_validate_contents
(
     IAS_BPF *bpf                       /* I/O: BPF data to validate       */
) 
{
    const struct IAS_BPF_FILE_ATTRIBUTES *file_attributes = NULL;
                                        /* FILE_ATTRIBUTES group data */

    const struct IAS_BPF_ORBIT_PARAMETERS *orbit_parameters = NULL;
                                        /* ORBIT_PARAMETERS group data */

    const struct IAS_BPF_BIAS_MODEL       *bias_model = NULL;
                                        /* BIAS_MODEL group data */

    IAS_SENSOR_ID sensor_id;            /* ID number for the given BPF
                                           sensor */

    char expected_filename[IAS_BPF_FILE_NAME_LENGTH];        
                                        /* file name we would expect based on
                                           the file naming convention using 
                                           information from file attributes */
    char sensor_char[2];                /* Sensor character string
                                           "O" for OLI, "T" for TIRS */
    int band_list[IAS_MAX_NBANDS];      /* Sensor-specific band list */
    int bpf_is_valid = SUCCESS;         /* start assuming everything's ok  */
    int index;                          /* Index to sensor-specific band
                                           list */
    int number_of_bands;                /* Number of bands in band list */
    int status;                         /* date/time parsing status        */

    /* Parsed date/times */
    IAS_DATETIME effective_date_begin;  /* begin date of this bpf           */
    IAS_DATETIME effective_date_end;    /* end date of this bpf             */
    IAS_DATETIME temp_datetime;         /* parsed date to ensure other dates 
                                           in the file are valid            */

    /* First ensure that an actual structure was supplied */
    if (bpf == NULL)
    {
        IAS_LOG_ERROR("Input BPF structure is NULL");
        return ERROR;
    }

    /* Determine whether this is an OLI or TIRS BPF.  */
    sensor_id = ias_bpf_get_sensor_id(bpf);
    if (sensor_id == IAS_INVALID_SENSOR_ID)
    {
        IAS_LOG_ERROR("Retrieving sensor ID number from BPF file "
            "attributes sensor name");
        return ERROR;
    }
    if (sensor_id == IAS_OLI)
    {
        sprintf(sensor_char, "O");
    }
    else
    {
        sprintf(sensor_char, "T");
    }

    /* FILE ATTRIBUTES ------------------------------------------------- */
    /* Get the FILE_ATTRIBUTES group */
    file_attributes = ias_bpf_get_file_attributes(bpf);
    if (file_attributes == NULL)
    {
        IAS_LOG_ERROR("Cannot retrieve IAS_BPF_BAND_ATTRIBUTES group");
        return ERROR;
    }

    /* effective_date_begin: must be valid date format */
    status = ias_misc_parse_datetime_string(
        file_attributes->effective_date_begin,
        IAS_DATETIME_CPF_FORMAT, &effective_date_begin);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Format of effective_date_begin (%s)",
            file_attributes->effective_date_begin);
        bpf_is_valid = ERROR;
    }

    /* effective_date_end: must be valid date format and must be 
     * equal to or later to begin date */
    status = ias_misc_parse_datetime_string(
        file_attributes->effective_date_end,
        IAS_DATETIME_CPF_FORMAT, &effective_date_end);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Format of effective_date_end (%s)",
            file_attributes->effective_date_end);
        bpf_is_valid = ERROR;
    }
    if ((effective_date_end.year * 366 * 24 * 60 * 60
        + effective_date_end.day_of_year * 24 * 60 * 60
        + effective_date_end.hour  * 60 * 60
        + effective_date_end.minute * 60
        + effective_date_end.second)
        < (effective_date_begin.year * 366 * 24 * 60 * 60
        + effective_date_begin.day_of_year * 24 * 60 * 60
        + effective_date_begin.hour  * 60 * 60
        + effective_date_begin.minute * 60
        + effective_date_begin.second))
    {
        IAS_LOG_ERROR("End date (%s) is earlier than begin date (%s)",
            file_attributes->effective_date_end,
            file_attributes->effective_date_begin);
        bpf_is_valid = ERROR;
    }

    /* Set description if not supplied */
    if (strcmp(file_attributes->description, "") == 0)
    {
        ias_bpf_set_file_attributes_description(bpf, DESCRIPTION);
    }

    /* Set satellite if not supplied */
    if (strcmp(file_attributes->spacecraft_name, "") == 0)
    {
        ias_bpf_set_file_attributes_spacecraft_name(bpf, SPACECRAFT);
    }

    /* baseline_date: must be valid date format */
    if (strcmp(file_attributes->baseline_date, "") == 0)
    {
        /* If not specified, set today's date */
        status = ias_misc_get_timestamp("%FT%T",
                sizeof(file_attributes->baseline_date),
                (char *)file_attributes->baseline_date);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Obtaining current date/time for BPF baseline"
                          "date");
            bpf_is_valid = ERROR;
        }
    }
    else
    {
        status = ias_misc_parse_datetime_string(
            file_attributes->baseline_date,
            IAS_DATETIME_CPF_FORMAT, &temp_datetime);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Format of baseline_date (%s)", 
                file_attributes->baseline_date);
            bpf_is_valid = ERROR;
        }
    }

    /* version: must be >= 0 */
    if (file_attributes->version < 0)
    {
        IAS_LOG_ERROR("Invalid value for version (%d)", 
            file_attributes->version);
        bpf_is_valid = ERROR;
    }

    /* file_name: check format & that it matches effective dates & version */
    status = snprintf(expected_filename, sizeof(expected_filename),
        "L%s8BPF%04d%02d%02d%02d%02d%02d_%04d%02d%02d%02d%02d%02d.%02d",
        sensor_char,
        effective_date_begin.year,         effective_date_begin.month, 
        effective_date_begin.day_of_month, effective_date_begin.hour, 
        effective_date_begin.minute,       (int)effective_date_begin.second,
        effective_date_end.year,           effective_date_end.month, 
        effective_date_end.day_of_month,   effective_date_end.hour, 
        effective_date_end.minute,         (int)effective_date_end.second, 
        file_attributes->version);
    if ((status < 0) || (status >= sizeof(expected_filename)))
    {
        IAS_LOG_ERROR("Constructing expected BPF file name string");
        bpf_is_valid = ERROR;
    }

    if (strcmp(file_attributes->file_name, "") == 0)
    {
        /* If not specified, set the file name */
        ias_bpf_set_file_attributes_file_name(bpf, expected_filename);
    }
    else
    {
        /* Make sure the provided file name at least contains the 
         * expected format.  This will allow an "eval_" prefix as well */
        if (strstr(file_attributes->file_name, expected_filename) == NULL)
        {
            IAS_LOG_ERROR("BPF file name %s does not match effective dates "
                "and/or version", file_attributes->file_name);
            bpf_is_valid = ERROR;
        }
    }

    if (strcmp(file_attributes->file_source, "") == 0)
    {
        /* If not specified, set the file source to a blank string */
        /* so it doesn't come out of the service as "null" */
        ias_bpf_set_file_attributes_file_source(bpf, " ");
    }

    /* ORBIT PARAMETERS -----------------------------------------------*/
    orbit_parameters = ias_bpf_get_orbit_parameters(bpf);
    if (orbit_parameters == NULL)
    {
        IAS_LOG_ERROR("Cannot retrieve IAS_BPF_ORBIT_PARAMETERS group");
        return ERROR;
    }

    /* orbit_number: must be >=0 */
    if (orbit_parameters->begin_orbit_number < 0)
    {
        IAS_LOG_ERROR("Invalid value for begin_orbit_number (%d)", 
            orbit_parameters->begin_orbit_number);
        bpf_is_valid = ERROR;
    }

    /* BIAS MODEL PARAMETERS -----------------------------------------*/
    /* band_number: must be a valid OLI or TIRS band */
    /* band_classification: one of the known BPF band classification types */
    /* spectral type:  correct for the given band number */
    bias_model = ias_bpf_get_bias_model(bpf);
    if (bias_model == NULL)
    {
        IAS_LOG_ERROR("Cannot retrieve IAS_BPF_BIAS_MODEL group");
        return ERROR;
    }

    /* Get the band list for the specified sensor. */
    status = ias_sat_attr_get_sensor_band_numbers(sensor_id,
        IAS_NORMAL_BAND, 0, band_list, IAS_MAX_NBANDS, &number_of_bands);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Retrieving sensor-specific band list");
        return ERROR;
    }

    /* Check the given set of bands in the bias model array. */ 
    for (index = 0; index < number_of_bands; index++)
    {
        int band_class_match;
        int band_number = band_list[index];
        int band_index  =
            ias_sat_attr_convert_band_number_to_index(band_number);
        if (band_index == ERROR)
        {
            IAS_LOG_ERROR("Invalid band number %d", band_number);
            return ERROR;
        }

        /* Check for valid band numbers. */
        if (band_number != bias_model[band_index].band_number)
        {
            IAS_LOG_ERROR("Band number %d does not match expected "
                "band number %d for current index %d", band_number,
                bias_model[band_index].band_number, band_index);
            bpf_is_valid = ERROR;
        }

        /* Confirm the current bias model band number is an imaging band. */
        band_class_match = ias_sat_attr_band_classification_matches(
            bias_model[band_index].band_number, IAS_NORMAL_BAND);
        if (!band_class_match)
        {
            IAS_LOG_ERROR("Bias model band number %d does not "
                "correspond to a normal OLI / TIRS imaging band",
                bias_model[band_index].band_number);
            bpf_is_valid = ERROR;
        }

        /* Confirm the bias model spectral type is correct for the
           current band number. */
        if (bias_model[band_index].spectral_type
            != ias_sat_attr_get_spectral_type_from_band_number(band_number))
        {
            IAS_LOG_ERROR("Spectral type for bias model band number %d "
                "does not match the expected spectral type for band "
                "number %d", bias_model[band_index].band_number,
                band_number);
            bpf_is_valid = ERROR;
        }

        /* Check the data buffers corresponding to each spectral type to
           ensure they've been allocated (and populated) before now. */
        switch (bias_model[band_index].spectral_type)
        {
            case IAS_SPECTRAL_PAN:
            {
                if (bias_model[band_index].pan == NULL)
                {
                    IAS_LOG_ERROR("OLI PAN band data buffers are not valid");
                    bpf_is_valid = ERROR;
                }
                break;
            }
            case IAS_SPECTRAL_VNIR:
            {
                if (bias_model[band_index].vnir == NULL)
                {
                    IAS_LOG_ERROR("OLI VNIR band %d data buffer is not "
                        "valid", band_number);
                    bpf_is_valid = ERROR;
                }
                break;
            }
            case IAS_SPECTRAL_SWIR:
            {
                if (bias_model[band_index].swir == NULL)
                {
                    IAS_LOG_ERROR("OLI SWIR band %d data buffer is not "
                        "valid", band_number);
                    bpf_is_valid = ERROR;
                }
                break;
            }
            case IAS_SPECTRAL_THERMAL:
            {
                if (bias_model[band_index].thermal == NULL)
                {
                    IAS_LOG_ERROR("TIRS thermal band %d data buffer is "
                        "not valid", band_number);
                    bpf_is_valid = ERROR;
                }
                break;
            }
            default:  /* Should NEVER get here normally--the earlier
                         comparison of the spectral type against the
                         expected type for the current band number should
                         catch an invalid value.  If we do get here, some
                         serious investigation is required to figure out
                         why...*/
            {
                IAS_LOG_ERROR("Invalid spectral type %d for band %d",
                    bias_model[band_index].spectral_type, band_number);
                bpf_is_valid = ERROR;
            }
        }
    }

    /* Done */
    return bpf_is_valid;

} /* end of ias_bpf_db_validate_contents */
