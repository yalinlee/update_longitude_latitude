#ifndef IAS_CPF_H 
#define IAS_CPF_H
/*************************************************************************

NAME: ias_cpf.h

PURPOSE: Header file defining cpf structure and prototypes to move data
         between odltree and internal cpf structure.

Notes:
- This library is not thread-safe.  Do not call it from multiple threads 
  without serializing it.
- The ias_cpf_get_* routine returns a constant pointer to structures in the 
  allocated CPF.  So, ias_cpf_free can not be called until all the pointers
  returned by ias_cpf_get_* routines are no longer needed.
- More information about the units of each element can be found in the
  CPF DFCB.
******************************************************************************/

#include "ias_satellite_attributes.h"
#include "ias_math.h"

/***********************************************************************
  The defines located in this header file are ias specific and will not
  be used else where.  Note that this should only be used if absolutely
  necessary.  It is preferred that arrays be sized dynamically.
***********************************************************************/
#define IAS_UT1_PERIOD 180      /* number of days of utm/utc data in cpf */
#define IAS_UT1_MONTH_STRLEN 4  /* length of ut1 abreviated month */
#define IAS_TIRS_MAX_NBANDS 2       /* max tirs band count */
#define IAS_OLI_MAX_NBANDS 9        /* max oli band count */
#define IAS_TIRS_MAX_NSCAS 3        /* max tirs sca count */
#define IAS_CPF_LOS_EPHEMERIS_APRI_COUNT 12 /* los model correction parameter */
#define IAS_CPF_LOS_ATTITUDE_APRI_COUNT 12 /* los model correction parameter */
#define IAS_CPF_LOS_OBSERVATION_APRI_COUNT 2 /* los model correction parameter */
/***********************************************************************
  The structures to go into the IAS L8 OLI/TIRS CPF structure
***********************************************************************/

/* OLI & TIRS radiometric processing, Gain Application */
typedef struct IAS_CPF_ABSOLUTE_GAINS
{
    double *gain[IAS_MAX_NBANDS];
}IAS_CPF_ABSOLUTE_GAINS;

/* Sensor ancillary data preprocessing */
typedef struct IAS_CPF_ANCILLARY_ENG_CONV
{
    double quaternion_eng_conv;
    double ang_velocity_eng_conv;
    double quaternion_conv_vector;
    double quaternion_conv_scalar;
    double star_conv_time;
    double star_conv_position;
    double star_conv_intensity;
    double gyro_conv_angle;
    double gyro_conv_time;
    double tirs_integration_time_scale;
    double oli_integration_time_scale;
    int oli_integration_time_ms_offset_nominal;
    int oli_integration_time_pan_offset_nominal;
    int oli_integration_time_ms_offset_8x;
    int oli_integration_time_pan_offset_8x;
}IAS_CPF_ANCILLARY_ENG_CONV;

/* Sensor ancillary data preprocessing Off Nadir Scene Framing,  */
typedef struct IAS_CPF_ANCILLARY_QA_THRESHOLDS
{
    double orbit_radius_tolerance;      
    double angular_momentum_tolerance;  
    double spacecraft_clock_tai_epoch;  
    double iru_outlier_threshold[4];    
    double quaternion_normalization_outlier_threshold;
}IAS_CPF_ANCILLARY_QA_THRESHOLDS;

/* Sensor geometric processing, Off Nadir Scene Framing, TIRS Alignment,
   Calibration, TIRS LOS Model Creation */
typedef struct IAS_CPF_ATTITUDE_PARAMETERS
{
    double cm_to_oli_offsets[3];
    double iru_to_acs_align_matrix[3][3];
    double acs_to_oli_rotation_matrix[3][3];
    double acs_to_tirs_rotation_matrix[3][3];
    double cm_to_tirs_offsets[3];
    double siru_axis_a[3];
    double siru_axis_b[3];
    double siru_axis_c[3];
    double siru_axis_d[3];
}IAS_CPF_ATTITUDE_PARAMETERS;

