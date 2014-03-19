/**************************************************************************
 NAME:     ias_rlut_write_linearization_params

 PURPOSE:  Writes the linearzation param table for the current band/SCA
           to the RLUT file for the current band and SCA

 RETURNS:  Integer status code of SUCCESS or ERROR

 NOTES:    It is assumed the routine that originally created the
           RLUT file (ias_rlut_open_file() with IAS_WRITE access mode)
           has already created the root linearization parameters group
**************************************************************************/
#include "hdf5.h"
#include "hdf5_hl.h"
#include "ias_logging.h"
#include "ias_rlut.h"
#include "ias_rlut_private.h"
#include "ias_const.h"


int ias_rlut_write_linearization_params
(
    const IAS_RLUT_IO *rlut_file,    /* I: Open RLUT file */
    const IAS_RLUT_LINEARIZATION_PARAMS *linearization_params,
                                     /* I: Pointer to an array of data
                                        structures containing the
                                        linearization parameters for all
                                        detectors in the current band/SCA */
    int band_number,                 /* I: Current band number */
    int sca_number,                  /* I: Current SCA number*/
    int num_detectors                /* I: Number of detectors in the
                                        current band/SCA */
)
{
    char bandsca_parameter_name[IAS_RLUT_BANDSCA_GROUP_NAME_LENGTH + 1];
                                           /* Linearization parameter group
                                              name for the current band/SCA */
    const char *field_names[IAS_RLUT_PARAM_NFIELDS];
                                           /* Name of each linearization
                                              parameter */
    size_t offsets[IAS_RLUT_PARAM_NFIELDS];/* Data offsets in
                                              LINEARIZATION_PARAMS
                                              data structure for each
                                              field*/
    size_t field_sizes[IAS_RLUT_PARAM_NFIELDS];
                                           /* Size of each field */
    hid_t field_types[IAS_RLUT_PARAM_NFIELDS];
                                           /* Data type for each field */
    hid_t fields_to_close[IAS_RLUT_PARAM_NFIELDS];
                                           /* Flags indicating open
                                              fields needing to be closed */
    hid_t linearization_param_group_id;    /* Root LINEARIZATION_PARAMETERS
                                              group ID */
    hid_t bandsca_group_id;                /* SCA group ID */
    hsize_t type_size;                     /* Size of base
                                              LINEARIZATION_PARAMS
                                              data structure */
    herr_t hdf_status;                     /* HDF5 error status flag */
    int status;                            /* IAS status flags */
    int return_status = SUCCESS;


    /* Make sure the RLUT file is actually open */
    if ((rlut_file == NULL) || (rlut_file->file_id < 0))
    {
        IAS_LOG_ERROR("NULL pointer to IAS_RLUT_IO data block, or no RLUT "
            "file has been opened");
        return ERROR;
    }

    /* Construct the group name for the current band/SCA */
    status = snprintf(bandsca_parameter_name, sizeof(bandsca_parameter_name),
        "%s/Band%02d_SCA%02d", LINEARIZATION_PARAMS_GROUP_NAME, band_number,
        sca_number);
    if ((status < 0) || (status >= sizeof(bandsca_parameter_name)))
    {
        IAS_LOG_ERROR("Creating group name for band %d SCA %d "
            "linearization parameters", band_number, sca_number);
        return ERROR;
    }

    /* Open the root group */
    linearization_param_group_id = H5Gopen(rlut_file->file_id,
            LINEARIZATION_PARAMS_GROUP_NAME, H5P_DEFAULT);
    if (linearization_param_group_id < 0)
    {
        IAS_LOG_ERROR("Opening root linearization parameters group");
        return ERROR;
    }

    /* Create a new group for the current band/SCA within this group */
    bandsca_group_id = H5Gcreate(linearization_param_group_id,
       bandsca_parameter_name, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (bandsca_group_id < 0)
    {
        IAS_LOG_ERROR("Creating group for band %d SCA %d linearization "
            "parameters", band_number, sca_number);
        H5Gclose(linearization_param_group_id);
        return ERROR;
    }

    /* Build the table definition */
    status = ias_rlut_build_linearization_params_table_description(offsets,
        field_names, field_types, fields_to_close, field_sizes);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Building linearization parameter table description");
        H5Gclose(bandsca_group_id);
        H5Gclose(linearization_param_group_id);
        return ERROR;
    }

    /* Get the size of the data structure */
    type_size = sizeof(*linearization_params);

    /* Write the parameter set for the current band/SCA */
    hdf_status = H5TBmake_table(LINEARIZATION_PARAMS_TABLE_NAME,
        bandsca_group_id, LINEARIZATION_PARAMS_DATASET_NAME,
        IAS_RLUT_PARAM_NFIELDS, num_detectors, type_size, field_names,
        offsets, field_types, sizeof(IAS_RLUT_LINEARIZATION_PARAMS), NULL, 0, 
        linearization_params);

    /* Cleanup the table description */
    ias_rlut_cleanup_table_description(fields_to_close,
        IAS_RLUT_PARAM_NFIELDS);

    /* Check the return status from the write */
    if (hdf_status < 0)
    {
        IAS_LOG_ERROR("Writing band %d SCA %d linearization parameter "
            "table to RLUT file %s", band_number, sca_number,
            rlut_file->filename);
        return_status = ERROR;
    }

    /* Close the local SCA group */
    hdf_status = H5Gclose(bandsca_group_id);
    if (hdf_status < 0)
    {
        IAS_LOG_ERROR("Closing band %d SCA %d linearization parameter "
            "group", band_number, sca_number);
        return_status = ERROR;
    }

    /* Close the main linearization parameter group */
    hdf_status = H5Gclose(linearization_param_group_id);
    if (hdf_status < 0)
    {
        IAS_LOG_ERROR("Closing root LINEARIZATION_PARAMETERS group");
        return_status = ERROR;
    }

    return return_status;
}   /* END ias_rlut_write_linearization_params */ 
