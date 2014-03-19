#ifndef _COMMON_MODEL_H
#define _COMMON_MODEL_H

/*************************************************************************
 This file is for local defines and local functions only.
**************************************************************************/
#include <hdf5.h>
#include <hdf5_hl.h>
#include "ias_const.h"
#include "ias_los_model.h"

/* Convenience enum for identifying which table to create */
typedef enum
{
    GENERAL_TABLE,
    SENSOR_TABLE,
    PRECISION_TABLE,
    EARTH_TABLE,
    BAND_TABLE,
    BAND_SCA_TABLE
} MODEL_TABLE;

#define ATTITUDE_MODEL_GROUP_NAME "Attitude"
#define EPHEMERIS_MODEL_GROUP_NAME "Ephemeris"
#define JITTER_MODEL_GROUP_NAME "Jitter Model"
#define UTC_EPOCH_TIME "UTC Epoch Time"
#define NOMINAL_SAMPLE_TIME "Nominal Sample Time"
#define ALIGNMENT_MATRIX "Alignment Matrix"

/* These are the table and dataset names stored in the HDF */
#define ROOT_TABLE_NAME "LOS Model"
#define GENERAL_TABLE_NAME "General Metadata"
#define SENSOR_TABLE_NAME "Sensor Model"
#define PRECISION_TABLE_NAME "Precision Correction Model"
#define EARTH_TABLE_NAME "Earth Model"
#define BAND_TABLE_NAME "Band Model"
#define SCA_TABLE_NAME "SCA Model"
#define FRAME_TIMES_RECORD_NAME "Frame Time From Epoch"
#define SCA_L0R_FILL_RECORD_NAME "L0R Fill"
#define SCA_ALONG_DET_OFF_RECORD_NAME "Along-track Detector Offsets"
#define SCA_ACROSS_DET_OFF_RECORD_NAME "Across-track Detector_Offsets"
#define ATTITUDE_RECORD_NAME "Attitude model record"
#define EPHEMERIS_RECORD_NAME "Ephemeris model record"
#define SSM_RECORD_NAME "SSM model record"

/*******************************************************/
#define OLI_JITTER_ENTRIES_NAME "OLI Jitter Entries Per Frame"
#define TIRS_JITTER_ENTRIES_NAME "TIRS Jitter Entries Per Frame"
/* If one of the above names for the jitter table entries
   per frame is updated, update the below max length if needed
   too!  The max length is the length of the longest of the two
   record name strings. It doesn't need to account for the
   null-terminator. */
#define JITTER_ENTRIES_NAME_MAX_LENGTH   29

#define OLI_JITTER_TABLE_NAME "OLI Jitter table"
#define TIRS_JITTER_TABLE_NAME "TIRS Jitter table"
/* If one of the above table names for the jitter table
   is updated, update the below max length if needed too!
   The max length is the length of the longest of the two
   record name strings. It doesn't need to account for
   the null-terminator. */
#define JITTER_TABLE_NAME_MAX_LENGTH   17
/*******************************************************/

/* For error checking these must be set to the number of fields as listed in
   the model structure. NFIELDS *must* be as large as the largest number of
   fields. */
#define NUM_GENERAL_FIELDS          6
#define NUM_SENSOR_FIELDS           5
#define NUM_PRECISION_FIELDS        9
#define NUM_EARTH_FIELDS            8
#define NUM_BAND_SAMP_CHAR_FIELDS   12
#define NUM_SCA_FIELDS              3
#define NFIELDS                     NUM_BAND_SAMP_CHAR_FIELDS

#define MODEL_FORMAT_VERSION 1

/* Local convenience structures for use with HDF5 */
typedef struct general_model
{
    /* -------------These come from IAS_LOS_MODEL------------- */
    int satellite_id;             /* Satellite ID */
    char sw_version[IAS_SOFTWARE_VERSION_SIZE];
                                  /* Producing system software version */
    int wrs_path;                 /* WRS target path number */
    int wrs_row;                  /* WRS target row number */
    int acquisition_type;         /* Acquisition type: Earth,Lunar,or Stellar */
    int correction_type;          /* Correction type: Systematic or Precision */
} GENERAL_MODEL;

typedef struct sensor_location_model
{
    IAS_SENSOR_ID sensor_id;        /* Sensor id */
    int sensor_present;             /* 1 if bands from this sensor
                                       represented by model, else 0 */
    double sensor2acs[TRANSFORMATION_MATRIX_DIM][TRANSFORMATION_MATRIX_DIM];
                                    /* Sensor to ACS transformation */
    double center_mass2sensor_offset[TRANSFORMATION_MATRIX_DIM];
                                    /* Offset meters between SC center
                                       of mass and the sensor */
    int ssm_model_present;          /* Flag if Scene select mirror
                                       model is present */
} SENSOR_LOCATION_MODEL;