/* Struct for OLI sensor bias determination, 9 bands 14 scas */
typedef struct IAS_CPF_AVERAGE_BIAS
{
    double *bias_odd_pan[IAS_MAX_NSCAS];   /* Band 8 Odd Frames, scas 14 */
    double *bias_even_pan[IAS_MAX_NSCAS];  /* Band 8 Odd Frames, scas 14 */
    double *bias_swir[IAS_OLI_MAX_NBANDS][IAS_MAX_NSCAS];
                                                  /* Bands 6, 7, 9, scas 14 */
    double *bias_vnir[IAS_OLI_MAX_NBANDS][IAS_MAX_NSCAS];
                                                  /* Bands 1-5, scas 14 */
}IAS_CPF_AVERAGE_BIAS;

/* OLI and TIRS band 2 band characterization */
typedef struct IAS_CPF_B2B_ASSESSMENT
{
    int corr_window_size[2]  /* 0 = line, 1 = sample */;
    int corr_pix_min;
    int corr_pix_max;
    int corr_fit_method;
    double fill_threshold;
    double max_displacement_offset;
    double min_corr_strength;
    double trend_threshold_line[IAS_MAX_NBANDS][IAS_MAX_NSCAS];
    double trend_threshold_sample[IAS_MAX_NBANDS][IAS_MAX_NSCAS]; 
}IAS_CPF_B2B_ASSESSMENT;

/* Cloud Cover Assessment */
typedef struct IAS_CPF_CLOUD_COVER_ASSESSMENT
{
    int number_of_classes;
    int number_of_algorithms;
    double cirrus_threshold;
    char **cca_class_type;
    char **algorithm_names;
    int *run_if_thermal; /* 0=No 1=Yes 2=Always */
    double **weights;
}IAS_CPF_CLOUD_COVER_ASSESSMENT;

/* OLI & TIRS radiometric processing, Impulse Noise Char */
/* we use IAS_MAX_TOTAL_BANDS here because this group includes normal,
   vrp and blind bands */
typedef struct IAS_CPF_DETECTOR_NOISE
{
    double *per_detector[IAS_MAX_TOTAL_BANDS][IAS_MAX_NSCAS];
}IAS_CPF_DETECTOR_NOISE;

/* OLI & TIRS radiometric processing, Histogram Stats, */
typedef struct IAS_CPF_DETECTOR_OFFSETS
{
    double *along_per_detector[IAS_MAX_NBANDS][IAS_MAX_NSCAS];
    double *across_per_detector[IAS_MAX_NBANDS][IAS_MAX_NSCAS];
}IAS_CPF_DETECTOR_OFFSETS;

/* OLI & TIRS radiometric processing, Inoperable Det Fill */
typedef struct IAS_CPF_DETECTOR_STATUS
{
    int oli_detector_select_table;
    int oli_detector_select_table_id;
    int inoperable_count[IAS_MAX_TOTAL_BANDS][IAS_MAX_NSCAS];
    int *inoperable[IAS_MAX_TOTAL_BANDS][IAS_MAX_NSCAS];
    int out_of_spec_count[IAS_MAX_TOTAL_BANDS][IAS_MAX_NSCAS];
    int *out_of_spec[IAS_MAX_TOTAL_BANDS][IAS_MAX_NSCAS];
    int tirs_detector_map_id;
}IAS_CPF_DETECTOR_STATUS;

/* OLI Detector Response Solar Diffuser Input */
typedef struct IAS_CPF_DIFFUSER_RADIANCE
{
    double *diff_bidir_refl_prim[IAS_MAX_NBANDS][IAS_MAX_NSCAS]; 
    double *diff_bidir_refl_pris[IAS_MAX_NBANDS][IAS_MAX_NSCAS];
    double *diff_rad_prim[IAS_MAX_NBANDS][IAS_MAX_NSCAS]; 
    double *diff_rad_pris[IAS_MAX_NBANDS][IAS_MAX_NSCAS];
}IAS_CPF_DIFFUSER_RADIANCE;

