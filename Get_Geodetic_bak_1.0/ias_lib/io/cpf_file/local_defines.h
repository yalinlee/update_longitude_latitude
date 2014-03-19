#ifndef LOCAL_DEFINES_H
#define LOCAL_DEFINES_H

/***********************************************************************
NAME:       local_defines.h

PURPOSE:    Provide the local structure, constants and prototype
            definitions for the CPF i/o lib.

******************************************************************************/

#include "ias_satellite_attributes.h"
#include "ias_odl.h"
#include "ias_cpf.h"
#include "ias_const.h"
#include "ias_logging.h"

#define ATTRIB_STRLEN 50  /* local define for attribute string length */

/* Define macros to make it easier to use the group cache */
#define GET_GROUP_FROM_CACHE(cpf, group, tree) \
    tree = ias_cpf_get_odl_tree_from_cache(cpf, group); \
    if (!tree) \
    { \
        IAS_LOG_ERROR("Error reading CPF group %s from cache", group); \
        return ERROR; \
    }

#define DROP_ODL_TREE(tree) \
    ias_odl_free_tree(tree); \
    tree = NULL

/***********************************************************************
  The CPF Stucture 
***********************************************************************/
struct IAS_CPF
{
    struct IAS_CPF_ABSOLUTE_GAINS abs_gains;
    struct IAS_CPF_ABSOLUTE_GAINS tirs_abs_gains_blind;
    struct IAS_CPF_ANCILLARY_ENG_CONV ancil_eng_conv;
    struct IAS_CPF_ANCILLARY_QA_THRESHOLDS ancil_qa_thresholds;
    struct IAS_CPF_ATTITUDE_PARAMETERS attitude;
    struct IAS_CPF_AVERAGE_BIAS oli_avg_bias;
    struct IAS_CPF_B2B_ASSESSMENT b2b_assess;
    struct IAS_CPF_CLOUD_COVER_ASSESSMENT cc_assessment;
    struct IAS_CPF_DETECTOR_NOISE detector_noise;
    struct IAS_CPF_DETECTOR_OFFSETS detector_offsets;
    struct IAS_CPF_DETECTOR_STATUS detector_status;
    struct IAS_CPF_DETECTOR_STATUS tirs_det_status_blind;
    struct IAS_CPF_DIFFUSER_RADIANCE diffuser_rad;
    struct IAS_CPF_EARTH_CONSTANTS earth;
    struct IAS_CPF_FILE_ATTRIBUTES file_attribs;
    struct IAS_CPF_FOCAL_PLANE focal_plane;
    struct IAS_CPF_FOCAL_PLANE_CAL fp_cal;
    struct IAS_CPF_GCP_CORRELATION gcp_corr;
    struct IAS_CPF_GEO_SYSTEM geo_sys;
    struct IAS_CPF_HISTOGRAM_CHAR histogram_char;
    struct IAS_CPF_I2I_ASSESSMENT i2i_assess;
    struct IAS_CPF_IMPULSE_NOISE impulse_noise;
    struct IAS_CPF_LAMP_RADIANCE lamp_rad;
    struct IAS_CPF_LOS_MODEL_CORRECTION los_model_correction;
    struct IAS_CPF_LUNAR_IRRADIANCE lunar_irrad;
    struct IAS_CPF_NONUNIFORMITY nonuniformity;
    struct IAS_CPF_OLI_PARAMETERS oli_parameters;
    struct IAS_CPF_ORBIT_PARAMETERS orbit;
    struct IAS_CPF_RELATIVE_GAINS rel_gains;
    struct IAS_CPF_RELATIVE_GAINS pre_rel_gains;
    struct IAS_CPF_RELATIVE_GAINS post_rel_gains;
    struct IAS_CPF_RELATIVE_GAINS tirs_rel_gains_blind;
    struct IAS_CPF_RELATIVE_GAINS tirs_pre_rel_gains_blind;
    struct IAS_CPF_RELATIVE_GAINS tirs_post_rel_gains_blind;
    struct IAS_CPF_SATURATION_LEVEL saturation;
    struct IAS_CPF_SCA_PARAMETERS sca_parms;
    struct IAS_CPF_RADIANCE_RESCALE radiance_rescale;
    struct IAS_CPF_TIRS_THERMAL_CONSTANTS tirs_thermal_constants;
    struct IAS_CPF_REFLECTANCE_CONVERSION reflect_conv;
    struct IAS_CPF_TEMP_SENSITIVITY temp_sens;
    struct IAS_CPF_TIRS_ALIGN_CAL tirs_align_cal;
    struct IAS_CPF_TIRS_DETECTOR_RESPONSE tirs_det_response;
    struct IAS_CPF_TIRS_DETECTOR_RESPONSE tirs_det_response_blind;
    struct IAS_CPF_FOCAL_PLANE tirs_focal_plane;
    struct IAS_CPF_TIRS_PARAMETERS tirs_parameters;
    struct IAS_CPF_UT1_TIME_PARAMETERS ut1_times;

