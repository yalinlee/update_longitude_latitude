/*-----------------------------------------------------------------------------

NAME: ias_l0r_header.c

PURPOSE: Functions used for read and writing Frame/Image header data

ALGORITHM REFERENCES: NONE

NOTES:

-----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

/* include library headers */
#include "hdf5.h"

/* project headers */
#include "ias_l0r.h"
#include "ias_l0r_hdf.h"
#include "ias_logging.h"
#include "ias_types.h"

/*******************************************************************************
 constant values for the sizes/offsets/names/etc used with tables
*******************************************************************************/
#define GROUP_NAME_OLI "OLI"
#define GROUP_NAME_TIRS "TIRS"

#define TABLE_NAME_IMAGEHEADER "Image_Header"
#define TABLE_NAME_FRAMEHEADER "Frame_Headers"

#define TABLE_SIZE_OLI_IMAGE_HEADER 24
#define TABLE_SIZE_OLI_FRAME_HEADER 10
#define TABLE_SIZE_TIRS_FRAME_HEADER 18

/*******************************************************************************
*Image Header
*declare a set of constant values for the sizes/offsets used with the OHD table
*******************************************************************************/
static const hid_t  *field_type_image_header = NULL;
static hid_t image_header_types[TABLE_SIZE_OLI_IMAGE_HEADER];

static const char *IAS_FRAME_FIELD_NAMES[TABLE_SIZE_OLI_IMAGE_HEADER] =
{
    "l0r_time_days_from_J2000",
    "l0r_time_seconds_of_day",
    "days_original",
    "milliseconds_original",
    "microseconds_original",
    "blind_data_included_in_frame",
    "frame_number",
    "time_error",
    "reserved",
    "frame_status",
    "length_of_image",
    "image_content_definition",
    "ms_integration_time",
    "pan_integration_time",
    "ms_data_word",
    "pan_data_word",
    "extended_integration_flag",
    "blind_band_record_rate",
    "test_pattern_setting",
    "current_detector_select_table",
    "reserved_1",
    "detector_select_table_id_number",
    "image_data_truncation_setting",
    "reserved_2"
};
static const IAS_L0R_OLI_IMAGE_HEADER *oliImageHeaderPtr = NULL;
static const size_t IAS_L0R_OLI_IMAGE_HEADER_SIZE =
    sizeof( IAS_L0R_OLI_IMAGE_HEADER );
static const size_t
    IAS_L0R_OLI_IMAGE_HEADER_SIZES[TABLE_SIZE_OLI_IMAGE_HEADER] =
{
    sizeof( (*oliImageHeaderPtr).frame_header.l0r_time.days_from_J2000),
    /*uint32_t days_from_J2000*/
    sizeof( (*oliImageHeaderPtr).frame_header.l0r_time.seconds_of_day),
    /*double seconds*/
    sizeof( (*oliImageHeaderPtr).frame_header.day),
    sizeof( (*oliImageHeaderPtr).frame_header.milliseconds),
    sizeof( (*oliImageHeaderPtr).frame_header.microseconds),
    sizeof( (*oliImageHeaderPtr).frame_header.blind_data_included_in_frame),
    sizeof( (*oliImageHeaderPtr).frame_header.frame_number),
    sizeof( (*oliImageHeaderPtr).frame_header.time_error),
    sizeof( (*oliImageHeaderPtr).frame_header.reserved),
    sizeof( (*oliImageHeaderPtr).frame_header.frame_status),
    sizeof( (*oliImageHeaderPtr).length_of_image),
    sizeof( (*oliImageHeaderPtr).image_content_definition),
    sizeof( (*oliImageHeaderPtr).ms_integration_time),
    sizeof( (*oliImageHeaderPtr).pan_integration_time),
    sizeof( (*oliImageHeaderPtr).ms_data_word),
    sizeof( (*oliImageHeaderPtr).pan_data_word),
    sizeof( (*oliImageHeaderPtr).extended_integration_flag),
    sizeof( (*oliImageHeaderPtr).blind_band_record_rate),
    sizeof( (*oliImageHeaderPtr).test_pattern_setting),
    sizeof( (*oliImageHeaderPtr).current_detector_select_table),
    sizeof( (*oliImageHeaderPtr).reserved_1),
    sizeof( (*oliImageHeaderPtr).detector_select_table_id_number),
    sizeof( (*oliImageHeaderPtr).image_data_truncation_setting),
    sizeof( (*oliImageHeaderPtr).reserved_2)
};

static const size_t IAS_IMAGE_HEADER_OFFSET[TABLE_SIZE_OLI_IMAGE_HEADER] =
{
    HOFFSET( IAS_L0R_OLI_IMAGE_HEADER,frame_header.l0r_time.days_from_J2000),
    HOFFSET( IAS_L0R_OLI_IMAGE_HEADER,frame_header.l0r_time.seconds_of_day),
    HOFFSET( IAS_L0R_OLI_IMAGE_HEADER,frame_header.day),
    HOFFSET( IAS_L0R_OLI_IMAGE_HEADER,frame_header.milliseconds),
    HOFFSET( IAS_L0R_OLI_IMAGE_HEADER,frame_header.microseconds),
    HOFFSET( IAS_L0R_OLI_IMAGE_HEADER,frame_header.
        blind_data_included_in_frame),
    HOFFSET( IAS_L0R_OLI_IMAGE_HEADER,frame_header.frame_number),
    HOFFSET( IAS_L0R_OLI_IMAGE_HEADER,frame_header.time_error),
    HOFFSET( IAS_L0R_OLI_IMAGE_HEADER,frame_header.reserved),
    HOFFSET( IAS_L0R_OLI_IMAGE_HEADER,frame_header.frame_status),
    HOFFSET( IAS_L0R_OLI_IMAGE_HEADER,length_of_image),
    HOFFSET( IAS_L0R_OLI_IMAGE_HEADER,image_content_definition),
    HOFFSET( IAS_L0R_OLI_IMAGE_HEADER,ms_integration_time),
    HOFFSET( IAS_L0R_OLI_IMAGE_HEADER,pan_integration_time),
    HOFFSET( IAS_L0R_OLI_IMAGE_HEADER,ms_data_word),
    HOFFSET( IAS_L0R_OLI_IMAGE_HEADER,pan_data_word),
    HOFFSET( IAS_L0R_OLI_IMAGE_HEADER,extended_integration_flag),
    HOFFSET( IAS_L0R_OLI_IMAGE_HEADER,blind_band_record_rate),
    HOFFSET( IAS_L0R_OLI_IMAGE_HEADER,test_pattern_setting),
    HOFFSET( IAS_L0R_OLI_IMAGE_HEADER,current_detector_select_table),
    HOFFSET( IAS_L0R_OLI_IMAGE_HEADER,reserved_1),
    HOFFSET( IAS_L0R_OLI_IMAGE_HEADER,detector_select_table_id_number),
    HOFFSET( IAS_L0R_OLI_IMAGE_HEADER,image_data_truncation_setting),
    HOFFSET( IAS_L0R_OLI_IMAGE_HEADER,reserved_2)
};

/*******************************************************************************
*Frame Header
*declare a set of constant values for the sizes/offsets used with the OHD table
*******************************************************************************/
static const hid_t *field_type_frame_header = NULL;
static hid_t oli_frame_header_types[TABLE_SIZE_OLI_FRAME_HEADER];

