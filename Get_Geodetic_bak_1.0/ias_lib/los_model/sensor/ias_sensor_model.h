#ifndef IAS_SENSOR_MODEL_H
#define IAS_SENSOR_MODEL_H

#include "ias_const.h"
#include "ias_types.h"
#include "ias_l0r.h"
#include "ias_structures.h"
#include "ias_cpf.h"
#include "ias_math.h"
#include "ias_satellite_attributes.h"

#define TRANSFORMATION_MATRIX_DIM 3  /* sizes of arrays/matrices in
                                        sensor model */

/* The imagery is collected on a frame by frame basis. A timestamp is collected
   at the end of each frame. During a frame, a single line of MS data is
   collected while 2 lines of Pan data are collected. The lines_per_frame helps
   us relate the lines of imagery to the frames and, ultimately, the timestamps
   at the end of each frame. */

/* Structures needed to support both model init and the Ingest system to be
   able to collect the L0R data required to set in the model */
typedef struct oli_sensor_l0r_data
{
    int sensor_present;         /* 1 if OLI band(s) in data, else 0 */
    double nominal_frame_time;  /* Nominal OLI frame time from CPF */
    IAS_L0R_OLI_FRAME_HEADER *frame_headers;   /* OLI line headers meta */
    int num_frames;             /* Number of OLI image frames */
    int num_bad_frame_times;    /* Number of frame times that were corrected */
} OLI_SENSOR_L0R_DATA;

typedef struct tirs_sensor_l0r_data
{
    int sensor_present;         /* 1 if TIRS bands(s) in data, else 0 */
    double nominal_frame_time;  /* Nominal TIRS frame time from CPF */
    IAS_L0R_TIRS_FRAME_HEADER *frame_headers;   /* TIRS line headers meta */
    int num_frames;             /* Number of TIRS image frames */
    IAS_L0R_TIRS_SSM_ENCODER *ssm_records; /* SSM records from L0R */
    int num_ssm_records;        /* Number of SSM records */
    int num_bad_frame_times;    /* Number of frame times that were corrected */
    int num_bad_ssm_times;      /* Number of SSM times that were corrected */
    int num_bad_ssm_encoder_values; /* Number of SSM encoder values that were
                                       corrected */
} TIRS_SENSOR_L0R_DATA;

typedef struct ias_sensor_l0r_data
{
    OLI_SENSOR_L0R_DATA oli_data;   /* OLI sensor data */
    TIRS_SENSOR_L0R_DATA tirs_data; /* TIRS sensor data */
    int bands_present[IAS_MAX_NBANDS];  /* 1 if band present, else 0 for each */
    IAS_L0R_OLI_IMAGE_HEADER image_header; /* L0R image header meta */
    int image_header_flag;          /* Flag if image_header set from L0R */
    uint16_t **l0r_detector_offsets;/* Before detector offsets per band */
    const IAS_MATH_LEAP_SECONDS_DATA *leap_seconds_data; /* CPF leap seconds */
} IAS_SENSOR_L0R_DATA;

typedef enum ias_sensor_detector_type
{
    IAS_NOMINAL_DETECTOR,   /* Nominal (central) detector location */
    IAS_ACTUAL_DETECTOR,    /* Location including even/odd offset */
    IAS_EXACT_DETECTOR,     /* Location including detector delays */
    IAS_MAXIMUM_DETECTOR    /* Maximum detector delay */
} IAS_SENSOR_DETECTOR_TYPE;