    /* flags to indicate specified structures have been loaded */
    int histogram_char_loaded;
    int los_model_correction_loaded;
    int tirs_det_status_blind_loaded;
    int tirs_det_response_loaded;
    int tirs_det_response_blind_loaded;
    int file_attribs_loaded;
    int impulse_noise_loaded;
    int focal_plane_loaded;
    int fp_cal_loaded;
    int earth_loaded;
    int orbit_loaded;
    int oli_parameter_loaded;
    int tirs_parameter_loaded;
    int attitude_loaded;
    int temp_sens_loaded;
    int radiance_rescale_loaded;
    int tirs_thermal_constants_loaded;
    int reflect_conv_loaded;
    int ancil_qa_thresh_loaded;
    int ancil_eng_conv_loaded;
    int b2b_assess_loaded;
    int geo_sys_loaded;
    int gcp_corr_loaded;
    int diffuser_rad_loaded;
    int i2i_assess_loaded;
    int sca_parm_loaded;
    int saturation_loaded;
    int nonuniformity_loaded;
    int abs_gains_loaded;
    int detector_status_loaded;
    int detector_offsets_loaded;
    int avg_gains_loaded;
    int lamp_rad_loaded;
    int pre_rel_gains_loaded;
    int post_rel_gains_loaded;
    int rel_gains_loaded;
    int tirs_rel_gains_blind_loaded;
    int tirs_pre_rel_gains_blind_loaded;
    int tirs_post_rel_gains_blind_loaded;
    int detector_noise_loaded;
    int lunar_irrad_loaded;
    int ut1_times_loaded;
    int tirs_align_cal_loaded;
    int tirs_abs_gains_blind_loaded;
    int avg_bias_loaded;
    int cc_assessment_loaded;

    char *raw_file_buffer;  /* buffer that holds the entire contents of the 
                               CPF file */
};

/***********************************************************************
  Function prototypes
***********************************************************************/
void ias_cpf_free_nonuniformity_memory
(
    struct IAS_CPF_NONUNIFORMITY *nonuniformity /* cpf struct to free */
);

void ias_cpf_free_saturation_level_memory
(
    struct IAS_CPF_SATURATION_LEVEL *saturation /* cpf saturation struct */
);

int ias_cpf_parse_histogram_characterization
(
    const IAS_CPF *cpf,                       /* I: CPF structure */
    struct IAS_CPF_HISTOGRAM_CHAR *histogram_char /* O: Struct for histogram
                                                        characterization */
);

int ias_cpf_parse_los_model_correction
(
    const IAS_CPF *cpf,                       /* I: CPF structure */
    struct IAS_CPF_LOS_MODEL_CORRECTION *los_model_correction 
                                              /* O: Struct for los model 
                                                    correction */
);
int ias_cpf_parse_oli_radiance_rescale
(
    const IAS_CPF *cpf,                       /* I: CPF structure */
    struct IAS_CPF_RADIANCE_RESCALE *radiance_rescale 
                                              /* O: Struct for rad rescale */
);

int ias_cpf_parse_tirs_radiance_rescale
(
    const IAS_CPF *cpf,                       /* I: CPF structure */
    struct IAS_CPF_RADIANCE_RESCALE *radiance_rescale 
                                              /* O: Struct for rad rescale */
);