static const char *IAS_L0R_OLI_FRAME_HEADER_NAMES[TABLE_SIZE_OLI_FRAME_HEADER] =
{
    "l0r_time_days_from_J2000",
    "l0r_time_seconds_of_day",
    "days_original",
    "milliseconds_original",
    "microseconds_original",
    "blind_data_included_in_frame",
    "frame_number",
    "time_error",
    "reserved",
    "frame_status"
};
static const IAS_L0R_OLI_FRAME_HEADER *oliFrameHeaderPtr = NULL;
static const size_t IAS_L0R_OLI_FRAME_HEADER_SIZE =
    sizeof( IAS_L0R_OLI_FRAME_HEADER );
static const size_t
    IAS_L0R_OLI_FRAME_HEADER_SIZES[TABLE_SIZE_OLI_FRAME_HEADER] =
{
    sizeof( (*oliFrameHeaderPtr).l0r_time.days_from_J2000),
    /*uint32_t days_from_J2000*/
    sizeof( (*oliFrameHeaderPtr).l0r_time.seconds_of_day),
    /*double seconds*/
    sizeof( (*oliFrameHeaderPtr).day),
    sizeof( (*oliFrameHeaderPtr).milliseconds),
    sizeof( (*oliFrameHeaderPtr).microseconds),
    sizeof( (*oliFrameHeaderPtr).blind_data_included_in_frame),
    sizeof( (*oliFrameHeaderPtr).frame_number), /*uint32_t frame_number*/
    sizeof( (*oliFrameHeaderPtr).time_error), /*uint8_t time_error*/
    sizeof( (*oliFrameHeaderPtr).reserved), /*uint8_t reserved*/
    sizeof( (*oliFrameHeaderPtr).frame_status)/*uint16_t frame_status*/
};

static const size_t IAS_FRAME_HEADER_OFFSET[TABLE_SIZE_OLI_FRAME_HEADER] =
{
    HOFFSET( IAS_L0R_OLI_FRAME_HEADER,l0r_time.days_from_J2000),
    HOFFSET( IAS_L0R_OLI_FRAME_HEADER,l0r_time.seconds_of_day),
    HOFFSET( IAS_L0R_OLI_FRAME_HEADER,day),
    HOFFSET( IAS_L0R_OLI_FRAME_HEADER,milliseconds),
    HOFFSET( IAS_L0R_OLI_FRAME_HEADER,microseconds),
    HOFFSET( IAS_L0R_OLI_FRAME_HEADER,blind_data_included_in_frame),
    HOFFSET( IAS_L0R_OLI_FRAME_HEADER,frame_number),
    HOFFSET( IAS_L0R_OLI_FRAME_HEADER,time_error),
    HOFFSET( IAS_L0R_OLI_FRAME_HEADER,reserved),
    HOFFSET( IAS_L0R_OLI_FRAME_HEADER,frame_status)
};
/******************************************************************************
*TIRS Frame header
******************************************************************************/
static const hid_t  *field_type_tirs_frame_header = NULL;
static hid_t tirs_frame_header_types[TABLE_SIZE_TIRS_FRAME_HEADER];

static const char *FIELD_NAMES_TIRS_LINE[TABLE_SIZE_TIRS_FRAME_HEADER] =
{
    "l0r_time_days_from_J2000", /* uint32_t days_from_J2000 */
    "l0r_time_seconds_of_day", /* double milliseconds */
    "days_original",
    "milliseconds_original",
    "microseconds_original",
    "frame_number",
    "sync_byte", /* uint8_t sync_byte */
    "reserved", /* uint8_t reserved */
    "data_set_type", /* uint8_t data_set_type */
    "integration_duration", /* uint8_t integration_duration */
    "total_frames_requested",
    "row_offsets",
    "d_header",
    "fpe_words",
    "roic_crc_status_blind",
    "roic_crc_status_10_8",
    "roic_crc_status_12",
    "frame_status" /* uint16_t frame_status */
};

static const IAS_L0R_TIRS_FRAME_HEADER *tirsFrameHeaderPtr;
static const size_t IAS_L0R_TIRS_FRAME_HEADER_SIZE =
    sizeof(IAS_L0R_TIRS_FRAME_HEADER);
static const size_t
    IAS_L0R_TIRS_FRAME_HEADER_SIZES[TABLE_SIZE_TIRS_FRAME_HEADER] =
{
    sizeof( (*tirsFrameHeaderPtr).l0r_time.days_from_J2000),
    sizeof( (*tirsFrameHeaderPtr).l0r_time.seconds_of_day),
    sizeof( (*tirsFrameHeaderPtr).day),
    sizeof( (*tirsFrameHeaderPtr).milliseconds),
    sizeof( (*tirsFrameHeaderPtr).microseconds),
    sizeof( (*tirsFrameHeaderPtr).frame_number),
    sizeof( (*tirsFrameHeaderPtr).sync_byte),
    sizeof( (*tirsFrameHeaderPtr).reserved),
    sizeof( (*tirsFrameHeaderPtr).data_set_type),
    sizeof( (*tirsFrameHeaderPtr).integration_duration),
    sizeof( (*tirsFrameHeaderPtr).total_frames_requested),
    sizeof( (*tirsFrameHeaderPtr).row_offset),
    sizeof( (*tirsFrameHeaderPtr).d_header),
    sizeof( (*tirsFrameHeaderPtr).fpe_words),
    sizeof( (*tirsFrameHeaderPtr).roic_crc_status_blind),
    sizeof( (*tirsFrameHeaderPtr).roic_crc_status_10_8),
    sizeof( (*tirsFrameHeaderPtr).roic_crc_status_12),
    sizeof( (*tirsFrameHeaderPtr).frame_status)
};

static const size_t
    IAS_L0R_TIRS_FRAME_HEADER_OFFSET[TABLE_SIZE_TIRS_FRAME_HEADER] =
{
     HOFFSET( IAS_L0R_TIRS_FRAME_HEADER,l0r_time.days_from_J2000),
     HOFFSET( IAS_L0R_TIRS_FRAME_HEADER,l0r_time.seconds_of_day),
     HOFFSET( IAS_L0R_TIRS_FRAME_HEADER,day),
     HOFFSET( IAS_L0R_TIRS_FRAME_HEADER,milliseconds),
     HOFFSET( IAS_L0R_TIRS_FRAME_HEADER,microseconds),
     HOFFSET( IAS_L0R_TIRS_FRAME_HEADER,frame_number),
     HOFFSET( IAS_L0R_TIRS_FRAME_HEADER,sync_byte),
     HOFFSET( IAS_L0R_TIRS_FRAME_HEADER,reserved),
     HOFFSET( IAS_L0R_TIRS_FRAME_HEADER,data_set_type),
     HOFFSET( IAS_L0R_TIRS_FRAME_HEADER,integration_duration),
     HOFFSET( IAS_L0R_TIRS_FRAME_HEADER,total_frames_requested),
     HOFFSET( IAS_L0R_TIRS_FRAME_HEADER,row_offset),
     HOFFSET( IAS_L0R_TIRS_FRAME_HEADER,d_header),
     HOFFSET( IAS_L0R_TIRS_FRAME_HEADER,fpe_words),
     HOFFSET( IAS_L0R_TIRS_FRAME_HEADER,roic_crc_status_blind),
     HOFFSET( IAS_L0R_TIRS_FRAME_HEADER,roic_crc_status_10_8),
     HOFFSET( IAS_L0R_TIRS_FRAME_HEADER,roic_crc_status_12),
     HOFFSET( IAS_L0R_TIRS_FRAME_HEADER,frame_status)
};

