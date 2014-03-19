/**********************************************************************
 NAME:                    ancillary_write

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


/*---------------------------------------------------------------------
 NAME:                     write_epoch_time

 PURPOSE:  Writes the requested epoch time attribute to the ancillary
           data file

 RETURNS:  Integer sttaucode of SUCCESS or ERROR

-----------------------------------------------------------------------*/
static int write_epoch_time
(
    hid_t hdf_file_id,        /* I:  Open ancillary data file handle  */
    const char *epoch_name,   /* I:  Epoch time name  */
    const double epoch_time[3]
                              /* I:  Array containing epoch time components */ 
)
{
    herr_t hdf_error_status;       /* HDF5 error status flag */


    /* Write the epoch time components by "setting" the attribute value
       associated with the file. */
    hdf_error_status = H5LTset_attribute_double(hdf_file_id, "/",
        epoch_name, epoch_time, 3);

    if (hdf_error_status < 0)
    {
        IAS_LOG_ERROR("Writing epoch time attribute");
        return ERROR;
    }

    /* Done */
    return SUCCESS;
}



/*---------------------------------------------------------------------*
 NAME:                      write_attitude_data

 PURPOSE:  Writes attitude data to HDF5-formatted tables in the
           ancillary data file

 RETURNS:  Integer status code of SUCCESS or ERROR

-----------------------------------------------------------------------*/
static int write_attitude_data
(
    hid_t hdf_file_id,                     /* I: HDF5 ancillary data file
                                              handle */
    const IAS_ANC_ATTITUDE_DATA *attitude_data   
                                          /* I: Pointer to attitude data
                                              structure  */
)
{
    const char *field_names[ATTITUDE_NFIELDS];

    size_t field_offsets[ATTITUDE_NFIELDS];
    size_t field_sizes[ATTITUDE_NFIELDS];

    int status = SUCCESS;              /* IAS function return status code  */

    herr_t hdf_error_status = -1;      /* HDF5 I/O error status   */

    hid_t field_types[ATTITUDE_NFIELDS];
    hid_t fields_to_close[ATTITUDE_NFIELDS];


    /* Write the attitude epoch time to the ancillary data file. */
    status = write_epoch_time(hdf_file_id,
        ATTITUDE_EPOCH_TIME_ATTRIBUTE_NAME,
        attitude_data->utc_epoch_time);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Writing attitude epoch time to ancillary file");
        return ERROR;
    }

    /* Build the attitude table definition. */
    status = ias_ancillary_build_attitude_table_definition(field_names,
        field_offsets, field_types, field_sizes, fields_to_close);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Cannot build attitude table definition");
        return ERROR;
    }

    /* Write the information to the attitude table.  Since the
       file was created using the H5F_ACC_TRUNC keyword (which
       wipes out any previous file contents), we'll
       have to write all attitude records to the table. */
    hdf_error_status = H5TBmake_table(ATTITUDE_DATA_TABLE_NAME, hdf_file_id,
        ATTITUDE_DATA_DATASET_NAME, ATTITUDE_NFIELDS,
        attitude_data->number_of_samples,
        sizeof(attitude_data->records), field_names,
        field_offsets, field_types, sizeof(IAS_ANC_ATTITUDE_RECORD),
        NULL, 0, attitude_data->records);

    ias_ancillary_cleanup_table_definition(fields_to_close,
        ATTITUDE_NFIELDS);

    /* Check for errors writing the attitude data to the table.  */
    if (hdf_error_status < 0)
    {
        IAS_LOG_ERROR("Writing attitude data table to ancillary file");
        return ERROR;
    }

    /* Everything went OK.  */
    return SUCCESS;
}



