/*--------------------------------------------------------------------------
NAME:    ias_rlut_write_tirs_secondary_linearization_params

PURPOSE: Writes the TIRS secondary linearization parameters for the current
         band/SCA.

RETURNS: SUCCESS or ERROR
--------------------------------------------------------------------------*/
#include "hdf5.h"
#include "hdf5_hl.h"
#include "ias_logging.h"
#include "ias_rlut.h"
#include "ias_rlut_private.h"
#include "ias_const.h"


/*------------------------------------------------------------------------------
NAME:    write_array

PURPOSE: This utility function will write a 2D array of values to a data set
         in an HDF5 file for the TIRS secondary linearization function.  This
         function can be used to write the lookup DN values as well as
         correction values.

RETURNS: SUCCESS or ERROR
------------------------------------------------------------------------------*/
static int write_array
(
    hid_t group_id,       /* I: HDF5 group ID being written to */
    hsize_t dims[2],      /* I: Dimensions of the 'values' parameter */
    const char *hdf_path, /* I: Full path of the data set */
    double *values        /* I: 2D array to be written to the file */
)
{
    herr_t h_status;      /* HDF5 status */
    hid_t  dataset_id;
    hid_t  dataspace_id;

    /* Create a new HDF5 dataspace for the table of values.  The first argument
       indicates the number of dimensions in the data space.  The last argument
       is the maximum dimensions of the data space, but by passing in a NULL
       pointer, the maximum dimensions will be set to the same as the initial
       dimensions. */
    dataspace_id = H5Screate_simple(2, dims, NULL);
    if (dataspace_id < 0)
    {
        IAS_LOG_ERROR("Creating an HDF5 data space for %s", hdf_path);
        return ERROR;
    }

    /* Create the data set for the table of values within the HDF5 file */
    dataset_id = H5Dcreate(group_id, hdf_path, H5T_IEEE_F64LE,
        dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (dataset_id < 0)
    {
        IAS_LOG_ERROR("Creating an HDF5 data space for %s", hdf_path);
        H5Sclose(dataspace_id);
        return ERROR;
    }

    /* Write the 2D array of values to the HDF5 file */
    h_status = H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL,
        H5P_DEFAULT, values);
    if (h_status < 0)
    {
        IAS_LOG_ERROR("Writing to HDF5 data space for %s", hdf_path);
        H5Dclose(dataset_id);
        H5Sclose(dataspace_id);
        return ERROR;
    }

    /* Close the data set we just wrote */
    h_status = H5Dclose(dataset_id);
    if (h_status < 0)
    {
        IAS_LOG_ERROR("Closing data set for %s", hdf_path);
        H5Sclose(dataspace_id);
        return ERROR;
    }

    /* Close the data space we created for the table */
    h_status = H5Sclose(dataspace_id);
    if (h_status < 0)
    {
        IAS_LOG_ERROR("Closing data space for %s", hdf_path);
        return ERROR;
    }

    return SUCCESS;
}  /* END write_array */


/*------------------------------------------------------------------------------
NAME:    ias_rlut_write_tirs_secondary_linearization_params

PURPOSE: Write the parameter tables contained in the structure to the RLUT for
         the current band and SCA

RETURNS: SUCCESS or ERROR
------------------------------------------------------------------------------*/
int ias_rlut_write_tirs_secondary_linearization_params
(
    const IAS_RLUT_IO *rlut_file,    /* I: Open RLUT file */
    int band_number,                 /* I: */
    int sca_number,                  /* I: */
    int num_detectors,               /* I: */
    IAS_RLUT_TIRS_SECONDARY_LINEARIZATION_PARAMS *linearization_params
                                     /* I: Pointer to a structure containing
                                           linearization parameters for all
                                           detectors of the band/SCA */
)
{
    int status;
    char group_name[TIRS_DATASET_LENGTH];
    hid_t root_group_id;
    hid_t group_id;
    hsize_t dims[2];
    herr_t hdf_status;

    /* Ensure the RLUT file is open */
    if ( rlut_file == NULL || rlut_file->file_id < 0 )
    {
        IAS_LOG_ERROR("The RLUT file has not been opened");
        return ERROR;
    }

    /* Create the group name */
    status = snprintf( group_name, TIRS_DATASET_LENGTH, "/%s/Band%02d_SCA%02d",
        TIRS_SECONDARY_LINEARIZATION_GROUP_NAME, band_number, sca_number );
    if ( status < 0 || status >= TIRS_DATASET_LENGTH )
    {
        IAS_LOG_ERROR("Creating the group name vor Band %d SCA %d",
            band_number, sca_number );
        return ERROR;
    }

    /* Open the root group */
    root_group_id = H5Gopen( rlut_file->file_id,
        TIRS_SECONDARY_LINEARIZATION_GROUP_NAME, H5P_DEFAULT );
    if ( root_group_id < 0 )
    {
        IAS_LOG_ERROR("Opening root %s group",
            TIRS_SECONDARY_LINEARIZATION_GROUP_NAME );
        return ERROR;
    }

    /* Create a new group for the band/SCA */
    group_id = H5Gcreate( root_group_id, group_name, H5P_DEFAULT,
        H5P_DEFAULT, H5P_DEFAULT );
    if ( group_id < 0 )
    {
        IAS_LOG_ERROR("Creating group for Band %d SCA %d\n+++ %s", band_number,
            sca_number, group_name );
        H5Gclose( root_group_id );
        return ERROR;
    }

    /* Initialize the dimensions of the data */
    dims[0] = linearization_params->num_detectors;
    dims[1] = linearization_params->num_values;

    /* Write the DN lookup values */
    status = write_array( group_id, dims, TIRS_DN_LUT,
        linearization_params->input_dn );
    if ( status != SUCCESS )
    {
        IAS_LOG_ERROR("Writing the %s group for Band %d SCA %d",
            TIRS_DN_LUT, band_number, sca_number );
        H5Gclose( group_id );
        H5Gclose( root_group_id );
        return ERROR;
    }

    /* Write the Correction Factor lookup values */
    status = write_array( group_id, dims, TIRS_CORR_FAC,
        linearization_params->output_correction );
    if ( status != SUCCESS )
    {
        IAS_LOG_ERROR("Writing the Correction Factor group for Band %d SCA %d",
            band_number, sca_number );
        H5Gclose( group_id );
        H5Gclose( root_group_id );
        return ERROR;
    }

    /* Close the groups */
    hdf_status = H5Gclose( group_id );
    if ( hdf_status < 0 )
    {
        IAS_LOG_ERROR("Closing group %s group_name", group_name );
        H5Gclose( root_group_id );
        return ERROR;
    }
    hdf_status = H5Gclose( root_group_id );
    if ( hdf_status < 0 )
    {
        IAS_LOG_ERROR("Closing group %s",
            TIRS_SECONDARY_LINEARIZATION_GROUP_NAME );
        return ERROR;
    }

    return SUCCESS;
}  /* END ias_rlut_write_tirs_secondary_linearization_params */