/******************************************************************************
*private functions
******************************************************************************/
/******************************************************************************
 NAME: ias_l0r_establish_header_file

 PURPOSE: Initializes access to or creates the header file

 RETURNS: SUCCESS- Access had been established
          ERROR- Could not establish access
******************************************************************************/
static int ias_l0r_establish_header_file
(
    HDFIO *hdfio_ptr, /* I: Structure used in I/O */
    const int create_if_absent /* I: Create the file if it does not exist */
)
{
    if (hdfio_ptr == NULL)
    {
        return ERROR;
    }

    char ancillary_filename[IAS_L0R_FILE_NAME_LENGTH+1];
    const char *ancillary_appendix = "_ANC.h5\0";

    if (hdfio_ptr->file_id_header > 0)
    {
        return SUCCESS;
    }

    /* If the file for the ancillary data has been opened reuse the id */
    if (hdfio_ptr->file_id_ancillary > 0)
    {
        hdfio_ptr->file_id_header = hdfio_ptr->file_id_ancillary;
        return SUCCESS;
    }

    if (hdfio_ptr->access_mode_header == -1)
    {
        IAS_LOG_ERROR("Header file is not open");
        return ERROR;
    }

    if ((strlen(hdfio_ptr->l0r_name_prefix) + strlen(ancillary_appendix) + 1)  >
        IAS_L0R_FILE_NAME_LENGTH)
    {
        IAS_LOG_ERROR("Ancillary file name of %s%s is to long. Max is %i",
            hdfio_ptr->l0r_name_prefix, ancillary_appendix,
            IAS_L0R_FILE_NAME_LENGTH);
        return ERROR;
    }

    snprintf(ancillary_filename, IAS_L0R_FILE_NAME_LENGTH,
        "%s%s", hdfio_ptr->l0r_name_prefix,
        ancillary_appendix);

    return ias_l0r_hdf_establish_file(hdfio_ptr->path, ancillary_filename,
        &hdfio_ptr->file_id_header, hdfio_ptr->access_mode_header,
        create_if_absent);
}

/******************************************************************************
* NAME: ias_l0r_table_header_records_count
*
* PURPOSE: an internal routine used to obtain the size of a table
*
* RETURNS: SUCCESS- Size was determined
*          FAILURE- Unable to determine size
******************************************************************************/
static int ias_l0r_table_header_records_count
(
    HDFIO *hdfio_ptr, /* I: Pointer to structure used in I/O */
    const char *table, /* I: Table name */
    int *size          /* O: Number of records in the table */
)
{
    int status;
    status = ias_l0r_establish_header_file(hdfio_ptr, FALSE);
    if (status == ERROR)
    {
        return ERROR;
    }
    /*Since empty files are not to exist the case for size 0 is as follows
    *the establish_file routine did not have an error, but the id is unset */
    else if (hdfio_ptr->file_id_header < 1)
    {
        *size = 0;
        return SUCCESS;
    }

    /* if the file exists and there wasn't an error, get the size of the
    * table */
    status = ias_l0r_hdf_table_records_count(hdfio_ptr->file_id_header,
         table, size);

    return status;
}


/******************************************************************************
 NAME: ias_l0r_get_headers

 PURPOSE: Internal routine to read the headers from the table name
          specified into a buffer

 RETURNS: SUCCESS- Data was read into the buffer
          FAILURE- Unable to read into buffer
******************************************************************************/
static int ias_l0r_get_headers
(
    L0RIO *l0r, /* I: structure for the file used in I/O */
    const char *table, /* I: Table name to read from */
    const int index, /* I: first record to read */
    const int count, /* I: number of records to read */
    const size_t size, /* I: size in bytes of the structure to read into */
    const size_t *offset, /* I: List of the offsets for each member */
    const size_t *size_list, /* I: List of sizes for each member */
    void *buffer /* O: Data buffer */
)
{
    int table_size;
    int status;
    herr_t hdf_status;
    HDFIO *hdfio_ptr = NULL;

    if (l0r == NULL)
    {
        IAS_LOG_ERROR("Error NULL pointer received");
        return ERROR;
    }

    hdfio_ptr =  l0r;

    if (hdfio_ptr->access_mode_header < 0)
    {
        IAS_LOG_ERROR("Access to OLI header data is not open, unable to read");
        return ERROR;
    }

    /*clear the buffer to initialize structure members that are not in the
    *table, useful for reading older files that may not contain new fields */
    memset(buffer, 0, size * count);

    /* make sure the file exists and is in read mode */
    status = ias_l0r_establish_header_file(hdfio_ptr, FALSE);
    if (status == ERROR || hdfio_ptr->file_id_header <= 0)
    {
        IAS_LOG_ERROR("Error establishing a file for frame header data");
        return ERROR;
    }

    /* verify there are an adequate number of records for the read */
    if (ias_l0r_table_header_records_count(hdfio_ptr,
         table, &table_size) == ERROR)
    {
        IAS_LOG_ERROR("Unable to determine size of %s", table);
        return ERROR;
    }
    if (table_size < index + count)
    {
        IAS_LOG_ERROR("Insufficient records to read from "
            "%s. %d attempted from %d at"
            " %d", table, count, table_size, index);
        return ERROR;
    }

    /* read records */
    hdf_status = H5TBread_records(hdfio_ptr->file_id_header,
        table, index, count, size, offset,
        size_list, buffer);
    if (hdf_status < 0)
    {
        IAS_LOG_ERROR("Error reading %s", table);
        return ERROR;
    }
    return SUCCESS;
}


/******************************************************************************
 NAME: ias_l0r_truncate_headers

 PURPOSE: Internal routine to truncate the headers contained in the table name
          specified.

 RETURNS: SUCCESS- Table was truncated
          FAILURE- Table could not be truncated
******************************************************************************/
static int ias_l0r_truncate_headers
(
    L0RIO *l0r, /* I: structure for the file used in I/O */
    const char *table, /* I: Table name to truncate */
    const int count /* I: number of records desired for table */
)
{
    hsize_t tabledims;
    hsize_t maxdims;
    hid_t dataset_id;
    hid_t dataspace_id;
    herr_t hdf_status;

    if (l0r == NULL)
    {
        IAS_LOG_ERROR("Error NULL pointer received");
        return ERROR;
    }

    HDFIO *hdfio_ptr =  l0r;
    
    if (hdfio_ptr->access_mode_header != IAS_WRITE)
    {
        IAS_LOG_ERROR("Access to header data is not open, unable to write");
        return ERROR;
    }

    /* get table size, then extend it if needed */
    dataset_id = H5Dopen(hdfio_ptr->file_id_header, table, H5P_DEFAULT);
    if (dataset_id < 0)
    {
        IAS_LOG_ERROR("Retrieving id for %s", table);
        return ERROR;
    }
    dataspace_id = H5Dget_space(dataset_id);
    if (dataspace_id < 0)
    {
        IAS_LOG_ERROR("Unable to get dataspace id for dataset: %i in %s",
            dataset_id, table);
         H5Dclose(dataset_id);
        return ERROR;
    }
    hdf_status = H5Sget_simple_extent_dims(dataspace_id, &tabledims, &maxdims );
    if (hdf_status < 0)
    {
         IAS_LOG_ERROR("Error getting dataspace dimension for id %i in %s",
             dataspace_id, table);
         H5Dclose(dataset_id);
         H5Sclose(dataspace_id);
         return ERROR;
    }
    tabledims = count;
    hdf_status = H5Dset_extent(dataset_id, &tabledims);
    if (hdf_status < 0)
    {
        IAS_LOG_ERROR("Error setting extent for %s", table);
        return ERROR;
    }

    hdf_status = H5Dclose(dataset_id);
    if (hdf_status < 0)
    {
        IAS_LOG_ERROR("Error closing dataset_id for %s",table);
        return ERROR;
    }

    hdf_status = H5Sclose(dataspace_id);
    if (hdf_status < 0)
    {
        IAS_LOG_ERROR("Error closing dataspace_id for %s",table);
        return ERROR;
    }

    return SUCCESS;
}
 