int ias_cpf_parse_tirs_thermal_constants
(
    const IAS_CPF *cpf,                  /* I: CPF structure */
    struct IAS_CPF_TIRS_THERMAL_CONSTANTS *tirs_thermal_constants 
                                         /* O: CPF thermail constants data */
);

int ias_cpf_parse_tirs_detector_response
(
    const IAS_CPF *cpf,                       /* I: CPF structure */
    struct IAS_CPF_TIRS_DETECTOR_RESPONSE *tirs_det_response 
                                              /* O: Struct for det response */
);

int ias_cpf_parse_tirs_detector_response_blind
(
    const IAS_CPF *cpf,                       /* I: CPF structure */
    struct IAS_CPF_TIRS_DETECTOR_RESPONSE *tirs_det_response_blind 
                                              /* O: Struct for det response */
);

int ias_cpf_parse_oli_avg_bias
(
    const IAS_CPF *cpf,                       /* I: CPF structure */
    struct IAS_CPF_AVERAGE_BIAS *oli_avg_bias /* O: Struct for average bias */
);

int ias_cpf_parse_lunar_irradiance
(
    const IAS_CPF *cpf,                   /* I: CPF structure */
    struct IAS_CPF_LUNAR_IRRADIANCE *lunar_irrad 
                                          /* O: Struct for lunar irradiance */
);

int ias_cpf_parse_tirs_align_cal
(
    const IAS_CPF *cpf,                   /* I: CPF structure */
    struct IAS_CPF_TIRS_ALIGN_CAL *tirs_align_cal 
                                          /* O: Structure for tirs align cal */
);

int ias_cpf_parse_impulse_noise
(
    const IAS_CPF *cpf,                    /* I: CPF structure */
    struct IAS_CPF_IMPULSE_NOISE *impulse_noise     
                                           /* O: Structure for impulse nosie */
);

int ias_cpf_parse_oli_det_offsets
(
    const IAS_CPF *cpf,                   /* I: CPF structure */
    struct IAS_CPF_DETECTOR_OFFSETS *detector_offsets 
                                          /* O: Structure for detector ofsets */
);

int ias_cpf_parse_tirs_det_offsets
(
    const IAS_CPF *cpf,                /* I: CPF structure */
    struct IAS_CPF_DETECTOR_OFFSETS *detector_offsets 
                                       /* O: Struct for tirs detector offsets */
);

int ias_cpf_parse_oli_det_status
(
    const IAS_CPF *cpf,                 /* I: CPF structure */
    struct IAS_CPF_DETECTOR_STATUS *detector_status 
                                        /* O: Structure for det status */
);

int ias_cpf_parse_tirs_det_status
(
    const IAS_CPF *cpf,                 /* I: CPF structure */
    struct IAS_CPF_DETECTOR_STATUS *tirs_det_status 
                                        /* O: Struct for det status */
);

int ias_cpf_parse_tirs_det_status_blind
(
    const IAS_CPF *cpf,                 /* I: CPF structure */
    struct IAS_CPF_DETECTOR_STATUS *det_status_blind       
                                        /* O: Struct det status */
);

int ias_cpf_parse_ut1_times
(
    const IAS_CPF *cpf,                  /* I: CPF structure */
    struct IAS_CPF_UT1_TIME_PARAMETERS *ut1_times  
                                         /* O: cpf struct for gcp corr group */
);

int ias_cpf_parse_lamp_radiance
(
    const IAS_CPF *cpf,             /* I: CPF structure */
    struct IAS_CPF_LAMP_RADIANCE *lamp_rad  
                                    /* O: Structure for lamp radiance params */
);

int ias_cpf_parse_sca_parameters
(
    const IAS_CPF *cpf,                         /* I: CPF structure */
    int sensor,                                 /* I: sensor, tirs or oli */
    const char *group_name,                     /* I: group to get */
    struct IAS_CPF_SCA_PARAMETERS *sca_parms    /* O: Struct for sca params */
); 

int ias_cpf_parse_saturation_level
(
    const IAS_CPF *cpf,                 /* I: CPF structure */
    int sensor,                         /* I: sensor tirs or oli */
    const char *group_name,             /* I: group to retrieve */
    struct IAS_CPF_SATURATION_LEVEL *saturation   
                                        /* O: cpf saturation structure */
);