typedef struct ias_sensor_detector_sampling_characteristics
{
    double integration_time;       /* Integration time in seconds; the length
        of time during which the detectors are collecting a charge */
    double sampling_time;          /* Computed sampling time in seconds; the
        length of time between successive measurements */
    int lines_per_frame;          /* Number of lines per image frame. The
        timestamps are collected at the end of each frame, so this can also
        be thought of as the number of lines collected for each timestamp.
        For OLI, this is 2 for the PAN band and 1 for the other bands since
        the PAN band is sampled twice for every MS band sample. */
    double settling_time;          /* Sampling settling time (seconds) */
    double along_ifov;             /* Along track IFOV */
    double across_ifov;            /* Across track IFOV */
    double maximum_detector_delay; /* Maximum detector delay (in IFOV) */
    int time_codes_at_frame_start; /* flag to indicate whether the time codes
                are relative to the start or end of the frame (1 = start of
                frame).  OLI time codes are collected at the end of the frame
                while TIRS are collected at the start of the frame. */
    int frame_delay;               /* flag to indicate if this is a band that
                                      has the frame time delay to adjust for
                                      (set for all OLI bands present; TIRS does
                                       not have the delay) */
} IAS_SENSOR_DETECTOR_SAMPLING_CHARACTERISTICS;

typedef struct ias_sensor_ssm_record
{
    double seconds_from_epoch;    /* Seconds from epoch for this SSM sample */
    double mirror_angle;          /* SSM angle (radians) */
} IAS_SENSOR_SSM_RECORD;

/* TIRS-specific; not utilized for OLI data. */
typedef struct ias_sensor_scene_select_mirror_model
{
    double alignment_matrix[TRANSFORMATION_MATRIX_DIM]
            [TRANSFORMATION_MATRIX_DIM]; /* SSM alignment matrix */
    double utc_epoch_time[3];      /* Year, day of year, seconds of day for
        time of first mirror times record. The seconds_from_epoch values in the
        records are relative to this epoch time. FIXME - the calval prototype
        does not have an epoch time in this structure.  Instead, the times
        are relative to the epoch of the epoch time for the frames.  For now,
        we'll copy that value here, but it might make sense to eliminate it. */
    int ssm_record_count;          /* Count of SSM records */
    IAS_SENSOR_SSM_RECORD *records;/* Pointer to the array of SSM records */

} IAS_SENSOR_SCENE_SELECT_MIRROR_MODEL;

typedef struct ias_sensor_location_model
{
    IAS_SENSOR_ID sensor_id;        /* Sensor id */
    int sensor_present;             /* 1 if bands from this sensor
                                       represented by model, else 0 */
    double sensor2acs[TRANSFORMATION_MATRIX_DIM][TRANSFORMATION_MATRIX_DIM];
                                    /* Sensor to ACS transformation */
    double center_mass2sensor_offset[TRANSFORMATION_MATRIX_DIM];
                                    /* Offset meters between SC center
                                       of mass and the sensor */
    IAS_VECTOR *jitter_table;       /* The high frequency attitude perturbations
                                       filtered from the original attitude data.
                                       These have been interpolated to a per
                                       image line basis. For OLI, the data are
                                       based on PAN band (one per PAN line). */
    int jitter_table_count;         /* The number of vectors in the jitter
                                       table */
    int jitter_entries_per_frame;   /* The lines_per_frame value used for this
                                       sensor when creating the jitter table */
    IAS_SENSOR_SCENE_SELECT_MIRROR_MODEL *ssm_model;
                                    /* Scene select mirror model pointer.
                                       NULL if no scene select mirror. */
    /* As needed, add fields that may only apply to only some sensors */
    /* As needed, add function pointers to customize per sensor handling */
} IAS_SENSOR_LOCATION_MODEL;

typedef struct ias_sensor_sca_model
{
    int detectors;    /* Number of detectors on the SCA */
    int nominal_fill; /* Pixel offset fill needed to ensure positive pixel
        locations in L0R for this SCA. */
    int *l0r_detector_offsets; /* 1D array for along track pixel offset fill
        inserted in L0R for each detector. In usual case, where L1 processing
        uses the same CPF as L0 processing did (or a different CPF, but the
        offsets were not modified), then these values would be very similar to
        the offsets pulled from the CPF for L1 processing, stored in this
        structure also in detector_offsets_along_track. So, in the usual case,
        the values in l0r_detector_offsets should match the integer portion of
        the values in detector_offsets_along_track. */
    double *detector_offsets_along_track;  /* Along track pixel offsets (in
        IFOV) for each detector (radians) */
    double *detector_offsets_across_track; /* Across track pixel offsets (in
        IFOV) for each detector (radians) */
    double sca_coef_x[IAS_LOS_LEGENDRE_TERMS];
      /* Along-track SCA LOS Legendre coeffs. Note that "_x" refers to the
         sensor coordinate system where the x direction is in the direction
         of satellite motion. (radians) */
    double sca_coef_y[IAS_LOS_LEGENDRE_TERMS];
      /* Across-track SCA LOS Legendre coeffs. Note that "_y" refers to the
         sensor coordinate system where the y direction is perpendicular to
         the plane formed by the x axis (satellite motion) and the z axis
         pointing to the earth's center. (radians) */
} IAS_SENSOR_SCA_MODEL;