/******************************************************************************
 NAME: ias_l0r_set_headers

 PURPOSE: Internal routine to write the headers contained the table name
          specified from a buffer

 RETURNS: SUCCESS- Data was written to the table
          FAILURE- Data could not be written to the table
******************************************************************************/
static int ias_l0r_set_headers
(
    L0RIO *l0r, /* I: structure for the file used in I/O */
    const char *table, /* I: Table name to write to */
    const int index, /* I: first record to write */
    const int count, /* I: number of records to write */
    const size_t size, /* I: size in bytes of the structure to read into */
    const size_t *offset, /* I: List of the offsets for each member */
    const size_t *size_list, /* I: List of sizes for each member */
    const void *buffer /* I: Data buffer */
)
{
    hsize_t tabledims;
    hsize_t maxdims;
    herr_t hdf_status;
    int error_encountered = 0;

    if (l0r == NULL)
    {
        IAS_LOG_ERROR("Error NULL pointer received");
        return ERROR;
    }

    HDFIO *hdfio_ptr =  l0r;
    
    if (hdfio_ptr->access_mode_header != IAS_WRITE)
    {
        IAS_LOG_ERROR("Access to OLI header data is not open, unable to write");
        return ERROR;
    }

    /* get table size, then extend it if needed */
    hid_t dataset_id = H5Dopen(hdfio_ptr->file_id_header,
        table, H5P_DEFAULT);
    if (dataset_id < 0)
    {
        IAS_LOG_ERROR("Retrieving id for %s", table);
        return ERROR;
    }
    hid_t dataspace_id = H5Dget_space(dataset_id);
    if (dataspace_id < 0)
    {
        IAS_LOG_ERROR("Unable to get dataspace id for dataset: %i",
            dataset_id);
         H5Dclose(dataset_id);
        return ERROR;
    }
    hdf_status = H5Sget_simple_extent_dims(dataspace_id, &tabledims, &maxdims );
    if (hdf_status < 0)
    {
         IAS_LOG_ERROR("Error getting dataspace dimension for id %i",
             dataspace_id);
         H5Dclose(dataset_id);
         H5Sclose(dataspace_id);
         return ERROR;
    }
    if ((index + count) > tabledims)
    {
        tabledims = index + count;
        hdf_status = H5Dset_extent(dataset_id, &tabledims);
        if (hdf_status < 0)
        {
            IAS_LOG_ERROR("Error setting extent for %s", table);
            return ERROR;
        }
    }

    /* write the records */
    hdf_status = H5TBwrite_records(hdfio_ptr->file_id_header, table, index,
        count, size, offset, size_list, buffer);
    if (hdf_status < 0)
        error_encountered = TRUE;

    hdf_status = H5Dclose(dataset_id);
    if (hdf_status < 0)
        error_encountered = TRUE;

    hdf_status = H5Sclose(dataspace_id);
    if (hdf_status < 0)
        error_encountered = TRUE;

    if (error_encountered == TRUE)
    {
        IAS_LOG_ERROR("Error writing or closing dataset_id, dataspace_id");
        return ERROR;
    }

    return SUCCESS;
}


/* sub-routines used for initialization */

/******************************************************************************
 NAME: ias_l0r_image_header_init

 PURPOSE: Initializes the HDF fields for an Image Header

 RETURNS: Pointer to the types array
******************************************************************************/
const hid_t* ias_l0r_image_header_init(void)
{
    int i = 0;
    hid_t array_id1;
    hid_t array_id2;
    hid_t array_id3;

    hsize_t dims1[1] = {IAS_L0R_FRAME_HEADER_RESERVED_LENGTH};
    hsize_t dims2[1] = {IAS_L0R_IMAGE_HEADER_RESERVED_LENGTH_1};
    hsize_t dims3[1] = {IAS_L0R_IMAGE_HEADER_RESERVED_LENGTH_2};

    /* set up the parameters that are arrays */
    array_id1 = H5Tarray_create( H5T_STD_U8LE, 1, dims1);
    if (array_id1 < 0)
    {
        IAS_LOG_ERROR("Call to H5Tarray_create failed");
        return NULL;
    }
    array_id2 = H5Tarray_create( H5T_STD_U8LE, 1, dims2);
    if (array_id2 < 0)
    {
        IAS_LOG_ERROR("Call to H5Tarray_create failed");
        return NULL;
    }
    array_id3 = H5Tarray_create( H5T_STD_U8LE, 1, dims3);
    if (array_id3 < 0)
    {
        IAS_LOG_ERROR("Call to H5Tarray_create failed");
        return NULL;
    }

    image_header_types[i++] = H5T_STD_I32LE;
            /* int32_t Days from J2000 */
    image_header_types[i++] = H5T_IEEE_F64LE; /* double Seconds */
    image_header_types[i++] = H5T_STD_I16LE; /* day */
    image_header_types[i++] = H5T_STD_I32LE; /* milliseconds of day */
    image_header_types[i++] = H5T_STD_I16LE; /* microsecond of ms */
    image_header_types[i++] = H5T_STD_U8LE;
        /* uint8_t Blind_Data_included_in */
    image_header_types[i++] = H5T_STD_U32LE; /* uint32_t frame_number */
    image_header_types[i++] = H5T_STD_U8LE;  /* uint8_t time_error */
    image_header_types[i++] = array_id1;
        /* uint8_t Reserved[LINE_RESERVED_LEN] */
    image_header_types[i++] = H5T_STD_U16LE; /* uint16_t frame_status */
    image_header_types[i++] = H5T_STD_U32LE;
    image_header_types[i++] = H5T_STD_U32LE;
    image_header_types[i++] = H5T_STD_U16LE;
    image_header_types[i++] = H5T_STD_U16LE;
    image_header_types[i++] = H5T_STD_U32LE;
    image_header_types[i++] = H5T_STD_U32LE;
    image_header_types[i++] = H5T_STD_U8LE;
    image_header_types[i++] = H5T_STD_U8LE;
    image_header_types[i++] = H5T_STD_U8LE;
    image_header_types[i++] = H5T_STD_U8LE;
    image_header_types[i++] = array_id2;
    image_header_types[i++] = H5T_STD_U32LE;
    image_header_types[i++] = H5T_STD_U8LE;
    image_header_types[i++] = array_id3;

    if (i != TABLE_SIZE_OLI_IMAGE_HEADER)
    {
        IAS_LOG_ERROR(
            "The number of types %d does not match the number of columns %d"
            " for table %s",
            i, TABLE_SIZE_OLI_IMAGE_HEADER,
            GROUP_NAME_OLI"/"TABLE_NAME_IMAGEHEADER);
        return NULL;
    }

    return image_header_types;
}