int ias_cpf_parse_relative_gains
(
    const IAS_CPF *cpf,                 /* I: CPF structure */
    int sensor,                         /* I: sensor tirs or oli */
    int band_type,                      /* I: normal or blind */
    const char *group_name,             /* I: group to retrieve */
    const char *attribute_prefix,       /* I: Rel_Gains, Pre_Rel_Gains,
                                              Post_Rel_Gains */
    struct IAS_CPF_RELATIVE_GAINS *rel_gains 
                                        /* O: Struct for relative gains */
);

int ias_cpf_parse_tirs_rel_gains_blind
(
    const IAS_CPF *cpf,                 /* I: CPF structure */
    const char *group_name,             /* I: group to retrieve */
    const char *attribute_prefix,       /* I: Rel_Gains_Blind,
                                              Pre_Rel_Gains_Blind,
                                              Post_Rel_Gains_Blind */
    struct IAS_CPF_RELATIVE_GAINS *tirs_rel_gains_blind 
                                        /* O: Struct for tirs relative gains */
);

int ias_cpf_parse_nonuniformity
(
    const IAS_CPF *cpf,                         /* I: CPF structure */
    int sensor,                                 /* I: sensor tirs or oli */
    const char *group_name,                     /* I: group to retrieve */
    struct IAS_CPF_NONUNIFORMITY *nonuniformity /* O: Struct for det offsets */
);

int ias_cpf_parse_i2i_assessment
(
    const IAS_CPF *cpf,                 /* I: CPF structure */
    struct IAS_CPF_I2I_ASSESSMENT *i2i_assess  
                                        /* O: Structure for i2i_assessment */
);

int ias_cpf_parse_oli_det_noise
(
    const IAS_CPF *cpf,                           /* I: CPF structure */
    struct IAS_CPF_DETECTOR_NOISE *oli_detector_noise 
                                                  /* O: Struct for det noise */
);

int ias_cpf_parse_tirs_det_noise
(
    const IAS_CPF *cpf,                     /* I: CPF structure */
    struct IAS_CPF_DETECTOR_NOISE *tirs_detector_noise 
                                            /* O: Structure for det noise */
);

int ias_cpf_parse_geo_system
(
    const IAS_CPF *cpf,                /* I: CPF structure */
    struct IAS_CPF_GEO_SYSTEM *geo_sys /* O: structure for geo systems group */
);

int ias_cpf_parse_diffuser_rad
(
    const IAS_CPF *cpf,                   /* I: CPF structure */
    struct IAS_CPF_DIFFUSER_RADIANCE *diffuser_rad 
                                          /* O: cpf diffuser radiance struct*/
);

int ias_cpf_parse_gcp_correlation
(
    const IAS_CPF *cpf,                  /* I: CPF structure */
    struct IAS_CPF_GCP_CORRELATION *gcp_corr 
                                         /* O: cpf struct for gcp corr group */
);

int ias_cpf_parse_b2b_assessment
(
    const IAS_CPF *cpf,                     /* I: CPF structure */
    struct IAS_CPF_B2B_ASSESSMENT *b2b_assess 
                                            /* O: Struct for b2b_assessment */
);

int ias_cpf_parse_ancil_qa_thresholds
(
    const IAS_CPF *cpf,                     /* I: CPF structure */
    struct IAS_CPF_ANCILLARY_QA_THRESHOLDS *ancil_qa_thresholds 
                                            /* O: struct for ancil data */
);

int ias_cpf_parse_ancil_conv
(
    const IAS_CPF *cpf,                 /* I: CPF structure */
    struct IAS_CPF_ANCILLARY_ENG_CONV *ancil_eng_conv  
);                                      /* O: structure for ancillary data */

int ias_cpf_parse_oli_abs_gains
(
    const IAS_CPF *cpf,                       /* I: CPF structure */
    struct IAS_CPF_ABSOLUTE_GAINS *abs_gains  /* O: structure for abs gains */
);

int ias_cpf_parse_tirs_abs_gains
(
    const IAS_CPF *cpf,                      /* I: CPF structure */
    struct IAS_CPF_ABSOLUTE_GAINS *abs_gains /* O: struct for abs gains */
);

