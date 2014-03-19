/*******************************************************************************
Name: ias_l1g_set_file_metadata
      ias_l1g_get_file_metadata

Purpose: Provide routines to read and write L1G file level metadata

*******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include "hdf5.h"
#include "hdf5_hl.h"        /* for H5TB functions */
#include "ias_types.h"      /* IAS_READ definition */
#include "ias_const.h"      /* SUCCESS/ERROR definition */
#include "ias_logging.h"
#include "ias_l1g.h"
#include "ias_l1g_private.h"

#define NFIELDS 21 
/* fields in the file metadata information structure */

/******************************************************************************
Name: cleanup_table_description

Purpose: Cleans up the resources allocated to the table description

Returns:
    nothing
******************************************************************************/
static void cleanup_table_description
(
    hid_t fields_to_close[NFIELDS]  /* I: array of fields to close */
)
{
    int i;

    /* close any types created for the table description */
    for (i = 0; i < NFIELDS; i++)
    {
        if (fields_to_close[i] >= 0)
            H5Tclose(fields_to_close[i]);
    }
}

/******************************************************************************
Name: build_table_description

Purpose: Builds the table description for the file metadata table.

Returns:
    SUCCESS or ERROR
******************************************************************************/
static int build_table_description
(
    size_t offsets[NFIELDS],         /* O: array of offsets in the structure */
    const char *field_names[NFIELDS],/* O: array of pointers to field names */
    hid_t field_type[NFIELDS],       /* O: array of field types */
    hid_t fields_to_close[NFIELDS],  /* O: array of fields to close */
    size_t field_size[NFIELDS]       /* O: array of field sizes */
)
{
    int i;
    int error = 0;
    IAS_L1G_FILE_METADATA md; /* for field sizes */
    hsize_t parm_dims[1] = {sizeof(md.projection_parameters)
                            / sizeof(md.projection_parameters[0])};

    /* initialize the list of fields to close */
    for (i = 0; i < NFIELDS; i++)
        fields_to_close[i] = -1;

    /* define the table contents.  For each field in the table, define the
       offset to the data in the structure, the field name, data type, and 
       field size.  For fields that need to be properly sized, H5Tset_size
       is called with the correct size and the created type is added to the
       list of fields that need to be closed. */
    i = 0;
    offsets[i] = HOFFSET(IAS_L1G_FILE_METADATA, projection_code);
    field_names[i] = "Projection Code";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(md.projection_code);

    offsets[++i] = HOFFSET(IAS_L1G_FILE_METADATA, zone_code);
    field_names[i] = "Zone";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(md.zone_code);

    offsets[++i] = HOFFSET(IAS_L1G_FILE_METADATA, datum);
    field_names[i] = "Datum";
    field_type[i] = H5Tcopy(H5T_C_S1);
    if (H5Tset_size(field_type[i], sizeof(md.datum)) < 0)
    {
        IAS_LOG_ERROR("Setting size of Datum field");
        error = 1;
    }
    fields_to_close[i] = field_type[i];
    field_size[i] = sizeof(md.datum);

    offsets[++i] = HOFFSET(IAS_L1G_FILE_METADATA, spheroid_code);
    field_names[i] = "Spheroid";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(md.spheroid_code);

    offsets[++i] = HOFFSET(IAS_L1G_FILE_METADATA, projection_units);
    field_names[i] = "Projection Units";
    field_type[i] = H5Tcopy(H5T_C_S1);
    if (H5Tset_size(field_type[i], sizeof(md.projection_units)) < 0)
    {
        IAS_LOG_ERROR("Setting size of Projection Units field");
        error = 1;
    }
    fields_to_close[i] = field_type[i];
    field_size[i] = sizeof(md.projection_units);

    offsets[++i] = HOFFSET(IAS_L1G_FILE_METADATA, projection_parameters);
    field_names[i] = "Projection Parameters";
    field_type[i] = H5Tarray_create(H5T_NATIVE_DOUBLE, 1, parm_dims);
    if (field_type[i] < 0)
    {
        IAS_LOG_ERROR("Setting size of Projection Parameters field");
        error = 1;
    }
    fields_to_close[i] = field_type[i];
    field_size[i] = sizeof(md.projection_parameters);

    offsets[++i] = HOFFSET(IAS_L1G_FILE_METADATA, wrs_path);
    field_names[i] = "WRS Path";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(md.wrs_path);

    offsets[++i] = HOFFSET(IAS_L1G_FILE_METADATA, wrs_row);
    field_names[i] = "WRS Row";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(md.wrs_row);

    offsets[++i] = HOFFSET(IAS_L1G_FILE_METADATA, roll_angle);
    field_names[i] = "Roll Angle";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_size[i] = sizeof(md.roll_angle);

    offsets[++i] = HOFFSET(IAS_L1G_FILE_METADATA, spacecraft);
    field_names[i] = "Spacecraft";
    field_type[i] = H5Tcopy(H5T_C_S1);
    if (H5Tset_size(field_type[i], sizeof(md.spacecraft)) < 0)
    {
        IAS_LOG_ERROR("Setting size of Spacecraft field");
        error = 1;
    }
    fields_to_close[i] = field_type[i];
    field_size[i] = sizeof(md.spacecraft);

    offsets[++i] = HOFFSET(IAS_L1G_FILE_METADATA, collection_type);
    field_names[i] = "Collection Type";
    field_type[i] = H5Tcopy(H5T_C_S1);
    if (H5Tset_size(field_type[i], sizeof(md.collection_type)) < 0)
    {
        IAS_LOG_ERROR("Setting size of Collection Type field");
        error = 1;
    }
    fields_to_close[i] = field_type[i];
    field_size[i] = sizeof(md.collection_type);

    offsets[++i] = HOFFSET(IAS_L1G_FILE_METADATA, capture_direction);
    field_names[i] = "Capture Direction";
    field_type[i] = H5Tcopy(H5T_C_S1);
    if (H5Tset_size(field_type[i], sizeof(md.capture_direction)) < 0)
    {
        IAS_LOG_ERROR("Setting size of Capture Direction field");
        error = 1;
    }
    fields_to_close[i] = field_type[i];
    field_size[i] = sizeof(md.capture_direction);

    offsets[++i] = HOFFSET(IAS_L1G_FILE_METADATA, capture_date);
    field_names[i] = "Capture Date";
    field_type[i] = H5Tcopy(H5T_C_S1);
    if (H5Tset_size(field_type[i], sizeof(md.capture_date)) < 0)
    {
        IAS_LOG_ERROR("Setting size of Capture Date field");
        error = 1;
    }
    fields_to_close[i] = field_type[i];
    field_size[i] = sizeof(md.capture_date);

    offsets[++i] = HOFFSET(IAS_L1G_FILE_METADATA, capture_time);
    field_names[i] = "Capture Time";
    field_type[i] = H5Tcopy(H5T_C_S1);
    if (H5Tset_size(field_type[i], sizeof(md.capture_time)) < 0)
    {
        IAS_LOG_ERROR("Setting size of Capture Time field");
        error = 1;
    }
    fields_to_close[i] = field_type[i];
    field_size[i] = sizeof(md.capture_time);

    offsets[++i] = HOFFSET(IAS_L1G_FILE_METADATA, correction_type);
    field_names[i] = "Correction Type";
    field_type[i] = H5Tcopy(H5T_C_S1);
    if (H5Tset_size(field_type[i], sizeof(md.correction_type)) < 0)
    {
        IAS_LOG_ERROR("Setting size of Correction Type field");
        error = 1;
    }
    fields_to_close[i] = field_type[i];
    field_size[i] = sizeof(md.correction_type);

    offsets[++i] = HOFFSET(IAS_L1G_FILE_METADATA, resample_type);
    field_names[i] = "Resample Method";
    field_type[i] = H5Tcopy(H5T_C_S1);
    if (H5Tset_size(field_type[i], sizeof(md.resample_type)) < 0)
    {
        IAS_LOG_ERROR("Setting size of Resample Method field");
        error = 1;
    }
    fields_to_close[i] = field_type[i];
    field_size[i] = sizeof(md.resample_type);

    offsets[++i] = HOFFSET(IAS_L1G_FILE_METADATA, software_version);
    field_names[i] = "Software Version";
    field_type[i] = H5Tcopy(H5T_C_S1);
    if (H5Tset_size(field_type[i], sizeof(md.software_version)) < 0)
    {
        IAS_LOG_ERROR("Setting size of Software Version field");
        error = 1;
    }
    fields_to_close[i] = field_type[i];
    field_size[i] = sizeof(md.software_version);

    offsets[++i] = HOFFSET(IAS_L1G_FILE_METADATA, ingest_software_version);
    field_names[i] = "Ingest Software Version";
    field_type[i] = H5Tcopy(H5T_C_S1);
    if (H5Tset_size(field_type[i], sizeof(md.ingest_software_version)) < 0)
    {
        IAS_LOG_ERROR("Setting size of Ingest Software Version field");
        error = 1;
    }
    fields_to_close[i] = field_type[i];
    field_size[i] = sizeof(md.ingest_software_version);

    offsets[++i] = HOFFSET(IAS_L1G_FILE_METADATA, sun_azimuth);
    field_names[i] = "Sun Azimuth";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_size[i] = sizeof(md.sun_azimuth);

    offsets[++i] = HOFFSET(IAS_L1G_FILE_METADATA, sun_elevation);
    field_names[i] = "Sun Elevation";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_size[i] = sizeof(md.sun_elevation);

    offsets[++i] = HOFFSET(IAS_L1G_FILE_METADATA, sun_angles_valid);
    field_names[i] = "Sun Angles Valid";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(md.sun_angles_valid);

    if (error)
    {
        /* an error occurred generating the table */
        IAS_LOG_ERROR("Building file metadata table definition");
        cleanup_table_description(fields_to_close);
        return ERROR;
    }
    if (i < (NFIELDS - 1))
    {
        /* fewer fields were added than expected */
        IAS_LOG_ERROR("Defined %d fields, but expected %d", i + 1, NFIELDS);
        cleanup_table_description(fields_to_close);
        return ERROR;
    }
    else if (i >= NFIELDS)
    {
        /* more fields than expected.  The stack is probably now corrupt so
           just exit since this is an obvious programming error that was just
           introduced. */
        IAS_LOG_ERROR("Too many fields found - stack probably "
                      "corrupted - exiting");
        exit(EXIT_FAILURE);
    }

    return SUCCESS;
}