/******************************************************************************
 NAME: ias_l0r_oli_frame_header_init

 PURPOSE: Initializes the HDF types for a frame header

 RETURNS: Pointer to the types array
******************************************************************************/
const hid_t* ias_l0r_oli_frame_header_init(void)
{
    int i = 0;
    /* set up the parameters that are arrays */
    hsize_t dims[1] = {IAS_L0R_FRAME_HEADER_RESERVED_LENGTH};
    hid_t array_id;

    array_id = H5Tarray_create( H5T_STD_U8LE, 1, dims);
    if (array_id < 0)
    {
        IAS_LOG_ERROR("Call to H5Tarray_create failed");
        return NULL;
    }

    oli_frame_header_types[i++] = H5T_STD_I32LE;
            /* int32_t Days from J2000 */
    oli_frame_header_types[i++] = H5T_IEEE_F64LE; /* double Seconds */
    oli_frame_header_types[i++] = H5T_STD_I16LE; /* day */
    oli_frame_header_types[i++] = H5T_STD_I32LE; /* milliseconds of day */
    oli_frame_header_types[i++] = H5T_STD_I16LE; /* microsecond of ms */
    oli_frame_header_types[i++] = H5T_STD_U8LE;
        /* uint8_t Blind_Data_included_in */
    oli_frame_header_types[i++] = H5T_STD_U32LE; /* uint32_t frame_number */
    oli_frame_header_types[i++] = H5T_STD_U8LE;  /* uint8_t time_error */
    oli_frame_header_types[i++] = array_id;
        /* uint8_t Reserved[LINE_RESERVED_LEN] */
    oli_frame_header_types[i++] = H5T_STD_U16LE; /* uint16_t frame_status */

    if (i != TABLE_SIZE_OLI_FRAME_HEADER)
    {
        IAS_LOG_ERROR(
            "The number of types %d does not match the number of columns %d"
            " for table %s",
            i, TABLE_SIZE_OLI_FRAME_HEADER,
            GROUP_NAME_OLI"/"TABLE_NAME_FRAMEHEADER);
        return NULL;
    }

    return oli_frame_header_types;
}

/******************************************************************************
 NAME: ias_l0r_tirs_frame_header_init

 PURPOSE: Initializes the HDF types for a frame header

 RETURNS: Pointer to the types array
******************************************************************************/
const hid_t* ias_l0r_tirs_frame_header_init(void)
{
    const int OFFSET_AND_FPE_ARRAY_SIZE = IAS_L0R_TIRS_NUM_BANDS *
        IAS_L0R_TIRS_NUM_SCAS * IAS_L0R_TIRS_NUM_ROWS;

    hid_t array_id_row_offset;
    hsize_t dims_row_offset[3] = {IAS_L0R_TIRS_NUM_BANDS, 
        IAS_L0R_TIRS_NUM_SCAS, IAS_L0R_TIRS_NUM_ROWS};
    hid_t array_id_d_header;
    hsize_t dims_d_header[2] = {3, IAS_L0R_TIRS_NUM_BANDS};
    hid_t array_id_fpe_words;
    hsize_t dims_fpe_words[2] = {7, OFFSET_AND_FPE_ARRAY_SIZE};

    array_id_row_offset = H5Tarray_create( H5T_STD_U8LE, 3, dims_row_offset);
    if (array_id_row_offset < 0)
    {
        IAS_LOG_ERROR("Call to H5Tarray_create failed");
        return NULL;
    }
    array_id_d_header = H5Tarray_create( H5T_STD_U16LE, 2, dims_d_header);
    if (array_id_d_header < 0)
    {
        IAS_LOG_ERROR("Call to H5Tarray_create failed");
        return NULL;
    }
    array_id_fpe_words = H5Tarray_create( H5T_STD_U16LE, 2, dims_fpe_words);
    if (array_id_fpe_words < 0)
    {
        IAS_LOG_ERROR("Call to H5Tarray_create failed");
        return NULL;
    }

    int i = 0;
    tirs_frame_header_types[i++] =
                H5T_STD_I32LE;/* uint32_t days_from_J2000; */
    tirs_frame_header_types[i++] = H5T_IEEE_F64LE; /*double Seconds*/
    tirs_frame_header_types[i++] = H5T_STD_I16LE; /* day */
    tirs_frame_header_types[i++] = H5T_STD_I32LE; /* milliseconds of day */
    tirs_frame_header_types[i++] = H5T_STD_I16LE; /* microsecond of ms */
    tirs_frame_header_types[i++] = H5T_STD_U32LE; /* frame_number */
    tirs_frame_header_types[i++] =  H5T_STD_U8LE; /* sync_byte */
    tirs_frame_header_types[i++] =  H5T_STD_U8LE; /* reserved */
    tirs_frame_header_types[i++] =  H5T_STD_U8LE; /* data_set_type */
    tirs_frame_header_types[i++] =  H5T_IEEE_F64LE; /* integration_duration */
    tirs_frame_header_types[i++] =  H5T_STD_U32LE; /* total_frame_requested */
    tirs_frame_header_types[i++] = array_id_row_offset;
    tirs_frame_header_types[i++] = array_id_d_header;
    tirs_frame_header_types[i++] = array_id_fpe_words;
    tirs_frame_header_types[i++] = H5T_STD_U8LE; /* roic_crc_status_blind */
    tirs_frame_header_types[i++] = H5T_STD_U8LE; /* roic_crc_status_10_8 */
    tirs_frame_header_types[i++] = H5T_STD_U8LE; /* roic_crc_status_12 */
    tirs_frame_header_types[i++] = H5T_STD_U16LE;/* uint16_t frame_status; */
    if (i != TABLE_SIZE_TIRS_FRAME_HEADER)
    {
        IAS_LOG_ERROR(
            "The number of types %d does not match the number of columns %d"
            " for table %s",
            i, TABLE_SIZE_TIRS_FRAME_HEADER,
            GROUP_NAME_TIRS"/"TABLE_NAME_FRAMEHEADER);
        return NULL;
    }

    return tirs_frame_header_types;
}

/*******************************************************************************
*Generic header routines
*******************************************************************************/
/******************************************************************************
 NAME: ias_l0r_header_init

 PURPOSE: performs all initializations needed for the library to access
          ancillary data stored in HDF files.

 RETURNS: SUCCESS- Header types populated
          ERROR- Unable to populate header types
******************************************************************************/

int ias_l0r_header_init(void)
{
    if (field_type_image_header == NULL)
    {
        field_type_image_header = ias_l0r_image_header_init();
        if (field_type_image_header == NULL)
            return ERROR;
    }
    if (field_type_frame_header == NULL)
    {
        field_type_frame_header = ias_l0r_oli_frame_header_init();
        if (field_type_frame_header == NULL)
            return ERROR;
    }
    if (field_type_tirs_frame_header == NULL)
    {
        field_type_tirs_frame_header = ias_l0r_tirs_frame_header_init();
        if (field_type_tirs_frame_header == NULL)
            return ERROR;
    }
    return SUCCESS;
}

/*******************************************************************************
*Frame header routines
*******************************************************************************/

