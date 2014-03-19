#ifndef IAS_L1R_H
#define IAS_L1R_H

#include "ias_types.h" /* IAS_DATA_TYPE */
#include "ias_const.h" /* IAS-specific constants */

/* create a forward reference to the HDF5IO and HDF5_BAND_IO structures that
   are defined locally in the library routine since external users should not 
   need to know what is contained in the structure */
typedef struct L1RIO L1RIO;
typedef struct L1R_BAND_IO L1R_BAND_IO;


/****************************************************************************
Define the structures for holding file and band metadata
****************************************************************************/
#define IAS_L1R_SOURCE_SIZE 21
#define IAS_L1R_MAX_RAD_STEPS_SIZE 2048
#define IAS_L1R_BIAS_SOURCE_SIZE 26


typedef struct ias_l1r_file_metadata
{
    char software_version[IAS_SOFTWARE_VERSION_SIZE];  /* software version
                                                           generating L1R
                                                           product */
    char ingest_software_version[IAS_SOFTWARE_VERSION_SIZE]; /* ingest
                                                           software version */
    char collect_type[IAS_COLLECT_TYPE_SIZE];          /* L1R collection
                                                          type */
    int band_average_gain_applied;                     /* 0=FALSE, 1=TRUE */
    int relative_gain_applied;                         /* 0=FALSE, 1=TRUE */
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
} IAS_L1R_FILE_METADATA;


typedef struct ias_l1r_band_metadata
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
    char bias_source[IAS_L1R_BIAS_SOURCE_SIZE];  /* Source of bias */
    char linearization[50]; /* Field indicates the type of linearization
                               used on the data: "Standard" or "Alternate".
                               The alternate methods Lin or Nuc (or both)
                               could also be appented to the "Alternate"
                               string. */
    int bias_temp_sensitivity_correction_applied;      /* 0=FALSE, 1=TRUE */
    int gain_temp_sensitivity_correction_applied;      /* 0=FALSE, 1=TRUE */
    int bias_correction_applied;                       /* 0=FALSE, 1=TRUE */
} IAS_L1R_BAND_METADATA;


/****************************************************************************
Function prototypes
****************************************************************************/

L1RIO *ias_l1r_open_image
(
    const char *image_filename, /* I: Path & name of the 1R image file*/
    IAS_ACCESS_MODE access_mode /* I: Requested access mode of image */
);

int ias_l1r_close_image
(
    L1RIO *l1r      /* I: L1R IO structure */
);

L1R_BAND_IO *ias_l1r_open_band
(
    L1RIO *l1r,             /* I: L1R info structure for the image */
    int band_number,        /* I: band number to open */
    IAS_DATA_TYPE *data_type,/* I: data type for the imagery */
    int *number_of_scas,    /* I/O: SCAs to include in the band (1 if an
                                    SCA combined image is needed) */
    int *lines,             /* I/O: lines in the band (input if writing a new 
                                    band, output if opening an existing band) */
    int *samples            /* I/O: samples in the band (input if writing a new
                                    band, output if opening an existing band) */
);

int ias_l1r_close_band
(
    L1R_BAND_IO *l1r_band      /* I: L1R BAND IO structure */
);

int ias_l1r_read_image
(
    const L1R_BAND_IO *l1r_band,   /* I: L1R_BAND_IO structure */
    int sca_index,        /* I: SCA to write to (0-rel) */
    int line_start,       /* I: Line to start reading (0-rel) */
    int sample_start,     /* I: Sample to start reading (0-rel) */
    int lines,            /* I: Number of lines to read */
    int samples,          /* I: Number of samples to read */
    void *data            /* O: Data buffer */
);

int ias_l1r_write_image
(
    const L1R_BAND_IO *l1r_band,   /* I: L1R_BAND_IO structure */
    int sca_index,      /* I: SCA to write to (0-rel) */
    int line_start,     /* I: Line to start writing (0-rel) */
    int sample_start,   /* I: Sample to start writing (0-rel) */
    int lines,          /* I: Number of lines to write */
    int samples,        /* I: Number of samples to write */
    void *data          /* I: Data buffer */
);

int ias_l1r_get_file_metadata
(
    L1RIO *l1r,             /* I: L1R IO structure */
    IAS_L1R_FILE_METADATA *metadata /* O: output metadata */
);

int ias_l1r_set_file_metadata
(
    L1RIO *l1r,             /* I: L1R IO structure */
    const IAS_L1R_FILE_METADATA *metadata /* I: metadata to set */
);

int ias_l1r_get_band_metadata
(
    L1RIO *l1r,                     /* I: file to read metadata from */
    int band_number,                /* I: band number to get */
    IAS_L1R_BAND_METADATA *metadata /* O: buffer to return data in */
);

int ias_l1r_set_band_metadata
(
    L1RIO *l1r,                 /* I: file to set metadata for */
    const IAS_L1R_BAND_METADATA *metadata, /* I: array of metadata to set */
    int metadata_size           /* I: number of entries in metadata array */
);

int ias_l1r_get_band_count
(
    L1RIO *l1r              /* I: L1R info structure for the image */
);

int ias_l1r_is_band_present
(
    L1RIO *l1r,             /* I: L1R info structure for the image */
    int band_number         /* I: band number to check */
);

int ias_l1r_get_band_size
(
    L1RIO *l1r,             /* I: L1R info structure for the image */
    int band_number,        /* I: band number to get information for */
    IAS_DATA_TYPE *data_type,/* O: data type for the imagery */
    int *number_of_scas,    /* O: SCAs in the band */
    int *lines,             /* O: lines in the band */
    int *samples            /* O: samples in the band */
);

int ias_l1r_get_band_list
(
    L1RIO *l1g_file,            /* I: file to search for the band */
    int *band_number_list,      /* O: array of band numbers found in file */
    int band_number_list_size,  /* I: size of band_number_list array */
    int *number_of_bands        /* O: pointer to the number of bands returned
                                      in the band_number_list */
);

int ias_l1r_is_l1r_file
(
    const char *filename    /* I: HDF input file name */
);

#endif
