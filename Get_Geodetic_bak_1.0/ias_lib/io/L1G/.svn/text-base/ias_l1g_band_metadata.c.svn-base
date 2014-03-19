/******************************************************************************
Name: ias_l1g_get_band_metadata
      ias_l1g_set_band_metadata

Purpose: routines to read/write band metadata

******************************************************************************/
#include <stdlib.h>          /* for malloc */
#include <string.h>
#include "hdf5.h"
#include "hdf5_hl.h"
#include "ias_types.h"      /* IAS_READ definition */
#include "ias_const.h"      /* SUCCESS/ERROR definition */
#include "ias_logging.h"
#include "ias_l1g.h"         /* L1G structure definition and APIs */
#include "ias_l1g_private.h" /* Internal/private definitions */

#define NFIELDS 24
/* fields in the band metadata information structure */

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

Purpose: Builds the table description for the band metadata table.

Returns:
    SUCCESS or ERROR
******************************************************************************/
static int build_table_description
(
    size_t offsets[NFIELDS],         /* O: array of offsets in the structure */
    const char *field_names[NFIELDS],/* O: array of pointers to field names */
    hid_t field_type[NFIELDS],       /* O: array of field types */
    hid_t fields_to_close[NFIELDS],  /* O: array of fields to close */
    size_t field_sizes[NFIELDS]      /* O: array of field sizes */
)
{
    int i;
    int error = 0;             /* flag to indicate an error was encountered */
    IAS_L1G_BAND_METADATA bmd; /* dummy structure for easy size values */

    /* initialize the list of fields to close */
    for (i = 0; i < NFIELDS; i++)
        fields_to_close[i] = -1;

    /* define the table contents.  For each field in the table, define the
       offset to the data in the structure, the field name, data type, and 
       field size.  For fields that need to be properly sized, H5Tset_size
       is called with the correct size and the created type is added to the
       list of fields that need to be closed. */
    i = 0;

    offsets[i] = HOFFSET(IAS_L1G_BAND_METADATA, band_number);
    field_names[i] = "Band Number";
    field_type[i] = H5T_NATIVE_INT;
    field_sizes[i] = sizeof(bmd.band_number);

    offsets[++i] = HOFFSET(IAS_L1G_BAND_METADATA, band_name);
    field_names[i] = "Band Name";
    field_type[i] = H5Tcopy(H5T_C_S1);
    if (H5Tset_size(field_type[i], sizeof(bmd.band_name)) < 0)
    {
        IAS_LOG_ERROR("Setting Band Name field size");
        error = 1;
    }
    fields_to_close[i] = field_type[i];
    field_sizes[i] = sizeof(bmd.band_name);

    offsets[++i] = HOFFSET(IAS_L1G_BAND_METADATA, upper_left_y);
    field_names[i] = "Upper Left Y";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_sizes[i] = sizeof(bmd.upper_left_y);

    offsets[++i] = HOFFSET(IAS_L1G_BAND_METADATA, upper_left_x);
    field_names[i] = "Upper Left X";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_sizes[i] = sizeof(bmd.upper_left_x);

    offsets[++i] = HOFFSET(IAS_L1G_BAND_METADATA, upper_right_y);
    field_names[i] = "Upper Right Y";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_sizes[i] = sizeof(bmd.upper_right_y);

    offsets[++i] = HOFFSET(IAS_L1G_BAND_METADATA, upper_right_x);
    field_names[i] = "Upper Right X";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_sizes[i] = sizeof(bmd.upper_right_x);

    offsets[++i] = HOFFSET(IAS_L1G_BAND_METADATA, lower_left_y);
    field_names[i] = "Lower Left Y";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_sizes[i] = sizeof(bmd.lower_left_y);

    offsets[++i] = HOFFSET(IAS_L1G_BAND_METADATA, lower_left_x);
    field_names[i] = "Lower Left X";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_sizes[i] = sizeof(bmd.lower_left_x);

    offsets[++i] = HOFFSET(IAS_L1G_BAND_METADATA, lower_right_y);
    field_names[i] = "Lower Right Y";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_sizes[i] = sizeof(bmd.lower_right_y);

    offsets[++i] = HOFFSET(IAS_L1G_BAND_METADATA, lower_right_x);
    field_names[i] = "Lower Right X";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_sizes[i] = sizeof(bmd.lower_right_x);

    offsets[++i] = HOFFSET(IAS_L1G_BAND_METADATA, projection_distance_x);
    field_names[i] = "Projection Distance X";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_sizes[i] = sizeof(bmd.projection_distance_x);

    offsets[++i] = HOFFSET(IAS_L1G_BAND_METADATA, projection_distance_y);
    field_names[i] = "Projection Distance Y";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_sizes[i] = sizeof(bmd.projection_distance_y);

    offsets[++i] = HOFFSET(IAS_L1G_BAND_METADATA, maximum_pixel_value);
    field_names[i] = "Maximum Pixel Value";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_sizes[i] = sizeof(bmd.maximum_pixel_value);

    offsets[++i] = HOFFSET(IAS_L1G_BAND_METADATA, minimum_pixel_value);
    field_names[i] = "Minimum Pixel Value";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_sizes[i] = sizeof(bmd.minimum_pixel_value);

    offsets[++i] = HOFFSET(IAS_L1G_BAND_METADATA, pixel_range_valid);
    field_names[i] = "Pixel Range Valid";
    field_type[i] = H5T_NATIVE_INT;
    field_sizes[i] = sizeof(bmd.pixel_range_valid);

    offsets[++i] = HOFFSET(IAS_L1G_BAND_METADATA, maximum_radiance);
    field_names[i] = "Maximum Radiance";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_sizes[i] = sizeof(bmd.maximum_radiance);

    offsets[++i] = HOFFSET(IAS_L1G_BAND_METADATA, minimum_radiance);
    field_names[i] = "Minimum Radiance";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_sizes[i] = sizeof(bmd.minimum_radiance);

    offsets[++i] = HOFFSET(IAS_L1G_BAND_METADATA,
                           spectral_radiance_scaling_offset);
    field_names[i] = "Spectral Radiance Scaling Offset";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_sizes[i] = sizeof(bmd.spectral_radiance_scaling_offset);

    offsets[++i] = HOFFSET(IAS_L1G_BAND_METADATA,
                           spectral_radiance_scaling_gain);
    field_names[i] = "Spectral Radiance Scaling Gain";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_sizes[i] = sizeof(bmd.spectral_radiance_scaling_gain);

    offsets[++i] = HOFFSET(IAS_L1G_BAND_METADATA, radiance_valid);
    field_names[i] = "Radiance Valid";
    field_type[i] = H5T_NATIVE_INT;
    field_sizes[i] = sizeof(bmd.radiance_valid);

    offsets[++i] = HOFFSET(IAS_L1G_BAND_METADATA, reflectance_scaling_offset);
    field_names[i] = "Reflectance Scaling Offset";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_sizes[i] = sizeof(bmd.reflectance_scaling_offset);

    offsets[++i] = HOFFSET(IAS_L1G_BAND_METADATA, reflectance_scaling_gain);
    field_names[i] = "Reflectance Scaling Gain";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_sizes[i] = sizeof(bmd.reflectance_scaling_gain);

    offsets[++i] = HOFFSET(IAS_L1G_BAND_METADATA, reflectance_valid);
    field_names[i] = "Reflectance Valid";
    field_type[i] = H5T_NATIVE_INT;
    field_sizes[i] = sizeof(bmd.reflectance_valid);

    offsets[++i] = HOFFSET(IAS_L1G_BAND_METADATA, instrument_source);
    field_names[i] = "Instrument Source";
    field_type[i] = H5Tcopy(H5T_C_S1);
    if (H5Tset_size(field_type[i], sizeof(bmd.instrument_source)) < 0)
    {
        IAS_LOG_ERROR("Setting Instrument Source field size");
        error = 1;
    }
    fields_to_close[i] = field_type[i];
    field_sizes[i] = sizeof(bmd.instrument_source);

    if (error)
    {
        /* an error occurred generating the table */
        IAS_LOG_ERROR("Building band metadata table definition");
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
        IAS_LOG_ERROR("Too many fields found - stack probably corrupted - "
                      "exiting");
        exit(EXIT_FAILURE);
    }

    return SUCCESS;
}

