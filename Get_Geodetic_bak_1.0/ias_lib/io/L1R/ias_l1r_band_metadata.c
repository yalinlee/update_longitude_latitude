/******************************************************************************
Name: ias_l1r_get_band_metadata
      ias_l1r_set_band_metadata

Purpose: Routines to read/write band metadata

Note:    To provide backward compatibility, it is allowed to read older
         versions of the metadata.  The data is read from the older version
         and copied into the current version's data structure.  If the
         data stored in the old version doesn't translate directly into the
         new structure it's lost in the translation.  Data that was moved
         from file metadata to band metadata is lost since it can't be
         reliably translated.

         Sections follow that contain the data structure; table definition;
         and routine to copy from old to new.  The final section contains the
         current version table and external get and set routines.

Routines:   ias_l1r_get_band_metadata
            ias_l1r_set_band_metadata
            copy_v1_band_metadata                     (internal)
            build_v1_table_description                (internal)
            copy_v2_band_metadata                     (internal)
            build_v2_table_description                (internal)
            cleanup_table_description                 (internal)
            build_table_description                   (internal)
            read_band_metadata                        (internal)
            write_band_metadata                       (internal)
            create_and_add_band_metadata_to_list      (internal)
            find_metadata_for_band                    (internal)
******************************************************************************/
#include <stdlib.h>         /* for malloc */
#include <string.h>
#include "ias_types.h"
#include "ias_const.h"
#include "ias_logging.h"
#include "ias_l1r.h"        /* L1R structure definition and APIs */
#include "ias_l1r_private.h" /* Internal/private definitions */
#include "hdf5_hl.h"

/* Number of fields in earlier versions of the band metadata structure */
#define V1_BAND_NFIELDS 4
#define V2_BAND_NFIELDS 5

/* Number of fields in the current band metadata information structure */
#define NFIELDS 10

/* Define the maximum number of fields -- this must be the largest of the
   previously defined NFIELDS values or bad things will happen */
#define MAX_NFIELDS NFIELDS

/* Version 1 of the band metadata structure */
typedef struct ias_l1r_v1_band_metadata
{
    int band_number;                     /* band number */
    char band_name[IAS_BAND_NAME_SIZE];  /* from ias_satellite_attributes.h
                                            "IAS_COASTAL_AEROSOL_BAND",
                                            "IAS_BLUE_BAND", "IAS_GREEN_BAND",
                                            "IAS_RED_BAND", "IAS_NIR_BAND",
                                            "IAS_SWIR1_BAND", "IAS_SWIR2_BAND",
                                            "IAS_PAN_BAND", "IAS_CIRRUS_BAND",
                                            "IAS_THERMAL1_BAND",
                                            "IAS_THERMAL2_BAND" */
    char instrument_source[IAS_INSTRUMENT_SOURCE_SIZE];
    double reflectance_to_radiance_coefficient;  /* Conversion factor to
                                                    go from reflectance
                                                    to radiance */
} IAS_L1R_V1_BAND_METADATA;

/* Version 2 of the band metadata structure */
typedef struct ias_l1r_v2_band_metadata
{
   int band_number;                     /* band number */
   char band_name[IAS_BAND_NAME_SIZE];  /* from ias_satellite_attributes.h
                                           "IAS_COASTAL_AEROSOL_BAND",
                                           "IAS_BLUE_BAND", "IAS_GREEN_BAND",
                                           "IAS_RED_BAND", "IAS_NIR_BAND",
                                           "IAS_SWIR1_BAND", "IAS_SWIR2_BAND",
                                           "IAS_PAN_BAND", "IAS_CIRRUS_BAND",
                                           "IAS_THERMAL1_BAND",
                                           "IAS_THERMAL2_BAND" */
   char instrument_source[IAS_INSTRUMENT_SOURCE_SIZE];
   char discontinuity_factor_source[IAS_L1R_SOURCE_SIZE];
                                        /* "CPF", "SCA_OVERLAP_CHAR" */
   double reflectance_to_radiance_coefficient;  /* Conversion factor to
                                                   go from reflectance
                                                   to radiance */
} IAS_L1R_V2_BAND_METADATA;


