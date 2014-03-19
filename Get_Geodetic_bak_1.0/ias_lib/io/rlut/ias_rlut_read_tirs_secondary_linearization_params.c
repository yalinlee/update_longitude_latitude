/*--------------------------------------------------------------------------
NAME:    ias_rlut_read_tirs_secondary_linearization_params
         ias_rlut_free_tirs_secondary_linearization

PURPOSE: Reads the TIRS secondary linearization parameters for the current
         band/SCA.

RETURNS: Pointer to a IAS_RLUT_TIRS_SECONDARY_LINEARIZATION_PARAMS structure
         NULL on ERROR

NOTE:    The calling routine is responsible for freeing the memory allocated
         here with a call to ias_rlut_free_tirs_secondary_linearization
--------------------------------------------------------------------------*/
#include <stdlib.h>

#include "hdf5.h"
#include "hdf5_hl.h"
#include "ias_logging.h"
#include "ias_rlut.h"
#include "ias_rlut_private.h"
#include "ias_const.h"


/*-------------------------------------------------------------------------
NAME:    read_dataset

PURPOSE: Local routine to allocate memory for the dataset named in the input
         parameter and read the dataset

RETURNS: pointer to allocated and filled memory
         NULL on error
--------------------------------------------------------------------------*/
static double *read_dataset
(
    const IAS_RLUT_IO *rlut, /* I: Open RLUT file */
    char *dataset_name,      /* I: */
    hsize_t dims[2]          /* O: The dimensions of the dataset */
)
{
    int status;
    double *params;
    hid_t dataset_id;
    hid_t dataspace_id;
    herr_t hdf_status;

    /* Open the data set for reading */
    dataset_id = H5Dopen( rlut->file_id, dataset_name, H5P_DEFAULT );
    if ( dataset_id < 0 )
    {
        IAS_LOG_ERROR("Opening the dataset: %s", dataset_name );
        return NULL;
    }

    /* Get the data space for the data set */
    dataspace_id = H5Dget_space( dataset_id );
    if ( dataspace_id < 0 )
    {
        IAS_LOG_ERROR("Getting the data space for %s", dataset_name );
        H5Dclose( dataset_id );
        return NULL;
    }

    /* Get the dimensions of the data space */
    status = H5Sget_simple_extent_ndims( dataspace_id );
    if ( status != 2 )
    {
        IAS_LOG_ERROR("Data set is not 2 dimensional: %s", dataset_name );
        H5Dclose( dataset_id );
        return NULL;
    }
    hdf_status = H5Sget_simple_extent_dims( dataspace_id, dims, NULL );
    if ( hdf_status < 0 )
    {
        IAS_LOG_ERROR("Getting the dimensions of %s", dataset_name );
        H5Dclose( dataset_id );
        return NULL;
    }

    /* Allocate the array of return parameters */
    params = malloc( sizeof(double) * dims[0] * dims[1] );
    if ( ! params )
    {
        IAS_LOG_ERROR("Allocation memory for %s", dataset_name );
        H5Dclose( dataset_id );
        return NULL;
    }

    /* Read the DN lookup table */
    hdf_status = H5Dread( dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL,
        H5P_DEFAULT, params );
    if ( hdf_status < 0 )
    {
        IAS_LOG_ERROR("Reading data set %s", dataset_name );
        free( params );
        H5Dclose( dataset_id );
        return NULL;
    }

    hdf_status = H5Dclose( dataset_id );
    if ( hdf_status < 0 )
    {
        IAS_LOG_ERROR("Closing data set %s", dataset_name );
        free( params );
        return NULL;
    }
    return params;
}  /* END read_dataset */