/******************************************************************************
 NAME: ias_l0r_append_oli_frame_headers

 PURPOSE: appends frame headers to the frame header table

 RETURNS: SUCCESS- The data was successfully written
          ERROR- The data could not be written
******************************************************************************/
int ias_l0r_append_oli_frame_headers
(
    L0RIO *l0r, /* I: append the frame to the table in this file */
    const int num_frames_to_be_written, /* I: number of frames to write */
    const IAS_L0R_OLI_FRAME_HEADER *frame_header /* I: the frame
                                                header data to append */
)
{
    int status;

    herr_t hdf_status;
  
    if (l0r == NULL)
    {
        IAS_LOG_ERROR("Error NULL pointer received");
        return ERROR;
    }
    HDFIO *hdfio_ptr =  l0r;
    
    if (hdfio_ptr->access_mode_header != IAS_WRITE)
    {
        IAS_LOG_ERROR("Access to OLI header data is not in write mode");
        return ERROR;
    }

    /* make sure the file exists and is write mode */
    status = ias_l0r_establish_header_file(hdfio_ptr, TRUE);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Error establishing a file for frame header data");
        return ERROR;
    }

    /* establish the structure in the file to contain the data  */
    status = ias_l0r_hdf_establish_table(GROUP_NAME_OLI,
        TABLE_NAME_FRAMEHEADER, hdfio_ptr->file_id_header,
        &hdfio_ptr->table_id_oli_frame_header, TABLE_SIZE_OLI_FRAME_HEADER,
        IAS_L0R_OLI_FRAME_HEADER_SIZE, IAS_L0R_OLI_FRAME_HEADER_NAMES,
        IAS_FRAME_HEADER_OFFSET, field_type_frame_header);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Error establishing a table for frame header data");
        return ERROR;
    }

    hdf_status = H5TBappend_records(hdfio_ptr->file_id_header,
        GROUP_NAME_OLI"/"TABLE_NAME_FRAMEHEADER, num_frames_to_be_written,
        IAS_L0R_OLI_FRAME_HEADER_SIZE, IAS_FRAME_HEADER_OFFSET,
        IAS_L0R_OLI_FRAME_HEADER_SIZES, frame_header);
    if (hdf_status < 0)
    {
        IAS_LOG_ERROR("Error appending frame header data");
        return ERROR;
    }

    return SUCCESS;
}

/******************************************************************************
 NAME: ias_l0r_append_tirs_frame_headers

 PURPOSE:  appends frame headers to the frame header table

 RETURNS: SUCCESS- The data was successfully written
          ERROR- The data could not be written
******************************************************************************/
int ias_l0r_append_tirs_frame_headers
(
    L0RIO *l0r, /* I: append the frame to the table in this file */
    const int num_frames_to_be_written, /* I: number of frames to write */
    const IAS_L0R_TIRS_FRAME_HEADER *frame_header   /* I: the frame header
                                                       data to append */
)
{
    int status;
    HDFIO *hdfio_ptr =  l0r;

    /* make sure the file exists and is write mode */
    status = ias_l0r_establish_header_file(hdfio_ptr, TRUE);
    if (status == ERROR)
    {
        IAS_LOG_ERROR(
            "Error establishing a file for frame header data");
        return ERROR;
    }

    /* establish the structure in the file to contain the data */
    status = ias_l0r_hdf_establish_table(GROUP_NAME_TIRS,
        TABLE_NAME_FRAMEHEADER, hdfio_ptr->file_id_header,
        &hdfio_ptr->table_id_tirs_frame_header, TABLE_SIZE_TIRS_FRAME_HEADER,
        IAS_L0R_TIRS_FRAME_HEADER_SIZE, FIELD_NAMES_TIRS_LINE,
        IAS_L0R_TIRS_FRAME_HEADER_OFFSET, field_type_tirs_frame_header);
    if (status == ERROR)
    {
        IAS_LOG_ERROR(
            "Error establishing a table for frame header data");
        return ERROR;
    }

    /* write the records to the table */
    status = H5TBappend_records(hdfio_ptr->file_id_header,
        GROUP_NAME_TIRS"/"TABLE_NAME_FRAMEHEADER,
        num_frames_to_be_written, IAS_L0R_TIRS_FRAME_HEADER_SIZE,
        IAS_L0R_TIRS_FRAME_HEADER_OFFSET, IAS_L0R_TIRS_FRAME_HEADER_SIZES,
        frame_header);
    if (status < 0)
    {
        IAS_LOG_ERROR("Error appending to %s",
            GROUP_NAME_TIRS"/"TABLE_NAME_FRAMEHEADER);
        return ERROR;
    }
    return SUCCESS;
}

/*******************************************************************************
 NAME: ias_l0r_get_oli_frame_headers

 PURPOSE: Reads OLI frame headers starting at start record,
          reading 'num_records_to_read'
          
 RETURNS: SUCCESS- Frame headers read into buffer
          ERROR- Unable to read into buffer
*******************************************************************************/
int ias_l0r_get_oli_frame_headers
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    const int start_record, /* I: First record to read */
    const int num_records_to_read, /* I: Number to read */
    IAS_L0R_OLI_FRAME_HEADER *frame_header /* O: Data read */
)
{
    if (l0r == NULL)
    {
        IAS_LOG_ERROR("Error NULL pointer received");
        return ERROR;
    }

    return ias_l0r_get_headers(l0r,
        GROUP_NAME_OLI"/"TABLE_NAME_FRAMEHEADER,
        start_record, num_records_to_read, IAS_L0R_OLI_FRAME_HEADER_SIZE,
        IAS_FRAME_HEADER_OFFSET, IAS_L0R_OLI_FRAME_HEADER_SIZES, frame_header);
}

/*******************************************************************************
 NAME: ias_l0r_get_oli_frame_headers_records_count
 
 PURPOSE: Gets the number of entries for the frame header table
 
 RETURNS: SUCCESS- Size retrieved
          ERROR- Unable to determine size
******************************************************************************/
int ias_l0r_get_oli_frame_headers_records_count
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    int *size          /* O: number of records in the OLI frame header dataset*/
)
{
    if (l0r == NULL)
    {
        IAS_LOG_ERROR("Error NULL pointer received");
        return ERROR;
    }
    HDFIO *hdfio_ptr =  l0r;
    return ias_l0r_table_header_records_count(hdfio_ptr,
        GROUP_NAME_OLI"/"TABLE_NAME_FRAMEHEADER, size);
}

/*******************************************************************************
 NAME: ias_l0r_get_tirs_frame_headers

 PURPOSE: Reads TIRS frame headers starting at start record,
          reading 'num_records_to_read'
          
 RETURNS: SUCCESS- Frame headers read into buffer
          ERROR- Unable to read into buffer
*******************************************************************************/
int ias_l0r_get_tirs_frame_headers
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    const int start_record, /* I: First record to read */
    const int num_records_to_read, /* I: Number to read */
    IAS_L0R_TIRS_FRAME_HEADER *frame_header /* O: Data read */
)
{
    if (l0r == NULL)
    {
        IAS_LOG_ERROR(
            "Error NULL pointer received");
        return ERROR;
    }

    return ias_l0r_get_headers(l0r,
        GROUP_NAME_TIRS"/"TABLE_NAME_FRAMEHEADER,
        start_record, num_records_to_read, IAS_L0R_TIRS_FRAME_HEADER_SIZE,
        IAS_L0R_TIRS_FRAME_HEADER_OFFSET, IAS_L0R_TIRS_FRAME_HEADER_SIZES,
        frame_header);
}

/*******************************************************************************
 NAME: ias_l0r_get_tirs_frame_headers_records_count
 
 PURPOSE: Gets the number of entries for the frame header table
 
 RETURNS: SUCCESS- Size retrieved
          ERROR- Unable to determine size
******************************************************************************/
int ias_l0r_get_tirs_frame_headers_records_count
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    int *size          /* O: number of records in  the TIRS frame header
                             dataset*/
)
{
    if (l0r == NULL)
    {
        IAS_LOG_ERROR("Error NULL pointer received");
        return ERROR;
    }
    HDFIO *hdfio_ptr =  l0r;
    return ias_l0r_table_header_records_count(hdfio_ptr,
        GROUP_NAME_TIRS"/"TABLE_NAME_FRAMEHEADER, size);
}

