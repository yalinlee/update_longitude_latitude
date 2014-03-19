#ifndef IAS_L1G_H
#define IAS_L1G_H

#include "ias_types.h" /* IAS_DATA_TYPE */
#include "ias_const.h" /* IAS-specific constants */

/* create a forward reference to the HDF5IO and HDF5_BAND_IO structures that 
   are defined locally in the library routine since external users should not 
   need to know what is contained in the structure */
typedef struct L1GIO L1GIO;
typedef struct L1G_BAND_IO L1G_BAND_IO;


/****************************************************************************
Define the structures for holding file and band metadata
****************************************************************************/
#define IAS_L1G_SPACECRAFT_SIZE 32
#define IAS_L1G_CAPTURE_DIRECTION_SIZE 32
#define IAS_L1G_CAPTURE_DATE_SIZE 11
#define IAS_L1G_CAPTURE_TIME_SIZE 9
#define IAS_L1G_CORRECTION_TYPE_SIZE 5
#define IAS_L1G_RESAMPLE_TYPE_SIZE 4

typedef struct ias_l1g_file_metadata
{
    int projection_code;        /* GCTP projection code for the image */
    int zone_code;              /* zone code for the UTM projection */
    char datum[IAS_DATUM_SIZE]; /* datum (usually WGS84) */
    int spheroid_code;          /* GCTP spheroid code (usually 12 for WGS84) */
    char projection_units[IAS_UNITS_SIZE];
                        /* projection units (usually "METERS" or "DEGREES")  */
    double projection_parameters[IAS_PROJ_PARAM_SIZE];
    int wrs_path;
    int wrs_row;
    double roll_angle;          /* roll angle for off-nadir collections.  A
                                   positive angle represents a roll to the
                                   right in the along track direction.  For
                                   descending collections, that is to the
                                   left in the imagery. */
    char spacecraft[IAS_L1G_SPACECRAFT_SIZE]; /* spacecraft name */
    char collection_type[IAS_COLLECT_TYPE_SIZE];
                                /* typically EARTH, LUNAR, or STELLAR */
    char capture_direction[IAS_L1G_CAPTURE_DIRECTION_SIZE];
                                /* ASCENDING or DESCENDING */
    char capture_date[IAS_L1G_CAPTURE_DATE_SIZE]; /* YYYY/MM/DD format */
    char capture_time[IAS_L1G_CAPTURE_TIME_SIZE]; /* HH:MM:SS format */
    char correction_type[IAS_L1G_CORRECTION_TYPE_SIZE]; /* L1G/L1T/L1Gt */
    char resample_type[IAS_L1G_RESAMPLE_TYPE_SIZE]; /* BI/NN/CC/MTF */
    char software_version[IAS_SOFTWARE_VERSION_SIZE];
                                /* software version used to create the image */
    char ingest_software_version[IAS_SOFTWARE_VERSION_SIZE];
                                /* ingest software version */
    double sun_azimuth;         /* sun azimuth angle at the scene center */
    double sun_elevation;       /* sun elevation angle at the scene center */
    int sun_angles_valid;       /* 1 for sun data loaded and 0 if not */
} IAS_L1G_FILE_METADATA;

typedef struct ias_l1g_band_metadata
{
    int band_number;
    char band_name[IAS_BAND_NAME_SIZE];/* user readable band name.  For 
                                          example, might be "RED" or "GREEN" */
    double upper_left_y;            /* corner coordinates */
    double upper_left_x;
    double upper_right_y;
    double upper_right_x;
    double lower_left_y;
    double lower_left_x;
    double lower_right_y;
    double lower_right_x;
    double projection_distance_y;   /* pixel size in projection coordinates */
    double projection_distance_x;
    double maximum_pixel_value;     /* maximum DN of pixels */
    double minimum_pixel_value;     /* minimum DN of pixels */
    int pixel_range_valid;          /* indicates valid min/max pixel values */
    double maximum_radiance;        /* maximum radiance value */
    double minimum_radiance;        /* minimum radiance value */
    double spectral_radiance_scaling_offset;
                                /* offset to convert to spectral radiance */
    double spectral_radiance_scaling_gain;
                                /* gain to convert to spectral radiance */
    int radiance_valid;         /* indicates radiance items are valid */
    double reflectance_scaling_offset;/* offset to convert to reflectance */
    double reflectance_scaling_gain;  /* gain to convert to reflectance */
    int reflectance_valid;      /* indicates reflectance items are valid */
    char instrument_source[IAS_INSTRUMENT_SOURCE_SIZE]; /* source of the data */

} IAS_L1G_BAND_METADATA;