/*--------------------------------------------------------------------------
NAME:    ias_rlut_free_tirs_secondary_linearization

PURPOSE: Free the memory allocated in the read routine

RETURNS: none
--------------------------------------------------------------------------*/
void ias_rlut_free_tirs_secondary_linearization
(
    IAS_RLUT_TIRS_SECONDARY_LINEARIZATION_PARAMS *params /* I: structure to
                                                               free */
)
{
    if ( params == NULL )
        return;
    free( params->input_dn );
    params->input_dn = NULL;
    free( params->output_correction );
    params->output_correction = NULL;
    free( params );
}  /* END ias_rlut_free_tirs_secondary_linearization */


IAS_RLUT_TIRS_SECONDARY_LINEARIZATION_PARAMS *
    ias_rlut_read_tirs_secondary_linearization_params
(
    const IAS_RLUT_IO *rlut, /* I: Open RLUT file */
    int band_number,         /* I: */
    int sca_number,          /* I: */
    int num_detectors        /* I: Number of detectors in the SCA */
)
{
    IAS_RLUT_TIRS_SECONDARY_LINEARIZATION_PARAMS *params;
    int status;
    char dataset_name[TIRS_DATASET_LENGTH];
    hsize_t dims[2];

    /* Ensure the RLUT file is open */
    if ( rlut == NULL || rlut->file_id < 0 )
    {
        IAS_LOG_ERROR("The RLUT file has not been opened");
        return NULL;
    }

    /* Create the dataset name for the DN lookup */
    status = snprintf( dataset_name, TIRS_DATASET_LENGTH,
        "/%s/Band%02d_SCA%02d/%s", TIRS_SECONDARY_LINEARIZATION_GROUP_NAME,
        band_number, sca_number, TIRS_DN_LUT );
    if ( status < 0 || status >= TIRS_DATASET_LENGTH )
    {
        IAS_LOG_ERROR("Creating the group name for Band %d SCA %d",
            band_number, sca_number );
        return NULL;
    }

    /* Allocate the return structure */
    params = malloc( sizeof(IAS_RLUT_TIRS_SECONDARY_LINEARIZATION_PARAMS));
    if ( ! params )
    {
        IAS_LOG_ERROR("Allocating data structure for Band %d SCA %d",
            band_number, sca_number );
        return NULL;
    }
    params->output_correction = NULL;
    params->input_dn = NULL;

    /* Read the DN LUT */
    params->input_dn = read_dataset( rlut, dataset_name, dims );
    if ( ! params->input_dn )
    {
        IAS_LOG_ERROR("Reading data set: %s", dataset_name );
        ias_rlut_free_tirs_secondary_linearization( params );
        return NULL;
    }
    params->num_detectors = dims[0];
    params->num_values = dims[1];

    /* Create the dataset name for the linearization parameters */
    status = snprintf( dataset_name, TIRS_DATASET_LENGTH,
        "/%s/Band%02d_SCA%02d/%s",
        TIRS_SECONDARY_LINEARIZATION_GROUP_NAME, band_number, sca_number,
        TIRS_CORR_FAC );
    if ( status < 0 || status >= TIRS_DATASET_LENGTH )
    {
        IAS_LOG_ERROR("Creating the group name for Band %d SCA %d",
            band_number, sca_number );
        ias_rlut_free_tirs_secondary_linearization( params );
        return NULL;
    }

    /* Read the linearity corrections */
    params->output_correction = read_dataset( rlut, dataset_name, dims );
    if ( ! params->output_correction )
    {
        IAS_LOG_ERROR("Reading data set: %s", dataset_name );
        ias_rlut_free_tirs_secondary_linearization( params );
        return NULL;
    }

    /* The dimensions of the linearity correction must match the DN LUT */
    if ( params->num_detectors != dims[0]
        || params->num_values != dims[1] )
    {
        IAS_LOG_ERROR("The dimentions of the TIRS secondary linearity "
            "table do not match: %dx%d and %dx%d", params->num_detectors,
            params->num_values, (int)dims[0], (int)dims[1] );
        ias_rlut_free_tirs_secondary_linearization( params );
        return NULL;
    }

    return params;
}  /* END ias_rlut_read_tirs_secondary_linearization_params */
