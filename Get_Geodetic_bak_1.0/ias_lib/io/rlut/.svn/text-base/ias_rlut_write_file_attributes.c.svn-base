/***************************************************************************
 NAME:     ias_rlut_write_file_attributes

 PURPOSE:  Writes RLUT file attribute information to the FILE_ATTRIBUTES
           group in the RLUT file

 RETURNS:  Integer status code of SUCCESS or ERROR
****************************************************************************/
#include "hdf5.h"
#include "hdf5_hl.h"
#include "ias_logging.h"
#include "ias_rlut.h"
#include "ias_rlut2.h"
#include "ias_rlut_private.h"
#include "ias_const.h"


/***************************************************************************
 NAME:     write_file_attributes

 PURPOSE:  Common routine that writes FILE_ATTRIBUTES information to the
           FILE_ATTRIBUTES group in an RLUT or RLUT2 file

 RETURNS:  Integer status code of SUCCESS or ERROR

****************************************************************************/
static int write_file_attributes
(
    hid_t file_id,                         /* I: HDF file id */
    const IAS_RLUT_FILE_ATTRIBUTES *file_attr  /* I: File attributes
                                                  information to write */
)
{
    const char *field_names[IAS_RLUT_ATTR_NFIELDS];
                                           /* Name of each file attribute
                                              parameter */
    size_t offsets[IAS_RLUT_ATTR_NFIELDS]; /* Data offsets in
                                              IAS_RLUT_FILE_ATTRIBUTES data
                                              structure for each field*/
    size_t field_sizes[IAS_RLUT_ATTR_NFIELDS];
                                           /* Size of each field */
    hid_t field_types[IAS_RLUT_ATTR_NFIELDS];
                                           /* Data type for each field */
    hid_t fields_to_close[IAS_RLUT_ATTR_NFIELDS];
                                           /* Flags indicating open
                                              fields needing to be closed */
    hid_t file_attr_group;                 /* Root file attributes group ID */
    hsize_t type_size;                     /* Size of base
                                              IAS_RLUT_FILE_ATTRIBUTES
                                              data structure */
    herr_t hdf_status;                     /* HDF5 error status */
    int status;                            /* IAS status flags */
    int return_status = SUCCESS;


    /* Open the "FILE_ATTRIBUTES" group */
    file_attr_group = H5Gopen(file_id, FILE_ATTRIBUTES_GROUP_NAME,
        H5P_DEFAULT);
    if (file_attr_group < 0)
    {
        IAS_LOG_ERROR("Opening root file attributes group");
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

    /* Get the size of the FILE_ATTRIBUTES data structure */
    type_size = sizeof(*file_attr);

    /* Write the file attributes parameter set */
    hdf_status = H5TBmake_table(FILE_ATTRIBUTES_TABLE_NAME, file_attr_group,
        FILE_ATTRIBUTES_DATASET_NAME, IAS_RLUT_ATTR_NFIELDS, 1, type_size,
        field_names, offsets, field_types, sizeof(IAS_RLUT_FILE_ATTRIBUTES),
        NULL, 0, file_attr);

    /* Close the table fields */
    ias_rlut_cleanup_table_description(fields_to_close,
        IAS_RLUT_ATTR_NFIELDS);

    /* Check the return status of the write */
    if (hdf_status < 0)
    {
        IAS_LOG_ERROR("Writing file attributes data");
        return_status = ERROR;
    }

    /* Close the group */
    hdf_status = H5Gclose(file_attr_group);
    if (hdf_status < 0)
    {
        IAS_LOG_ERROR("Closing root file attributes group");
        return_status = ERROR;
    }

    return return_status;
}   /* END ias_rlut_write_file_attributes */

/***************************************************************************
 NAME:     ias_rlut_write_file_attributes

 PURPOSE:  Writes FILE_ATTRIBUTES information to the FILE_ATTRIBUTES
           group in an RLUT file

 RETURNS:  Integer status code of SUCCESS or ERROR

****************************************************************************/
int ias_rlut_write_file_attributes
(
    const IAS_RLUT_IO *rlut_file,              /* I: Open HDF5 RLUT file */
    const IAS_RLUT_FILE_ATTRIBUTES *file_attr  /* I: File attributes
                                                  information to write */
)
{
    /* Make sure the RLUT file is actually open */
    if ((rlut_file == NULL) || (rlut_file->file_id < 0))
    {
        IAS_LOG_ERROR("NULL pointer to IAS_RLUT_IO data block, or no RLUT "
            "file has been opened");
        return ERROR;
    }

    if (write_file_attributes(rlut_file->file_id, file_attr) != SUCCESS)
    {
        IAS_LOG_ERROR("Writing file attributes information to RLUT file "
            "%s", rlut_file->filename);
        return ERROR;
    }

    return SUCCESS;
}

/***************************************************************************
 NAME:     ias_rlut2_write_file_attributes

 PURPOSE:  Writes FILE_ATTRIBUTES information to the FILE_ATTRIBUTES
           group in an RLUT2 file

 RETURNS:  Integer status code of SUCCESS or ERROR

****************************************************************************/
int ias_rlut2_write_file_attributes
(
    const IAS_RLUT2_IO *rlut_file,             /* I: Open HDF5 RLUT file */
    const IAS_RLUT_FILE_ATTRIBUTES *file_attr  /* I: File attributes
                                                  information to write */
)
{
    /* Make sure the RLUT file is actually open */
    if ((rlut_file == NULL) || (rlut_file->file_id < 0))
    {
        IAS_LOG_ERROR("NULL pointer to IAS_RLUT_IO data block, or no RLUT "
            "file has been opened");
        return ERROR;
    }

    if (write_file_attributes(rlut_file->file_id, file_attr) != SUCCESS)
    {
        IAS_LOG_ERROR("Writing file attributes information to RLUT file "
            "%s", rlut_file->filename);
        return ERROR;
    }

    return SUCCESS;
}