/******************************************************************************
Name: ias_l1g_set_file_metadata

Purpose: Sets the file level metadata in a file that is opened in write or
    update mode.

Returns:
    SUCCESS or ERROR
******************************************************************************/
int ias_l1g_set_file_metadata
(
    L1GIO *l1g_file,             /* I: L1G IO structure to access */
    const IAS_L1G_FILE_METADATA *metadata /* I: metadata to set */
)
{
    int status;

    /* arrays to hold the description of every field in the file metadata 
       table */
    size_t offsets[NFIELDS];
    const char *field_names[NFIELDS];
    hid_t field_type[NFIELDS];
    hid_t fields_to_close[NFIELDS];
    size_t field_size[NFIELDS];

    /* verify the input parameter is valid */
    if (!l1g_file)
    {
        IAS_LOG_ERROR("NULL pointer provided for the L1G file");
        return ERROR;
    }

    /* return an error if file is opened read only */
    if (l1g_file->access_mode == IAS_READ)
    {
        IAS_LOG_ERROR("Attempting to write to file %s which is opened "
                      "for reading", l1g_file->filename);
        return ERROR;
    }

    if (build_table_description(offsets, field_names, field_type,
                fields_to_close, field_size) == ERROR)
    {
        IAS_LOG_ERROR("Building the file metadata table description for "
                      "file %s", l1g_file->filename);
        return ERROR;
    }

    if (!l1g_file->file_metadata_present)
    {
        /* create the table since it doesn't exist yet.  Set the chunk size to
           one since only one file metadata record will be present in the 
           file. */
        status = H5TBmake_table("File Metadata", l1g_file->file_id, 
                                FILE_METADATA_TABLE, NFIELDS, 1, 
                                sizeof(*metadata), field_names, offsets, 
                                field_type, 1, NULL, NO_COMPRESSION, metadata);
        l1g_file->file_metadata_present = TRUE;
    }
    else
    {
        /* table exists, so overwrite the first record */
        status = H5TBwrite_records(l1g_file->file_id, FILE_METADATA_TABLE, 0,
                                   1, sizeof(*metadata), offsets, field_size,
                                   metadata);
    }

    cleanup_table_description(fields_to_close);

    if (status < 0)
    {
        IAS_LOG_ERROR("Writing file metadata table to file %s",
                      l1g_file->filename);
        return ERROR;
    }   

    return SUCCESS;
}