/******************************************************************************
Name: create_and_add_band_metadata_to_list

Purpose: Allocates a node for the band metadata cache and adds it to the 
    linked list used to maintain the cache.

Returns:
    Pointer to the cache node or NULL if an error occurs
******************************************************************************/
static BAND_METADATA_CACHE_NODE *create_and_add_band_metadata_to_list
(
    L1GIO *l1g_file        /* I: file to search for metadata */
)
{
    BAND_METADATA_CACHE_NODE *current;

    /* allocate memory for the node */
    current = malloc(sizeof(*current));
    if (!current)
    {
        IAS_LOG_ERROR("Allocating memory for band metadata node");
        return NULL;
    }

    /* initialize the node and add it to the linked list */
    memset(&current->metadata, 0, sizeof(current->metadata));
    ias_linked_list_initialize_node(&current->node);
    ias_linked_list_add_tail(&l1g_file->band_metadata, &current->node);

    return current;
}

/******************************************************************************
Name: find_metadata_for_band

Purpose: Searches for the metadata for a specific band in the cache.

Returns:
    A pointer to the cache node if the band is found, or NULL if the band is
    not found.
******************************************************************************/
static BAND_METADATA_CACHE_NODE *find_metadata_for_band
(
    L1GIO *l1g_file,        /* I: file to search for metadata */
    int band_number         /* I: Band number to find */
)
{
    BAND_METADATA_CACHE_NODE *current;

    /* search the metadata for the correct band */
    GET_OBJECT_FOR_EACH_ENTRY(current, &l1g_file->band_metadata,
                BAND_METADATA_CACHE_NODE, node)
    {
        if (band_number == current->metadata.band_number)
            return current;
    }

    /* the band wasn't found */
    return NULL;
}

