/**********************************************************************
 NAME:                         ancillary_read

 PURPOSE: Implements functionality to read attitude and ephemeris data
          records from the ancillary data file.

***********************************************************************/
#include <string.h>
#include <hdf5.h>
#include <hdf5_hl.h>

#include "ias_const.h"
#include "ias_logging.h"

#include "ias_ancillary_io.h"
#include "ias_ancillary_io_private.h"


/*----------------------------------------------------------------------
 NAME:                     read_epoch_time

 PURPOSE:  Reads the requested epoch time attribute from the ancillary
           data file

 RETURNS:  Integer status code of SUCCESS or ERROR

-------------------------------------------------------------------------*/
static int read_epoch_time
(
    hid_t hdf_file_id,       /* I:  Open ancillary data file handle */
    const char *epoch_name,  /* I:  Epoch time name  */
    double epoch_time[3]     /* O:  Array containing epoch time components */
)
{
    herr_t hdf_error_status = -1;    /* HDF5 error status code    */

    hsize_t epoch_dims = 0;          /* Epoch array dimension */

    H5T_class_t type_class;          /* Type class identifier */

    size_t type_size = 0;            /* Size of datatype in bytes */

    int rank = 0;                    /* Number of dimensions for the
                                        epoch time attribute   */


    /* Verify the epoch time array attribute has an array dimension of 1 */
    hdf_error_status = H5LTget_attribute_ndims(hdf_file_id, "/",
        epoch_name, &rank);
    if (hdf_error_status < 0)
    {
        IAS_LOG_ERROR("Retrieving epoch time attribute dimensions");
        return ERROR;
    }
    if (rank != 1)      /* Should only be 1D array */
    {
        IAS_LOG_ERROR("Invalid rank %d detected for epoch time array, "
            "should be 1", rank);
        return ERROR;
    }

    /* The epoch time attribute should be a 1D, 3-element array of datatype
       double */
    hdf_error_status = H5LTget_attribute_info(hdf_file_id, "/",
        epoch_name, &epoch_dims, &type_class, &type_size);
    if ((hdf_error_status < 0) || (epoch_dims != 3)
            || (type_class != H5T_FLOAT)
            || (type_size != sizeof(double)))
    {
        IAS_LOG_ERROR("Invalid epoch time array dimensions/datatype class/"
            "datatype size information");
        return ERROR;
    }

    /* Now get the epoch time components */
    hdf_error_status = H5LTget_attribute_double(hdf_file_id, "/",
        epoch_name, epoch_time);
    if (hdf_error_status < 0)
    {
        IAS_LOG_ERROR("Retrieving epoch time attribute values");
        return ERROR;
    }

   

    /* Done */
    return SUCCESS;
}



