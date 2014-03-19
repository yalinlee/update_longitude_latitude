/**************************************************************************
 NAME:     ias_rlut_read_linearization_params

 PURPOSE:  Reads the HDF5 file containing the linearization parameters
           for all detectors in the current band and SCA

 RETURNS:  Pointer to an array of LINEARIZATION_PARAMS data
           structures if successful; NULL pointer if there's an error

 NOTES:    The calling routine is responsible for freeing the memory
           block allocated here
**************************************************************************/
#include <stdlib.h>
#include "hdf5.h"
#include "hdf5_hl.h"
#include "ias_logging.h"
#include "ias_rlut.h"
#include "ias_rlut_private.h"
#include "ias_const.h"


IAS_RLUT_LINEARIZATION_PARAMS *ias_rlut_read_linearization_params
(
    const IAS_RLUT_IO *rlut,         /* I: Open RLUT file */
    int band_number,                 /* I: Current RLUT band number */
    int sca_number,                  /* I: Current SCA number */
    int num_detectors                /* I: Number of detectors in the
                                        current band/SCA */
)
{
    IAS_RLUT_LINEARIZATION_PARAMS *linearization_params = NULL;
                                               /* Pointer to an array of
                                                  data structures containing
                                                  the linearization
                                                  parameters for all detectors
                                                  in the current band/SCA */
    char bandsca_parameter_name[IAS_RLUT_BANDSCA_GROUP_NAME_LENGTH + 1];
                                               /* Linearization parameter
                                                  group name for the current
                                                  band/SCA */
    const char *field_names[IAS_RLUT_PARAM_NFIELDS];
                                               /* Name of each linearization
                                                  parameter */
    size_t offsets[IAS_RLUT_PARAM_NFIELDS];    /* Data offsets in
                                                  LINEARIZATION_PARAMS
                                                  data structure for each
                                                  field*/
    size_t field_sizes[IAS_RLUT_PARAM_NFIELDS];/* Size of each field */
    hid_t field_types[IAS_RLUT_PARAM_NFIELDS]; /* Data type for each field */
    hid_t fields_to_close[IAS_RLUT_PARAM_NFIELDS];
                                               /* Flags indicating open
                                                  fields needing to be
                                                  closed */
    hid_t linearization_param_group_id;        /* Root
                                                  LINEARIZATION_PARAMETERS
                                                  group */
    hid_t bandsca_group_id;                    /* SCA group handle */
    hsize_t nfields = 0;                       /* Number of fields in the
                                                  table description */
    hsize_t nrecords = 0;                      /* Number of records in the
                                                  table description (should
                                                  equal the number of
                                                  detectors) */
    hsize_t type_size;                         /* Size of entire data
                                                  structure to be read into */
    herr_t hdf_status;                         /* HDF5 error status flag */
    int status;                                /* IAS status */


    /* Make sure the RLUT file is actually open */
    if ((rlut == NULL) || (rlut->file_id < 0))
    {
        IAS_LOG_ERROR("NULL pointer to IAS_RLUT_IO data block, or no RLUT "
            "file has been opened");
        return NULL; 
    }

    /* Construct the group name for the current band/SCA */
    status = snprintf(bandsca_parameter_name, sizeof(bandsca_parameter_name),
        "%s/Band%02d_SCA%02d", LINEARIZATION_PARAMS_GROUP_NAME, band_number,
        sca_number);
    if ((status < 0) || (status >= sizeof(bandsca_parameter_name)))
    {
        IAS_LOG_ERROR("Creating group name for band %d SCA %d "
            "linearization parameters", band_number, sca_number);
        return NULL;
    }

    /* Open the root group*/
    linearization_param_group_id = H5Gopen(rlut->file_id,
        LINEARIZATION_PARAMS_GROUP_NAME, H5P_DEFAULT);
    if (linearization_param_group_id < 0)
    {
        IAS_LOG_ERROR("Opening root linearization parameters group");
        return NULL;
    }

    /* Try to open the group for the current band/SCA */
    bandsca_group_id = H5Gopen(linearization_param_group_id,
        bandsca_parameter_name, H5P_DEFAULT);
    if (bandsca_group_id < 0)
    {
        IAS_LOG_ERROR("Opening band %d SCA %d linearization parameter group",
            band_number, sca_number);
        H5Gclose(linearization_param_group_id);
        return NULL;
    }

    /* Build the table definition */
    status = ias_rlut_build_linearization_params_table_description(offsets,
        field_names, field_types, fields_to_close, field_sizes);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Building linearization parameter table description");
        H5Gclose(bandsca_group_id);
        H5Gclose(linearization_param_group_id);
        return NULL;
    }

    /* Get the number of fields and records */
    hdf_status = H5TBget_table_info(bandsca_group_id,
        LINEARIZATION_PARAMS_DATASET_NAME, &nfields, &nrecords);
    if (hdf_status < 0)
    {
        IAS_LOG_ERROR("Getting parameter table information for band %d SCA "
            "%d", band_number, sca_number);
        ias_rlut_cleanup_table_description(fields_to_close,
            IAS_RLUT_PARAM_NFIELDS);
        H5Gclose(bandsca_group_id);
        H5Gclose(linearization_param_group_id);
        return NULL;
    }
    else if (nfields != IAS_RLUT_PARAM_NFIELDS)
    {
        IAS_LOG_ERROR("Number of defined fields %d not equal to number of "
            "returned fields %d", IAS_RLUT_PARAM_NFIELDS, (int)nfields);
        ias_rlut_cleanup_table_description(fields_to_close,
            IAS_RLUT_PARAM_NFIELDS);
        H5Gclose(bandsca_group_id);
        H5Gclose(linearization_param_group_id);
        return NULL;
    }
    else if (nrecords != num_detectors)
    {
        IAS_LOG_ERROR("Band %d SCA %d parameter table should have %d "
            "records, found %d records instead", band_number, sca_number,
            num_detectors, (int)nrecords);
        ias_rlut_cleanup_table_description(fields_to_close,
            IAS_RLUT_PARAM_NFIELDS);
        H5Gclose(bandsca_group_id);
        H5Gclose(linearization_param_group_id);
        return NULL;
    }

    /* Allocate the parameter data buffer for the current band/SCA */
    linearization_params = malloc(
        num_detectors * sizeof(IAS_RLUT_LINEARIZATION_PARAMS));
    if (linearization_params == NULL)
    {
        IAS_LOG_ERROR("Allocating linearization parameter data buffer for "
            "band %d SCA %d", band_number, sca_number);
        ias_rlut_cleanup_table_description(fields_to_close,
            IAS_RLUT_PARAM_NFIELDS);
        H5Gclose(bandsca_group_id);
        H5Gclose(linearization_param_group_id);
        return NULL;
    }

    /* Read the parameter set for the current band/SCA */
    type_size = sizeof(*linearization_params);
    hdf_status = H5TBread_table(bandsca_group_id,
        LINEARIZATION_PARAMS_DATASET_NAME, type_size, offsets,
        field_sizes, linearization_params);

    /* Cleanup the table description */
    ias_rlut_cleanup_table_description(fields_to_close,
        IAS_RLUT_PARAM_NFIELDS);

    /* Check the return status from the read */
    if (hdf_status < 0)
    {
        IAS_LOG_ERROR("Reading parameters for band %d SCA %d", band_number,
            sca_number);
        free(linearization_params);
        linearization_params = NULL;
    }

    /* Close the group for the current band/SCA */
    hdf_status = H5Gclose(bandsca_group_id);
    if (hdf_status < 0)
    {
        IAS_LOG_ERROR("Closing band %d SCA %d linearization parameter group",
            band_number, sca_number);
        if (linearization_params)
        {
            free(linearization_params);
            linearization_params = NULL;
        }
    }

    /* Close the main linearization parameter group */
    hdf_status = H5Gclose(linearization_param_group_id);
    if (hdf_status < 0)
    {
        IAS_LOG_ERROR("Closing root linearization parameters group");
        if (linearization_params)
        {
            free(linearization_params);
            linearization_params = NULL;
        }
    }

    return linearization_params;
}   /* END ias_rlut_read_linearization_params */