/* OLI & TIRS geometric processing, LOS Model Creation */
typedef struct IAS_CPF_EARTH_CONSTANTS
{
    char ellipsoid_name[6];
    double semi_major_axis;
    double semi_minor_axis;
    double ellipticity;
    double eccentricity;
    double gravity_constant;
    double j2_earth_model;
    double earth_angular_velocity;
    char datum[30];
    double speed_of_light;
    int spheroid_code;
    IAS_MATH_LEAP_SECONDS_DATA leap_seconds_data;
}IAS_CPF_EARTH_CONSTANTS;

/* CPF file descripters */
typedef struct IAS_CPF_FILE_ATTRIBUTES
{
    char effective_date_begin[20];
    char effective_date_end[20];
    char baseline_date[20];
    char file_name[200];
    char file_source[200];
    char spacecraft_name[100];
    char sensor_name[100];
    char description[4000];
    int version;
}IAS_CPF_FILE_ATTRIBUTES;

/* OLI & TIRS radiometric processing, L1R Stitch, Focal Plane Alignment
   Calibration, TIRS LOS Projection */
typedef struct IAS_CPF_FOCAL_PLANE
{
    int detectors_per_band[IAS_MAX_NBANDS];
    int sca_overlap[IAS_MAX_NBANDS];
    char band_names[IAS_MAX_NBANDS][IAS_BAND_NAME_SIZE];
    int sca_offset[IAS_MAX_NBANDS][IAS_MAX_NSCAS];            
    int band_offset[IAS_MAX_NBANDS];
    int band_order[IAS_MAX_NBANDS];
    int nominal_fill_offset[IAS_MAX_NBANDS][IAS_MAX_NSCAS];  
    int primary_row_offsets[IAS_MAX_NBANDS][IAS_MAX_NSCAS]; 
    int alternate_row_offsets[IAS_MAX_NBANDS][IAS_MAX_NSCAS];
    double along_los_legendre[IAS_MAX_NBANDS][IAS_MAX_NSCAS]
                                                [IAS_LOS_LEGENDRE_TERMS];
    double across_los_legendre[IAS_MAX_NBANDS][IAS_MAX_NSCAS]
                                                [IAS_LOS_LEGENDRE_TERMS];
}IAS_CPF_FOCAL_PLANE;

/* OLI Focal Plane Alignment Calibration */
typedef struct IAS_CPF_FOCAL_PLANE_CAL
{
    int corr_window_size[2];     /* 0 = line, 1 = sample */
    double min_corr_strength;
    double max_displacement_offset;
    double fill_threshold;
    int corr_fit_method;
    int fit_order;
    double tie_point_weight;
    double along_postfit_rmse_threshold;
    double across_postfit_rmse_threshold;
}IAS_CPF_FOCAL_PLANE_CAL;

/* OLI & TIRS geometric processing, GCP Correlation */
typedef struct IAS_CPF_GCP_CORRELATION
{
    int corr_fit_method;
    int corr_window_size[2];    /* 0 = line, 1 = sample */
    int max_displacement_offset;
    double min_corr_strength;
    double fill_threshold;
    int corr_fill_value;
}IAS_CPF_GCP_CORRELATION;

/* OLI LOS model correction, LOS projection, LOS Model Correcvtion */
typedef struct IAS_CPF_GEO_SYSTEM
{
    double x_prefit_gcp_rms;
    double y_prefit_gcp_rms;
    double x_postfit_gcp_rms;
    double y_postfit_gcp_rms;
    double max_percent_gcp_outliers;
    double percent_outlier_threshold;
    double time_code_outlier_threshold;
    int ms_grid_density_elev;
    int pan_grid_density_elev;
    int tirs_grid_density_elev;
    int ms_grid_density_lines;
    int pan_grid_density_lines;
    int tirs_grid_density_lines;
    int ms_grid_density_samples;
    int pan_grid_density_samples;
    int tirs_grid_density_samples;
    int optimal_band_order[IAS_MAX_NBANDS]; /* includes TIRS bands */
    double low_pass_cutoff;
    int minimum_number_correlated_validation_gcps;
    int minimum_number_nonoutlier_control_gcps;
}IAS_CPF_GEO_SYSTEM;