/******************************************************************************
 NAME: ias_l0r_truncate_oli_frame_headers

 PURPOSE: truncates the OLI frame headers table to 'count'

 RETURNS: SUCCESS- The table was successfully truncated
          ERROR- The table could not be truncated 
******************************************************************************/
int ias_l0r_truncate_oli_frame_headers
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    const int count /* I: number of records after truncation */
)
{
    int status;

    HDFIO *hdfio_ptr =  l0r;

    IAS_LOG_WARNING("Calling truncate function. This function does not reclaim"
        " the space previously used. Depending on the purpose of this data,"
        " running h5repack on the file is advisable");

    if (hdfio_ptr->access_mode_header != IAS_WRITE)
    {
        IAS_LOG_ERROR("Access to OLI header data is not in write mode");
        return ERROR;
    }

    /* make sure the file exists and is in write mode */
    status = ias_l0r_establish_header_file(hdfio_ptr, TRUE);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Error establishing a file for frame header data");
        return ERROR;
    }

    /* establish the structure in the file to contain the data  */
    status = ias_l0r_hdf_establish_table(GROUP_NAME_OLI,
        TABLE_NAME_FRAMEHEADER, hdfio_ptr->file_id_header,
        &hdfio_ptr->table_id_oli_frame_header, TABLE_SIZE_OLI_FRAME_HEADER,
        IAS_L0R_OLI_FRAME_HEADER_SIZE, IAS_L0R_OLI_FRAME_HEADER_NAMES,
        IAS_FRAME_HEADER_OFFSET, field_type_frame_header);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Error establishing a table for frame header data");
        return ERROR;
    }

    return ias_l0r_truncate_headers(l0r,
        GROUP_NAME_OLI"/"TABLE_NAME_FRAMEHEADER, count );
}

/******************************************************************************
 NAME: ias_l0r_set_oli_frame_headers

 PURPOSE: writes the frame headers starting at start record, writing
          'num_records_to_write'

 RETURNS: SUCCESS- The data was successfully written
          ERROR- The data could not be written
******************************************************************************/
int ias_l0r_set_oli_frame_headers
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    const int index, /* I: first record to write */
    const int count, /* I: number to write */
    const IAS_L0R_OLI_FRAME_HEADER *buffer /* I: data to write */
)
{
    int status;

    HDFIO *hdfio_ptr =  l0r;

    if (hdfio_ptr->access_mode_header != IAS_WRITE)
    {
        IAS_LOG_ERROR("Access to OLI header data is not in write mode");
        return ERROR;
    }

    /* make sure the file exists and is in write mode */
    status = ias_l0r_establish_header_file(hdfio_ptr, TRUE);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Error establishing a file for frame header data");
        return ERROR;
    }

    /* establish the structure in the file to contain the data  */
    status = ias_l0r_hdf_establish_table(GROUP_NAME_OLI,
        TABLE_NAME_FRAMEHEADER, hdfio_ptr->file_id_header,
        &hdfio_ptr->table_id_oli_frame_header, TABLE_SIZE_OLI_FRAME_HEADER,
        IAS_L0R_OLI_FRAME_HEADER_SIZE, IAS_L0R_OLI_FRAME_HEADER_NAMES,
        IAS_FRAME_HEADER_OFFSET, field_type_frame_header);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Error establishing a table for frame header data");
        return ERROR;
    }

    return ias_l0r_set_headers(l0r,
        GROUP_NAME_OLI"/"TABLE_NAME_FRAMEHEADER, index, count,
        IAS_L0R_OLI_FRAME_HEADER_SIZE, IAS_FRAME_HEADER_OFFSET,
        IAS_L0R_OLI_FRAME_HEADER_SIZES, buffer);
}

/******************************************************************************
 NAME: ias_l0r_truncate_tirs_frame_headers

 PURPOSE: truncates the TIRS frame headers table to 'count'

 RETURNS: SUCCESS- The table was successfully truncated
          ERROR- The table could not be truncated 
******************************************************************************/
int ias_l0r_truncate_tirs_frame_headers
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    const int count /* I: number of records after truncation */
)
{
    int status;

    HDFIO *hdfio_ptr =  l0r;

    IAS_LOG_WARNING("Calling truncate function. This function does not reclaim"
        " the space previously used. Depending on the purpose of this data,"
        " running h5repack on the file is advisable");

    /* Make sure file is in write mode */
    if (hdfio_ptr->access_mode_header != IAS_WRITE)
    {
        IAS_LOG_ERROR("Access to TIRS header data is not in write mode");
        return ERROR;
    }

    /* make sure the file exists */
    status = ias_l0r_establish_header_file(hdfio_ptr, TRUE);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Error establishing a file for frame header data");
        return ERROR;
    }

    /* establish the structure in the file to contain the data  */
    status = ias_l0r_hdf_establish_table(GROUP_NAME_TIRS,
        TABLE_NAME_FRAMEHEADER, hdfio_ptr->file_id_header,
        &hdfio_ptr->table_id_tirs_frame_header, TABLE_SIZE_TIRS_FRAME_HEADER,
        IAS_L0R_TIRS_FRAME_HEADER_SIZE, FIELD_NAMES_TIRS_LINE,
        IAS_L0R_TIRS_FRAME_HEADER_OFFSET, field_type_tirs_frame_header);

    if (status == ERROR)
    {
        IAS_LOG_ERROR("Error establishing a table for frame header data");
        return ERROR;
    }

    return ias_l0r_truncate_headers(l0r,
        GROUP_NAME_TIRS"/"TABLE_NAME_FRAMEHEADER, count );
}

/******************************************************************************
 NAME: ias_l0r_set_tirs_frame_headers

 PURPOSE: writes the frame headers starting at start record, writing
          'num_records_to_write'

 RETURNS: SUCCESS- The data was successfully written
          ERROR- The data could not be written
******************************************************************************/
int ias_l0r_set_tirs_frame_headers
(
    L0RIO *l0r, /* I: structure for the file used in I/O */
    const int index, /* I: first record to write */
    const int count, /* I: number to write */
    const IAS_L0R_TIRS_FRAME_HEADER *buffer /* I: data to write */
)
{
    int status;

    HDFIO *hdfio_ptr =  l0r;

    /* Make sure file is in write mode */
    if (hdfio_ptr->access_mode_header != IAS_WRITE)
    {
        IAS_LOG_ERROR(
            "Access to TIRS header data is not in write mode");
        return ERROR;
    }

    /* make sure the file exists */
    status = ias_l0r_establish_header_file(hdfio_ptr, TRUE);
    if (status == ERROR)
    {
        IAS_LOG_ERROR(
            "Error establishing a file for frame header data");
        return ERROR;
    }

    /* establish the structure in the file to contain the data */
    status = ias_l0r_hdf_establish_table(GROUP_NAME_TIRS,
        TABLE_NAME_FRAMEHEADER, hdfio_ptr->file_id_header,
        &hdfio_ptr->table_id_tirs_frame_header, TABLE_SIZE_TIRS_FRAME_HEADER,
        IAS_L0R_TIRS_FRAME_HEADER_SIZE, FIELD_NAMES_TIRS_LINE,
        IAS_L0R_TIRS_FRAME_HEADER_OFFSET, field_type_tirs_frame_header);
    if (status == ERROR)
    {
        IAS_LOG_ERROR(
            "Error establishing %s/%s", GROUP_NAME_TIRS,
            TABLE_NAME_FRAMEHEADER);
        return ERROR;
    }

    return ias_l0r_set_headers(l0r,
        GROUP_NAME_TIRS"/"TABLE_NAME_FRAMEHEADER, index, count,
        IAS_L0R_TIRS_FRAME_HEADER_SIZE, IAS_L0R_TIRS_FRAME_HEADER_OFFSET,
        IAS_L0R_TIRS_FRAME_HEADER_SIZES, buffer);
}