/*----------------------------------------------------------------------
 NAME:                      read_attitude_data

 PURPOSE:  Reads attitude data from HDF5-formatted tables in the
           ancillary data file

 RETURNS:  Pointer to an allocated/populated IAS_ANC_ATTITUDE_DATA
           structure if successful, NULL pointer if allocation fails
           or data read fails

------------------------------------------------------------------------*/
static IAS_ANC_ATTITUDE_DATA *read_attitude_data
(
    hid_t hdf_file_id,              /* I: HDF5 ancillary data file handle */
    int file_format_version         /* I: current file format version */
)
{
    const char *field_names[ATTITUDE_NFIELDS];

    double epoch_time[3] = {0.0, 0.0, 0.0};
                                    /* Temporary buffer for epoch time
                                       data */

    int status = SUCCESS;           /* Function return status code */

    size_t field_offsets[ATTITUDE_NFIELDS];
    size_t field_sizes[ATTITUDE_NFIELDS];

    herr_t hdf_error_status = -1;  /* HDF5 I/O error status */

    hid_t field_types[ATTITUDE_NFIELDS];
    hid_t fields_to_close[ATTITUDE_NFIELDS];

    hsize_t nfields = 0;              /* Number of table fields per record */
    hsize_t nrecords = 0;             /* Number of records in table */
    hsize_t type_size = 0;            /* Size of data structure to read */

    IAS_ANC_ATTITUDE_DATA *data = NULL;
                                      /* Pointer to attitude data buffer */


    /* Read the attitude epoch time information */
    status = read_epoch_time(hdf_file_id,
        ATTITUDE_EPOCH_TIME_ATTRIBUTE_NAME, epoch_time);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Reading attitude epoch time attribute");
        return NULL;
    }

    /* Build the attitude table definition. */
    status = ias_ancillary_build_attitude_table_definition(field_names,
       field_offsets, field_types, field_sizes, fields_to_close);  
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Building attitude table definition");
        return NULL;
    }

    /* Get the number of records in the attitude data table.  We need
       this before we can allocate the proper-sized IAS_ANC_ATTITUDE_DATA
       buffer.   If the table doesn't exist or there's a table defined
       with 0 records, consider it an error. */
    hdf_error_status = H5TBget_table_info(hdf_file_id,
        ATTITUDE_DATA_DATASET_NAME, &nfields, &nrecords);
    if (hdf_error_status < 0)
    {
        IAS_LOG_ERROR("Obtaining attitude table information");
        ias_ancillary_cleanup_table_definition(fields_to_close, 
            ATTITUDE_NFIELDS);
        return NULL;
    }
    else if (nrecords < 1)
    {
        IAS_LOG_ERROR("No records found in attitude data table");
        ias_ancillary_cleanup_table_definition(fields_to_close, 
            ATTITUDE_NFIELDS);
        return NULL;
    }

    /* Allocate the attitude data buffer. */
    data = ias_ancillary_allocate_attitude(nrecords);
    if (data == NULL)
    {
        IAS_LOG_ERROR("Allocating attitude data buffer");
        ias_ancillary_cleanup_table_definition(fields_to_close, 
            ATTITUDE_NFIELDS);
        return NULL;
    }
    else
    {
        /* Copy the attitude epoch time info to the data structure. */
        memcpy(data->utc_epoch_time, epoch_time, sizeof(epoch_time));

        /* Read the table contents into the data structure. */
        type_size = sizeof(data->records);
        hdf_error_status = H5TBread_table(hdf_file_id,
            ATTITUDE_DATA_DATASET_NAME, type_size,
            field_offsets, field_sizes, data->records);
        if (hdf_error_status < 0)
        {
            IAS_LOG_ERROR("Reading attitude data table");
            ias_ancillary_free_attitude(data);
            ias_ancillary_cleanup_table_definition(fields_to_close, 
                ATTITUDE_NFIELDS);
            return NULL;
        }
    }

    /* Close any "open" datatype field objects. */
    ias_ancillary_cleanup_table_definition(fields_to_close,
        ATTITUDE_NFIELDS);

    /* Done */
    return data;
}



