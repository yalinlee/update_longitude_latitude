/*******************************************************************************
Name: ias_l1r_set_file_metadata
      ias_l1r_get_file_metadata

Purpose: Provide routines to read and write L1R file level metadata

Note:    To provide backward compatibility, it is allowed to read older
         versions of the metadata.  The data is read from the older version
         and copied into the current version's data structure.  If the
         data stored in the old version doesn't translate directly into the
         new structure it's lost in the translation.  Most of the changes
         involve moving file metadata to band metadata, which can't be
         translated into per-band values reliably, and is also lost.

         Sections follow that contain the data structure; table definition;
         and routine to copy from old to new.  The final section contains the
         current version table and external get and set routines.

Routines;      cleanup_table_description       (internal)
               copy_v1_file_metadata           (internal)
               build_v1_table_description      (internal)
               copy_v2_file_metadata           (internal)
               build_v2_table_description      (internal)
               build_table_description         (internal)
               ias_l1r_get_file_metadata
               ias_l1r_set_file_metadata
******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include "ias_types.h"
#include "ias_const.h"
#include "ias_logging.h"
#include "ias_l1r.h"
#include "ias_l1r_private.h"
#include "hdf5_hl.h"  /* for H5TB functions */

/* Numer of fields in earlier versions of the band metadata structure */
#define V1_FILE_NFIELDS 18
#define V2_FILE_NFIELDS 17

/* Number of fields in the current band metadata information structure */
#define NFIELDS 12

/* Define the maximum number of fields -- this must be the largest of the
   previously defined NFIELDS values or bad things will happen */
#define MAX_NFIELDS V1_FILE_NFIELDS

/* Version 1 file and band metadata structure declarations */
typedef struct ias_l1r_v1_file_metadata
{
    char software_version[IAS_SOFTWARE_VERSION_SIZE];  /* software version
                                                           generating L1R
                                                           product */
    char ingest_software_version[IAS_SOFTWARE_VERSION_SIZE]; /* ingest
                                                           software version */
    char collect_type[IAS_COLLECT_TYPE_SIZE];          /* L1R collection
                                                          type */
    int bias_correction_applied;                       /* 0=FALSE, 1=TRUE */
    char bias_source[IAS_L1R_BIAS_SOURCE_SIZE];        /* Source of bias
                                                          data */
    int bias_temp_sensitivity_correction_applied;      /* 0=FALSE, 1=TRUE */
    int linearization_correction_applied;              /* 0=FALSE, 1=TRUE */
    int band_average_gain_applied;                     /* 0=FALSE, 1=TRUE */
    int relative_gain_applied;                         /* 0=FALSE, 1=TRUE */
    int gain_temp_sensitivity_correction_applied;      /* 0=FALSE, 1=TRUE */

    int sca_discontinuity_correction_applied;          /* 0=FALSE, 1=TRUE */
    char discontinuity_factor_source[IAS_L1R_SOURCE_SIZE];
                                             /* "CPF", "SCA_OVERLAP_CHAR" */
    int inoperable_detector_fill_applied;              /* 0=FALSE, 1=TRUE */
    int saturated_pixel_correction_applied;            /* 0=FALSE, 1=TRUE */
    int residual_striping_correction_applied;          /* 0=FALSE, 1=TRUE */
    int reflectance_conversion_applied;                /* 0=FALSE, 1=TRUE */
    double earth_sun_distance;                         /* Earth_sun_distance,
                                                          unit in AU */
    /* String to allow CVT to track "custom" radiometric
       processing steps.  The list is comma-delimited.  */
    char custom_rad_processing_steps_applied[IAS_L1R_MAX_RAD_STEPS_SIZE];
} IAS_L1R_V1_FILE_METADATA;


typedef struct ias_l1r_v2_file_metadata
{
    char software_version[IAS_SOFTWARE_VERSION_SIZE];  /* software version
                                                           generating L1R
                                                           product */
    char ingest_software_version[IAS_SOFTWARE_VERSION_SIZE]; /* ingest
                                                           software version */
    char collect_type[IAS_COLLECT_TYPE_SIZE];          /* L1R collection
                                                          type */
    int bias_correction_applied;                       /* 0=FALSE, 1=TRUE */
    char bias_source[IAS_L1R_BIAS_SOURCE_SIZE];        /* Source of bias
                                                          data */
    int bias_temp_sensitivity_correction_applied;      /* 0=FALSE, 1=TRUE */
    int linearization_correction_applied;              /* 0=FALSE, 1=TRUE */
    int band_average_gain_applied;                     /* 0=FALSE, 1=TRUE */
    int relative_gain_applied;                         /* 0=FALSE, 1=TRUE */
    int gain_temp_sensitivity_correction_applied;      /* 0=FALSE, 1=TRUE */
    int sca_discontinuity_correction_applied;          /* 0=FALSE, 1=TRUE */
    int inoperable_detector_fill_applied;              /* 0=FALSE, 1=TRUE */
    int saturated_pixel_correction_applied;            /* 0=FALSE, 1=TRUE */
    int residual_striping_correction_applied;          /* 0=FALSE, 1=TRUE */
    int reflectance_conversion_applied;                /* 0=FALSE, 1=TRUE */
    double earth_sun_distance;                         /* Earth_sun_distance,
                                                          unit in AU */
    /* String to allow CVT to track "custom" radiometric
       processing steps.  The list is comma-delimited.  */
    char custom_rad_processing_steps_applied[IAS_L1R_MAX_RAD_STEPS_SIZE];
} IAS_L1R_V2_FILE_METADATA;