/* OLI and TIRS Histogram Characterization parameters */
typedef struct IAS_CPF_HISTOGRAM_CHAR
{
    int frames_to_skip_top;
    int frames_to_skip_bottom;
}IAS_CPF_HISTOGRAM_CHAR;

/* OLI Image Registration Accuracy Assessment */
typedef struct IAS_CPF_I2I_ASSESSMENT
{
    int corr_fit_method;
    int corr_window_size[2] /* 0 = line, 1 = sample */;
    int corr_pix_min;
    int corr_pix_max;
    double min_corr_strength;
    double fill_threshold;
    int max_displacement_offset;
    double trend_threshold_line[IAS_MAX_NBANDS];
    double trend_threshold_sample[IAS_MAX_NBANDS];
}IAS_CPF_I2I_ASSESSMENT;

/* OLI and TIRS Impulse Noise Charaterization */
/* NOTE: We are using IAS_MAX_TOTAL_BANDS to allocate space as in_limit and
   median_filter_width will include OLI and TIRS bands 1-15, the vrp elements 
   include the oli vrp bands 1-9.  The unused pointers will be set to NULL */
typedef struct IAS_CPF_IMPULSE_NOISE
{
    int in_limit[IAS_MAX_TOTAL_BANDS];
    int median_filter_width[IAS_MAX_TOTAL_BANDS];
}IAS_CPF_IMPULSE_NOISE;

/* Utilized by OLI Detector Response Characterization (LAMP) */
typedef struct IAS_CPF_LAMP_RADIANCE
{
    double *effective_rad_working[IAS_OLI_MAX_NBANDS][IAS_MAX_NSCAS]; 
    double *effective_rad_backup[IAS_OLI_MAX_NBANDS][IAS_MAX_NSCAS]; 
    double *effective_rad_pristine[IAS_OLI_MAX_NBANDS][IAS_MAX_NSCAS]; 
}IAS_CPF_LAMP_RADIANCE;

/* OLI & TIRS los model correction processing */
typedef struct IAS_CPF_LOS_MODEL_CORRECTION
{
    double attitude_apri[IAS_CPF_LOS_ATTITUDE_APRI_COUNT];
    double ephemeris_apri[IAS_CPF_LOS_EPHEMERIS_APRI_COUNT];
    double doq_observation_apri[IAS_CPF_LOS_OBSERVATION_APRI_COUNT];
    double gls_observation_apri[IAS_CPF_LOS_OBSERVATION_APRI_COUNT];
}IAS_CPF_LOS_MODEL_CORRECTION;

/* Thresholds used during OLI Lunar Irradiance Characterization */
typedef struct IAS_CPF_LUNAR_IRRADIANCE
{
    int median_filter_size;
    double irradiance_conversion[IAS_OLI_MAX_NBANDS];
    double integration_threshold_factor[IAS_OLI_MAX_NBANDS];
}IAS_CPF_LUNAR_IRRADIANCE;

/* OLI & TIRS Nonuiformity Characterization */
/* For OLI, scale_factor_1 and scale_factor_2 hold the */
/* Solar_Diffuser_Scale_Primary and Solar_Diffuser_Scale_Pristine */
/* CPF parameter data.  For TIRS, scale_factor_1 holds the */
/* Blackbody_Scale CPF parameter data. */
typedef struct IAS_CPF_NONUNIFORMITY
{
   double *scale_factor_1[IAS_MAX_NBANDS][IAS_MAX_NSCAS];
   double *scale_factor_2[IAS_MAX_NBANDS][IAS_MAX_NSCAS];

}IAS_CPF_NONUNIFORMITY;

/* OLI & TIRS geometric processing, B2B Cal, Focal Plane Alignment Calibration, 
   LOS Model Creation */