/*----------------------------------------------------------------------
 NAME:                      read_ephemeris_data

 PURPOSE:  Reads ephemeris data from HDF5-formatted tables in the
           ancillary data file

 RETURNS:  Pointer to an allocated/populated IAS_ANC_EPHEMERIS_DATA
           structure if successful, NULL pointer if allocation fails
           or data read fails

------------------------------------------------------------------------*/
static IAS_ANC_EPHEMERIS_DATA *read_ephemeris_data
(
    hid_t hdf_file_id,              /* I: HDF5 ancillary data file handle */
    int file_format_version         /* I: current file format version */
)
{
    const char *field_names[EPHEMERIS_NFIELDS];

    double epoch_time[3] = {0.0, 0.0, 0.0};
                                    /* Temporary buffer for epoch time data */

    int status = SUCCESS;           /* Function return status code  */

    size_t field_offsets[EPHEMERIS_NFIELDS];
    size_t field_sizes[EPHEMERIS_NFIELDS];

    herr_t hdf_error_status = -1;   /* HDF5 I/O error status   */

    hid_t field_types[EPHEMERIS_NFIELDS];
    hid_t fields_to_close[EPHEMERIS_NFIELDS];

    hsize_t nfields = 0;            /* Number of table fields per record */
    hsize_t nrecords = 0;           /* Number of records in table */
    hsize_t type_size = 0;          /* Size of data structure to read */

    IAS_ANC_EPHEMERIS_DATA *data = NULL;
                                    /* Pointer to attitude data buffer */


    /* Read the attitude epoch time from the ancillary data file. */
    status = read_epoch_time(hdf_file_id,
        EPHEMERIS_EPOCH_TIME_ATTRIBUTE_NAME, epoch_time);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Reading ephemeris epoch time attribute");
        return NULL;
    }

    /* Build the ephemeris table definition. */
    status = ias_ancillary_build_ephemeris_table_definition(field_names,
        field_offsets, field_types, field_sizes, fields_to_close);  
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Building ephemeris table definition");
        return NULL;
    }

    /* Get the number of records in the ephemeris data table.  We need
       this before we can allocate the proper-sized IAS_ANC_EPHEMERIS_DATA
       buffer.   If the table doesn't exist or there's a table defined
       with 0 records, consider it an error. */
    hdf_error_status = H5TBget_table_info(hdf_file_id,
        EPHEMERIS_DATA_DATASET_NAME, &nfields, &nrecords);
    if (hdf_error_status < 0)
    {
        IAS_LOG_ERROR("Obtaining ephemeris table information");
        ias_ancillary_cleanup_table_definition(fields_to_close,
            EPHEMERIS_NFIELDS);
        return NULL;
    }
    else if (nrecords < 1)
    {
        IAS_LOG_ERROR("No records found in ephemeris data table");
        ias_ancillary_cleanup_table_definition(fields_to_close,
            EPHEMERIS_NFIELDS);
        return NULL;
    }

    /* Allocate the ephemeris data buffer. */
    data = ias_ancillary_allocate_ephemeris(nrecords);
    if (data == NULL)
    {
        IAS_LOG_ERROR("Allocating ephemeris data buffer");
        ias_ancillary_cleanup_table_definition(fields_to_close,
            EPHEMERIS_NFIELDS);
        return NULL;
    }
    else
    {
        /* Copy the attitude epoch time info to the data structure. */
        memcpy(data->utc_epoch_time, epoch_time, sizeof(epoch_time));

        /* Read the table contents into the data structure. */
        type_size = sizeof(data->records);
        hdf_error_status = H5TBread_table(hdf_file_id,
            EPHEMERIS_DATA_DATASET_NAME, type_size,
            field_offsets, field_sizes, data->records);
        if (hdf_error_status < 0)
        {
            IAS_LOG_ERROR("Reading ephemeris data table");
            ias_ancillary_free_ephemeris(data);
            ias_ancillary_cleanup_table_definition(fields_to_close,
                EPHEMERIS_NFIELDS);
            return NULL;
        }
    }

    /* Close any "open" datatype field objects. */
    ias_ancillary_cleanup_table_definition(fields_to_close,
        EPHEMERIS_NFIELDS);

    /* Done */
    return data;
}