int ias_cpf_parse_tirs_abs_gains_blind
(
    const IAS_CPF *cpf,                          /* I: CPF structure */
    struct IAS_CPF_ABSOLUTE_GAINS *tirs_abs_gains_blind 
                                                 /* O: struct for abs gains */
);

int ias_cpf_parse_file_attributes
(
    const IAS_CPF *cpf,                      /* I: CPF structure */
    struct IAS_CPF_FILE_ATTRIBUTES *file_att /* O: structure for file atts */
);

int ias_cpf_parse_reflect_conv
(
    const IAS_CPF *cpf,                      /* I: CPF structure */
    struct IAS_CPF_REFLECTANCE_CONVERSION *reflect_conv 
                                             /* O: structure for reflectance 
                                                   conversion values */
);

int ias_cpf_parse_oli_temp_sens
(
    const IAS_CPF *cpf,                        /* I: CPF structure */
    struct IAS_CPF_TEMP_SENSITIVITY *temp_sens /* O: temp sensitivity struct */
);

int ias_cpf_parse_tirs_temp_sens
(
    const IAS_CPF *cpf,                        /* I: CPF structure */
    struct IAS_CPF_TEMP_SENSITIVITY *tirs_temp_sens 
                                               /* O: CPF det offsets data */
);

int ias_cpf_parse_focal_plane_cal
(
    const IAS_CPF *cpf,                 /* I: CPF structure */
    struct IAS_CPF_FOCAL_PLANE_CAL *fp_cal 
                                        /* O: struct for oli focal plane cal */
);

int ias_cpf_parse_oli_focal_plane
(
    const IAS_CPF *cpf,                     /* I: CPF structure */
    struct IAS_CPF_FOCAL_PLANE *focal_plane /* O: struct for oli focal plane */
);

int ias_cpf_parse_tirs_focal_plane
(
    const IAS_CPF *cpf,                     /* I: CPF structure */
    struct IAS_CPF_FOCAL_PLANE *focal_plane /* O: struct for oli focal plane */
);

int ias_cpf_parse_earth_const
(
    const IAS_CPF *cpf,                    /* I: CPF structure */
    struct IAS_CPF_EARTH_CONSTANTS *earth  /* O: struct for earth constants */
);

int ias_cpf_parse_oli_parameters
(
    const IAS_CPF *cpf,                      /* I: CPF structure */
    struct IAS_CPF_OLI_PARAMETERS *oli_parameters 
                                             /* O: struct for OLI parameters */
);

int ias_cpf_parse_tirs_parameters
(
    const IAS_CPF *cpf,                     /* I: CPF structure */
    struct IAS_CPF_TIRS_PARAMETERS *tirs_parameters     
                                            /* O: struct for TIRS parameters */
);

int ias_cpf_parse_attitude_params
(
    const IAS_CPF *cpf,                 /* I: CPF structure */
    struct IAS_CPF_ATTITUDE_PARAMETERS *attitude
);                                      /* O: struct for attitude parameters */

int ias_cpf_parse_orbit_parameters
(
    const IAS_CPF *cpf,                /* I: CPF structure */
    struct IAS_CPF_ORBIT_PARAMETERS *orbit
);                                     /* O: struct for orbit parameters */

int ias_cpf_parse_cloud_cover_assessment
(
    const IAS_CPF *cpf,                /* I: CPF structure */
    struct IAS_CPF_CLOUD_COVER_ASSESSMENT *cc_assessment
);

int ias_cpf_cache_file
(
    const char *filename,        /* I: CPF file name */
    IAS_CPF *cpf                 /* O: CPF struct with array containing cpf */
);

IAS_OBJ_DESC *ias_cpf_get_odl_tree_from_cache
(
    const IAS_CPF *cpf,         /* I: CPF struct with array containing cpf */
    const char *group_name      /* I: Name of group to retrieve */
);

int ias_cpf_convert_3digit_month_to_number
(
    char *ascii_3digit_month,  /* I: Three char string representing month */
    int *number                /* O: The numerical value of the month */
);

#endif