typedef struct band_samp_char_model
{
    /* -------------These come from IAS_SENSOR_BAND_MODEL------------- */
    int band_present;           /* 1 if band is present in the image
                                   and represented by model, else 0 */
    int sca_count;              /* Number of SCAs in the band */
    double utc_epoch_time[3];   /* Year, day of year, seconds of day for
        start of image. This will be the same for all the bands from the
        same sensor. */

    /* ---These come from IAS_SENSOR_DETECTOR_SAMPLING_CHARACTERISTICS--- */
    double integration_time;    /* Integration time in seconds; the length of
        time during which the detectors are collecting a charge */
    double sampling_time;       /* Computed sampling time in seconds; the
        length of time between successive measurements */
    int lines_per_frame;        /* Number of lines per image frame. The
        timestamps are collected at the end of each frame, so this can also
        be thought of as the number of lines collected for each timestamp.
        For OLI, this is 2 for the PAN band and 1 for the other bands since
        the PAN band is sampled twice for every MS band sample. */
    double settling_time;       /* Sampling settling time (seconds) */
    double along_ifov;          /* Along track IFOV */
    double across_ifov;         /* Across track IFOV */
    double maximum_detector_delay; /* Maximum detector delay (in IFOV) */
    int time_codes_at_frame_start; /* flag indicating time codes collected at
                                      the start of the frame */
    int frame_delay;            /* flag indicating if time code has the OLI
                                   frame time delay or not */
} BAND_SAMP_CHAR_MODEL;

typedef struct sca_model
{
    /* -------------These come from IAS_SENSOR_SCA_MODEL------------- */
    int nominal_fill; /* Pixel offset fill needed to ensure positive pixel
        locations in L0R for this SCA. */
    double sca_coef_x[IAS_LOS_LEGENDRE_TERMS];
      /* Along-track SCA LOS Legendre coeffs. Note that "_x" refers to the
         sensor coordinate system where the x direction is in the direction
         of satellite motion. (radians) */
    double sca_coef_y[IAS_LOS_LEGENDRE_TERMS];
      /* Across-track SCA LOS Legendre coeffs. Note that "_y" refers to the
         sensor coordinate system where the y direction is perpendicular to
         the plane formed by the x axis (satellite motion) and the z axis
         pointing to the earth's center. (radians) */
} SCA_MODEL;

/* Function prototypes */
/* Read model routines found in read_model.c */
int ias_model_read_table
(
    hid_t file,             /* I: An open HDF5 file handle */
    MODEL_TABLE table,      /* I: Flag indicating which table to read */
    int band_to_read,       /* I: For reading band or SCA tables, else -1 */
    int sca_to_read,        /* I: For reading SCA table, else -1 */
    IAS_LOS_MODEL *los_model/* O: Model structure to store data */
);
int ias_model_read_frame_times
(
    hid_t file,                 /* I: An open HDF5 file handle */
    IAS_LOS_MODEL *los_model    /* O: Model structure to fill */
);
int ias_model_read_band_model
(
    hid_t file,                 /* I: An open HDF5 file handle */
    IAS_LOS_MODEL *los_model    /* O: Model structure to store data */
);
int ias_model_read_attitude_model
(
    hid_t file,                 /* I: An open HDF5 file handle */
    IAS_SC_ATTITUDE_MODEL *att  /* I: Attitude data to read */
);
int ias_model_read_ephemeris_model
(
    hid_t file,                     /* I: An open HDF5 file handle */
    IAS_SC_EPHEMERIS_MODEL *ephem   /* I: ephemeris data to read */
);
int ias_model_read_jitter_table
(
    hid_t file,                     /* I: An open HDF5 file handle */
    IAS_SENSOR_MODEL *sensor        /* O: Jitter structure to fill */
);
int ias_model_read_ssm_models
(
    hid_t file,                     /* I: An open HDF5 file handle */
    IAS_SENSOR_MODEL *sensor        /* I/O: SSM data to read */
);