/*******************************************************************************
 NAME: ias_l0r_close_header

 PURPOSE: Closes access to the header data
 
 RETURNS: SUCCESS- Header access closed
          ERROR- Unable to close access
*******************************************************************************/
int ias_l0r_close_header
(
    L0RIO *l0r /* I: structure for the file used in I/O */
)
{
    int error_count = 0;
    herr_t hdf_status;

    if (l0r == NULL)
    {
        IAS_LOG_ERROR("NULL pointer received");
        return ERROR;
    }

    HDFIO *hdfio_ptr = l0r;

    hdfio_ptr->access_mode_header = -1;

    /* Close dataset resources before closing the file */
    if (hdfio_ptr->table_id_oli_frame_header > 0)
    {
        hdf_status = H5Dclose(hdfio_ptr->table_id_oli_frame_header);
        if (hdf_status < 0)
        {
            IAS_LOG_ERROR(
                "Error closing HDF OLI frame header resource");
            error_count++;
        }
        hdfio_ptr->table_id_oli_frame_header = -1;
    }
    if (hdfio_ptr->table_id_oli_image_header > 0)
    {
        hdf_status = H5Dclose(hdfio_ptr->table_id_oli_image_header);
        if (hdf_status < 0)
        {
            IAS_LOG_ERROR(
                "Error closing HDF OLI image header resource");
            error_count++;
        }
        hdfio_ptr->table_id_oli_image_header = -1;
    }
    if (hdfio_ptr->table_id_tirs_frame_header > 0)
    {
        hdf_status = H5Dclose(hdfio_ptr->table_id_tirs_frame_header);
        if (hdf_status < 0)
        {
            IAS_LOG_ERROR(
                "Error closing HDF TIRS frame header resource");
            error_count++;
        }
        hdfio_ptr->table_id_tirs_frame_header = -1;
    }

    hdfio_ptr->access_mode_header = -1;

    /* The header data shares the ancillary file. If the file is in use
    *do not close access to it.  Instead just flush the data. */
    if (hdfio_ptr->access_mode_ancillary < 0)
    {
        if (hdfio_ptr->file_id_header > 0)
        {
            hdf_status = H5Fclose(hdfio_ptr->file_id_header);
            if (hdf_status < 0)
            {
                IAS_LOG_ERROR(
                    "Error closing hdf resources, file_id_header %d",
                    hdfio_ptr->file_id_header);
                return ERROR;
            }
            hdfio_ptr->file_id_header = -1;
        }
    }
    else
    {
        if (hdfio_ptr->file_id_header > 0)
        {
            hdf_status = H5Fflush(hdfio_ptr->file_id_header, H5F_SCOPE_LOCAL);
            if (hdf_status < 0)
            {
                IAS_LOG_ERROR("Error flushing hdf resources, "
                    "file_id_header %d",
                    hdfio_ptr->file_id_header);
                return ERROR;
            }
            hdfio_ptr->file_id_header = -1;
        }
    }

    if (error_count > 0)
    {
        return ERROR;
    }

    return SUCCESS;
}
                                                                     
/*******************************************************************************
 NAME: ias_l0r_open_header
 
 PURPOSE: Opens access to the header data
 
 RETURNS: SUCCESS- Header access opened
          ERROR- Unable to open access
*******************************************************************************/
int ias_l0r_open_header
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    const IAS_ACCESS_MODE file_access_mode /* I: Mode to open file in */
)
{
    if (l0r == NULL)
    {
        IAS_LOG_ERROR("NULL pointer received");
        return ERROR;
    }
    HDFIO *hdfio_ptr = l0r;

    if (hdfio_ptr->access_mode_header != -1)
    {
        IAS_LOG_ERROR("File access mode already set");
        return ERROR;
    }

    hdfio_ptr->access_mode_header = file_access_mode;
    return SUCCESS;
}

/*******************************************************************************
 NAME: ias_l0r_get_oli_image_header
 
 PURPOSE: Reads the single image header record

 RETURNS: SUCCESS- Record retrieved
          ERROR- Unable to retrieve record
*******************************************************************************/
int ias_l0r_get_oli_image_header
(
    L0RIO *l0r, /* I: structure used with the L0R data  */
    IAS_L0R_OLI_IMAGE_HEADER *image_header /* O: Data read */
)
{
    if (l0r == NULL)
    {
        IAS_LOG_ERROR("NULL pointer received");
        return ERROR;
    }

    return ias_l0r_get_headers(l0r,
        GROUP_NAME_OLI"/"TABLE_NAME_IMAGEHEADER, 0, 1,
        IAS_L0R_OLI_IMAGE_HEADER_SIZE, IAS_IMAGE_HEADER_OFFSET,
        IAS_L0R_OLI_IMAGE_HEADER_SIZES, image_header);
}

/*******************************************************************************
 NAME: ias_l0r_is_oli_image_header_present
 
 PURPOSE: Determines if a image header is present in the dataset. Size will
          be 1 if the header is present and 0 if it is not.
 
 RETURNS: SUCCESS- Size retrieved
          ERROR- Unable to determine size
******************************************************************************/
int ias_l0r_is_oli_image_header_present
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    int *size          /* O: number of records in the frame header dataset*/
)
{
    if (l0r == NULL)
    {
        IAS_LOG_ERROR("NULL pointer received");
        return ERROR;
    }
    HDFIO *hdfio_ptr =  l0r;

    return ias_l0r_table_header_records_count(hdfio_ptr,
        GROUP_NAME_OLI"/"TABLE_NAME_IMAGEHEADER, size);
}

/******************************************************************************
 NAME: ias_l0r_set_oli_image_header

 PURPOSE: writes the single image header record

 RETURNS: SUCCESS- The data was successfully written
          ERROR- The data could not be written
******************************************************************************/
int ias_l0r_set_oli_image_header
(
    L0RIO *l0r, /* I: structure used with the L0R data */
    const IAS_L0R_OLI_IMAGE_HEADER *image_header /* I: frame header data
                                                     to append */
)
{
    int status;
    HDFIO *hdfio_ptr =  l0r;

    /* Make sure file is in write mode */
    if (hdfio_ptr->access_mode_header != IAS_WRITE)
    {
        IAS_LOG_ERROR("Access to header data is not in write mode");
        return ERROR;
    }

    /* make sure the file exists */
    status = ias_l0r_establish_header_file(hdfio_ptr, TRUE);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Error establishing a file for image header data");
        return ERROR;
    }

    /* establish the structure in the file to contain the data  */
    status = ias_l0r_hdf_establish_table(GROUP_NAME_OLI,
        TABLE_NAME_IMAGEHEADER, hdfio_ptr->file_id_header,
        &hdfio_ptr->table_id_oli_image_header, TABLE_SIZE_OLI_IMAGE_HEADER,
        IAS_L0R_OLI_IMAGE_HEADER_SIZE, IAS_FRAME_FIELD_NAMES,
        IAS_IMAGE_HEADER_OFFSET, field_type_image_header);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Error establishing a table for image header data");
        return ERROR;
    }

    return ias_l0r_set_headers(l0r,
        GROUP_NAME_OLI"/"TABLE_NAME_IMAGEHEADER, 0, 1,
        IAS_L0R_OLI_IMAGE_HEADER_SIZE, IAS_IMAGE_HEADER_OFFSET,
        IAS_L0R_OLI_IMAGE_HEADER_SIZES,
        image_header);
}