/*---------------------------------------------------------------------*
 NAME:                      write_ephemeris_data

 PURPOSE:  Writes ephemeris data to HDF5-formatted tables in the
           ancillary data file

 RETURNS:  Integer status code of SUCCESS or ERROR

-----------------------------------------------------------------------*/
static int write_ephemeris_data
(
    hid_t hdf_file_id,                      /* I: HDF5 ancillary data file
                                               handle */
    const IAS_ANC_EPHEMERIS_DATA *ephemeris_data
                                            /* I: Pointer to ephemeris data
                                               structure   */
)
{  
    const char *field_names[EPHEMERIS_NFIELDS];

    int status = SUCCESS;                   /* IAS function return status
                                               code   */

    size_t field_offsets[EPHEMERIS_NFIELDS];
    size_t field_sizes[EPHEMERIS_NFIELDS];

    herr_t hdf_error_status = -1;           /* HDF5 I/O error status    */

    hid_t field_types[EPHEMERIS_NFIELDS];
    hid_t fields_to_close[EPHEMERIS_NFIELDS];


    /* Write the ephemeris epoch time to the ancillary data file. */
    status = write_epoch_time(hdf_file_id,
        EPHEMERIS_EPOCH_TIME_ATTRIBUTE_NAME,
        ephemeris_data->utc_epoch_time); 
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Writing ephemeris epoch time to ancillary file");
        return ERROR;
    }

    /* Build the ephemeris table definition. */
    status = ias_ancillary_build_ephemeris_table_definition(field_names,
        field_offsets, field_types, field_sizes, fields_to_close);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Cannot build ephemeris table definition");
        return ERROR;
    }

    /* Write the information to the ephemeris table.  Since the
       file was created using the H5F_ACC_TRUNC keyword (which
       wipes out any previous file contents), we'll
       have to write all ephemeris records to the table. */
    hdf_error_status = H5TBmake_table(EPHEMERIS_DATA_TABLE_NAME,
        hdf_file_id, EPHEMERIS_DATA_DATASET_NAME, EPHEMERIS_NFIELDS,
        ephemeris_data->number_of_samples, sizeof(ephemeris_data->records),
        field_names, field_offsets, field_types,
        sizeof(IAS_ANC_EPHEMERIS_RECORD), NULL, 0, ephemeris_data->records);

    /* Cleanup.  Close any "open" datatype field objects.  */
   ias_ancillary_cleanup_table_definition(fields_to_close,
       EPHEMERIS_NFIELDS);

    /* Check for errors. */
    if (hdf_error_status < 0)
    {
        IAS_LOG_ERROR("Writing ephemeris data table to ancillary file");
        return ERROR;
    }

    /* Done */
    return SUCCESS;
}



/***********************************************************************
 NAME:                     ias_ancillary_write

 PURPOSE:  Calls routines to write ancillary and ephemeris data

 RETURNS:  Integer status code of SUCCESS or ERROR

 NOTES:    Calling routine is responsible for ensuring i) a valid filename;
           and 2) that the attitude and ephemeris data buffers are valid
           and populated

************************************************************************/
int ias_ancillary_write
(
    const char *ancillary_filename,         /* I: Name of data file   */
    const IAS_ANC_ATTITUDE_DATA *attitude_data,
                                            /* I: Pointer to attitude data */
    const IAS_ANC_EPHEMERIS_DATA *ephemeris_data
                                            /* I: Pointer to ephemeris data */
)
{
    int status = SUCCESS;          /* General status code  */
    int file_format_version = 0;   /* Ancillary data file format
                                      version number */

    hid_t hdf_file_id = -1;        /* HDF5 ancillary data file handle  */
    herr_t hdf_error_status = -1;  /* HDF5 error status code;
                                      negative values indicate an
                                      error */


    /* Open the file for writing, "truncating" it (i.e.
       removing the current contents) so that we're always writing the
       most up-to-date attitude and ephemeris data we have. */
    hdf_file_id = H5Fcreate(ancillary_filename, H5F_ACC_TRUNC, H5P_DEFAULT,
        H5P_DEFAULT);
    if (hdf_file_id < 0)
    {
        IAS_LOG_ERROR("Creating ancillary data file %s.", ancillary_filename);
        return ERROR;
    }

    /* Update the file format version information. */
    file_format_version = FILE_FORMAT_VERSION;
    hdf_error_status = H5LTset_attribute_int(hdf_file_id, "/",
        FILE_FORMAT_VERSION_ATTRIBUTE_NAME, &file_format_version, 1);
    if (hdf_error_status < 0)
    {
        IAS_LOG_ERROR("Writing file format version attribute to ancillary "
            "file %s", ancillary_filename);
        H5Fclose(hdf_file_id);
        return ERROR;
    }

    /* Write the attitude data to the file. */
    status = write_attitude_data(hdf_file_id, attitude_data);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Writing attitude data to ancillary file %s",
            ancillary_filename);
        H5Fclose(hdf_file_id);
        return ERROR;
    }

    /* Now write the ephemeris data to the file. */
    status = write_ephemeris_data(hdf_file_id, ephemeris_data);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Writing ephemeris data to ancillary file %s",
            ancillary_filename);
        H5Fclose(hdf_file_id);
        return ERROR;
    }

    /* Close the file. */
    hdf_error_status = H5Fclose(hdf_file_id);
    if (hdf_error_status < 0)
    {
        IAS_LOG_ERROR("Closing ancillary data file %s", ancillary_filename);
        return ERROR;
    }

    /* Done */
    return SUCCESS;
}