/* Write model routines found in write_model.c */
int ias_model_write_table
(
    hid_t file,           /* I: An open HDF5 file handle */
    MODEL_TABLE table,    /* I: Flag indicating which table to read */
    int band_to_write,    /* I: For writing band or SCA tables, or -1 */
    int sca_to_write,     /* I: For writing SCA table, or -1 */
    const IAS_LOS_MODEL *los_model /* I: Model structure with data to write */
);
int ias_model_write_frame_times
(
    hid_t file,                    /* I: An open HDF5 file handle */
    const IAS_LOS_MODEL *los_model /* I: Model structure with data to write */
);
int ias_model_write_band_model
(
    hid_t file,                    /* I: An open HDF5 file handle */
    const IAS_LOS_MODEL *los_model /* I: Model structure with data to write */
);
int ias_model_write_attitude_model
(
    hid_t file,                         /* I: An open HDF5 file handle */
    const IAS_SC_ATTITUDE_MODEL *att    /* I: Attitude data to write */
);
int ias_model_write_ephemeris_model
(
    hid_t file,                         /* I: An open HDF5 file handle */
    const IAS_SC_EPHEMERIS_MODEL *ephem /* I: Ephemeris data to write */
);
int ias_model_write_jitter_table
(
    hid_t file,                         /* I: An open HDF5 file handle */
    const IAS_SENSOR_MODEL *sensor      /* I: Jitter table data to write */
);
int ias_model_write_ssm_model
(
    hid_t file,                         /* I: An open HDF5 file handle */
    const IAS_SENSOR_MODEL *sensor      /* I: SSM data to write */
);

/* Common model I/O routines found in common_model_io.c */
int ias_model_build_general_description
(
    size_t offsets[NFIELDS],          /* O: Offsets into data structure */
    const char *field_names[NFIELDS], /* O: Names of the fields */
    hid_t field_types[NFIELDS],       /* O: Data types of the fields */
    hid_t fields_to_close[NFIELDS],   /* O: List of fields to be closed */
    size_t field_sizes[NFIELDS],      /* O: Sizes of the fields */
    int *number_fields                /* O: Total number of fields */
);
int ias_model_build_earth_description
(
    size_t offsets[NFIELDS],          /* O: Offsets into data structure */
    const char *field_names[NFIELDS], /* O: Names of the fields */
    hid_t field_types[NFIELDS],       /* O: Data types of the fields */
    hid_t fields_to_close[NFIELDS],   /* O: List of fields to be closed */
    size_t field_sizes[NFIELDS],      /* O: Sizes of the fields */
    int *number_fields                /* O: Total number of fields */
);
int ias_model_build_sensor_description
(
    size_t offsets[NFIELDS],          /* O: Offsets into data structure */
    const char *field_names[NFIELDS], /* O: Names of the fields */
    hid_t field_types[NFIELDS],       /* O: Data types of the fields */
    hid_t fields_to_close[NFIELDS],   /* O: List of fields to be closed */
    size_t field_sizes[NFIELDS],      /* O: Sizes of the fields */
    int *number_fields                /* O: Total number of fields */
);
int ias_model_build_precision_description
(
    size_t offsets[NFIELDS],          /* O: Offsets into data structure */
    const char *field_names[NFIELDS], /* O: Names of the fields */
    hid_t field_types[NFIELDS],       /* O: Data types of the fields */
    hid_t fields_to_close[NFIELDS],   /* O: List of fields to be closed */
    size_t field_sizes[NFIELDS],      /* O: Sizes of the fields */
    int *number_fields                /* O: Total number of fields */
);
int ias_model_build_band_description
(
    size_t offsets[NFIELDS],          /* O: Offsets into data structure */
    const char *field_names[NFIELDS], /* O: Names of the fields */
    hid_t field_types[NFIELDS],       /* O: Data types of the fields */
    hid_t fields_to_close[NFIELDS],   /* O: List of fields to be closed */
    size_t field_sizes[NFIELDS],      /* O: Sizes of the fields */
    int *number_fields                /* O: Total number of fields */
);
int ias_model_build_band_sca_description
(
    int detectors,                    /* I: Number of detectors on this SCA */
    size_t offsets[NFIELDS],          /* O: Offsets into data structure */
    const char *field_names[NFIELDS], /* O: Names of the fields */
    hid_t field_types[NFIELDS],       /* O: Data types of the fields */
    hid_t fields_to_close[NFIELDS],   /* O: List of fields to be closed */
    size_t field_sizes[NFIELDS],      /* O: Sizes of the fields */
    int *number_fields                /* O: Total number of fields */
);
int ias_model_create_compound_vector_type
(
    hid_t *vector                       /* O: Vector inserted */
);
int ias_model_create_compound_attitude_type
(
    hid_t vector,       /* I: An open ID of an IAS_VECTOR compound data type */
    hid_t *record       /* O: Record inserted */
);
hid_t ias_model_create_compound_ephemeris_type
(
    hid_t vector,       /* I: An open ID of an IAS_VECTOR compound data type */
    hid_t *record       /* O: Record inserted */
);
int ias_model_create_compound_ssm_type
(
    hid_t *record       /* O: Record created */
);
void ias_model_close_the_fields
(
    hid_t fields_to_close[NFIELDS] /* I: Array of field identifiers */
);
#endif
