/***************************************************************************
 NAME:     ias_rlut_read_file_attributes

 PURPOSE:  Reads FILE_ATTRIBUTES information from the FILE_ATTRIBUTES
           group in the RLUT file

 RETURNS:  Integer status code of SUCCESS or ERROR

 NOTES:    If the calling routine dynamically allocated a
           FILE_ATTRIBUTES data block, it is responsible for freeing
           the block when no longer needed
****************************************************************************/
#include <stdlib.h>
#include "hdf5.h"
#include "hdf5_hl.h"
#include "ias_logging.h"
#include "ias_rlut.h"
#include "ias_rlut2.h"
#include "ias_rlut_private.h"
#include "ias_const.h"


/***************************************************************************
 NAME:     read_file_attributes

 PURPOSE:  Common routine that reads FILE_ATTRIBUTES information from the
           FILE_ATTRIBUTES group in an RLUT or RLUT2 file

 RETURNS:  Integer status code of SUCCESS or ERROR

****************************************************************************/
static int read_file_attributes
(
    hid_t file_id,                         /* I: HDF file id */
    IAS_RLUT_FILE_ATTRIBUTES *attr         /* O: Attributes to read */
)
{
    /* Linearization parameter table description arrays */
    const char *field_names[IAS_RLUT_ATTR_NFIELDS];
                                                /* Name of each attribute
                                                   parameter */
    size_t offsets[IAS_RLUT_ATTR_NFIELDS];      /* Data offsets in
                                                   FILE_ATTRIBUTES
                                                   data structure for each
                                                   field*/
    size_t field_sizes[IAS_RLUT_ATTR_NFIELDS];  /* Size of each field */
    hid_t field_types[IAS_RLUT_ATTR_NFIELDS];   /* Data type for each field */
    hid_t fields_to_close[IAS_RLUT_ATTR_NFIELDS];
                                                /* Flags indicating open
                                                   fields needing to be
                                                   closed */
    hid_t file_attr_group;                      /* Root file attributes group
                                                   ID */
    hsize_t type_size;                          /* Size of base
                                                   FILE_ATTRIBUTEs data
                                                   structure */
    hsize_t nfields = 0;                        /* Number of fields in the
                                                   table description */
    hsize_t nrecords = 0;                       /* Number of records in the
                                                   table (should be 1) */
    herr_t hdf_status;                          /* HDF5 error status */
    int status;                                 /* IAS status flags */
    int return_status = SUCCESS;


    /* Open the "FILE_ATTRIBUTES" group */
    file_attr_group = H5Gopen(file_id, FILE_ATTRIBUTES_GROUP_NAME,
        H5P_DEFAULT);
    if (file_attr_group < 0)
    {
        IAS_LOG_ERROR("Opening existing root file attributes group");
        return ERROR;
    }

    /* Build the FILE_ATTRIBUTES table definition */
    status = ias_rlut_build_file_attributes_table_description(offsets,
        field_names, field_types, fields_to_close, field_sizes);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Building file attributes table definition");
        H5Gclose(file_attr_group);
        return ERROR;
    }

    /* Get the number of fields and records */
    hdf_status = H5TBget_table_info(file_attr_group,
       FILE_ATTRIBUTES_DATASET_NAME, &nfields, &nrecords);
    if (hdf_status < 0)
    {
        IAS_LOG_ERROR("Getting attribute table information");
        ias_rlut_cleanup_table_description(fields_to_close,
            IAS_RLUT_ATTR_NFIELDS);
        H5Gclose(file_attr_group);
        return ERROR;
    }
    else if (nfields != IAS_RLUT_ATTR_NFIELDS)
    {
        IAS_LOG_ERROR("Number of defined fields %d not equal to number "
            "of returned fields %d", IAS_RLUT_ATTR_NFIELDS, (int)nfields);
        ias_rlut_cleanup_table_description(fields_to_close,
            IAS_RLUT_ATTR_NFIELDS);
        H5Gclose(file_attr_group);
        return ERROR;
    }
    else if (nrecords != 1)
    {
        IAS_LOG_ERROR("Attribute table should have only one record,"
            "records, found %d records instead", (int)nrecords);
        ias_rlut_cleanup_table_description(fields_to_close,
            IAS_RLUT_ATTR_NFIELDS);
        H5Gclose(file_attr_group);
        return ERROR;
    }

    /* Try to read the FILE_ATTRIBUTES data */
    type_size = sizeof(*attr);
    hdf_status = H5TBread_table(file_attr_group, FILE_ATTRIBUTES_DATASET_NAME,
        type_size, offsets, field_sizes, attr);

    /* Clean up the table definition */
    ias_rlut_cleanup_table_description(fields_to_close, IAS_RLUT_ATTR_NFIELDS);

    /* Check the read status */
    if (hdf_status < 0)
    {
        IAS_LOG_ERROR("Reading file attributes information");
        H5Gclose(file_attr_group);
        return_status = ERROR;
    }

    /* Close the FILE_ATTRIBUTES group */
    hdf_status = H5Gclose(file_attr_group);
    if (hdf_status < 0)
    {
        IAS_LOG_ERROR("Closing file attributes root group");
        return_status = ERROR;
    }

    return return_status;
}   /* END ias_rlut_read_file_attributes */

/***************************************************************************
 NAME:     ias_rlut_read_file_attributes

 PURPOSE:  Reads FILE_ATTRIBUTES information from the FILE_ATTRIBUTES
           group in an RLUT file

 RETURNS:  Integer status code of SUCCESS or ERROR

****************************************************************************/
int ias_rlut_read_file_attributes
(
    const IAS_RLUT_IO *rlut_file,          /* I: Open HDF5 RLUT file */
    IAS_RLUT_FILE_ATTRIBUTES *attr         /* O: Attributes to read */
)
{
    /* Make sure an RLUT file is actually open */
    if ((rlut_file == NULL) || (rlut_file->file_id < 0))
    {
        IAS_LOG_ERROR("NULL pointer to IAS_RLUT_IO data block, or no RLUT "
            "file has been opened");
        return ERROR;
    }

    if (read_file_attributes(rlut_file->file_id, attr) != SUCCESS)
    {
        IAS_LOG_ERROR("Reading file attributes information from RLUT file "
            "%s", rlut_file->filename);
        return ERROR;
    }

    return SUCCESS;
}

/***************************************************************************
 NAME:     ias_rlut2_read_file_attributes

 PURPOSE:  Reads FILE_ATTRIBUTES information from the FILE_ATTRIBUTES
           group in an RLUT2 file

 RETURNS:  Integer status code of SUCCESS or ERROR

****************************************************************************/
int ias_rlut2_read_file_attributes
(
    const IAS_RLUT2_IO *rlut_file,         /* I: Open HDF5 RLUT file */
    IAS_RLUT_FILE_ATTRIBUTES *attr         /* O: Attributes to read */
)
{
    /* Make sure an RLUT file is actually open */
    if ((rlut_file == NULL) || (rlut_file->file_id < 0))
    {
        IAS_LOG_ERROR("NULL pointer to IAS_RLUT2_IO data block, or no RLUT "
            "file has been opened");
        return ERROR;
    }

    if (read_file_attributes(rlut_file->file_id, attr) != SUCCESS)
    {
        IAS_LOG_ERROR("Reading file attributes information from alternate "
            "RLUT file %s", rlut_file->filename);
        return ERROR;
    }

    return SUCCESS;
}