typedef struct IAS_CPF_OLI_PARAMETERS
{
    int band_count;
    int sca_count;
    int vrp_count[IAS_MAX_NBANDS];
    double nominal_frame_time;
    double nominal_integration_time_ms;
    double nominal_integration_time_pan;
    double integration_time_tolerance;
    double integration_time_scale;
    double detector_settling_time_ms;
    double detector_settling_time_pan;
    double along_ifov_ms;
    double along_ifov_pan;
    double across_ifov_ms;
    double across_ifov_pan;
    double rollover_error_tolerance;
    double frame_time_fill_offset_tolerance;
    double frame_time_clock_drift_tolerance;
}IAS_CPF_OLI_PARAMETERS;

/* Sensor Orbit Attributes */
typedef struct IAS_CPF_ORBIT_PARAMETERS
{
    char launch_date[20];
    int wrs_cycle_days;
    int wrs_cycle_orbits;
    int scenes_per_orbit;
    double orbital_period;
    double nominal_angular_momentum;
    double nominal_orbit_radius;
    double semi_major_axis;
    double semi_minor_axis;
    double eccentricity;
    double inclination_angle;
    double argument_of_perigee;
    int descending_node_row;
    double long_path1_row60;
    char descending_node_time_min[6];
    char descending_node_time_max[6];
    double nodal_regression_rate;
}IAS_CPF_ORBIT_PARAMETERS;

/* OLI & TIRS radiometric processing, Radiance Rescaling */
typedef struct IAS_CPF_RADIANCE_RESCALE
{
    double bias[IAS_MAX_NBANDS];
    double gain[IAS_MAX_NBANDS];
}IAS_CPF_RADIANCE_RESCALE;

/* TIRS radiometric processing, Thermal Constant */
typedef struct IAS_CPF_TIRS_THERMAL_CONSTANTS
{
    double k1_constant[IAS_TIRS_MAX_NBANDS];
    double k2_constant[IAS_TIRS_MAX_NBANDS];
}IAS_CPF_TIRS_THERMAL_CONSTANTS;

/* OLI & TIRS radiometric processing, Radiance Rescaling */
typedef struct IAS_CPF_REFLECTANCE_CONVERSION
{
    double reflect_conv_coeff[IAS_MAX_NBANDS];
}IAS_CPF_REFLECTANCE_CONVERSION;

/* OLI & TIRS radiomentric processing, Rel Gain Char 90 degree Yaw */
typedef struct IAS_CPF_RELATIVE_GAINS
{
    double *per_detector[IAS_MAX_NBANDS][IAS_MAX_NSCAS];
}IAS_CPF_RELATIVE_GAINS;

/* TIRS Response Lineriazation look up table */
/* NOTE: this may be going away, replaced by external file */
typedef struct IAS_CPF_RESPONSE_LINEARIZATION
{
    /* per-detector cutoff thresholds for low and high cutoff.
       there are are two thresholds per detector.     */
    double *low_cutoff_thresholds[IAS_TIRS_MAX_NBANDS][IAS_TIRS_MAX_NSCAS];
    double *high_cutoff_thresholds[IAS_TIRS_MAX_NBANDS][IAS_TIRS_MAX_NSCAS];
    /* The per-detector remapping coefficients for the 
       low, mid, and high ranges  DN values           */
    double *low_remap_coeff0[IAS_TIRS_MAX_NBANDS][IAS_TIRS_MAX_NSCAS];
    double *low_remap_coeff1[IAS_TIRS_MAX_NBANDS][IAS_TIRS_MAX_NSCAS];
    double *low_remap_coeff2[IAS_TIRS_MAX_NBANDS][IAS_TIRS_MAX_NSCAS];
    double *mid_remap_coeff0[IAS_TIRS_MAX_NBANDS][IAS_TIRS_MAX_NSCAS];
    double *mid_remap_coeff1[IAS_TIRS_MAX_NBANDS][IAS_TIRS_MAX_NSCAS];
    double *mid_remap_coeff2[IAS_TIRS_MAX_NBANDS][IAS_TIRS_MAX_NSCAS];
    double *high_remap_coeff0[IAS_TIRS_MAX_NBANDS][IAS_TIRS_MAX_NSCAS];
    double *high_remap_coeff1[IAS_TIRS_MAX_NBANDS][IAS_TIRS_MAX_NSCAS];
    double *high_remap_coeff2[IAS_TIRS_MAX_NBANDS][IAS_TIRS_MAX_NSCAS];
}IAS_CPF_RESPONSE_LINEARIZATION;