/******************************************************************************
Name: cleanup_table_description

Purpose: Cleans up the resources allocated to the table description

Returns:
    nothing
******************************************************************************/
static void cleanup_table_description
(
    hid_t fields_to_close[MAX_NFIELDS] /* I: array of fields to close */
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
/*          This section contains Version 1 of the file metadata          */
/*------------------------------------------------------------------------*/
/*************************************************************************
 NAME:     copy_v1_file_metadata

 PURPOSE:  Copies version 1 file metadata to current format

 RETURNS:  Nothing

 NOTES:    As currently implemented, the version 1 SCA discontinuity
           correction factor source information is lost.
           Also lost due to Version 3 changes:
               bias_temp_sensitivity_correction_applied
               gain_temp_sensitivity_correction_applied
               linearization_correction_applied
**************************************************************************/
static void copy_v1_file_metadata
(
    const IAS_L1R_V1_FILE_METADATA *v1_fmd, /* I: version 1 metadata */
    IAS_L1R_FILE_METADATA *fmd              /* O: metadata in current format */
)
{
    /* Copy the existing information in the version 1 structure to the
       current structure */
    memset(fmd, 0, sizeof(*fmd));

    strcpy(fmd->software_version, v1_fmd->software_version);
    strcpy(fmd->ingest_software_version, v1_fmd->ingest_software_version);
    strcpy(fmd->collect_type, v1_fmd->collect_type);

    fmd->band_average_gain_applied = v1_fmd->band_average_gain_applied;
    fmd->relative_gain_applied = v1_fmd->relative_gain_applied;

    fmd->sca_discontinuity_correction_applied =
        v1_fmd->sca_discontinuity_correction_applied;
    fmd->inoperable_detector_fill_applied =
        v1_fmd->inoperable_detector_fill_applied;
    fmd->saturated_pixel_correction_applied =
        v1_fmd->saturated_pixel_correction_applied;
    fmd->residual_striping_correction_applied =
        v1_fmd->residual_striping_correction_applied;
    fmd->reflectance_conversion_applied =
        v1_fmd->reflectance_conversion_applied;
    fmd->earth_sun_distance = v1_fmd->earth_sun_distance;
    strcpy(fmd->custom_rad_processing_steps_applied,
        v1_fmd->custom_rad_processing_steps_applied);
}


/*************************************************************************
 NAME:      build_v1_table_description

 PURPOSE:   Internal routine to build the version 1 L1R file metadata
            table description

 RETURNS:   Integer status code of SUCCESS or ERROR
**************************************************************************/
static int build_v1_table_description
(
    size_t offsets[MAX_NFIELDS],         /* O: array of offsets in the
                                            structure */
    const char *field_names[MAX_NFIELDS],/* O: array of pointers to field
                                            names */
    hid_t field_type[MAX_NFIELDS],       /* O: array of field types */
    hid_t fields_to_close[MAX_NFIELDS],  /* O: array of fields to close */
    size_t field_size[MAX_NFIELDS]       /* O: array of field sizes */
)
{
    int i;
    int error = 0;
    IAS_L1R_V1_FILE_METADATA metadata; /* for field sizes */

    /* initialize the list of fields to close */
    for (i = 0; i < MAX_NFIELDS; i++)
        fields_to_close[i] = -1;

    /* define the table contents.  For each field in the table, define the
       offset to the data in the structure, the field name, data type, and 
       field size.  For fields that need to be properly sized, H5Tset_size
       is called with the correct size and the created type is added to the
       list of fields that need to be closed. */
    i = 0;

    offsets[i] = HOFFSET(IAS_L1R_V1_FILE_METADATA, software_version);
    field_names[i] = "Software Version Generating L1R Product";
    field_type[i] = H5Tcopy(H5T_C_S1);
    if (H5Tset_size(field_type[i], sizeof(metadata.software_version)) < 0)
    {
        IAS_LOG_ERROR("Setting size of Software Version field");
        error = 1;
    }
    fields_to_close[i] = field_type[i];
    field_size[i] = sizeof(metadata.software_version);

    offsets[++i] = HOFFSET(IAS_L1R_V1_FILE_METADATA, ingest_software_version);
    field_names[i] = "Ingest Software Version";
    field_type[i] = H5Tcopy(H5T_C_S1);
    if (H5Tset_size(field_type[i], 
            sizeof(metadata.ingest_software_version)) < 0)
    {
        IAS_LOG_ERROR("Setting size of Ingest Software Version field");
        error = 1;
    }
    fields_to_close[i] = field_type[i];
    field_size[i] = sizeof(metadata.ingest_software_version);

    offsets[++i] = HOFFSET(IAS_L1R_V1_FILE_METADATA, collect_type);
    field_names[i] = "Collection Type";
    field_type[i] = H5Tcopy(H5T_C_S1);
    if (H5Tset_size(field_type[i], sizeof(metadata.collect_type)) < 0)
    {
        IAS_LOG_ERROR("Setting size of Ingest Collection Type field");
        error = 1;
    }
    fields_to_close[i] = field_type[i];
    field_size[i] = sizeof(metadata.collect_type);

    offsets[++i] = HOFFSET(IAS_L1R_V1_FILE_METADATA,
                           bias_correction_applied);
    field_names[i] = "Bias Correction Applied Flag";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(metadata.bias_correction_applied);

    offsets[++i] = HOFFSET(IAS_L1R_V1_FILE_METADATA, bias_source);
    field_names[i] = "Bias Source";
    field_type[i] = H5Tcopy(H5T_C_S1);
    if (H5Tset_size(field_type[i], sizeof(metadata.bias_source)) < 0)
    {
        IAS_LOG_ERROR("Setting size of Bias Source field");
        error = 1;
    }
    fields_to_close[i] = field_type[i];
    field_size[i] = sizeof(metadata.bias_source);

    offsets[++i] = HOFFSET(IAS_L1R_V1_FILE_METADATA,
                           bias_temp_sensitivity_correction_applied);
    field_names[i] = "Bias Temperature Sensitivity Correction Applied Flag";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(metadata.bias_temp_sensitivity_correction_applied);

    offsets[++i] = HOFFSET(IAS_L1R_V1_FILE_METADATA, 
                           linearization_correction_applied);
    field_names[i] = "Linearization Correction Applied Flag";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(metadata.linearization_correction_applied);

    offsets[++i] = HOFFSET(IAS_L1R_V1_FILE_METADATA, band_average_gain_applied);
    field_names[i] = "Band Average Gain Applied Flag";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(metadata.band_average_gain_applied);

    offsets[++i] = HOFFSET(IAS_L1R_V1_FILE_METADATA, relative_gain_applied);
    field_names[i] = "Relative Gain Applied Flag";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(metadata.relative_gain_applied);

    offsets[++i] = HOFFSET(IAS_L1R_V1_FILE_METADATA, 
                           gain_temp_sensitivity_correction_applied);
    field_names[i] = "Gain Temperature Sensitivity Correction Applied Flag";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(metadata.gain_temp_sensitivity_correction_applied);

    offsets[++i] = HOFFSET(IAS_L1R_V1_FILE_METADATA, 
                           sca_discontinuity_correction_applied);
    field_names[i] = "SCA Discontinuity Correction Applied Flag";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(metadata.sca_discontinuity_correction_applied);

    offsets[++i] = HOFFSET(IAS_L1R_V1_FILE_METADATA,
                           discontinuity_factor_source);
    field_names[i] = "Discontinuity Factor Source";
    field_type[i] = H5Tcopy(H5T_C_S1);
    if (H5Tset_size(field_type[i], 
            sizeof(metadata.discontinuity_factor_source)) < 0)
    {
        IAS_LOG_ERROR("Setting size of Discontinuity Factor Source field");
        error = 1;
    }
    fields_to_close[i] = field_type[i];
    field_size[i] = sizeof(metadata.discontinuity_factor_source);

    offsets[++i] = HOFFSET(IAS_L1R_V1_FILE_METADATA, 
        inoperable_detector_fill_applied);
    field_names[i] = "Inoperable Detector Fill Applied Flag";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(metadata.inoperable_detector_fill_applied);

    offsets[++i] = HOFFSET(IAS_L1R_V1_FILE_METADATA, 
        saturated_pixel_correction_applied);
    field_names[i] = "Saturated Pixel Correction Applied Flag";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(metadata.saturated_pixel_correction_applied);

    offsets[++i] = HOFFSET(IAS_L1R_V1_FILE_METADATA, 
        residual_striping_correction_applied);
    field_names[i] = "Residual Striping Correction Applied Flag";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(metadata.residual_striping_correction_applied);

    offsets[++i] = HOFFSET(IAS_L1R_V1_FILE_METADATA, 
        reflectance_conversion_applied);
    field_names[i] = "Reflectance Conversion Applied Flag";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(metadata.reflectance_conversion_applied);

    offsets[++i] = HOFFSET(IAS_L1R_V1_FILE_METADATA, 
        earth_sun_distance);
    field_names[i] = "Earth Sun Distance Calculated";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_size[i] = sizeof(metadata.earth_sun_distance);

    offsets[++i] = HOFFSET(IAS_L1R_V1_FILE_METADATA, 
        custom_rad_processing_steps_applied);
    field_names[i] = "Custom Radiometric Processing Steps Applied";
    field_type[i] = H5Tcopy(H5T_C_S1);
    if (H5Tset_size(field_type[i], 
        sizeof(metadata.custom_rad_processing_steps_applied)) < 0)
    {
        IAS_LOG_ERROR("Setting size of Custom Radiometric Processing Steps"
            " Applied field");
        error = 1;
    }
    fields_to_close[i] = field_type[i];
    field_size[i] = sizeof(metadata.custom_rad_processing_steps_applied);

    if (error)
    {
        /* an error occurred generating the table */
        IAS_LOG_ERROR("Building file metadata table definition");
        cleanup_table_description(fields_to_close);
        return ERROR;
    }
    if (i < (V1_FILE_NFIELDS - 1))
    {
        /* fewer fields were added than expected */
        IAS_LOG_ERROR("Defined %d fields, but expected %d", i + 1,
            V1_FILE_NFIELDS);
        cleanup_table_description(fields_to_close);
        return ERROR;
    }
    else if (i >= V1_FILE_NFIELDS)
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


/*------------------------------------------------------------------------*/
/*          This section contains Version 2 of the file metadata          */
/*------------------------------------------------------------------------*/
/*************************************************************************
 NAME:     copy_v2_file_metadata

 PURPOSE:  Copies version 2 file metadata to current format

 RETURNS:  Nothing

 NOTE:     The following fields are lost in the conversion:
                bias_temp_sensitivity_correction_applied
                gain_temp_sensitivity_correction_applied
                linearization_correction_applied
**************************************************************************/
static void copy_v2_file_metadata
(
    const IAS_L1R_V2_FILE_METADATA *v2_fmd, /* I: version 2 metadata */
    IAS_L1R_FILE_METADATA *fmd              /* O: metadata in current format */
)
{
    /* Copy the existing information in the version 1 structure to the
       current structure */
    memset(fmd, 0, sizeof(*fmd));

    strcpy(fmd->software_version, v2_fmd->software_version);
    strcpy(fmd->ingest_software_version, v2_fmd->ingest_software_version);
    strcpy(fmd->collect_type, v2_fmd->collect_type);

    fmd->relative_gain_applied = v2_fmd->relative_gain_applied;

    fmd->sca_discontinuity_correction_applied =
        v2_fmd->sca_discontinuity_correction_applied;
    fmd->inoperable_detector_fill_applied =
        v2_fmd->inoperable_detector_fill_applied;
    fmd->saturated_pixel_correction_applied =
        v2_fmd->saturated_pixel_correction_applied;
    fmd->residual_striping_correction_applied =
        v2_fmd->residual_striping_correction_applied;
    fmd->reflectance_conversion_applied =
        v2_fmd->reflectance_conversion_applied;
    fmd->earth_sun_distance = v2_fmd->earth_sun_distance;
    strcpy(fmd->custom_rad_processing_steps_applied,
        v2_fmd->custom_rad_processing_steps_applied);
}


/******************************************************************************
Name:    build_v2_table_description

Purpose: Builds the table description for version 2 of the file metadata table

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
    size_t field_size[MAX_NFIELDS]       /* O: array of field sizes */
)
{
    int i;
    int error = 0;
    IAS_L1R_V2_FILE_METADATA metadata; /* for field sizes */

    /* initialize the list of fields to close */
    for (i = 0; i < MAX_NFIELDS; i++)
        fields_to_close[i] = -1;

    /* define the table contents.  For each field in the table, define the
       offset to the data in the structure, the field name, data type, and 
       field size.  For fields that need to be properly sized, H5Tset_size
       is called with the correct size and the created type is added to the
       list of fields that need to be closed. */
    i = 0;

    offsets[i] = HOFFSET(IAS_L1R_V2_FILE_METADATA, software_version);
    field_names[i] = "Software Version Generating L1R Product";
    field_type[i] = H5Tcopy(H5T_C_S1);
    if (H5Tset_size(field_type[i], sizeof(metadata.software_version)) < 0)
    {
        IAS_LOG_ERROR("Setting size of Software Version field");
        error = 1;
    }
    fields_to_close[i] = field_type[i];
    field_size[i] = sizeof(metadata.software_version);

    offsets[++i] = HOFFSET(IAS_L1R_V2_FILE_METADATA, ingest_software_version);
    field_names[i] = "Ingest Software Version";
    field_type[i] = H5Tcopy(H5T_C_S1);
    if (H5Tset_size(field_type[i], 
            sizeof(metadata.ingest_software_version)) < 0)
    {
        IAS_LOG_ERROR("Setting size of Ingest Software Version field");
        error = 1;
    }
    fields_to_close[i] = field_type[i];
    field_size[i] = sizeof(metadata.ingest_software_version);

    offsets[++i] = HOFFSET(IAS_L1R_V2_FILE_METADATA, collect_type);
    field_names[i] = "Collection Type";
    field_type[i] = H5Tcopy(H5T_C_S1);
    if (H5Tset_size(field_type[i], sizeof(metadata.collect_type)) < 0)
    {
        IAS_LOG_ERROR("Setting size of Ingest Collection Type field");
        error = 1;
    }
    fields_to_close[i] = field_type[i];
    field_size[i] = sizeof(metadata.collect_type);

    offsets[++i] = HOFFSET(IAS_L1R_V2_FILE_METADATA,
                           bias_correction_applied);
    field_names[i] = "Bias Correction Applied Flag";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(metadata.bias_correction_applied);

    offsets[++i] = HOFFSET(IAS_L1R_V2_FILE_METADATA, bias_source);
    field_names[i] = "Bias Source";
    field_type[i] = H5Tcopy(H5T_C_S1);
    if (H5Tset_size(field_type[i], sizeof(metadata.bias_source)) < 0)
    {
        IAS_LOG_ERROR("Setting size of Bias Source field");
        error = 1;
    }
    fields_to_close[i] = field_type[i];
    field_size[i] = sizeof(metadata.bias_source);

    offsets[++i] = HOFFSET(IAS_L1R_V2_FILE_METADATA,
                           bias_temp_sensitivity_correction_applied);
    field_names[i] = "Bias Temperature Sensitivity Correction Applied Flag";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(metadata.bias_temp_sensitivity_correction_applied);

    offsets[++i] = HOFFSET(IAS_L1R_V2_FILE_METADATA, 
                           linearization_correction_applied);
    field_names[i] = "Linearization Correction Applied Flag";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(metadata.linearization_correction_applied);

    offsets[++i] = HOFFSET(IAS_L1R_V2_FILE_METADATA, band_average_gain_applied);
    field_names[i] = "Band Average Gain Applied Flag";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(metadata.band_average_gain_applied);

    offsets[++i] = HOFFSET(IAS_L1R_V2_FILE_METADATA, relative_gain_applied);
    field_names[i] = "Relative Gain Applied Flag";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(metadata.relative_gain_applied);

    offsets[++i] = HOFFSET(IAS_L1R_V2_FILE_METADATA, 
                           gain_temp_sensitivity_correction_applied);
    field_names[i] = "Gain Temperature Sensitivity Correction Applied Flag";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(metadata.gain_temp_sensitivity_correction_applied);

    offsets[++i] = HOFFSET(IAS_L1R_V2_FILE_METADATA, 
                           sca_discontinuity_correction_applied);
    field_names[i] = "SCA Discontinuity Correction Applied Flag";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(metadata.sca_discontinuity_correction_applied);

    offsets[++i] = HOFFSET(IAS_L1R_V2_FILE_METADATA, 
                           inoperable_detector_fill_applied);
    field_names[i] = "Inoperable Detector Fill Applied Flag";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(metadata.inoperable_detector_fill_applied);

    offsets[++i] = HOFFSET(IAS_L1R_V2_FILE_METADATA, 
                           saturated_pixel_correction_applied);
    field_names[i] = "Saturated Pixel Correction Applied Flag";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(metadata.saturated_pixel_correction_applied);

    offsets[++i] = HOFFSET(IAS_L1R_V2_FILE_METADATA, 
                           residual_striping_correction_applied);
    field_names[i] = "Residual Striping Correction Applied Flag";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(metadata.residual_striping_correction_applied);

    offsets[++i] = HOFFSET(IAS_L1R_V2_FILE_METADATA, 
                           reflectance_conversion_applied);
    field_names[i] = "Reflectance Conversion Applied Flag";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(metadata.reflectance_conversion_applied);

    offsets[++i] = HOFFSET(IAS_L1R_V2_FILE_METADATA, 
                           earth_sun_distance);
    field_names[i] = "Earth Sun Distance Calculated";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_size[i] = sizeof(metadata.earth_sun_distance);

    offsets[++i] = HOFFSET(IAS_L1R_V2_FILE_METADATA, 
                           custom_rad_processing_steps_applied);
    field_names[i] = "Custom Radiometric Processing Steps Applied";
    field_type[i] = H5Tcopy(H5T_C_S1);
    if (H5Tset_size(field_type[i], 
                    sizeof(metadata.custom_rad_processing_steps_applied)) < 0)
    {
        IAS_LOG_ERROR("Setting size of Custom Radiometric Processing Steps"
                      " Applied field");
        error = 1;
    }
    fields_to_close[i] = field_type[i];
    field_size[i] = sizeof(metadata.custom_rad_processing_steps_applied);

    if (error)
    {
        /* an error occurred generating the table */
        IAS_LOG_ERROR("Building file metadata table definition");
        cleanup_table_description(fields_to_close);
        return ERROR;
    }
    if (i < (V2_FILE_NFIELDS - 1))
    {
        /* fewer fields were added than expected */
        IAS_LOG_ERROR("Defined %d fields, but expected %d", i + 1,
            NFIELDS);
        cleanup_table_description(fields_to_close);
        return ERROR;
    }
    else if (i >= V2_FILE_NFIELDS)
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

Purpose: Builds the table description for the file metadata table.

Returns:
    SUCCESS or ERROR
******************************************************************************/
static int build_table_description
(
    size_t offsets[MAX_NFIELDS],         /* O: array of offsets in the
                                            structure */
    const char *field_names[MAX_NFIELDS],/* O: array of pointers to field
                                            names */
    hid_t field_type[MAX_NFIELDS],       /* O: array of field types */
    hid_t fields_to_close[MAX_NFIELDS],  /* O: array of fields to close */
    size_t field_size[MAX_NFIELDS]       /* O: array of field sizes */
)
{
    int i;
    int error = 0;
    IAS_L1R_FILE_METADATA metadata; /* for field sizes */

    /* initialize the list of fields to close */
    for (i = 0; i < MAX_NFIELDS; i++)
        fields_to_close[i] = -1;

    /* define the table contents.  For each field in the table, define the
       offset to the data in the structure, the field name, data type, and 
       field size.  For fields that need to be properly sized, H5Tset_size
       is called with the correct size and the created type is added to the
       list of fields that need to be closed. */
    i = 0;

    offsets[i] = HOFFSET(IAS_L1R_FILE_METADATA, software_version);
    field_names[i] = "Software Version Generating L1R Product";
    field_type[i] = H5Tcopy(H5T_C_S1);
    if (H5Tset_size(field_type[i], sizeof(metadata.software_version)) < 0)
    {
        IAS_LOG_ERROR("Setting size of Software Version field");
        error = 1;
    }
    fields_to_close[i] = field_type[i];
    field_size[i] = sizeof(metadata.software_version);

    offsets[++i] = HOFFSET(IAS_L1R_FILE_METADATA, ingest_software_version);
    field_names[i] = "Ingest Software Version";
    field_type[i] = H5Tcopy(H5T_C_S1);
    if (H5Tset_size(field_type[i], 
            sizeof(metadata.ingest_software_version)) < 0)
    {
        IAS_LOG_ERROR("Setting size of Ingest Software Version field");
        error = 1;
    }
    fields_to_close[i] = field_type[i];
    field_size[i] = sizeof(metadata.ingest_software_version);

    offsets[++i] = HOFFSET(IAS_L1R_FILE_METADATA, collect_type);
    field_names[i] = "Collection Type";
    field_type[i] = H5Tcopy(H5T_C_S1);
    if (H5Tset_size(field_type[i], sizeof(metadata.collect_type)) < 0)
    {
        IAS_LOG_ERROR("Setting size of Ingest Collection Type field");
        error = 1;
    }
    fields_to_close[i] = field_type[i];
    field_size[i] = sizeof(metadata.collect_type);

    offsets[++i] = HOFFSET(IAS_L1R_FILE_METADATA, band_average_gain_applied);
    field_names[i] = "Band Average Gain Applied Flag";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(metadata.band_average_gain_applied);

    offsets[++i] = HOFFSET(IAS_L1R_FILE_METADATA, relative_gain_applied);
    field_names[i] = "Relative Gain Applied Flag";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(metadata.relative_gain_applied);

    offsets[++i] = HOFFSET(IAS_L1R_FILE_METADATA, 
                           sca_discontinuity_correction_applied);
    field_names[i] = "SCA Discontinuity Correction Applied Flag";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(metadata.sca_discontinuity_correction_applied);

    offsets[++i] = HOFFSET(IAS_L1R_FILE_METADATA, 
                           inoperable_detector_fill_applied);
    field_names[i] = "Inoperable Detector Fill Applied Flag";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(metadata.inoperable_detector_fill_applied);

    offsets[++i] = HOFFSET(IAS_L1R_FILE_METADATA, 
                           saturated_pixel_correction_applied);
    field_names[i] = "Saturated Pixel Correction Applied Flag";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(metadata.saturated_pixel_correction_applied);

    offsets[++i] = HOFFSET(IAS_L1R_FILE_METADATA, 
                           residual_striping_correction_applied);
    field_names[i] = "Residual Striping Correction Applied Flag";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(metadata.residual_striping_correction_applied);

    offsets[++i] = HOFFSET(IAS_L1R_FILE_METADATA, 
                           reflectance_conversion_applied);
    field_names[i] = "Reflectance Conversion Applied Flag";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(metadata.reflectance_conversion_applied);

    offsets[++i] = HOFFSET(IAS_L1R_FILE_METADATA, 
                           earth_sun_distance);
    field_names[i] = "Earth Sun Distance Calculated";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_size[i] = sizeof(metadata.earth_sun_distance);

    offsets[++i] = HOFFSET(IAS_L1R_FILE_METADATA, 
                           custom_rad_processing_steps_applied);
    field_names[i] = "Custom Radiometric Processing Steps Applied";
    field_type[i] = H5Tcopy(H5T_C_S1);
    if (H5Tset_size(field_type[i], 
                    sizeof(metadata.custom_rad_processing_steps_applied)) < 0)
    {
        IAS_LOG_ERROR("Setting size of Custom Radiometric Processing Steps"
                      " Applied field");
        error = 1;
    }
    fields_to_close[i] = field_type[i];
    field_size[i] = sizeof(metadata.custom_rad_processing_steps_applied);

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
Name: ias_l1r_set_file_metadata

Purpose: Sets the file level metadata in a file that is opened in write or
    update mode.

Returns:
    SUCCESS or ERROR
******************************************************************************/
int ias_l1r_set_file_metadata
(
    L1RIO *l1r,             /* I: L1R IO structure to access */
    const IAS_L1R_FILE_METADATA *metadata /* I: metadata to set */
)
{
    int status;

    /* arrays to hold the description of every field in the file metadata 
       table */
    size_t offsets[MAX_NFIELDS];
    const char *field_names[MAX_NFIELDS];
    hid_t field_type[MAX_NFIELDS];
    hid_t fields_to_close[MAX_NFIELDS];
    size_t field_size[MAX_NFIELDS];

    /* verify the input parameter is valid */
    if (!l1r)
    {
        IAS_LOG_ERROR("NULL pointer provided for the L1R file");
        return ERROR;
    }

    /* return an error if file is opened read only */
    if (l1r->access_mode == IAS_READ)
    {
        IAS_LOG_ERROR("Attempting to write to file %s which is opened "
                      "for reading", l1r->filename);
        return ERROR;
    }

    if (build_table_description(offsets, field_names, field_type,
                fields_to_close, field_size) == ERROR)
    {
        IAS_LOG_ERROR("A problem was encountered building the file metadata "
                      "table description for file %s", l1r->filename);
        return ERROR;
    }

    if (!l1r->file_metadata_in_file)
    {
        /* create the table since it doesn't exist yet */
        status = H5TBmake_table("File Metadata", l1r->file_id, 
                                FILE_METADATA_TABLE, NFIELDS, 1, 
                                sizeof(*metadata), field_names, offsets, 
                                field_type, 1, NULL, 0, metadata);
        l1r->file_metadata_in_file = TRUE;
    }
    else
    {
        /* table exists, so overwrite the first record */
        status = H5TBwrite_records(l1r->file_id, FILE_METADATA_TABLE, 0,
                                   1, sizeof(*metadata), offsets, field_size,
                                   metadata);
    }

    cleanup_table_description(fields_to_close);

    if (status < 0)
    {
        IAS_LOG_ERROR("Writing file metadata table to file %s",
                      l1r->filename);
        return ERROR;
    }   

    return SUCCESS;
}



/******************************************************************************
Name: ias_l1r_get_file_metadata

Purpose: Gets the file level metadata from an image file.

Returns:
    SUCCESS or ERROR
******************************************************************************/
int ias_l1r_get_file_metadata
(
    L1RIO *l1r_file,                /* I: L1R IO structure for file */
    IAS_L1R_FILE_METADATA *metadata /* O: output metadata */
)
{
    const char *field_names[MAX_NFIELDS];
    size_t offsets[MAX_NFIELDS];
    size_t field_size[MAX_NFIELDS];
    hid_t fields_to_close[MAX_NFIELDS];
    hid_t field_type[MAX_NFIELDS];
    hsize_t number_of_fields;
    hsize_t number_of_records;
    herr_t hdf_status;
    int number_of_fields_in_this_version;
    int status;

    /* Initialize the metadata structure */
    memset(metadata, 0, sizeof(IAS_L1R_FILE_METADATA));

    /* verify the input parameter is valid */
    if (!l1r_file)
    {
        IAS_LOG_ERROR("NULL pointer provided for the L1R file");
        return ERROR;
    }

    /* make sure there's metadata info available */
    if (!l1r_file->file_metadata_in_file)
    {
        /* the file metadata is not available yet, so it is an error */
        IAS_LOG_ERROR("Attempted to read file metadata from file '%s' "
            "before it has been set", l1r_file->filename);
        return ERROR;
    }

    /* Get the file metadata table information */
    hdf_status = H5TBget_table_info(l1r_file->file_id, FILE_METADATA_TABLE,
        &number_of_fields, &number_of_records);
    if (hdf_status < 0)
    {
        IAS_LOG_ERROR("Could not get grid table information:  '%s'",
            FILE_METADATA_TABLE);
        return ERROR;
    }

    /* there should be only one file metadata record in the table. */
    if (number_of_records != 1)
    {
        IAS_LOG_ERROR("There should only be one file metadata record in "
            "'%s', but H5TBget_info returned %d records",
            l1r_file->filename, (int)number_of_records);
        return ERROR;
    }

    switch ( l1r_file->file_format_version )
    {
        case 1 :
            number_of_fields_in_this_version = V1_FILE_NFIELDS;
            break;
        case 2 :
            number_of_fields_in_this_version = V2_FILE_NFIELDS;
            break;
        default :
            number_of_fields_in_this_version = NFIELDS;
            break;
    }

    /* make sure we've got the right number of fields in the table. */
    if (number_of_fields != number_of_fields_in_this_version)
    {
        IAS_LOG_ERROR("Version %d of table '%s' should have %d fields, but "
            "H5TBget_info returned %d fields", l1r_file->file_format_version,
            FILE_METADATA_TABLE, number_of_fields_in_this_version,
            (int)number_of_fields);
        return ERROR;
    }

    /* Build the correct table description based on the file format version
       number */
    if (l1r_file->file_format_version == 1)
    {
        IAS_L1R_V1_FILE_METADATA v1_metadata;
 
        /* build the version 1 table description. */
        status = build_v1_table_description(offsets, field_names,
            field_type, fields_to_close, field_size);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Building version 1 file metadata table "
                "description");
            return ERROR;
        }

        /* read the metadata from the table and populate the version 1
           data structure. */
        memset(&v1_metadata, 0, sizeof(IAS_L1R_V1_FILE_METADATA));
        hdf_status = H5TBread_table(l1r_file->file_id, FILE_METADATA_TABLE,
            sizeof(v1_metadata), offsets, field_size, &v1_metadata);

        /* cleanup the table description */
        cleanup_table_description(fields_to_close);

        /* check the status of the table read */
        if (hdf_status < 0)
        {
            IAS_LOG_ERROR("Reading version 1 file metadata in '%s'",
                l1r_file->filename);
            return ERROR;
        }

        /* copy the version 1 metadata to the current version data
           structure.  */
        copy_v1_file_metadata(&v1_metadata, metadata);
    }
    else if (l1r_file->file_format_version == 2)
    {
        IAS_L1R_V2_FILE_METADATA v2_metadata;
 
        /* build the version 1 table description. */
        status = build_v2_table_description(offsets, field_names,
            field_type, fields_to_close, field_size);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Building version 2 file metadata table "
                "description");
            return ERROR;
        }

        /* read the metadata from the table and populate the version 2
           data structure. */
        memset(&v2_metadata, 0, sizeof(IAS_L1R_V2_FILE_METADATA));
        hdf_status = H5TBread_table(l1r_file->file_id, FILE_METADATA_TABLE,
            sizeof(v2_metadata), offsets, field_size, &v2_metadata);

        /* cleanup the table description */
        cleanup_table_description(fields_to_close);

        /* check the status of the table read */
        if (hdf_status < 0)
        {
            IAS_LOG_ERROR("Reading version 2 file metadata in '%s'",
                l1r_file->filename);
            return ERROR;
        }

        /* copy the version 2 metadata to the current version data
           structure.  */
        copy_v2_file_metadata(&v2_metadata, metadata);
    }
    else
    {
        /* build the table description. */
        status = build_table_description(offsets, field_names,
            field_type, fields_to_close, field_size);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Building file metadata table description");
            return ERROR;
        }

        /* read the table and populate the metadata structure. */
        memset(metadata, 0, sizeof(*metadata));
        hdf_status = H5TBread_table(l1r_file->file_id, FILE_METADATA_TABLE,
            sizeof(*metadata), offsets, field_size, metadata);

        /* clean up the table description */
        cleanup_table_description(fields_to_close);

        /* check the status of the table read */
        if (status < 0)
        {
            IAS_LOG_ERROR("Reading file metadata in '%s'",
                l1r_file->filename);
            return ERROR;
        }
    }

    return SUCCESS;
}