/******************************************************************************
Name: cleanup_table_description

Purpose: Cleans up the resources allocated to an L1R band metadata
         table description

Returns:
    nothing
******************************************************************************/
static void cleanup_table_description
(
    hid_t fields_to_close[MAX_NFIELDS]    /* I: array of fields to close */
)
{
    int i;

    /* close any types created for the table description */
    for (i = 0; i < MAX_NFIELDS; i++)
    {
        if (fields_to_close[i] >= 0)
            H5Tclose(fields_to_close[i]);
    }
}


/*------------------------------------------------------------------------*/
/*          This section contains Version 1 of the band metadata          */
/*------------------------------------------------------------------------*/
/**************************************************************************
 NAME:     copy_v1_band_metadata

 PURPOSE:  Copies version 1 band metadata to current format

 RETURNS:  Nothing
***************************************************************************/
static void copy_v1_band_metadata
(
    const IAS_L1R_V1_BAND_METADATA *v1_bmd, /* I: version 1 metadata */
    IAS_L1R_BAND_METADATA *bmd              /* O: metadata in version 2
                                               format */
)
{
    /* Copy the existing version 1 metadata to the current data
       structure. Initialize the current data structure to 0 before
       copying anything.  */
    memset(bmd, 0, sizeof(*bmd));
    strcpy(bmd->band_name, v1_bmd->band_name);
    strcpy(bmd->instrument_source, v1_bmd->instrument_source);
    bmd->band_number = v1_bmd->band_number;
    bmd->reflectance_to_radiance_coefficient =
        v1_bmd->reflectance_to_radiance_coefficient;
}