/* OLI & TIRS radiometric processing, Saturated Pixel Char, Saturated
   Pixel Replacement */
typedef struct IAS_CPF_SATURATION_LEVEL
{
    double low_radiance_saturation[IAS_MAX_NBANDS];
    double high_radiance_saturation[IAS_MAX_NBANDS];
    int *analog_low_saturation_level[IAS_MAX_TOTAL_BANDS][IAS_MAX_NSCAS];   
    int *analog_high_saturation_level[IAS_MAX_TOTAL_BANDS][IAS_MAX_NSCAS]; 
    int *digital_low_saturation_level[IAS_MAX_TOTAL_BANDS][IAS_MAX_NSCAS]; 
    int *digital_high_saturation_level[IAS_MAX_TOTAL_BANDS][IAS_MAX_NSCAS];
}IAS_CPF_SATURATION_LEVEL;

/* OLI radiometric processing, Sca Overlp Statistics, Striping Char */
typedef struct IAS_CPF_SCA_PARAMETERS
{
    double discontinuity_ratio[IAS_MAX_NBANDS][IAS_MAX_NSCAS-1];
    double sca_overlap_threshold[IAS_MAX_NBANDS];
    double stripe_cutoff[IAS_MAX_NBANDS];
    int max_valid_correlation_shift[IAS_MAX_NBANDS];
    int min_valid_neighbor_segments[IAS_MAX_NBANDS]; 
}IAS_CPF_SCA_PARAMETERS;

/* OLI & TIRS Temperature Sensitivity Correction */
typedef struct IAS_CPF_TEMP_SENSITIVITY
{
    double oli_reference_temp;
    double tirs_reference_temp;
    int oli_thermistor_flag[2];
    int tirs_thermistor_flag[4];
    double *temp_sensitivity_coeff[IAS_MAX_NBANDS][IAS_MAX_NSCAS];
}IAS_CPF_TEMP_SENSITIVITY;

/* TIRS geometric processing Alignment Calibration */
typedef struct IAS_CPF_TIRS_ALIGN_CAL
{
   int corr_window_size[2]   /* 0 = line, 1 = sample */;
   double min_corr_strength;
   double max_displacement_offset;
   double fill_threshold;
   int corr_fit_method;
   int fit_order;
   double tie_point_weight;
   double along_postfit_rmse_threshold;
   double across_postfit_rmse_threshold;
   double align_constraint_weight;
}IAS_CPF_TIRS_ALIGN_CAL;

/* TIRS radiometric processing, Bias Removal, Dark Response Determination,
   TIRS Gain Determination */
typedef struct IAS_CPF_TIRS_DETECTOR_RESPONSE
{
    double *baseline_dark_response[IAS_TIRS_MAX_NBANDS][IAS_TIRS_MAX_NSCAS]; 
    double *background_response[IAS_TIRS_MAX_NBANDS][IAS_TIRS_MAX_NSCAS];
    double *gain_offsets[IAS_TIRS_MAX_NBANDS][IAS_TIRS_MAX_NSCAS];
    double blackbody_thermistor_weights[4];
}IAS_CPF_TIRS_DETECTOR_RESPONSE;

/* TIRS geometric processing, Focal Plane Alignment, LOS Model Creation */
typedef struct IAS_CPF_TIRS_PARAMETERS
{
    int band_count;
    int sca_count;
    double nominal_frame_time;
    double nominal_integration_time;
    double integration_time_tolerance;
    double integration_time_scale;
    double ssm_mirror_angle;
    double ssm_mirror_angle_deviation;
    double ssm_encoder_origin_side_a;
    double ssm_encoder_origin_side_b;
    double ssm_encoder_time_offset;
    double ssm_telescope_roll_offset;
    double ssm_telescope_pitch_offset;
    double ssm_telescope_yaw_offset;
    double ssm_tolerance;
    double along_ifov_thermal;
    double across_ifov_thermal;
    double frame_time_fill_offset_tolerance;
    double frame_time_clock_drift_tolerance;
}IAS_CPF_TIRS_PARAMETERS;