/****************************************************************************
Function prototypes
****************************************************************************/

L1GIO *ias_l1g_open_image
(
    const char *image_filename, /* I: Path & name of the 1G image file*/
    IAS_ACCESS_MODE access_mode /* I: Requested access mode of image */
);

int ias_l1g_close_image
(
    L1GIO *l1g      /* I: L1G IO structure */
);

L1G_BAND_IO *ias_l1g_open_band
(
    L1GIO *l1g_file,        /* I: L1G info structure for the image */
    int band_number,        /* I: band number to open */
    IAS_DATA_TYPE *data_type,/* I: data type for the imagery */
    int *number_of_scas,    /* I/O: SCAs to include in the band (1 if an
                                    SCA combined image is needed) */
    int *line_count,        /* I/O: lines in the band (input if writing a new 
                                    band, output if opening an existing band) */
    int *sample_count       /* I/O: samples in the band (input if writing a new
                                    band, output if opening an existing band) */
);

int ias_l1g_close_band
(
    L1G_BAND_IO *l1g_band      /* I: L1G_BAND_IO structure */
);

int ias_l1g_read_image
(
    const L1G_BAND_IO *l1g_band,   /* I: L1G_BAND_IO structure */
    int sca_index,      /* I: SCA to write to (0-rel) */
    int  start_line,    /* I: Line to start reading (0-rel) */
    int  start_sample,  /* I: Sample to start reading (0-rel) */
    int  line_count,    /* I: Number of lines to read */
    int  sample_count,  /* I: Number of samples to read */
    void *data          /* O: Data buffer */
);

int ias_l1g_write_image
(
    const L1G_BAND_IO *l1g_band,   /* I: L1G_BAND_IO structure */
    int sca_index,      /* I: SCA to write to (0-rel) */
    int start_line,     /* I: Line to start writing (0-rel) */
    int start_sample,   /* I: Sample to start writing (0-rel) */
    int line_count,     /* I: Number of lines to write */
    int sample_count,   /* I: Number of samples to write */
    void *data          /* I: Data buffer */
);

int ias_l1g_set_file_metadata
(
    L1GIO *l1g_file,        /* I: L1G IO structure */
    const IAS_L1G_FILE_METADATA *metadata /* I: metadata to set */
);

int ias_l1g_get_file_metadata
(
    L1GIO *l1g_file,        /* I: L1G IO structure */
    IAS_L1G_FILE_METADATA *metadata /* O: output metadata */
);

int ias_l1g_set_band_metadata
(
    L1GIO *l1g_file,            /* I: file to set metadata for */
    const IAS_L1G_BAND_METADATA *metadata, /* I: array of metadata to set */
    int metadata_size           /* I: number of entries in metadata array */
);

int ias_l1g_get_band_metadata
(
    L1GIO *l1g_file,                /* I: file to read metadata from */
    int band_number,                /* I: band number to get */
    IAS_L1G_BAND_METADATA *metadata /* O: buffer to return data in */
);

int ias_l1g_get_band_count
(
    L1GIO *l1g              /* I: L1G info structure for the image */
);

int ias_l1g_is_band_present
(
    L1GIO *l1g_file,        /* I: L1G info structure for the image */
    int band_number         /* I: band number to check */
);

int ias_l1g_get_band_size
(
    L1GIO *l1g_file,        /* I: L1G info structure for the image */
    int band_number,        /* I: band number to get information for */
    IAS_DATA_TYPE *data_type,/* O: data type for the imagery */
    int *number_of_scas,    /* O: SCAs in the band */
    int *line_count,        /* O: lines in the band */
    int *sample_count       /* O: samples in the band */
);

int ias_l1g_get_band_list
(
    L1GIO *l1g_file,            /* I: file to search for the band */
    int *band_number_list,      /* O: array of band numbers found in file */
    int band_number_list_size,  /* I: size of band_number_list array */
    int *number_of_bands        /* O: pointer to the number of bands returned
                                      in the band_number_list */
);

int ias_l1g_is_l1g_file
(
    const char *grid_filename /* I: HDF input file name                    */
);

#endif