/******************************************************************************
Name: read_band_metadata

Purpose: Reads the band metadata table from the file and loads it into the 
    band metadata cache.

Returns:
    SUCCESS or ERROR
******************************************************************************/
static int read_band_metadata
(
    L1GIO *l1g_file              /* I: file to read metadata for */
)
{
    int status;
    hsize_t number_of_fields;
    hsize_t number_of_records;
    int record;

    /* if it is known that the band metadata table doesn't exist, return */
    if (!l1g_file->band_metadata_in_file)
    {
        l1g_file->band_metadata_valid = TRUE;
        l1g_file->band_metadata_records_in_file = 0;
        return SUCCESS;
    }

    /* get the info about the band metadata table from the file, primarily
       for the number of records in the table */
    status = H5TBget_table_info(l1g_file->file_id, BAND_METADATA_TABLE,
                &number_of_fields, &number_of_records);
    if (status < 0)
    {
        /* the table should have existed, so that is an error */
        IAS_LOG_ERROR("Unable to read band metadata table for file %s",
                      l1g_file->filename);
        return ERROR;
    }

    /* make sure the expected number of fields was found */
    if (number_of_fields != NFIELDS)
    {
        IAS_LOG_WARNING("Expected %d fields in the band metadata table but "
                        "found %d", NFIELDS, (int)number_of_fields);
    }

    /* read the data from the table */
    if (number_of_records > 0)
    {
        /* arrays to hold the table description info */
        size_t offsets[NFIELDS];
        const char *field_names[NFIELDS];
        hid_t field_type[NFIELDS];
        size_t field_sizes[NFIELDS];
        hid_t fields_to_close[NFIELDS];

        status = build_table_description(offsets, field_names, field_type,
                                         fields_to_close, field_sizes);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Building the band metadata table description for "
                          "file %s", l1g_file->filename);
            return ERROR;
        }

        /* read each of the records and add them to the cache */
        for (record = 0; record < number_of_records; record++)
        {
            IAS_L1G_BAND_METADATA metadata;
            BAND_METADATA_CACHE_NODE *current;

            /* read the record */
            if (H5TBread_records(l1g_file->file_id, BAND_METADATA_TABLE, 
                          record, 1, sizeof(metadata), offsets, field_sizes,
                          &metadata) < 0)
            {
                IAS_LOG_ERROR("Reading band metadata table for record"
                              " %d in %s", record, l1g_file->filename);
                cleanup_table_description(fields_to_close);
                return ERROR;
            }

            /* as a consistency check, verify the band just read doesn't
               already exist in the band metadata list.  If it does, there is
               a bug writing the data to the file. */
            if (find_metadata_for_band(l1g_file, metadata.band_number) != NULL)
            {
                IAS_LOG_ERROR("Duplicate entries for band number %d found "
                    "in the band metadata in %s", metadata.band_number,
                    l1g_file->filename);
                cleanup_table_description(fields_to_close);
                return ERROR;
            }

            /* allocate space for the record */
            current = create_and_add_band_metadata_to_list(l1g_file);
            if (!current)
            {
                IAS_LOG_ERROR("Creating new band metadata node");
                cleanup_table_description(fields_to_close);
                return ERROR;
            }

            /* copy the metadata to the record */
            current->metadata = metadata;
        }

        cleanup_table_description(fields_to_close);
    }

    /* remember the number of records in the file and that the band metadata
       is valid */
    l1g_file->band_metadata_records_in_file = number_of_records;
    l1g_file->band_metadata_valid = TRUE;

    return SUCCESS;
}