/***********************************************************************
 NAME:                     ias_ancillary_read

 PURPOSE:  Calls routines to read ancillary and ephemeris data

 RETURNS:  Integer status code of SUCCESS or ERROR

 NOTES:    Calling routine is responsible for ensuring a valid filename

************************************************************************/
int ias_ancillary_read
(
    const char *ancillary_filename,          /* I: Name of data file */
    IAS_ANC_ATTITUDE_DATA **attitude_data,   /* I/O: Pointer to attitude
                                                data */
    IAS_ANC_EPHEMERIS_DATA **ephemeris_data  /* I/O: Pointer to ephemeris
                                                data */
)
{
    hid_t hdf_file_id = -1;          /* HDF5 file handle */

    herr_t hdf_error_status = -1;    /* HDF5 error status code; negative
                                        values indicate an error */
    int file_format_version = 0;     /* File format version */
    int rank = 0;                    /* Number of dimensions for the
                                        format version attribute */

    H5T_class_t type_class;          /* Type class identifier */

    size_t type_size = 0;            /* Size of datatype in bytes */

    hsize_t version_dims = 0;        /* Format version array dimension */

    IAS_ANC_ATTITUDE_DATA *attitude_read = NULL;
    /* working attitude data buffer */

    IAS_ANC_EPHEMERIS_DATA *ephemeris_read = NULL;
    /* working ephemeris data buffer */


    /* Initialize the pointers so caller has something to check against */
    *attitude_data = NULL;
    *ephemeris_data = NULL;

    /* Make sure the ancillary file is "valid" */
    if (!ias_ancillary_is_ancillary_file(ancillary_filename))
    {
        IAS_LOG_ERROR("Ancillary file %s not valid", ancillary_filename);
        return ERROR;
    }

    /* Open the file for reading. */
    hdf_file_id = H5Fopen(ancillary_filename, H5F_ACC_RDONLY, H5P_DEFAULT);
    if (hdf_file_id < 0)
    {
        IAS_LOG_ERROR("Opening ancillary data file %s", ancillary_filename);
        return ERROR;
    }

    /* Verify the file format version attribute has a dimension of 1 */
    hdf_error_status = H5LTget_attribute_ndims(hdf_file_id, "/",
        FILE_FORMAT_VERSION_ATTRIBUTE_NAME, &rank);
    if (hdf_error_status < 0)
    {
        IAS_LOG_ERROR("Retrieving file format version attribute dimensions "
            "in ancillary file %s", ancillary_filename);
        return ERROR;
    }
    if (rank != 1)      /* Should only be 1D array */
    {
        IAS_LOG_ERROR("Invalid rank %d detected for file format version "
            "attribute in ancillary file %s, should be 1",
            rank, ancillary_filename);
        return ERROR;
    }

    /* The file format version attribute should be a scalar of integer
       data type*/
    hdf_error_status = H5LTget_attribute_info(hdf_file_id, "/",
        FILE_FORMAT_VERSION_ATTRIBUTE_NAME, &version_dims,
        &type_class, &type_size);
    if ((hdf_error_status < 0) || (version_dims != 1)
            || (type_class != H5T_INTEGER)
            || (type_size != sizeof(int)))
    {
        IAS_LOG_ERROR("Invalid file format version attribute information "
            "in ancillary file %s", ancillary_filename);
        return ERROR;
    }

    /* Read the file format version information */
    hdf_error_status = H5LTget_attribute_int(hdf_file_id, "/",
        FILE_FORMAT_VERSION_ATTRIBUTE_NAME, &file_format_version);
    if (hdf_error_status < 0) 
    {
        IAS_LOG_ERROR("Reading file format version attribute in ancillary "
            "file %s", ancillary_filename);
        H5Fclose(hdf_file_id);
        return ERROR;
    }

    /* Read the attitude data from the file.   */
    attitude_read = read_attitude_data(hdf_file_id, file_format_version);
    if (attitude_read == NULL)
    {
        IAS_LOG_ERROR("Reading attitude data from ancillary data file %s",
            ancillary_filename);
        H5Fclose(hdf_file_id);
        return ERROR;
    }

    /* Read the ephemeris data from the file.  */
    ephemeris_read = read_ephemeris_data(hdf_file_id, file_format_version);
    if (ephemeris_read == NULL)
    {
        IAS_LOG_ERROR("Reading ephemeris data from ancillary data file %s",
            ancillary_filename);
        ias_ancillary_free_attitude(attitude_read);
        H5Fclose(hdf_file_id);
        return ERROR;
    }

    /* Close the file now that the attitude and ephemeris data have been
       read from it. */
    hdf_error_status = H5Fclose(hdf_file_id);
    if (hdf_error_status < 0)
    {
        IAS_LOG_ERROR("Closing ancillary data file %s", ancillary_filename);
        ias_ancillary_free_attitude(attitude_read);
        ias_ancillary_free_ephemeris(ephemeris_read);
        return ERROR;
    }

    /* Point the input pointers to the working buffers. */
    *attitude_data = attitude_read;
    *ephemeris_data = ephemeris_read;

    /* Done */
    return SUCCESS;
}