/* OLI & TIRS geometric processing, LOS model creation, MTF bridge char
   Off Nadir Scene Framing, */
typedef struct IAS_CPF_UT1_TIME_PARAMETERS
{
    int ut1_year[IAS_UT1_PERIOD];
    char ut1_month[IAS_UT1_PERIOD][IAS_UT1_MONTH_STRLEN];
    int ut1_day[IAS_UT1_PERIOD];
    int ut1_modified_julian[IAS_UT1_PERIOD];
    double ut1_x[IAS_UT1_PERIOD];
    double ut1_y[IAS_UT1_PERIOD];
    double ut1_utc[IAS_UT1_PERIOD];
}IAS_CPF_UT1_TIMES;

/* forward reference to the CPF.  Only the CPF file IO library should be
   able to see the IAS_CPF structure definition. */
typedef struct IAS_CPF IAS_CPF;

/***********************************************************************
  Prototype functions for READING the ODL IAS L8 OLI/TIRS CPF
***********************************************************************/
const struct IAS_CPF_HISTOGRAM_CHAR *ias_cpf_get_histogram_char
(
    IAS_CPF *cpf
);

const struct IAS_CPF_LOS_MODEL_CORRECTION *ias_cpf_get_los_model_correction
(
    IAS_CPF *cpf
);

const struct IAS_CPF_DETECTOR_STATUS *ias_cpf_get_tirs_det_status_blind
(
    IAS_CPF *cpf
);

const struct IAS_CPF_TIRS_DETECTOR_RESPONSE *ias_cpf_get_tirs_det_response_blind
(
    IAS_CPF *cpf
);

const struct IAS_CPF_TIRS_DETECTOR_RESPONSE *ias_cpf_get_tirs_det_response
(
    IAS_CPF *cpf
);

const struct IAS_CPF_TIRS_ALIGN_CAL *ias_cpf_get_tirs_align_cal
(
    IAS_CPF *cpf
);

const struct IAS_CPF_AVERAGE_BIAS *ias_cpf_get_oli_avg_bias
(
    IAS_CPF *cpf
);


const struct IAS_CPF_RADIANCE_RESCALE *ias_cpf_get_radiance_rescale
(
    IAS_CPF *cpf
);

const struct IAS_CPF_TIRS_THERMAL_CONSTANTS *ias_cpf_get_tirs_thermal_constants
(
    IAS_CPF *cpf
);

const struct IAS_CPF_LUNAR_IRRADIANCE *ias_cpf_get_lunar_irradiance
(
    IAS_CPF *cpf
);

const struct IAS_CPF_LAMP_RADIANCE *ias_cpf_get_lamp_radiance
(
    IAS_CPF *cpf
);

const struct IAS_CPF_IMPULSE_NOISE *ias_cpf_get_impulse_noise
(
    IAS_CPF *cpf
);

const struct IAS_CPF_TEMP_SENSITIVITY *ias_cpf_get_temp_sensitivity
(
    IAS_CPF *cpf
);

const struct IAS_CPF_REFLECTANCE_CONVERSION *ias_cpf_get_reflect_conv
(
    IAS_CPF *cpf
);

const struct IAS_CPF_SATURATION_LEVEL *ias_cpf_get_saturation_level
(
    IAS_CPF *cpf
);

const struct IAS_CPF_RELATIVE_GAINS *ias_cpf_get_relative_gains
(
    IAS_CPF *cpf
);

const struct IAS_CPF_RELATIVE_GAINS *ias_cpf_get_prelaunch_relative_gains
(
    IAS_CPF *cpf
);

const struct IAS_CPF_RELATIVE_GAINS *ias_cpf_get_postlaunch_relative_gains
(
    IAS_CPF *cpf
);

const struct IAS_CPF_RELATIVE_GAINS *ias_cpf_get_tirs_rel_gains_blind
(
    IAS_CPF *cpf
);