/******************************************************************************
Name: write_band_metadata

Purpose: Writes the band metadata table present in the cache to the file.

Returns:
    SUCCESS or ERROR
******************************************************************************/
static int write_band_metadata
(
    L1GIO *l1g_file        /* I: file to write band metadata in */
)
{
    int status = 0;
    int record_index;
    BAND_METADATA_CACHE_NODE *current;

    /* arrays for holding the table description */
    size_t offsets[NFIELDS];
    const char *field_names[NFIELDS];
    hid_t field_type[NFIELDS];
    hid_t fields_to_close[NFIELDS];
    size_t field_sizes[NFIELDS];

    if (build_table_description(offsets, field_names, field_type,
                                fields_to_close, field_sizes) == ERROR)
    {
        IAS_LOG_ERROR("Building the band metadata table description for "
                      "file %s", l1g_file->filename);
        return ERROR;
    }

    /* loop through the list of metadata and write it to the table a record
       at a time */
    record_index = 0;
    GET_OBJECT_FOR_EACH_ENTRY(current, &l1g_file->band_metadata,
                BAND_METADATA_CACHE_NODE, node)
    {
        /* if the table doesn't exist yet, create it when the first
           record is written */
        if (!l1g_file->band_metadata_in_file)
        {
            /* table isn't already in the file, so create it and add the first
               record to it,  Set the chunk size to 11 since in most cases that
               should be sufficient for all the bands in an L1G file. */
            status = H5TBmake_table("Band Metadata", l1g_file->file_id,
                        BAND_METADATA_TABLE, NFIELDS, 1,
                        sizeof(IAS_L1G_BAND_METADATA), field_names, offsets, 
                        field_type, 11, NULL, NO_COMPRESSION, 
                        &current->metadata);
            if (status >= 0)
            {
                l1g_file->band_metadata_in_file = TRUE;
                l1g_file->band_metadata_records_in_file = 1;
            }
        }
        else if (record_index < l1g_file->band_metadata_records_in_file)
        {
            /* overwrite the initial records in the file */
            status = H5TBwrite_records(l1g_file->file_id, BAND_METADATA_TABLE,
                        record_index, 1, sizeof(IAS_L1G_BAND_METADATA), offsets,
                        field_sizes, &current->metadata);
        }
        else
        {
            /* append new records at the end */
            status = H5TBappend_records(l1g_file->file_id, BAND_METADATA_TABLE,
                     1, sizeof(IAS_L1G_BAND_METADATA), offsets, field_sizes,
                     &current->metadata);
            l1g_file->band_metadata_records_in_file++;
        }
        if (status < 0)
        {
            cleanup_table_description(fields_to_close);
            IAS_LOG_ERROR("Writing band metadata record %d to file %s",
                          record_index, l1g_file->filename);
            return ERROR;
        }
        record_index++;
    }

    cleanup_table_description(fields_to_close);

    return SUCCESS;
}

/******************************************************************************
Name: ias_l1g_get_band_metadata

Purpose: Retrieves the metadata for a requested band.

Returns:
    SUCCESS or ERROR
******************************************************************************/
int ias_l1g_get_band_metadata
(
    L1GIO *l1g_file,                /* I: file to read metadata from */
    int band_number,                /* I: band number to get */
    IAS_L1G_BAND_METADATA *metadata /* O: buffer to return data in */
)
{
    BAND_METADATA_CACHE_NODE *current;

    /* verify the input parameter is valid */
    if (!l1g_file)
    {
        IAS_LOG_ERROR("NULL pointer provided for the L1G file");
        return ERROR;
    }

    /* if metadata is not valid yet, read it from the file */
    if (!l1g_file->band_metadata_valid)
    {
        /* read the band metadata into the cache */
        if (read_band_metadata(l1g_file) != SUCCESS)
        {
            IAS_LOG_ERROR("Reading band metadata for %s",
                          l1g_file->filename);
            return ERROR;
        }
    }

    /* search the metadata for the correct band */
    current = find_metadata_for_band(l1g_file, band_number);
    if (!current)
    {
        IAS_LOG_ERROR("Metadata not found for band %d in file %s",
                      band_number, l1g_file->filename);
        return ERROR;
    }

    /* copy metadata for the entry found */
    memcpy(metadata, &current->metadata, sizeof(*metadata));

    return SUCCESS;
}