typedef struct ias_sensor_band_model
{  
    int band_present;                    /* 1 if band is present in the image
                                            and represented by model, else 0 */
    int frame_count;                     /* Number of frames in imagery */
    int sca_count;                       /* Number of SCAs in the band */
    IAS_SENSOR_LOCATION_MODEL *sensor;   /* Pointer to the sensor model
        information in the main model structure */
    double utc_epoch_time[3];            /* Year, day of year, seconds of day
        for start of image. This will be the same for all the bands from the
        same sensor. frame_seconds_from_epoch values are offsets from this. */
    double *frame_seconds_from_epoch;     /* Pointer to correct sensor model
        frame_seconds_from_epoch array in the IAS_SENSOR_MODEL to prevent
        redundantly storing the data. Frame times are the same for all bands
        from the same sensor. */
    IAS_SENSOR_DETECTOR_SAMPLING_CHARACTERISTICS sampling_char; 
                                  /* Sampling characteristics for this band */
    IAS_SENSOR_SCA_MODEL *scas;   /* 1D array of LOS model info for each SCA */
    /* As needed, add function pointers to tailor per band model handling */
} IAS_SENSOR_BAND_MODEL;

typedef struct ias_sensor_model
{
    int band_count;                        /* Number of bands on the sensor */
    IAS_SENSOR_BAND_MODEL *bands;          /* 1D array of band sensor models */
    /* The following member tracks the seconds each frame is from the epoch
       for the sensor. Since a satellite can have multiple sensors, the
       frame times need to be tracked separately for each sensor. To prevent
       replicating the same data for every band of a sensor, they are stored
       once here and each band points to the correct set. This array makes
       it easy to free the memory. */
    double *frame_seconds_from_epoch[IAS_MAX_SENSORS]; /* 1D array containing
        the seconds from epoch for each frame present in image data. First
        frame time is 0 since that is epoch, all following frame times are the
        seconds from the first frame - calculated as a given frame's frame time
        minus the epoch frame time resulting in this value being an offset from
        the epoch frame time. */
    int frame_counts[IAS_MAX_SENSORS]; /* Number of the entries in each of
        the frame_seconds_from_epoch arrays to allow reading/writing the data */
    /* The following member is an array of sensor specific information for each
       of the supported sensors. The sensor_id field will be set to
       IAS_MAX_SENSORS if the structure is not initialized. */
    IAS_SENSOR_LOCATION_MODEL sensors[IAS_MAX_SENSORS];
}IAS_SENSOR_MODEL;


/***************************************************************************
Function prototypes
****************************************************************************/
int ias_sensor_adjust_los_for_ssm
(
    IAS_SENSOR_SCENE_SELECT_MIRROR_MODEL *ssm, /* I: SSM structure */
    double seconds_from_epoch,   /* I: seconds from epoch */
    IAS_VECTOR *in_los,          /* I: telescope LOS vector */
    IAS_VECTOR *out_los          /* O: SSM adjusted LOS vector */
);

void ias_sensor_align_ssm_data
(
    IAS_L0R_TIRS_SSM_ENCODER *l0r_tirs_ssm, /* I/O: SSM telemetry records */
    int count                               /* I: Number SSM records in L0R */
);