const struct IAS_CPF_RELATIVE_GAINS *ias_cpf_get_tirs_pre_rel_gains_blind
(
    IAS_CPF *cpf
);

const struct IAS_CPF_RELATIVE_GAINS *ias_cpf_get_tirs_post_rel_gains_blind
(
    IAS_CPF *cpf
);

const struct IAS_CPF_I2I_ASSESSMENT *ias_cpf_get_i2i_assessment
(
    IAS_CPF *cpf
);

const struct IAS_CPF_GEO_SYSTEM *ias_cpf_get_geo_system
(
    IAS_CPF *cpf
);

const struct IAS_CPF_GCP_CORRELATION *ias_cpf_get_gcp_correlation
(
    IAS_CPF *cpf
);

const struct IAS_CPF_FOCAL_PLANE_CAL *ias_cpf_get_focal_plane_cal
(
    IAS_CPF *cpf
);

const struct IAS_CPF_NONUNIFORMITY *ias_cpf_get_nonuniformity
(
    IAS_CPF *cpf
);

const struct IAS_CPF_FOCAL_PLANE *ias_cpf_get_focal_plane
(
    IAS_CPF *cpf
);

const struct IAS_CPF_DIFFUSER_RADIANCE *ias_cpf_get_diffuser_rad
(
    IAS_CPF *cpf
);

const struct IAS_CPF_DETECTOR_OFFSETS *ias_cpf_get_det_offsets
(
    IAS_CPF *cpf
);

const struct IAS_CPF_DETECTOR_STATUS *ias_cpf_get_det_status
(
    IAS_CPF *cpf
);

const struct IAS_CPF_DETECTOR_NOISE *ias_cpf_get_det_noise
(
    IAS_CPF *cpf
);

const struct IAS_CPF_B2B_ASSESSMENT *ias_cpf_get_b2b_assessment
(
    IAS_CPF *cpf
);

const struct IAS_CPF_ATTITUDE_PARAMETERS *ias_cpf_get_attitude_params
(
    IAS_CPF *cpf
);

const struct IAS_CPF_ANCILLARY_ENG_CONV *ias_cpf_get_ancil_conv
(
    IAS_CPF *cpf
);

const struct IAS_CPF_ANCILLARY_QA_THRESHOLDS *ias_cpf_get_ancil_qa_thresholds
(
    IAS_CPF *cpf
);

const struct IAS_CPF_ABSOLUTE_GAINS *ias_cpf_get_abs_gains
(
    IAS_CPF *cpf
);

const struct IAS_CPF_ABSOLUTE_GAINS *ias_cpf_get_tirs_abs_gains_blind
(
    IAS_CPF *cpf
);

const struct IAS_CPF_EARTH_CONSTANTS *ias_cpf_get_earth_const
(
    IAS_CPF *cpf
);

const struct IAS_CPF_FILE_ATTRIBUTES *ias_cpf_get_file_attributes
(
    IAS_CPF *cpf
);

const struct IAS_CPF_ORBIT_PARAMETERS *ias_cpf_get_orbit
(
    IAS_CPF *cpf
);

const struct IAS_CPF_OLI_PARAMETERS *ias_cpf_get_oli_parameters
(
    IAS_CPF *cpf
);

const struct IAS_CPF_TIRS_PARAMETERS *ias_cpf_get_tirs_parameters
(
    IAS_CPF *cpf
);

const struct IAS_CPF_SCA_PARAMETERS *ias_cpf_get_sca_parameters
(
    IAS_CPF *cpf
);

const struct IAS_CPF_UT1_TIME_PARAMETERS *ias_cpf_get_ut1_times
(
    IAS_CPF *cpf
);

const struct IAS_CPF_CLOUD_COVER_ASSESSMENT *ias_cpf_get_cloud_cover_assessment
(
    IAS_CPF *cpf
);

struct IAS_CPF *ias_cpf_read
(
    const char *filename      /* I: name of the CPF file to read */
);

void ias_cpf_free
(
    IAS_CPF *cpf
);

#endif