/******************************************************************************
Name: ias_l1g_set_band_metadata

Purpose: Writes the metadata provided to the file.

Returns:
    SUCCESS or ERROR
******************************************************************************/
int ias_l1g_set_band_metadata
(
    L1GIO *l1g_file,            /* I: file to set metadata for */
    const IAS_L1G_BAND_METADATA *metadata, /* I: array of metadata to set */
    int metadata_size           /* I: number of entries in metadata array */
)
{
    int new_index;
    IAS_L1G_BAND_METADATA *destination;  /* destination band metadata pointer */
    const IAS_L1G_BAND_METADATA *source; /* source band metadata pointer */
    BAND_METADATA_CACHE_NODE *metadata_node;

    /* verify the input parameter is valid */
    if (!l1g_file)
    {
        IAS_LOG_ERROR("NULL pointer provided for the L1G file");
        return ERROR;
    }

    /* make sure the file is open in write or update mode before allowing
       metadata to be set */
    if (l1g_file->access_mode == IAS_READ)
    {
        IAS_LOG_ERROR("Attempting to write to file %s which is opened "
                      "for reading", l1g_file->filename);
        return ERROR;
    }

    /* if metadata is not valid yet, read it from the file */
    if (!l1g_file->band_metadata_valid)
    {
        /* read the band metadata into the cache */
        if (read_band_metadata(l1g_file) != SUCCESS)
        {
            IAS_LOG_ERROR("Reading band metadata from file %s",
                          l1g_file->filename);
            return ERROR;
        }
    }

    /* merge the new metadata into the existing metadata cache*/
    for (new_index = 0; new_index < metadata_size; new_index++)
    {
        /* check whether there is a band to replace */
        metadata_node = find_metadata_for_band(l1g_file, 
                                         metadata[new_index].band_number);

        /* if the band isn't already in the list, allocate a new node and
           add it to the list */
        if (!metadata_node)
        {
            metadata_node = create_and_add_band_metadata_to_list(l1g_file);
            if (!metadata_node)
            {
                IAS_LOG_ERROR("Adding band metadata to list");
                return ERROR;
            }
        }

        /* populate the new metadata.  This isn't simply done with a memcpy
           since some fields are only set if the input information is
           considered valid. */
        destination = &metadata_node->metadata;
        source = &metadata[new_index];
        memset(destination, 0, sizeof(*destination));
        destination->band_number = source->band_number;
        strcpy(destination->band_name, source->band_name);
        destination->upper_left_y = source->upper_left_y;
        destination->upper_left_x = source->upper_left_x;
        destination->upper_right_y = source->upper_right_y;
        destination->upper_right_x = source->upper_right_x;
        destination->lower_left_y = source->lower_left_y;
        destination->lower_left_x = source->lower_left_x;
        destination->lower_right_y = source->lower_right_y;
        destination->lower_right_x = source->lower_right_x;
        destination->projection_distance_x = source->projection_distance_x;
        destination->projection_distance_y = source->projection_distance_y;
        if (source->pixel_range_valid)
        {
            destination->maximum_pixel_value = source->maximum_pixel_value;
            destination->minimum_pixel_value = source->minimum_pixel_value;
        }
        destination->pixel_range_valid = source->pixel_range_valid;
        if (source->radiance_valid)
        {
            destination->maximum_radiance = source->maximum_radiance;
            destination->minimum_radiance = source->minimum_radiance;
            destination->spectral_radiance_scaling_offset
                    = source->spectral_radiance_scaling_offset;
            destination->spectral_radiance_scaling_gain
                    = source->spectral_radiance_scaling_gain;
        }
        destination->radiance_valid = source->radiance_valid;
        if (source->reflectance_valid)
        {
            destination->reflectance_scaling_offset
                    = source->reflectance_scaling_offset;
            destination->reflectance_scaling_gain
                    = source->reflectance_scaling_gain;
        }
        destination->reflectance_valid = source->reflectance_valid;
        strcpy(destination->instrument_source, source->instrument_source);
    }

    /* write the new band metadata to the file */
    return write_band_metadata(l1g_file);
}
