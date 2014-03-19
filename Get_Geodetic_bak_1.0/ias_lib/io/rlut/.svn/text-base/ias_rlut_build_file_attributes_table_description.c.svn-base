/*************************************************************************
 NAME:     ias_rlut_build_file_attributes_table_description

 PURPOSE:  Builds the table description for the RLUT file attributes
           information

 RETURNS:  Integer status code of SUCCESS or ERROR
***************************************************************************/
#include "hdf5.h"
#include "hdf5_hl.h"
#include "ias_logging.h"
#include "ias_rlut.h"
#include "ias_rlut_private.h"
#include "ias_const.h"


int ias_rlut_build_file_attributes_table_description
(
    size_t offsets[IAS_RLUT_ATTR_NFIELDS],           /* O: Offsets into data
                                                        structure */
    const char *field_names[IAS_RLUT_ATTR_NFIELDS],  /* O: Field names */
    hid_t field_types[IAS_RLUT_ATTR_NFIELDS],        /* O: Field data types */
    hid_t fields_to_close[IAS_RLUT_ATTR_NFIELDS],    /* O: List of fields to
                                                        be closed */
    size_t field_sizes[IAS_RLUT_ATTR_NFIELDS]        /* O: Field sizes  */
)
{
    IAS_RLUT_FILE_ATTRIBUTES file_attr;     /* Data structure reference */
    int field_count;                        /* Generic field counter */
    int return_status = SUCCESS;


    /* Initialize */
    for (field_count = 0; field_count < IAS_RLUT_ATTR_NFIELDS; field_count++)
    {
        fields_to_close[field_count] = -1;
    }

    /* Define the table, starting with the file source attribute  */
    field_count = 0;
    offsets[field_count] = HOFFSET(IAS_RLUT_FILE_ATTRIBUTES, file_source);
    field_names[field_count] = "File Source";
    field_types[field_count] = H5Tcopy(H5T_C_S1);
    field_sizes[field_count] = sizeof(file_attr.file_source);
    if (H5Tset_size(field_types[field_count], field_sizes[field_count]) < 0)
    {
        IAS_LOG_ERROR("Establishing file source attribute size in table--"
            "%d bytes is incorrect", (int)field_sizes[field_count]);
        return_status = ERROR;
    }
    fields_to_close[field_count] = field_types[field_count];

    /* Effective RLUT begin date */
    offsets[++field_count] = HOFFSET(IAS_RLUT_FILE_ATTRIBUTES,
        effective_date_begin);
    field_names[field_count] = "Effective Begin Date";
    field_types[field_count] = H5Tcopy(H5T_C_S1);
    field_sizes[field_count] = sizeof(file_attr.effective_date_begin);
    if (H5Tset_size(field_types[field_count], field_sizes[field_count]) < 0)
    {
        IAS_LOG_ERROR("Establishing begin date attribute size in table--"
            "%d bytes is incorrect", (int)field_sizes[field_count]);
        return_status = ERROR;
    }
    fields_to_close[field_count] = field_types[field_count];

    /* Effective RLUT end date  */
    offsets[++field_count] = HOFFSET(IAS_RLUT_FILE_ATTRIBUTES,
        effective_date_end);
    field_names[field_count] = "Effective End Date";
    field_types[field_count] = H5Tcopy(H5T_C_S1);
    field_sizes[field_count] = sizeof(file_attr.effective_date_end);
    if (H5Tset_size(field_types[field_count], field_sizes[field_count]) < 0)
    {
        IAS_LOG_ERROR("Establishing end date attribute size in table--"
            "%d bytes is incorrect", (int)field_sizes[field_count]);
        return_status = ERROR;
    }
    fields_to_close[field_count] = field_types[field_count];

    /* RLUT status */
    offsets[++field_count] = HOFFSET(IAS_RLUT_FILE_ATTRIBUTES, status);
    field_names[field_count] = "Effective Status";
    field_types[field_count] = H5Tcopy(H5T_C_S1);
    field_sizes[field_count] = sizeof(file_attr.status);
    if (H5Tset_size(field_types[field_count], field_sizes[field_count]) < 0)
    {
        IAS_LOG_ERROR("Establishing status attribute size in table--"
            "%d bytes is incorrect", (int)field_sizes[field_count]);
        return_status = ERROR;
    }
    fields_to_close[field_count] = field_types[field_count];

    /* RLUT baseline date */
    offsets[++field_count] = HOFFSET(IAS_RLUT_FILE_ATTRIBUTES, baseline_date);
    field_names[field_count] = "Baseline Date";
    field_types[field_count] = H5Tcopy(H5T_C_S1);
    field_sizes[field_count] = sizeof(file_attr.baseline_date);
    if (H5Tset_size(field_types[field_count], field_sizes[field_count]) < 0)
    {
        IAS_LOG_ERROR("Establishing baseline date size in table--"
            "%d bytes is incorrect", (int)field_sizes[field_count]);
        return_status = ERROR;
    }
    fields_to_close[field_count] = field_types[field_count];

    /* RLUT description */
    offsets[++field_count] = HOFFSET(IAS_RLUT_FILE_ATTRIBUTES, description);
    field_names[field_count] = "Description";
    field_types[field_count] = H5Tcopy(H5T_C_S1);
    field_sizes[field_count] = sizeof(file_attr.description);
    if (H5Tset_size(field_types[field_count], field_sizes[field_count]) < 0)
    {
        IAS_LOG_ERROR("Establishing description attribute size in table--"
            "%d bytes is incorrect", (int)field_sizes[field_count]);
        return_status = ERROR;
    }
    fields_to_close[field_count] = field_types[field_count];

    /* RLUT version */
    offsets[++field_count] = HOFFSET(IAS_RLUT_FILE_ATTRIBUTES, version);
    field_names[field_count] = "File Version";
    field_types[field_count] = H5T_NATIVE_INT;
    field_sizes[field_count] = sizeof(file_attr.version);

    /* Increment the final field count */
    field_count++;

    /* Check for errors */
    if (field_count < IAS_RLUT_ATTR_NFIELDS)
    {
        IAS_LOG_ERROR("Too few fields found");
        ias_rlut_cleanup_table_description(fields_to_close,
            IAS_RLUT_ATTR_NFIELDS);
        return_status = ERROR;
    }
    else if (field_count > IAS_RLUT_ATTR_NFIELDS)
    {
        IAS_LOG_ERROR("Too many fields found -- stack probably corrupted");
        ias_rlut_cleanup_table_description(fields_to_close,
            IAS_RLUT_ATTR_NFIELDS);
        return_status = ERROR;
    }

    return return_status;
}   /* END ias_rlut_build_file_attributes_table_description */