int ias_sensor_check_ssm_encoder_data
(
    IAS_L0R_TIRS_SSM_ENCODER *l0r_tirs_ssm, /* I/O: L0R SSM encoder records */
    int count,                          /* I: Number of SSM encoder records */
    double encoder_origin,              /* I: Encoder nadir angle for current
                                              MCE side */
    IAS_CPF *cpf,                       /* I: Calibration parameter info */
    int *num_bad_ssm_times,             /* O: number of SSM times corrected */
    int *num_bad_ssm_encoder_values     /* O: number of SSM encoder values
                                              corrected */
);

int ias_sensor_find_los_vector
(
    int sca_index,                     /* I: Input point SCA index (0-based) */
    double detector,                   /* I: Input point detector number */
    IAS_SENSOR_DETECTOR_TYPE type,     /* I: Type of detector model to use */
    const IAS_SENSOR_BAND_MODEL *band, /* I: band model structure */
    IAS_VECTOR *losv                   /* O: Line of sight vector */
);

int ias_sensor_find_time
(
   double line,         /* I: Line from start of the image (0-rel)*/
   double sample,       /* I: Sample from start of the line (0-rel)*/
   int band_index,      /* I: Band in the image */
   int sca_index,       /* I: Current SCA in image */
   const IAS_SENSOR_MODEL *model,/* I: model structure */
   IAS_SENSOR_DETECTOR_TYPE type,/* I: Detector option NOMINAL or ACTUAL  */
   double *time         /* O: Time from start of image (seconds) */
);

int ias_sensor_get_jitter
(
    double line,                    /* I: Line from start of image (0-rel) */
    double sample,                  /* I: Sample from start of line (0-rel) */
    int band_index,                 /* I: Band index */
    int sca_index,                  /* I: Current SCA index */
    const IAS_SENSOR_MODEL *sensor, /* I: IAS sensor model structure */
    double *roll,                   /* O: Roll jitter (radians) */
    double *pitch,                  /* O: Pitch jitter (radians) */
    double *yaw                     /* O: Yaw jitter (radians) */
);

int ias_sensor_set_cpf
(
    IAS_CPF *cpf,                /* I: CPF structure pointer with values to
                                   set into the sensor structure */
    IAS_SENSOR_L0R_DATA *l0r_sensor_data,
                                /* O: Sensor L0R data needed by other funcs */
    IAS_SENSOR_MODEL *sensor    /* IO: sensor model structure to populate */
);


/////////////////////////////////////////////////////////////////////////////
//added by LYL at 2014/3/6
int ias_sensor_set_cpf_for_MWD
(
    IAS_CPF *cpf,                /* I: CPF structure pointer with values to
                                   set into the sensor structure */
    IAS_SENSOR_MODEL *sensor    /* IO: sensor model structure to populate */
);
////////////////////////////////////////////////////////////////////////////


int ias_sensor_set_l0r
(
    IAS_CPF *cpf,                    /* I: CPF SSM info and tolerance checks */
    IAS_SENSOR_L0R_DATA *l0r_data,  /* I: L0R data to create a model */
    IAS_SENSOR_MODEL *sensor        /* O: Model structure to populate */
);

int ias_sensor_set_frame_times
(
    IAS_SENSOR_MODEL *model,          /* I: model to set the frame times in */
    IAS_SENSOR_ID sensor_id,          /* I: sensor id to set frame times for */
    double *frame_seconds_from_epoch, /* I: array of frame times */
    int frame_count                   /* I: count of frame times in the array */
);

int ias_sensor_set_ssm_from_l0r
(
    TIRS_SENSOR_L0R_DATA *l0r_data, /* I: L0R data to create a model */
    IAS_CPF *cpf,                    /* I: Calibration parameter file */
    IAS_SENSOR_MODEL *sensor,       /* IO: Sensor model to update with flags */
    IAS_SENSOR_ID sensor_id         /* I: Sensor id to set SSM info into */
);

int ias_sensor_smooth_ssm_data
(
    IAS_SENSOR_SCENE_SELECT_MIRROR_MODEL *ssm_model /* I/O: SSM data to smooth*/
);

double ias_sensor_get_maximum_detector_delay();

#endif