/******************************************************************************
Name:     build_v1_table_description

Purpose: Builds the description of the version 1 band metadata
         table.

Returns:
    SUCCESS or ERROR
******************************************************************************/
static int build_v1_table_description
(
    size_t offsets[MAX_NFIELDS],  /* O: array of offsets in the
                                         structure */
    const char *field_names[MAX_NFIELDS],
                                      /* O: array of pointers to field names */
    hid_t field_type[MAX_NFIELDS],    /* O: array of field types */
    hid_t fields_to_close[MAX_NFIELDS],
                                      /* O: array of fields to close */
    size_t field_sizes[MAX_NFIELDS]
                                      /* O: array of field sizes */
)
{
    int i;
    int error = 0;             /* flag to indicate an error was encountered */
    IAS_L1R_V1_BAND_METADATA bmd; /* dummy structure for easy size values */

    /* initialize the list of fields to close */
    for (i = 0; i < MAX_NFIELDS; i++)
        fields_to_close[i] = -1;

    /* define the table contents.  For each field in the table, define the
       offset to the data in the structure, the field name, data type, and 
       field size.  For fields that need to be properly sized, H5Tset_size
       is called with the correct size and the created type is added to the
       list of fields that need to be closed. */
    i = 0;
    offsets[i] = HOFFSET(IAS_L1R_V1_BAND_METADATA, band_number);
    field_names[i] = "Band Number";
    field_type[i] = H5T_NATIVE_INT;
    field_sizes[i] = sizeof(bmd.band_number);

    offsets[++i] = HOFFSET(IAS_L1R_V1_BAND_METADATA, band_name);
    field_names[i] = "Band Name";
    field_type[i] = H5Tcopy(H5T_C_S1);
    if (H5Tset_size(field_type[i], sizeof(bmd.band_name)) < 0)
    {
        IAS_LOG_ERROR("Setting Band Name field size");
        error = 1;
    }
    fields_to_close[i] = field_type[i];
    field_sizes[i] = sizeof(bmd.band_name);

    offsets[++i] = HOFFSET(IAS_L1R_V1_BAND_METADATA, instrument_source);
    field_names[i] = "Instrument Source";
    field_type[i] = H5Tcopy(H5T_C_S1);
    if (H5Tset_size(field_type[i], sizeof(bmd.instrument_source)) < 0)
    {
        IAS_LOG_ERROR("Setting Instrument Source field size");
        error = 1;
    }
    fields_to_close[i] = field_type[i];
    field_sizes[i] = sizeof(bmd.instrument_source);

    offsets[++i] = HOFFSET(IAS_L1R_V1_BAND_METADATA, 
                           reflectance_to_radiance_coefficient);
    field_names[i] = "Reflectance To Radiance Coefficient";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_sizes[i] = sizeof(bmd.reflectance_to_radiance_coefficient);

    if (error)
    {
        /* an error occurred generating the table */
        IAS_LOG_ERROR("Building band metadata table definition");
        cleanup_table_description(fields_to_close);
        return ERROR;
    }
    if (i < (V1_BAND_NFIELDS - 1))
    {
        /* fewer fields were added than expected */
        IAS_LOG_ERROR("Defined %d fields, but expected %d", i + 1,
            NFIELDS);
        cleanup_table_description(fields_to_close);
        return ERROR;
    }
    else if (i >= V1_BAND_NFIELDS)
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


/*--------------------------------------------------------------------------*/
/*           This section contains Version 2 of the band metadata           */
/*--------------------------------------------------------------------------*/
/**************************************************************************
 NAME:     copy_v2_band_metadata

 PURPOSE:  Copies version 2 band metadata to current format

 RETURNS:  Nothing
***************************************************************************/
static void copy_v2_band_metadata
(
    const IAS_L1R_V2_BAND_METADATA *v2_bmd, /* I: version 2 metadata */
    IAS_L1R_BAND_METADATA *bmd              /* O: metadata in current format */
)
{
    /* Copy the existing version 2 metadata to the current data
       structure. Initialize the current data structure to 0 before
       copying anything.  */
    memset(bmd, 0, sizeof(*bmd));
    strcpy(bmd->band_name, v2_bmd->band_name);
    strcpy(bmd->instrument_source, v2_bmd->instrument_source);
    bmd->band_number = v2_bmd->band_number;
    bmd->reflectance_to_radiance_coefficient =
        v2_bmd->reflectance_to_radiance_coefficient;
    strcpy(bmd->discontinuity_factor_source,
        v2_bmd->discontinuity_factor_source);
}


/******************************************************************************
Name:    build_v2_table_description

Purpose: Builds the current table description for the version 2 band
         metadata table.

Returns: SUCCESS or ERROR
******************************************************************************/
static int build_v2_table_description
(
    size_t offsets[MAX_NFIELDS],         /* O: array of offsets in the
                                            structure */
    const char *field_names[MAX_NFIELDS],/* O: array of pointers to field
                                            names */
    hid_t field_type[MAX_NFIELDS],       /* O: array of field types */
    hid_t fields_to_close[MAX_NFIELDS],  /* O: array of fields to close */
    size_t field_sizes[MAX_NFIELDS]      /* O: array of field sizes */
)
{
    int i;
    int error = 0;             /* flag to indicate an error was encountered */
    IAS_L1R_V2_BAND_METADATA bmd; /* dummy structure for easy size values */

    /* initialize the list of fields to close */
    for (i = 0; i < MAX_NFIELDS; i++)
        fields_to_close[i] = -1;

    /* define the table contents.  For each field in the table, define the
       offset to the data in the structure, the field name, data type, and 
       field size.  For fields that need to be properly sized, H5Tset_size
       is called with the correct size and the created type is added to the
       list of fields that need to be closed. */
    i = 0;
    offsets[i] = HOFFSET(IAS_L1R_V2_BAND_METADATA, band_number);
    field_names[i] = "Band Number";
    field_type[i] = H5T_NATIVE_INT;
    field_sizes[i] = sizeof(bmd.band_number);

    offsets[++i] = HOFFSET(IAS_L1R_V2_BAND_METADATA, band_name);
    field_names[i] = "Band Name";
    field_type[i] = H5Tcopy(H5T_C_S1);
    if (H5Tset_size(field_type[i], sizeof(bmd.band_name)) < 0)
    {
        IAS_LOG_ERROR("Setting Band Name field size");
        error = 1;
    }
    fields_to_close[i] = field_type[i];
    field_sizes[i] = sizeof(bmd.band_name);

    offsets[++i] = HOFFSET(IAS_L1R_V2_BAND_METADATA, instrument_source);
    field_names[i] = "Instrument Source";
    field_type[i] = H5Tcopy(H5T_C_S1);
    if (H5Tset_size(field_type[i], sizeof(bmd.instrument_source)) < 0)
    {
        IAS_LOG_ERROR("Setting Instrument Source field size");
        error = 1;
    }
    fields_to_close[i] = field_type[i];
    field_sizes[i] = sizeof(bmd.instrument_source);

    offsets[++i] = HOFFSET(IAS_L1R_V2_BAND_METADATA,
        discontinuity_factor_source);
    field_names[i] = "Discontinuity Factor Source";
    field_type[i] = H5Tcopy(H5T_C_S1);
    if (H5Tset_size(field_type[i], 
            sizeof(bmd.discontinuity_factor_source)) < 0)
    {
        IAS_LOG_ERROR("Setting size of Discontinuity Factor Source field");
        error = 1;
    }
    fields_to_close[i] = field_type[i];
    field_sizes[i] = sizeof(bmd.discontinuity_factor_source);

    offsets[++i] = HOFFSET(IAS_L1R_V2_BAND_METADATA, 
                           reflectance_to_radiance_coefficient);
    field_names[i] = "Reflectance To Radiance Coefficient";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_sizes[i] = sizeof(bmd.reflectance_to_radiance_coefficient);

    if (error)
    {
        /* an error occurred generating the table */
        IAS_LOG_ERROR("Building band metadata table definition");
        cleanup_table_description(fields_to_close);
        return ERROR;
    }
    if (i < (V2_BAND_NFIELDS - 1))
    {
        /* fewer fields were added than expected */
        IAS_LOG_ERROR("Defined %d fields, but expected %d", i + 1,
            NFIELDS);
        cleanup_table_description(fields_to_close);
        return ERROR;
    }
    else if (i >= V2_BAND_NFIELDS)
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


/*--------------------------------------------------------------------------*/
/*        This section contains current version of the band metadata        */
/*--------------------------------------------------------------------------*/
/******************************************************************************
Name: build_table_description

Purpose: Builds the current table description for the band metadata table.

Returns: SUCCESS or ERROR
******************************************************************************/
static int build_table_description
(
    size_t offsets[MAX_NFIELDS],         /* O: array of offsets in the
                                            structure */
    const char *field_names[MAX_NFIELDS],/* O: array of pointers to field
                                            names */
    hid_t field_type[MAX_NFIELDS],       /* O: array of field types */
    hid_t fields_to_close[MAX_NFIELDS],  /* O: array of fields to close */
    size_t field_sizes[MAX_NFIELDS]      /* O: array of field sizes */
)
{
    int i;
    int error = 0;             /* flag to indicate an error was encountered */
    IAS_L1R_BAND_METADATA bmd; /* dummy structure for easy size values */

    /* initialize the list of fields to close */
    for (i = 0; i < MAX_NFIELDS; i++)
        fields_to_close[i] = -1;

    /* define the table contents.  For each field in the table, define the
       offset to the data in the structure, the field name, data type, and 
       field size.  For fields that need to be properly sized, H5Tset_size
       is called with the correct size and the created type is added to the
       list of fields that need to be closed. */
    i = 0;
    offsets[i] = HOFFSET(IAS_L1R_BAND_METADATA, band_number);
    field_names[i] = "Band Number";
    field_type[i] = H5T_NATIVE_INT;
    field_sizes[i] = sizeof(bmd.band_number);

    offsets[++i] = HOFFSET(IAS_L1R_BAND_METADATA, band_name);
    field_names[i] = "Band Name";
    field_type[i] = H5Tcopy(H5T_C_S1);
    if (H5Tset_size(field_type[i], sizeof(bmd.band_name)) < 0)
    {
        IAS_LOG_ERROR("Setting Band Name field size");
        error = 1;
    }
    fields_to_close[i] = field_type[i];
    field_sizes[i] = sizeof(bmd.band_name);

    offsets[++i] = HOFFSET(IAS_L1R_BAND_METADATA, instrument_source);
    field_names[i] = "Instrument Source";
    field_type[i] = H5Tcopy(H5T_C_S1);
    if (H5Tset_size(field_type[i], sizeof(bmd.instrument_source)) < 0)
    {
        IAS_LOG_ERROR("Setting Instrument Source field size");
        error = 1;
    }
    fields_to_close[i] = field_type[i];
    field_sizes[i] = sizeof(bmd.instrument_source);

    offsets[++i] = HOFFSET(IAS_L1R_BAND_METADATA,
        discontinuity_factor_source);
    field_names[i] = "Discontinuity Factor Source";
    field_type[i] = H5Tcopy(H5T_C_S1);
    if (H5Tset_size(field_type[i], 
            sizeof(bmd.discontinuity_factor_source)) < 0)
    {
        IAS_LOG_ERROR("Setting size of Discontinuity Factor Source field");
        error = 1;
    }
    fields_to_close[i] = field_type[i];
    field_sizes[i] = sizeof(bmd.discontinuity_factor_source);

    offsets[++i] = HOFFSET(IAS_L1R_BAND_METADATA, 
                           reflectance_to_radiance_coefficient);
    field_names[i] = "Reflectance To Radiance Coefficient";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_sizes[i] = sizeof(bmd.reflectance_to_radiance_coefficient);

    offsets[++i] = HOFFSET(IAS_L1R_BAND_METADATA, bias_source);
    field_names[i] = "Bias Source";
    field_type[i] = H5Tcopy(H5T_C_S1);
    if (H5Tset_size(field_type[i], sizeof(bmd.bias_source)) < 0)
    {
        IAS_LOG_ERROR("Setting size of Bias Source field");
        error = 1;
    }
    fields_to_close[i] = field_type[i];
    field_sizes[i] = sizeof(bmd.bias_source);

    offsets[++i] = HOFFSET(IAS_L1R_BAND_METADATA, linearization);
    field_names[i] = "Linearization Applied";
    field_type[i] = H5Tcopy(H5T_C_S1);
    if (H5Tset_size(field_type[i], sizeof(bmd.linearization)) < 0)
    {
        IAS_LOG_ERROR("Setting size of Linearization Applied field");
        error = 1;
    }
    fields_to_close[i] = field_type[i];
    field_sizes[i] = sizeof(bmd.linearization);

    offsets[++i] = HOFFSET(IAS_L1R_BAND_METADATA,
                           bias_temp_sensitivity_correction_applied);
    field_names[i] = "Bias Temperature Sensitivity Correction Applied Flag";
    field_type[i] = H5T_NATIVE_INT;
    field_sizes[i] = sizeof(bmd.bias_temp_sensitivity_correction_applied);

    offsets[++i] = HOFFSET(IAS_L1R_BAND_METADATA,
                           gain_temp_sensitivity_correction_applied);
    field_names[i] = "Gain Temperature Sensitivity Correction Applied Flag";
    field_type[i] = H5T_NATIVE_INT;
    field_sizes[i] = sizeof(bmd.gain_temp_sensitivity_correction_applied);

    offsets[++i] = HOFFSET(IAS_L1R_BAND_METADATA,
        bias_correction_applied);
    field_names[i] = "Bias Correction Applied Flag";
    field_type[i] = H5T_NATIVE_INT;
    field_sizes[i] = sizeof(bmd.bias_correction_applied);

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
        IAS_LOG_ERROR("Defined %d fields, but expected %d", i + 1,
            NFIELDS);
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
Name: create_and_add_band_metadata_to_list

Purpose: Allocates a node for the band metadata cache and adds it to the 
    linked list used to maintain the cache.

Returns:
    Pointer to the cache node or NULL if an error occurs
******************************************************************************/
static BAND_METADATA_CACHE_NODE *create_and_add_band_metadata_to_list
(
    L1RIO *l1r_file        /* I: file to search for metadata */
)
{
    BAND_METADATA_CACHE_NODE *current;
    current = NULL;

    /* allocate memory for the node */
    current = malloc(sizeof(*current));
    if (!current)
    {
        IAS_LOG_ERROR("Allocating memory");
        return NULL;
    }

    /* initialize the node and add it to the linked list */
    memset(&current->metadata, 0, sizeof(current->metadata));
    ias_linked_list_initialize_node(&current->node);
    ias_linked_list_add_tail(&l1r_file->band_metadata, &current->node);

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
    L1RIO *l1r_file,        /* I: file to search for metadata */
    int band_number         /* I: Band number to find */
)
{
    BAND_METADATA_CACHE_NODE *current;

    /* search the metadata for the correct band */
    GET_OBJECT_FOR_EACH_ENTRY(current, &l1r_file->band_metadata,
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
    L1RIO *l1r_file             /* I: file to read metadata for */
)
{
    hsize_t number_of_fields;
    hsize_t number_of_records;
    herr_t  hdf_status;
    int record;
    int status;
    int number_of_fields_in_this_version;

    /* if it is known that the band metadata table doesn't exist, return */
    if (!l1r_file->band_metadata_in_file)
    {
        /* there is no metadata in the file, so consider the empty list
           valid */
        l1r_file->band_metadata_valid = TRUE;
        l1r_file->band_metadata_records_in_file = 0;
        return SUCCESS;
    }

    /* get the info about the band metadata table from the file, primarily
       for the number of records in the table */
    hdf_status = H5TBget_table_info(l1r_file->file_id, BAND_METADATA_TABLE,
        &number_of_fields, &number_of_records);
    if (hdf_status < 0)
    {
        /* the table should have existed, so that is an error */
        IAS_LOG_ERROR("Unable to read band metadata table for file %s",
            l1r_file->filename);
        return ERROR;
    }

    /* set the correct expected number of fields in the table depending
       on which version we're going to read. */
    if (l1r_file->file_format_version == 1)
        number_of_fields_in_this_version = V1_BAND_NFIELDS;
    else if (l1r_file->file_format_version == 2 )
        number_of_fields_in_this_version = V2_BAND_NFIELDS;
    else
        number_of_fields_in_this_version = NFIELDS;

    /* make sure we've got the correct number of fields for the version
       we're reading. */
    if (number_of_fields != number_of_fields_in_this_version)
    {
        IAS_LOG_ERROR("For version %d, table '%s' should have %d fields,"
            "but H5TBget_table_info returned %d fields",
            l1r_file->file_format_version, BAND_METADATA_TABLE,
            number_of_fields_in_this_version, (int)number_of_fields);
        return ERROR;
    }

    /* get the record(s) that are in the table */
    if (number_of_records > 0)
    {
        const char *field_names[MAX_NFIELDS];
        size_t offsets[MAX_NFIELDS];
        size_t field_sizes[MAX_NFIELDS];
        hid_t  field_type[MAX_NFIELDS];
        hid_t  fields_to_close[MAX_NFIELDS];

        if (l1r_file->file_format_version == 1)
        {
            /* build the version 1 table description */
            status = build_v1_table_description(offsets, field_names,
                field_type, fields_to_close, field_sizes);
            if (status != SUCCESS)
            {
                IAS_LOG_ERROR("A problem was encountered building the "
                    "version 1 band metadata table description for file "
                    "'%s'", l1r_file->filename);
                return ERROR;
            }
        }
        else if (l1r_file->file_format_version == 2)
        {
            /* build the version 2 table description */
            status = build_v2_table_description(offsets, field_names,
                field_type, fields_to_close, field_sizes);
            if (status != SUCCESS)
            {
                IAS_LOG_ERROR("A problem was encountered building the "
                    "version 2 band metadata table description for file "
                    "'%s'", l1r_file->filename);
                return ERROR;
            }
        }
        else
        {
            /* build the current version table description */
            status = build_table_description(offsets, field_names,
                field_type, fields_to_close, field_sizes);
            if (status != SUCCESS)
            {
                IAS_LOG_ERROR("A problem was encountered building the "
                    "band metadata table description for file '%s'",
                    l1r_file->filename);
                return ERROR;
            }
        }

        for (record = 0; record < number_of_records; record++)
        {
            IAS_L1R_BAND_METADATA    metadata;
            BAND_METADATA_CACHE_NODE *current;

            if (l1r_file->file_format_version == 1)
            {
                IAS_L1R_V1_BAND_METADATA v1_metadata;

                /* get the current version 1 record. */
                memset(&v1_metadata, 0, sizeof(v1_metadata));
                hdf_status = H5TBread_records(l1r_file->file_id,
                    BAND_METADATA_TABLE, record, 1, sizeof(v1_metadata),
                    offsets, field_sizes, &v1_metadata);
                if (hdf_status < 0)
                {
                    IAS_LOG_ERROR("Reading version 1 band metadata table "
                        "for record %d in '%s'", (record + 1),
                        l1r_file->filename);
                    cleanup_table_description(fields_to_close);
                    return ERROR;
                }

                /* copy the version 1 information to the current version
                   data structure. */
                copy_v1_band_metadata(&v1_metadata, &metadata);
            }
            else if (l1r_file->file_format_version == 2)
            {
                IAS_L1R_V2_BAND_METADATA v2_metadata;

                /* get the current version 1 record. */
                memset(&v2_metadata, 0, sizeof(v2_metadata));
                hdf_status = H5TBread_records(l1r_file->file_id,
                    BAND_METADATA_TABLE, record, 1, sizeof(v2_metadata),
                    offsets, field_sizes, &v2_metadata);
                if (hdf_status < 0)
                {
                    IAS_LOG_ERROR("Reading version 2 band metadata table "
                        "for record %d in '%s'", (record + 1),
                        l1r_file->filename);
                    cleanup_table_description(fields_to_close);
                    return ERROR;
                }

                /* copy the version 2 information to the current version
                   data structure. */
                copy_v2_band_metadata(&v2_metadata, &metadata);
            }
            else
            {
                /* get the current version record */
                memset(&metadata, 0, sizeof(metadata));
                hdf_status = H5TBread_records(l1r_file->file_id,
                    BAND_METADATA_TABLE, record, 1, sizeof(metadata),
                    offsets, field_sizes, &metadata);
                if (hdf_status < 0)
                {
                    IAS_LOG_ERROR("Reading band metadata table for record %d "
                        "in '%s'", (record + 1), l1r_file->filename);
                    cleanup_table_description(fields_to_close);
                    return ERROR;
                }
            }

            /* as a consistency check, verify the band just read doesn't
               already exist in the band metadata list.  If it does, there
               is a bug writing the data to the file. */
            if (find_metadata_for_band(l1r_file, metadata.band_number)
                != NULL)
            {
                IAS_LOG_ERROR("Duplicate entries for band number %d found "
                    "in the band metadata in %s", metadata.band_number,
                    l1r_file->filename);
                cleanup_table_description(fields_to_close);
                return ERROR;
            }

            /* allocate space for the record */
            current = create_and_add_band_metadata_to_list(l1r_file);
            if (!current)
            {
                IAS_LOG_ERROR("Creating new band metadata node");
                cleanup_table_description(fields_to_close);
                return ERROR;
            }

            /* copy the metadata to the record */
            current->metadata = metadata;
        }

        /* clean up the table description */
        cleanup_table_description(fields_to_close);
    }

    /* remember the number of records in the file and that the band metadata
       is valid */
    l1r_file->band_metadata_valid = TRUE;
    l1r_file->band_metadata_records_in_file = number_of_records;

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
    L1RIO *l1r_file        /* I: file to write band metadata in */
)
{
    int status = 0;
    int record_index;
    BAND_METADATA_CACHE_NODE *current;

    /* arrays for holding the table description */
    size_t offsets[MAX_NFIELDS];
    const char *field_names[MAX_NFIELDS];
    hid_t field_type[MAX_NFIELDS];
    hid_t fields_to_close[MAX_NFIELDS];
    size_t field_sizes[MAX_NFIELDS];

    /* Build the table description for the current set of band metadata. */
    if (build_table_description(offsets, field_names, field_type,
                                fields_to_close, field_sizes) == ERROR)
    {
        IAS_LOG_ERROR("A problem was encountered building the band metadata"
                      " table description for file %s", l1r_file->filename);
        return ERROR;
    }

    /* loop through the list of metadata and write it to the table a record
       at a time. */
    record_index = 0;
    GET_OBJECT_FOR_EACH_ENTRY(current, &l1r_file->band_metadata,
                BAND_METADATA_CACHE_NODE, node)
    {
        /* if the table doesn't exist yet, create it when the first
           record is written */
        if (!l1r_file->band_metadata_in_file)
        {
            /* table isn't already in the file, so create it and add the first
               record to it */
            status = H5TBmake_table("Band Metadata", l1r_file->file_id,
                        BAND_METADATA_TABLE, NFIELDS, 1,
                        sizeof(IAS_L1R_BAND_METADATA), field_names, offsets, 
                        field_type, 10, NULL, 0, &current->metadata);
            if (status >= 0)
            {
                l1r_file->band_metadata_in_file = TRUE;
                l1r_file->band_metadata_records_in_file = 1;
            }
        }
        else if (record_index < l1r_file->band_metadata_records_in_file)
        {
            /* overwrite the initial records in the file */
            status = H5TBwrite_records(l1r_file->file_id, BAND_METADATA_TABLE,
                        record_index, 1, sizeof(IAS_L1R_BAND_METADATA),
                        offsets, field_sizes, &current->metadata);
        }
        else
        {
            /* append new records at the end */
            status = H5TBappend_records(l1r_file->file_id, BAND_METADATA_TABLE,
                     1, sizeof(IAS_L1R_BAND_METADATA), offsets, field_sizes,
                     &current->metadata);
            l1r_file->band_metadata_records_in_file++;
        }
        if (status < 0)
        {
            cleanup_table_description(fields_to_close);
            IAS_LOG_ERROR("Writing band metadata record %d to file %s",
                          record_index, l1r_file->filename);
            return ERROR;
        }
        record_index++;
    }

    cleanup_table_description(fields_to_close);

    return SUCCESS;
}

/******************************************************************************
Name: ias_l1r_get_band_metadata

Purpose: Retrieves the metadata for a requested band.

Returns:
    SUCCESS or ERROR
******************************************************************************/
int ias_l1r_get_band_metadata
(
    L1RIO *l1r_file,                /* I: file to read metadata from */
    int band_number,                /* I: band number to get */
    IAS_L1R_BAND_METADATA *metadata /* O: buffer to return data in */
)
{
    BAND_METADATA_CACHE_NODE *current;

    /* if metadata is not valid yet, read it from the file */
    if (!l1r_file->band_metadata_valid)
    {
        /* read the band metadata into the cache */
        if (read_band_metadata(l1r_file) != SUCCESS)
        {
            IAS_LOG_ERROR("Reading band metadata for %s", l1r_file->filename);
            return ERROR;
        }
    }

    /* search the metadata for the correct band */
    current = find_metadata_for_band(l1r_file, band_number);
    if (!current)
    {
        IAS_LOG_ERROR("Metadata not found for band %d in file %s",
            band_number, l1r_file->filename);
        return ERROR;
    }

    /* copy metadata for the entry found */
    memcpy(metadata, &current->metadata, sizeof(*metadata));

    return SUCCESS;
}

/******************************************************************************
Name: ias_l1r_set_band_metadata

Purpose: Writes the metadata provided to the file.

Returns:
    SUCCESS or ERROR
******************************************************************************/
int ias_l1r_set_band_metadata
(
    L1RIO *l1r_file,                 /* I: file to set metadata for */
    const IAS_L1R_BAND_METADATA *metadata, /* I: array of metadata to set */
    int metadata_size           /* I: number of entries in metadata array */
)
{
    int new_index;
    IAS_L1R_BAND_METADATA *destination_metadata;
    const IAS_L1R_BAND_METADATA *source_metadata;
    BAND_METADATA_CACHE_NODE *metadata_node;

    /* make sure the file is open in write or update mode before allowing
       metadata to be set */
    if (l1r_file->access_mode == IAS_READ)
    {
        IAS_LOG_ERROR("Attempting to write to file %s which is opened "
                      "for reading", l1r_file->filename);
        return ERROR;
    }

    /* if metadata is not valid yet, read it from the file */
    if (!l1r_file->band_metadata_valid)
    {
        /* read the band metadata into the cache */
        if (read_band_metadata(l1r_file) != SUCCESS)
        {
            IAS_LOG_ERROR("Reading band metadata from file %s",
                          l1r_file->filename);
            return ERROR;
        }
    }

    /* merge the new metadata into the existing metadata cache*/
    for (new_index = 0; new_index < metadata_size; new_index++)
    {
        /* check whether there is a band to replace */
        metadata_node = find_metadata_for_band(l1r_file, 
                                         metadata[new_index].band_number);

        /* if the band isn't already in the list, allocate a new node and
           add it to the list */
        if (!metadata_node)
        {
            metadata_node = create_and_add_band_metadata_to_list(l1r_file);
            if (!metadata_node)
            {
                IAS_LOG_ERROR("Adding band metadata to list");
                return ERROR;
            }
        }

        /* populate the new metadata.  This isn't simply done with a memcpy
           since some fields are only set if the input information is
           considered valid. */
        destination_metadata = &metadata_node->metadata;
        source_metadata = &metadata[new_index];
        memset(destination_metadata, 0, sizeof(*destination_metadata));

        destination_metadata->band_number = source_metadata->band_number;
        strcpy(destination_metadata->band_name, source_metadata->band_name);
        strcpy(destination_metadata->instrument_source, 
            source_metadata->instrument_source);
        strcpy(destination_metadata->discontinuity_factor_source,
            source_metadata->discontinuity_factor_source);
        destination_metadata->reflectance_to_radiance_coefficient = 
            source_metadata->reflectance_to_radiance_coefficient;
        strcpy(destination_metadata->bias_source,
            source_metadata->bias_source);
        destination_metadata->bias_temp_sensitivity_correction_applied = 
            source_metadata->bias_temp_sensitivity_correction_applied;
        destination_metadata->gain_temp_sensitivity_correction_applied = 
            source_metadata->gain_temp_sensitivity_correction_applied;
        strcpy(destination_metadata->linearization,
            source_metadata->linearization);
        destination_metadata->bias_correction_applied = source_metadata->
            bias_correction_applied;
    }

    /* write the new band metadata to the file */
    return write_band_metadata(l1r_file);
}