/******************************************************************************
Name: ias_l1g_get_file_metadata

Purpose: Gets the file level metadata from an image file.

Returns:
    SUCCESS or ERROR
******************************************************************************/
int ias_l1g_get_file_metadata
(
    L1GIO *l1g_file,                /* I: L1G IO structure for file */
    IAS_L1G_FILE_METADATA *metadata /* O: output metadata */
)
{
    int status;

    /* arrays to hold the description of every field in the file metadata 
       table */
    size_t offsets[NFIELDS];
    const char *field_names[NFIELDS];
    hid_t field_type[NFIELDS];
    hid_t fields_to_close[NFIELDS];
    size_t field_size[NFIELDS];

    /* verify the input parameter is valid */
    if (!l1g_file)
    {
        IAS_LOG_ERROR("NULL pointer provided for the L1G file");
        return ERROR;
    }

    if (!l1g_file->file_metadata_present)
    {
        /* the file metadata is not available yet, so it is an error */
        IAS_LOG_ERROR("Attempted to read file metadata from file %s before "
                      "it has been set", l1g_file->filename);
        return ERROR;
    }

    if (build_table_description(offsets, field_names, field_type,
            fields_to_close, field_size) == ERROR)
    {
        IAS_LOG_ERROR("Building the file metadata table description for "
                      "file %s", l1g_file->filename);
        return ERROR;
    }

    /* make sure the metadata structure is initialized before reading */
    memset(metadata, 0, sizeof(*metadata));

    /* read the metadata */
    status = H5TBread_table(l1g_file->file_id, FILE_METADATA_TABLE,
                            sizeof(*metadata), offsets, field_size, metadata);

    cleanup_table_description(fields_to_close);

    if (status < 0)
    {
        IAS_LOG_ERROR("Reading file metadata for file %s",
                      l1g_file->filename);
        return ERROR;
    }
    
    return SUCCESS;
}
